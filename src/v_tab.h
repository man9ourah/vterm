#ifndef V_TAB_H
#define V_TAB_H
#include "vte/vte.h"
#include "common.h"
#include "v_config.h"

#include <map>

using namespace std;
namespace VTERM{
    class VTerm;

    class VTab{
        public:

            /*
             * Parent VTerm object
             */
            VTerm* vterm;

            /*
             * HBox: parent of all!
             */
            GtkBox* hbox;

            /*
             * VTE terminal widget
             */
            VteTerminal* vte_terminal;

            /*
             * Tab label
             */
            GtkLabel* tab_label;

            /*
             * Scrollbar
             */
            GtkScrollbar* scrollbar;

            /*
             * Struct encapsulating current mode info
             */
            struct ModeInfo{
                // Which mode?
                enum ModeOp{
                    INSERT_MODE = 0,

                    NORMAL_MODE,
                } mode;

            } current_mode;

            /*
             * Flag that we are being destructed
             */
            gboolean in_destruction = false;

            /*
             * Static methods
             */

            /*
             * Events callback functions
             */
            static void terminal_child_exit_cb(VteTerminal* _vte_terminal, gint _status, gpointer data);
            static void terminal_create_cb(VteTerminal* _vte_terminal, GPid pid, GError *error, gpointer data);
            static void terminal_title_changed_cb(VteTerminal* vte_terminal, gpointer data);
            static gboolean terminal_key_press_cb(VteTerminal* terminal, GdkEventKey* event, gpointer data);

            /*
             * Static VTab instance builder to decide cmd & cwd
             */
            static VTab* create_tab(VTerm* vterm, gboolean is_first_tab);

            /*
             * Non-static methods
             */

            /*
             * Connect signals of widgets under vtab
             */
            void connect_signals();

            /*
             * Switches the tab to new mode
             */
            void switch_mode(ModeInfo::ModeOp new_mode);

            /*
             * Setter:: create a new tab label
             */
            void create_tab_label();

            /*
             * Setter:: sync tab label with vte_terminal
             */
            void sync_tab_label(const gchar* title){
                gtk_label_set_text(tab_label, title);
            }
        private:
            /*
             * Prevent making instances directly
             */
            VTab(VTerm* vterm, gchar* cwd, gchar** cmd, gchar** env);
            VTab(VTab const&);
            void operator=(VTab const&);

    };
}
#endif
