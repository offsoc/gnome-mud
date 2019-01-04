/* mud-character.c
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

#include "mud-character.h"
#include "mud-profile.h"
#include "mud-mud.h"

#include <gio/gio.h>

typedef struct _MudProfile MudProfile;
typedef struct _MudMud     MudMud;

struct _MudCharacter
{
  GObject     parent_instance;

  gchar      *id;
  MudProfile *profile;
  MudMud     *mud;
  gchar      *name;
  gchar      *connect_string;
  GSettings  *settings;
};

G_DEFINE_TYPE (MudCharacter, mud_character, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_ID,
  PROP_PROFILE,
  PROP_MUD,
  PROP_NAME,
  PROP_CONNECT_STRING,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

MudCharacter *
mud_character_new (const gchar *id, const MudProfile *profile, const MudMud *mud)
{
  return g_object_new (MUD_TYPE_CHARACTER, "id", id, "profile", profile, "mud", mud, NULL);
}

const gchar *
mud_character_get_id (MudCharacter *self)
{
  g_return_val_if_fail (MUD_IS_CHARACTER (self), NULL);

  return self->id;
}

const gchar *
mud_character_get_name (MudCharacter *self)
{
  g_return_val_if_fail (MUD_IS_CHARACTER (self), NULL);

  return self->name;
}

void
mud_character_set_name (MudCharacter *self, const gchar *name)
{
  g_return_if_fail (MUD_IS_CHARACTER (self));

  if (g_strcmp0 (self->name, name) != 0)
    {
      g_free (self->name);
      self->name = g_strdup (name);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
    }
}

const gchar *
mud_character_get_connect_string (MudCharacter *self)
{
  g_return_val_if_fail (MUD_IS_CHARACTER (self), NULL);

  return self->connect_string;
}

void
mud_character_set_connect_string (MudCharacter *self, const gchar *connect_string)
{
  g_return_if_fail (MUD_IS_CHARACTER (self));

  if (g_strcmp0 (self->connect_string, connect_string) != 0)
    {
      g_free (self->connect_string);
      self->connect_string = g_strdup (connect_string);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CONNECT_STRING]);
    }
}

MudProfile *
mud_character_get_profile (MudCharacter *self)
{
  g_return_val_if_fail (MUD_IS_CHARACTER (self), NULL);

  return self->profile;
}

MudMud *
mud_character_get_mud (MudCharacter *self)
{
  g_return_val_if_fail (MUD_IS_CHARACTER (self), NULL);

  return self->mud;
}

const gchar *
mud_character_get_icon (MudCharacter *self)
{
  g_return_val_if_fail (MUD_IS_CHARACTER (self), NULL);

  /* TODO: Implement */
  return "gnome-mud";
}

void
mud_character_set_icon (MudCharacter *self,
                        const gchar  *icon_name)
{
  g_return_if_fail (MUD_IS_CHARACTER (self));
  /* TODO: Implement */
}

int
mud_character_compare (const MudCharacter *char1, const MudCharacter *char2)
{
  return g_strcmp0 (char1->id, char2->id);
}

static GVariant *
_set_profile_id_mapping (const GValue       *value,
                         const GVariantType *expected_type,
                         gpointer            user_data)
{
  GVariant *ret;
  guint64 id = g_value_get_uint64 (value);
  gchar *profile_id = g_strdup_printf ("profile-%" G_GUINT64_FORMAT, id);
  ret = g_variant_new_string (profile_id);
  g_free (profile_id);
  return ret;
}

static void
mud_character_constructed (GObject *object)
{
  gchar *character_path;
  MudCharacter *self = (MudCharacter *)object;

  G_OBJECT_CLASS (mud_character_parent_class)->constructed (object);

  /* TODO: Change ID property to default to G_MAXUINT64 and figure out a new ID to use here, instead of caller having to do it */

  character_path = g_strdup_printf("/org/gnome/MUD/Characters/%s/", self->id);
  self->settings = g_settings_new_with_path("org.gnome.MUD.Character", character_path);
  /* TODO: Reaction to MUD/Profile changes; but maybe this binding to other objects 'id' is enough? */
  /* FIXME: For now just setting mud/profile names for the benefit of new freshly created characters; think this through more and maybe do elsewhere, as during load we'd get that value already too and figure it out in manager */
  g_settings_bind_with_mapping (self->settings, "profile",
                                self->profile, "id",
                                G_SETTINGS_BIND_SET | G_SETTINGS_BIND_NO_SENSITIVITY,
                                NULL, _set_profile_id_mapping, /* TODO: Get rid of the mapping once we have converted profile over to use full name instead of integer ID too */
                                NULL, NULL);
  g_settings_bind (self->settings, "mud",
                   self->mud, "id",
                   G_SETTINGS_BIND_SET | G_SETTINGS_BIND_NO_SENSITIVITY);
  g_settings_bind (self->settings, "name",
                   self, "name",
                   G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_NO_SENSITIVITY);
  g_settings_bind (self->settings, "connect-string",
                   self, "connect-string",
                   G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_NO_SENSITIVITY);
  g_free (character_path);
}

static void
mud_character_finalize (GObject *object)
{
  MudCharacter *self = (MudCharacter *)object;

  g_object_unref(self->settings); /* TODO: Shouldn't this be done in dispose? */

  G_OBJECT_CLASS (mud_character_parent_class)->finalize (object);
}

static void
mud_character_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  MudCharacter *self = MUD_CHARACTER (object);

  switch (prop_id)
    {
    case PROP_ID:
      g_value_set_string (value, self->id);
      break;

    case PROP_PROFILE:
      g_value_take_object (value, self->profile);
      break;

    case PROP_MUD:
      g_value_take_object (value, G_OBJECT (mud_character_get_mud(self)));
      break;

    case PROP_NAME:
      g_value_set_string (value, mud_character_get_name (self));
      break;

    case PROP_CONNECT_STRING:
      g_value_set_string (value, mud_character_get_connect_string (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
mud_character_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  MudCharacter *self = MUD_CHARACTER (object);

  switch (prop_id)
    {
    case PROP_ID: /* construct-only */
      self->id = g_strdup (g_value_get_string (value));
      break;

    case PROP_PROFILE:
      self->profile = g_value_get_object (value); /* TODO: setter with notify (once we can change profile again) */
      break;

    case PROP_MUD: /* construct-only */
      self->mud = g_value_get_object (value);
      break;

    case PROP_NAME:
      mud_character_set_name (self, g_value_get_string (value));
      break;

    case PROP_CONNECT_STRING:
      mud_character_set_connect_string (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
mud_character_class_init (MudCharacterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = mud_character_constructed;
  object_class->finalize = mud_character_finalize;
  object_class->get_property = mud_character_get_property;
  object_class->set_property = mud_character_set_property;

  /* Create and Install Properties */
  properties [PROP_ID] =
    g_param_spec_string ("id",
                         "ID",
                         "The ID of the MUD character.",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_PROFILE] =
    g_param_spec_object ("profile",
                         "Profile",
                         "Profile used by this MUD character.",
                         MUD_TYPE_PROFILE,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS)); /* TODO: Make changing of profile possible again */
  properties [PROP_MUD] =
    g_param_spec_object ("mud",
                         "MUD",
                         "The MUD this character is for.",
                         MUD_TYPE_MUD,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  properties [PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "The visible name of the MUD character.",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS)); /* TODO: Debug how things differ with EXPLICIT_NOTIFY vs not and notify_by_pspec call existing or not */
  properties [PROP_CONNECT_STRING] =
    g_param_spec_string ("connect-string",
                         "Connect String",
                         "Commands to send to MUD upon establishing connection.",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS)); /* TODO: Debug how things differ with EXPLICIT_NOTIFY vs not and notify_by_pspec call existing or not */

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
mud_character_init (MudCharacter *self)
{
}
