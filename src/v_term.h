#ifndef V_TERM_H
#define V_TERM_H
#include "common.h"
#include "v_config.h"
#include <memory>
#include "v_tab.h"

using namespace std;
namespace VTERM{

    class VTerm{
        public:

            // Main Gtk window
            GtkWidget* window;

            // Tab bar, i.e. Gtk notebook
            GtkNotebook* notebook;

            // Runtime variables
            gchar** user_def_shell = get_user_shell();
            gchar* user_home = g_strdup(g_get_home_dir());

            // Notebook pages to vtab map
            map<GtkWidget*, VTab*> hboxVTabMap;

            // up-to-date window size info
            gint window_width_cache = -1, window_height_cache = -1;

            // Alternate tab
            VTab* alternate_tab = nullptr;

            // Current tab
            VTab* current_tab = nullptr;

            /*
             * Default constructor & destructor
             */
            VTerm();
            ~VTerm();

            /*
             * Events callback functions
             */
            static gboolean window_focus_changed_cb(GtkWidget* _window, GdkEvent *event, gpointer _data);

            static void window_screen_changed_cb(GtkWidget* window, GdkScreen* _prev_screen, gpointer _data);

            static void notebook_switch_page_cb(GtkNotebook* _notebook, GtkWidget* hbox,
                    guint _page_nu, gpointer _data);

            static gboolean window_key_press_cb(GtkWidget* window, GdkEventKey* event, gpointer data);
            static gboolean window_key_release_cb(GtkWidget* window, GdkEventKey* event, gpointer data);

            /*
             * Updates the window's geometry hints to the WM
             */
            static void window_update_geometry(VTab* vtab);

            /*
             * Sets the window size to match current tab & geometry hint
             */
            static void window_set_size();

            /*
             * Connect signals of widgets under vterm
             */
            void connect_signals();

            /*
             * Create first tab, set the geometry hints and run gtk main
             */
            void run();

            /*
             * Get VTab given parent hbox
             */
            VTab* getVTab(GtkWidget* hbox){
                // This throws an exception if not found... good.
                return hboxVTabMap.at(hbox);
            }

            /*
             * Get VTab at page pn
             */
            VTab* getVTab(gint pn){
                GtkWidget* hbox = gtk_notebook_get_nth_page(notebook, pn);
                if(hbox)
                    return getVTab(hbox);
                return nullptr;
            }

            /*
             * Gets current VTab
             */
            VTab* getCurrentVTab(){
                return getVTab(gtk_notebook_get_current_page(notebook));
            }

            /*
             * Inserts new tab to notebook
             */
            void insertVTab(VTab* vtab){
                GtkWidget* hbox = vtab->hbox;

                // Register it in our map
                hboxVTabMap[hbox] = vtab;

                // Figure out the position
                gint pn = -1;
                if(VConf(insert_after_current)){
                    // if there are no pages, this will become 0
                    pn = gtk_notebook_get_current_page(notebook) + 1;
                }

                // insert in the right position
                gtk_notebook_insert_page(notebook, hbox, vtab->tab_label, pn);

                // can reorder by drag & drop
                gtk_notebook_set_tab_reorderable(notebook, hbox, true);

                // flag them to be shown
                gtk_widget_show_all(GTK_WIDGET(hbox));

                // Switch to the new tab
                gtk_notebook_set_current_page(notebook, pn);

                // tab bar should fill the space
                gtk_container_child_set(GTK_CONTAINER(notebook), hbox,
                        "tab-fill", true,
                        "tab-expand", true, nullptr);

                // Give focus to terminal
                gtk_widget_grab_focus(GTK_WIDGET(vtab->vte_terminal));
            }

            /*
             * Deletes vtab
             */
            void deleteVTab(VTab* vtab){
                if(vtab->in_destruction)
                    return;

                vtab->in_destruction = true;
                GtkWidget* hbox = vtab->hbox;

                // If the only tab, exit with success
                if(gtk_notebook_get_n_pages(notebook) == 1)
                    exit_success();

                gtk_widget_destroy(GTK_WIDGET(vtab->hbox));
                gtk_widget_queue_draw(GTK_WIDGET(notebook));

                // Also, explicitly delete this vtab
                delete vtab;

                hboxVTabMap.erase(hbox);
            }

            /*
             * Sync the window title
             */
            void sync_window_title(const gchar* title){
                if(!VConf(window_title))
                    gtk_window_set_title(GTK_WINDOW(window), title);
            }

    };

    // Global vterm
    extern unique_ptr<VTerm> vterm;
}
#endif
