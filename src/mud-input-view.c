/* mud-input-view.c
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

#include "mud-input-view.h"

struct _MudInputView
{
  GtkStack     parent_instance;
  GtkWidget   *scrolled_text_view;
  GtkTextView *text_view;
  GtkEntry    *password_entry;
};

G_DEFINE_TYPE (MudInputView, mud_input_view, GTK_TYPE_STACK)

enum {
  PROP_0,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

MudInputView *
mud_input_view_new (void)
{
  return g_object_new (MUD_TYPE_INPUT_VIEW, NULL);
}

void
mud_input_view_show_password (MudInputView *self)
{
  g_return_if_fail (MUD_IS_INPUT_VIEW (self));

  gtk_stack_set_visible_child (GTK_STACK (self), GTK_WIDGET (self->password_entry));
}

void
mud_input_view_hide_password (MudInputView *self)
{
  g_return_if_fail (MUD_IS_INPUT_VIEW (self));

  gtk_stack_set_visible_child (GTK_STACK (self), GTK_WIDGET (self->scrolled_text_view));
}

GtkTextView *
mud_input_view_get_text_view (MudInputView *self)
{
  /* TODO: Temporary accessor until more functionality gets moved to this class directly (the members shouldn't be accessible from outside) */
  g_return_val_if_fail (MUD_IS_INPUT_VIEW (self), NULL);

  return self->text_view;
}

GtkEntry *
mud_input_view_get_password_entry (MudInputView *self)
{
  /* TODO: Temporary accessor until more functionality gets moved to this class directly (the members shouldn't be accessible from outside) */
  g_return_val_if_fail (MUD_IS_INPUT_VIEW (self), NULL);

  return self->password_entry;
}

static void
mud_input_view_finalize (GObject *object)
{
  MudInputView *self = (MudInputView *)object;

  G_OBJECT_CLASS (mud_input_view_parent_class)->finalize (object);
}

static void
mud_input_view_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  MudInputView *self = MUD_INPUT_VIEW (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
mud_input_view_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  MudInputView *self = MUD_INPUT_VIEW (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
mud_input_view_grab_focus (GtkWidget *widget)
{
  MudInputView *self = MUD_INPUT_VIEW (widget);

  if (gtk_stack_get_visible_child (GTK_STACK (self)) == self->password_entry)
    {
      gtk_widget_grab_focus (self->password_entry);
    }
  else
    {
      gtk_widget_grab_focus (self->text_view);
    }
}

static void
mud_input_view_class_init (MudInputViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = mud_input_view_finalize;
  object_class->get_property = mud_input_view_get_property;
  object_class->set_property = mud_input_view_set_property;
  widget_class->grab_focus = mud_input_view_grab_focus;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/MUD/ui/mud-input-view.ui");
  gtk_widget_class_bind_template_child (widget_class, MudInputView, scrolled_text_view);
  gtk_widget_class_bind_template_child (widget_class, MudInputView, text_view);
  gtk_widget_class_bind_template_child (widget_class, MudInputView, password_entry);
}

static void
mud_input_view_init (MudInputView *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  gtk_stack_set_visible_child (GTK_STACK (self), self->scrolled_text_view);
}
