/* GNOME-Mud - A simple Mud Client
 * mud-window-profile.c
 * Copyright 2008-2009 Les Harris <lharris@gnome.org>
 * Copyright 2018 Mart Raudsepp <leio@gentoo.org>
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

#include <glib/gi18n.h>
#include <string.h>
#include <glib/gprintf.h>

#include "gnome-mud.h"
#include "mud-connection-view.h"
#include "mud-window.h"
#include "mud-window-profile.h"
#include "resources.h"
#include "utils.h"
#include "mud-profile-manager.h"
#include "mud-window-prefs.h"

struct _MudProfileWindowPrivate
{
    GtkWidget *window;
    GtkWidget *treeview; /* TODO: Port to GtkListBox or some other redesign with gtk3 */

    GtkWidget *btnAdd;
    GtkWidget *btnDelete;
    GtkWidget *btnEdit;

    MudProfile *CurrSelRowProfile; /* TODO: We might lose the pointer while having a pointer stored here; reference it or add other safeguards (e.g. storing uuid instead and handling it appropriately in the face of uuid removals) */

    GtkListStore *liststore;
    GtkTreeViewColumn *col;
};

/* Model Columns */
enum
{
    NAME_COLUMN,
    PROFILE_COLUMN,
    N_COLUMNS
};

/* Property Identifiers */
enum
{
    PROP_MUD_PROFILE_WINDOW_0,
    PROP_PARENT_WINDOW
};

/* Create the Type */
G_DEFINE_TYPE(MudProfileWindow, mud_profile_window, G_TYPE_OBJECT);

/* Class Functions */
static void mud_profile_window_init (MudProfileWindow *preferences);
static void mud_profile_window_class_init (MudProfileWindowClass *klass);
static void mud_profile_window_finalize (GObject *object);
static GObject *mud_profile_window_constructor (GType gtype,
                                                guint n_properties,
                                                GObjectConstructParam *properties);
static void mud_profile_window_set_property(GObject *object,
                                            guint prop_id,
                                            const GValue *value,
                                            GParamSpec *pspec);
static void mud_profile_window_get_property(GObject *object,
                                            guint prop_id,
                                            GValue *value,
                                            GParamSpec *pspec);

/* Callbacks */
static gint mud_profile_window_close_cb(GtkWidget *widget, MudProfileWindow *profwin);
static void mud_profile_window_add_cb(GtkWidget *widget, MudProfileWindow *profwin);
static void mud_profile_window_del_cb(GtkWidget *widget, MudProfileWindow *profwin);
static void mud_profile_window_edit_cb(GtkWidget *widget, MudProfileWindow *profwin);
static gboolean mud_profile_window_tree_select_cb(GtkTreeSelection *selection,
                     			   GtkTreeModel     *model,
                     			   GtkTreePath      *path,
                   			   gboolean        path_currently_selected,
                     			   gpointer          userdata);
/* Private Methods */
static void mud_profile_window_populate_treeview(MudProfileWindow *profwin);

/* Class Functions */
static void
mud_profile_window_class_init (MudProfileWindowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    /* Override base object constructor */
    object_class->constructor = mud_profile_window_constructor;

    /* Override base object's finalize */
    object_class->finalize = mud_profile_window_finalize;

    /* Override base object property methods */
    object_class->set_property = mud_profile_window_set_property;
    object_class->get_property = mud_profile_window_get_property;

    /* Add private data to class */
    g_type_class_add_private(klass, sizeof(MudProfileWindowPrivate));

    /* Install Properties */
    g_object_class_install_property(object_class,
            PROP_PARENT_WINDOW,
            g_param_spec_object("parent-window",
                "parent gtk window",
                "the gtk window parent of this window",
                MUD_TYPE_WINDOW,
                G_PARAM_READWRITE|G_PARAM_CONSTRUCT_ONLY));
}

static void
mud_profile_window_init (MudProfileWindow *self)
{
    /* Get our private data */
    self->priv = MUD_PROFILE_WINDOW_GET_PRIVATE(self);

    /* set public members to defaults */
    self->parent_window = NULL;
}

static GObject *
mud_profile_window_constructor (GType gtype,
                                guint n_properties,
                                GObjectConstructParam *properties)
{
    MudProfileWindow *profwin;
    GObject *obj;

    MudProfileWindowClass *klass;
    GObjectClass *parent_class;

    GtkBuilder *builder;
    GBytes *res_bytes;
    gconstpointer res_data;
    gsize res_size;
    GError *error = NULL;
    GtkWindow *main_window;
    GtkCellRenderer *renderer;

    /* Chain up to parent constructor */
    klass = MUD_PROFILE_WINDOW_CLASS( g_type_class_peek(MUD_TYPE_PROFILE_WINDOW) );
    parent_class = G_OBJECT_CLASS( g_type_class_peek_parent(klass) );
    obj = parent_class->constructor(gtype, n_properties, properties);

    profwin = MUD_PROFILE_WINDOW(obj);

    if(!profwin->parent_window)
    {
        g_printf("ERROR: Tried to instantiate MudProfileWindow without passing parent GtkWindow\n");
        g_error("Tried to instantiate MudProfileWindow without passing parent GtkWindow\n");
    }

    builder = gtk_builder_new ();
    res_bytes = g_resource_lookup_data (gnome_mud_get_resource (), "/org/gnome/MUD/prefs.ui", 0, NULL);
    res_data = g_bytes_get_data (res_bytes, &res_size);
    if (gtk_builder_add_from_string (builder, res_data, res_size, &error) == 0)
        g_error ("Failed to load resources: %s", error->message);
    g_bytes_unref (res_bytes);

    profwin->priv->window = GTK_WIDGET(gtk_builder_get_object(builder, "profiles_window"));

    profwin->priv->btnAdd = GTK_WIDGET(gtk_builder_get_object(builder, "btnAdd"));
    profwin->priv->btnDelete = GTK_WIDGET(gtk_builder_get_object(builder, "btnDelete"));
    profwin->priv->btnEdit = GTK_WIDGET(gtk_builder_get_object(builder, "btnEdit"));

    profwin->priv->treeview = GTK_WIDGET(gtk_builder_get_object(builder, "profilesView"));
    profwin->priv->liststore = GTK_LIST_STORE(gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER));

    g_object_set(profwin->priv->treeview,
                 "model", GTK_TREE_MODEL(profwin->priv->liststore),
                 "rules-hint", TRUE,
                 "headers-visible", FALSE,
                 NULL);
    
    profwin->priv->col = gtk_tree_view_column_new();

    gtk_tree_view_append_column(GTK_TREE_VIEW(profwin->priv->treeview),
                                profwin->priv->col);
    
    renderer = gtk_cell_renderer_text_new();
    
    gtk_tree_view_column_pack_start(profwin->priv->col,
                                    renderer,
                                    TRUE);

    gtk_tree_view_column_add_attribute(profwin->priv->col,
                                       renderer,
                                       "text", NAME_COLUMN);

    gtk_tree_selection_set_select_function(
            gtk_tree_view_get_selection(GTK_TREE_VIEW(profwin->priv->treeview)),
            mud_profile_window_tree_select_cb,
            profwin, NULL);

    mud_profile_window_populate_treeview(profwin);

    g_signal_connect(profwin->priv->window, "destroy",
            G_CALLBACK(mud_profile_window_close_cb),
            profwin);
    g_signal_connect(profwin->priv->btnAdd, "clicked",
            G_CALLBACK(mud_profile_window_add_cb),
            profwin);
    g_signal_connect(profwin->priv->btnDelete, "clicked",
            G_CALLBACK(mud_profile_window_del_cb),
            profwin);
    g_signal_connect(profwin->priv->btnEdit, "clicked",
            G_CALLBACK(mud_profile_window_edit_cb),
            profwin);

    g_object_get(profwin->parent_window, "window", &main_window, NULL);

    gtk_window_set_transient_for(
            GTK_WINDOW(profwin->priv->window),
            main_window);

    gtk_widget_show_all(profwin->priv->window);

    g_object_unref(builder);

    return obj;
}

static void
mud_profile_window_finalize (GObject *object)
{
    GObjectClass *parent_class;

    parent_class = g_type_class_peek_parent(G_OBJECT_GET_CLASS(object));
    parent_class->finalize(object);
}

static void
mud_profile_window_set_property(GObject *object,
                                guint prop_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
    MudProfileWindow *self;

    self = MUD_PROFILE_WINDOW(object);

    switch(prop_id)
    {
        /* Parent is Construct Only */
        case PROP_PARENT_WINDOW:
            self->parent_window = MUD_WINDOW(g_value_get_object(value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void
mud_profile_window_get_property(GObject *object,
                                guint prop_id,
                                GValue *value,
                                GParamSpec *pspec)
{
    MudProfileWindow *self;

    self = MUD_PROFILE_WINDOW(object);

    switch(prop_id)
    {
        case PROP_PARENT_WINDOW:
            g_value_take_object(value, self->parent_window);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

/* Callbacks */
static void
mud_profile_window_add_cb(GtkWidget *widget, MudProfileWindow *profwin)
{
    GtkBuilder *builder;
    GBytes *res_bytes;
    gconstpointer res_data;
    gsize res_size;
    GError *error = NULL;
    GtkWidget *window;
    GtkWidget *entry_profile;
    gchar *profile_name;
    gint result;
    MudProfile *prof;

    builder = gtk_builder_new ();
    res_bytes = g_resource_lookup_data (gnome_mud_get_resource (), "/org/gnome/MUD/prefs.ui", 0, NULL);
    res_data = g_bytes_get_data (res_bytes, &res_size);
    if (gtk_builder_add_from_string (builder, res_data, res_size, &error) == 0)
        g_error ("Failed to load resources: %s", error->message);
    g_bytes_unref (res_bytes);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "newprofile_dialog"));

    gtk_window_set_transient_for(
            GTK_WINDOW(window),
            GTK_WINDOW(profwin->priv->window));

    entry_profile = GTK_WIDGET(gtk_builder_get_object(builder, "entry_profile"));

    result = gtk_dialog_run(GTK_DIALOG(window));
    if (result == GTK_RESPONSE_OK)
    {
        profile_name = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_profile)));
        prof = mud_profile_manager_add_profile(profwin->parent_window->profile_manager,
                                               profile_name);

        mud_profile_window_populate_treeview(profwin);

        g_free(profile_name);
    }

    gtk_widget_destroy(window);
    g_object_unref(builder);
}

static void
mud_profile_window_del_cb(GtkWidget *widget, MudProfileWindow *profwin)
{
#warning Add safeguards against deleting default or last item
    mud_profile_manager_delete_profile(profwin->parent_window->profile_manager,
                                       profwin->priv->CurrSelRowProfile);

    mud_profile_window_populate_treeview(profwin);
}

static void
mud_profile_window_edit_cb (GtkWidget *widget, MudProfileWindow *profwin)
{
  g_object_new(MUD_TYPE_WINDOW_PREFS,
               "parent-window", profwin->parent_window,
               "mud-profile", profwin->priv->CurrSelRowProfile,
               NULL);
}

static gint
mud_profile_window_close_cb(GtkWidget *widget, MudProfileWindow *profwin)
{
    g_object_unref(profwin);

    return TRUE;
}

static gboolean
mud_profile_window_tree_select_cb(GtkTreeSelection *selection,
                                  GtkTreeModel     *model,
                                  GtkTreePath      *path,
                                  gboolean          path_currently_selected,
                                  gpointer          userdata)
{
    GtkTreeIter iter;
    MudProfileWindow *profwin = (MudProfileWindow *)userdata;

    if (gtk_tree_model_get_iter(model, &iter, path))
    {
        gtk_tree_model_get(model, &iter, PROFILE_COLUMN, &profwin->priv->CurrSelRowProfile, -1);

        gtk_widget_set_sensitive(profwin->priv->btnEdit, TRUE);

        /* TODO: Don't allow deleting default profile (in addition to the last one, which should be the default too */
        gtk_widget_set_sensitive(profwin->priv->btnDelete, gtk_tree_model_iter_n_children(GTK_TREE_MODEL(profwin->priv->liststore), NULL) > 1);
    }
    /* TODO: desensitize btnEdit if selection is removed (ctrl+click on selected item with treeview); alternative just ensure a selection is always kept, perhaps after GtkListBox or redesign */

    return TRUE;
}

/* Private Methods */
static void
mud_profile_window_populate_treeview (MudProfileWindow *profwin)
{
  GSequence *profiles;
  MudProfile *profile;
  GtkListStore *store;
  GtkTreeIter iter;
  GSequenceIter *seqiter;

  g_return_if_fail(MUD_IS_PROFILE_WINDOW(profwin));

  store = profwin->priv->liststore;

  gtk_list_store_clear (store);

  gtk_widget_set_sensitive (profwin->priv->btnDelete, FALSE);
  gtk_widget_set_sensitive (profwin->priv->btnEdit, FALSE);

  profiles = mud_profile_manager_get_profiles (profwin->parent_window->profile_manager);

  for (seqiter = g_sequence_get_begin_iter (profiles);
       !g_sequence_iter_is_end (seqiter);
       seqiter = g_sequence_iter_next (seqiter))
    {
      profile = g_sequence_get (seqiter);

      gtk_list_store_append(store, &iter);
      gchar *name2;
      g_object_get (profile,
                    "name", &name2,
                    NULL);
      g_free (name2);
      gtk_list_store_set (store, &iter,
                          NAME_COLUMN, profile->visible_name, /* TODO: Handle outside visible-name renames */
                          PROFILE_COLUMN, profile,
                          -1);

    }
}

