/* AMCL - A simple Mud CLient
 * Copyright (C) 1998-2000 Robin Ericsson <lobbin@localhost.nu>
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

#include "config.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glib.h>

#include "amcl.h"

static char const rcsid[] = "$Id$";

/*
 * Local Functions
 */
gushort convert_color  (guint color);
void    extract_color  (GdkColor *color,
                        guint red,
                        guint green,
                        guint blue);
void    connect_window (GtkWidget *widget, gpointer data);
void    about_window   (GtkWidget *widget, gpointer data);
void    do_connection  (GtkWidget *widget, gpointer data);

/*
 * Global Variables
 */
CONNECTION_DATA *main_connection;
CONNECTION_DATA *connections[15];
GtkWidget *main_notebook;
GtkWidget *text_entry;
GtkWidget *entry_host;
GtkWidget *entry_port;
GtkWidget *menu_plugin_menu;
GtkWidget *menu_main_wizard;
GtkWidget *menu_main_connect;
GtkWidget *menu_main_disconnect;
GtkWidget *menu_main_close;
GtkWidget *menu_option_prefs;
GtkWidget *menu_option_alias;
GtkWidget *menu_option_mapper;
GtkWidget *menu_option_colors;
GtkWidget *menu_option_action;
GtkWidget *menu_option_keys;
GtkWidget *window;
/*
 * Colors...
 */
GdkColormap *cmap;
GdkColor color_white;
GdkColor color_black;
GdkColor color_blue;
GdkColor color_green;
GdkColor color_red;
GdkColor color_brown;
GdkColor color_magenta;
GdkColor color_lightred;
GdkColor color_yellow;
GdkColor color_lightgreen;
GdkColor color_cyan;
GdkColor color_lightcyan;
GdkColor color_lightblue;
GdkColor color_lightmagenta;
GdkColor color_grey;
GdkColor color_lightgrey;

GdkFont  *font_normal;
GdkFont  *font_fixed;

GList *EntryHistory = NULL;
GList *EntryCurr    = NULL;
bool   Keyflag      = TRUE;

void close_window (GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy (GTK_WIDGET (data));
}

void destroy (GtkWidget *widget)
{
    gtk_main_quit ();
}

void connect_window (GtkWidget *widget, gpointer data)
{
    GtkWidget *window;
    GtkWidget *label;
    GtkWidget *vbox;
    GtkWidget *hbox, *hbox2, *hbox3;
    GtkWidget *button;
    GtkWidget *button_close;
    GtkWidget *separator;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window),"Connect...");

    vbox = gtk_vbox_new (FALSE, 5);
    gtk_container_border_width (GTK_CONTAINER (vbox), 5);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_widget_show (vbox  );
    
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (vbox), hbox);
    gtk_widget_show (hbox  );

    label = gtk_label_new ("Host:");
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, FALSE, 5);
    gtk_widget_show (label);

    entry_host = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (hbox), entry_host, FALSE, FALSE, 0);
    gtk_widget_show (entry_host);

    hbox2 = gtk_hbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (vbox), hbox2);
    gtk_widget_show (hbox2);

    label = gtk_label_new ("Port:");
    gtk_box_pack_start (GTK_BOX (hbox2), label, TRUE, FALSE, 5);
    gtk_widget_show (label);

    entry_port = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (hbox2), entry_port, FALSE, FALSE, 0);
    gtk_widget_show (entry_port);
    
    separator = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, TRUE, 5);
    gtk_widget_show (separator);
    
    hbox3 = gtk_hbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (vbox), hbox3);
    gtk_widget_show (hbox3);
    
    button       = gtk_button_new_with_label (" connect ");
    button_close = gtk_button_new_with_label ("  close  ");
    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (do_connection), NULL );
    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (close_window), window);
    gtk_signal_connect (GTK_OBJECT (button_close), "clicked",
                        GTK_SIGNAL_FUNC (close_window), window);
    gtk_box_pack_start (GTK_BOX (hbox3), button, TRUE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (hbox3), button_close, TRUE, TRUE, 5);
    gtk_widget_show (button);
    gtk_widget_show (button_close);

    if ( port != NULL && host != NULL )
    {
        gtk_entry_set_text ((GtkEntry *) entry_host, host);
        gtk_entry_set_text ((GtkEntry *) entry_port, port);
    }
    gtk_widget_show (window);
}

void about_window (GtkWidget *widget, gpointer data)
{
    GtkWidget *label;
    GtkWidget *button;
    GtkWidget *main_box;
    GtkWidget *box2;
    GtkWidget *a_window;
    GtkWidget *separator;

    a_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (a_window),"About");

    main_box = gtk_vbox_new (FALSE, 0);
    gtk_container_border_width (GTK_CONTAINER (main_box), 5);
    gtk_container_add (GTK_CONTAINER (a_window), main_box);

    label = gtk_label_new ("AMCL version "VERSION"");
    gtk_box_pack_start (GTK_BOX (main_box), label, FALSE, FALSE, 5);
    gtk_widget_show (label);

    label = gtk_label_new ("Copyright � 1998-2000 Robin Ericsson <lobbin@localhost.nu>");
    gtk_box_pack_start (GTK_BOX (main_box), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    label = gtk_label_new ("   Licensed under GNU GENERAL PUBLIC LICENSE (GPL)    ");
    gtk_box_pack_start (GTK_BOX (main_box), label, FALSE, FALSE, 5);
    gtk_widget_show (label);

    label = gtk_label_new ("Homepage: http://www.localhost.nu/apps/amcl/");
    gtk_box_pack_start (GTK_BOX (main_box), label, FALSE, FALSE, 5);
    gtk_widget_show (label);

    separator = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (main_box), separator, FALSE, TRUE, 5);
    gtk_widget_show (separator);

    box2 = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start (GTK_BOX (main_box), box2, FALSE, FALSE, 0);

    button = gtk_button_new_with_label ( " close ");
    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (close_window), a_window);
    gtk_box_pack_start (GTK_BOX (box2), button, TRUE, TRUE, 5);

    gtk_widget_show (button  );
    gtk_widget_show (box2    );
    gtk_widget_show (main_box);
    gtk_widget_show (a_window);
}

GList *text_entry_find (gchar *text)
{
    GList *list;

    list = EntryHistory;

    while ( list && list->data )
    {
        if ( !strcmp ( (char *) list->data, text) )
            return list;
        list = list->next;
    }

    return NULL;
}

int text_entry_key_press_cb (GtkEntry *entry, GdkEventKey *event, gpointer data)
{
  CONNECTION_DATA *cd;
  gint   number;
  GList *li;
  KEYBIND_DATA *scroll = KB_head;
  
  number = gtk_notebook_get_current_page (GTK_NOTEBOOK (main_notebook));
  cd = connections[number];
  
    if ( event->state & GDK_CONTROL_MASK )
    {
    }
    else
    {if (EntryCurr)
        switch ( event->keyval )
        {
        case GDK_Page_Up:
        case GDK_Page_Down:
            
        case GDK_Up:
	  //case GDK_KP_Up:
	  if (EntryCurr->prev) {
	    li = EntryCurr->prev;
	    
	    if ( !prefs.KeepText && Keyflag) li = li->next;
	    EntryCurr = li;
	    gtk_entry_set_text (GTK_ENTRY (text_entry), (gchar *) li->data);
	    gtk_entry_set_position (GTK_ENTRY (text_entry) ,GTK_ENTRY (text_entry)->text_length);
	    gtk_entry_select_region (GTK_ENTRY (text_entry), 0,
				     GTK_ENTRY (text_entry)->text_length);
	    Keyflag = FALSE;
	  }
	  gtk_signal_emit_stop_by_name (GTK_OBJECT (entry), "key_press_event");
	  return TRUE;
	  break;

        case GDK_Down:
	  //case GDK_KP_Down:
	  if (EntryCurr->next) {
	    li = EntryCurr->next;
	    EntryCurr = li;
	    gtk_entry_set_text (GTK_ENTRY (text_entry), (gchar *) li->data);
	    gtk_entry_set_position (GTK_ENTRY (text_entry) ,GTK_ENTRY (text_entry)->text_length);
	    gtk_entry_select_region (GTK_ENTRY (text_entry), 0,
				     GTK_ENTRY (text_entry)->text_length);
	  } else {
	    EntryCurr = g_list_last (EntryHistory);
	    gtk_entry_set_text (GTK_ENTRY (text_entry), "");
	  }            
	  gtk_signal_emit_stop_by_name (GTK_OBJECT (entry), "key_press_event");
	  return TRUE;
	  break;
        }
    }

    for( scroll = KB_head ;scroll != NULL; scroll = scroll->next)
      if ( (scroll->state) == ((event->state)&12) && 
	   (scroll->keyv) == (event->keyval) )
	{
	  connection_send(cd, scroll->data);
	  connection_send(cd, "\n");
	  gtk_signal_emit_stop_by_name (GTK_OBJECT (entry), "key_press_event");
	  return TRUE;	  
	}
    
    
    return FALSE;
}

void do_connection (GtkWidget *widget, gpointer data)
{
  CONNECTION_DATA *cd;

  g_free(host); g_free(port);
  host = g_strdup (gtk_entry_get_text (GTK_ENTRY( entry_host)));
  port = g_strdup (gtk_entry_get_text (GTK_ENTRY( entry_port)));  

  make_connection(host,port);
}

void do_close (GtkWidget *widget, gpointer data)
{
  CONNECTION_DATA *cd;
  gint number;

  number = gtk_notebook_get_current_page (GTK_NOTEBOOK (main_notebook));

  cd = connections[number];

  if (cd->connected)
    disconnect (NULL, cd);

  gtk_notebook_remove_page (GTK_NOTEBOOK (main_notebook), number);
  free_connection_data (cd);
}

void do_disconnect (GtkWidget *widget, gpointer data)
{
  CONNECTION_DATA *cd;
  GList *list;
  gint number;

  number = gtk_notebook_get_current_page (GTK_NOTEBOOK (main_notebook));

  cd = connections[number];
  disconnect (NULL, cd);

  if (!cd->connected && menu_main_disconnect)
    gtk_widget_set_sensitive (menu_main_disconnect, FALSE);
}

void init_window ()
{
    GtkWidget *label;
    GtkWidget *box_main;
    GtkWidget *box_main2;
    GtkWidget *box_h_low;
    GtkWidget *menu_bar;
    GtkWidget *menu_main;
    GtkWidget *menu_help;
    GtkWidget *menu_option;
    GtkWidget *menu_plugin;
    GtkWidget *menu_main_menu;
    GtkWidget *menu_help_menu;
    GtkWidget *menu_option_menu;
    GtkWidget *menu_main_quit;
    GtkWidget *menu_plugin_info;
    GtkWidget *menu_help_about;
    GtkWidget *menu_help_readme;
    GtkWidget *menu_help_authors;

    GtkWidget *separator;
    GtkWidget *v_scrollbar;

    font_fixed = gdk_font_load("fixed");

    /* init widgets */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "AMCL "VERSION"");
    gtk_signal_connect_object (GTK_OBJECT (window), "destroy",
                               GTK_SIGNAL_FUNC(destroy), NULL );

    box_main = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), box_main);
    
    menu_bar = gtk_menu_bar_new ();
    gtk_box_pack_start (GTK_BOX( box_main), menu_bar, FALSE, FALSE, 0);

    menu_main = gtk_menu_item_new_with_label ("Main");
    gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), menu_main);

    menu_main_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_main), menu_main_menu);

    menu_main_wizard = gtk_menu_item_new_with_label ("Connection Wizard...");
    gtk_menu_append (GTK_MENU (menu_main_menu), menu_main_wizard);
    gtk_signal_connect_object (GTK_OBJECT (menu_main_wizard), "activate",
                               GTK_SIGNAL_FUNC (window_wizard), NULL);
    gtk_widget_show (menu_main_wizard);

    separator = gtk_menu_item_new ();
    gtk_menu_append (GTK_MENU (menu_main_menu), separator);
    gtk_widget_show (separator);

    menu_main_connect = gtk_menu_item_new_with_label ("Connect...");
    gtk_menu_append (GTK_MENU (menu_main_menu), menu_main_connect );
    gtk_signal_connect_object (GTK_OBJECT (menu_main_connect), "activate",
                               GTK_SIGNAL_FUNC (connect_window), NULL );

    menu_main_disconnect = gtk_menu_item_new_with_label ("Disconnect");
    gtk_menu_append (GTK_MENU (menu_main_menu), menu_main_disconnect );
    gtk_signal_connect_object (GTK_OBJECT (menu_main_disconnect), "activate",
                               GTK_SIGNAL_FUNC (do_disconnect), NULL );

    separator = gtk_menu_item_new ();
    gtk_menu_append (GTK_MENU (menu_main_menu), separator);
    gtk_widget_show (separator);

    menu_main_close = gtk_menu_item_new_with_label ("Close Window");
    gtk_menu_append (GTK_MENU (menu_main_menu), menu_main_close);
    gtk_signal_connect_object (GTK_OBJECT (menu_main_close), "activate",
			       GTK_SIGNAL_FUNC (do_close), NULL);
    gtk_widget_show (menu_main_close);

    separator = gtk_menu_item_new ();
    gtk_menu_append (GTK_MENU (menu_main_menu), separator);
    gtk_widget_show (separator);

    menu_main_quit = gtk_menu_item_new_with_label ("Quit");
    gtk_menu_append (GTK_MENU (menu_main_menu), menu_main_quit );
    gtk_signal_connect_object (GTK_OBJECT (menu_main_quit), "activate",
                               GTK_SIGNAL_FUNC (destroy), NULL );

    menu_option = gtk_menu_item_new_with_label ("Options");
    gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), menu_option);

    menu_option_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_option), menu_option_menu);

    menu_option_alias = gtk_menu_item_new_with_label ("Alias...");
    gtk_menu_append (GTK_MENU (menu_option_menu), menu_option_alias);
    gtk_signal_connect_object (GTK_OBJECT (menu_option_alias), "activate",
                               GTK_SIGNAL_FUNC (window_alias), NULL);

    menu_option_prefs = gtk_menu_item_new_with_label ("Preferences...");
    gtk_menu_append (GTK_MENU (menu_option_menu), menu_option_prefs);
    gtk_signal_connect_object (GTK_OBJECT (menu_option_prefs), "activate",
                               GTK_SIGNAL_FUNC (window_prefs), NULL);

#ifndef WITHOUT_MAPPER
    menu_option_mapper = gtk_menu_item_new_with_label ("Auto Mapper...");
    gtk_menu_append (GTK_MENU (menu_option_menu), menu_option_mapper);
    gtk_signal_connect (GTK_OBJECT (menu_option_mapper), "activate",
                        GTK_SIGNAL_FUNC (window_automap), NULL);
#endif

    menu_option_colors = gtk_menu_item_new_with_label ("Colors...");
    gtk_menu_append (GTK_MENU (menu_option_menu), menu_option_colors);
    gtk_signal_connect (GTK_OBJECT (menu_option_colors), "activate",
                        GTK_SIGNAL_FUNC (window_color), NULL);

    menu_option_action = gtk_menu_item_new_with_label ("Actions/Triggers...");
    gtk_menu_append (GTK_MENU (menu_option_menu), menu_option_action);
    gtk_signal_connect (GTK_OBJECT (menu_option_action), "activate",
                        GTK_SIGNAL_FUNC (window_action), NULL);

    menu_option_keys = gtk_menu_item_new_with_label ("Keys...");
    gtk_menu_append (GTK_MENU (menu_option_menu), menu_option_keys);
    gtk_signal_connect (GTK_OBJECT (menu_option_keys), "activate",
                        GTK_SIGNAL_FUNC (window_keybind), NULL);
    
    menu_plugin = gtk_menu_item_new_with_label ("Plugins");
    gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), menu_plugin);

    menu_plugin_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_plugin), menu_plugin_menu);
    
    separator = gtk_menu_item_new ();
    gtk_menu_append (GTK_MENU (menu_plugin_menu), separator);
    gtk_widget_show (separator);

    menu_plugin_info = gtk_menu_item_new_with_label ("Plugin Information");
    gtk_menu_append (GTK_MENU (menu_plugin_menu), menu_plugin_info);
    gtk_signal_connect (GTK_OBJECT (menu_plugin_info), "activate",
			GTK_SIGNAL_FUNC (do_plugin_information), NULL);
    gtk_widget_show (menu_plugin_info);

    menu_help = gtk_menu_item_new_with_label ("Help");
    gtk_menu_item_right_justify (GTK_MENU_ITEM (menu_help ));
    gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), menu_help);

    menu_help_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_help), menu_help_menu);

    menu_help_readme = gtk_menu_item_new_with_label ("Readme");
    gtk_menu_append (GTK_MENU (menu_help_menu), menu_help_readme);
    gtk_signal_connect (GTK_OBJECT (menu_help_readme), "activate",
                        GTK_SIGNAL_FUNC (doc_dialog),
                        GINT_TO_POINTER (1));

    menu_help_authors = gtk_menu_item_new_with_label ("Authors");
    gtk_menu_append (GTK_MENU (menu_help_menu), menu_help_authors);
    gtk_signal_connect (GTK_OBJECT (menu_help_authors), "activate",
                        GTK_SIGNAL_FUNC (doc_dialog),
                        GINT_TO_POINTER (2));

    separator = gtk_menu_item_new ();
    gtk_menu_append (GTK_MENU (menu_help_menu), separator);
    gtk_widget_show (separator);
    
    menu_help_about = gtk_menu_item_new_with_label ("About...");
    gtk_menu_append (GTK_MENU (menu_help_menu), menu_help_about );
    gtk_signal_connect_object (GTK_OBJECT (menu_help_about),"activate",
                               GTK_SIGNAL_FUNC (about_window), NULL);

    gtk_widget_set_sensitive ((GtkWidget*)menu_main_disconnect, FALSE);

    gtk_widget_show (menu_bar);
    
    box_main2 = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (box_main), box_main2, TRUE, TRUE, 5);

    box_h_low = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (box_main2), box_h_low, TRUE, TRUE, 5);

    main_notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK (main_notebook), GTK_POS_BOTTOM);
    gtk_signal_connect (GTK_OBJECT (main_notebook), "switch-page",
                        GTK_SIGNAL_FUNC (switch_page_cb), NULL);
    gtk_box_pack_start (GTK_BOX (box_h_low), main_notebook, TRUE, TRUE, 0);
    gtk_widget_show (main_notebook);

    main_connection = g_malloc0( sizeof (CONNECTION_DATA));
    main_connection->notebook = 0;
    main_connection->window = gtk_text_new (NULL, NULL);
    gtk_widget_set_usize (main_connection->window, 500, 320);
    gtk_widget_show (main_connection->window);
    connections[0] = main_connection;
    
    foreground = &color_white;
    background = &color_black;

    label = gtk_label_new ("Main");
    gtk_notebook_append_page (GTK_NOTEBOOK (main_notebook), main_connection->window, label);

    v_scrollbar = gtk_vscrollbar_new (GTK_TEXT(main_connection->window)->vadj);
    gtk_box_pack_start (GTK_BOX (box_h_low), v_scrollbar, FALSE, FALSE, 0);

    text_entry = gtk_entry_new ();
    gtk_signal_connect (GTK_OBJECT (text_entry), "key_press_event",
                        GTK_SIGNAL_FUNC (text_entry_key_press_cb), NULL);
    gtk_signal_connect (GTK_OBJECT (text_entry), "activate",
                        GTK_SIGNAL_FUNC (send_to_connection), NULL);
    gtk_box_pack_start (GTK_BOX(box_main), text_entry, FALSE, TRUE, 0);
    gtk_widget_grab_focus (text_entry);
    gtk_widget_show (text_entry);

    /* show them */
    gtk_widget_show (v_scrollbar         );
    gtk_widget_show (box_h_low           );
    gtk_widget_show (box_main2           );
    gtk_widget_show (menu_help_readme    );
    gtk_widget_show (menu_help_authors   );
    gtk_widget_show (menu_help_about     );
    gtk_widget_show (menu_main_disconnect);
    gtk_widget_show (menu_main_connect   );
    gtk_widget_show (menu_main_quit      );
    gtk_widget_show (menu_option_alias   );
    gtk_widget_show (menu_option_prefs   );
    gtk_widget_show (menu_option_mapper  );
    gtk_widget_show (menu_option_colors  );
    gtk_widget_show (menu_option_action  );
    gtk_widget_show (menu_option_keys    );
    gtk_widget_show (menu_plugin         );
    gtk_widget_show (menu_option         );
    gtk_widget_show (menu_help           );
    gtk_widget_show (menu_main           );
    gtk_widget_show (box_main            );
    gtk_widget_show (window              );

    gtk_widget_realize (main_connection->window);
    gdk_window_set_background (GTK_TEXT (main_connection->window)->text_area, &color_black);

    {
        char  buf[1024];

        get_version_info (buf);

        gtk_text_insert (GTK_TEXT (main_connection->window), NULL, &color_lightgrey, NULL, "Distributed under the terms of the GNU General Public License.\n", -1);
        gtk_text_insert (GTK_TEXT (main_connection->window), NULL, &color_lightgrey, NULL, buf, -1);
    }

    //create_color_box ();
}
