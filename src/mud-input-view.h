/* mud-input-view.h
 *
 * Copyright 2019 Mart Raudsepp <leio@gentoo.org>
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

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MUD_TYPE_INPUT_VIEW (mud_input_view_get_type())

G_DECLARE_FINAL_TYPE (MudInputView, mud_input_view, MUD, INPUT_VIEW, GtkStack)

MudInputView *mud_input_view_new                (void);
void          mud_input_view_show_password      (MudInputView *self);
void          mud_input_view_hide_password      (MudInputView *self);
GtkTextView  *mud_input_view_get_text_view      (MudInputView *self);
GtkEntry     *mud_input_view_get_password_entry (MudInputView *self);

G_END_DECLS
