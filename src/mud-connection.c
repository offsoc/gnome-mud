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

#include <string.h>
#include <glib.h>
#include <gio/gio.h>

#include "mud-connection.h"

/* This is the initial size of the output buffer, which grows dynamically,
 * and the total size of the input buffer, which does not.
 */
#define BUFFER_RESERVE 32768

struct _MudConnection
{
    GObject parent_instance;

    GCancellable *connect_cancel;
    GSocketClient *client;
    GByteArray *inbuf;
    GByteArray *outbuf;
    GSocketConnection *scon;
    GSocket *socket;
    guint read_source_id;
    GSource *write_source;
};

G_DEFINE_TYPE(MudConnection, mud_connection, G_TYPE_OBJECT)

enum
{
    SIGNAL_CONNECTED,
    SIGNAL_DISCONNECTED,
    SIGNAL_DATA_READY,

    /* Must be last. */
    SIGNAL_COUNT
};

static guint mud_connection_signal[SIGNAL_COUNT];

static void
mud_connection_finalize(GObject *object)
{
    MudConnection *self = MUD_CONNECTION(object);

    if(self->connect_cancel)
        g_object_unref(self->connect_cancel);

    if(self->client)
        g_object_unref(self->client);

    if(self->inbuf)
        g_byte_array_free(self->inbuf, TRUE);

    if(self->outbuf)
        g_byte_array_free(self->outbuf, TRUE);

    if(self->scon)
        g_io_stream_close(G_IO_STREAM(self->scon), NULL, NULL);

    if(self->read_source_id > 0)
        g_source_remove(self->read_source_id);

    if(self->write_source)
        g_source_remove(g_source_get_id(self->write_source));

    G_OBJECT_CLASS(mud_connection_parent_class)->finalize(object);
}

static void
mud_connection_class_init(MudConnectionClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    mud_connection_signal[SIGNAL_CONNECTED] =
        g_signal_new("connected",
                     MUD_TYPE_CONNECTION,
                     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                     0,
                     NULL,
                     NULL,
                     g_cclosure_marshal_VOID__VOID,
                     G_TYPE_NONE, 0);

    mud_connection_signal[SIGNAL_DISCONNECTED] =
        g_signal_new("disconnected",
                     MUD_TYPE_CONNECTION,
                     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                     0,
                     NULL,
                     NULL,
                     g_cclosure_marshal_VOID__VOID,
                     G_TYPE_NONE, 0);

    mud_connection_signal[SIGNAL_DATA_READY] =
        g_signal_new("data-ready",
                     MUD_TYPE_CONNECTION,
                     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                     0,
                     NULL,
                     NULL,
                     g_cclosure_marshal_VOID__VOID,
                     G_TYPE_NONE, 0);

    object_class->finalize = mud_connection_finalize;
}

static void
mud_connection_init(MudConnection *self)
{
    self->connect_cancel = NULL;
    self->client = g_socket_client_new();
    self->inbuf = g_byte_array_sized_new(BUFFER_RESERVE);
    self->outbuf = g_byte_array_sized_new(BUFFER_RESERVE);
    self->scon = NULL;
    self->socket = NULL;
    self->read_source_id = 0;
    self->write_source = NULL;
}

void
mud_connection_disconnect(MudConnection *self)
{
    gboolean was_connected;

    g_assert(MUD_IS_CONNECTION(self));

    was_connected = mud_connection_is_connected(self);

    g_cancellable_cancel(self->connect_cancel);
    self->connect_cancel = NULL;

    self->socket = NULL;

    if(self->scon)
    {
        g_io_stream_close(G_IO_STREAM(self->scon), NULL, NULL);
        self->scon = NULL;
    }

    if(was_connected)
        g_signal_emit_by_name(self, "disconnected");

    if(self->outbuf)
        g_byte_array_set_size(self->outbuf, 0);
}

static gboolean
mud_connection_flush(MudConnection *self)
{
    GError *error = NULL;

    g_assert(MUD_IS_CONNECTION(self));
    g_assert(mud_connection_is_connected(self));

    if(self->outbuf->len == 0)
        return TRUE;

    gssize ret = g_socket_send(self->socket, (const gchar *)self->outbuf->data,
                               self->outbuf->len, NULL, &error);

    if(error)
    {
        if(!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK))
        {
            g_warning("%s: g_socket_send failed: %s",
                      __FUNCTION__, error->message);
        }

        g_clear_error(&error);
    }

    if(ret == 0)
    {
        g_debug("Disconnecting because the connection was lost during a flush");
        mud_connection_disconnect(self);
        return FALSE;
    }

    if(ret > 0)
        g_byte_array_remove_range(self->outbuf, 0, ret);

    return TRUE;
}

static gboolean
socket_read_ready_cb(GSocket *socket, GIOCondition condition, gpointer user_data)
{
    MudConnection *self = MUD_CONNECTION(user_data);
    gssize bytes_read;
    GError *error = NULL;

    /* We only get this condition from a connection cancellation via
     * mud_connection_disconnect().
     */
    if(self->scon == NULL)
    {
        g_clear_object(&self->connect_cancel);
        return FALSE;
    }

    g_byte_array_set_size(self->inbuf, BUFFER_RESERVE);

    do
    {
        bytes_read = g_socket_receive(socket, (gchar *)self->inbuf->data,
                                      BUFFER_RESERVE, NULL, &error);

        if(error)
        {
            if(!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK))
            {
                g_warning("%s: g_socket_receive failed: %s",
                          __FUNCTION__, error->message);
            }

            g_clear_error(&error);
        }

        if(bytes_read == 0)
        {
            g_debug("Disconnecting because the connection was lost during a read");
            mud_connection_disconnect(self);
            return FALSE;
        }

        if(bytes_read > 0)
        {
            g_byte_array_set_size(self->inbuf, bytes_read);
            g_signal_emit_by_name(self, "data-ready");
        }
    }
    while(bytes_read > 0);

    return TRUE;
}

static gboolean
socket_write_ready_cb(GSocket *socket, GIOCondition condition, gpointer user_data)
{
    MudConnection *self = MUD_CONNECTION(user_data);

    if(!mud_connection_flush(self) || self->outbuf->len == 0)
    {
        self->write_source = NULL;
        return FALSE;
    }

    return TRUE;
}

static void
socket_client_connect_cb(GObject *source_object, GAsyncResult *res,
                         gpointer user_data)
{
    MudConnection *self = MUD_CONNECTION(user_data);
    GSource *source;

    self->scon = g_socket_client_connect_to_host_finish(self->client, res, NULL);

    if(self->connect_cancel == NULL)
    {
        /* The only way we can get here is if mud_connection_disconnect() was
         * called on an incomplete connection, so don't emit
         * #MudConnection::disconnected as it's already done.
         */
        g_debug("The connection attempt was cancelled");
        return;
    }

    g_clear_object(&self->connect_cancel);

    if(self->scon == NULL)
    {
        g_debug("The connection attempt failed");
        g_signal_emit_by_name(self, "disconnected");
        return;
    }

    self->socket = g_socket_connection_get_socket(self->scon);
    g_socket_set_blocking(self->socket, FALSE);
    g_socket_set_keepalive(self->socket, TRUE);

    g_signal_emit_by_name(self, "connected");

    source = g_socket_create_source(self->socket, G_IO_IN, NULL);
    g_source_set_callback(source, (GSourceFunc)socket_read_ready_cb, self, NULL);
    g_source_set_can_recurse(source, FALSE);
    self->read_source_id = g_source_attach(source, NULL);
    g_source_unref(source);
}

gboolean
mud_connection_connect(MudConnection *self, const gchar *hostname, guint port,
                       const gchar *proxy_uri)
{
    g_assert(MUD_IS_CONNECTION(self));
    g_assert(!self->scon);
    g_assert(hostname);
    g_assert(port <= 65535);

    if(mud_connection_is_connected(self))
    {
        g_warning("Attempting to reconnect when not disconnected");
        return FALSE;
    }

    g_debug("Connecting to %s:%d via %s", hostname, port, proxy_uri);

    if(proxy_uri)
    {
        /* TODO: Consider using ignore_hosts for 127.0.0.1/8 and localhost. */
        GProxyResolver *proxy_resolver;

        proxy_resolver = g_simple_proxy_resolver_new(proxy_uri, NULL);
        g_socket_client_set_proxy_resolver(self->client, proxy_resolver);
        g_object_unref(proxy_resolver);

        g_socket_client_set_enable_proxy(self->client, TRUE);
    }
    else
        g_socket_client_set_enable_proxy(self->client, FALSE);

    self->connect_cancel = g_cancellable_new();
    g_socket_client_connect_to_host_async(self->client,
                                          hostname,
                                          port,
                                          self->connect_cancel,
                                          socket_client_connect_cb,
                                          self);

    return TRUE;
}

gboolean
mud_connection_is_connected(MudConnection *self)
{
    g_assert(MUD_IS_CONNECTION(self));

    /* It's possible for the underlying socket to be disconnected, but
     * until disconnect has been called, resources aren't cleaned up and should
     * be considered connected.
     */
    return self->scon != NULL || self->connect_cancel != NULL;
}

void
mud_connection_send(MudConnection *self, const gchar *buf, gssize len)
{
    g_assert(MUD_IS_CONNECTION(self));
    g_assert(buf);

    if(len == 0)
        return;

    if(len < 0)
        len = strlen(buf);

    g_byte_array_append(self->outbuf, (const guint8 *)buf, len);

    if(!self->write_source)
    {
        self->write_source = g_socket_create_source(self->socket, G_IO_OUT, NULL);
        g_source_set_callback(self->write_source, (GSourceFunc)socket_write_ready_cb, self, NULL);
        g_source_set_can_recurse(self->write_source, FALSE);
        g_source_attach(self->write_source, NULL);
        g_source_unref(self->write_source);
    }
}

const GByteArray *
mud_connection_get_input(MudConnection *self)
{
    g_assert(MUD_IS_CONNECTION(self));
    return self->inbuf;
}
