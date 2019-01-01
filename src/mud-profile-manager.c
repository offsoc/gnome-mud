/* GNOME-Mud - A simple Mud Client
 * mud-profile-manager.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>

#include "gnome-mud.h"
#include "mud-window.h"
#include "mud-profile-manager.h"
#include "mud-connection-view.h"
#include "mud-character.h"
#include "mud-mud.h"

/* TODO: This now manages not only profiles, but also the list of known MUDs and characters; so either rename it all to more appropriate, or at some point refactor it into nicer small structs or singleton objects */

/* TODO: Move to G_DEFINE_TYPE and G_DECLARE_FINAL_TYPE (no private if possible) */
struct _MudProfileManagerPrivate
{
    GSequence *profiles;
    GSequence *muds;
    GSequence *characters;
    MudWindow *parent;
};

static MudProfileManager * manager_singleton = NULL; /* TODO: Just return this if possible in our still to be created mud_profile_manager_init too */

/* Property Identifiers */
enum
{
    PROP_MUD_PROFILE_MANAGER_0,
    PROP_PARENT
};

/* Create the Type */
G_DEFINE_TYPE(MudProfileManager, mud_profile_manager, G_TYPE_OBJECT);

/* Class Functions */
static void mud_profile_manager_init (MudProfileManager *profile_manager);
static void mud_profile_manager_class_init (MudProfileManagerClass *klass);
static void mud_profile_manager_finalize (GObject *object);
static GObject *mud_profile_manager_constructor (GType gtype,
                                                 guint n_properties,
                                                 GObjectConstructParam *properties);
static void mud_profile_manager_set_property(GObject *object,
                                             guint prop_id,
                                             const GValue *value,
                                             GParamSpec *pspec);
static void mud_profile_manager_get_property(GObject *object,
                                             guint prop_id,
                                             GValue *value,
                                             GParamSpec *pspec);

/* Private Methods */
static void mud_profile_manager_load_profiles (MudProfileManager *self);
static void mud_profile_manager_load_muds (MudProfileManager *self);
static void mud_profile_manager_load_characters (MudProfileManager *self);

static gint
mud_profile_compare_func(MudProfile *profile1, MudProfile *profile2, gpointer userdata)
{
    if (profile1->id < profile2->id)
        return -1;
    else if (profile1->id > profile2->id)
        return 1;
    else {
        g_assert_not_reached();
        return 0;
    }
}

/* MudProfileManager class functions */
static void
mud_profile_manager_class_init (MudProfileManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    /* Override base object constructor */
    object_class->constructor = mud_profile_manager_constructor;

    /* Override base object's finalize */
    object_class->finalize = mud_profile_manager_finalize;

    /* Override base object property methods */
    object_class->set_property = mud_profile_manager_set_property;
    object_class->get_property = mud_profile_manager_get_property;

    /* Add private data to class */
    g_type_class_add_private(klass, sizeof(MudProfileManagerPrivate));

    /* Install Properties */
    g_object_class_install_property(object_class,
            PROP_PARENT,
            g_param_spec_object("parent-window",
                "parent MudWindow",
                "The parent MudWindow",
                MUD_TYPE_WINDOW,
                G_PARAM_READWRITE|G_PARAM_CONSTRUCT_ONLY));

}

static void
mud_profile_manager_init (MudProfileManager *self)
{
    /* Get our private data */
    self->priv = MUD_PROFILE_MANAGER_GET_PRIVATE(self);

    /* set private members to defaults */
    self->priv->profiles = g_sequence_new(g_object_unref);
    self->priv->muds = g_sequence_new(g_object_unref);
    self->priv->characters = g_sequence_new(g_object_unref);
}

static GObject *
mud_profile_manager_constructor (GType type,
                                 guint n_properties,
                                 GObjectConstructParam *construct_params)
{
  GObject *obj;
  MudProfileManagerClass *klass;

  if (manager_singleton == NULL)
    {
    /* Chain up to parent constructor */
      klass = MUD_PROFILE_MANAGER_CLASS( g_type_class_peek(MUD_TYPE_PROFILE_MANAGER) ); /* TODO: Use parent_class static after moving to G_DEFINE_TYPE */
      obj = G_OBJECT_CLASS (g_type_class_peek_parent(klass))->constructor (type, n_properties, construct_params);
      manager_singleton = MUD_PROFILE_MANAGER (obj);
      g_object_add_weak_pointer (G_OBJECT (manager_singleton), (gpointer) &manager_singleton);

      /* TODO: Move everything but singleton handling away from here */

      /* TODO: Make MudWindow a singleton too, otherwise we rely on the singleton to be first requested with the parent passed via construct properties, and accidentally work without it in subsequent calls (though this is guaranteed for now) */
      if(!manager_singleton->priv->parent)
      {
          g_printf("ERROR: Tried to instantiate MudProfileManager without passing parent MudWindow\n");
          g_error("Tried to instantiate MudProfileManager without passing parent MudWindow");
      }

      mud_profile_manager_load_profiles(manager_singleton);
      mud_profile_manager_load_muds(manager_singleton);
      mud_profile_manager_load_characters(manager_singleton);
      return obj;
    }

  return g_object_ref (manager_singleton);
}

static void
mud_profile_manager_finalize (GObject *object)
{
    MudProfileManager *profile_manager;
    GObjectClass *parent_class;

    profile_manager = MUD_PROFILE_MANAGER(object);

    g_sequence_free(profile_manager->priv->profiles);
    g_sequence_free(profile_manager->priv->muds);
    g_sequence_free(profile_manager->priv->characters);

    parent_class = g_type_class_peek_parent(G_OBJECT_GET_CLASS(object));
    parent_class->finalize(object);
}

static void
mud_profile_manager_set_property(GObject *object,
                                 guint prop_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
    MudProfileManager *self;

    self = MUD_PROFILE_MANAGER(object);

    switch(prop_id)
    {
        case PROP_PARENT:
            self->priv->parent = MUD_WINDOW(g_value_get_object(value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void
mud_profile_manager_get_property(GObject *object,
                                 guint prop_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
    MudProfileManager *self;

    self = MUD_PROFILE_MANAGER(object);

    switch(prop_id)
    {
        case PROP_PARENT:
            g_value_take_object(value, self->priv->parent);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

/* Private Methods */
static void
mud_profile_manager_load_profiles (MudProfileManager *self)
{
  gchar *default_profile;
  gchar **profile_list, **i;
  MudProfile *profile;
  GSettings *s;

  g_return_if_fail (MUD_IS_PROFILE_MANAGER(self));

  s = g_settings_new ("org.gnome.MUD");

  profile_list = g_settings_get_strv (s, "profile-list");
  default_profile = g_settings_get_string (s, "default-profile");

#warning Do something with the default profile

  for (i = profile_list; *i; ++i)
    {
      guint64 res;
      guint id;
      gchar *endptr = NULL;
      gchar *profile_id = *i;

      if (!g_str_has_prefix (profile_id, "profile-") || profile_id[8] == '\0')
        continue;

      res = g_ascii_strtoull (&profile_id[8], &endptr, 10);
      if ((res == G_MAXUINT64 && errno == ERANGE) || ((res == 0) && endptr == &profile_id[8]) || (res >= G_MAXUINT64))
        continue;

      id = (guint)res; /* TODO: Why downcast? */

      profile = g_object_new (MUD_TYPE_PROFILE,
                              "id", id,
                              "parent", self,
                              NULL);
      g_sequence_append (self->priv->profiles, profile);
    }
  g_sequence_sort (self->priv->profiles, (GCompareDataFunc)mud_profile_compare_func, NULL);

  g_free (default_profile);
  g_strfreev (profile_list);
  g_object_unref (s);
}

static MudProfile *
mud_profile_manager_get_profile_by_id (MudProfileManager *self,
                                       const gchar       *id)
{
  GSequenceIter *iter;
  MudProfile *profile = NULL;
  gchar *profile_id = NULL;

  /* FIXME: This complicated lookup suggests we might do better with a
   * GHashTable instead of GSeqence, afterall, for simple key-based lookup */
  for (iter = g_sequence_get_begin_iter (self->priv->profiles);
       !g_sequence_iter_is_end (iter);
       iter = g_sequence_iter_next (iter))
    {
      profile_id = g_strdup_printf ("profile-%" G_GUINT64_FORMAT, ((MudProfile *)g_sequence_get (iter))->id);
      if (strcmp (id, profile_id) == 0)
        {
          g_free (profile_id);
          profile = g_sequence_get (iter);
          break;
        }
      g_free (profile_id);
    }

  return profile;
}

static MudMud *
mud_profile_manager_get_mud_by_id (MudProfileManager *self,
                                   const gchar       *id)
{
  GSequenceIter *iter;
  MudMud *mud;

  /* FIXME: This complicated lookup suggests we might do better with a
   * GHashTable instead of GSeqence, afterall, for simple key-based lookup */
  for (iter = g_sequence_get_begin_iter (self->priv->muds);
       !g_sequence_iter_is_end (iter);
       iter = g_sequence_iter_next (iter))
    {
      mud = g_sequence_get (iter);
      if (strcmp (id, mud_mud_get_id (mud)) == 0)
        return mud;
    }

  return NULL;
}

static void
mud_profile_manager_load_muds (MudProfileManager *self)
{
  GSettings *s;
  gchar **mud_list, **i;

  g_return_if_fail (MUD_IS_PROFILE_MANAGER(self));

  s = g_settings_new ("org.gnome.MUD");
  mud_list = g_settings_get_strv (s, "mud-list");

  for (i = mud_list; *i; ++i)
    g_sequence_append (self->priv->muds, mud_mud_new (*i));

  g_sequence_sort (self->priv->muds, (GCompareDataFunc)mud_mud_compare, NULL);

  g_strfreev (mud_list);
  g_object_unref (s);
}

static void
mud_profile_manager_load_characters (MudProfileManager *self)
{
  GSettings *s;
  gchar **character_list, **i;
  gchar *character_path, *profile_id, *mud_id;
  MudProfile *profile;
  MudMud *mud;

  g_return_if_fail (MUD_IS_PROFILE_MANAGER(self));

  s = g_settings_new ("org.gnome.MUD");
  character_list = g_settings_get_strv (s, "character-list");

  for (i = character_list; *i; ++i)
    {
      /* TODO: Ignore all that don't begin with 'character-' and continue with a guint64 - rest of the code can't deal with it yet due to relying on GSequence sorting */
      if (!g_str_has_prefix (*i, "character-") || (*i)[10] == '\0')
        continue;
      /* TODO: Not bothering with a guint64 follow-up here, as the restriction should be restricted instead soon */

      /* TODO: Clean this up somehow (to not need to access GSettings here) by logically moving the profile/mud binding setup to MudCharacter class? */
      character_path = g_strdup_printf("/org/gnome/MUD/Characters/%s/", *i);
      s = g_settings_new_with_path("org.gnome.MUD.Character", character_path);
      profile_id = g_settings_get_string (s, "profile");
      g_free (character_path);

      profile = mud_profile_manager_get_profile_by_id (self, profile_id);
      /* TODO: If profile wasn't found, set it to the default instead of bailing */
      if (profile == NULL)
        {
          g_free (profile_id);
          continue;
        }
      g_free (profile_id);

      mud_id = g_settings_get_string (s, "mud");
      mud = mud_profile_manager_get_mud_by_id (self, mud_id);
      if (mud == NULL)
        {
          g_free (mud_id);
          g_object_unref (s);
          continue;
        }
      g_free (mud_id);

      g_sequence_append (self->priv->characters, mud_character_new (*i, profile, mud));
    }

  g_sequence_sort (self->priv->characters, (GCompareDataFunc)mud_character_compare, NULL);

  g_strfreev (character_list);
  g_object_unref (s);
}

/* Public Methods */
MudProfile*
mud_profile_manager_add_profile (MudProfileManager *self,
                                 const gchar *profile_name)
{
    MudProfile *profile, *last_profile;
    guint new_id, i;
    gchar **profile_list;
    gchar *profile_id;
    GSettings *settings;
    GVariantBuilder builder;

    g_return_val_if_fail (MUD_IS_PROFILE_MANAGER(self), NULL);

    last_profile = MUD_PROFILE (g_sequence_get (g_sequence_iter_prev (g_sequence_get_end_iter (self->priv->profiles))));
    /* TODO: This can crash if no profiles exist for some reason - make that impossible? */
    g_return_val_if_fail (last_profile->id < (G_MAXUINT64 - 1), NULL);

    new_id = last_profile->id + 1;
    profile = g_object_new(MUD_TYPE_PROFILE,
                           "id", new_id,
                           "name", profile_name,
                           "parent", self,
                           NULL);

    g_sequence_insert_sorted(self->priv->profiles, profile, (GCompareDataFunc)mud_profile_compare_func, NULL);

    settings = g_settings_new("org.gnome.MUD");
    profile_list = g_settings_get_strv(settings, "profile-list");
    g_variant_builder_init (&builder, G_VARIANT_TYPE_STRING_ARRAY);

    /* TODO: Instead of this, perhaps keep a good list of UUIDs in manager and then just write that out after the new profile has been added to it?
     * Alternatively (and perhaps even better), try to bind the available profiles directly with ProfilesList.list, so that they can come and go via dconf (other processes)?
     */
    for (i = 0; profile_list[i]; ++i)
        g_variant_builder_add(&builder, "s", profile_list[i]);

    profile_id = g_strdup_printf("profile-%u", new_id);
    g_variant_builder_add(&builder, "s", profile_id);
    g_free(profile_id);
    g_settings_set_value(settings, "profile-list", g_variant_builder_end(&builder));

    mud_window_populate_profiles_menu(self->priv->parent);

    g_strfreev(profile_list);
    g_object_unref(settings);
    return profile;
}

void
mud_profile_manager_delete_profile(MudProfileManager *self,
                                   MudProfile *profile)
{
    g_return_if_fail(MUD_IS_PROFILE_MANAGER(self));

    if (profile) /* TODO: programmer error if not passed? */
    {
        GSettings *global_settings;
        GSettingsSchema *schema;
        gchar **profile_list, **keys;
        gchar *profile_to_delete;
        GVariantBuilder builder;
        guint i;
#if 0
        gchar *key, *pname;
        GSList *connections, *entry;
#endif

        g_sequence_remove (g_sequence_lookup (self->priv->profiles,
                                              profile,
                                              (GCompareDataFunc)mud_profile_compare_func,
                                              NULL));

        /* Remove from gsettings profiles list */
        global_settings = g_settings_new("org.gnome.MUD");
        profile_list = g_settings_get_strv(global_settings, "profile-list");
        g_variant_builder_init (&builder, G_VARIANT_TYPE_STRING_ARRAY);
        profile_to_delete = g_strdup_printf("profile-%" G_GUINT64_FORMAT, profile->id);
        for (i = 0; profile_list[i]; ++i)
            if (strcmp(profile_list[i], profile_to_delete) != 0)
                g_variant_builder_add(&builder, "s", profile_list[i]);
        g_free (profile_to_delete);
        g_settings_set_value(global_settings, "list", g_variant_builder_end(&builder));
        g_strfreev(profile_list);
        g_object_unref(global_settings);

        /* Reset all keys, effectively removing the custom profile path from GSettings backend */
        g_object_get(profile->settings, "settings-schema", &schema, NULL);
        keys = g_settings_schema_list_keys (schema);
        for (i = 0; keys[i] != NULL; ++i)
            g_settings_reset (profile->settings, keys[i]);

        g_strfreev(keys);
        g_object_unref(schema);


#warning Reimplement profile updates in connections
#if 0
        /* If the user deletes a profile we need to switch
         * any connections using that profile to Default to prevent
         * mass breakage */
        connections = gconf_client_all_dirs(client,
                                            "/apps/gnome-mud/muds",
                                            NULL);

        entry = connections;

        while(entry)
        {
            gchar *base = g_path_get_basename((gchar *)entry->data);
            key = g_strdup_printf("/apps/gnome-mud/muds/%s/profile",
                                 base);
            g_free(base);

            pname = gconf_client_get_string(client, key, NULL);

            if(g_str_equal(pname, name))
                gconf_client_set_string(client, key, "Default", NULL);

            g_free(key);
            g_free(entry->data);
            entry = g_slist_next(entry);
        }

        g_slist_free(connections);

        gconf_client_suggest_sync(client, NULL);
#endif
        /* Update the profiles menu */
        mud_window_populate_profiles_menu(self->priv->parent);

        g_object_unref(profile);
    }
}

GSequence *
mud_profile_manager_get_characters (MudProfileManager *self)
{
    if(!MUD_IS_PROFILE_MANAGER(self))
        return NULL;

    return self->priv->characters;
}

GSequence *
mud_profile_manager_get_profiles (MudProfileManager *self)
{
    if(!MUD_IS_PROFILE_MANAGER(self))
        return NULL;

    return self->priv->profiles;
}

void
mud_profile_manager_delete_character (MudProfileManager *self,
                                      MudCharacter      *character)
{
  g_return_if_fail (MUD_IS_PROFILE_MANAGER (self));
  /* TODO: Implement */
}

static void
mud_profile_manager_save_mud_list (MudProfileManager *self)
{
  GVariantBuilder builder;
  GSequenceIter *iter;
  GSettings *settings = g_settings_new("org.gnome.MUD");

  g_variant_builder_init (&builder, G_VARIANT_TYPE_STRING_ARRAY);
  for (iter = g_sequence_get_begin_iter (self->priv->muds);
       !g_sequence_iter_is_end (iter);
       iter = g_sequence_iter_next (iter))
    {
      g_variant_builder_add(&builder, "s", mud_mud_get_id (g_sequence_get (iter)));
    }
  g_settings_set_value(settings, "mud-list", g_variant_builder_end(&builder));
  g_object_unref (settings);
}

static void
mud_profile_manager_save_character_list (MudProfileManager *self)
{
  GVariantBuilder builder;
  GSequenceIter *iter;
  GSettings *settings = g_settings_new("org.gnome.MUD");

  g_variant_builder_init (&builder, G_VARIANT_TYPE_STRING_ARRAY);
  for (iter = g_sequence_get_begin_iter (self->priv->characters);
       !g_sequence_iter_is_end (iter);
       iter = g_sequence_iter_next (iter))
    {
      g_variant_builder_add(&builder, "s", mud_character_get_id (g_sequence_get (iter)));
    }
  g_settings_set_value(settings, "character-list", g_variant_builder_end(&builder));
  g_object_unref (settings);
}

MudMud *
mud_profile_manager_add_mud (MudProfileManager *self,
                             const gchar       *name,
                             const gchar       *hostname,
                             guint              port)
{
  guint64 id = 0;
  gchar *new_id;
  MudMud *mud;

  /* We currently guarantee that all sequence items start with 'mud-', so
   * no need to do g_sequence_lookup checks for now */
  if (!g_sequence_is_empty (self->priv->muds))
    {
      gchar *last_id;
      gchar *endptr = NULL;
      g_object_get (MUD_PROFILE (g_sequence_get (g_sequence_iter_prev (g_sequence_get_end_iter (self->priv->muds)))),
                    "id", &last_id,
                    NULL);
      id = g_ascii_strtoull(last_id + 4, &endptr, 10);
      g_free (last_id);
      if ((id == G_MAXUINT64 && errno == ERANGE) || ((id == 0) && endptr == last_id + 4) || (id >= (G_MAXUINT64 - 1)))
          return NULL;
      ++id;
    }

  new_id = g_strdup_printf ("mud-%" G_GUINT64_FORMAT, id);
  mud = g_object_new (MUD_TYPE_MUD,
                      "id", new_id,
                      "name", name,
                      "hostname", hostname,
                      "port", port,
                      NULL);
  /* TODO: If these sequences would be wrapped in an object, we could just bind things and even g_settings_bind_with_mapping the list? */

  g_sequence_insert_sorted (self->priv->muds, mud, (GCompareDataFunc)mud_mud_compare, NULL);
  mud_profile_manager_save_mud_list (self);

  g_free (new_id);
  return mud;
}

MudCharacter *
mud_profile_manager_add_character (MudProfileManager *self,
                                   MudProfile        *profile,
                                   MudMud            *mud,
                                   const gchar       *name,
                                   const gchar       *connect_string,
                                   const gchar       *icon)
{
  guint64 id = 0;
  gchar *new_id;
  MudCharacter *character;

  /* We currently guarantee that all sequence items start with 'character-', so
   * no need to do g_sequence_lookup checks for now */
  if (!g_sequence_is_empty (self->priv->characters))
    {
      gchar *last_id;
      gchar *endptr = NULL;
      g_object_get (MUD_PROFILE (g_sequence_get (g_sequence_iter_prev (g_sequence_get_end_iter (self->priv->characters)))),
                    "id", &last_id,
                    NULL);
      id = g_ascii_strtoull(last_id + 10, &endptr, 10);
      g_free (last_id);
      if ((id == G_MAXUINT64 && errno == ERANGE) || ((id == 0) && endptr == last_id + 10) || (id >= (G_MAXUINT64 - 1)))
          return NULL;
      ++id;
    }

  new_id = g_strdup_printf ("character-%" G_GUINT64_FORMAT, id);
  character = g_object_new (MUD_TYPE_CHARACTER,
                            "id", new_id,
                            "profile", profile,
                            "mud", mud,
                            "name", name,
                            "connect-string", connect_string,
                            NULL); /* TODO: icon */

  g_sequence_insert_sorted (self->priv->characters, character, (GCompareDataFunc)mud_character_compare, NULL);
  mud_profile_manager_save_character_list (self);

  g_free (new_id);
  return character;
}
