/* GNOME-Mud - A simple Mud Client
 * mud-profile.c
 * Copyright (C) 1998-2005 Robin Ericsson <lobbin@localhost.nu>
 * Copyright (C) 2005-2009 Les Harris <lharris@gnome.org>
 * Copyright (C) 2018 Mart Raudsepp <leio@gentoo.org>
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

#include <string.h>

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <glib/gprintf.h>

#include "mud-profile-manager.h"
#include "mud-profile.h"
#include "utils.h"

#define I_(string) g_intern_static_string (string)

struct _MudProfilePrivate
{
    guint settings_changed_id;

    MudPrefs preferences;
    guint gconf_signal;

    MudProfileManager *parent;
};

/* Create the Type */
G_DEFINE_TYPE(MudProfile, mud_profile, G_TYPE_OBJECT);

/* Property Identifiers */
enum
{
    PROP_MUD_PROFILE_0,
    PROP_ID,
    PROP_NAME,
    PROP_PARENT
};

/* Signal Indices */
enum
{
    CHANGED,
    LAST_SIGNAL
};

/* Signal Identifier Map */
static guint mud_profile_signal[LAST_SIGNAL] = { 0 };

/* Class Functions */
static void mud_profile_init          (MudProfile *profile);
static void mud_profile_class_init    (MudProfileClass *profile);
static void mud_profile_finalize      (GObject *object);
static GObject *mud_profile_constructor (GType gtype,
                                         guint n_properties,
                                         GObjectConstructParam *properties);
static void mud_profile_set_property(GObject *object,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *pspec);
static void mud_profile_get_property(GObject *object,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *pspec);

/* Callbacks */
static void mud_profile_settings_changed_cb (GSettings *settings, const char *prop_name, MudProfile *mud_profile);

/* Profile Set Functions */
static gboolean set_FontName(MudProfile *profile, const gchar *candidate);
static gboolean set_CommDev(MudProfile *profile, const gchar *candidate);
static gboolean set_Scrollback(MudProfile *profile, const guint candidate);
static gboolean set_ProxyVersion(MudProfile *profile, const gchar *candidate);
static gboolean set_ProxyHostname(MudProfile *profile, const gchar *candidate);
static gboolean set_Encoding(MudProfile *profile, const gchar *candidate);
static gboolean set_Foreground(MudProfile *profile, const gchar *candidate);
static gboolean set_Background(MudProfile *profile, const gchar *candidate);
static gboolean set_Colors(MudProfile *profile, gchar **palette);

const gchar *
mud_profile_get_name (MudProfile *self)
{
  g_return_val_if_fail (IS_MUD_PROFILE (self), NULL);

  return self->visible_name;
}

/* Private Methods */
static void mud_profile_set_proxy_combo_full(MudProfile *profile, gchar *version);
static void mud_profile_load_preferences(MudProfile *profile);

/* MudProfile Class Functions */
static void
mud_profile_class_init (MudProfileClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    /* Override base object constructor */
    object_class->constructor = mud_profile_constructor;

    /* Override base object finalize */
    object_class->finalize = mud_profile_finalize;

    /* Override base object property methods */
    object_class->set_property = mud_profile_set_property;
    object_class->get_property = mud_profile_get_property;

    /* Add private data to class */
    g_type_class_add_private(klass, sizeof(MudProfilePrivate));

    /* Create and Install Properties */
    g_object_class_install_property(object_class,
            PROP_ID,
            g_param_spec_uint64("id", "ID",
                                "The ID of the profile.",
                                0, G_MAXUINT64, G_MAXUINT64,
                                G_PARAM_READWRITE|G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property(object_class,
            PROP_NAME,
            g_param_spec_string("name",
                "Name",
                "The visible name of the profile.",
                NULL,
                G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property(object_class,
            PROP_PARENT,
            g_param_spec_object("parent",
                "Parent",
                "The parent MudProfileManager",
                MUD_TYPE_PROFILE_MANAGER,
                G_PARAM_READWRITE|G_PARAM_CONSTRUCT_ONLY));

    /* Register Signals */
    mud_profile_signal[CHANGED] =
        g_signal_new("changed",
                G_OBJECT_CLASS_TYPE(object_class),
                G_SIGNAL_RUN_LAST,
                G_STRUCT_OFFSET(MudProfileClass, changed),
                NULL, NULL,
                g_cclosure_marshal_VOID__POINTER,
                G_TYPE_NONE, 1, G_TYPE_POINTER);
}

static void
mud_profile_init (MudProfile *profile)
{
    /* Set private data */
    profile->priv = MUD_PROFILE_GET_PRIVATE(profile);

    /* Set defaults */ 
    profile->id = 0;
    profile->settings = NULL;

    profile->priv->parent = NULL;
    
    profile->priv->settings_changed_id = 0;
    profile->preferences = &profile->priv->preferences;
}

static GObject *
mud_profile_constructor (GType gtype,
                         guint n_properties,
                         GObjectConstructParam *properties)
{
    MudProfile *self;
    GObject *obj;
    MudProfileClass *klass;
    GObjectClass *parent_class;
    gchar *profile_path;

    /* Chain up to parent constructor */
    klass = MUD_PROFILE_CLASS( g_type_class_peek(MUD_TYPE_PROFILE) );
    parent_class = G_OBJECT_CLASS( g_type_class_peek_parent(klass) );
    obj = parent_class->constructor(gtype, n_properties, properties);

    /* TODO: Move most of this to constucted? */
    self = MUD_PROFILE(obj);

    if(self->id == G_MAXUINT64)
    {
        g_printf("ERROR: Tried to instantiate MudProfile without passing ID.\n");
        g_error("Tried to instantiate MudProfile without passing ID.");
    }

    if(!self->priv->parent || !MUD_IS_PROFILE_MANAGER(self->priv->parent))
    {
        g_printf("ERROR: Tried to instantiate MudProfile without passing parent manager.\n");
        g_error("Tried to instantiate MudProfile without passing parent manager.");
    }

    profile_path = g_strdup_printf("/org/gnome/MUD/Profiles/profile-%" G_GUINT64_FORMAT "/", self->id);
    self->settings = g_settings_new_with_path("org.gnome.MUD.Profile", profile_path);
    g_settings_bind(self->settings,
                    "visible-name",
                    self, "name",
                    G_SETTINGS_BIND_DEFAULT);
    g_free(profile_path);

    mud_profile_load_preferences (self); /* Is this necessary at all if mud_profile_settings_changed_cb gets called? */

    self->priv->settings_changed_id =
        g_signal_connect(self->settings,
                         "changed",
                         G_CALLBACK (mud_profile_settings_changed_cb),
                         self);
    mud_profile_settings_changed_cb (self->settings, NULL, self);

    return obj;
}

static void
mud_profile_finalize (GObject *object)
{
    MudProfile *self;
    GObjectClass *parent_class;

    self = MUD_PROFILE (object);

    g_signal_handler_disconnect (self->settings, self->priv->settings_changed_id);
    self->priv->settings_changed_id = 0;
    g_object_unref (self->settings);

    g_free (self->priv->preferences.FontName);
    g_free (self->priv->preferences.CommDev);
    g_free (self->priv->preferences.Encoding);
    g_free (self->priv->preferences.ProxyVersion);
    g_free (self->priv->preferences.ProxyHostname);

    parent_class = g_type_class_peek_parent (G_OBJECT_GET_CLASS(object));
    parent_class->finalize (object);
}

static void
mud_profile_set_property(GObject *object,
                         guint prop_id,
                         const GValue *value,
                         GParamSpec *pspec)
{
    MudProfile *self;

    self = MUD_PROFILE(object);

    switch(prop_id)
    {
        case PROP_ID:
            self->id = g_value_get_uint64(value);
            break;

        case PROP_NAME:
          {
            const gchar *new_string = g_value_get_string(value);
            /* TODO: Move this to an EXPLICIT_NOTIFY setter with notify_by_pspec only if changed logic */
            g_free (self->visible_name);
            self->visible_name = g_strdup (new_string);
            break;
          }

        case PROP_PARENT:
            self->priv->parent = MUD_PROFILE_MANAGER(g_value_get_object(value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void
mud_profile_get_property(GObject *object,
                         guint prop_id,
                         GValue *value,
                         GParamSpec *pspec)
{
    MudProfile *self;

    self = MUD_PROFILE(object);

    switch(prop_id)
    {
        case PROP_ID:
            g_value_set_uint64(value, self->id);
            break;

        case PROP_NAME:
            g_value_set_string(value, self->visible_name);
            break;

        case PROP_PARENT:
            g_value_take_object(value, self->priv->parent);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

/* MudProfile Public Methods */
void
mud_profile_copy_preferences(MudProfile *from, MudProfile *to)
{
#if 0
/* TODO: Port and reintroduce for a "clone profile" feature (previously this was mainly used to create new profile from "Default" */
    gint i;
    gchar *s;
    const gchar *key;

    g_return_if_fail(IS_MUD_PROFILE(from));
    g_return_if_fail(IS_MUD_PROFILE(to));

    mud_profile_set_echotext(to, from->preferences->EchoText);
    mud_profile_set_keeptext(to, from->preferences->KeepText);
    mud_profile_set_scrolloutput(to, from->preferences->ScrollOnOutput);
    mud_profile_set_commdev(to, from->preferences->CommDev);
    mud_profile_set_scrollback(to, from->preferences->Scrollback);
    mud_profile_set_font(to, from->preferences->FontName);
    mud_profile_set_foreground(to, from->preferences->Foreground);
    mud_profile_set_background(to, from->preferences->Background);

    for (i = 0; i < C_MAX; i++)
    {
        to->preferences->Colors[i].red = from->preferences->Colors[i].red;
        to->preferences->Colors[i].blue = from->preferences->Colors[i].blue;
        to->preferences->Colors[i].green = from->preferences->Colors[i].green;
    }
    key = mud_profile_gconf_get_key(to, "ui/palette");
    s = gtk_color_selection_palette_to_string(to->preferences->Colors, C_MAX);
    gconf_client_set_string(from->priv->gconf_client, key, s, NULL);

    mud_profile_set_encoding_combo(to, from->preferences->Encoding);
    mud_profile_set_encoding_check(to, from->preferences->UseRemoteEncoding);
    mud_profile_set_proxy_check(to, from->preferences->UseProxy);
    mud_profile_set_msp_check(to, from->preferences->UseRemoteDownload);
    mud_profile_set_proxy_combo_full(to, from->preferences->ProxyVersion);
    mud_profile_set_proxy_entry(to, from->preferences->ProxyHostname);
#endif
}

/* TODO: Can't we just get rid of this? */
static void
mud_profile_load_preferences (MudProfile *profile)
{
  MudPrefs *prefs;
  gchar *color_string;
  gchar **color_palette;
  GdkRGBA color;

  /* TODO: Get rid of this - currently needed due to keeping a copy of setting values in profile->preferences */
  prefs = profile->preferences;

  /* FIXME: Many of them will get out of sync on first set, as many don't go via setters that update prefs, just GSettings */
  prefs->EchoText = g_settings_get_boolean (profile->settings, "echo");
  prefs->KeepText = g_settings_get_boolean (profile->settings, "keep-text");
  prefs->ScrollOnOutput = g_settings_get_boolean (profile->settings, "scroll-on-output");

  prefs->FontName = g_settings_get_string (profile->settings, "font");
  prefs->CommDev = g_settings_get_string (profile->settings, "command-divider");
  prefs->Encoding = g_settings_get_string (profile->settings, "encoding");
  prefs->ProxyVersion = g_settings_get_string (profile->settings, "proxy-socks-version");
  prefs->ProxyHostname = g_settings_get_string (profile->settings, "proxy-hostname");

  prefs->Scrollback = g_settings_get_uint (profile->settings, "scrollback-lines");

  color_string = g_settings_get_string (profile->settings, "foreground-color");
  if (color_string && gdk_rgba_parse (&color, color_string))
    prefs->Foreground = color;
  g_free (color_string);

  color_string = g_settings_get_string (profile->settings, "background-color");
  if (color_string && gdk_rgba_parse (&color, color_string))
    prefs->Background = color;
  g_free (color_string);

  prefs->UseRemoteEncoding = g_settings_get_boolean (profile->settings, "remote-encoding");
  prefs->UseProxy = g_settings_get_boolean (profile->settings, "use-proxy");
  prefs->UseRemoteDownload = g_settings_get_boolean (profile->settings, "remote-download");

  color_palette = g_settings_get_strv (profile->settings, "palette");
  set_Colors (profile, color_palette);
  g_strfreev (color_palette);
}

/* MudProfile Private Methods */
static void
mud_profile_settings_changed_cb (GSettings *settings,
                                 const char *prop_name,
                                 MudProfile *profile)
{
  MudProfileMask mask = {FALSE, };
  gboolean bool_setting;
  gchar *str_setting;

  /* TODO: freeze_notify and thaw? */

#define SETTINGS_STRING(pref, key)                        \
  do { if (!prop_name || prop_name == I_(key))            \
    {                                                     \
      str_setting = g_settings_get_string(settings, key); \
      mask.pref = set_##pref(profile, str_setting);       \
      g_free (str_setting);                               \
    }                                                     \
  } while (0)

#define SETTINGS_BOOLEAN(pref, key)                           \
  do { if (!prop_name || prop_name == I_(key))                \
    {                                                         \
      bool_setting = g_settings_get_boolean(settings, key);   \
      if(bool_setting != profile->priv->preferences.pref)     \
        {                                                     \
          mask.pref = TRUE;                                   \
          profile->priv->preferences.pref = bool_setting;     \
        }                                                     \
    }                                                         \
  } while (0)

  /* UI */
  SETTINGS_STRING (FontName, "font");
  SETTINGS_STRING (Foreground, "foreground-color");
  SETTINGS_STRING (Background, "background-color");

  if (!prop_name || prop_name == I_("palette"))
    {
      gchar **palette = g_settings_get_strv (settings, "palette");
      mask.Colors = set_Colors(profile, palette);
      g_strfreev (palette);
    }

  if (!prop_name || prop_name == I_("scrollback-lines"))
    mask.Scrollback = set_Scrollback(profile, g_settings_get_uint(settings, "scrollback-lines"));

  /* Functionality */
  SETTINGS_STRING  (Encoding,          "encoding");
  SETTINGS_BOOLEAN (UseRemoteEncoding, "remote-encoding");
  SETTINGS_BOOLEAN (UseRemoteDownload, "remote-download");
  SETTINGS_STRING  (CommDev,           "command-divider");
  SETTINGS_BOOLEAN (EchoText,          "echo");
  SETTINGS_BOOLEAN (KeepText,          "keep-text");
  SETTINGS_BOOLEAN (ScrollOnOutput,    "scroll-on-output");
  SETTINGS_BOOLEAN (UseProxy,          "use-proxy");
  SETTINGS_STRING  (ProxyVersion,      "proxy-socks-version");
  SETTINGS_STRING  (ProxyHostname,     "proxy-hostname");

#undef SETTINGS_STRING
#undef SETTINGS_BOOLEAN

  g_signal_emit(profile, mud_profile_signal[CHANGED], 0, &mask);
}


/* Profile Set Functions */
static gboolean
set_FontName(MudProfile *profile, const gchar *candidate)
{
    if (candidate && strcmp(profile->priv->preferences.FontName, candidate) == 0)
        return FALSE;

    if (candidate != NULL)
    {
        g_free(profile->priv->preferences.FontName);
        profile->priv->preferences.FontName = g_strdup(candidate);
        return TRUE;
    }

    return FALSE;
}

static gboolean
set_CommDev(MudProfile *profile, const gchar *candidate)
{
    if (candidate && strcmp(profile->priv->preferences.CommDev, candidate) == 0)
        return FALSE;

    if (candidate != NULL)
    {
        g_free(profile->priv->preferences.CommDev);
        profile->priv->preferences.CommDev = g_strdup(candidate);
        return TRUE;
    }

    return FALSE;
}

static gboolean
set_Scrollback(MudProfile *profile, const guint candidate)
{
    if (candidate >= 1 && candidate != profile->priv->preferences.Scrollback)
    {
        profile->priv->preferences.Scrollback = candidate;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static gboolean
set_ProxyVersion(MudProfile *profile, const gchar *candidate)
{

    if (candidate && strcmp(profile->priv->preferences.ProxyVersion, candidate) == 0)
        return FALSE;

    if (candidate != NULL)
    {
        g_free(profile->priv->preferences.ProxyVersion);
        profile->priv->preferences.ProxyVersion = g_strdup(candidate);
        return TRUE;
    }

    return FALSE;
}

static gboolean
set_ProxyHostname(MudProfile *profile, const gchar *candidate)
{
    if (candidate && strcmp(profile->priv->preferences.ProxyHostname, candidate) == 0)
        return FALSE;

    if (candidate != NULL)
    {
        g_free(profile->priv->preferences.ProxyHostname);
        profile->priv->preferences.ProxyHostname = g_strdup(candidate);
        return TRUE;
    }

    return FALSE;
}

static gboolean
set_Encoding(MudProfile *profile, const gchar *candidate)
{
    if (candidate && strcmp(profile->priv->preferences.Encoding, candidate) == 0)
        return FALSE;

    if (candidate != NULL)
    {
        g_free(profile->priv->preferences.Encoding);
        profile->priv->preferences.Encoding = g_strdup(candidate);
        return TRUE;
    }

    return FALSE;
}

/* TODO: Remove all this and rely on object/signal bindings, or just set directly in all callers */
void
mud_profile_set_scrolloutput (MudProfile *profile, gboolean value)
{
    g_settings_set_boolean(profile->settings, "scroll-on-output", value);
}

void
mud_profile_set_keeptext (MudProfile *profile, gboolean value)
{
    g_settings_set_boolean(profile->settings, "keep-text", value);
}

void
mud_profile_set_echotext (MudProfile *profile, gboolean value)
{
    g_settings_set_boolean(profile->settings, "echo", value);
}

void
mud_profile_set_commdev (MudProfile *profile, const gchar *value)
{
    g_settings_set_string(profile->settings, "command-divider", value);
}

void
mud_profile_set_encoding_combo(MudProfile *profile, const gchar *value)
{
    g_settings_set_string(profile->settings, "encoding", value);
}

void
mud_profile_set_encoding_check (MudProfile *profile, gboolean value)
{
    g_settings_set_boolean(profile->settings, "remote-encoding", value);
}

void
mud_profile_set_proxy_check (MudProfile *profile, const gboolean value)
{
    g_settings_set_boolean(profile->settings, "use-proxy", value);
}

void
mud_profile_set_msp_check (MudProfile *profile, const gboolean value)
{
    g_settings_set_boolean(profile->settings, "remote-download", value);
}

static void
mud_profile_set_proxy_combo_full(MudProfile *profile, gchar *version)
{
    g_settings_set_string(profile->settings, "proxy-socks-version", version);
}

void
mud_profile_set_proxy_combo(MudProfile *profile, GtkComboBox *combo)
{
    gchar *version = gtk_combo_box_get_active_id(combo);
    mud_profile_set_proxy_combo_full(profile, version);
}

void
mud_profile_set_proxy_entry (MudProfile *profile, const gchar *value)
{
    if(value)
        g_settings_set_string(profile->settings, "proxy-hostname", value);
    else
        g_settings_set_string(profile->settings, "proxy-hostname", "");
}

void
mud_profile_set_font (MudProfile *profile, const gchar *value)
{
    g_settings_set_string(profile->settings, "font", value);
}

void
mud_profile_set_foreground (MudProfile *profile, GdkRGBA *color)
{
    gchar *s = gdk_rgba_to_string(color);
    g_settings_set_string(profile->settings, "foreground-color", s);
    g_free(s);
}

void
mud_profile_set_background (MudProfile *profile, GdkRGBA *color)
{
    gchar *s = gdk_rgba_to_string(color);
    g_settings_set_string(profile->settings, "background-color", s);
    g_free(s);
}

void
mud_profile_set_colors (MudProfile *profile,
                        gint        nr,
                        GdkRGBA    *color)
{
  gchar **value;
  gchar *s = gdk_rgba_to_string (color);
  /* FIXME: Guard against retrieved old value not having 16 entries */
  value = g_settings_get_strv (profile->settings, "palette");
  g_free (value[nr]); /* FIXME: Guard against wrong 'nr' value */
  value[nr] = s;
  g_settings_set_strv (profile->settings, "palette", (const gchar * const *)value);

  g_strfreev(value);
}

void
mud_profile_set_scrollback(MudProfile *profile, const guint value)
{
    g_settings_set_uint(profile->settings, "scrollback-lines", value);
}

static gboolean
set_Foreground(MudProfile *profile, const gchar *candidate)
{
    GdkRGBA color;

    if (candidate && gdk_rgba_parse(&color, candidate))
    {
        if (!gdk_rgba_equal(&color, &profile->priv->preferences.Foreground))
        {
            profile->priv->preferences.Foreground = color;
            return TRUE;
        }
    }

    return FALSE;
}

static gboolean
set_Background(MudProfile *profile, const gchar *candidate)
{
    GdkRGBA color;

    if (candidate && gdk_rgba_parse(&color, candidate))
    {
        if (!gdk_rgba_equal(&color, &profile->priv->preferences.Background))
        {
            profile->priv->preferences.Background = color;
            return TRUE;
        }
    }

    return FALSE;
}

static gboolean
set_Colors (MudProfile *profile, gchar **palette)
{
  guint n_colors = (palette == NULL) ? 0 : g_strv_length (palette);
  GdkRGBA colors[C_MAX];

  if ((palette == NULL) || (n_colors < C_MAX))
    {
      g_printerr (ngettext ("Palette had %u entry instead of %u\n",
                            "Palette had %u entries instead of %u\n",
                            n_colors),
                  n_colors, C_MAX);

      return FALSE;
    }

  for (gsize i = 0; i < C_MAX; ++i)
    if (!gdk_rgba_parse (&colors[i], palette[i]))
      return FALSE;

  memcpy(profile->priv->preferences.Colors, &colors, C_MAX * sizeof(GdkRGBA));
  return TRUE;
}
