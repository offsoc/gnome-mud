/* mud-connections.h
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

#define MUD_TYPE_CONNECTIONS (mud_connections_get_type ())
G_DECLARE_FINAL_TYPE (MudConnections, mud_connections, MUD, CONNECTIONS, GObject); /* TODO: Inherit from the used widget type instead, to extend it, instead of parallel control of it? */

G_END_DECLS
