#ifndef V_CONFIG_H
#define V_CONFIG_H
#include "pango/pango-font.h"
#include "vte/vte.h"
#include "common.h"
#include <array>

using namespace std;
/*
 * Shorter name to get config values
 */ 
#define VConf(key) VTERM::VConfig::getVConfig().key

/*
 * Macro constructs to parse a string config, compare 
 * it to values and assign accordingly.
 */ 
#define _PARSE_STRING_START(sec, key) \
                gchar* temp_##key = g_key_file_get_string(config, #sec, #key, &gerror); \
                if(!is_gerror(#key)){ \
                    temp_##key = g_strstrip(temp_##key);

#define _PARSE_STRING_COMPARE(key, value, assignment) \
                    if(g_ascii_strcasecmp(temp_##key, value) == 0) \
                        key = assignment

#define _PARSE_STRING_ANOTHER_COMPARE2(key, value, assignment) \
                    else _PARSE_STRING_COMPARE(key, value, assignment);

#define _PARSE_STRING_ANOTHER_COMPARE4(key, value1, assignment1, value2, assignment2) \
                    _PARSE_STRING_ANOTHER_COMPARE2(key, value1, assignment1) \
                    _PARSE_STRING_ANOTHER_COMPARE2(key, value2, assignment2)

#define _PARSE_STRING_ANOTHER_COMPARE6(key, value1, assignment1, value2, assignment2, \
                                        value3, assignment3) \
                    _PARSE_STRING_ANOTHER_COMPARE4(key, value1, assignment1, value2, assignment2) \
                    _PARSE_STRING_ANOTHER_COMPARE2(key, value3, assignment3)

#define _PARSE_STRING_ANOTHER_COMPARE8(key, value1, assignment1, value2, assignment2, \
                                        value3, assignment3, value4, assignment4) \
                    _PARSE_STRING_ANOTHER_COMPARE4(key, value1, assignment1, value2, assignment2) \
                    _PARSE_STRING_ANOTHER_COMPARE4(key, value3, assignment3, value4, assignment4)

#define _PARSE_STRING_END(key)\
                    else \
                        g_printerr("Error loading config \"%s\"\n", #key); \
                    g_free(temp_##key); \
                }

#define _GET_OVERRIDE(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME

#define DUMMY _

#define _ELSE_COMPARE(key, ...) _GET_OVERRIDE(__VA_ARGS__,\
        _PARSE_STRING_ANOTHER_COMPARE8, DUMMY, \
        _PARSE_STRING_ANOTHER_COMPARE6, DUMMY, \
        _PARSE_STRING_ANOTHER_COMPARE4, DUMMY, \
        _PARSE_STRING_ANOTHER_COMPARE2)\
        (key, __VA_ARGS__)

/*
 * Use this macro to get string config from section `sec` and key `key`, then
 * set the config variable to the assignment based on the parsed string value
 * See examples in `configure_vterm()`
 */ 
#define PARSE_STRING(sec, key, value, assignment, ...) \
                _PARSE_STRING_START(sec, key) \
                _PARSE_STRING_COMPARE(key, value, assignment); \
                _ELSE_COMPARE(key, __VA_ARGS__) \
                _PARSE_STRING_END(key)

/*
 * Gets a string but free the one before it if it was not nullptr
 */ 
#define GET_STRING_AND_FREE(sec, key)   gchar* temp_##key = g_key_file_get_string(config, #sec, #key, &gerror); \
                                        if(!is_gerror(#key)){ \
                                            if(key) \
                                                g_free(key); \
                                            key = g_strstrip(temp_##key); \
                                        }

/*
 * Gets a number, tys is the short type name (int), tyf is the long one (integer)
 */ 
#define GET_NUMBER(sec, key, tys, tyf)  g##tys temp_##key = g_key_file_get_##tyf(config, #sec, #key, &gerror); \
                                    if(!is_gerror(#key)){ \
                                        key = temp_##key; \
                                    }

namespace VTERM{

    /*
     * Possible options for new tab cwd
     */ 
    enum NewTabCWD{
        // Use cwd of the current tab.
        CURRENT_TAB_CWD,

        // Use cwd passed in cli 
        CLI_CWD,

        // Use user's home directory 
        HOME_CWD
    };

    /*
     * Possible options for new tab cmd
     */ 
    enum NewTabCMD{
        // Use default user shell
        DEFAULT_CMD,

        // Use the command pass in cli for all tabs
        CLI_CMD
    };

    /*
     * Possible options for tab show 
     */ 
    enum ShowTabPolicy{
        // Always show tabs
        ALWAYS,

        // Show tabs only if > 1
        SMART,

        // Show at key press
        AUTO,
    };

    /*
     * VTerminal configuration
     * Steps to add new config option:
     *     1. Add class member with proper type.
     *     2. Add code to set the config option in configure_vterm()
     *     3. Add code to apply the config option in apply_config()
     *     4. If option is specific to vte version > 52, add guards.
     */
    class VConfig{
        public:
            static VConfig& initVConfig(gchar* cli_config_path = nullptr, 
                    gchar *cli_cwd = nullptr, gchar **cli_cmd = nullptr){
                static VConfig vconfig (cli_config_path, cli_cwd, cli_cmd);
                return vconfig;
            }

            static VConfig& getVConfig(){
                return initVConfig();
            }

        public:
            /*
             * VTerm Configuration
             */ 
            gchar* cli_config_path;
            NewTabCWD tab_cwd = CURRENT_TAB_CWD;
            NewTabCMD tab_cmd = DEFAULT_CMD;

            gchar *cli_cwd = nullptr, **cli_cmd = nullptr;

            /*
             * VTE configuration
             */ 
            gboolean audible_bell = false,
                     bold_is_bright = true,
                     allow_hyperlink = true,
                     scroll_on_output = true,
                     scroll_on_keystroke = true,
                     mouse_autohide = true,
                     enable_bidi = true,
                     enable_shaping = true,
                     focus_aware_color_background = false;

            gdouble font_scale = 1,
                    cell_height_scale = 1,
                    cell_width_scale = 1,
                    color_background_transparency = 0,
                    focus_out_color_background_transparency = 0; 
            
            gchar* word_char_exceptions = nullptr;

            gint scrollback_lines = 10000;

            PangoFontDescription* font = pango_font_description_from_string("Monospace Normal 12");

            GdkRGBA color_bold,
                    color_foreground,
                    color_background,
                    focus_out_color_background,
                    color_cursor,
                    color_cursor_foreground,
                    color_highlight,
                    color_highlight_foreground;

            array<GdkRGBA, 256> palette;

            VteCursorShape cursor_shape = VTE_CURSOR_SHAPE_BLOCK;
            VteCursorBlinkMode cursor_blink_mode = VTE_CURSOR_BLINK_ON;

            VteEraseBinding backspace_binding = VTE_ERASE_AUTO;
            VteEraseBinding delete_binding = VTE_ERASE_AUTO;

            /*
             * Window configuration
             */
            gchar   *window_title = nullptr, // null: mirror current tab title
                    *window_role = nullptr;  // null: wont be set

            gboolean window_size_hints = true;

            /*
             * Notebook configuration
             */ 
            GtkPositionType tabs_position = GTK_POS_TOP;

            //TODO: autohide & smart hide
            ShowTabPolicy show_tab_policy = ALWAYS;

            gboolean tab_label_trim_first = true,
                     insert_after_current = true,
                     show_scrollbar = true;

        private:
            VConfig(gchar* cli_config_path, gchar *cli_cwd, gchar **cli_cmd) : 
                cli_config_path(cli_config_path), cli_cwd(cli_cwd), cli_cmd(cli_cmd){
                load_config_file();
            }

        public:
            ~VConfig(){
                if(cli_config_path)
                    g_free(cli_config_path);
                if(cli_cwd)
                    g_free(cli_cwd);
                if(cli_cmd)
                    g_strfreev(cli_cmd);
                if(word_char_exceptions)
                    g_free(word_char_exceptions);
                if(font)
                    pango_font_description_free(font);
            }

        private:
            // Singleton
            VConfig(VConfig const&);
            void operator=(VConfig const&);

        public:
            /*
             * Applies the configuration on window
             */ 
            void apply_window_config(GtkWindow* window){
                if(window_title)
                    gtk_window_set_title(window, window_title);
                if(window_role)
                    gtk_window_set_role(window, window_role);
                if(color_background_transparency > 0){
                    gtk_widget_set_app_paintable(GTK_WIDGET(window), true);
                    color_background.alpha = double(100 - CLAMP(color_background_transparency, 0, 100)) / 100.0;
                }
                if(focus_out_color_background_transparency > 0){
                    gtk_widget_set_app_paintable(GTK_WIDGET(window), true);
                    focus_out_color_background.alpha = double(100 - CLAMP(
                                focus_out_color_background_transparency, 0, 100)) / 100.0;
                }
            }

            /*
             * Applies the configuration on notebook
             */ 
            void apply_notebook_config(GtkNotebook* notebook){
                gtk_notebook_set_tab_pos(notebook, tabs_position);
                gtk_notebook_set_scrollable(notebook, true);
                gtk_notebook_set_show_border(notebook, false);
                gtk_widget_set_can_focus(GTK_WIDGET(notebook), false);
            }

            /*
             * Applies the configuration on vte_terminal
             */ 
            void apply_vte_config(VteTerminal* vte_terminal){
                vte_terminal_set_audible_bell(vte_terminal, audible_bell);
                vte_terminal_set_allow_hyperlink(vte_terminal, allow_hyperlink);
                vte_terminal_set_scroll_on_output(vte_terminal, scroll_on_output); 
                vte_terminal_set_scroll_on_keystroke(vte_terminal, scroll_on_keystroke);
                vte_terminal_set_mouse_autohide(vte_terminal, mouse_autohide);

#if VTE_CHECK_VERSION(0, 58, 0)
                vte_terminal_set_enable_bidi(vte_terminal, enable_bidi);
                vte_terminal_set_enable_shaping(vte_terminal, enable_shaping);
#endif

                vte_terminal_set_font_scale(vte_terminal, font_scale);
                vte_terminal_set_bold_is_bright(vte_terminal, false);
                vte_terminal_set_cell_width_scale(vte_terminal, cell_width_scale);
                vte_terminal_set_cell_height_scale(vte_terminal, cell_height_scale);
                vte_terminal_set_word_char_exceptions(vte_terminal, word_char_exceptions);
                vte_terminal_set_scrollback_lines(vte_terminal, scrollback_lines);
                vte_terminal_set_font(vte_terminal, font);
                vte_terminal_set_color_bold(vte_terminal, &color_bold); 
                vte_terminal_set_color_foreground(vte_terminal, &color_foreground);
                vte_terminal_set_color_background(vte_terminal, &color_background); 
                vte_terminal_set_color_cursor(vte_terminal, &color_cursor);
                vte_terminal_set_color_cursor_foreground(vte_terminal, &color_cursor_foreground); 
                vte_terminal_set_color_highlight(vte_terminal, &color_highlight); 
                vte_terminal_set_color_highlight_foreground(vte_terminal, &color_highlight_foreground);
                vte_terminal_set_colors(vte_terminal, &color_foreground, 
                        &color_background, palette.data(), palette.size());
                vte_terminal_set_cursor_shape(vte_terminal, cursor_shape);
                vte_terminal_set_cursor_blink_mode(vte_terminal, cursor_blink_mode);
                vte_terminal_set_backspace_binding(vte_terminal, backspace_binding);
                vte_terminal_set_delete_binding(vte_terminal, delete_binding);
            }

            /*
             * Attmepts to load the configuration file.
             * Adopted from termite
             */ 
            void load_config_file(){
                // First set the defaults 
                build_default_style();

                GKeyFile *config = g_key_file_new();
                GError *gerror = nullptr;
                gboolean loaded = false;

                // First, try config in cli options
                if (cli_config_path) {
                    loaded = g_key_file_load_from_file(config,
                                                       cli_config_path,
                                                       G_KEY_FILE_NONE, &gerror);
                    if (!loaded){
                        if(gerror->code == G_KEY_FILE_ERROR_PARSE || 
                                gerror->code == G_FILE_ERROR_NOENT)
                            g_printerr("Config file parsing failed (%s): %s\n", cli_config_path,
                                   gerror->message);
                        g_clear_error(&gerror);
                    }
                }
                
                // If not, try in the user config path, usually /home/foo/.config 
                if (!loaded) {
                    gchar* path = g_strconcat(g_get_user_config_dir(), VTERM_DEF_CONFIG, nullptr);
                    loaded = g_key_file_load_from_file(config,
                                                       path,
                                                       G_KEY_FILE_NONE, &gerror);
                    if (!loaded){
                        if(gerror->code == G_KEY_FILE_ERROR_PARSE)
                            g_printerr("Config file parsing failed (%s): %s\n", path,
                                   gerror->message);
                        g_clear_error(&gerror);
                    }
                    g_free(path);
                }

                // If not, go nuts! 
                for (const gchar *const *dir = g_get_system_config_dirs();
                     !loaded && *dir; dir++) {
                    gchar* path = g_strconcat(*dir, VTERM_DEF_CONFIG, nullptr);
                    loaded = g_key_file_load_from_file(config, path,
                                                       G_KEY_FILE_NONE, &gerror);
                    if (!loaded){
                        if(gerror->code == G_KEY_FILE_ERROR_PARSE)
                            g_printerr("Config file parsing failed (%s): %s\n", path,
                                   gerror->message);
                        g_clear_error(&gerror);
                    }
                    g_free(path);       
                }

                // Yes! now set the configs, if not, defaults are already inplace 
                if (loaded) {
                    configure_vterm(config);
                }else{
                    g_printerr("No config file, using defaults.\n");
                }
                g_key_file_free(config);
            }

            /*
             * Whether transparency of any kind will be applied
             */ 
            gboolean is_transparency(){
                return(color_background_transparency > 0 || 
                    (focus_aware_color_background && 
                        (focus_out_color_background_transparency > 0)));
            }

         private:
            /*
             * Fill up this configuration object from the loaded file
             */ 
            void configure_vterm(GKeyFile* config){
                GError *gerror = nullptr;
                // Helpers for parsing configs

                // Check if last parsing results in error
                auto is_gerror = [&](const gchar *key){
                    if(gerror){
                        if(gerror->code == G_KEY_FILE_ERROR_INVALID_VALUE)
                            g_printerr("Error loading config \"%s\": %s\n",
                                    key, gerror->message);

                        g_clear_error(&gerror);
                        return true;
                    }
                    return false;
                };

                // Gets a boolean value or keep the default if key does not exist
                auto get_bool_or_def = [&](const gchar *section, const gchar *key, gboolean* def){
                    gboolean temp = g_key_file_get_boolean(config, section, key, &gerror);
                    if(!is_gerror(key))
                        *def = temp;
                };

                // Gets a color, or keep the default if key does not exist
                auto get_color_or_def = [&](const gchar *key, GdkRGBA* def){
                    gchar* rgba_string =  g_key_file_get_string(config, "style", key, &gerror);
                    if(!is_gerror(key)){
                        if(!gdk_rgba_parse(def, rgba_string)){
                            g_printerr("Error parsing color \"%s\"\n", rgba_string);
                        }
                        g_free(rgba_string);
                    }
                };


                /*
                 * Section [behavior]
                 */
                get_bool_or_def("behavior", "audible_bell", &audible_bell);
                get_bool_or_def("behavior", "bold_is_bright", &bold_is_bright);
                get_bool_or_def("behavior", "allow_hyperlink", &allow_hyperlink);
                get_bool_or_def("behavior", "scroll_on_output", &scroll_on_output);
                get_bool_or_def("behavior", "scroll_on_keystroke", &scroll_on_keystroke);
                get_bool_or_def("behavior", "mouse_autohide", &mouse_autohide);
                get_bool_or_def("behavior", "enable_bidi", &enable_bidi);
                get_bool_or_def("behavior", "enable_shaping", &enable_shaping);

                GET_STRING_AND_FREE(behavior, word_char_exceptions)

                GET_NUMBER(behavior, scrollback_lines, int, integer)

                PARSE_STRING(behavior, cursor_blink_mode, 
                        "on", VTE_CURSOR_BLINK_ON, 
                        "system", VTE_CURSOR_BLINK_SYSTEM, 
                        "off", VTE_CURSOR_BLINK_OFF)

                PARSE_STRING(behavior, backspace_binding,
                        "auto", VTE_ERASE_AUTO,
                        "backspace", VTE_ERASE_ASCII_BACKSPACE,
                        "delete", VTE_ERASE_ASCII_DELETE,
                        "@7", VTE_ERASE_DELETE_SEQUENCE, 
                        "term-erase", VTE_ERASE_TTY)

                PARSE_STRING(behavior, delete_binding,
                        "auto", VTE_ERASE_AUTO,
                        "backspace", VTE_ERASE_ASCII_BACKSPACE,
                        "delete", VTE_ERASE_ASCII_DELETE,
                        "@7", VTE_ERASE_DELETE_SEQUENCE, 
                        "term-erase", VTE_ERASE_TTY)

                PARSE_STRING(behavior, tab_cwd,
                        "current", CURRENT_TAB_CWD, 
                        "cli", CLI_CWD, 
                        "home", HOME_CWD)

                PARSE_STRING(behavior, tab_cmd,
                        "current", DEFAULT_CMD, 
                        "cli", CLI_CMD)

                GET_STRING_AND_FREE(behavior, window_title)
                GET_STRING_AND_FREE(behavior, window_role)

                get_bool_or_def("behavior", "window_size_hints", &window_size_hints);

                PARSE_STRING(behavior, tabs_position, 
                        "top", GTK_POS_TOP,
                        "bottom", GTK_POS_BOTTOM)

                PARSE_STRING(behavior, show_tab_policy,
                        "always", ALWAYS, 
                        "smart", SMART,
                        "auto", AUTO)

                get_bool_or_def("behavior", "tab_label_trim_first", &tab_label_trim_first);
                get_bool_or_def("behavior", "insert_after_current", &insert_after_current);

                /*
                 * Section [style]
                 */
                get_bool_or_def("style", "focus_aware_color_background", &focus_aware_color_background);
                get_bool_or_def("style", "show_scrollbar", &show_scrollbar);

                GET_NUMBER(style, font_scale, double, double)
                
                GET_NUMBER(style, cell_height_scale, double, double)

                GET_NUMBER(style, cell_width_scale, double, double)

                GET_NUMBER(style, color_background_transparency, double, double) 

                GET_NUMBER(style, focus_out_color_background_transparency, double, double)

                gchar* temp_font_desc = g_key_file_get_string(config, "style", "font", &gerror);
                if(!is_gerror("font")){
                    PangoFontDescription *temp_font = pango_font_description_from_string(temp_font_desc);
                    if(temp_font){
                        if(font)
                            pango_font_description_free(font);
                        font = temp_font;
                    }else{
                        g_printerr("Error loading font \"%s\"\n", temp_font_desc);
                    }
                    g_free(temp_font_desc);
                }

                get_color_or_def("color_bold", &color_bold); 
                get_color_or_def("color_foreground", &color_foreground); 
                get_color_or_def("color_background", &color_background); 
                get_color_or_def("focus_out_color_background", &focus_out_color_background); 
                get_color_or_def("color_cursor", &color_cursor); 
                get_color_or_def("color_cursor_foreground", &color_cursor_foreground); 
                get_color_or_def("color_highlight", &color_highlight); 
                get_color_or_def("color_highlight_foreground", &color_highlight_foreground);

                gchar color_key[] = "color000";
                for(gint i = 0; i < 256; i++){
                    snprintf(color_key, sizeof(color_key), "color%u", i);
                    get_color_or_def(color_key, &palette[i]);
                }

                PARSE_STRING(style, cursor_shape,
                        "block", VTE_CURSOR_SHAPE_BLOCK,
                        "ibeam", VTE_CURSOR_SHAPE_IBEAM,
                        "underline", VTE_CURSOR_SHAPE_UNDERLINE)

            }
           
            /*
             * Builds the default style for vterm
             */ 
            void build_default_style(){
                // Colors
                gdk_rgba_parse(&color_bold, "#f8f8f2");
                gdk_rgba_parse(&color_foreground, "#f8f8f2");
                gdk_rgba_parse(&color_background, "#282a36");
                gdk_rgba_parse(&focus_out_color_background, "#282a36");
                gdk_rgba_parse(&color_cursor, "#f8f8f2");
                gdk_rgba_parse(&color_cursor_foreground, "#000000");
                gdk_rgba_parse(&color_highlight, "#f8f8f2");
                gdk_rgba_parse(&color_highlight_foreground, "#282a36");

                // Dracula theme (palette 0-15)
                gdk_rgba_parse(&palette[0],  "#ffffff");
                gdk_rgba_parse(&palette[8],  "#4d4d4d");
                gdk_rgba_parse(&palette[1],  "#ff5555");
                gdk_rgba_parse(&palette[9],  "#ff6e67");
                gdk_rgba_parse(&palette[2],  "#50fa7b");
                gdk_rgba_parse(&palette[10], "#5af78e");
                gdk_rgba_parse(&palette[3],  "#f1fa8c");
                gdk_rgba_parse(&palette[11], "#f4f99d");
                gdk_rgba_parse(&palette[4],  "#bd93f9");
                gdk_rgba_parse(&palette[12], "#caa9fa");
                gdk_rgba_parse(&palette[5],  "#ff79c6");
                gdk_rgba_parse(&palette[13], "#ff92d0");
                gdk_rgba_parse(&palette[6],  "#8be9fd");
                gdk_rgba_parse(&palette[14], "#9aedfe");
                gdk_rgba_parse(&palette[7],  "#bfbfbf");
                gdk_rgba_parse(&palette[15], "#e6e6e6");

                // Rest of xterm palette
                // Auto generated
                gdk_rgba_parse(&palette[16],  "#000000");
                gdk_rgba_parse(&palette[124], "#af0000");
                gdk_rgba_parse(&palette[17],  "#00005f");
                gdk_rgba_parse(&palette[125], "#af005f");
                gdk_rgba_parse(&palette[18],  "#000087");
                gdk_rgba_parse(&palette[126], "#af0087");
                gdk_rgba_parse(&palette[19],  "#0000af");
                gdk_rgba_parse(&palette[127], "#af00af");
                gdk_rgba_parse(&palette[20],  "#0000d7");
                gdk_rgba_parse(&palette[128], "#af00d7");
                gdk_rgba_parse(&palette[21],  "#0000ff");
                gdk_rgba_parse(&palette[129], "#af00ff");
                gdk_rgba_parse(&palette[22],  "#005f00");
                gdk_rgba_parse(&palette[130], "#af5f00");
                gdk_rgba_parse(&palette[23],  "#005f5f");
                gdk_rgba_parse(&palette[131], "#af5f5f");
                gdk_rgba_parse(&palette[24],  "#005f87");
                gdk_rgba_parse(&palette[132], "#af5f87");
                gdk_rgba_parse(&palette[25],  "#005faf");
                gdk_rgba_parse(&palette[133], "#af5faf");
                gdk_rgba_parse(&palette[26],  "#005fd7");
                gdk_rgba_parse(&palette[134], "#af5fd7");
                gdk_rgba_parse(&palette[27],  "#005fff");
                gdk_rgba_parse(&palette[135], "#af5fff");
                gdk_rgba_parse(&palette[28],  "#008700");
                gdk_rgba_parse(&palette[136], "#af8700");
                gdk_rgba_parse(&palette[29],  "#00875f");
                gdk_rgba_parse(&palette[137], "#af875f");
                gdk_rgba_parse(&palette[30],  "#008787");
                gdk_rgba_parse(&palette[138], "#af8787");
                gdk_rgba_parse(&palette[31],  "#0087af");
                gdk_rgba_parse(&palette[139], "#af87af");
                gdk_rgba_parse(&palette[32],  "#0087d7");
                gdk_rgba_parse(&palette[140], "#af87d7");
                gdk_rgba_parse(&palette[33],  "#0087ff");
                gdk_rgba_parse(&palette[141], "#af87ff");
                gdk_rgba_parse(&palette[34],  "#00af00");
                gdk_rgba_parse(&palette[142], "#afaf00");
                gdk_rgba_parse(&palette[35],  "#00af5f");
                gdk_rgba_parse(&palette[143], "#afaf5f");
                gdk_rgba_parse(&palette[36],  "#00af87");
                gdk_rgba_parse(&palette[144], "#afaf87");
                gdk_rgba_parse(&palette[37],  "#00afaf");
                gdk_rgba_parse(&palette[145], "#afafaf");
                gdk_rgba_parse(&palette[38],  "#00afd7");
                gdk_rgba_parse(&palette[146], "#afafd7");
                gdk_rgba_parse(&palette[39],  "#00afff");
                gdk_rgba_parse(&palette[147], "#afafff");
                gdk_rgba_parse(&palette[40],  "#00d700");
                gdk_rgba_parse(&palette[148], "#afd700");
                gdk_rgba_parse(&palette[41],  "#00d75f");
                gdk_rgba_parse(&palette[149], "#afd75f");
                gdk_rgba_parse(&palette[42],  "#00d787");
                gdk_rgba_parse(&palette[150], "#afd787");
                gdk_rgba_parse(&palette[43],  "#00d7af");
                gdk_rgba_parse(&palette[151], "#afd7af");
                gdk_rgba_parse(&palette[44],  "#00d7d7");
                gdk_rgba_parse(&palette[152], "#afd7d7");
                gdk_rgba_parse(&palette[45],  "#00d7ff");
                gdk_rgba_parse(&palette[153], "#afd7ff");
                gdk_rgba_parse(&palette[46],  "#00ff00");
                gdk_rgba_parse(&palette[154], "#afff00");
                gdk_rgba_parse(&palette[47],  "#00ff5f");
                gdk_rgba_parse(&palette[155], "#afff5f");
                gdk_rgba_parse(&palette[48],  "#00ff87");
                gdk_rgba_parse(&palette[156], "#afff87");
                gdk_rgba_parse(&palette[49],  "#00ffaf");
                gdk_rgba_parse(&palette[157], "#afffaf");
                gdk_rgba_parse(&palette[50],  "#00ffd7");
                gdk_rgba_parse(&palette[158], "#afffd7");
                gdk_rgba_parse(&palette[51],  "#00ffff");
                gdk_rgba_parse(&palette[159], "#afffff");
                gdk_rgba_parse(&palette[52],  "#5f0000");
                gdk_rgba_parse(&palette[160], "#d70000");
                gdk_rgba_parse(&palette[53],  "#5f005f");
                gdk_rgba_parse(&palette[161], "#d7005f");
                gdk_rgba_parse(&palette[54],  "#5f0087");
                gdk_rgba_parse(&palette[162], "#d70087");
                gdk_rgba_parse(&palette[55],  "#5f00af");
                gdk_rgba_parse(&palette[163], "#d700af");
                gdk_rgba_parse(&palette[56],  "#5f00d7");
                gdk_rgba_parse(&palette[164], "#d700d7");
                gdk_rgba_parse(&palette[57],  "#5f00ff");
                gdk_rgba_parse(&palette[165], "#d700ff");
                gdk_rgba_parse(&palette[58],  "#5f5f00");
                gdk_rgba_parse(&palette[166], "#d75f00");
                gdk_rgba_parse(&palette[59],  "#5f5f5f");
                gdk_rgba_parse(&palette[167], "#d75f5f");
                gdk_rgba_parse(&palette[60],  "#5f5f87");
                gdk_rgba_parse(&palette[168], "#d75f87");
                gdk_rgba_parse(&palette[61],  "#5f5faf");
                gdk_rgba_parse(&palette[169], "#d75faf");
                gdk_rgba_parse(&palette[62],  "#5f5fd7");
                gdk_rgba_parse(&palette[170], "#d75fd7");
                gdk_rgba_parse(&palette[63],  "#5f5fff");
                gdk_rgba_parse(&palette[171], "#d75fff");
                gdk_rgba_parse(&palette[64],  "#5f8700");
                gdk_rgba_parse(&palette[172], "#d78700");
                gdk_rgba_parse(&palette[65],  "#5f875f");
                gdk_rgba_parse(&palette[173], "#d7875f");
                gdk_rgba_parse(&palette[66],  "#5f8787");
                gdk_rgba_parse(&palette[174], "#d78787");
                gdk_rgba_parse(&palette[67],  "#5f87af");
                gdk_rgba_parse(&palette[175], "#d787af");
                gdk_rgba_parse(&palette[68],  "#5f87d7");
                gdk_rgba_parse(&palette[176], "#d787d7");
                gdk_rgba_parse(&palette[69],  "#5f87ff");
                gdk_rgba_parse(&palette[177], "#d787ff");
                gdk_rgba_parse(&palette[70],  "#5faf00");
                gdk_rgba_parse(&palette[178], "#d7af00");
                gdk_rgba_parse(&palette[71],  "#5faf5f");
                gdk_rgba_parse(&palette[179], "#d7af5f");
                gdk_rgba_parse(&palette[72],  "#5faf87");
                gdk_rgba_parse(&palette[180], "#d7af87");
                gdk_rgba_parse(&palette[73],  "#5fafaf");
                gdk_rgba_parse(&palette[181], "#d7afaf");
                gdk_rgba_parse(&palette[74],  "#5fafd7");
                gdk_rgba_parse(&palette[182], "#d7afd7");
                gdk_rgba_parse(&palette[75],  "#5fafff");
                gdk_rgba_parse(&palette[183], "#d7afff");
                gdk_rgba_parse(&palette[76],  "#5fd700");
                gdk_rgba_parse(&palette[184], "#d7d700");
                gdk_rgba_parse(&palette[77],  "#5fd75f");
                gdk_rgba_parse(&palette[185], "#d7d75f");
                gdk_rgba_parse(&palette[78],  "#5fd787");
                gdk_rgba_parse(&palette[186], "#d7d787");
                gdk_rgba_parse(&palette[79],  "#5fd7af");
                gdk_rgba_parse(&palette[187], "#d7d7af");
                gdk_rgba_parse(&palette[80],  "#5fd7d7");
                gdk_rgba_parse(&palette[188], "#d7d7d7");
                gdk_rgba_parse(&palette[81],  "#5fd7ff");
                gdk_rgba_parse(&palette[189], "#d7d7ff");
                gdk_rgba_parse(&palette[82],  "#5fff00");
                gdk_rgba_parse(&palette[190], "#d7ff00");
                gdk_rgba_parse(&palette[83],  "#5fff5f");
                gdk_rgba_parse(&palette[191], "#d7ff5f");
                gdk_rgba_parse(&palette[84],  "#5fff87");
                gdk_rgba_parse(&palette[192], "#d7ff87");
                gdk_rgba_parse(&palette[85],  "#5fffaf");
                gdk_rgba_parse(&palette[193], "#d7ffaf");
                gdk_rgba_parse(&palette[86],  "#5fffd7");
                gdk_rgba_parse(&palette[194], "#d7ffd7");
                gdk_rgba_parse(&palette[87],  "#5fffff");
                gdk_rgba_parse(&palette[195], "#d7ffff");
                gdk_rgba_parse(&palette[88],  "#870000");
                gdk_rgba_parse(&palette[196], "#ff0000");
                gdk_rgba_parse(&palette[89],  "#87005f");
                gdk_rgba_parse(&palette[197], "#ff005f");
                gdk_rgba_parse(&palette[90],  "#870087");
                gdk_rgba_parse(&palette[198], "#ff0087");
                gdk_rgba_parse(&palette[91],  "#8700af");
                gdk_rgba_parse(&palette[199], "#ff00af");
                gdk_rgba_parse(&palette[92],  "#8700d7");
                gdk_rgba_parse(&palette[200], "#ff00d7");
                gdk_rgba_parse(&palette[93],  "#8700ff");
                gdk_rgba_parse(&palette[201], "#ff00ff");
                gdk_rgba_parse(&palette[94],  "#875f00");
                gdk_rgba_parse(&palette[202], "#ff5f00");
                gdk_rgba_parse(&palette[95],  "#875f5f");
                gdk_rgba_parse(&palette[203], "#ff5f5f");
                gdk_rgba_parse(&palette[96],  "#875f87");
                gdk_rgba_parse(&palette[204], "#ff5f87");
                gdk_rgba_parse(&palette[97],  "#875faf");
                gdk_rgba_parse(&palette[205], "#ff5faf");
                gdk_rgba_parse(&palette[98],  "#875fd7");
                gdk_rgba_parse(&palette[206], "#ff5fd7");
                gdk_rgba_parse(&palette[99],  "#875fff");
                gdk_rgba_parse(&palette[207], "#ff5fff");
                gdk_rgba_parse(&palette[100], "#878700");
                gdk_rgba_parse(&palette[208], "#ff8700");
                gdk_rgba_parse(&palette[101], "#87875f");
                gdk_rgba_parse(&palette[209], "#ff875f");
                gdk_rgba_parse(&palette[102], "#878787");
                gdk_rgba_parse(&palette[210], "#ff8787");
                gdk_rgba_parse(&palette[103], "#8787af");
                gdk_rgba_parse(&palette[211], "#ff87af");
                gdk_rgba_parse(&palette[104], "#8787d7");
                gdk_rgba_parse(&palette[212], "#ff87d7");
                gdk_rgba_parse(&palette[105], "#8787ff");
                gdk_rgba_parse(&palette[213], "#ff87ff");
                gdk_rgba_parse(&palette[106], "#87af00");
                gdk_rgba_parse(&palette[214], "#ffaf00");
                gdk_rgba_parse(&palette[107], "#87af5f");
                gdk_rgba_parse(&palette[215], "#ffaf5f");
                gdk_rgba_parse(&palette[108], "#87af87");
                gdk_rgba_parse(&palette[216], "#ffaf87");
                gdk_rgba_parse(&palette[109], "#87afaf");
                gdk_rgba_parse(&palette[217], "#ffafaf");
                gdk_rgba_parse(&palette[110], "#87afd7");
                gdk_rgba_parse(&palette[218], "#ffafd7");
                gdk_rgba_parse(&palette[111], "#87afff");
                gdk_rgba_parse(&palette[219], "#ffafff");
                gdk_rgba_parse(&palette[112], "#87d700");
                gdk_rgba_parse(&palette[220], "#ffd700");
                gdk_rgba_parse(&palette[113], "#87d75f");
                gdk_rgba_parse(&palette[221], "#ffd75f");
                gdk_rgba_parse(&palette[114], "#87d787");
                gdk_rgba_parse(&palette[222], "#ffd787");
                gdk_rgba_parse(&palette[115], "#87d7af");
                gdk_rgba_parse(&palette[223], "#ffd7af");
                gdk_rgba_parse(&palette[116], "#87d7d7");
                gdk_rgba_parse(&palette[224], "#ffd7d7");
                gdk_rgba_parse(&palette[117], "#87d7ff");
                gdk_rgba_parse(&palette[225], "#ffd7ff");
                gdk_rgba_parse(&palette[118], "#87ff00");
                gdk_rgba_parse(&palette[226], "#ffff00");
                gdk_rgba_parse(&palette[119], "#87ff5f");
                gdk_rgba_parse(&palette[227], "#ffff5f");
                gdk_rgba_parse(&palette[120], "#87ff87");
                gdk_rgba_parse(&palette[228], "#ffff87");
                gdk_rgba_parse(&palette[121], "#87ffaf");
                gdk_rgba_parse(&palette[229], "#ffffaf");
                gdk_rgba_parse(&palette[122], "#87ffd7");
                gdk_rgba_parse(&palette[230], "#ffffd7");
                gdk_rgba_parse(&palette[123], "#87ffff");
                gdk_rgba_parse(&palette[231], "#ffffff");
                gdk_rgba_parse(&palette[232], "#080808");
                gdk_rgba_parse(&palette[244], "#808080");
                gdk_rgba_parse(&palette[233], "#121212");
                gdk_rgba_parse(&palette[245], "#8a8a8a");
                gdk_rgba_parse(&palette[234], "#1c1c1c");
                gdk_rgba_parse(&palette[246], "#949494");
                gdk_rgba_parse(&palette[235], "#262626");
                gdk_rgba_parse(&palette[247], "#9e9e9e");
                gdk_rgba_parse(&palette[236], "#303030");
                gdk_rgba_parse(&palette[248], "#a8a8a8");
                gdk_rgba_parse(&palette[237], "#3a3a3a");
                gdk_rgba_parse(&palette[249], "#b2b2b2");
                gdk_rgba_parse(&palette[238], "#444444");
                gdk_rgba_parse(&palette[250], "#bcbcbc");
                gdk_rgba_parse(&palette[239], "#4e4e4e");
                gdk_rgba_parse(&palette[251], "#c6c6c6");
                gdk_rgba_parse(&palette[240], "#585858");
                gdk_rgba_parse(&palette[252], "#d0d0d0");
                gdk_rgba_parse(&palette[241], "#626262");
                gdk_rgba_parse(&palette[253], "#dadada");
                gdk_rgba_parse(&palette[242], "#6c6c6c");
                gdk_rgba_parse(&palette[254], "#e4e4e4");
                gdk_rgba_parse(&palette[243], "#767676");
                gdk_rgba_parse(&palette[255], "#eeeeee");
            }
    };
}
#endif
