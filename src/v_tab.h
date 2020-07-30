#ifndef V_TAB_H
#define V_TAB_H
#include "vte/vte.h"
#include "common.h"
#include <map>
#include "v_config.h"

using namespace std;
namespace VTERM{

    /*
     * Represent one tab
     */
    class VTab{
        public:
            // HBox
            GtkWidget* hbox;

            // vte widget
            GtkWidget* vte_terminal;

            // tab label
            GtkWidget* tab_label;

            // terminal scrollbar
            GtkWidget* scrollbar;

        public:
            /*
             * Static instance builder to decide cmd & cwd
             */
            static VTab* create_tab(gboolean is_first_tab);

            /*
             * Events callback functions
             */
            static void terminal_child_exit_cb(VteTerminal* _vte_terminal, gint _status, gpointer data);
            static void terminal_title_changed_cb(VteTerminal* vte_terminal, gpointer data);
            static void terminal_create_cb(VteTerminal* _vte_terminal, GPid pid, GError *error, gpointer data);

            /*
             * Connect signals of widgets under vtab
             */
            void connect_signals();

            /*
             * Create a new tab label
             */
            GtkWidget* create_tab_label(){
                GtkLabel* label = GTK_LABEL(gtk_label_new("VTerminal"));

                gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_CENTER);
                gtk_widget_set_valign (GTK_WIDGET(label), GTK_ALIGN_BASELINE);
                gtk_widget_set_margin_start(GTK_WIDGET(label), 0);
                gtk_widget_set_margin_end(GTK_WIDGET(label), 0);
                gtk_widget_set_margin_top(GTK_WIDGET(label), 0);
                gtk_widget_set_margin_bottom(GTK_WIDGET(label), 0);

                gtk_label_set_single_line_mode (label, true);
                gtk_label_set_ellipsize(label, VConf(tab_label_trim_first) ?
                                                PANGO_ELLIPSIZE_START :
                                                PANGO_ELLIPSIZE_END);

                gtk_label_set_width_chars(label, 15);

                return GTK_WIDGET(label);
            }

            /*
             * Sync tab label with vte_terminal
             */
            void sync_tab_label(const gchar* title){
                gtk_label_set_text(GTK_LABEL(tab_label), title);
            }

        private:
            /*
             * Prevent making instances directly
             */
            VTab(gchar* cwd, gchar** cmd, gchar** env);
            VTab(VTab const&);
            void operator=(VTab const&);

    };
}
#endif
