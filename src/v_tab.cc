#include "common.h"
#include "v_config.h"
#include "v_keybindings.h"
#include "v_tab.h"
#include "v_term.h"
#include "vtepcre2.h"
#include "terminal-regex.h"

#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <limits>
#include <memory>

using namespace std;

namespace VTERM{

    void VTab::terminal_child_exit_cb(VteTerminal* _vte_terminal, gint _status, gpointer data){
        VTab* vtab = (VTab*)data;
        vtab->vterm->deleteVTab(vtab);
    }

    void VTab::terminal_create_cb(VteTerminal* _vte_terminal, GPid pid, GError *error, gpointer data){
        if(pid < 0){
            // Opps!
            // Report the error..
            g_printerr("Could not create a new tab: %s\n", error->message);

            VTab* vtab = (VTab*)data;
            // If the only tab, exit with error
            if(gtk_notebook_get_n_pages(vtab->vterm->notebook) == 1)
                exit_error();

            // Delete the vtab
            vtab->vterm->deleteVTab((VTab*)data);

            return;
        }else{
            ((VTab*)data)->child_pid = pid;
        }
    }

    void VTab::terminal_title_changed_cb(VteTerminal* vte_terminal, gpointer data){
        VTab* vtab = (VTab*)data;
        const gchar* title = vte_terminal_get_window_title(vte_terminal);
        vtab->sync_tab_label(title);
        vtab->vterm->sync_window_title(title);
    }

    gboolean VTab::terminal_key_press_cb(VteTerminal* vte_terminal, GdkEventKey* event, gpointer data){
        const guint modifiers = event->state & gtk_accelerator_get_default_mod_mask();
        const guint keypressed = gdk_keyval_to_lower(event->keyval);
        VTab* vtab = (VTab*)data;
        VTerm* vterm = vtab->vterm;

        /*
         * This will get really complicated overtime.. so:
         *  - NO FALL THROUGH!! direct fall through when there is no code between
         *  two cases are fine.
         *
         *  - If case is entered, it should RETURN from the function. It should
         *  not depend on any logic after its parent switch case.
         *
         *  - All cases should be surrounded by {}
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
                    vtab->current_mode->switch_mode(VMode::ModeOp::NORMAL_MODE);
                    return true;
                }

                /*
                 * Switch to visual mode
                 */
                case VKEY_SWITCH_VISUAL_LEFT:
                case VKEY_SWITCH_VISUAL_RIGHT:{
                    vtab->current_mode->switch_mode(VMode::ModeOp::VISUAL_MODE);
                    // Pass to vmode to move cursor left/right
                    vtab->current_mode->handle_keyboard_events(event);
                    return true;
                }

                /*
                 * Backward search
                 */
                case GDK_KEY_question:{
                    // Show the widget
                    gtk_widget_show(GTK_WIDGET(vtab->current_mode->search_entry));

                    // Give focus to search widget
                    gtk_widget_grab_focus(GTK_WIDGET(vtab->current_mode->search_entry));

                    // Start backward search
                    VMode::search_entry_prev_cb(vtab->current_mode->search_entry, vtab->current_mode);
                    return true;
                }

                /*
                 * Backward search find prev
                 */
                case GDK_KEY_n:{
                    VMode::search_entry_prev_cb(vtab->current_mode->search_entry, vtab->current_mode);
                    vtab->current_mode->do_search();
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
                    vte_terminal_prompt_prev(vte_terminal);
                    return true;
                }

                /*
                 * Prompt scroll down
                 */
                case VKEY_DOWN_PROMPT:{
                    vte_terminal_prompt_next(vte_terminal);
                    return true;
                }

                /*
                 * Move tab to right
                 * PageUp seems standard for this too
                 */
                case GDK_KEY_Page_Down:
                case VKEY_MOVE_TAB_RIGHT:{
                    gint max_pn = gtk_notebook_get_n_pages(vterm->notebook);
                    gtk_notebook_reorder_child(vterm->notebook, GTK_WIDGET(vtab->hbox),
                            (gtk_notebook_page_num(vterm->notebook, GTK_WIDGET(vtab->hbox)) + 1) % max_pn);
                    return true;
                }

                /*
                 * Move tab to left
                 * PageDown seems standard for this too
                 */
                case GDK_KEY_Page_Up:
                case VKEY_MOVE_TAB_LEFT:{
                    gtk_notebook_reorder_child(vterm->notebook, GTK_WIDGET(vtab->hbox),
                            gtk_notebook_page_num(vterm->notebook, GTK_WIDGET(vtab->hbox)) - 1);
                    return true;
                }

                /*
                 * Copy
                 */
                case VKEY_COPY:{
                    vte_terminal_copy_clipboard_format(vte_terminal, VTE_FORMAT_TEXT);
                    return true;
                }

                /*
                 * Paste
                 */
                case VKEY_PASTE:{
                    vte_terminal_paste_clipboard(vte_terminal);
                    DEBUG_PRINT("Pasted\n");
                    return true;
                }

                /*
                 * Standard zoom-in keybinding
                 */
                case GDK_KEY_plus:{
                    vte_terminal_set_font_scale(vte_terminal,
                            vte_terminal_get_font_scale(vte_terminal) * 1.2);
                    vterm->window_set_size();
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
                 * Standard zoom-in keybinding
                 */
                case GDK_KEY_KP_Add:{
                    vte_terminal_set_font_scale(vte_terminal,
                            vte_terminal_get_font_scale(vte_terminal) * 1.2);
                    vterm->window_set_size();
                    return true;
                }

                /*
                 * Standard zoom-out keybinding
                 */
                case GDK_KEY_minus:
                case GDK_KEY_KP_Subtract:{
                    vte_terminal_set_font_scale(vte_terminal,
                            vte_terminal_get_font_scale(vte_terminal) / 1.2);
                    vterm->window_set_size();
                    return true;
                }

                /*
                 * Standard? reset font scale
                 */
                case GDK_KEY_equal:{
                    vte_terminal_set_font_scale(vte_terminal, VConf(font_scale));
                    vterm->window_set_size();
                    return true;
                }
            }
        } // endif (modifiers == GDK_CONTROL_MASK)

        // See if the vmode wants it
        if(vtab->current_mode->handle_keyboard_events(event)){
            // vmode handled it
            return true;
        }

        // Nobody wants it!
        return false;
    }

    gboolean VTab::terminal_button_press_cb(VteTerminal* _terminal, GdkEventButton* event, gpointer data){
        VTab* vtab = (VTab*)data;
        char* match = nullptr;
        int tag;
        if(!(match = vte_terminal_match_check_event(vtab->vte_terminal, (GdkEvent*) event, &tag))){
            // the event is not interesting
            return false;
        }

        auto desc = vtab->regexTagMap.find(tag);
        if(desc == vtab->regexTagMap.end()){
            g_free(match);
            return false;
        }

        char* cmd[3] = {nullptr, match, nullptr};
        switch(desc->second){
            case Regex_Desc::EMAIL:{
                // Open mailto
                cmd[0] = VConf(mail);
                break;
            }
            case Regex_Desc::URL:{
                // Open url
                cmd[0] = VConf(browser);
                break;
            }
        }

        launch_app(cmd);

        g_free(match);
        return true;
    }

    VTab* VTab::create_tab(VTerm* vterm, gboolean is_first_tab){
        gchar *cwd = VConf(cli_cwd), **cmd = VConf(cli_cmd), **env = nullptr;

        // export WINDOWID env var..
        // termite's code
#ifdef GDK_WINDOWING_X11
        env = g_get_environ();
        if (GDK_IS_X11_SCREEN(gtk_widget_get_screen(GTK_WIDGET(vterm->window)))) {
            GdkWindow *gdk_window = gtk_widget_get_window(GTK_WIDGET(vterm->window));
            if (gdk_window) {
                char xid_s[std::numeric_limits<long unsigned>::digits10 + 1];
                snprintf(xid_s, sizeof(xid_s), "%lu", GDK_WINDOW_XID(gdk_window));
                env = g_environ_setenv(env, "WINDOWID", xid_s, true);
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

            if(VConf(tab_cmd) == VConfig::NewTabCMD::DEFAULT_CMD)
                // use default shell, note that it is initialized to cli_cmd
                // so CLI_CMD is covered.
                cmd = vterm->user_def_shell;

            if(VConf(tab_cwd) == VConfig::NewTabCWD::CURRENT_TAB_CWD){
                // We need to get the current tab cwd
                // Utilize vte OSC 7
                VTab* current_vtab = vterm->getCurrentVTab();
                const gchar* cwd_uri = vte_terminal_get_current_directory_uri(current_vtab->vte_terminal);

                if(cwd_uri){
                    cwd = g_filename_from_uri(cwd_uri, nullptr, nullptr);
                    VTab* new_tab =  new VTab(vterm, cwd, cmd, env);
                    g_free(cwd);
                    return new_tab;
                }else{
                    // In case we did not find it, fallback to vte default which is
                    // cwd of parent
                    g_printerr("Could not retrieve cwd of current tab.. make sure vte.sh is sourced.\n");
                    cwd = nullptr;
                }
            }else if(VConf(tab_cwd) == VConfig::NewTabCWD::HOME_CWD){
                cwd = vterm->user_home;
            }
            // The case of CLI_CWD is handled by initialization
        }

        return new VTab(vterm, cwd, cmd, env);
    }

    void VTab::add_regex(Regex_Desc regex_desc, const char* pattern){
        // Add it to terminal
        int tag = vte_terminal_match_add_regex(vte_terminal,
            vte_regex_new_for_match(pattern,
                                    (gssize) strlen(pattern),
                                    PCRE2_MULTILINE | PCRE2_NOTEMPTY,
                                    nullptr),
            0);
        vte_terminal_match_set_cursor_name(vte_terminal, tag, "hand");

        // Add it to our map
        regexTagMap[tag] = regex_desc;
    }

    // Copied from gnome-terminal
    gboolean VTab::has_foreground_process(){
        char *command = NULL;
        char *data_buf = NULL;
        char *basename = NULL;
        char *name = NULL;
        VtePty *pty;
        int fd;
#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
        int mib[4];
#else
        char filename[64];
#endif
        char *data;
        gsize i;
        gsize len;
        int fgpid;

        if(child_pid == -1)
            return false;

        pty = vte_terminal_get_pty(vte_terminal);
        if(!pty)
            return false;

        fd = vte_pty_get_fd(pty);
        if(fd == -1)
            return false;

        fgpid = tcgetpgrp(fd);
        if(fgpid == -1 || fgpid == child_pid)
            return false;

#if defined(__FreeBSD__) || defined(__DragonFly__)
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_ARGS;
        mib[3] = fgpid;
        if(sysctl(mib, G_N_ELEMENTS(mib), NULL, &len, NULL, 0) == -1)
            return true;

        data_buf = g_malloc0 (len);
        if(sysctl(mib, G_N_ELEMENTS(mib), data_buf, &len, NULL, 0) == -1)
            return true;
        data = data_buf;
#elif defined(__OpenBSD__)
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC_ARGS;
        mib[2] = fgpid;
        mib[3] = KERN_PROC_ARGV;
        if(sysctl(mib, G_N_ELEMENTS(mib), NULL, &len, NULL, 0) == -1)
            return true;

        data_buf = g_malloc0(len);
        if(sysctl(mib, G_N_ELEMENTS(mib), data_buf, &len, NULL, 0) == -1)
            return true;
        data = ((char**)data_buf)[0];
#else
        g_snprintf (filename, sizeof(filename), "/proc/%d/cmdline", fgpid);
        if (!g_file_get_contents(filename, &data_buf, &len, NULL))
            return true;
        data = data_buf;
#endif

        basename = g_path_get_basename(data);
        if(!basename)
            return true;

        name = g_filename_to_utf8(basename, -1, NULL, NULL, NULL);
        if(!name)
            return true;

        if (len > 0 && data[len - 1] == '\0')
            len--;
        for(i = 0; i < len; i++){
            if (data[i] == '\0')
                data[i] = ' ';
        }

        command = g_filename_to_utf8(data, -1, NULL, NULL, NULL);
        if(!command)
            return true;

        return true;
    }

    void VTab::connect_signals(){
        //TODO:: handle resize-window signal
        g_signal_connect(vte_terminal, "key-press-event", G_CALLBACK(terminal_key_press_cb), this);
        g_signal_connect(vte_terminal, "child-exited", G_CALLBACK(VTab::terminal_child_exit_cb), this);
        g_signal_connect(vte_terminal, "window-title-changed", G_CALLBACK(VTab::terminal_title_changed_cb), this);
        g_signal_connect(vte_terminal, "button-press-event", G_CALLBACK(VTab::terminal_button_press_cb), this);
    }

    void VTab::create_tab_label(){
        tab_label = GTK_LABEL(gtk_label_new("VTerminal"));

        gtk_widget_set_halign(GTK_WIDGET(tab_label), GTK_ALIGN_CENTER);
        gtk_widget_set_valign (GTK_WIDGET(tab_label), GTK_ALIGN_BASELINE);
        gtk_widget_set_margin_start(GTK_WIDGET(tab_label), 0);
        gtk_widget_set_margin_end(GTK_WIDGET(tab_label), 0);
        gtk_widget_set_margin_top(GTK_WIDGET(tab_label), 0);
        gtk_widget_set_margin_bottom(GTK_WIDGET(tab_label), 0);

        gtk_label_set_single_line_mode (tab_label, true);
        gtk_label_set_ellipsize(tab_label, VConf(tab_label_trim_first) ?
                                        PANGO_ELLIPSIZE_START :
                                        PANGO_ELLIPSIZE_END);

        // TODO: Make width configurable
        gtk_label_set_width_chars(tab_label, 20);
    }

    VTab::VTab(VTerm* vterm, gchar* cwd, gchar** cmd, gchar** env): vterm(vterm){
        // Create the box
        hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));

        // Create the overlay
        overlay = GTK_OVERLAY(gtk_overlay_new());

        // Create terminal
        vte_terminal = VTE_TERMINAL(vte_terminal_new());

        // Create the tab label
        create_tab_label();

        // Add the terminal to the overlay as main child
        gtk_container_add(GTK_CONTAINER(overlay), GTK_WIDGET(vte_terminal));

        // Add overlay to the box
        gtk_box_pack_start(hbox, GTK_WIDGET(overlay), true, true, 0);

        // Create scrollbar and add it to the box
        if(VConf(show_scrollbar)){
            scrollbar = GTK_SCROLLBAR(gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL,
                    gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(vte_terminal))));
            gtk_box_pack_start(hbox, GTK_WIDGET(scrollbar), false, false, 0);
        }

        // Apply the config on vte terminal
        VConfig::getVConfig().apply_vte_config(vte_terminal);

        // Create a new VMode to prepare for operation
        current_mode = new VMode(this);

        // Connect the signals
        connect_signals();

        // Add regex
        add_regex(Regex_Desc::URL, REGEX_URL_AS_IS);
        add_regex(Regex_Desc::URL, REGEX_URL_HTTP);
        add_regex(Regex_Desc::EMAIL, REGEX_EMAIL);

        // insert it to the notebook
        vterm->insertVTab(this);

        // Finally, spawn the child
        GSpawnFlags spawn_flags = G_SPAWN_SEARCH_PATH_FROM_ENVP;
        vte_terminal_spawn_async(vte_terminal, VTE_PTY_DEFAULT, cwd, cmd, env,
                spawn_flags, NULL, NULL, NULL, -1, NULL, terminal_create_cb, this);
    }

    VTab::~VTab(){
        // Delete inner current mode
        delete current_mode;
    }
}
