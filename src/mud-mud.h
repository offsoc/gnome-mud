/* mud-mud.h
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

#define MUD_TYPE_MUD (mud_mud_get_type())

G_DECLARE_FINAL_TYPE (MudMud, mud_mud, MUD, MUD, GObject);

MudMud      *mud_mud_new          (const gchar  *id);
const gchar *mud_mud_get_id       (MudMud       *self);
const gchar *mud_mud_get_name     (MudMud       *self);
void         mud_mud_set_name     (MudMud       *self,
                                   const gchar  *name);
const gchar *mud_mud_get_hostname (MudMud       *self);
void         mud_mud_set_hostname (MudMud       *self,
                                   const gchar  *hostname);
const guint  mud_mud_get_port     (MudMud       *self);
void         mud_mud_set_port     (MudMud       *self,
                                   const guint   port);
gint         mud_mud_compare      (const MudMud *mud1,
                                   const MudMud *mud2);


G_END_DECLS
