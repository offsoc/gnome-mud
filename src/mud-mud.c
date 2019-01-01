/* mud-mud.c
 *
 * Copyright 2018 Mart Raudsepp <leio@gentoo.org>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "mud-mud.h"

#include <gio/gio.h>

struct _MudMud
{
  GObject    parent_instance;

  gchar     *id;
  gchar     *name;
  gchar     *hostname;
  guint      port;
  GSettings *settings;
};

G_DEFINE_TYPE (MudMud, mud_mud, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_ID,
  PROP_NAME,
  PROP_HOSTNAME,
  PROP_PORT,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

MudMud *
mud_mud_new (const gchar *id)
{
  return g_object_new (MUD_TYPE_MUD, "id", id, NULL);
}

const gchar *
mud_mud_get_id (MudMud *self)
{
  g_return_val_if_fail (MUD_IS_MUD (self), NULL);

  return self->id;
}

const gchar *
mud_mud_get_name (MudMud *self)
{
  g_return_val_if_fail (MUD_IS_MUD (self), NULL);

  return self->name;
}

void
mud_mud_set_name (MudMud *self, const gchar *name)
{
  g_return_if_fail (MUD_IS_MUD (self));

  if (g_strcmp0 (self->name, name) != 0)
    {
      g_free (self->name);
      self->name = g_strdup (name);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
    }
}

const gchar *
mud_mud_get_hostname (MudMud *self)
{
  g_return_val_if_fail (MUD_IS_MUD (self), NULL);

  return self->hostname;
}

void
mud_mud_set_hostname (MudMud *self, const gchar *hostname)
{
  g_return_if_fail (MUD_IS_MUD (self));

  if (g_strcmp0 (self->hostname, hostname) != 0)
    {
      g_free (self->hostname);
      self->hostname = g_strdup (hostname);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_HOSTNAME]);
    }
}

const guint
mud_mud_get_port (MudMud *self)
{
  g_return_val_if_fail (MUD_IS_MUD (self), 23);

  return self->port;
}

void
mud_mud_set_port (MudMud      *self,
                  const guint  port)
{
  g_return_if_fail (MUD_IS_MUD (self));

  self->port = port;
}

gint
mud_mud_compare (const MudMud *mud1, const MudMud *mud2)
{
  return g_strcmp0 (mud1->id, mud2->id);
}

static void
mud_mud_constructed (GObject *object)
{
  gchar *mud_path;
  MudMud *self = (MudMud *)object;

  G_OBJECT_CLASS (mud_mud_parent_class)->constructed (object);

  /* TODO: Change ID property to default to G_MAXUINT64 and figure out a new ID to use here, instead of caller having to do it */

  mud_path = g_strdup_printf("/org/gnome/MUD/MUDs/%s/", self->id);
  self->settings = g_settings_new_with_path("org.gnome.MUD.Mud", mud_path);
  g_settings_bind(self->settings,
                  "name",
                  self, "name",
                  G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_NO_SENSITIVITY);
  g_settings_bind(self->settings,
                  "hostname",
                  self, "hostname",
                  G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_NO_SENSITIVITY);
  g_settings_bind(self->settings,
                  "port",
                  self, "port",
                  G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_NO_SENSITIVITY);
  g_free (mud_path);
}

static void
mud_mud_finalize (GObject *object)
{
  MudMud *self = (MudMud *)object;

  g_object_unref(self->settings); /* TODO: Shouldn't this be done in dispose? */
  g_clear_pointer(&self->name, g_free);
  g_clear_pointer(&self->hostname, g_free);

  G_OBJECT_CLASS (mud_mud_parent_class)->finalize (object);
}

static void
mud_mud_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  MudMud *self = MUD_MUD (object);

  switch (prop_id)
    {
    case PROP_ID:
      g_value_set_string (value, mud_mud_get_id (self));
      break;

    case PROP_NAME:
      g_value_set_string (value, mud_mud_get_name (self));
      break;

    case PROP_HOSTNAME:
      g_value_set_string (value, mud_mud_get_hostname (self));
      break;

    case PROP_PORT:
      g_value_set_uint (value, mud_mud_get_port (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
mud_mud_set_property (GObject      *object,
                      guint         prop_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
  MudMud *self = MUD_MUD (object);

  switch (prop_id)
    {
    case PROP_ID:
      self->id = g_strdup (g_value_get_string (value));
      break;

    case PROP_NAME:
      mud_mud_set_name (self, g_value_get_string (value));
      break;

    case PROP_HOSTNAME:
      mud_mud_set_hostname (self, g_value_get_string (value));
      break;

    case PROP_PORT:
      mud_mud_set_port (self, g_value_get_uint (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
mud_mud_class_init (MudMudClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = mud_mud_constructed;
  object_class->finalize = mud_mud_finalize;
  object_class->get_property = mud_mud_get_property;
  object_class->set_property = mud_mud_set_property;

  /* Create and Install Properties */
  properties [PROP_ID] =
    g_param_spec_string ("id",
                         "ID",
                         "The ID of the MUD.",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "The visible name of the MUD.",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS)); /* TODO: Debug how things differ with EXPLICIT_NOTIFY vs not and notify_by_pspec call existing or not */
  properties [PROP_HOSTNAME] =
    g_param_spec_string ("hostname",
                         "Hostname",
                         "The hostname of the MUD.",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS)); /* TODO: Debug how things differ with EXPLICIT_NOTIFY vs not and notify_by_pspec call existing or not */
  properties [PROP_PORT] =
    g_param_spec_uint ("port",
                       "Port",
                       "The port of the MUD",
                       0, G_MAXUINT, 23,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
mud_mud_init (MudMud *self)
{
}
