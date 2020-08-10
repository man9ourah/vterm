#ifndef V_TERM_H
#define V_TERM_H
#include "common.h"
#include "v_config.h"
#include "v_tab.h"

#include <memory>

using namespace std;
namespace VTERM{

    class VTerm{
        public:

            /*
             * Main Gtk window
             */
            GtkWindow* window;

            /* 
             * Gtk notebook aka tab bar
             */
            GtkNotebook* notebook;

            /*
             * Runtime variables
             */
            gchar** user_def_shell = get_user_shell();
            gchar* user_home = g_strdup(g_get_home_dir());

            /*
             * Notebook pages to vtab objects map
             */
            unordered_map<GtkBox*, VTab*> hboxVTabMap;

            /*
             * up-to-date window size info
             */
            gint window_width_cache = -1, window_height_cache = -1;

            /*
             * Current tab
             */
            VTab* current_tab = nullptr;

            /*
             * Alternate tab
             */
            VTab* alternate_tab = nullptr;

            /*
             * Static methods
             */

            /*
             * Events callback functions
             */

            static void window_realize_cb(GtkWindow* _window, gpointer data);
            static gboolean window_focus_changed_cb(GtkWindow* _window, GdkEvent *event, gpointer data);
            static void window_screen_changed_cb(GtkWindow* window, GdkScreen* _prev_screen, gpointer _data);
            static gboolean window_key_press_cb(GtkWindow* window, GdkEventKey* event, gpointer data);
            static gboolean window_key_release_cb(GtkWindow* window, GdkEventKey* event, gpointer data);
            static void notebook_switch_page_cb(GtkNotebook* _notebook, GtkBox* hbox,
                    guint _page_nu, gpointer data);

            /*
             * Non-static methods
             */

            /*
             * Updates the window's geometry hints to the WM
             */
            void window_update_geometry(VTab* vtab);

            /*
             * Sets the window size to match current tab & geometry hint
             */
            void window_set_size();

            /*
             * Inserts new tab to notebook
             */
            void insertVTab(VTab* vtab);

            /*
             * Deletes vtab
             */
            void deleteVTab(VTab* vtab);

            /*
             * Connect signals of widgets under vterm
             */
            void connect_signals();

            /*
             * Create first tab, set the geometry hints and run gtk main
             */
            void run();

            /*
             * Getter:: Get VTab given parent hbox
             */
            VTab* getVTab(GtkBox* hbox){
                // This throws an exception if not found... good.
                return hboxVTabMap.at(hbox);
            }

            /*
             * Getter:: Get VTab at page pn
             */
            VTab* getVTab(gint pn){
                GtkBox* hbox = GTK_BOX(gtk_notebook_get_nth_page(notebook, pn));
                if(hbox)
                    return getVTab(hbox);
                return nullptr;
            }

            /*
             * Getter:: Gets current VTab
             */
            VTab* getCurrentVTab(){
                return getVTab(gtk_notebook_get_current_page(notebook));
            }

            /*
             * Setter:: Sync the window title
             */
            void sync_window_title(const gchar* title){
                if(!VConf(window_title))
                    gtk_window_set_title(window, title);
            }

            /*
             * Default constructor & destructor
             */
            VTerm();
            ~VTerm();
    };
}
#endif
