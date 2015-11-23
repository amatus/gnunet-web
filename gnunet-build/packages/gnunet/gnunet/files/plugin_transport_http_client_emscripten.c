/*
     This file is part of GNUnet
     Copyright (C) 2002-2015 Christian Grothoff (and other contributing authors)

     GNUnet is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published
     by the Free Software Foundation; either version 3, or (at your
     option) any later version.

     GNUnet is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with GNUnet; see the file COPYING.  If not, write to the
     Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
     Boston, MA 02110-1301, USA.
*/

/**
 * @file transport/plugin_transport_http_client.c
 * @brief HTTP/S client transport plugin
 * @author Matthias Wachs
 * @author Christian Grothoff
 * @author David Barksdale <amatus@amatus.name>
 */

#if BUILD_HTTPS
#define PLUGIN_NAME "https_client"
#define HTTP_STAT_STR_CONNECTIONS "# HTTPS client connections"
#define LIBGNUNET_PLUGIN_TRANSPORT_INIT libgnunet_plugin_transport_https_client_init
#define LIBGNUNET_PLUGIN_TRANSPORT_DONE libgnunet_plugin_transport_https_client_done
#else
#define PLUGIN_NAME "http_client"
#define HTTP_STAT_STR_CONNECTIONS "# HTTP client connections"
#define LIBGNUNET_PLUGIN_TRANSPORT_INIT libgnunet_plugin_transport_http_client_init
#define LIBGNUNET_PLUGIN_TRANSPORT_DONE libgnunet_plugin_transport_http_client_done
#endif

#define PUT_DISCONNECT_TIMEOUT GNUNET_TIME_relative_multiply (GNUNET_TIME_UNIT_SECONDS, 1)

#define ENABLE_PUT GNUNET_YES
#define ENABLE_GET GNUNET_YES

#include "platform.h"
#include "gnunet_util_lib.h"
#include "gnunet_protocols.h"
#include "gnunet_transport_plugin.h"
#include "plugin_transport_http_common.h"
#include <emscripten.h>


#define LOG(kind,...) GNUNET_log_from(kind, PLUGIN_NAME, __VA_ARGS__)

/**
 * Encapsulation of all of the state of the plugin.
 */
struct HTTP_Client_Plugin;


/**
 *  Message to send using http
 */
struct HTTP_Message
{
  /**
   * next pointer for double linked list
   */
  struct HTTP_Message *next;

  /**
   * previous pointer for double linked list
   */
  struct HTTP_Message *prev;

  /**
   * buffer containing data to send
   */
  char *buf;

  /**
   * Continuation function to call once the transmission buffer
   * has again space available.  NULL if there is no
   * continuation to call.
   */
  GNUNET_TRANSPORT_TransmitContinuation transmit_cont;

  /**
   * Closure for @e transmit_cont.
   */
  void *transmit_cont_cls;

  /**
   * amount of data already sent
   */
  size_t pos;

  /**
   * buffer length
   */
  size_t size;
};


/**
 * Session handle for connections.
 */
struct GNUNET_ATS_Session
{
  /**
   * The URL to connect to
   */
  char *url;

  /**
   * Address
   */
  struct GNUNET_HELLO_Address *address;

  /**
   * Pointer to the global plugin struct.
   */
  struct HTTP_Client_Plugin *plugin;

  /**
   * Handle for the HTTP GET request
   */
  int get;

  /**
   * Message stream tokenizer for incoming data
   */
  struct GNUNET_SERVER_MessageStreamTokenizer *msg_tk;

  /**
   * Session timeout task
   */
  struct GNUNET_SCHEDULER_Task *timeout_task;

  /**
   * Absolute time when to receive data again
   * Used for receive throttling
   */
  struct GNUNET_TIME_Absolute next_receive;

  /**
   * When does this session time out.
   */
  struct GNUNET_TIME_Absolute timeout;

  /**
   * Number of bytes waiting for transmission to this peer.
   */
  unsigned long long bytes_in_queue;

  /**
   * Outbound overhead due to HTTP connection
   * Add to next message of this session when calling callback
   */
  size_t overhead;

  /**
   * ATS network type.
   */
  enum GNUNET_ATS_Network_Type scope;
};


/**
 * Encapsulation of all of the state of the plugin.
 */
struct HTTP_Client_Plugin
{
  /**
   * Our environment.
   */
  struct GNUNET_TRANSPORT_PluginEnvironment *env;

  /**
   * Open sessions.
   */
  struct GNUNET_CONTAINER_MultiPeerMap *sessions;

  /**
   * Function to call about session status changes.
   */
  GNUNET_TRANSPORT_SessionInfoCallback sic;

  /**
   * Closure for @e sic.
   */
  void *sic_cls;

  /**
   * Plugin name
   */
  char *name;

  /**
   * Protocol
   */
  char *protocol;

  /**
   * My options to be included in the address
   */
  uint32_t options;

  /**
   * Maximum number of sockets the plugin can use
   * Each http inbound /outbound connections are two connections
   */
  unsigned int max_connections;

  /**
   * Current number of sockets the plugin can use
   * Each http inbound /outbound connections are two connections
   */
  unsigned int cur_connections;

  /**
   * Last used unique HTTP connection tag
   */
  uint32_t last_tag;

  /**
   * use IPv6
   */
  uint16_t use_ipv6;

  /**
   * use IPv4
   */
  uint16_t use_ipv4;
};



/**
 * If a session monitor is attached, notify it about the new
 * session state.
 *
 * @param plugin our plugin
 * @param session session that changed state
 * @param state new state of the session
 */
static void
notify_session_monitor (struct HTTP_Client_Plugin *plugin,
                        struct GNUNET_ATS_Session *session,
                        enum GNUNET_TRANSPORT_SessionState state)
{
  struct GNUNET_TRANSPORT_SessionInfo info;

  if (NULL == plugin->sic)
    return;
  memset (&info, 0, sizeof (info));
  info.state = state;
  info.is_inbound = GNUNET_NO;
  info.num_msg_pending = 0;
  info.num_bytes_pending = 0;
  info.receive_delay = session->next_receive;
  info.session_timeout = session->timeout;
  info.address = session->address;
  plugin->sic (plugin->sic_cls,
               session,
               &info);
}


/**
 * Delete session @a s
 *
 * @param s the session to delete
 */
static void
client_delete_session (struct GNUNET_ATS_Session *s)
{
  struct HTTP_Client_Plugin *plugin = s->plugin;

  if (NULL != s->timeout_task)
  {
    GNUNET_SCHEDULER_cancel (s->timeout_task);
    s->timeout_task = NULL;
    s->timeout = GNUNET_TIME_UNIT_ZERO_ABS;
  }
  GNUNET_assert (GNUNET_OK ==
                 GNUNET_CONTAINER_multipeermap_remove (plugin->sessions,
                                                       &s->address->peer,
                                                       s));
  if (s->get)
  {
    LOG (GNUNET_ERROR_TYPE_DEBUG,
         "Session %p/connection %d: disconnecting GET connection to peer `%s'\n",
         s, s->get,
         GNUNET_i2s (&s->address->peer));
    GNUNET_assert (plugin->cur_connections > 0);
    plugin->cur_connections--;
    EM_ASM_INT({
      Module.print('Aborting xhr: ' + $0);
      xhrs[$0].abort();
    }, s->get);
    s->get = 0;
  }
  GNUNET_STATISTICS_set (plugin->env->stats,
                         HTTP_STAT_STR_CONNECTIONS,
                         plugin->cur_connections,
                         GNUNET_NO);

  notify_session_monitor (plugin,
                          s,
                          GNUNET_TRANSPORT_SS_DONE);
  if (NULL != s->msg_tk)
  {
    GNUNET_SERVER_mst_destroy (s->msg_tk);
    s->msg_tk = NULL;
  }
  GNUNET_HELLO_address_free (s->address);
  GNUNET_free (s->url);
  GNUNET_free (s);
}


/**
 * Increment session timeout due to activity for session @a s
 * @param s the session
 */
static void
client_reschedule_session_timeout (struct GNUNET_ATS_Session *s)
{
  GNUNET_assert (NULL != s->timeout_task);
  s->timeout = GNUNET_TIME_relative_to_absolute (GNUNET_CONSTANTS_IDLE_CONNECTION_TIMEOUT);
}


/**
 * Function that can be used by the transport service to transmit
 * a message using the plugin.   Note that in the case of a
 * peer disconnecting, the continuation MUST be called
 * prior to the disconnect notification itself.  This function
 * will be called with this peer's HELLO message to initiate
 * a fresh connection to another peer.
 *
 * @param cls closure
 * @param s which session must be used
 * @param msgbuf the message to transmit
 * @param msgbuf_size number of bytes in @a msgbuf
 * @param priority how important is the message (most plugins will
 *                 ignore message priority and just FIFO)
 * @param to how long to wait at most for the transmission (does not
 *                require plugins to discard the message after the timeout,
 *                just advisory for the desired delay; most plugins will ignore
 *                this as well)
 * @param cont continuation to call once the message has
 *        been transmitted (or if the transport is ready
 *        for the next transmission call; or if the
 *        peer disconnected...); can be NULL
 * @param cont_cls closure for cont
 * @return number of bytes used (on the physical network, with overheads);
 *         -1 on hard errors (i.e. address invalid); 0 is a legal value
 *         and does NOT mean that the message was not transmitted (DV)
 */
static ssize_t
http_client_plugin_send (void *cls,
                         struct GNUNET_ATS_Session *s,
                         const char *msgbuf,
                         size_t msgbuf_size,
                         unsigned int priority,
                         struct GNUNET_TIME_Relative to,
                         GNUNET_TRANSPORT_TransmitContinuation cont,
                         void *cont_cls)
{
  LOG (GNUNET_ERROR_TYPE_DEBUG,
       "Session %p: Sending message with %u to peer `%s' \n",
       s,
       msgbuf_size, GNUNET_i2s (&s->address->peer));

  EM_ASM_INT({
    var url = Pointer_stringify($0);
    var data = HEAP8.subarray($1, $1 + $2);
    var data_size = $2;
    var cont = $3;
    var cont_cls = $4;
    var target = $5;
    var xhr = new XMLHttpRequest();
    xhr.open('PUT', url);
    xhr.send(data);
    xhr.onload = function(e) {
      Module.print('put onload readyState ' + xhr.readyState + ' status ' + xhr.status);
      if (cont) {
        Runtime.dynCall('viiiii', cont, [cont_cls, target, 1, data_size, data_size]);
      }
    };
    xhr.onerror = function(e) {
      Module.print('put onerror readyState ' + xhr.readyState + ' status ' + xhr.status);
      if (cont) {
        Runtime.dynCall('viiiii', cont, [cont_cls, target, -1, data_size, data_size]);
      }
    };
  },
  s->url,             // $0
  msgbuf,             // $1
  msgbuf_size,        // $2
  cont,               // $3
  cont_cls,           // $4
  &s->address->peer   // $5
  );
  return msgbuf_size;
}


/**
 * Disconnect a session
 *
 * @param cls the `struct HTTP_Client_Plugin *`
 * @param s session
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on error
 */
static int
http_client_plugin_session_disconnect (void *cls,
                                       struct GNUNET_ATS_Session *s)
{
  struct HTTP_Client_Plugin *plugin = cls;

  LOG (GNUNET_ERROR_TYPE_DEBUG,
       "Session %p: notifying transport about ending session\n",s);

  plugin->env->session_end (plugin->env->cls,
                            s->address,
                            s);
  client_delete_session (s);

  return GNUNET_OK;
}


/**
 * Function that is called to get the keepalive factor.
 * #GNUNET_CONSTANTS_IDLE_CONNECTION_TIMEOUT is divided by this number to
 * calculate the interval between keepalive packets.
 *
 * @param cls closure with the `struct Plugin`
 * @return keepalive factor
 */
static unsigned int
http_client_query_keepalive_factor (void *cls)
{
  return 3;
}

/**
 * Callback to destroys all sessions on exit.
 *
 * @param cls the `struct HTTP_Client_Plugin *`
 * @param peer identity of the peer
 * @param value the `struct GNUNET_ATS_Session *`
 * @return #GNUNET_OK (continue iterating)
 */
static int
destroy_session_cb (void *cls,
                    const struct GNUNET_PeerIdentity *peer,
                    void *value)
{
  struct HTTP_Client_Plugin *plugin = cls;
  struct GNUNET_ATS_Session *session = value;

  http_client_plugin_session_disconnect (plugin, session);
  return GNUNET_OK;
}


/**
 * Function that can be used to force the plugin to disconnect
 * from the given peer and cancel all previous transmissions
 * (and their continuationc).
 *
 * @param cls closure
 * @param target peer from which to disconnect
 */
static void
http_client_plugin_peer_disconnect (void *cls,
                                    const struct GNUNET_PeerIdentity *target)
{
  struct HTTP_Client_Plugin *plugin = cls;

  LOG (GNUNET_ERROR_TYPE_DEBUG,
       "Transport tells me to disconnect `%s'\n",
       GNUNET_i2s (target));

  GNUNET_CONTAINER_multipeermap_get_multiple (plugin->sessions,
                                              target,
                                              &destroy_session_cb,
                                              plugin);
}

/**
 * Closure for #session_lookup_client_by_address().
 */
struct SessionClientCtx
{
  /**
   * Address we are looking for.
   */
  const struct GNUNET_HELLO_Address *address;

  /**
   * Session that was found.
   */
  struct GNUNET_ATS_Session *ret;
};


/**
 * Locate the seession object for a given address.
 *
 * @param cls the `struct SessionClientCtx *`
 * @param key peer identity
 * @param value the `struct GNUNET_ATS_Session` to check
 * @return #GNUNET_NO if found, #GNUNET_OK if not
 */
static int
session_lookup_client_by_address (void *cls,
                                  const struct GNUNET_PeerIdentity *key,
                                  void *value)
{
  struct SessionClientCtx *sc_ctx = cls;
  struct GNUNET_ATS_Session *s = value;

  if (0 == GNUNET_HELLO_address_cmp (sc_ctx->address,
                                     s->address))
  {
    sc_ctx->ret = s;
    return GNUNET_NO;
  }
  return GNUNET_YES;
}


/**
 * Check if a sessions exists for an specific address
 *
 * @param plugin the plugin
 * @param address the address
 * @return the session or NULL
 */
static struct GNUNET_ATS_Session *
client_lookup_session (struct HTTP_Client_Plugin *plugin,
                       const struct GNUNET_HELLO_Address *address)
{
  struct SessionClientCtx sc_ctx;

  sc_ctx.address = address;
  sc_ctx.ret = NULL;
  GNUNET_CONTAINER_multipeermap_iterate (plugin->sessions,
                                         &session_lookup_client_by_address,
                                         &sc_ctx);
  return sc_ctx.ret;
}


/**
 * Callback for message stream tokenizer
 *
 * @param cls the session
 * @param client not used
 * @param message the message received
 * @return always #GNUNET_OK
 */
static int
client_receive_mst_cb (void *cls,
                       void *client,
                       const struct GNUNET_MessageHeader *message)
{
  struct GNUNET_ATS_Session *s = cls;
  struct HTTP_Client_Plugin *plugin;
  struct GNUNET_TIME_Relative delay;
  char *stat_txt;

  plugin = s->plugin;
  delay = s->plugin->env->receive (plugin->env->cls,
                                   s->address,
                                   s,
                                   message);
  GNUNET_asprintf (&stat_txt,
                   "# bytes received via %s_client",
                   plugin->protocol);
  GNUNET_STATISTICS_update (plugin->env->stats,
                            stat_txt,
                            ntohs(message->size),
                            GNUNET_NO);
  GNUNET_free (stat_txt);

  s->next_receive = GNUNET_TIME_absolute_add (GNUNET_TIME_absolute_get (), delay);
  if (GNUNET_TIME_absolute_get ().abs_value_us < s->next_receive.abs_value_us)
  {
    LOG (GNUNET_ERROR_TYPE_DEBUG,
         "Client: peer `%s' address `%s' next read delayed for %s\n",
         GNUNET_i2s (&s->address->peer),
         http_common_plugin_address_to_string (s->plugin->protocol,
                                               s->address->address,
                                               s->address->address_length),
         GNUNET_STRINGS_relative_time_to_string (delay,
                                                 GNUNET_YES));
  }
  client_reschedule_session_timeout (s);
  return GNUNET_OK;
}


/**
 * Callback method used with libcurl when data for a GET connection are
 * received. Forward to MST
 *
 * @param stream pointer where to write data
 * @param size size of an individual element
 * @param nmemb count of elements that can be written to the buffer
 * @param cls destination pointer, passed to the libcurl handle
 * @return bytes read from stream
 */
static size_t
client_receive (void *stream,
                size_t size,
                size_t nmemb,
                void *cls)
{
  struct GNUNET_ATS_Session *s = cls;
  struct GNUNET_TIME_Absolute now;
  size_t len = size * nmemb;

  GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, s->plugin->name,
                   "Session %p / connection %d: Received %u bytes from peer `%s'\n",
                   s, s->get,
                   len, GNUNET_i2s (&s->address->peer));
  now = GNUNET_TIME_absolute_get ();
  if (now.abs_value_us < s->next_receive.abs_value_us)
  {
    struct GNUNET_TIME_Absolute now = GNUNET_TIME_absolute_get ();
    struct GNUNET_TIME_Relative delta
      = GNUNET_TIME_absolute_get_difference (now, s->next_receive);

    LOG (GNUNET_ERROR_TYPE_DEBUG,
         "Session %p / connection %d: No inbound bandwidth available! Next read was delayed for %s\n",
         s,
         s->get,
		     GNUNET_STRINGS_relative_time_to_string (delta,
                                                 GNUNET_YES));
    return 0;
  }
  if (NULL == s->msg_tk)
    s->msg_tk = GNUNET_SERVER_mst_create (&client_receive_mst_cb,
                                          s);
  GNUNET_SERVER_mst_receive (s->msg_tk,
                             s,
                             stream,
                             len,
                             GNUNET_NO,
                             GNUNET_NO);
  return len;
}

int next_xhr = 1;

/**
 * Connect GET connection for a session
 *
 * @param s the session to connect
 * @return #GNUNET_OK on success, #GNUNET_SYSERR otherwise
 */
static int
client_connect_get (struct GNUNET_ATS_Session *s)
{
  /* create get connection */
  s->get = next_xhr++;
  s->plugin->cur_connections++;
  EM_ASM_INT({
    var s = $1;
    var url = Pointer_stringify($2);
    var client_receive = $3;
    var http_client_plugin_session_disconnect = $4;
    var plugin  = $5;
    Module.print('Creating new get xhr: ' + $0);
    var xhr = new XMLHttpRequest();
    xhrs[$0] = xhr;
    xhr.responseType = 'arraybuffer';
    xhr.resend = function() {
      xhr.open('GET', url + ',1');
      xhr.send();
    };
    xhr.onreadystatechange = function() {
      Module.print('xhr' + $0 + ' readyState ' + xhr.readyState +
          ' status ' +  xhr.status + ':' + xhr.statusText);
    };
    xhr.onload = function(e) {
      var response = new Uint8Array(e.target.response);
      Module.print('xhr' + $0 + ' got ' + response.length + ' bytes');
      ccallFunc(Runtime.getFuncWrapper(client_receive, 'iiiii'), 'number',
        ['array', 'number', 'number', 'number'],
        [response, response.length, 1, s]);
      xhr.resend();
    };
    xhr.onerror = function(e) {
      Module.print('xhr' + $0 + ' status:'
        + xhr.status + ':' + xhr.statusText);
      ccallFunc(Runtime.getFuncWrapper(http_client_plugin_session_disconnect, 'iii'),
        'number',
        ['number', 'number'],
        [plugin, s]);
    };
    xhr.onabort = function() {
      Module.print('xhr' + $0 + ' aborted');
    };
    xhr.ontimeout = function() {
      Module.print('xhr' + $0 + ' timedout');
      xhr.resend();
    };
    xhr.resend();
  },
    s->get,                                 // $0
    s,                                      // $1
    s->url,                                 // $2
    &client_receive,                        // $3
    &http_client_plugin_session_disconnect, // $4
    s->plugin                               // $5
      );
  return GNUNET_OK;
}


/**
 * Connect both PUT and GET connection for a session
 *
 * @param s the session to connect
 * @return #GNUNET_OK on success, #GNUNET_SYSERR otherwise
 */
static int
client_connect (struct GNUNET_ATS_Session *s)
{
  struct HTTP_Client_Plugin *plugin = s->plugin;
  int res = GNUNET_OK;

  /* create url */
  if (NULL == http_common_plugin_address_to_string (plugin->protocol,
          s->address->address, s->address->address_length))
  {
    LOG (GNUNET_ERROR_TYPE_DEBUG, "Invalid address peer `%s'\n",
         GNUNET_i2s (&s->address->peer));
    return GNUNET_SYSERR;
  }

  GNUNET_asprintf (&s->url, "%s/%s;%u",
      http_common_plugin_address_to_url (NULL, s->address->address,
          s->address->address_length),
      GNUNET_i2s_full (plugin->env->my_identity), plugin->last_tag);

  plugin->last_tag++;

  LOG (GNUNET_ERROR_TYPE_DEBUG,
       "Initiating outbound session peer `%s' using address `%s'\n",
       GNUNET_i2s (&s->address->peer), s->url);

  if (GNUNET_SYSERR == client_connect_get (s))
    return GNUNET_SYSERR;

  LOG (GNUNET_ERROR_TYPE_DEBUG,
       "Session %p: connected with connection GET %d\n",
       s, s->get);

  /* Perform connect */
  GNUNET_STATISTICS_set (plugin->env->stats,
                         HTTP_STAT_STR_CONNECTIONS,
                         plugin->cur_connections,
                         GNUNET_NO);
  return res;
}


/**
 * Function obtain the network type for a session
 *
 * @param cls closure (`struct Plugin*`)
 * @param session the session
 * @return the network type
 */
static enum GNUNET_ATS_Network_Type
http_client_plugin_get_network (void *cls,
                                struct GNUNET_ATS_Session *session)
{
  return session->scope;
}


/**
 * Function obtain the network type for an address.
 *
 * @param cls closure (`struct Plugin *`)
 * @param address the address
 * @return the network type
 */
static enum GNUNET_ATS_Network_Type
http_client_plugin_get_network_for_address (void *cls,
                                            const struct GNUNET_HELLO_Address *address)
{
  struct HTTP_Client_Plugin *plugin = cls;

  return http_common_get_network_for_address (plugin->env,
                                              address);
}


/**
 * Session was idle, so disconnect it
 *
 * @param cls the `struct GNUNET_ATS_Session` of the idle session
 * @param tc scheduler context
 */
static void
client_session_timeout (void *cls,
                        const struct GNUNET_SCHEDULER_TaskContext *tc)
{
  struct GNUNET_ATS_Session *s = cls;
  struct GNUNET_TIME_Relative left;

  s->timeout_task = NULL;
  left = GNUNET_TIME_absolute_get_remaining (s->timeout);
  if (0 != left.rel_value_us)
  {
    /* not actually our turn yet, but let's at least update
       the monitor, it may think we're about to die ... */
    notify_session_monitor (s->plugin,
                            s,
                            GNUNET_TRANSPORT_SS_UPDATE);
    s->timeout_task = GNUNET_SCHEDULER_add_delayed (left,
                                                    &client_session_timeout,
                                                    s);
    return;
  }
  LOG (TIMEOUT_LOG,
       "Session %p was idle for %s, disconnecting\n",
       s,
	     GNUNET_STRINGS_relative_time_to_string (HTTP_CLIENT_SESSION_TIMEOUT,
                                               GNUNET_YES));

  GNUNET_assert (GNUNET_OK ==
                 http_client_plugin_session_disconnect (s->plugin,
                                                 s));
}


/**
 * Creates a new outbound session the transport service will use to
 * send data to the peer
 *
 * @param cls the plugin
 * @param address the address
 * @return the session or NULL of max connections exceeded
 */
static struct GNUNET_ATS_Session *
http_client_plugin_get_session (void *cls,
                                const struct GNUNET_HELLO_Address *address)
{
  struct HTTP_Client_Plugin *plugin = cls;
  struct GNUNET_ATS_Session *s;
  struct sockaddr *sa;
  enum GNUNET_ATS_Network_Type net_type;
  size_t salen = 0;
  int res;

  GNUNET_assert (NULL != address->address);

  /* find existing session */
  s = client_lookup_session (plugin, address);
  if (NULL != s)
    return s;

  if (plugin->max_connections <= plugin->cur_connections)
  {
    LOG (GNUNET_ERROR_TYPE_WARNING,
         "Maximum number of connections (%u) reached: "
         "cannot connect to peer `%s'\n",
         plugin->max_connections,
         GNUNET_i2s (&address->peer));
    return NULL;
  }

  /* Determine network location */
  net_type = GNUNET_ATS_NET_UNSPECIFIED;
  sa = http_common_socket_from_address (address->address, address->address_length, &res);
  if (GNUNET_SYSERR == res)
    return NULL;
  if (GNUNET_YES == res)
  {
    GNUNET_assert (NULL != sa);
    if (AF_INET == sa->sa_family)
    {
      salen = sizeof (struct sockaddr_in);
    }
    else if (AF_INET6 == sa->sa_family)
    {
      salen = sizeof (struct sockaddr_in6);
    }
    net_type = plugin->env->get_address_type (plugin->env->cls, sa, salen);
    GNUNET_free (sa);
  }
  else if (GNUNET_NO == res)
  {
    /* Cannot convert to sockaddr -> is external hostname */
    net_type = GNUNET_ATS_NET_WAN;
  }
  if (GNUNET_ATS_NET_UNSPECIFIED == net_type)
  {
    GNUNET_break (0);
    return NULL;
  }

  s = GNUNET_new (struct GNUNET_ATS_Session);
  s->plugin = plugin;
  s->address = GNUNET_HELLO_address_copy (address);
  s->scope = net_type;
  s->timeout = GNUNET_TIME_relative_to_absolute (HTTP_CLIENT_SESSION_TIMEOUT);
  s->timeout_task =  GNUNET_SCHEDULER_add_delayed (HTTP_CLIENT_SESSION_TIMEOUT,
                                                   &client_session_timeout,
                                                   s);
  LOG (GNUNET_ERROR_TYPE_DEBUG,
       "Created new session %p for `%s' address `%s''\n",
       s,
       http_common_plugin_address_to_string (plugin->protocol,
                                             s->address->address,
                                             s->address->address_length),
       GNUNET_i2s (&s->address->peer));

  /* add new session */
  (void) GNUNET_CONTAINER_multipeermap_put (plugin->sessions,
                                            &s->address->peer,
                                            s,
                                            GNUNET_CONTAINER_MULTIHASHMAPOPTION_MULTIPLE);

  /* initiate new connection */
  if (GNUNET_SYSERR == client_connect (s))
  {
    LOG (GNUNET_ERROR_TYPE_ERROR,
         "Cannot connect to peer `%s' address `%s''\n",
         http_common_plugin_address_to_string (plugin->protocol,
                                               s->address->address,
                                               s->address->address_length),
         GNUNET_i2s (&s->address->peer));
    client_delete_session (s);
    return NULL;
  }
  notify_session_monitor (plugin,
                          s,
                          GNUNET_TRANSPORT_SS_INIT);
  notify_session_monitor (plugin,
                          s,
                          GNUNET_TRANSPORT_SS_UP); /* or handshake? */
  return s;
}


/**
 * Another peer has suggested an address for this
 * peer and transport plugin.  Check that this could be a valid
 * address.  If so, consider adding it to the list
 * of addresses.
 *
 * @param cls closure with the `struct Plugin`
 * @param addr pointer to the address
 * @param addrlen length of @a addr
 * @return #GNUNET_OK if this is a plausible address for this peer
 *         and transport; always returns #GNUNET_NO (this is the client!)
 */
static int
http_client_plugin_address_suggested (void *cls,
                                      const void *addr,
                                      size_t addrlen)
{
  /* A HTTP/S client does not have any valid address so:*/
  return GNUNET_NO;
}


/**
 * Exit point from the plugin.
 *
 * @param cls api as closure
 * @return NULL
 */
void *
LIBGNUNET_PLUGIN_TRANSPORT_DONE (void *cls)
{
  struct GNUNET_TRANSPORT_PluginFunctions *api = cls;
  struct HTTP_Client_Plugin *plugin = api->cls;

  if (NULL == api->cls)
  {
    /* Stub shutdown */
    GNUNET_free (api);
    return NULL;
  }
  LOG (GNUNET_ERROR_TYPE_DEBUG,
       _("Shutting down plugin `%s'\n"),
       plugin->name);
  GNUNET_CONTAINER_multipeermap_iterate (plugin->sessions,
                                         &destroy_session_cb,
                                         plugin);
  LOG (GNUNET_ERROR_TYPE_DEBUG,
       _("Shutdown for plugin `%s' complete\n"),
       plugin->name);
  GNUNET_CONTAINER_multipeermap_destroy (plugin->sessions);
  GNUNET_free (plugin);
  GNUNET_free (api);
  return NULL;
}


/**
 * Configure plugin
 *
 * @param plugin the plugin handle
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on failure
 */
static int
client_configure_plugin (struct HTTP_Client_Plugin *plugin)
{
  unsigned long long max_connections;

  /* Optional parameters */
  if (GNUNET_OK != GNUNET_CONFIGURATION_get_value_number (plugin->env->cfg,
                      plugin->name,
                      "MAX_CONNECTIONS", &max_connections))
    max_connections = 128;
  plugin->max_connections = max_connections;

  LOG (GNUNET_ERROR_TYPE_DEBUG,
       _("Maximum number of connections is %u\n"),
       plugin->max_connections);
  return GNUNET_OK;
}


/**
 * Function called by the pretty printer for the resolved address for
 * each human-readable address obtained.  The callback can be called
 * several times. The last invocation must be with a @a address of
 * NULL and a @a res of #GNUNET_OK.  Thus, to indicate conversion
 * errors, the callback might be called first with @a address NULL and
 * @a res being #GNUNET_SYSERR.  In that case, there must still be a
 * subsequent call later with @a address NULL and @a res #GNUNET_OK.
 *
 * @param cls closure
 * @param address one of the names for the host, NULL on last callback
 * @param res #GNUNET_OK if conversion was successful, #GNUNET_SYSERR on failure,
 *      #GNUNET_OK on last callback
 */
static const char *
http_client_plugin_address_to_string (void *cls,
                                      const void *addr,
                                      size_t addrlen)
{
  return http_common_plugin_address_to_string (PLUGIN_NAME,
                                               addr,
                                               addrlen);
}


/**
 * Function that will be called whenever the transport service wants to
 * notify the plugin that a session is still active and in use and
 * therefore the session timeout for this session has to be updated
 *
 * @param cls closure
 * @param peer which peer was the session for
 * @param session which session is being updated
 */
static void
http_client_plugin_update_session_timeout (void *cls,
                                           const struct GNUNET_PeerIdentity *peer,
                                           struct GNUNET_ATS_Session *session)
{
  client_reschedule_session_timeout (session);
}


/**
 * Function that will be called whenever the transport service wants to
 * notify the plugin that the inbound quota changed and that the plugin
 * should update it's delay for the next receive value
 *
 * @param cls closure
 * @param peer which peer was the session for
 * @param session which session is being updated
 * @param delay new delay to use for receiving
 */
static void
http_client_plugin_update_inbound_delay (void *cls,
                                         const struct GNUNET_PeerIdentity *peer,
                                         struct GNUNET_ATS_Session *s,
                                         struct GNUNET_TIME_Relative delay)
{
  s->next_receive = GNUNET_TIME_relative_to_absolute (delay);
  LOG (GNUNET_ERROR_TYPE_DEBUG,
       "New inbound delay %s\n",
       GNUNET_STRINGS_relative_time_to_string (delay,
                                               GNUNET_NO));
}


/**
 * Return information about the given session to the
 * monitor callback.
 *
 * @param cls the `struct Plugin` with the monitor callback (`sic`)
 * @param peer peer we send information about
 * @param value our `struct GNUNET_ATS_Session` to send information about
 * @return #GNUNET_OK (continue to iterate)
 */
static int
send_session_info_iter (void *cls,
                        const struct GNUNET_PeerIdentity *peer,
                        void *value)
{
  struct HTTP_Client_Plugin *plugin = cls;
  struct GNUNET_ATS_Session *session = value;

  notify_session_monitor (plugin,
                          session,
                          GNUNET_TRANSPORT_SS_INIT);
  notify_session_monitor (plugin,
                          session,
                          GNUNET_TRANSPORT_SS_UP); /* FIXME: or handshake? */
  return GNUNET_OK;
}


/**
 * Begin monitoring sessions of a plugin.  There can only
 * be one active monitor per plugin (i.e. if there are
 * multiple monitors, the transport service needs to
 * multiplex the generated events over all of them).
 *
 * @param cls closure of the plugin
 * @param sic callback to invoke, NULL to disable monitor;
 *            plugin will being by iterating over all active
 *            sessions immediately and then enter monitor mode
 * @param sic_cls closure for @a sic
 */
static void
http_client_plugin_setup_monitor (void *cls,
                                  GNUNET_TRANSPORT_SessionInfoCallback sic,
                                  void *sic_cls)
{
  struct HTTP_Client_Plugin *plugin = cls;

  plugin->sic = sic;
  plugin->sic_cls = sic_cls;
  if (NULL != sic)
  {
    GNUNET_CONTAINER_multipeermap_iterate (plugin->sessions,
                                           &send_session_info_iter,
                                           plugin);
    /* signal end of first iteration */
    sic (sic_cls, NULL, NULL);
  }
}


/**
 * Entry point for the plugin.
 */
void *
LIBGNUNET_PLUGIN_TRANSPORT_INIT (void *cls)
{
  struct GNUNET_TRANSPORT_PluginEnvironment *env = cls;
  struct GNUNET_TRANSPORT_PluginFunctions *api;
  struct HTTP_Client_Plugin *plugin;

  if (NULL == env->receive)
  {
    /* run in 'stub' mode (i.e. as part of gnunet-peerinfo), don't fully
       initialze the plugin or the API */
    api = GNUNET_new (struct GNUNET_TRANSPORT_PluginFunctions);
    api->cls = NULL;
    api->address_to_string = &http_client_plugin_address_to_string;
    api->string_to_address = &http_common_plugin_string_to_address;
    api->address_pretty_printer = &http_common_plugin_address_pretty_printer;
    return api;
  }

  plugin = GNUNET_new (struct HTTP_Client_Plugin);
  plugin->env = env;
  plugin->sessions = GNUNET_CONTAINER_multipeermap_create (128,
                                                           GNUNET_YES);
  api = GNUNET_new (struct GNUNET_TRANSPORT_PluginFunctions);
  api->cls = plugin;
  api->send = &http_client_plugin_send;
  api->disconnect_session = &http_client_plugin_session_disconnect;
  api->query_keepalive_factor = &http_client_query_keepalive_factor;
  api->disconnect_peer = &http_client_plugin_peer_disconnect;
  api->check_address = &http_client_plugin_address_suggested;
  api->get_session = &http_client_plugin_get_session;
  api->address_to_string = &http_client_plugin_address_to_string;
  api->string_to_address = &http_common_plugin_string_to_address;
  api->address_pretty_printer = &http_common_plugin_address_pretty_printer;
  api->get_network = &http_client_plugin_get_network;
  api->get_network_for_address = &http_client_plugin_get_network_for_address;
  api->update_session_timeout = &http_client_plugin_update_session_timeout;
  api->update_inbound_delay = &http_client_plugin_update_inbound_delay;
  api->setup_monitor = &http_client_plugin_setup_monitor;
#if BUILD_HTTPS
  plugin->name = "transport-https_client";
  plugin->protocol = "https";
#else
  plugin->name = "transport-http_client";
  plugin->protocol = "http";
#endif
  plugin->last_tag = 1;

  if (GNUNET_SYSERR == client_configure_plugin (plugin))
  {
    LIBGNUNET_PLUGIN_TRANSPORT_DONE (api);
    return NULL;
  }
  return api;
}

/* vim: set expandtab ts=2 sw=2: */
