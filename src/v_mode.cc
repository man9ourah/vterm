#include "cairo.h"
#include "gdk/gdkkeysyms.h"
#include "src/v_term.h"
#include "v_tab.h"
#include "vtepcre2.h"

/*
 * Regext functions taken from vte example code
 */
static void
jit_regex(VteRegex* regex)
{
        GError *error = nullptr;
        if (!vte_regex_jit(regex, PCRE2_JIT_COMPLETE, &error) ||
            !vte_regex_jit(regex, PCRE2_JIT_PARTIAL_SOFT, &error)) {
                DEBUG_PRINT("JITing regex failed: %s\n", error->message);
        }
}

static VteRegex*
compile_regex_for_search(char const* pattern)
{
        uint32_t flags = PCRE2_UTF | PCRE2_NO_UTF_CHECK | PCRE2_MULTILINE;
        auto regex = vte_regex_new_for_search(pattern, strlen(pattern), flags, nullptr);
        if (regex != nullptr)
                jit_regex(regex);

        return regex;
}

namespace VTERM{

    gboolean VTab::VMode::cursor_indicator_draw_cb(GtkDrawingArea* _cursor_indicator, cairo_t* cr, gpointer data){
        VMode* vmode = (VMode*)data;
        vte_terminal_vterm_cursor_draw(vmode->parent_vtab->vte_terminal, cr);
        return true;
    }

    void VTab::VMode::cursor_indicator_realize_cb(GtkDrawingArea* cursor_indicator, gpointer _data){
        // Drawing area has its own gdk window, we have to set it manually to be pass
        // through 
        GdkWindow* gdkwindow = gtk_widget_get_window(GTK_WIDGET(cursor_indicator));
        gdk_window_set_pass_through(gdkwindow, true);
    }

    void VTab::VMode::search_entry_next_cb(GtkSearchEntry* _search_entry, gpointer data){
        VMode* vmode = (VMode*)data;

        if(vmode->search_dir == SearchDir::FORWARD_SEARCH)
            return;

        // Change the icon
        gtk_entry_set_icon_from_icon_name(GTK_ENTRY(vmode->search_entry),
                GTK_ENTRY_ICON_PRIMARY, "go-down-symbolic");

        // Change our member var
        vmode->search_dir = SearchDir::FORWARD_SEARCH;
    }

    void VTab::VMode::search_entry_prev_cb(GtkSearchEntry* _search_entry, gpointer data){
        VMode* vmode = (VMode*)data;

        if(vmode->search_dir == SearchDir::BACKWARD_SEARCH)
            return;

        // Change the icon
        gtk_entry_set_icon_from_icon_name(GTK_ENTRY(vmode->search_entry),
                GTK_ENTRY_ICON_PRIMARY, "go-up-symbolic");

        // Change our member var
        vmode->search_dir = SearchDir::BACKWARD_SEARCH;
    }

    void VTab::VMode::search_entry_changed_cb(GtkSearchEntry* _search_entry, gpointer data){
        VMode* vmode = (VMode*)data;

        // Set the regex
        VteRegex *regex = compile_regex_for_search(gtk_entry_get_text(GTK_ENTRY(vmode->search_entry)));
        vte_terminal_search_set_regex(vmode->parent_vtab->vte_terminal, regex, 0);
        if(regex)
            vte_regex_unref(regex);

        // Delete selection to search from beginning
        vte_terminal_vterm_cursor_selection(vmode->parent_vtab->vte_terminal,
                VTermSelectionType::VTERM_SELECTION_NONE);

        // Do search
        vmode->do_search();
    }

    void VTab::VMode::search_entry_stop_cb(GtkSearchEntry* _search_entry, gpointer data){
        VMode* vmode = (VMode*)data;

        // Then hide the search widget
        gtk_widget_hide(GTK_WIDGET(vmode->search_entry));

        // Give focus to terminal
        gtk_widget_grab_focus(GTK_WIDGET(vmode->parent_vtab->vte_terminal));
    }

    void VTab::VMode::do_search(){
        switch(search_dir){
            case BACKWARD_SEARCH:{
                DEBUG_PRINT("BACKWARD_SEARCH\n");
                vte_terminal_search_find_previous(parent_vtab->vte_terminal);
                break;
            }

            case FORWARD_SEARCH:{
                DEBUG_PRINT("FORWARD_SEARCH\n");
                vte_terminal_search_find_next(parent_vtab->vte_terminal);
                break;
            }
        }
    }

    gboolean VTab::VMode::handle_keyboard_events(GdkEventKey* event){
        const guint modifiers = event->state & gtk_accelerator_get_default_mod_mask();
        const guint keypressed = gdk_keyval_to_lower(event->keyval);

        /*
         * Now to the keybindings that are specific to certain modes
         */
        switch(mode){
            /*
             * Keybindings specific to normal mode
             */
            case VMode::ModeOp::VISUAL_MODE:
            case VMode::ModeOp::VISUAL_LINE_MODE:
            case VMode::ModeOp::VISUAL_BLOCK_MODE:
            case VMode::ModeOp::NORMAL_MODE:{
                if(modifiers == GDK_SHIFT_MASK){
                    switch(keypressed){
                        case GDK_KEY_h:
                        case GDK_KEY_H:{
                            DEBUG_PRINT("\nVTermCursor:  top\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::TOP);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_l:
                        case GDK_KEY_L:{
                            DEBUG_PRINT("\nVTermCursor:  bottom\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::BOTTOM);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_m:
                        case GDK_KEY_M:{
                            DEBUG_PRINT("\nVTermCursor:  middle\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::MIDDLE);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_w:
                        case GDK_KEY_W:{
                            DEBUG_PRINT("\nVTermCursor:  right statement\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::RIGHT_STMT);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_b:
                        case GDK_KEY_B:{
                            DEBUG_PRINT("\nVTermCursor:  left statement\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::LEFT_STMT);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_e:
                        case GDK_KEY_E:{
                            DEBUG_PRINT("\nVTermCursor:  end right statement\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::RIGHT_STMT_END);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_dollar:{
                            DEBUG_PRINT("\nVTermCursor:  eol\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::EOL);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_V:
                        case GDK_KEY_v:{
                            switch_mode(VMode::ModeOp::VISUAL_LINE_MODE);
                            return true;
                        }

                        case GDK_KEY_n:
                        case GDK_KEY_N:{
                            search_entry_prev_cb(search_entry, this);
                            do_search();
                            return true;
                        }

                        case GDK_KEY_question:{
                            // Show the widget
                            gtk_widget_show(GTK_WIDGET(search_entry));

                            // Give focus to search widget
                            gtk_widget_grab_focus(GTK_WIDGET(search_entry));

                            // Start forward search
                            search_entry_prev_cb(search_entry, this);
                            return true;
                        }
                    }
                }else if(modifiers == GDK_CONTROL_MASK){
                    switch(keypressed){
                        case GDK_KEY_v:{
                            switch_mode(VMode::ModeOp::VISUAL_BLOCK_MODE);
                            return true;
                        }
                    }
                }else{
                    switch(keypressed){
                        case GDK_KEY_k:
                        case GDK_KEY_Up:{
                            DEBUG_PRINT("\nVTermCursor:  up\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::UP);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_j:
                        case GDK_KEY_Down:{
                            DEBUG_PRINT("\nVTermCursor:  down\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::DOWN);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_h:
                        case GDK_KEY_Left:{
                            DEBUG_PRINT("\nVTermCursor:  left\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::LEFT);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_l:
                        case GDK_KEY_Right:{
                            DEBUG_PRINT("\nVTermCursor:  right\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::RIGHT);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_0:
                        case GDK_KEY_KP_0:
                        case GDK_KEY_Home:{
                            DEBUG_PRINT("\nVTermCursor:  bol\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::BOL);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_dollar:
                        case GDK_KEY_End:{
                            DEBUG_PRINT("\nVTermCursor:  eol\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::EOL);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_w:{
                            DEBUG_PRINT("\nVTermCursor:  right word\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::RIGHT_WORD);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_b:{
                            DEBUG_PRINT("\nVTermCursor:  left word\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::LEFT_WORD);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_e:{
                            DEBUG_PRINT("\nVTermCursor:  end right word\n");
                            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::RIGHT_WORD_END);
                            gtk_widget_queue_draw(GTK_WIDGET(cursor_indicator));
                            return true;
                        }

                        case GDK_KEY_slash:{
                            // Show the widget
                            gtk_widget_show(GTK_WIDGET(search_entry));

                            // Give focus to search widget
                            gtk_widget_grab_focus(GTK_WIDGET(search_entry));

                            // Start forward search
                            search_entry_next_cb(search_entry, this);
                            return true;
                        }

                        case GDK_KEY_question:{
                            // Show the widget
                            gtk_widget_show(GTK_WIDGET(search_entry));

                            // Give focus to search widget
                            gtk_widget_grab_focus(GTK_WIDGET(search_entry));

                            // Start forward search
                            search_entry_prev_cb(search_entry, this);
                            return true;
                        }

                        case GDK_KEY_n:{
                            search_entry_next_cb(search_entry, this);
                            do_search();
                            return true;
                        }

                        case GDK_KEY_v:{
                            switch_mode(VMode::ModeOp::VISUAL_MODE);
                            return true;
                        }
                    }
                }

                return false;
            }

            /*
             * Keybindings specific to insert mode
             */
            case VMode::ModeOp::INSERT_MODE:{
                // None
                // Pass event
                return false;
            }
        }
        return false;
    }

    void VTab::VMode::show_vmode(){
        switch(mode){
            case VMode::ModeOp::VISUAL_MODE:
            case VMode::ModeOp::VISUAL_LINE_MODE:
            case VMode::ModeOp::VISUAL_BLOCK_MODE:
            case VMode::ModeOp::NORMAL_MODE:{
                gtk_widget_show(GTK_WIDGET(cursor_indicator));
                break;
            }

            case VMode::ModeOp::INSERT_MODE:{
                gtk_widget_hide(GTK_WIDGET(cursor_indicator));
                break;
            }
        }
    }

    void VTab::VMode::enter_normal_mode(){
        // Stop sending input to the terminal
        vte_terminal_set_input_enabled(parent_vtab->vte_terminal, false);

        // Dont scroll on keystroke or output
        vte_terminal_set_scroll_on_keystroke(parent_vtab->vte_terminal, false);
        vte_terminal_set_scroll_on_output(parent_vtab->vte_terminal, false);

        // Initialize the cursor
        vte_terminal_vterm_cursor_init(parent_vtab->vte_terminal, GTK_WIDGET(cursor_indicator));

        // Signal that we are showing vterm's cursor so that the other cursor is
        // hidden
        vte_terminal_vterm_cursor_set_shown(parent_vtab->vte_terminal, true);

        // If coming from insert mode, set the normal mode position same as the
        // insert's mode
        if(mode == VMode::ModeOp::INSERT_MODE)
            vte_terminal_vterm_cursor_move(parent_vtab->vte_terminal, VTermCursorMove::INPUT);

        vte_terminal_vterm_cursor_selection(parent_vtab->vte_terminal, VTermSelectionType::VTERM_SELECTION_NONE);

        // Set our mode to normal
        mode = VMode::ModeOp::NORMAL_MODE;

        // Show normal mode cursor
        show_vmode();
    }

    void VTab::VMode::enter_insert_mode(){
        // Reverse: Stop sending input to the terminal
        vte_terminal_set_input_enabled(parent_vtab->vte_terminal, true);

        // Reverse: Dont scroll on keystroke or output
        vte_terminal_set_scroll_on_keystroke(parent_vtab->vte_terminal, VConf(scroll_on_output));
        vte_terminal_set_scroll_on_output(parent_vtab->vte_terminal, VConf(scroll_on_keystroke));
        
        // Reverse: Signal that we hiding vterm's cursor so that the other cursor is
        // shown
        vte_terminal_vterm_cursor_set_shown(parent_vtab->vte_terminal, false);

        vte_terminal_vterm_cursor_selection(parent_vtab->vte_terminal, VTermSelectionType::VTERM_SELECTION_NONE);

        // Set our mode to insert
        mode = VMode::ModeOp::INSERT_MODE;

        // Hide normal mode cursor
        show_vmode();
    }

    void VTab::VMode::enter_visual_mode(ModeOp visual_mode_kind){
        // We have to be already in normal mode!
        g_assert(mode == VMode::ModeOp::NORMAL_MODE);

        VTermSelectionType selection_type = VTERM_CHAR_SELECTION;
        if(visual_mode_kind == VISUAL_LINE_MODE)
            selection_type = VTERM_LINE_SELECTION;
        else if(visual_mode_kind == VISUAL_BLOCK_MODE)
            selection_type = VTERM_BLOCK_SELECTION;

        vte_terminal_vterm_cursor_selection(parent_vtab->vte_terminal, selection_type);

        // Set our mode to visual
        mode = visual_mode_kind;
    }

    void VTab::VMode::switch_mode(VMode::ModeOp new_mode){
        // First, what mode are we in?
        switch(mode){
            case VMode::ModeOp::NORMAL_MODE:{

                // We are in normal mode
                // What mode we want to switch to?
                switch(new_mode){
                    /*
                     * normal->visual
                     */
                    case VMode::ModeOp::VISUAL_MODE:
                    case VMode::ModeOp::VISUAL_LINE_MODE:
                    case VMode::ModeOp::VISUAL_BLOCK_MODE:{
                        DEBUG_PRINT("\nMODE: NORMAL->VISAUL\n");
                        enter_visual_mode(new_mode);
                        break;
                    }

                    /*
                     * normal->normal TOGGLE(insert)
                     * normal->insert
                     */
                    case VMode::ModeOp::NORMAL_MODE:
                    case VMode::ModeOp::INSERT_MODE:{
                        DEBUG_PRINT("\nMODE: NORMAL->INSERT\n");
                        enter_insert_mode();
                        break;
                    }
                }

                break;
            }

            case VMode::ModeOp::VISUAL_MODE:
            case VMode::ModeOp::VISUAL_LINE_MODE:
            case VMode::ModeOp::VISUAL_BLOCK_MODE:{
                // We are in normal mode
                // What mode we want to switch to?
                switch(new_mode){
                    /*
                     * visual->visual toggle(normal)
                     * visual->normal
                     */
                    case VMode::ModeOp::VISUAL_MODE:
                    case VMode::ModeOp::VISUAL_LINE_MODE:
                    case VMode::ModeOp::VISUAL_BLOCK_MODE:
                    case VMode::ModeOp::NORMAL_MODE:{
                        DEBUG_PRINT("\nmode: VISUAL->NORMAL\n");
                        enter_normal_mode();
                        break;
                    }

                    /*
                     * visual->insert
                     */
                    case VMode::ModeOp::INSERT_MODE:{
                        DEBUG_PRINT("\nMODE: VISUAL->INSERT\n");
                        enter_insert_mode();
                        break;
                    }
                }

                break;
            }

            case VMode::ModeOp::INSERT_MODE:{

                // We are in insert mode
                // What mode we want to switch to?
                switch(new_mode){
                    /*
                     * insert->visual
                     */
                    case VMode::ModeOp::VISUAL_MODE:
                    case VMode::ModeOp::VISUAL_LINE_MODE:
                    case VMode::ModeOp::VISUAL_BLOCK_MODE:{
                        DEBUG_PRINT("\nMODE: INSERT->VISAUL\n");
                        enter_normal_mode();
                        enter_visual_mode(new_mode);
                        break;
                    }

                    /*
                     * insert->normal
                     */
                    case VMode::ModeOp::NORMAL_MODE:{
                        DEBUG_PRINT("\nMODE: INSERT->NORMAL\n");
                        enter_normal_mode();
                        break;
                    }

                    /*
                     * insert->insert
                     * this should never happen
                     */
                    case VMode::ModeOp::INSERT_MODE:{
                        DEBUG_PRINT("\nMODE: INSERT->INSERT\n");
                        break;
                    }
                }

                break;
            }
        }
    }

    VTab::VMode::VMode(VTab* parent_vtab) : parent_vtab(parent_vtab){
        // Initial mode is insert mode
        mode = ModeOp::INSERT_MODE;

        /*
         * Cursor indicator
         */
        cursor_indicator = GTK_DRAWING_AREA(gtk_drawing_area_new());

        // Add it to the parent overlay
        gtk_overlay_add_overlay(parent_vtab->overlay, GTK_WIDGET(cursor_indicator));
        gtk_overlay_set_overlay_pass_through(parent_vtab->overlay, GTK_WIDGET(cursor_indicator), true);

        // Dont show the widget with show_all
        gtk_widget_set_no_show_all(GTK_WIDGET(cursor_indicator), true);

        // Connect the draw signal
        g_signal_connect(cursor_indicator, "draw", G_CALLBACK(cursor_indicator_draw_cb), this);
        g_signal_connect(cursor_indicator, "realize", G_CALLBACK(cursor_indicator_realize_cb), this);

        /*
         * Search entry
         */
        search_entry = GTK_SEARCH_ENTRY(gtk_search_entry_new());

        // Add it to overlay
        gtk_overlay_add_overlay(parent_vtab->overlay, GTK_WIDGET(search_entry));

        // Dont show the widget with show_all
        gtk_widget_set_no_show_all(GTK_WIDGET(search_entry), true);

        // Position it
        // TODO:: Make this position configurable?
        gtk_widget_set_halign(GTK_WIDGET(search_entry), GTK_ALIGN_END);
        gtk_widget_set_valign(GTK_WIDGET(search_entry), GTK_ALIGN_START);

        // Connect the search signal
        g_signal_connect(search_entry, "next-match", G_CALLBACK(search_entry_next_cb), this);
        g_signal_connect(search_entry, "previous-match", G_CALLBACK(search_entry_prev_cb), this);
        g_signal_connect(search_entry, "search-changed", G_CALLBACK(search_entry_changed_cb), this);
        g_signal_connect(search_entry, "stop-search", G_CALLBACK(search_entry_stop_cb), this);
        g_signal_connect(search_entry, "activate", G_CALLBACK(search_entry_stop_cb), this);
    }
}
