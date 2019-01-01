/* mud-character.h
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

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _MudProfile MudProfile;
typedef struct _MudMud     MudMud;

#define MUD_TYPE_CHARACTER (mud_character_get_type())

G_DECLARE_FINAL_TYPE (MudCharacter, mud_character, MUD, CHARACTER, GObject);

MudCharacter *mud_character_new                (const char         *id,
                                                const MudProfile   *profile,
                                                const MudMud       *mud);
const gchar  *mud_character_get_id             (MudCharacter       *self);
const gchar  *mud_character_get_name           (MudCharacter       *self);
void          mud_character_set_name           (MudCharacter       *self,
                                                const gchar        *name);
const gchar  *mud_character_get_connect_string (MudCharacter       *self);
void          mud_character_set_connect_string (MudCharacter       *self,
                                                const gchar        *connect_string);
MudProfile   *mud_character_get_profile        (MudCharacter       *self);
MudMud       *mud_character_get_mud            (MudCharacter       *self);
const gchar  *mud_character_get_icon           (MudCharacter       *self);
void          mud_character_set_icon           (MudCharacter       *self,
                                                const gchar        *icon_name);
int           mud_character_compare            (const MudCharacter *char1,
                                                const MudCharacter *char2);

G_END_DECLS
