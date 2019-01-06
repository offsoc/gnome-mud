/* mud-connections.c
 *
 * Copyright 2008-2009 Les Harris <lharris@gnome.org>
 * Copyright 2018-2019 Mart Raudsepp <leio@gentoo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/* TODO: Review includes.. */
#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <string.h>

#include "gnome-mud.h"
#include "mud-connections.h"
#include "mud-mud.h"
#include "mud-character.h"
#include "mud-window.h"
#include "mud-connection-view.h"
#include "mud-profile.h"
#include "utils.h"

struct _MudConnections
{
  GObject parent_instance;

  MudWindow *parent_window;

  // Main Window
  GtkWidget *winwidget;

  GtkWidget *window;
  GtkWidget *iconview;
  GtkWidget *popup;

  GtkTreeModel *icon_model;
  GtkTreeModel *profile_model;

  GtkWidget *qconnect_host_entry;
  GtkWidget *qconnect_port_entry;
  GtkWidget *qconnect_connect_button;

  // Properties Dialog
  GtkWidget *properties_window;
  GtkWidget *name_entry;
  GtkWidget *host_entry;
  GtkWidget *port_entry;

  GtkWidget *icon_button;
  GtkWidget *icon_image;

  GtkWidget *profile_combo;
  GtkCellRenderer *profile_combo_renderer;

  GtkWidget *character_name_entry;
  GtkWidget *logon_textview;

  gboolean changed;

  // Iconview Dialog
  GtkWidget *icon_dialog;
  GtkWidget *icon_dialog_view;
  GtkTreeModel *icon_dialog_view_model;
  GtkWidget *icon_dialog_chooser;
  gchar *icon_current;
};

enum
{
  MODEL_COLUMN_VISIBLE_NAME,
  MODEL_COLUMN_PIXBUF,
  MODEL_COLUMN_MUD_CHARACTER,
  MODEL_COLUMN_N
};

enum
{
  MODEL_ICON_NAME,
  MODEL_ICON_PIXBUF,
  MODEL_ICON_N
};

enum
{
  MODEL_PROFILE_NAME,
  MODEL_PROFILE_OBJECT, /* TODO: Maybe keep profile id in string here? */
  MODEL_PROFILE_N
};

enum
{
  PROP_0,
  PROP_PARENT_WINDOW,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

G_DEFINE_TYPE(MudConnections, mud_connections, G_TYPE_OBJECT);

/* Callbacks */
/* FIXME: Remove most forward decls, we can structure things to not need them */
static gint mud_connections_close_cb (GtkWidget *widget,
                                      MudConnections *self);
static void mud_connections_connect_cb (GtkWidget *widget,
                                        MudConnections *self);
static void mud_connections_add_cb (GtkWidget *widget,
                                    MudConnections *self);
static void mud_connections_delete_cb (GtkWidget *widget,
                                       MudConnections *self);
static void mud_connections_properties_cb (GtkWidget *widget,
                                           MudConnections *self);
static void mud_connections_property_cancel_cb (GtkWidget *widget,
                                                MudConnections *self);
static void mud_connections_property_save_cb (GtkWidget *widget,
                                              MudConnections *self);
static void mud_connections_property_icon_cb (GtkWidget *widget,
                                              MudConnections *self);
static void mud_connections_qconnect_cb (GtkWidget *widget,
                                         MudConnections *self);
static gboolean mud_connections_property_changed_cb (GtkWidget *widget,
                                                     GdkEventKey *event,
                                                     MudConnections *self);
static void mud_connections_property_combo_changed_cb (GtkWidget *widget,
                                                       MudConnections *self);
static gboolean mud_connections_property_delete_cb (GtkWidget *widget,
                                                    GdkEvent *event,
                                                    MudConnections *self);
static gint mud_connections_compare_func (GtkTreeModel *model,
                                          GtkTreeIter *a,
                                          GtkTreeIter *b,
                                          gpointer user_data);

/* Private Methods */
static void mud_connections_populate_iconview (MudConnections *self);
static void mud_connections_show_properties (MudConnections *self,
                                             MudCharacter *character);
static gboolean mud_connections_property_save (MudConnections *self);
static gint mud_connections_property_confirm (void);
static void mud_connections_property_populate_profiles (MudConnections *self);
static void mud_connections_property_set_profile (MudConnections *self,
                                                  const MudProfile *profile);
static void mud_connections_refresh_iconview (MudConnections *self);
static gboolean mud_connections_delete_confirm (gchar *name);
static gboolean mud_connections_button_press_cb (GtkWidget *widget,
                                                 GdkEventButton *event,
                                                 MudConnections *self);
static void mud_connections_popup (MudConnections *self,
                                   GdkEventButton *event);

/* IconDialog Prototypes */
static void mud_connections_show_icon_dialog (MudConnections *self);

/* IconDialog callbacks */
static void mud_connections_icon_fileset_cb (GtkFileChooserButton *widget,
                                             MudConnections *self);
static void mud_connections_icon_select_cb (GtkIconView *view,
                                            MudConnections *self);
void mud_connections_iconview_activate_cb (GtkIconView *iconview,
                                           GtkTreePath *path,
                                           MudConnections *self);

// MudConnections Private Methods
static gint
mud_connections_compare_func (GtkTreeModel *model,
                              GtkTreeIter  *a,
                              GtkTreeIter  *b,
                              gpointer      user_data)
{
  /* TODO: Simplify */
  gchar *item_one, *item_two;
  gboolean item_one_haschar, item_two_haschar;
  gint ret = 0;

  gtk_tree_model_get (model,
                      a,
                      MODEL_COLUMN_VISIBLE_NAME, &item_one,
                      -1);

  gtk_tree_model_get (model,
                      b,
                      MODEL_COLUMN_VISIBLE_NAME, &item_two,
                      -1);

  item_one_haschar = (g_strrstr (item_one, "\n") != NULL);
  item_two_haschar = (g_strrstr (item_two, "\n") != NULL);

  if (item_one_haschar && item_two_haschar)
    {
      gchar **item_onev, **item_twov;

      item_onev = g_strsplit (item_one, "\n", -1);
      item_twov = g_strsplit (item_two, "\n", -1);

      ret = strcmp (item_onev[1], item_twov[1]);

      g_strfreev (item_onev);
      g_strfreev (item_twov);
    } 
  else if (item_one_haschar && !item_two_haschar)
    ret = -1;
  else if (!item_one_haschar && item_two_haschar)
    ret = 1;
  else
    ret = strcmp (item_one, item_two);

  g_free (item_one);
  g_free (item_two);

  return ret;
}

static gint
mud_connections_close_cb (GtkWidget      *widget,
                          MudConnections *self)
{
  g_object_unref (self);

  return TRUE;
}

static void
mud_connections_connect_cb (GtkWidget      *widget,
                            MudConnections *self)
{
  GtkTreeIter iter;
  gchar *mud_name, *host, *connect_string;
  gint port;
  MudConnectionView *view;
  MudProfile *profile;
  MudCharacter *character;
  MudMud *mud;
  GList *selected = gtk_icon_view_get_selected_items (GTK_ICON_VIEW(self->iconview));

  if (g_list_length (selected) == 0)
    return;

  gtk_tree_model_get_iter (self->icon_model, &iter,
                           (GtkTreePath *)selected->data);
  gtk_tree_model_get (self->icon_model, &iter,
                      MODEL_COLUMN_MUD_CHARACTER, &character,
                      -1);

  g_object_get (character,
                "mud", &mud,
                "profile", &profile,
                "connect-string", &connect_string,
                NULL);
  g_object_get (mud,
                "name", &mud_name,
                "hostname", &host,
                "port", &port,
                NULL);

  /* TODO: Bind these instead? */
  view = g_object_new (MUD_TYPE_CONNECTION_VIEW,
                       "hostname", host,
                       "port", port,
                       "profile", profile,
                       "mud-name", mud_name,
                       "connect-string", (connect_string && strlen (connect_string) != 0) ? connect_string : NULL,
                       "window", self->parent_window,
                       NULL);

  mud_window_add_connection_view (self->parent_window, G_OBJECT(view), mud_name);
  mud_window_profile_menu_set_active (self->parent_window, profile);

  g_free (host);
  g_object_unref (profile); /* TODO: was take_object? */
  g_free (connect_string);

  g_list_foreach (selected, (GFunc)gtk_tree_path_free, NULL);
  g_list_free (selected);

  gtk_widget_destroy (self->window);
}

static void
mud_connections_qconnect_cb (GtkWidget      *widget,
                             MudConnections *self)
{
  MudConnectionView *view;
  const gchar *host = gtk_entry_get_text (GTK_ENTRY (self->qconnect_host_entry));
  gint port = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (self->qconnect_port_entry));

  if (strlen (host) != 0)
    {
      view = g_object_new (MUD_TYPE_CONNECTION_VIEW,
                           "hostname", host,
                           "port", port,
                           "profile-name", "Default",
                           "mud-name", (gchar *)host,
                           "connect-string", NULL,
                           "window", self->parent_window,
                           NULL);
      mud_window_add_connection_view (self->parent_window, G_OBJECT(view), (gchar *)host);

      gtk_widget_destroy(self->window);
    }
}

static void
mud_connections_add_cb (GtkWidget      *widget,
                        MudConnections *self)
{
  mud_connections_show_properties (self, NULL);
}

static void
mud_connections_delete_cb (GtkWidget      *widget,
                           MudConnections *self)
{
  GtkTreeIter iter;
  gchar *char_name;
  MudCharacter *character;
  MudMud *mud;
  GList *selected = gtk_icon_view_get_selected_items (GTK_ICON_VIEW (self->iconview));

  if (g_list_length (selected) == 0)
    return;

  gtk_tree_model_get_iter (self->icon_model, &iter,
                           (GtkTreePath *)selected->data);
  gtk_tree_model_get (self->icon_model, &iter,
                      MODEL_COLUMN_MUD_CHARACTER, &character,
                      -1);

  g_object_get (character,
                "mud", &mud,
                "name", &char_name,
                NULL);

  if (mud_connections_delete_confirm (char_name)) /* TODO: Show both mud and charname in confirmation dialog? */
    {
      mud_profile_manager_delete_character (self->parent_window->profile_manager, character);
      /* TODO: Might need to manually repopulate iconview until GSettings aren't hooked up */
    }

  g_free (char_name);

  g_list_foreach (selected, (GFunc)gtk_tree_path_free, NULL);
  g_list_free (selected);
}

static void
mud_connections_properties_cb (GtkWidget      *widget,
                               MudConnections *self)
{
  GList *selected = gtk_icon_view_get_selected_items (GTK_ICON_VIEW (self->iconview));
  GtkTreeIter iter;
  MudCharacter *character;

  if (g_list_length (selected) == 0)
    return;

  gtk_tree_model_get_iter (self->icon_model, &iter,
                           (GtkTreePath *)selected->data);
  gtk_tree_model_get (self->icon_model, &iter,
                      MODEL_COLUMN_MUD_CHARACTER, &character,
                      -1);

  mud_connections_show_properties (self, character);
}

static void
add_character_to_iconmodel (MudCharacter *character,
                            GtkListStore *model)
{
  GdkPixbuf *icon;
  GtkTreeIter iter;
  MudMud *mud;
  gchar *visible_name, *char_name;
  const gchar *mud_name;

  g_object_get (character,
                "mud", &mud,
                "name", &char_name,
                NULL);
  mud_name = mud_mud_get_name (mud);

  /* TODO: Reimplement icon support */
#if 0
  if (icon_name && strcmp (icon_name, "gnome-mud") != 0)
    {
      icon = gdk_pixbuf_new_from_file_at_size (icon_name,
                                               48, 48, NULL);
    }
  else
    {
      icon = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                       "gnome-mud", 48, 0, NULL);
    }
#endif
  icon = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                   "gnome-mud", 48, 0, NULL);

  visible_name = g_strdup_printf ("%s\n%s", char_name, mud_name);

  gtk_list_store_append (model, &iter);
  /* TODO: Handle character going away from under us, to support removal outside this dialog */
  gtk_list_store_set (model, &iter,
                      MODEL_COLUMN_VISIBLE_NAME, visible_name,
                      MODEL_COLUMN_PIXBUF, icon,
                      MODEL_COLUMN_MUD_CHARACTER, character,
                      -1);
  g_free (visible_name);
  g_free (char_name);
  g_object_unref (icon);
}

static void
mud_connections_populate_iconview (MudConnections *self)
{
  /* TODO: Inline again? */
  g_sequence_foreach (mud_profile_manager_get_characters (self->parent_window->profile_manager),
                      (GFunc)add_character_to_iconmodel,
                      GTK_LIST_STORE (self->icon_model));
}

#if 0
#warning Reimplement iconview refresh
static void
mud_connections_refresh_iconview(MudConnections *self)
{
    GConfClient *client = gconf_client_get_default();

    gtk_list_store_clear(GTK_LIST_STORE(self->icon_model));

    gconf_client_suggest_sync(client, NULL);

    mud_connections_populate_iconview(conn);
}

void
mud_connections_gconf_notify_cb(GConfClient *client, guint cnxn_id,
				GConfEntry *entry, gpointer *self)
{
    mud_connections_refresh_iconview((MudConnections *)conn);
}
#endif

void
mud_connections_iconview_activate_cb (GtkIconView    *iconview,
                                      GtkTreePath    *path,
                                      MudConnections *self)
{
  mud_connections_connect_cb (GTK_WIDGET (iconview), self);
}


static gboolean
mud_connections_button_press_cb (GtkWidget      *widget,
                                 GdkEventButton *event,
                                 MudConnections *self)
{
  if ((event->button == 3) &&
      !(event->state & (GDK_SHIFT_MASK |
                        GDK_CONTROL_MASK |
                        GDK_MOD1_MASK)))
    {
      mud_connections_popup (self, event);
      return TRUE;
    }

  return FALSE;
}

static void
mud_connections_popup (MudConnections *self,
                       GdkEventButton *event)
{
  GtkBuilder *builder;
  GtkWidget *popup;
  GList *selected = gtk_icon_view_get_selected_items (GTK_ICON_VIEW (self->iconview));

  if (g_list_length(selected) == 0)
    return;

  builder = gtk_builder_new_from_resource ("/org/gnome/MUD/muds.ui");

  popup = GTK_WIDGET (gtk_builder_get_object (builder, "popupmenu"));

  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "connect")),
                    "activate", G_CALLBACK (mud_connections_connect_cb),
                    self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "add")),
                    "activate", G_CALLBACK (mud_connections_add_cb),
                    self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "delete")),
                    "activate", G_CALLBACK (mud_connections_delete_cb),
                    self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "properties")),
                    "activate", G_CALLBACK (mud_connections_properties_cb),
                    self);

  g_object_unref (builder);

  gtk_menu_attach_to_widget (GTK_MENU(popup), self->iconview, NULL);
  gtk_widget_show_all (popup);
  gtk_menu_popup (GTK_MENU (popup), NULL, NULL, NULL, NULL,
                  event ? event->button : 0,
                  event ? event->time : gtk_get_current_event_time ());

  g_list_foreach (selected, (GFunc)gtk_tree_path_free, NULL);
  g_list_free (selected);
}

static gboolean
mud_connections_delete_confirm (gchar *name)
{
  GtkBuilder *builder;
  GtkWidget *dialog;
  GtkWidget *label;
  gint result;
  gchar *message;
  gchar *title;

  builder = gtk_builder_new_from_resource ("/org/gnome/MUD/muds.ui");

  dialog = GTK_WIDGET (gtk_builder_get_object (builder, "mudviewdelconfirm"));
  label = GTK_WIDGET (gtk_builder_get_object (builder, "message"));
  g_object_unref (builder);

  message = g_strdup_printf (_("Are you sure you want to delete %s?"), name);
  title = g_strdup_printf (_("Delete %s?"), name);

  gtk_label_set_text (GTK_LABEL (label), message);
  gtk_window_set_title (GTK_WINDOW (dialog), title);

  g_free (message);
  g_free (title);

  result = gtk_dialog_run (GTK_DIALOG(dialog));
  gtk_widget_destroy (dialog);

  return (result == GTK_RESPONSE_YES);
}

static void
mud_connections_show_properties (MudConnections *self,
                                 MudCharacter   *character)
{
  GtkBuilder *builder;
  GtkTextBuffer *buffer;
  MudMud *mud;
  const gchar *icon_name, *connect_string;

  builder = gtk_builder_new_from_resource ("/org/gnome/MUD/muds.ui");

  self->properties_window = GTK_WIDGET (gtk_builder_get_object (builder, "mudviewproperties"));
  self->name_entry     = GTK_WIDGET (gtk_builder_get_object (builder, "name_entry"));
  self->host_entry     = GTK_WIDGET (gtk_builder_get_object (builder, "host_entry"));
  self->port_entry     = GTK_WIDGET (gtk_builder_get_object (builder, "port_entry"));
  self->icon_button    = GTK_WIDGET (gtk_builder_get_object (builder, "icon_button"));
  self->icon_image     = GTK_WIDGET (gtk_builder_get_object (builder, "icon_image"));
  self->profile_combo  = GTK_WIDGET (gtk_builder_get_object (builder, "profile_combo"));
  self->character_name_entry = GTK_WIDGET (gtk_builder_get_object (builder, "character_name_entry"));
  self->logon_textview = GTK_WIDGET (gtk_builder_get_object (builder, "character_logon_textview"));

  g_clear_pointer (&self->icon_current, g_free);

  mud_connections_property_populate_profiles (self);
  gtk_combo_box_set_model (GTK_COMBO_BOX (self->profile_combo),
                           self->profile_model);
  g_object_unref (self->profile_model);

  self->profile_combo_renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (self->profile_combo),
                              self->profile_combo_renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (self->profile_combo),
                                  self->profile_combo_renderer, "text", 0, NULL);

  g_signal_connect (self->properties_window, "delete-event",
                    G_CALLBACK (mud_connections_property_delete_cb), self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "cancel_button")),
                    "clicked",
                    G_CALLBACK (mud_connections_property_cancel_cb),
                    self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "save_button")),
                    "clicked",
                    G_CALLBACK (mud_connections_property_save_cb),
                    self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object(builder, "icon_button")),
                    "clicked",
                    G_CALLBACK (mud_connections_property_icon_cb),
                    self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "name_entry")),
                    "key-press-event",
                    G_CALLBACK(mud_connections_property_changed_cb),
                    self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "host_entry")),
                    "key-press-event",
                    G_CALLBACK (mud_connections_property_changed_cb),
                    self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "port_entry")),
                    "key-press-event",
                    G_CALLBACK (mud_connections_property_changed_cb),
                    self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "character_name_entry")),
                    "key-press-event",
                    G_CALLBACK (mud_connections_property_changed_cb),
                    self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "character_logon_textview")),
                    "key-press-event",
                    G_CALLBACK (mud_connections_property_changed_cb),
                    self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object(builder, "profile_combo")),
                    "changed",
                    G_CALLBACK (mud_connections_property_combo_changed_cb),
                    self);

  g_object_unref (builder);

  g_object_set_data (G_OBJECT (self->properties_window), "current-character", character);
  gtk_widget_set_visible (self->properties_window, TRUE);

  if(character == NULL)
    return;

  mud = mud_character_get_mud (character);

  gtk_entry_set_text (GTK_ENTRY (self->name_entry),
                      mud_mud_get_name (mud));

  gtk_entry_set_text (GTK_ENTRY (self->character_name_entry),
                      mud_character_get_name (character));

  gtk_entry_set_text (GTK_ENTRY (self->host_entry),
                      mud_mud_get_hostname (mud));

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (self->port_entry),
                             mud_mud_get_port (mud));

  mud_connections_property_set_profile (self, mud_character_get_profile (character));

  connect_string = mud_character_get_connect_string (character);
  icon_name = mud_character_get_icon (character);

  if (icon_name && strcmp (icon_name, "gnome-mud") != 0)
    {
      GdkPixbuf *icon;

      self->icon_current = g_strdup (icon_name);

      icon = gdk_pixbuf_new_from_file_at_size (self->icon_current, 48, 48, NULL);
      gtk_image_set_from_pixbuf (GTK_IMAGE (self->icon_image), icon);

      g_object_unref (icon);
    }
    else
    {
      self->icon_current = g_strdup ("gnome-mud");
    }

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->logon_textview));
  gtk_text_buffer_set_text (buffer, connect_string, strlen (connect_string));

  self->changed = FALSE;
}

static void
mud_connections_property_cancel_cb (GtkWidget      *widget,
                                    MudConnections *self)
{
  gtk_widget_destroy (self->properties_window);
}

static void
mud_connections_property_save_cb (GtkWidget      *widget,
                                  MudConnections *self)
{
  if (mud_connections_property_save (self))
    gtk_widget_destroy (self->properties_window);
}

static void
mud_connections_property_icon_cb (GtkWidget      *widget,
                                  MudConnections *self)
{
  mud_connections_show_icon_dialog (self);
}

static gboolean
mud_connections_property_changed_cb (GtkWidget *widget,
                                     GdkEventKey *event,
                                     MudConnections *self)
{
  self->changed = TRUE;

  return FALSE;
}

static void
mud_connections_property_combo_changed_cb (GtkWidget      *widget,
                                           MudConnections *self)
{
  self->changed = TRUE;
}

static gboolean
mud_connections_property_delete_cb (GtkWidget      *widget,
                                    GdkEvent       *event,
                                    MudConnections *self)
{
  if (self->changed)
    switch (mud_connections_property_confirm ())
      {
      case -1:
        return FALSE;

      case 1:
        return mud_connections_property_save (self);

      case 0:
        return TRUE;
      }

  return FALSE;
}

static gint
mud_connections_property_confirm (void)
{
  GtkBuilder *builder;
  GtkWidget *dialog;
  gint result;

  builder = gtk_builder_new_from_resource ("/org/gnome/MUD/muds.ui");

  dialog = GTK_WIDGET (gtk_builder_get_object (builder, "mudviewconfirm"));
  g_object_unref (builder);

  result = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  switch (result)
    {
    case GTK_RESPONSE_OK:
      return 1;

    case GTK_RESPONSE_CLOSE:
      return -1;

    default:
      return 0;
    }
}

static gboolean
mud_connections_property_save (MudConnections *self)
{
  const gchar *mud_name, *hostname, *character_name;
  gchar *logon;
  MudProfile *profile;
  MudCharacter *character;
  MudMud *mud;
  GtkTextIter start, end;
  GtkTextBuffer *buffer;
  GtkTreeIter iter;
  guint port;

  mud_name = gtk_entry_get_text (GTK_ENTRY (self->name_entry));

  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (self->profile_combo), &iter))
    {
      gtk_tree_model_get (GTK_TREE_MODEL (self->profile_model),
                          &iter, MODEL_PROFILE_OBJECT, &profile, -1);
    }
  else
    {
      utils_error_message (self->properties_window, _("Error"),
                           "%s", _("No profile specified."));
      return FALSE;
    }

  if (mud_name[0] == '\0')
    {
      utils_error_message (self->properties_window, _("Error"),
                           "%s", _("No MUD name specified."));
      return FALSE;
    }

  character = g_object_get_data (G_OBJECT (self->properties_window), "current-character");

  hostname = gtk_entry_get_text (GTK_ENTRY (self->host_entry));
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->logon_textview));
  gtk_text_buffer_get_bounds (buffer, &start, &end);
  character_name = gtk_entry_get_text (GTK_ENTRY (self->character_name_entry));
  logon = gtk_text_iter_get_text (&start, &end);
  port = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (self->port_entry));

  if (character == NULL)
    {
      /* New character */
      /* TODO: Associate with existing MUD entry if possible (depends on dialog redesign) */
      mud = mud_profile_manager_add_mud (self->parent_window->profile_manager, mud_name, hostname, port);
      character = mud_profile_manager_add_character (self->parent_window->profile_manager, profile, mud, character_name, logon, self->icon_current);
    }
  else
    {
      g_object_get (character,
                    "mud", &mud,
                    NULL);

      mud_character_set_name (character, character_name);
      mud_character_set_connect_string (character, logon);
      mud_character_set_icon (character, self->icon_current);
      /* TODO: mud_character_set_profile (character, profile); */
      /* FIXME: These change the MUD for all character, but that's how it worked with gconf too - to be redesigned */
      mud_mud_set_name (mud, mud_name);
      mud_mud_set_hostname (mud, hostname);
      mud_mud_set_port (mud, port);
    }

  g_free (logon);

  return TRUE;
}

static void
mud_connections_property_populate_profiles (MudConnections *self)
{
  GSequence *profiles;
  GtkTreeIter iter;
  GSequenceIter *seqiter;

  profiles = mud_profile_manager_get_profiles (self->parent_window->profile_manager);

  self->profile_model = GTK_TREE_MODEL (gtk_list_store_new (MODEL_PROFILE_N, G_TYPE_STRING, MUD_TYPE_PROFILE));

  for (seqiter = g_sequence_get_begin_iter (profiles);
       !g_sequence_iter_is_end (seqiter);
       seqiter = g_sequence_iter_next (seqiter))
    {
      MudProfile *profile = g_sequence_get (seqiter);
      gtk_list_store_append (GTK_LIST_STORE (self->profile_model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (self->profile_model), &iter,
                          MODEL_PROFILE_NAME, profile->visible_name,
                          MODEL_PROFILE_OBJECT, profile, -1);
    }
}

static void
mud_connections_property_set_profile (MudConnections   *self,
                                      const MudProfile *profile)
{
  MudProfile *iterated_profile;
  GtkTreeIter iter;

  gtk_tree_model_get_iter_first(self->profile_model, &iter);

  do
    {
      gtk_tree_model_get(self->profile_model, &iter, MODEL_PROFILE_OBJECT, &iterated_profile, -1);

      if (iterated_profile == profile)
        {
          gtk_combo_box_set_active_iter (GTK_COMBO_BOX (self->profile_combo), &iter);
          return;
        }
    }
  while (gtk_tree_model_iter_next (self->profile_model, &iter));
}

// Iconview Dialog
static void
mud_connections_show_icon_dialog (MudConnections *self)
{
  GtkBuilder *builder;
  gint result;

  builder = gtk_builder_new_from_resource ("/org/gnome/MUD/muds.ui");

  self->icon_dialog = GTK_WIDGET (gtk_builder_get_object (builder, "iconselect"));
  self->icon_dialog_view = GTK_WIDGET (gtk_builder_get_object (builder, "view"));
  self->icon_dialog_chooser = GTK_WIDGET (gtk_builder_get_object (builder, "chooser"));
  g_object_unref (builder);

  self->icon_dialog_view_model =
    GTK_TREE_MODEL (gtk_list_store_new (MODEL_ICON_N,
                                        G_TYPE_STRING,
                                        GDK_TYPE_PIXBUF));

  gtk_icon_view_set_model (GTK_ICON_VIEW (self->icon_dialog_view),
                           self->icon_dialog_view_model);
  gtk_icon_view_set_text_column (GTK_ICON_VIEW (self->icon_dialog_view),
                                 MODEL_ICON_NAME);
  gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (self->icon_dialog_view),
                                   MODEL_ICON_PIXBUF);

  g_clear_pointer (&self->icon_current, g_free);

  g_signal_connect (self->icon_dialog_chooser, "current-folder-changed",
                    G_CALLBACK (mud_connections_icon_fileset_cb),
                    self);
  g_signal_connect (self->icon_dialog_view, "selection-changed",
                    G_CALLBACK (mud_connections_icon_select_cb),
                    self);

  g_object_set (self->icon_dialog_view, "item-width", 64, NULL);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (self->icon_dialog_chooser),
                                       "/usr/share/icons");

  result = gtk_dialog_run (GTK_DIALOG(self->icon_dialog));
  gtk_widget_destroy (self->icon_dialog);

  if (result == GTK_RESPONSE_OK)
    {
      if (self->icon_current == NULL)
        return;

      GdkPixbuf *icon = gdk_pixbuf_new_from_file_at_size (self->icon_current,
                                                          48, 48, NULL);
      gtk_image_set_from_pixbuf (GTK_IMAGE (self->icon_image),
                                 icon);
      g_object_unref(icon);
    }
}

void
mud_connections_icon_fileset_cb (GtkFileChooserButton *chooser,
                                 MudConnections *self)
{
  const gchar *file;
  gchar *current_folder = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (self->icon_dialog_chooser));
  GDir *dir = g_dir_open (current_folder, 0, NULL);
  GPatternSpec *xpm_pattern = g_pattern_spec_new ("*.xpm");
  GPatternSpec *svg_pattern = g_pattern_spec_new ("*.svg");
  GPatternSpec *bmp_pattern = g_pattern_spec_new ("*.bmp");
  GPatternSpec *png_pattern = g_pattern_spec_new ("*.png");

  gtk_list_store_clear (GTK_LIST_STORE (self->icon_dialog_view_model));

  while ( (file = g_dir_read_name (dir) ) != NULL)
    if (g_pattern_match_string (xpm_pattern, file) ||
        g_pattern_match_string (svg_pattern, file) ||
        g_pattern_match_string (bmp_pattern, file) ||
        g_pattern_match_string (png_pattern, file))
      {
        gchar *full_path = g_strconcat (current_folder, G_DIR_SEPARATOR_S, file, NULL);
        GdkPixbuf *icon = gdk_pixbuf_new_from_file_at_size (full_path, 48, 48, NULL);
        GtkTreeIter iter;

        if (icon)
          {
            gtk_list_store_append (GTK_LIST_STORE (self->icon_dialog_view_model),
                                   &iter);
            gtk_list_store_set (GTK_LIST_STORE (self->icon_dialog_view_model),
                                &iter,
                                MODEL_ICON_NAME, file,
                                MODEL_ICON_PIXBUF, icon,
                                -1);

            g_object_unref (icon);
          }

        g_free (full_path);
      }

  g_free (current_folder);
  g_dir_close (dir);
  g_pattern_spec_free (xpm_pattern);
  g_pattern_spec_free (svg_pattern);
  g_pattern_spec_free (bmp_pattern);
  g_pattern_spec_free (png_pattern);
}

static void
mud_connections_icon_select_cb (GtkIconView    *view,
                                MudConnections *self)
{
  GList *selected = gtk_icon_view_get_selected_items (GTK_ICON_VIEW (self->icon_dialog_view));
  GtkTreeIter iter;
  gchar *buf;

  if (g_list_length (selected) == 0)
    return;

  gtk_tree_model_get_iter (self->icon_dialog_view_model,
                           &iter,
                           (GtkTreePath *)selected->data);
  gtk_tree_model_get (self->icon_dialog_view_model,
                      &iter,
                      MODEL_ICON_NAME, &buf,
                      -1);

  g_free (self->icon_current);

  self->icon_current = g_strconcat (gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (self->icon_dialog_chooser)),
                                    G_DIR_SEPARATOR_S,
                                    buf, NULL);

  g_free (buf);

  g_list_foreach (selected, (GFunc)gtk_tree_path_free, NULL);
  g_list_free (selected);
}

static void
mud_connections_constructed (GObject *object)
{
  MudConnections *self = (MudConnections *)object;

  GtkBuilder *builder;
  GtkWidget *main_window;

  G_OBJECT_CLASS (mud_connections_parent_class)->constructed (object);

  g_object_get (self->parent_window,
                "window", &self->winwidget,
                NULL);

  builder = gtk_builder_new_from_resource ("/org/gnome/MUD/muds.ui");

  self->window = GTK_WIDGET (gtk_builder_get_object (builder, "mudviewwindow"));
  self->iconview = GTK_WIDGET (gtk_builder_get_object (builder, "iconview"));

  self->qconnect_host_entry =
    GTK_WIDGET (gtk_builder_get_object (builder, "qconnect_host_entry"));
  self->qconnect_port_entry =
    GTK_WIDGET (gtk_builder_get_object (builder, "qconnect_port_entry"));
  self->qconnect_connect_button =
    GTK_WIDGET (gtk_builder_get_object (builder, "qconnect_connect_button"));

  self->icon_model = GTK_TREE_MODEL (gtk_list_store_new (MODEL_COLUMN_N,
                                                         G_TYPE_STRING,
                                                         GDK_TYPE_PIXBUF,
                                                         MUD_TYPE_CHARACTER));

  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (self->icon_model),
                                        MODEL_COLUMN_VISIBLE_NAME,
                                        GTK_SORT_ASCENDING);

  gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (self->icon_model),
                                   MODEL_COLUMN_VISIBLE_NAME,
                                   mud_connections_compare_func,
                                   NULL,
                                   NULL);

  gtk_icon_view_set_model (GTK_ICON_VIEW (self->iconview), self->icon_model);
  g_object_unref (self->icon_model);

  gtk_icon_view_set_text_column (GTK_ICON_VIEW(self->iconview), MODEL_COLUMN_VISIBLE_NAME);
  gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW(self->iconview), MODEL_COLUMN_PIXBUF);

#warning Hook in iconview update watchers

  g_signal_connect (self->iconview, "item-activated",
                    G_CALLBACK (mud_connections_iconview_activate_cb),
                    self);
  g_signal_connect (self->iconview, "button-press-event",
                    G_CALLBACK (mud_connections_button_press_cb),
                    self);
  g_signal_connect (self->window, "destroy",
                    G_CALLBACK (mud_connections_close_cb), self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "connect_button")),
                    "clicked", G_CALLBACK (mud_connections_connect_cb), self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "add_button")),
                    "clicked", G_CALLBACK (mud_connections_add_cb), self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "delete_button")),
                    "clicked", G_CALLBACK (mud_connections_delete_cb), self);
  g_signal_connect (GTK_WIDGET (gtk_builder_get_object (builder, "properties_button")),
                    "clicked", G_CALLBACK (mud_connections_properties_cb), self);
  g_signal_connect (self->qconnect_connect_button, "clicked",
                    G_CALLBACK (mud_connections_qconnect_cb), self);

  mud_connections_populate_iconview (self);

  g_object_get (self->parent_window, "window", &main_window, NULL);

  g_object_set (self->window,
                "transient-for", GTK_WINDOW (main_window),
                NULL);

  gtk_widget_show_all (self->window);
  g_object_unref (builder);
}

static void
mud_connections_finalize (GObject *object)
{
  MudConnections *self = (MudConnections *)object;

  /* TODO */

  G_OBJECT_CLASS (mud_connections_parent_class)->finalize (object);
}

static void
mud_connections_get_property(GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  MudConnections *self;

  self = MUD_CONNECTIONS (object);

  switch (prop_id)
    {
    case PROP_PARENT_WINDOW:
      g_value_take_object (value, self->parent_window);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
mud_connections_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  MudConnections *self = MUD_CONNECTIONS (object);

  switch (prop_id)
    {
    case PROP_PARENT_WINDOW:
      self->parent_window = MUD_WINDOW (g_value_get_object(value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
mud_connections_class_init (MudConnectionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = mud_connections_constructed;
  object_class->finalize = mud_connections_finalize;
  object_class->get_property = mud_connections_get_property;
  object_class->set_property = mud_connections_set_property;

  /* TODO: Singleton MudWindow? */
  properties [PROP_PARENT_WINDOW] =
    g_param_spec_object("parent-window",
                        "parent mud window",
                        "the mud window the connections is attached to",
                        MUD_TYPE_WINDOW,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
mud_connections_init (MudConnections *self)
{
}
