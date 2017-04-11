/* Copyright (C) 2017 Steven Jackson <sj@oscode.net>
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

#ifndef MUD_CONNECTION_H
#define MUD_CONNECTION_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MUD_TYPE_CONNECTION     mud_connection_get_type()

G_DECLARE_FINAL_TYPE(MudConnection, mud_connection, MUD, CONNECTION, GObject)

/* mud_connection_connect:
 * @self: a #MudConnection.
 * @hostname: a hostname in a format understood by
 *   g_socket_client_connect_to_host_async().
 * @port: a port in the range of 0-65535.
 * @proxy_uri: (nullable): a proxy URI in a format understood by
 *   g_simple_proxy_resolver_new().
 *
 * Don't call this when a connection is active or a reconnect attempt is in
 * progress, i.e. when mud_connection_is_connected() returns %TRUE.
 *
 * The #MudConnection::connected or #MudConnection::disconnected signal is
 * emitted when the result of the connection is known.
 *
 * Returns: @TRUE when a connection attempt has begun.
 */
gboolean mud_connection_connect(MudConnection *self, const gchar *hostname,
                                guint port, const gchar *proxy_uri);

/* mud_connection_is_connected:
 * @self: a #MudConnection.
 *
 * If self is @NULL or disconected it returns early.
 */
void mud_connection_disconnect(MudConnection *self);

/* mud_connection_is_connected:
 * @self: (nullable): a #MudConnection.
 *
 * Returns: @TRUE is the connection is connected.
 */
gboolean mud_connection_is_connected(MudConnection *self);

/* mud_connection_get_input:
 * @self: a #MudConnection.
 * @buf: the data to send.
 * @len: the length of @buf, or -1 to calculate its @NULL-terminated length.
 *
 * The data might not be sent immediately, it's written when the operation
 * would not block.
 */
void mud_connection_send(MudConnection *self, const gchar *buf, gssize len);

/* mud_connection_get_input:
 * @self: a #MudConnection.
 *
 * See #MudConnection::data-ready.
 *
 * Returns: a #GByteArray or @NULL.
 */
const GByteArray *mud_connection_get_input(MudConnection *self);

G_END_DECLS

#endif /* MUD_CONNECTION_H */
