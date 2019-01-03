/* GNOME-Mud - A simple Mud Client
 * mud-window-prefs.c
 * Copyright 2005-2009 Les Harris <lharris@gnome.org>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include "gnome-mud.h"
#include "mud-profile.h"
#include "mud-profile-manager.h"
#include "mud-window.h"
#include "mud-window-prefs.h"
#include "resources.h"

struct _MudWindowPrefsPrivate
{
    MudWindow *parent;
    MudProfile *mud_profile;

    GtkWidget *window;
    gulong signal;
    gint notification_count;

    /* Terminal Tab */
    GtkWidget *echo_check;
    GtkWidget *keep_check;
    GtkWidget *div_entry;
    GtkWidget *encoding_combo;
    GtkWidget *scroll_check;
    GtkWidget *lines_spin;
    GtkWidget *font_button;
    GtkWidget *fore_button;
    GtkWidget *back_button;
    GtkWidget *colors[C_MAX];

    /* Network Tab */
    GtkWidget *proxy_check;
    GtkWidget *proxy_entry;
    GtkWidget *proxy_combo;
    GtkWidget *msp_check;
    GtkWidget *charset_check;

    /* Triggers Tab */

    /* Aliases Tab */

    /* Variables Tab */

    /* Timers Tab */

    /* Keybindings Tab */
};

/* Property Identifiers */
enum
{
    PROP_MUD_WINDOW_PREFS_0,
    PROP_PARENT,
    PROP_MUD_PROFILE
};

/* Create the Type */
G_DEFINE_TYPE(MudWindowPrefs, mud_window_prefs, G_TYPE_OBJECT);

#define RETURN_IF_CHANGING_PROFILES(self) \
    if (self->priv->notification_count) return

/* Class Functions */
static void mud_window_prefs_init (MudWindowPrefs *self);
static void mud_window_prefs_class_init (MudWindowPrefsClass *klass);
static void mud_window_prefs_finalize (GObject *object);
static void mud_window_prefs_constructed(GObject *object);
static GObject *mud_window_prefs_constructor (GType gtype,
                                              guint n_properties,
                                              GObjectConstructParam *properties);
static void mud_window_prefs_set_property(GObject *object,
                                          guint prop_id,
                                          const GValue *value,
                                          GParamSpec *pspec);
static void mud_window_prefs_get_property(GObject *object,
                                          guint prop_id,
                                          GValue *value,
                                          GParamSpec *pspec);

/************************ Window ************************/
static void mud_window_prefs_construct_window(MudWindowPrefs *self);

// Callbacks
static gboolean mud_window_prefs_delete_event_cb(GtkWidget *widget,
                                                 GdkEvent *event,
                                                 MudWindowPrefs *self);
static void mud_window_prefs_changed_cb(MudProfile *profile,
                                        MudProfileMask *mask,
                                        MudWindowPrefs *window);

// Private Methods
static void mud_window_prefs_set_preferences(MudWindowPrefs *self);


/************************ Terminal Tab ************************/
static void mud_window_prefs_construct_terminal_tab(MudWindowPrefs *self);

// Update Functions
static void mud_window_prefs_update_commdev(MudWindowPrefs *self,
                                            MudPrefs *preferences);
static void mud_window_prefs_update_scrolloutput(MudWindowPrefs *self,
                                                 MudPrefs *preferences);
static void mud_window_prefs_update_encoding_combo(MudWindowPrefs *self,
                                                   MudPrefs *preferences);
static void mud_window_prefs_update_keeptext(MudWindowPrefs *self,
                                             MudPrefs *preferences);
static void mud_window_prefs_update_echotext(MudWindowPrefs *self,
                                             MudPrefs *preferences);
static void mud_window_prefs_update_scrollback(MudWindowPrefs *self,
                                               MudPrefs *preferences);
static void mud_window_prefs_update_font(MudWindowPrefs *self,
                                         MudPrefs *preferences);
static void mud_window_prefs_update_foreground(MudWindowPrefs *self,
                                               MudPrefs *preferences);
static void mud_window_prefs_update_background(MudWindowPrefs *self,
                                               MudPrefs *preferences);
static void mud_window_prefs_update_colors(MudWindowPrefs *self,
                                           MudPrefs *preferences);

// Callbacks
static void mud_window_prefs_echo_cb(GtkWidget *widget,
                                     MudWindowPrefs *window);
static void mud_window_prefs_keeptext_cb(GtkWidget *widget,
                                         MudWindowPrefs *window);
static void mud_window_prefs_scrolloutput_cb(GtkWidget *widget,
                                             MudWindowPrefs *window);
static void mud_window_prefs_commdev_cb(GtkWidget *widget,
                                        MudWindowPrefs *window);
static void mud_window_prefs_scrollback_cb(GtkWidget *widget,
                                           MudWindowPrefs *window);
static void mud_window_prefs_font_cb(GtkWidget *widget,
                                     MudWindowPrefs *window);
static void mud_window_prefs_foreground_cb(GtkWidget *widget,
                                           MudWindowPrefs *window);
static void mud_window_prefs_background_cb(GtkWidget *widget,
                                           MudWindowPrefs *window);
static void mud_window_prefs_colors_cb(GtkWidget *widget,
                                       MudWindowPrefs *window);
static void mud_window_prefs_encoding_combo_cb(GtkWidget *widget,
                                               MudWindowPrefs *window);

/************************ Network Tab ************************/
static void mud_window_prefs_construct_network_tab(MudWindowPrefs *self);

// Update Functions
static void mud_window_prefs_update_proxy_check(MudWindowPrefs *self,
                                                MudPrefs *preferences);
static void mud_window_prefs_update_encoding_check(MudWindowPrefs *self,
                                                   MudPrefs *preferences);
static void mud_window_prefs_update_msp_check(MudWindowPrefs *self,
                                              MudPrefs *preferences);
static void mud_window_prefs_update_proxy_combo(MudWindowPrefs *self,
                                                MudPrefs *preferences);
static void mud_window_prefs_update_proxy_entry(MudWindowPrefs *self,
                                                MudPrefs *preferences);

static void mud_window_prefs_encoding_check_cb(GtkWidget *widget,
                                               MudWindowPrefs *window);
static void mud_window_prefs_proxy_check_cb(GtkWidget *widget,
                                            MudWindowPrefs *window);
static void mud_window_prefs_proxy_combo_cb(GtkWidget *widget,
                                            MudWindowPrefs *window);
static void mud_window_prefs_proxy_entry_cb(GtkWidget *widget,
                                            MudWindowPrefs *window);
static void mud_window_prefs_msp_check_cb(GtkWidget *widget,
                                          MudWindowPrefs *window);

/************************ Triggers Tab ************************/
static void mud_window_prefs_construct_triggers_tab(MudWindowPrefs *self);

/************************ Aliases Tab ************************/
static void mud_window_prefs_construct_aliases_tab(MudWindowPrefs *self);

/************************ Variables Tab ************************/
static void mud_window_prefs_construct_variables_tab(MudWindowPrefs *self);

/************************ Timers Tab ************************/
static void mud_window_prefs_construct_timers_tab(MudWindowPrefs *self);

/************************ Keybindings Tab ************************/
static void mud_window_prefs_construct_keybindings_tab(MudWindowPrefs *self);

/* MudWindowPrefs class functions */
static void
mud_window_prefs_class_init (MudWindowPrefsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    /* Override base object constructor */
    object_class->constructor = mud_window_prefs_constructor;
    object_class->constructed = mud_window_prefs_constructed;

    /* Override base object's finalize */
    object_class->finalize = mud_window_prefs_finalize;

    /* Override base object property methods */
    object_class->set_property = mud_window_prefs_set_property;
    object_class->get_property = mud_window_prefs_get_property;

    /* Add private data to class */
    g_type_class_add_private(klass, sizeof(MudWindowPrefsPrivate));

    /* Install Properties */
    g_object_class_install_property(object_class,
            PROP_PARENT,
            g_param_spec_object("parent-window",
                "Parent MudWindow",
                "The parent MudWindow object.",
                MUD_TYPE_WINDOW,
                G_PARAM_READWRITE|G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property(object_class,
            PROP_MUD_PROFILE,
            g_param_spec_object("mud-profile",
                "MudProfile",
                "The associated MudProfile object.",
                MUD_TYPE_PROFILE,
                G_PARAM_READWRITE|G_PARAM_CONSTRUCT_ONLY));
}

static void
mud_window_prefs_init (MudWindowPrefs *self)
{
    /* Get our private data */
    self->priv = MUD_WINDOW_PREFS_GET_PRIVATE(self);

    /* Set Defaults */
    self->priv->parent = NULL;
    self->priv->notification_count = 0;
}

static GObject *
mud_window_prefs_constructor (GType gtype,
                              guint n_properties,
                              GObjectConstructParam *properties)
{
    MudWindowPrefs *self;
    GObject *obj;
    MudWindowPrefsClass *klass;
    GObjectClass *parent_class;

    /* Chain up to parent constructor */
    klass = MUD_WINDOW_PREFS_CLASS( g_type_class_peek(MUD_TYPE_WINDOW_PREFS) );
    parent_class = G_OBJECT_CLASS( g_type_class_peek_parent(klass) );
    obj = parent_class->constructor(gtype, n_properties, properties);

    self = MUD_WINDOW_PREFS(obj);

    if(!self->priv->parent)
    {
        g_printf("ERROR: Tried to instantiate MudWindowPrefs without passing parent MudWindow\n");
        g_error("Tried to instantiate MudWindowPrefs without passing parent MudWindow");
    }

    if(!self->priv->mud_profile)
    {
        g_printf("ERROR: Tried to instantiate MudWindowPrefs without passing profile.\n");
        g_error("Tried to instantiate MudWindowPrefs without passing parent profile.");
    }

    mud_window_prefs_construct_window(self);
    mud_window_prefs_construct_terminal_tab(self);
    mud_window_prefs_construct_network_tab(self);
    mud_window_prefs_construct_triggers_tab(self);
    mud_window_prefs_construct_aliases_tab(self);
    mud_window_prefs_construct_variables_tab(self);
    mud_window_prefs_construct_timers_tab(self);
    mud_window_prefs_construct_keybindings_tab(self);

    self->priv->signal = g_signal_connect(self->priv->mud_profile,
                                          "changed",
                                          G_CALLBACK(mud_window_prefs_changed_cb),
                                          self);

    return obj;
}

static void
mud_window_prefs_constructed(GObject *object)
{
    MudWindowPrefs *self = MUD_WINDOW_PREFS(object);

    self->priv->notification_count++;
    mud_window_prefs_set_preferences(self);
    self->priv->notification_count--;

    gtk_widget_show_all(self->priv->window);
}

static void
mud_window_prefs_finalize (GObject *object)
{
    MudWindowPrefs *self;
    GObjectClass *parent_class;

    self = MUD_WINDOW_PREFS(object);

    g_signal_handler_disconnect(self->priv->mud_profile,
                                self->priv->signal);

    parent_class = g_type_class_peek_parent(G_OBJECT_GET_CLASS(object));
    parent_class->finalize(object);
}

static void
mud_window_prefs_set_property(GObject *object,
                              guint prop_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
    MudWindowPrefs *self;

    self = MUD_WINDOW_PREFS(object);

    switch(prop_id)
    {
        /* Parent is Construct Only */
        case PROP_PARENT:
            self->priv->parent = MUD_WINDOW(g_value_get_object(value));
            break;

        case PROP_MUD_PROFILE:
            self->priv->mud_profile = MUD_PROFILE(g_value_get_object(value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void
mud_window_prefs_get_property(GObject *object,
                              guint prop_id,
                              GValue *value,
                              GParamSpec *pspec)
{
    MudWindowPrefs *self;

    self = MUD_WINDOW_PREFS(object);

    switch(prop_id)
    {
        case PROP_PARENT:
            g_value_take_object(value, self->priv->parent);
            break;

        case PROP_MUD_PROFILE:
            g_value_take_object(value, self->priv->mud_profile);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

/************************ Window ************************/
static void
mud_window_prefs_construct_window(MudWindowPrefs *self)
{
    GtkBuilder *builder;
    GBytes *res_bytes;
    gconstpointer res_data;
    gsize res_size;
    GError *error = NULL;
    GtkWidget *main_window;
    gint i;

    builder = gtk_builder_new ();
    res_bytes = g_resource_lookup_data (gnome_mud_get_resource (), "/org/gnome/MUD/prefs.ui", 0, NULL);
    res_data = g_bytes_get_data (res_bytes, &res_size);
    if (gtk_builder_add_from_string (builder, res_data, res_size, &error) == 0)
        g_error ("Failed to load resources: %s", error->message);
    g_bytes_unref (res_bytes);

    self->priv->window = GTK_WIDGET(gtk_builder_get_object(builder, "preferences_window"));

    self->priv->echo_check     = GTK_WIDGET(gtk_builder_get_object(builder, "cb_echo"));
    self->priv->keep_check     = GTK_WIDGET(gtk_builder_get_object(builder, "cb_keep"));
    self->priv->div_entry      = GTK_WIDGET(gtk_builder_get_object(builder, "entry_commdev"));
    self->priv->encoding_combo = GTK_WIDGET(gtk_builder_get_object(builder, "encoding_combo"));
    self->priv->scroll_check   = GTK_WIDGET(gtk_builder_get_object(builder, "cb_scrollback"));
    self->priv->lines_spin     = GTK_WIDGET(gtk_builder_get_object(builder, "sb_lines"));
    self->priv->font_button    = GTK_WIDGET(gtk_builder_get_object(builder, "fp_font"));
    self->priv->fore_button    = GTK_WIDGET(gtk_builder_get_object(builder, "cb_foreground"));
    self->priv->back_button    = GTK_WIDGET(gtk_builder_get_object(builder, "cb_background"));
    self->priv->proxy_check    = GTK_WIDGET(gtk_builder_get_object(builder, "proxy_check"));
    self->priv->proxy_entry    = GTK_WIDGET(gtk_builder_get_object(builder, "proxy_entry"));
    self->priv->proxy_combo    = GTK_WIDGET(gtk_builder_get_object(builder, "proxy_combo"));
    self->priv->msp_check      = GTK_WIDGET(gtk_builder_get_object(builder, "msp_check"));
    self->priv->charset_check  = GTK_WIDGET(gtk_builder_get_object(builder, "charset_check"));

    for(i = 0; i < C_MAX; ++i)
    {
        gchar *cwidget = g_strdup_printf("cb%d", i);

        self->priv->colors[i]  = GTK_WIDGET(gtk_builder_get_object(builder, cwidget));

        g_free(cwidget); 
    }

    g_object_get(self->priv->parent, "window", &main_window, NULL);
    gtk_window_set_transient_for(GTK_WINDOW(self->priv->window),
                                 GTK_WINDOW(main_window));

    gtk_window_set_title(GTK_WINDOW(self->priv->window),
                         self->priv->mud_profile->visible_name);

    g_signal_connect(self->priv->window,
                     "delete-event",
                     G_CALLBACK(mud_window_prefs_delete_event_cb),
                     self);

    g_object_unref(builder);
}

// Callbacks
static gboolean
mud_window_prefs_delete_event_cb(GtkWidget *widget,
                                 GdkEvent *event,
                                 MudWindowPrefs *self)
{
    gtk_widget_destroy(self->priv->window);
    g_object_unref(self);

    return FALSE;
}

static void
mud_window_prefs_changed_cb(MudProfile *profile,
                            MudProfileMask *mask,
                            MudWindowPrefs *window)
{
    if (mask->EchoText)
        mud_window_prefs_update_echotext(window, profile->preferences);
    if (mask->KeepText)
        mud_window_prefs_update_keeptext(window, profile->preferences);
    if (mask->ScrollOnOutput)
        mud_window_prefs_update_scrolloutput(window, profile->preferences);
    if (mask->CommDev)
        mud_window_prefs_update_commdev(window, profile->preferences);
    if (mask->Scrollback)
        mud_window_prefs_update_scrollback(window, profile->preferences);
    if (mask->FontName)
        mud_window_prefs_update_font(window, profile->preferences);
    if (mask->Foreground)
        mud_window_prefs_update_foreground(window, profile->preferences);
    if (mask->Background)
        mud_window_prefs_update_background(window, profile->preferences);
    if (mask->Colors)
        mud_window_prefs_update_colors(window, profile->preferences);
    if (mask->UseProxy)
        mud_window_prefs_update_proxy_check(window, profile->preferences);
    if (mask->UseRemoteEncoding)
        mud_window_prefs_update_encoding_check(window, profile->preferences);
    if (mask->ProxyHostname)
        mud_window_prefs_update_proxy_entry(window, profile->preferences);
    if (mask->ProxyVersion)
        mud_window_prefs_update_proxy_combo(window, profile->preferences);
    if (mask->Encoding)
        mud_window_prefs_update_encoding_combo(window, profile->preferences);
    if (mask->UseRemoteDownload)
        mud_window_prefs_update_msp_check(window, profile->preferences);
}

// Private Methods
static void
mud_window_prefs_set_preferences(MudWindowPrefs *self)
{
    /* TODO: Just bind via GSettings instead? */
    // Terminal
    mud_window_prefs_update_echotext(self, self->priv->mud_profile->preferences);
    mud_window_prefs_update_keeptext(self, self->priv->mud_profile->preferences);
    mud_window_prefs_update_scrolloutput(self, self->priv->mud_profile->preferences);
    mud_window_prefs_update_commdev(self, self->priv->mud_profile->preferences);
    mud_window_prefs_update_scrollback(self, self->priv->mud_profile->preferences);
    mud_window_prefs_update_font(self, self->priv->mud_profile->preferences);
    mud_window_prefs_update_foreground(self, self->priv->mud_profile->preferences);
    mud_window_prefs_update_background(self, self->priv->mud_profile->preferences);
    mud_window_prefs_update_colors(self, self->priv->mud_profile->preferences);
    mud_window_prefs_update_encoding_combo(self, self->priv->mud_profile->preferences);

    // Network
    mud_window_prefs_update_proxy_check(self, self->priv->mud_profile->preferences);
    mud_window_prefs_update_proxy_combo(self, self->priv->mud_profile->preferences);
    mud_window_prefs_update_proxy_entry(self, self->priv->mud_profile->preferences);
    mud_window_prefs_update_encoding_check(self, self->priv->mud_profile->preferences);
    mud_window_prefs_update_msp_check(self, self->priv->mud_profile->preferences);
}

/************************ Terminal Tab ************************/
static void
mud_window_prefs_construct_terminal_tab(MudWindowPrefs *self)
{
    gint i;

    g_signal_connect(self->priv->echo_check,
                     "toggled",
                     G_CALLBACK(mud_window_prefs_echo_cb),
                     self);

    g_signal_connect(self->priv->keep_check,
                     "toggled",
                     G_CALLBACK(mud_window_prefs_keeptext_cb),
                     self);

    g_signal_connect(self->priv->scroll_check,
                     "toggled",
                     G_CALLBACK(mud_window_prefs_scrolloutput_cb),
                     self);
    
    g_signal_connect(self->priv->div_entry,
                     "changed",
                     G_CALLBACK(mud_window_prefs_commdev_cb),
                     self);

    g_signal_connect(self->priv->encoding_combo,
                     "changed",
                     G_CALLBACK(mud_window_prefs_encoding_combo_cb),
                     self);

    g_signal_connect(self->priv->lines_spin,
                     "changed",
                     G_CALLBACK(mud_window_prefs_scrollback_cb),
                     self);

    g_signal_connect(self->priv->font_button,
                     "font_set",
                     G_CALLBACK(mud_window_prefs_font_cb),
                     self);

    g_signal_connect(self->priv->fore_button,
                     "color_set",
                     G_CALLBACK(mud_window_prefs_foreground_cb),
                     self);

    g_signal_connect(self->priv->back_button,
                     "color_set",
                     G_CALLBACK(mud_window_prefs_background_cb),
                     self);

    for (i = 0; i < C_MAX; i++)
        g_signal_connect(self->priv->colors[i],
                         "color_set",
                         G_CALLBACK(mud_window_prefs_colors_cb),
                         self);
}

// Update Functions
static void
mud_window_prefs_update_commdev(MudWindowPrefs *self,
                                MudPrefs *preferences)
{
    gtk_entry_set_text(GTK_ENTRY(self->priv->div_entry),
                       preferences->CommDev);
}

static void
mud_window_prefs_update_scrolloutput(MudWindowPrefs *self,
                                     MudPrefs *preferences)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->priv->scroll_check),
                                 preferences->ScrollOnOutput);
}

static void
mud_window_prefs_update_encoding_combo(MudWindowPrefs *self,
                                       MudPrefs *preferences)
{
    GtkTreeModel *encodings =
        gtk_combo_box_get_model(GTK_COMBO_BOX(self->priv->encoding_combo));
    GtkTreeIter iter;
    gboolean valid;
    gint count = 0;

    valid = gtk_tree_model_get_iter_first(encodings, &iter);

    if(!preferences->Encoding)
        return;

    while(valid)
    {
        gchar *encoding;

        gtk_tree_model_get(encodings, &iter, 0, &encoding, -1);

        if(!encoding)
            continue;

        if(g_str_equal(encoding, preferences->Encoding))
            break;

        count++;

        valid = gtk_tree_model_iter_next(encodings, &iter);
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(self->priv->encoding_combo),
                             count);
}

static void
mud_window_prefs_update_keeptext(MudWindowPrefs *self,
                                 MudPrefs *preferences)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->priv->keep_check),
                                 preferences->KeepText);
}

static void
mud_window_prefs_update_echotext(MudWindowPrefs *self,
                                 MudPrefs *preferences)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->priv->echo_check),
                                 preferences->EchoText);
}

static void
mud_window_prefs_update_scrollback(MudWindowPrefs *self,
                                   MudPrefs *preferences)
{
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(self->priv->lines_spin),
                              preferences->Scrollback);
}

static void
mud_window_prefs_update_font(MudWindowPrefs *self,
                             MudPrefs *preferences)
{
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(self->priv->font_button),
                                  preferences->FontName);
}

static void
mud_window_prefs_update_foreground(MudWindowPrefs *self,
                                   MudPrefs *preferences)
{
    GdkColor color;

    color.red = preferences->Foreground.red;
    color.green = preferences->Foreground.green;
    color.blue = preferences->Foreground.blue;

    gtk_color_button_set_color(GTK_COLOR_BUTTON(self->priv->fore_button),
                               &color);
}

static void
mud_window_prefs_update_background(MudWindowPrefs *self,
                                   MudPrefs *preferences)
{
    GdkColor color;

    color.red = preferences->Background.red;
    color.green = preferences->Background.green;
    color.blue = preferences->Background.blue;

    gtk_color_button_set_color(GTK_COLOR_BUTTON(self->priv->back_button),
                               &color);
}

static void
mud_window_prefs_update_colors(MudWindowPrefs *self,
                               MudPrefs *preferences)
{
    gint i;
    GdkColor color;

    for (i = 0; i < C_MAX; i++)
    {
        color.red = preferences->Colors[i].red;
        color.green = preferences->Colors[i].green;
        color.blue = preferences->Colors[i].blue;

        gtk_color_button_set_color(GTK_COLOR_BUTTON(self->priv->colors[i]),
                                   &color);
    }
}

// Callbacks
static void
mud_window_prefs_scrolloutput_cb(GtkWidget *widget,
                                 MudWindowPrefs *self)
{
    gboolean value = GTK_TOGGLE_BUTTON(widget)->active ? TRUE : FALSE;
    RETURN_IF_CHANGING_PROFILES(self);

    mud_profile_set_scrolloutput(self->priv->mud_profile, value);
}

static void
mud_window_prefs_keeptext_cb(GtkWidget *widget,
                             MudWindowPrefs *self)
{
    gboolean value = GTK_TOGGLE_BUTTON(widget)->active ? TRUE : FALSE;
    RETURN_IF_CHANGING_PROFILES(self);

    mud_profile_set_keeptext(self->priv->mud_profile, value);
}

static void
mud_window_prefs_echo_cb(GtkWidget *widget,
                         MudWindowPrefs *self)
{
    gboolean value = GTK_TOGGLE_BUTTON(widget)->active ? TRUE : FALSE;
    RETURN_IF_CHANGING_PROFILES(self);

    mud_profile_set_echotext(self->priv->mud_profile, value);
}

static void
mud_window_prefs_commdev_cb(GtkWidget *widget,
                            MudWindowPrefs *self)
{

    const gchar *s = gtk_entry_get_text(GTK_ENTRY(widget));
    RETURN_IF_CHANGING_PROFILES(self);

    mud_profile_set_commdev(self->priv->mud_profile, s);
}

static void
mud_window_prefs_encoding_combo_cb(GtkWidget *widget,
                                   MudWindowPrefs *self)
{
    const gchar *s = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));
    RETURN_IF_CHANGING_PROFILES(self);

    mud_profile_set_encoding_combo(self->priv->mud_profile, s);
}

static void
mud_window_prefs_scrollback_cb(GtkWidget *widget,
                               MudWindowPrefs *self)
{
    /* TODO: Looks like a rather risky integer cast */
    const guint value = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
    RETURN_IF_CHANGING_PROFILES(self);

    mud_profile_set_scrollback(self->priv->mud_profile, value);
}

static void
mud_window_prefs_font_cb(GtkWidget *widget,
                         MudWindowPrefs *self)
{
    const gchar *fontname = gtk_font_button_get_font_name(GTK_FONT_BUTTON(widget));

    RETURN_IF_CHANGING_PROFILES(self);
    mud_profile_set_font(self->priv->mud_profile, fontname);
}

static void
mud_window_prefs_foreground_cb(GtkWidget *widget,
                               MudWindowPrefs *self)
{
    GdkColor color;

    RETURN_IF_CHANGING_PROFILES(self);

    gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), &color);
    mud_profile_set_foreground(self->priv->mud_profile, &color);
}

static void
mud_window_prefs_background_cb(GtkWidget *widget,
                               MudWindowPrefs *self)
{
    GdkColor color;

    RETURN_IF_CHANGING_PROFILES(self);

    gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), &color);
    mud_profile_set_background(self->priv->mud_profile, &color);
}

static void
mud_window_prefs_colors_cb(GtkWidget *widget,
                           MudWindowPrefs *self)
{
    gint i;
    GdkColor color;

    RETURN_IF_CHANGING_PROFILES(self);

    for (i = 0; i < C_MAX; i++)
    {
        if (widget == self->priv->colors[i])
        {
            gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), &color);
            mud_profile_set_colors(self->priv->mud_profile, i, &color);
        }
    }
}

/************************ Network Tab ************************/
static void
mud_window_prefs_construct_network_tab(MudWindowPrefs *self)
{
    g_signal_connect(self->priv->charset_check,
                     "toggled",
                     G_CALLBACK(mud_window_prefs_encoding_check_cb),
                     self);

    g_signal_connect(self->priv->proxy_check,
                     "toggled",
                     G_CALLBACK(mud_window_prefs_proxy_check_cb),
                     self);

    g_signal_connect(self->priv->proxy_combo,
                     "changed",
                     G_CALLBACK(mud_window_prefs_proxy_combo_cb),
                     self);

    g_signal_connect(self->priv->proxy_entry,
                     "changed",
                     G_CALLBACK(mud_window_prefs_proxy_entry_cb),
                     self);

    g_signal_connect(self->priv->msp_check,
                     "toggled",
                     G_CALLBACK(mud_window_prefs_msp_check_cb),
                     self);
}

// Update Functions
static void
mud_window_prefs_update_proxy_check(MudWindowPrefs *self,
                                    MudPrefs *preferences)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->priv->proxy_check),
                                 preferences->UseProxy);

}

static void
mud_window_prefs_update_encoding_check(MudWindowPrefs *self,
                                       MudPrefs *preferences)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->priv->charset_check),
                                 preferences->UseRemoteEncoding);

}

static void
mud_window_prefs_update_msp_check(MudWindowPrefs *self,
                                  MudPrefs *preferences)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->priv->msp_check),
                                 preferences->UseRemoteDownload);
}

static void
mud_window_prefs_update_proxy_combo(MudWindowPrefs *self,
                                    MudPrefs *preferences)
{
    gchar *version;
    gint active;
    gint current;

    version = g_settings_get_string(self->priv->mud_profile->settings, "proxy-socks-version");

    if(version)
    {
        current = gtk_combo_box_get_active(GTK_COMBO_BOX(self->priv->proxy_combo));

        if(strcmp(version,"4") == 0)
            active = 0;
        else
            active = 1;


        if(current != active)
            gtk_combo_box_set_active(GTK_COMBO_BOX(self->priv->proxy_combo),
                                     active);
    }

    g_free(version);
}

static void
mud_window_prefs_update_proxy_entry(MudWindowPrefs *self,
                                    MudPrefs *preferences)
{
    if(preferences->ProxyHostname)
        gtk_entry_set_text(GTK_ENTRY(self->priv->proxy_entry),
                           preferences->ProxyHostname);

}

// Callbacks
static void
mud_window_prefs_encoding_check_cb(GtkWidget *widget,
                                   MudWindowPrefs *self)
{
    gboolean value = GTK_TOGGLE_BUTTON(widget)->active ? TRUE : FALSE;
    RETURN_IF_CHANGING_PROFILES(self);

    mud_profile_set_encoding_check(self->priv->mud_profile, value);
}

static void
mud_window_prefs_proxy_check_cb(GtkWidget *widget,
                                MudWindowPrefs *self)
{
    gboolean value = GTK_TOGGLE_BUTTON(widget)->active ? TRUE : FALSE;

    gtk_widget_set_sensitive(self->priv->proxy_entry, value);
    gtk_widget_set_sensitive(self->priv->proxy_combo, value);

    RETURN_IF_CHANGING_PROFILES(self);

    mud_profile_set_proxy_check(self->priv->mud_profile, value);
}

static void
mud_window_prefs_msp_check_cb(GtkWidget *widget,
                              MudWindowPrefs *self)
{
    gboolean value = GTK_TOGGLE_BUTTON(widget)->active ? TRUE : FALSE;
    RETURN_IF_CHANGING_PROFILES(self);

    mud_profile_set_msp_check(self->priv->mud_profile, value);
}

static void
mud_window_prefs_proxy_combo_cb(GtkWidget *widget,
                                MudWindowPrefs *self)
{
    RETURN_IF_CHANGING_PROFILES(self);

    mud_profile_set_proxy_combo(self->priv->mud_profile, GTK_COMBO_BOX(widget));
}

static void
mud_window_prefs_proxy_entry_cb(GtkWidget *widget,
                                MudWindowPrefs *self)
{
    const gchar *s = gtk_entry_get_text(GTK_ENTRY(widget));
    RETURN_IF_CHANGING_PROFILES(self);

    if(s)
        mud_profile_set_proxy_entry(self->priv->mud_profile, s);
}

/************************ Triggers Tab ************************/
static void
mud_window_prefs_construct_triggers_tab(MudWindowPrefs *self)
{
}

/************************ Aliases Tab ************************/
static void
mud_window_prefs_construct_aliases_tab(MudWindowPrefs *self)
{
}

/************************ Variables Tab ************************/
static void
mud_window_prefs_construct_variables_tab(MudWindowPrefs *self)
{
}

/************************ Timers Tab ************************/
static void
mud_window_prefs_construct_timers_tab(MudWindowPrefs *self)
{
}

/************************ Keybindings Tab ************************/
static void
mud_window_prefs_construct_keybindings_tab(MudWindowPrefs *self)
{

}

