#ifndef COMMON_H
#define COMMON_H
#include "vte/vte.h"

#include <gtk/gtk.h>

/*
 * VTerm information
 */
#define VTERM_VERSION "0.1"
#define VTERM_CREDITS "Author: Mansour Alharthi <man9our.ah@gmail.com>."
#define VTERM_DEF_CONFIG "/vterm/vterm.conf"
#define VTERM_ASCIIART \
    " _   _   _____  _____ ______ ___  ___\n" \
    "| | | | |_   _||  ___|| ___ \\|  \\/  |\n" \
    "| | | |   | |  | |__  | |_/ /| .  . |\n" \
    "| | | |   | |  |  __| |    / | |\\/| |\n" \
    "\\ \\_/ /   | |  | |___ | |\\ \\ | |  | |\n" \
    " \\___/    \\_/  \\____/ \\_| \\_|\\_|  |_/\n" \
    "                                     \n"

#ifdef VTERM_DEBUG
#define DEBUG_PRINT(...) g_printerr(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

namespace VTERM{

    /*
     * Util functions
     */
    static inline void exit_error(){
        gtk_main_quit();
        exit(EXIT_FAILURE);
    }

    static inline void exit_success(){
        gtk_main_quit();
        exit(EXIT_FAILURE);
    }

    static inline gchar** get_user_shell() {
        gchar** shell = (gchar**)g_malloc0_n(2, sizeof(gchar*));

        shell[0] = vte_get_user_shell();
        if (shell[0] == nullptr || shell[0][0] == '\0') {
                g_free(shell[0]);
                shell[0] = g_strdup(g_getenv("SHELL"));
        }
        if (shell[0] == nullptr || shell[0][0] == '\0') {
                g_free(shell);
                shell[0] = g_strdup("/bin/sh");
        }
        return shell;
    }

    static inline void launch_app(char** cmd){
        GError *error = nullptr;

        GPid child_pid;
        if (!g_spawn_async(nullptr, cmd, nullptr, G_SPAWN_SEARCH_PATH,
                           nullptr, nullptr, &child_pid, &error)) {
            g_printerr("error launching '%s': %s\n", cmd[0], error->message);
            g_error_free(error);
        }
        g_spawn_close_pid(child_pid);
    }
}
#endif
