#include "v_term.h"
#include "v_config.h"
#include "v_tab.h"
#include <memory>
#include "common.h"

using namespace std;
namespace VTERM{

    unique_ptr<VTerm> vterm;

    VTerm::VTerm(){
        // Create top level window
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

        // Create notebook
        notebook = GTK_NOTEBOOK(gtk_notebook_new());

        // Do we need to set visual?
        if(VConfig::getVConfig().is_transparency())
            window_screen_changed_cb(window, nullptr, nullptr);

        // Apply the config on both
        VConfig::getVConfig().apply_window_config(GTK_WINDOW(window));
        VConfig::getVConfig().apply_notebook_config(GTK_NOTEBOOK(notebook));

        // Connect to thier events
        connect_signals();

        // Finally, add notebook to window
        gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(notebook));
    }

    VTerm::~VTerm(){
        g_strfreev(user_def_shell);
        g_free(user_home);
    }

    gboolean VTerm::window_focus_changed_cb(GtkWidget* _window, GdkEvent *event, gpointer _data){
        GdkEventFocus* fe = (GdkEventFocus*)event;
        if(VTab* current_tab = vterm->getCurrentVTab()){
            VteTerminal* current_vte_terminal = VTE_TERMINAL(current_tab->vte_terminal);
            if(fe->in)
                vte_terminal_set_color_background(current_vte_terminal, &VConf(color_background)); 
            else
                vte_terminal_set_color_background(current_vte_terminal, &VConf(focus_out_color_background));
        }
        return false;
    }

    void VTerm::window_screen_changed_cb(GtkWidget* window, GdkScreen* _prev_screen, gpointer _data){
        GdkScreen* screen = gtk_widget_get_screen(GTK_WIDGET(window));
        GdkVisual* visual = gdk_screen_get_rgba_visual(screen);
        if(visual != nullptr)
            gtk_widget_set_visual(GTK_WIDGET(window), visual);
    }

    void VTerm::notebook_switch_page_cb(GtkNotebook* _notebook, GtkWidget* hbox, 
            guint _page_nu, gpointer _data){
        VTab* vtab = vterm->getVTab(hbox);

        // Sync the window title 
        const gchar* title = gtk_label_get_text(GTK_LABEL(vtab->tab_label));
        vterm->sync_window_title(title);

        // Sets the show tab policy for NEEDED
        if(VConf(show_tab_policy) == NEEDED){
                gtk_notebook_set_show_tabs(vterm->notebook, 
                                           (gtk_notebook_get_n_pages(vterm->notebook) == 1)?
                                            false : true);
        }

        // Update the geometry hints; new tab could have different font and we
        // might have added/deleted the tabs bar
        window_set_size();
    }

    gboolean VTerm::window_key_press_cb(GtkWidget* _window, GdkEventKey* event, gpointer _data){
        const guint modifiers = event->state & gtk_accelerator_get_default_mod_mask();

        if(modifiers == (GDK_CONTROL_MASK|GDK_SHIFT_MASK)){
           switch(gdk_keyval_to_lower(event->keyval)){
                case GDK_KEY_t:
                    VTab::create_tab(false);
                    break;
           }
        }
        return false;
    }

    void VTerm::window_update_geometry(VTab* vtab){
        DEBUG_PRINT("\nUpdating window geometry..\n");
        /*
         * Ideally, we want a window size that is a multiple of char width and
         * height.. this will give us a terminal window that fits well around the
         * terminal without any extra spaces on any side. Unfortunately, this is
         * not always achievable; if the screen in fullscreen or in tiled WM,
         * and the allocated size is not a multiple of char width/height; we
         * cant really do anything about it.
         */ 
        VteTerminal* vte_terminal = VTE_TERMINAL(vtab->vte_terminal);
        GtkWidget* notebook = GTK_WIDGET(vterm->notebook);

        // So, first.. get the char width/height
        gint cell_width = vte_terminal_get_char_width(vte_terminal);
        gint cell_height = vte_terminal_get_char_height(vte_terminal);
        DEBUG_PRINT("cell_width: %d, cell_height: %d\n", cell_width, cell_height);

        gint col_count = vte_terminal_get_column_count(vte_terminal);
        gint row_count = vte_terminal_get_row_count(vte_terminal);
        DEBUG_PRINT("col_count: %d, row_count: %d\n", col_count, row_count);

        // Now size of chrome
        GtkRequisition notebook_request;
        gtk_widget_get_preferred_size(notebook, NULL, &notebook_request);

        gint chrome_width = notebook_request.width - (cell_width * col_count);
        gint chrome_height = notebook_request.height - (cell_height * row_count);
        DEBUG_PRINT("chrome_width: %d, chrome_height: %d\n", chrome_width, chrome_height);

        gint csd_width = 0;
        gint csd_height = 0;

        if(gtk_widget_get_realized(vterm->window) && VConf(window_size_hints)){
            // Now size of csd 
            GtkAllocation toplevel, contents;
            gtk_widget_get_allocation(vterm->window, &toplevel);
            gtk_widget_get_allocation(notebook, &contents);

            csd_width = toplevel.width - contents.width;
            csd_height = toplevel.height - contents.height;
            DEBUG_PRINT("csd_width: %d, csd_height: %d\n", csd_width, csd_height);

            // Now we set the size hints
            GdkGeometry geometry;

            geometry.base_width = csd_width + chrome_width;
            geometry.base_height = csd_height + chrome_height;
            geometry.width_inc = cell_width;
            geometry.height_inc = cell_height;
            geometry.min_width = geometry.base_width + cell_width * 5;
            geometry.min_height = geometry.base_height + cell_height * 2;

            gtk_window_set_geometry_hints(GTK_WINDOW(vterm->window),
                                          nullptr,
                                          &geometry,
                                          GdkWindowHints(GDK_HINT_RESIZE_INC |
                                                         GDK_HINT_MIN_SIZE |
                                                         GDK_HINT_BASE_SIZE));
        }

        vterm->window_width_cache = chrome_width + cell_width * col_count;
        vterm->window_height_cache = chrome_height + cell_height * row_count;
        DEBUG_PRINT("window_width_cache: %d, window_height_cache: %d\n", 
                    vterm->window_width_cache, vterm->window_height_cache);
    }

    void VTerm::window_set_size(){
        VTab* vtab = vterm->getCurrentVTab();

        // If we dont have any tab, no need yet..
        if(!vtab)
            return;

        // Are we up-to-date?
        window_update_geometry(vtab);

        // Not doing this to maximized or tiled windows!
        GdkWindow* gdk_window = gtk_widget_get_window (GTK_WIDGET (vterm->window));
        if (gdk_window != NULL && (gdk_window_get_state (gdk_window) &
            (GDK_WINDOW_STATE_MAXIMIZED | GDK_WINDOW_STATE_TILED | GDK_WINDOW_STATE_FULLSCREEN)))
            return;

        // If we have been realized, and csd size were figured out, resize..
        if(vterm->window_width_cache > 0 && vterm->window_height_cache > 0){ 

            DEBUG_PRINT("Resizing window: (%dX%d)\n", 
                        vterm->window_width_cache, vterm->window_height_cache);
            gtk_window_resize(GTK_WINDOW(vterm->window), 
                              vterm->window_width_cache, vterm->window_height_cache);
        }
    }

    void VTerm::connect_signals(){
        g_signal_connect(window, "key-press-event", G_CALLBACK(window_key_press_cb), nullptr);
        g_signal_connect(window, "realize", G_CALLBACK(window_set_size), nullptr);

        if(VConf(focus_aware_color_background)){
            g_signal_connect(window, "focus-in-event", G_CALLBACK(window_focus_changed_cb), nullptr);
            g_signal_connect(window, "focus-out-event", G_CALLBACK(window_focus_changed_cb), nullptr);
        }

        g_signal_connect(notebook, "switch-page", G_CALLBACK(notebook_switch_page_cb), nullptr);

        if(VConfig::getVConfig().is_transparency())
            g_signal_connect(window, "screen-changed", 
                    G_CALLBACK(window_screen_changed_cb), nullptr);

        g_signal_connect(window, "destroy", exit_success, nullptr);
    }

    void VTerm::run(){
        gtk_widget_show_all(window);

        VTab::create_tab(true);

        // fire!
        gtk_main();
    }

}

/*
 * Parse args, initialize configuration, create global vterm, 
 * pass execution to vterm.
 */ 
gint main(gint argc, gchar **argv) {
    DEBUG_PRINT("This is a debug build!\n");

    GError* gerror = nullptr;
    GOptionContext *context = g_option_context_new (nullptr);
    
    gboolean version = false;

    gchar *exec = nullptr, *cli_cwd = nullptr, 
         *cli_config_path = nullptr; 

    const GOptionEntry main_entries[] = {
        {"version", 'v', 0, G_OPTION_ARG_NONE, &version, "Print version and exit", "VERSION"},
        {"execute", 'e', 0, G_OPTION_ARG_STRING, &exec, "Execute given the command", "COMMAND"},
        {"directory", 'd', 0, G_OPTION_ARG_STRING, &cli_cwd, "Starting directory", "DIRECTORY"},
        {"config-file", 'c', 0, G_OPTION_ARG_STRING, &cli_config_path, "Load this config file", "CONFIG"},
        {nullptr}
    };
    g_option_context_add_main_entries(context, main_entries, nullptr);
    g_option_context_add_group (context, gtk_get_option_group (TRUE));
    g_option_context_set_summary(context, VTERM_ASCIIART);
    g_option_context_set_description(context, VTERM_CREDITS);

    if (!g_option_context_parse(context, &argc, &argv, &gerror)) {
        g_printerr("Options parsing failed: %s\n", gerror->message);
        g_printerr("%s\n", g_option_context_get_help(context, true, nullptr));
        g_clear_error (&gerror);
        return EXIT_FAILURE;
    }
    g_option_context_free(context);

    if(version){
        g_print("V-Term %s\n", VTERM_VERSION);
        return EXIT_SUCCESS;
    }

    gchar **cli_cmd = nullptr;
    if (exec) {
        gint argcp;
        gchar **argvp;
        g_shell_parse_argv(exec, &argcp, &argvp, &gerror);
        if (gerror) {
            g_printerr("Failed to parse command: %s\n", gerror->message);
            return EXIT_FAILURE;
        }
        cli_cmd = argvp;
    } 

    // Initialize config
    VTERM::VConfig::initVConfig(cli_config_path, cli_cwd, cli_cmd);

    // create global VTerm & run
    VTERM::vterm.reset(new VTERM::VTerm());
    VTERM::vterm->run();

    // If we reach here, something is wrong
    VTERM::exit_error();
}

// vim: et:sts=4:sw=4:cino=(0:cc=100
