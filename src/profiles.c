/* GNOME-Mud - A simple Mud CLient
 * Copyright (C) 1998-2001 Robin Ericsson <lobbin@localhost.nu>
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

#include <ctype.h>
#include <gnome.h>

#include "gnome-mud.h"

static char const rcsid[] =
    "$Id$";

WIZARD_DATA2 *connections_find_by_profilename(gchar *name);
gint 		  get_size(GtkCList *clist);
GtkWidget *clist;
GList	  *ProfilesList;
GList	  *ProfilesList_tmp;
GList	  *ProfilesData;
GList	  *Profiles;
gint	   selected;
gint	   connection_selected;

GtkWidget *entry_info_mud_title;
GtkWidget *entry_info_mud_host;
GtkWidget *entry_info_mud_port;
GtkWidget *entry_info_char_character;
GtkWidget *entry_info_char_password;
GtkWidget *label_info_prof;

GtkWidget *main_clist;

gint g_list_compare_strings(gchar *a, gchar *b)
{
	return g_strcasecmp((gchar *) a, (gchar *) b);
}

void load_profiles()
{
	// profiles
	{
		extern GList *ProfilesList;
		extern GList *ProfilesData;
		gint nr, i;
		gchar **profiles;
		gnome_config_get_vector("/gnome-mud/Data/Profiles", &nr, &profiles);

		if (nr == 0)
		{
			ProfilesList = g_list_append(ProfilesList, "Default");
		}

		for (i = 0; i < nr; i++)
		{
			PROFILE_DATA *pd = g_malloc0(sizeof(PROFILE_DATA));
			gchar name[50];
			gint  new_nr, k;
			gchar **vector, **vector2;
		
			pd->name  	  = profiles[i];
			pd->alias 	  = (GtkCList *) gtk_clist_new(2);
			pd->variables = (GtkCList *) gtk_clist_new(2);
			pd->triggers  = (GtkCList *) gtk_clist_new(2);

			g_snprintf(name, 50, "/gnome-mud/Profile: %s/Alias", profiles[i]);
			gnome_config_get_vector(name, &new_nr, &vector);
			g_snprintf(name, 50, "/gnome-mud/Profile: %s/AliasValue", profiles[i]);
			gnome_config_get_vector(name, &new_nr, &vector2);
			for (k = 0; k < new_nr; k++)
			{
				gchar *t[2] = { vector[k], vector2[k]};

				gtk_clist_append(pd->alias, t);
			}

			ProfilesList = g_list_append(ProfilesList, (gpointer) profiles[i]);
			ProfilesData = g_list_append(ProfilesData, (gpointer) pd);
		}
	}

	// wizard
	{
		WIZARD_DATA2 *wd;
		
		extern GList *Profiles;
		gint i = 0;
		gchar name[50];
		gchar *value;

		while(1)
		{
			g_snprintf(name, 50, "/gnome-mud/Connection%d/Name=", i);
			value = gnome_config_get_string(name);

			if (strlen(value) < 1)
			{
				break;
			}
			
			wd = g_malloc0(sizeof(WIZARD_DATA2));
			
			wd->name = value;
			
			g_snprintf(name, 50, "/gnome-mud/Connection%d/Host=", i);
			wd->hostname = gnome_config_get_string(name);
			
			g_snprintf(name, 50, "/gnome-mud/Connection%d/Port=", i);
			wd->port = gnome_config_get_string(name);
			
			g_snprintf(name, 50, "/gnome-mud/Connection%d/Char=", i);
			wd->playername = gnome_config_get_string(name);
			
			g_snprintf(name, 50, "/gnome-mud/Connection%d/Pass=", i);
			wd->password = gnome_config_private_get_string(name);
			
			g_snprintf(name, 50, "/gnome-mud/Connection%d/Profile=", i);
			wd->profile = gnome_config_get_string(name);

			Profiles = g_list_append(Profiles, (gpointer) wd);

			i++;
		}
	}	
}

void profiledata_save(gchar *profilename, GtkCList *clist, gchar *partname)
{
	gchar name[50], tmp[50];
	gint  clist_count = get_size(clist), i;

	gchar const *vector_name[clist_count];
	gchar const *vector_value[clist_count];

	gchar *n, *v;
	
	g_snprintf(name, 50, "/gnome-mud/Profile: %s", profilename);

	for (i = 0; i < clist_count; i++)
	{
		gtk_clist_get_text(clist, i, 0, &n);
		gtk_clist_get_text(clist, i, 1, &v);

		vector_name[i] = n;
		vector_value[i] = v;
	}

	g_snprintf(tmp, 50, "%s/%s", name, partname);
	gnome_config_set_vector(tmp, i, vector_name);

	g_snprintf(tmp, 50, "%s/%sValue", name, partname);
	gnome_config_set_vector(tmp, i, vector_value);
}

PROFILE_DATA *profiledata_find(gchar *profile)
{
	GList *t;

	for (t = g_list_first(ProfilesData); t != NULL; t = t->next)
	{
		if (!g_strcasecmp(profile, ((PROFILE_DATA *)t->data)->name))
		{
			return ((PROFILE_DATA *) t->data);
		}
	}

	return NULL;
}

static void profilelist_new_input_cb(gchar *string, gpointer data)
{
	gchar *text[1];
	GList *t;
	
	if (string == NULL)
	{
		return;
	}
	
	for (t = g_list_first(ProfilesList_tmp); t != NULL; t = t->next)
	{
		if (!g_strcasecmp(string, (gchar *) t->data))
		{
			return;
		}
	}
	
	text[0] = string;
	gtk_clist_append(GTK_CLIST(clist), text);
	ProfilesList_tmp = g_list_append(ProfilesList_tmp, g_strdup(string));
}

static void profilelist_new_cb(GtkWidget *widget, gpointer data)
{
	gnome_request_dialog(FALSE, _("Name of new profile:"), "", 50, profilelist_new_input_cb, NULL, NULL);
}

static void profilelist_delete_cb(GtkWidget *widget, gpointer data)
{
	if (selected != -1)
	{
		gchar *name;

		gtk_clist_get_text(GTK_CLIST(clist), selected, 0, &name);

		if (!g_strcasecmp(name, "Default"))
		{
			g_message("can't delete default profile");
			return;
		}
		
		if (connections_find_by_profilename(name) == NULL)
		{
			GList *remove;

			remove = g_list_find_custom(ProfilesList_tmp, name, (GCompareFunc) g_list_compare_strings);
			if (remove != NULL)
			{
				ProfilesList_tmp = g_list_remove_link(ProfilesList_tmp, remove);
				g_list_free_1(remove);
			}
			
			gtk_clist_remove(GTK_CLIST(clist), selected);
		}
		else
		{
			g_message("this profile is in use");
		}
	}
}

static void profilelist_button_ok_cb(GtkWidget *widget, GtkWidget *label)
{
	gchar *text = NULL;
	

	g_list_free(ProfilesList);
	ProfilesList = ProfilesList_tmp;
	ProfilesList_tmp = NULL;

	gtk_clist_get_text(GTK_CLIST(clist), selected, 0, &text);
	gtk_label_set_text(GTK_LABEL(label), g_strdup(text));
}

static void profilelist_button_cancel_cb(GtkWidget *widget, gpointer data)
{
	g_list_free(ProfilesList_tmp);
}

static void profilelist_unselect_row_cb(GtkCList *clist, gint row, gint column, GdkEventButton *event, gpointer data)
{
	selected = -1;
}

static void profilelist_select_row_cb(GtkCList *clist, gint row, gint column, GdkEventButton *event, gpointer data)
{
	selected = row;
}

static void profilelist_clist_fill(gchar *profile_name, GtkCList *clist)
{
	gchar *text[1];
	
	text[0] = profile_name;
	gtk_clist_append(clist, text);
}

static void profilelist_dialog (GtkWidget *label)
{
	static GtkWidget *dialog;
	GtkWidget *dialog_vbox1;
	GtkWidget *scrolledwindow;
	GtkWidget *dialog_action_area;
	GtkWidget *button_ok;
	GtkWidget *button_cancel;
	GtkWidget *popup_menu;

	GnomeUIInfo profilelist_menu[] = {
		GNOMEUIINFO_ITEM_STOCK(N_("New"),		NULL,	profilelist_new_cb,		GNOME_STOCK_MENU_NEW),
		GNOMEUIINFO_ITEM_STOCK(N_("Delete"),	NULL,	profilelist_delete_cb,	GNOME_STOCK_MENU_CLOSE),
		GNOMEUIINFO_END
	};
	
	if (dialog != NULL)
	{
		gdk_window_raise(dialog->window);
		gdk_window_show(dialog->window);
		return;
	}

	selected = -1;
	
	dialog = gnome_dialog_new (NULL, NULL);
	gtk_object_set_data (GTK_OBJECT (dialog), "dialog", dialog);
	gtk_window_set_policy (GTK_WINDOW (dialog), FALSE, FALSE, FALSE);
	gtk_window_set_title (GTK_WINDOW(dialog), _("GNOME-Mud: Profilelist"));

	dialog_vbox1 = GNOME_DIALOG (dialog)->vbox;
	gtk_object_set_data (GTK_OBJECT (dialog), "dialog_vbox1", dialog_vbox1);
	gtk_widget_show (dialog_vbox1);

	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_ref (scrolledwindow);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "scrolledwindow", scrolledwindow, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (scrolledwindow);
	gtk_box_pack_start (GTK_BOX (dialog_vbox1), scrolledwindow, TRUE, TRUE, 0);
	gtk_widget_set_usize (scrolledwindow, -2, 100);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	popup_menu = gnome_popup_menu_new(profilelist_menu);
	
	clist = gtk_clist_new (1);
	gtk_widget_ref (clist);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "clist", clist, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (clist);
	gtk_container_add (GTK_CONTAINER (scrolledwindow), clist);
	gtk_clist_set_column_width (GTK_CLIST (clist), 0, 80);
	gtk_clist_column_titles_hide (GTK_CLIST (clist));
	gtk_clist_set_shadow_type (GTK_CLIST (clist), GTK_SHADOW_ETCHED_IN);
	ProfilesList_tmp = g_list_copy(ProfilesList);
	g_list_foreach(ProfilesList_tmp, (GFunc) profilelist_clist_fill, (gpointer) clist);
	gtk_signal_connect(GTK_OBJECT(clist), "select-row", GTK_SIGNAL_FUNC(profilelist_select_row_cb), NULL);
	gtk_signal_connect(GTK_OBJECT(clist), "unselect-row", GTK_SIGNAL_FUNC(profilelist_unselect_row_cb), NULL);
	
	dialog_action_area = GNOME_DIALOG (dialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (dialog), "dialog_action_area", dialog_action_area);
	gtk_widget_show (dialog_action_area);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog_action_area), 8);

	gnome_dialog_append_button (GNOME_DIALOG (dialog), GNOME_STOCK_BUTTON_OK);
	button_ok = GTK_WIDGET (g_list_last (GNOME_DIALOG (dialog)->buttons)->data);
	gtk_widget_ref (button_ok);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "button_ok", button_ok, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (button_ok);
	GTK_WIDGET_SET_FLAGS (button_ok, GTK_CAN_DEFAULT);
	gnome_dialog_append_button (GNOME_DIALOG (dialog), GNOME_STOCK_BUTTON_CANCEL);
	
	button_cancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (dialog)->buttons)->data);
	gtk_widget_ref (button_cancel);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "button_cancel", button_cancel, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (button_cancel);
	GTK_WIDGET_SET_FLAGS (button_cancel, GTK_CAN_DEFAULT);

	gtk_signal_connect(GTK_OBJECT(button_ok), "clicked", GTK_SIGNAL_FUNC(profilelist_button_ok_cb), (gpointer) label);
	gtk_signal_connect_object(GTK_OBJECT(button_ok), "clicked", gtk_widget_destroy, (gpointer) dialog);
	gtk_signal_connect(GTK_OBJECT(button_cancel), "clicked", GTK_SIGNAL_FUNC(profilelist_button_cancel_cb), (gpointer) clist);
	gtk_signal_connect_object(GTK_OBJECT(button_cancel), "clicked", gtk_widget_destroy, (gpointer) dialog);
	gtk_signal_connect(GTK_OBJECT(dialog), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &dialog);
	
	gtk_widget_show(dialog);

	gnome_popup_menu_attach(popup_menu, clist, NULL);
}

WIZARD_DATA2 *connections_find(gchar *name, gchar *character)
{
	GList *t;

	for (t = g_list_first(Profiles); t != NULL; t = t->next)
	{
		if (!g_strcasecmp(name, ((WIZARD_DATA2 *)t->data)->name) && !g_strcasecmp(character, ((WIZARD_DATA2 *)t->data)->playername))
		{
			return ((WIZARD_DATA2 *)t->data);
		}
	}

	return NULL;
}

WIZARD_DATA2 *connections_find_by_profilename(gchar *name)
{
	GList *t;

	for (t = g_list_first(Profiles); t != NULL; t = t->next)
	{
		if (!g_strcasecmp(name, ((WIZARD_DATA2 *)t->data)->profile))
		{
			return ((WIZARD_DATA2 *)t->data);
		}
	}

	return NULL;
}

static void connections_entry_info_mud_port_cb(GtkEditable *editable, gchar *new_text, gint new_text_length, gint position, gpointer user_data)
{
	if (!isdigit(new_text[0]) || strlen(gtk_entry_get_text(GTK_ENTRY(editable))) > 4)
	{
		gtk_signal_emit_stop_by_name(GTK_OBJECT(editable), "insert-text");
	}
}

static void connections_button_info_prof_cb(GtkButton *button, GtkWidget *label)
{
	profilelist_dialog(label);
}

static void connections_clear()
{
	gtk_entry_set_text(GTK_ENTRY(entry_info_mud_title), 		"");
	gtk_entry_set_text(GTK_ENTRY(entry_info_mud_host),			"");
	gtk_entry_set_text(GTK_ENTRY(entry_info_mud_port),			"");
	gtk_entry_set_text(GTK_ENTRY(entry_info_char_character),	"");
	gtk_entry_set_text(GTK_ENTRY(entry_info_char_password),		"");

	gtk_label_set_text(GTK_LABEL(label_info_prof), _("Default"));
}

static void connections_button_info_apply_cb(GtkButton *button, GtkCList *clist)
{
	gchar *text[2];
	gchar *label;
	gchar *name	 = gtk_entry_get_text(GTK_ENTRY(entry_info_mud_title));
	gchar *charc = gtk_entry_get_text(GTK_ENTRY(entry_info_char_character));
	WIZARD_DATA2 *wd = connections_find(name, charc);

	gboolean new_connection = TRUE;

	if (strlen(name) < 1)
	{
		//no connection name
		return;
	}
	
	// FIXME, need some more error checking here
	if (wd == NULL)
	{
		wd = g_malloc0(sizeof(WIZARD_DATA2));
	}
	else
	{
		g_free(wd->name);
		g_free(wd->hostname);
		g_free(wd->port);
		g_free(wd->playername);
		g_free(wd->password);

		new_connection = FALSE;
	}

	wd->name 		= g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_info_mud_title)));
	wd->hostname	= g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_info_mud_host)));
	wd->port		= g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_info_mud_port)));
	wd->playername	= g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_info_char_character)));
	wd->password	= g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_info_char_password)));

	gtk_label_get(GTK_LABEL(label_info_prof), &label);
	wd->profile		= g_strdup(label);

	if (new_connection)
	{
		text[0] = name;
		text[1] = charc;
		gtk_clist_select_row(clist, gtk_clist_append(clist, text), 0);

		Profiles = g_list_append(Profiles, (gpointer) wd);
	}
}

static void connections_button_info_cancel_cb(GtkButton *button, gpointer data)
{
	connections_clear();
}

static void connections_button_connect_cb(GtkButton *button, gpointer data)
{
	if (connection_selected != -1)
	{
		gchar *name, *player;
		WIZARD_DATA2 *wd;
		CONNECTION_DATA *cd;

		gtk_clist_get_text(GTK_CLIST(main_clist), connection_selected, 0, &name);
		gtk_clist_get_text(GTK_CLIST(main_clist), connection_selected, 1, &player);

		wd = connections_find(name, player);

		cd = make_connection(wd->hostname, wd->port);

		if (cd && cd->connected)
		{
			if (strlen(wd->playername) > 0 && strlen(wd->password) > 0)
			{
				connection_send(cd, wd->playername);
				connection_send(cd, "\n");
				connection_send(cd, wd->password);
				connection_send(cd, "\n");
			}

			gtk_widget_destroy((GtkWidget *) data);
		}
	}
}

static void connections_delete_menu_cb(GtkButton *button, gpointer data)
{
	if (connection_selected != -1)
	{
		WIZARD_DATA2 *wd;
		gchar *name, *character;
		gtk_clist_get_text(GTK_CLIST(main_clist), connection_selected, 0, &name);
		gtk_clist_get_text(GTK_CLIST(main_clist), connection_selected, 1, &character);
	
		wd = connections_find(name, character);
		Profiles = g_list_remove(Profiles, (gpointer) wd);

		gtk_clist_remove(GTK_CLIST(main_clist), connection_selected);
	}
}

static void connections_clist_fill(WIZARD_DATA2 *wd, GtkCList *clist)
{
	gchar *text[2];
	
	text[0] = wd->name;
	text[1] = wd->playername;

	gtk_clist_append(clist, text);
}

static void connections_select_row_cb(GtkCList *clist, gint row, gint column, GdkEventButton *event, gpointer data)
{
	gchar *name, *character;
	WIZARD_DATA2 *wd;

	gtk_clist_get_text(clist, row, 0, &name);
	gtk_clist_get_text(clist, row, 1, &character);

	connection_selected = row;

	wd = connections_find(name, character);
	if (wd != NULL)
	{
		gtk_entry_set_text(GTK_ENTRY(entry_info_mud_title), 		wd->name);
		gtk_entry_set_text(GTK_ENTRY(entry_info_mud_host),  		wd->hostname);
		gtk_entry_set_text(GTK_ENTRY(entry_info_mud_port),			wd->port);
		gtk_entry_set_text(GTK_ENTRY(entry_info_char_character),	wd->playername);
		gtk_entry_set_text(GTK_ENTRY(entry_info_char_password),		wd->password);

		gtk_label_set_text(GTK_LABEL(label_info_prof), wd->profile);
	}
}

static void connections_unselect_row_cb(GtkCList *clist, gint row, gint column, GdkEventButton *event, gpointer data)
{
	connection_selected = -1;
}

void window_profiles(void)
{
	static GtkWidget *dialog;
	GtkWidget *dialog_vbox1;
	GtkWidget *main_hbox;
	GtkWidget *scrolledwindow;
	GtkWidget *label1;
	GtkWidget *label2;
	GtkWidget *main_vbox;
	GtkWidget *frame_info_mud;
	GtkWidget *table_info_mud;
	GtkWidget *label_info_mud_title;
	GtkWidget *label_info_mud_host;
	GtkWidget *label_info_mud_port;
	GtkWidget *frame_info_char;
	GtkWidget *table2;
	GtkWidget *label_info_char_character;
	GtkWidget *label_info_char_password;
	GtkWidget *frame_info_prof;
	GtkWidget *table3;
	GtkWidget *button_info_prof;
	GtkWidget *table_buttons;
	GtkWidget *button_info_apply;
	GtkWidget *button_info_cancel;
	GtkWidget *dialog_action_area;
	GtkWidget *button_connect;
	GtkWidget *button_cancel;
	GtkWidget *extra_menu;

	static GnomeUIInfo rightbutton_menu[] = {
		GNOMEUIINFO_ITEM_STOCK(N_("Delete"),	NULL,	connections_delete_menu_cb,		GNOME_STOCK_MENU_CLOSE),
		GNOMEUIINFO_END
	};

	if (dialog != NULL)
	{
		gdk_window_raise(dialog->window);
		gdk_window_show(dialog->window);
		return;
	}
 
	connection_selected = -1;
	extra_menu = gnome_popup_menu_new(rightbutton_menu);
	
	dialog = gnome_dialog_new (NULL, NULL);
	gtk_object_set_data (GTK_OBJECT (dialog), "dialog", dialog);
	gtk_window_set_policy (GTK_WINDOW (dialog), FALSE, FALSE, FALSE);
	gtk_window_set_title(GTK_WINDOW(dialog), _("GNOME-Mud Connections"));
	
	dialog_vbox1 = GNOME_DIALOG (dialog)->vbox;
	gtk_object_set_data (GTK_OBJECT (dialog), "dialog_vbox1", dialog_vbox1);
	gtk_widget_show (dialog_vbox1);

	main_hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_ref (main_hbox);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "main_hbox", main_hbox, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_hbox);
	gtk_box_pack_start (GTK_BOX (dialog_vbox1), main_hbox, TRUE, TRUE, 0);

	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_ref (scrolledwindow);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "scrolledwindow", scrolledwindow, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (scrolledwindow);
	gtk_box_pack_start (GTK_BOX (main_hbox), scrolledwindow, FALSE, TRUE, 0);
	gtk_widget_set_usize (scrolledwindow, 200, -2);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	main_clist = gtk_clist_new (2);
	gtk_widget_ref (main_clist);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "main_clist", main_clist, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_clist);
	gtk_container_add (GTK_CONTAINER (scrolledwindow), main_clist);
	gtk_clist_set_column_width (GTK_CLIST (main_clist), 0, 98);
	gtk_clist_set_column_width (GTK_CLIST (main_clist), 1, 80);
	gtk_clist_column_titles_show (GTK_CLIST (main_clist));
	g_list_foreach(Profiles, (GFunc) connections_clist_fill, main_clist);
	gtk_signal_connect(GTK_OBJECT(main_clist), "select-row", GTK_SIGNAL_FUNC(connections_select_row_cb), NULL);
	gtk_signal_connect(GTK_OBJECT(main_clist), "unselect-row", GTK_SIGNAL_FUNC(connections_unselect_row_cb), NULL);
	
	label1 = gtk_label_new (_("Mud"));
	gtk_widget_ref (label1);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "label1", label1, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label1);
	gtk_clist_set_column_widget (GTK_CLIST (main_clist), 0, label1);

	label2 = gtk_label_new (_("Character"));
	gtk_widget_ref (label2);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "label2", label2, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label2);
	gtk_clist_set_column_widget (GTK_CLIST (main_clist), 1, label2);
	gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);

	main_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_ref (main_vbox);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "main_vbox", main_vbox, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_vbox);
	gtk_box_pack_start (GTK_BOX (main_hbox), main_vbox, TRUE, TRUE, 0);

	frame_info_mud = gtk_frame_new (_(" Mud Information "));
	gtk_widget_ref (frame_info_mud);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "frame_info_mud", frame_info_mud, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (frame_info_mud);
	gtk_box_pack_start (GTK_BOX (main_vbox), frame_info_mud, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame_info_mud), 4);

	table_info_mud = gtk_table_new (2, 4, FALSE);
	gtk_widget_ref (table_info_mud);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "table_info_mud", table_info_mud, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (table_info_mud);
	gtk_container_add (GTK_CONTAINER (frame_info_mud), table_info_mud);
	gtk_container_set_border_width (GTK_CONTAINER (table_info_mud), 3);
	gtk_table_set_row_spacings (GTK_TABLE (table_info_mud), 2);
	gtk_table_set_col_spacings (GTK_TABLE (table_info_mud), 3);

	label_info_mud_title = gtk_label_new (_("Title:"));
	gtk_widget_ref (label_info_mud_title);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "label_info_mud_title", label_info_mud_title, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label_info_mud_title);
	gtk_table_attach (GTK_TABLE (table_info_mud), label_info_mud_title, 0, 1, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label_info_mud_title), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label_info_mud_title), 4, 0);

	label_info_mud_host = gtk_label_new (_("Host:"));
	gtk_widget_ref (label_info_mud_host);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "label_info_mud_host", label_info_mud_host, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label_info_mud_host);
	gtk_table_attach (GTK_TABLE (table_info_mud), label_info_mud_host, 0, 1, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label_info_mud_host), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label_info_mud_host), 4, 0);

	entry_info_mud_title = gtk_entry_new ();
	gtk_widget_ref (entry_info_mud_title);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "entry_info_mud_title", entry_info_mud_title, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (entry_info_mud_title);
	gtk_table_attach (GTK_TABLE (table_info_mud), entry_info_mud_title, 1, 4, 0, 1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	entry_info_mud_host = gtk_entry_new ();
	gtk_widget_ref (entry_info_mud_host);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "entry_info_mud_host", entry_info_mud_host, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (entry_info_mud_host);
	gtk_table_attach (GTK_TABLE (table_info_mud), entry_info_mud_host, 1, 2, 1, 2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label_info_mud_port = gtk_label_new (_("Port:"));
	gtk_widget_ref (label_info_mud_port);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "label_info_mud_port", label_info_mud_port, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label_info_mud_port);
	gtk_table_attach (GTK_TABLE (table_info_mud), label_info_mud_port, 2, 3, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label_info_mud_port), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label_info_mud_port), 3, 0);

	entry_info_mud_port = gtk_entry_new ();
	gtk_widget_ref (entry_info_mud_port);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "entry_info_mud_port", entry_info_mud_port, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (entry_info_mud_port);
	gtk_table_attach (GTK_TABLE (table_info_mud), entry_info_mud_port, 3, 4, 1, 2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_widget_set_usize (entry_info_mud_port, 40, -2);
	gtk_signal_connect(GTK_OBJECT(entry_info_mud_port), "insert-text", GTK_SIGNAL_FUNC(connections_entry_info_mud_port_cb), NULL);
			
	frame_info_char = gtk_frame_new (_(" Character Information "));
	gtk_widget_ref (frame_info_char);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "frame_info_char", frame_info_char, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (frame_info_char);
	gtk_box_pack_start (GTK_BOX (main_vbox), frame_info_char, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame_info_char), 4);

	table2 = gtk_table_new (2, 2, FALSE);
	gtk_widget_ref (table2);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "table2", table2, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (table2);
	gtk_container_add (GTK_CONTAINER (frame_info_char), table2);
	gtk_container_set_border_width (GTK_CONTAINER (table2), 3);
	gtk_table_set_row_spacings (GTK_TABLE (table2), 2);
	gtk_table_set_col_spacings (GTK_TABLE (table2), 3);

	label_info_char_character = gtk_label_new (_("Character:"));
	gtk_widget_ref (label_info_char_character);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "label_info_char_character", label_info_char_character, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label_info_char_character);
	gtk_table_attach (GTK_TABLE (table2), label_info_char_character, 0, 1, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label_info_char_character), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label_info_char_character), 3, 0);

	label_info_char_password = gtk_label_new (_("Password:"));
	gtk_widget_ref (label_info_char_password);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "label_info_char_password", label_info_char_password, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label_info_char_password);
	gtk_table_attach (GTK_TABLE (table2), label_info_char_password, 0, 1, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label_info_char_password), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label_info_char_password), 3, 0);

	entry_info_char_character = gtk_entry_new ();
	gtk_widget_ref (entry_info_char_character);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "entry_info_char_character", entry_info_char_character, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (entry_info_char_character);
	gtk_table_attach (GTK_TABLE (table2), entry_info_char_character, 1, 2, 0, 1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	entry_info_char_password = gtk_entry_new ();
	gtk_widget_ref (entry_info_char_password);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "entry_info_char_password", entry_info_char_password, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (entry_info_char_password);
	gtk_table_attach (GTK_TABLE (table2), entry_info_char_password, 1, 2, 1, 2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_visibility(GTK_ENTRY(entry_info_char_password), FALSE);

	frame_info_prof = gtk_frame_new (_(" Profile Information "));
	gtk_widget_ref (frame_info_prof);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "frame_info_prof", frame_info_prof, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (frame_info_prof);
	gtk_box_pack_start (GTK_BOX (main_vbox), frame_info_prof, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame_info_prof), 4);

	table3 = gtk_table_new (1, 2, FALSE);
	gtk_widget_ref (table3);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "table3", table3, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (table3);
	gtk_container_add (GTK_CONTAINER (frame_info_prof), table3);
	gtk_container_set_border_width (GTK_CONTAINER (table3), 3);
	gtk_table_set_row_spacings (GTK_TABLE (table3), 2);
	gtk_table_set_col_spacings (GTK_TABLE (table3), 3);

	label_info_prof = gtk_label_new (_("Default"));
	gtk_widget_ref (label_info_prof);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "label_info_prof", label_info_prof, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label_info_prof);
	gtk_table_attach (GTK_TABLE (table3), label_info_prof, 0, 1, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_widget_set_usize (label_info_prof, 152, -2);
	gtk_label_set_justify (GTK_LABEL (label_info_prof), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label_info_prof), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label_info_prof), 3, 0);

	button_info_prof = gtk_button_new_with_label (_("Select Profile"));
	gtk_widget_ref (button_info_prof);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "button_info_prof", button_info_prof, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (button_info_prof);
	gtk_table_attach (GTK_TABLE (table3), button_info_prof, 1, 2, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_signal_connect(GTK_OBJECT(button_info_prof), "clicked", GTK_SIGNAL_FUNC(connections_button_info_prof_cb), (gpointer) label_info_prof);

	table_buttons = gtk_table_new (1, 3, FALSE);
	gtk_widget_ref (table_buttons);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "table_buttons", table_buttons, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (table_buttons);
	gtk_box_pack_start (GTK_BOX (main_vbox), table_buttons, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (table_buttons), 5);
	gtk_table_set_row_spacings (GTK_TABLE (table_buttons), 2);
	gtk_table_set_col_spacings (GTK_TABLE (table_buttons), 3);

	button_info_apply = gnome_stock_button (GNOME_STOCK_BUTTON_APPLY);
	gtk_widget_ref (button_info_apply);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "button_info_apply", button_info_apply, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (button_info_apply);
	gtk_table_attach (GTK_TABLE (table_buttons), button_info_apply, 0, 1, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 5, 0);
	gtk_signal_connect(GTK_OBJECT(button_info_apply), "clicked", GTK_SIGNAL_FUNC(connections_button_info_apply_cb), (gpointer) main_clist);

	button_info_cancel = gnome_stock_button (GNOME_STOCK_BUTTON_CANCEL);
	gtk_widget_ref (button_info_cancel);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "button_info_cancel", button_info_cancel, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (button_info_cancel);
	gtk_table_attach (GTK_TABLE (table_buttons), button_info_cancel, 1, 2, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_signal_connect(GTK_OBJECT(button_info_cancel), "clicked", GTK_SIGNAL_FUNC(connections_button_info_cancel_cb), NULL);

	dialog_action_area = GNOME_DIALOG (dialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (dialog), "dialog_action_area", dialog_action_area);
	gtk_widget_show (dialog_action_area);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog_action_area), 8);
 
	gnome_dialog_append_button_with_pixmap (GNOME_DIALOG (dialog), _("Connect"), GNOME_STOCK_PIXMAP_JUMP_TO);
	button_connect = GTK_WIDGET (g_list_last (GNOME_DIALOG (dialog)->buttons)->data);
	gtk_widget_ref (button_connect);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "button_connect", button_connect, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (button_connect);
	GTK_WIDGET_SET_FLAGS (button_connect, GTK_CAN_DEFAULT);
	gtk_signal_connect(GTK_OBJECT(button_connect), "clicked", GTK_SIGNAL_FUNC(connections_button_connect_cb), (gpointer) dialog);

	gnome_dialog_append_button (GNOME_DIALOG (dialog), GNOME_STOCK_BUTTON_CLOSE);
	button_cancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (dialog)->buttons)->data);
	gtk_widget_ref (button_cancel);
	gtk_object_set_data_full (GTK_OBJECT (dialog), "button_cancel", button_cancel, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (button_cancel);
	GTK_WIDGET_SET_FLAGS (button_cancel, GTK_CAN_DEFAULT);
  
	gtk_signal_connect_object(GTK_OBJECT(button_cancel), "clicked", gtk_widget_destroy, (gpointer) dialog);
	gtk_signal_connect(GTK_OBJECT(dialog), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &dialog);

	gtk_widget_show(dialog);

	gnome_popup_menu_attach(extra_menu, main_clist, NULL);
}
