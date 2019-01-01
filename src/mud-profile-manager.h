/* GNOME-Mud - A simple Mud Client
 * mud-profile-manager.h
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

#ifndef MUD_PROFILE_MANAGER_H
#define MUD_PROFILE_MANAGER_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct _MudProfile   MudProfile;
typedef struct _MudMud       MudMud;
typedef struct _MudCharacter MudCharacter;

#define MUD_TYPE_PROFILE_MANAGER              (mud_profile_manager_get_type ())
#define MUD_PROFILE_MANAGER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MUD_TYPE_PROFILE_MANAGER, MudProfileManager))
#define MUD_PROFILE_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MUD_TYPE_PROFILE_MANAGER, MudProfileManagerClass))
#define MUD_IS_PROFILE_MANAGER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MUD_TYPE_PROFILE_MANAGER))
#define MUD_IS_PROFILE_MANAGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MUD_TYPE_PROFILE_MANAGER))
#define MUD_PROFILE_MANAGER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MUD_TYPE_PROFILE_MANAGER, MudProfileManagerClass))
#define MUD_PROFILE_MANAGER_GET_PRIVATE(obj)  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MUD_TYPE_PROFILE_MANAGER, MudProfileManagerPrivate))

typedef struct _MudProfileManager            MudProfileManager;
typedef struct _MudProfileManagerClass       MudProfileManagerClass;
typedef struct _MudProfileManagerPrivate     MudProfileManagerPrivate;

struct _MudProfileManagerClass
{
    GObjectClass parent_class;
};

struct _MudProfileManager
{
    GObject parent_instance;

    /*< private >*/
    MudProfileManagerPrivate *priv;
};

GType mud_profile_manager_get_type (void);

MudProfile   *mud_profile_manager_add_profile      (MudProfileManager *self,
                                                    const gchar       *name);
void          mud_profile_manager_delete_profile   (MudProfileManager *self,
                                                    MudProfile        *profile);
GSequence    *mud_profile_manager_get_profiles     (MudProfileManager *self);
MudMud       *mud_profile_manager_add_mud          (MudProfileManager *self,
                                                    const gchar       *name,
                                                    const gchar       *hostname,
                                                    guint              port);
GSequence    *mud_profile_manager_get_characters   (MudProfileManager *self);
MudCharacter *mud_profile_manager_add_character    (MudProfileManager *self,
                                                    MudProfile        *profile,
                                                    MudMud            *mud,
                                                    const gchar       *name,
                                                    const gchar       *connect_string,
                                                    const gchar       *icon);
void          mud_profile_manager_delete_character (MudProfileManager *self,
                                                    MudCharacter      *character);

G_END_DECLS

#endif // MUD_PROFILE_MANAGER_H

