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
             * Overlay, its main child is vte terminal
             */
            GtkOverlay* overlay;

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
             * Flag that we are being destructed
             */
            gboolean in_destruction = false;

            /*
             * Struct encapsulating current mode of operation
             */
            struct VMode{

                /*
                 * Current mode
                 */
                enum ModeOp{
                    INSERT_MODE = 0,

                    NORMAL_MODE,

                    VISUAL_MODE,
                    VISUAL_LINE_MODE,
                    VISUAL_BLOCK_MODE,
                } mode;

                /*
                 * Search direction
                 */
                enum SearchDir{
                    BACKWARD_SEARCH,

                    FORWARD_SEARCH,
                } search_dir;

                /*
                 * Parent VTab
                 */
                VTab* parent_vtab;

                /*
                 * Cursor indicator
                 */
                GtkDrawingArea* cursor_indicator;

                /*
                 * Search bar widgets
                 */
                GtkSearchEntry* search_entry;

                /*
                 * Static methods
                 */
                /*
                 * Events callback functions
                 */
                static gboolean cursor_indicator_draw_cb(GtkDrawingArea* _cursor_indicator, cairo_t* cr, gpointer data);
                static void cursor_indicator_realize_cb(GtkDrawingArea* cursor_indicator, gpointer _data);
                static void search_entry_next_cb(GtkSearchEntry* _search_entry, gpointer data);
                static void search_entry_prev_cb(GtkSearchEntry* _search_entry, gpointer data);
                static void search_entry_changed_cb(GtkSearchEntry* _search_entry, gpointer data);
                static void search_entry_stop_cb(GtkSearchEntry* _search_entry, gpointer data);

                /*
                 * Non-static methods
                 */

                /*
                 * Search for text
                 */
                void do_search();

                /*
                 * Enters insert mode
                 */
                void enter_insert_mode();

                /*
                 * Enters normal mode
                 */
                void enter_normal_mode();

                /*
                 * Enters visual mode
                 */
                void enter_visual_mode(ModeOp visual_mode_kind);

                /*
                 * Mode specific keyboard events handler
                 */
                gboolean handle_keyboard_events(GdkEventKey* event);

                /*
                 * Show overlay widgets based on mode
                 */
                void show_vmode();

                /*
                 * Switches the tab to new mode
                 */
                void switch_mode(VMode::ModeOp new_mode);

                VMode(VTab* parent_vtab);

            } *current_mode;

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
             * Setter:: create a new tab label
             */
            void create_tab_label();

            /*
             * Setter:: sync tab label with vte_terminal
             */
            void sync_tab_label(const gchar* title){
                gtk_label_set_text(tab_label, title);
            }

            // Destructor
            ~VTab();
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
