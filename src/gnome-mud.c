/* GNOME-Mud - A simple Mud Client
 * gnome-mud.c
 * Copyright (C) 1998-2005 Robin Ericsson <lobbin@localhost.nu>
 * Copyright (C) 2005-2009 Les Harris <lharris@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#if HAVE_CONFIG_H
#  include "config.h"
#endif

#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef ENABLE_GST
#include <gst/gst.h>
#endif

#include "gnome-mud.h"
#include "gnome-mud-icons.h"
#include "mud-connection-view.h"
#include "mud-window.h"
#include "utils.h"
#include "debug-logger.h"
#include "mud-trigger.h"

gint
main (gint argc, char *argv[])
{
    DebugLogger *logger;
    GString *dir;
    GSettings *global_settings;

#ifdef ENABLE_NLS
    /* Initialize internationalization */
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif

    /* Set up schema version */
    global_settings = g_settings_new("org.gnome.MUD");
    /* Add any future migration triggers here */
    g_settings_set_uint(global_settings, "schema-version", 1);
    g_object_unref(global_settings);

    gtk_init(&argc, &argv);

#ifdef ENABLE_GST
    /* Initialize GStreamer */
    gst_init(&argc, &argv);
#endif

    dir = g_string_new(NULL);
    g_string_printf(dir,
                    "%s%cgnome-mud%clogs",
                    g_get_user_data_dir(),
                    G_DIR_SEPARATOR,
                    G_DIR_SEPARATOR);
    g_mkdir_with_parents(dir->str, 0755);
    g_string_free(dir, TRUE);

    dir = g_string_new(NULL);
    g_string_printf(dir,
                    "%s%cgnome-mud%caudio",
                    g_get_user_data_dir(),
                    G_DIR_SEPARATOR,
                    G_DIR_SEPARATOR);
    g_mkdir_with_parents(dir->str, 0755); 
    g_string_free(dir, TRUE);

    gtk_window_set_default_icon_name(GMUD_STOCK_ICON);

     /*Setup debug logging */
     logger = g_object_new(TYPE_DEBUG_LOGGER, 
                          "use-color", TRUE,
                          "closeable", FALSE,
                          NULL);

    debug_logger_add_domain(logger, "Gnome-Mud", TRUE);
    debug_logger_add_domain(logger, "Telnet", FALSE);
    debug_logger_add_standard_domains(logger);

#ifdef ENABLE_DEBUG_LOGGER
    debug_logger_create_window(logger);
#endif

    //g_log_set_always_fatal(G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_WARNING);

    /* Let 'er rip */
    g_object_new(MUD_TYPE_WINDOW, NULL);

    gtk_main();

    g_object_unref(logger);

    return 0;
}

