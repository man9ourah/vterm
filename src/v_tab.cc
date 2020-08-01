#include "v_tab.h"
#include "common.h"
#include "gdk/gdkkeysyms.h"
#include "v_term.h"
#include <memory>
#include "v_config.h"
#include <gdk/gdkx.h>
#include <limits>
#include "v_keybindings.h"

using namespace std;

namespace VTERM{

    void VTab::terminal_create_cb(VteTerminal* _vte_terminal, GPid pid, GError *error, gpointer data){
        if(pid < 0){
            // Opps!
            // Report the error..
            g_printerr("Could not create a new tab: %s\n", error->message);

            GtkNotebook* notebook = GTK_NOTEBOOK(vterm->notebook);

            // If the only tab, exit with error
            if(gtk_notebook_get_n_pages(notebook) == 1)
                exit_error();

            // Delete the vtab
            vterm->deleteVTab((VTab*)data);

            return;
        }
    }

    void VTab::terminal_child_exit_cb(VteTerminal* _vte_terminal, gint _status, gpointer data){
        vterm->deleteVTab(((VTab*)data));
    }

    void VTab::terminal_title_changed_cb(VteTerminal* vte_terminal, gpointer data){
        VTab* vtab = (VTab*)data;
        const gchar* title = vte_terminal_get_window_title(VTE_TERMINAL(vte_terminal));
        vtab->sync_tab_label(title);
        vterm->sync_window_title(title);
    }

    gboolean VTab::terminal_key_press_cb(GtkWidget* vte_terminal, GdkEventKey* event, gpointer data){
        const guint modifiers = event->state & gtk_accelerator_get_default_mod_mask();
        const guint keypressed = gdk_keyval_to_lower(event->keyval);
        VTab* vtab = (VTab*)data;

        /*
         * This will get really complicated.. so:
         *  - NO FALL THROUGH!! direct fall through when there is no code between 
         *  two cases are fine.
         *
         *  - If case is entered, it should RETURN from the function. It should
         *  not depend on any logic after its parent switch case.
         *
         *  - All cases should be surrounded by {}
         *
         */

        /*
         * We first see to keybindings that should work in all modes.
         */
        if(modifiers == VKEY_MODIFIER){
            switch(keypressed){

                /*
                 * Toggle normal mode
                 */
                case VKEY_TOGGLE_NORMAL:{
                    vtab->switch_mode(ModeInfo::ModeOp::NORMAL_MODE);
                    return true;
                }

                /*
                 * Close this tab
                 */
                case VKEY_CLOSE_TAB:{
                    vterm->deleteVTab(vtab);
                    return true;
                }

                /*
                 * Prompt scroll up
                 */
                case VKEY_UP_PROMPT:{
                    vte_terminal_prompt_prev(VTE_TERMINAL(vte_terminal));
                    return true;
                }

                /*
                 * Prompt scroll down
                 */
                case VKEY_DOWN_PROMPT:{
                    vte_terminal_prompt_next(VTE_TERMINAL(vte_terminal));
                    return true;
                }

                /*
                 * Move tab to right
                 * PageUp seems standard for this too
                 */
                case GDK_KEY_Page_Down:
                case VKEY_MOVE_TAB_RIGHT:{
                    gint max_pn = gtk_notebook_get_n_pages(vterm->notebook);
                    gtk_notebook_reorder_child(vterm->notebook, vtab->hbox,
                            (gtk_notebook_page_num(vterm->notebook, vtab->hbox) + 1) % max_pn);
                    return true;
                }

                /*
                 * Move tab to left
                 * PageDown seems standard for this too
                 */
                case GDK_KEY_Page_Up:
                case VKEY_MOVE_TAB_LEFT:{
                    gtk_notebook_reorder_child(vterm->notebook, vtab->hbox,
                            gtk_notebook_page_num(vterm->notebook, vtab->hbox) - 1);
                    return true;
                }

                /*
                 * Copy
                 */
                case VKEY_COPY:{
                    vte_terminal_copy_clipboard_format(VTE_TERMINAL(vte_terminal), VTE_FORMAT_TEXT);
                    return true;
                }

                /*
                 * Paste
                 */
                case VKEY_PASTE:{
                    vte_terminal_paste_clipboard(VTE_TERMINAL(vte_terminal));
                    return true;
                }

                /*
                 * Standard zoom in keybinding
                 */
                case GDK_KEY_plus:{
                    vte_terminal_set_font_scale(VTE_TERMINAL(vte_terminal),
                            vte_terminal_get_font_scale(VTE_TERMINAL(vte_terminal)) * 1.2);
                    VTerm::window_set_size();
                    return true;
                }
            }

        /*
         * Control only keybindings
         * pretty stnadard keybindings
         */
        }else if(modifiers == GDK_CONTROL_MASK){
            switch(keypressed){
                /*
                 * Standard zoom in keybinding
                 */
                case GDK_KEY_KP_Add:{
                    vte_terminal_set_font_scale(VTE_TERMINAL(vte_terminal),
                            vte_terminal_get_font_scale(VTE_TERMINAL(vte_terminal)) * 1.2);
                    VTerm::window_set_size();
                    return true;
                }

                /*
                 * Standard zoom out keybinding
                 */
                case GDK_KEY_minus:
                case GDK_KEY_KP_Subtract:{
                    vte_terminal_set_font_scale(VTE_TERMINAL(vte_terminal),
                            vte_terminal_get_font_scale(VTE_TERMINAL(vte_terminal)) / 1.2);
                    VTerm::window_set_size();
                    return true;
                }

                /*
                 * Standard? reset font scale
                 */
                case GDK_KEY_equal:{
                    vte_terminal_set_font_scale(VTE_TERMINAL(vte_terminal), VConf(font_scale));
                    VTerm::window_set_size();
                    return true;
                }
            }
        } // endif (modifiers == GDK_CONTROL_MASK)

        /*
         * Now to the keybindings that are specific to certain modes
         */
        switch(vtab->current_mode.mode){
            /*
             * Keybindings specific to normal mode
             */
            case ModeInfo::ModeOp::NORMAL_MODE:{
                if(modifiers == GDK_CONTROL_MASK){

                }else{
                    switch(keypressed){
                        case GDK_KEY_k:
                        case GDK_KEY_Up:{
                            DEBUG_PRINT("\nNormal: cursor up\n");
                            return true;
                        }

                        case GDK_KEY_j:
                        case GDK_KEY_Down:{
                            DEBUG_PRINT("\nNormal: cursor down\n");
                            return true;
                        }

                        case GDK_KEY_l:
                        case GDK_KEY_Left:{
                            DEBUG_PRINT("\nNormal: cursor left\n");
                            return true;
                        }

                        case GDK_KEY_h:
                        case GDK_KEY_Right:{
                            DEBUG_PRINT("\nNormal: cursor right\n");
                            return true;
                        }

                    }
                }

                // Do not pass any event when in normal mode
                return true;
            }

            /*
             * Keybindings specific to insert mode
             */
            case ModeInfo::ModeOp::INSERT_MODE:{
                // None
                // Pass event
                return false;
            }
        }

        return false;
    }

    VTab* VTab::create_tab(gboolean is_first_tab){
        gchar *cwd = VConf(cli_cwd), **cmd = VConf(cli_cmd), **env = nullptr;

#ifdef GDK_WINDOWING_X11
        env = g_get_environ();
        if (GDK_IS_X11_SCREEN(gtk_widget_get_screen(vterm->window))) {
            GdkWindow *gdk_window = gtk_widget_get_window(vterm->window);
            if (gdk_window) {
                char xid_s[std::numeric_limits<long unsigned>::digits10 + 1];
                snprintf(xid_s, sizeof(xid_s), "%lu", GDK_WINDOW_XID(gdk_window));
                env = g_environ_setenv(env, "WINDOWID", xid_s, TRUE);
            }else{
                g_printerr("No gdk window.\n");
            }
        }
#endif

        if(is_first_tab){
            // first tab always use cli or defaults
            if(!cmd)
                cmd = vterm->user_def_shell;
            if(!cwd)
                cwd = nullptr; // vte will default to cwd of parent

        }else{
            // For other tabs, consult config

            if(VConf(tab_cmd) == DEFAULT_CMD)
                // use default shell, note that it is initialized to cli_cmd
                // so CLI_CMD is covered.
                cmd = vterm->user_def_shell;

            if(VConf(tab_cwd) == CURRENT_TAB_CWD){
                // We need to get the current tab cwd
                // Utilize vte OSC 7
                VTab* current_vtab = vterm->getCurrentVTab();
                VteTerminal* vte_terminal = VTE_TERMINAL(current_vtab->vte_terminal);
                const gchar* cwd_uri = vte_terminal_get_current_directory_uri(vte_terminal);

                if(cwd_uri){
                    cwd = g_filename_from_uri(cwd_uri, nullptr, nullptr);
                    VTab* new_tab =  new VTab(cwd, cmd, env);
                    g_free(cwd);
                    return new_tab;
                }else{
                    // In case we did not find it, fallback to vte default which is
                    // cwd of parent
                    g_printerr("Could not retrieve cwd of current tab.. make sure vte.sh is sourced.\n");
                    cwd = nullptr;
                }
            }else if(VConf(tab_cwd) == HOME_CWD){
                cwd = vterm->user_home;
            }
            // The case of CLI_CWD is handled by initialization
        }

        return new VTab(cwd, cmd, env);
    }

    void VTab::connect_signals(){
        //TODO:: handle resize-window signal
        g_signal_connect(vte_terminal, "key-press-event", G_CALLBACK(terminal_key_press_cb), this);
        g_signal_connect(vte_terminal, "child-exited", G_CALLBACK(VTab::terminal_child_exit_cb), this);
        g_signal_connect(vte_terminal, "window-title-changed", G_CALLBACK(VTab::terminal_title_changed_cb), this);
    }

    void VTab::switch_mode(ModeInfo::ModeOp new_mode){
        // First, what mode are we in?
        switch(current_mode.mode){
            case ModeInfo::ModeOp::NORMAL_MODE:{

                // We are in normal mode
                // What mode we want to switch to?
                switch(new_mode){
                    /*
                     * normal->normal TOGGLE(insert)
                     * normal->insert
                     */
                    case ModeInfo::ModeOp::NORMAL_MODE:
                    case ModeInfo::ModeOp::INSERT_MODE:{
                        DEBUG_PRINT("\nNORMAL->INSERT\n");
                        current_mode.mode = ModeInfo::ModeOp::INSERT_MODE;
                        vte_terminal_set_input_enabled(VTE_TERMINAL(vte_terminal), true);
                        return;
                    }
                }

                return;
            }

            case ModeInfo::ModeOp::INSERT_MODE:{

                // We are in insert mode
                // What mode we want to switch to?
                switch(new_mode){
                    /*
                     * insert->normal
                     */
                    case ModeInfo::ModeOp::NORMAL_MODE:{
                        DEBUG_PRINT("\nINSERT->NORMAL\n");
                        current_mode.mode = ModeInfo::ModeOp::NORMAL_MODE;
                        vte_terminal_set_input_enabled(VTE_TERMINAL(vte_terminal), false);
                        return;
                    }

                    /*
                     * insert->insert
                     * this should never happen
                     */
                    case ModeInfo::ModeOp::INSERT_MODE:{
                        DEBUG_PRINT("\nINSERT->INSERT\n");
                        return;
                    }
                }

                return;
            }
        }
    }

    VTab::VTab(gchar* cwd, gchar** cmd, gchar** env){
        // Create terminal
        vte_terminal = vte_terminal_new();
        tab_label = create_tab_label();

        // Box it
        hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start(GTK_BOX(hbox), vte_terminal, true, true, 0);

        // Scrollbar
        if(VConf(show_scrollbar)){
            scrollbar = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL,
                    gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(vte_terminal)));
            gtk_box_pack_start(GTK_BOX(hbox), scrollbar, false, false, 0);
        }

        // Apply the config on vte terminal
        VConfig::getVConfig().apply_vte_config(VTE_TERMINAL(vte_terminal));

        // Connect the signals
        connect_signals();

        // insert it to the notebook
        vterm->insertVTab(this);

        // Finally, spawn the child
        GSpawnFlags spawn_flags = G_SPAWN_SEARCH_PATH_FROM_ENVP;
        vte_terminal_spawn_async(VTE_TERMINAL(vte_terminal), VTE_PTY_DEFAULT, cwd, cmd, env,
                spawn_flags, NULL, NULL, NULL, -1, NULL, terminal_create_cb, this);
    }

}
