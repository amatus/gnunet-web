/*
     This file is part of GNUnet
     (C) 2014 David Barksdale <amatus@amatus.name>
     (C) 2002-2013 Christian Grothoff (and other contributing authors)

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
     Free Software Foundation, Inc., 59 Temple Place - Suite 330,
     Boston, MA 02111-1307, USA.
*/

/**
 * @file transport/plugin_transport_http_client.c
 * @brief HTTP/S client transport plugin
 * @author Matthias Wachs
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
   * amount of data already sent
   */
  size_t pos;

  /**
   * buffer length
   */
  size_t size;

  /**
   * Continuation function to call once the transmission buffer
   * has again space available.  NULL if there is no
   * continuation to call.
   */
  GNUNET_TRANSPORT_TransmitContinuation transmit_cont;

  /**
   * Closure for transmit_cont.
   */
  void *transmit_cont_cls;
};


/**
 * Session handle for connections.
 */
struct Session;


/**
 * A connection handle
 *
 */
struct ConnectionHandle
{
  /**
   * The curl easy handle
   */
  //XXX CURL *easyhandle;

  /**
   * The related session
   */
  struct Session *s;
};


/**
 * Session handle for connections.
 */
struct Session
{
  /**
   * To whom are we talking to (set to our identity
   * if we are still waiting for the welcome message)
   */
  struct GNUNET_PeerIdentity target;

  /**
   * Stored in a linked list.
   */
  struct Session *next;

  /**
   * Stored in a linked list.
   */
  struct Session *prev;

  /**
   * The URL to connect to
   */
  char *url;

  /**
   * Address
   */
  struct GNUNET_HELLO_Address *address;

  /**
   * ATS network type in NBO
   */
  uint32_t ats_address_network_type;

  /**
   * Pointer to the global plugin struct.
   */
  struct HTTP_Client_Plugin *plugin;

  /**
   * Client send handle
   */
  void *client_put;

  struct ConnectionHandle put;
  struct ConnectionHandle get;

  /**
   * Is the client PUT handle currently paused
   */
  int put_paused;

  /**
   * Is the client PUT handle disconnect in progress?
   */
  int put_tmp_disconnecting;

  /**
   * Is the client PUT handle temporarily disconnected?
   */
  int put_tmp_disconnected;

  /**
   * We received data to send while disconnecting, reconnect immediately
   */
  int put_reconnect_required;

  /**
   * Client receive handle
   */
  void *client_get;

  /**
   * Outbound overhead due to HTTP connection
   * Add to next message of this session when calling callback
   */
  size_t overhead;

  /**
   * next pointer for double linked list
   */
  struct HTTP_Message *msg_head;

  /**
   * previous pointer for double linked list
   */
  struct HTTP_Message *msg_tail;

  /**
   * Message stream tokenizer for incoming data
   */
  struct GNUNET_SERVER_MessageStreamTokenizer *msg_tk;

  /**
   * Session timeout task
   */
  //XXX GNUNET_SCHEDULER_TaskIdentifier put_disconnect_task;

  /**
   * Session timeout task
   */
  GNUNET_SCHEDULER_TaskIdentifier timeout_task;

  /**
   * Task to wake up client receive handle when receiving is allowed again
   */
  GNUNET_SCHEDULER_TaskIdentifier recv_wakeup_task;

  /**
  * Absolute time when to receive data again
  * Used for receive throttling
  */
  struct GNUNET_TIME_Absolute next_receive;
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
   * Linked list head of open sessions.
   */
  struct Session *head;

  /**
   * Linked list tail of open sessions.
   */
  struct Session *tail;

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

  /**
   * cURL Multihandle
   */
  //XXX CURLM *curl_multi_handle;

  /**
   * curl perform task
   */
  //XXX GNUNET_SCHEDULER_TaskIdentifier client_perform_task;
};


/**
 * Increment session timeout due to activity for a session
 * @param s the session
 */
static void
client_reschedule_session_timeout (struct Session *s);


/**
 * Connect a HTTP put connection
 *
 * @param s the session to connect
 * @return #GNUNET_SYSERR for hard failure, #GNUNET_OK for success
 */
static int
client_connect_put (struct Session *s);


/**
 * Does a session s exists?
 *
 * @param plugin the plugin
 * @param s desired session
 * @return #GNUNET_YES or #GNUNET_NO
 */
static int
client_exist_session (struct HTTP_Client_Plugin *plugin,
                      struct Session *s)
{
  struct Session * head;

  for (head = plugin->head; head != NULL; head = head->next)
    if (head == s)
      return GNUNET_YES;
  return GNUNET_NO;
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
 * @param msgbuf_size number of bytes in 'msgbuf'
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
                         struct Session *s,
                         const char *msgbuf, size_t msgbuf_size,
                         unsigned int priority,
                         struct GNUNET_TIME_Relative to,
                         GNUNET_TRANSPORT_TransmitContinuation cont,
                         void *cont_cls)
{
  struct HTTP_Client_Plugin *plugin = cls;
  struct HTTP_Message *msg;
  char *stat_txt;

  /* lookup if session is really existing */
  if (GNUNET_YES != client_exist_session (plugin, s))
  {
    GNUNET_break (0);
    return GNUNET_SYSERR;
  }

  GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, s->plugin->name,
                   "Session %p/connection %p: Sending message with %u to peer `%s' \n",
                   s, s->client_put,
                   msgbuf_size, GNUNET_i2s (&s->target));

  /* create new message and schedule */
  msg = GNUNET_malloc (sizeof (struct HTTP_Message) + msgbuf_size);
  msg->next = NULL;
  msg->size = msgbuf_size;
  msg->pos = 0;
  msg->buf = (char *) &msg[1];
  msg->transmit_cont = cont;
  msg->transmit_cont_cls = cont_cls;
  memcpy (msg->buf, msgbuf, msgbuf_size);
  GNUNET_CONTAINER_DLL_insert_tail (s->msg_head, s->msg_tail, msg);

  GNUNET_asprintf (&stat_txt,
                   "# bytes currently in %s_client buffers",
                   plugin->protocol);
  GNUNET_STATISTICS_update (plugin->env->stats,
                            stat_txt, msgbuf_size, GNUNET_NO);
  GNUNET_free (stat_txt);

  if (GNUNET_YES == s->put_tmp_disconnecting)
  {
    /* PUT connection is currently getting disconnected */
    s->put_reconnect_required = GNUNET_YES;
    GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, s->plugin->name,
                     "Session %p/connection %jp: currently disconnecting, reconnecting immediately\n",
                     s, s->client_put);
    return msgbuf_size;
  }
  else if (GNUNET_YES == s->put_paused)
  {
    /* PUT connection was paused, unpause */
    /*XXX GNUNET_assert (s->put_disconnect_task != GNUNET_SCHEDULER_NO_TASK);
    GNUNET_SCHEDULER_cancel (s->put_disconnect_task);
    s->put_disconnect_task = GNUNET_SCHEDULER_NO_TASK;*/
    GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, s->plugin->name,
                     "Session %p/connection %p: unpausing connection\n",
                     s, s->client_put);
    s->put_paused = GNUNET_NO;
    if (NULL != s->client_put)
      ; //XXX curl_easy_pause (s->client_put, CURLPAUSE_CONT);
  }
  else if (GNUNET_YES == s->put_tmp_disconnected)
  {
    /* PUT connection was disconnected, reconnect */
    GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, s->plugin->name,
                     "Session %p: Reconnecting PUT connection\n",
                     s);
    s->put_tmp_disconnected = GNUNET_NO;
    GNUNET_break (s->client_put == NULL);
    if (GNUNET_SYSERR == client_connect_put (s))
    {
      return GNUNET_SYSERR;
    }
  }

  return msgbuf_size;
}


/**
 * Delete session s
 *
 * @param s the session to delete
 */
static void
client_delete_session (struct Session *s)
{
  struct HTTP_Client_Plugin *plugin = s->plugin;
  struct HTTP_Message *pos;
  struct HTTP_Message *next;

  if (GNUNET_SCHEDULER_NO_TASK != s->timeout_task)
  {
    GNUNET_SCHEDULER_cancel (s->timeout_task);
    s->timeout_task = GNUNET_SCHEDULER_NO_TASK;
  }
  /*XXX if (GNUNET_SCHEDULER_NO_TASK != s->put_disconnect_task)
  {
      GNUNET_SCHEDULER_cancel (s->put_disconnect_task);
      s->put_disconnect_task = GNUNET_SCHEDULER_NO_TASK;
  }*/

  GNUNET_CONTAINER_DLL_remove (plugin->head, plugin->tail, s);

  next = s->msg_head;
  while (NULL != (pos = next))
  {
    next = pos->next;
    GNUNET_CONTAINER_DLL_remove (s->msg_head, s->msg_tail, pos);
    if (pos->transmit_cont != NULL)
      pos->transmit_cont (pos->transmit_cont_cls, &s->target, GNUNET_SYSERR,
                          pos->size, pos->pos + s->overhead);
    s->overhead = 0;
    GNUNET_free (pos);
  }

  if (s->msg_tk != NULL)
  {
    GNUNET_SERVER_mst_destroy (s->msg_tk);
    s->msg_tk = NULL;
  }
  GNUNET_HELLO_address_free (s->address);
  GNUNET_free (s->url);
  GNUNET_free (s);
}


/**
 * Disconnect a session
 *
 * @param cls the `struct HTTP_Client_Plugin`
 * @param s session
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on error
 */
static int
http_client_session_disconnect (void *cls,
                                struct Session *s)
{
  struct HTTP_Client_Plugin *plugin = cls;
  struct HTTP_Message *msg;
  struct HTTP_Message *t;
  int res = GNUNET_OK;

  if (GNUNET_YES != client_exist_session (plugin, s))
  {
    GNUNET_break (0);
    return GNUNET_SYSERR;
  }

  if (NULL != s->client_put)
  {
    GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, plugin->name,
                     "Session %p/connection %p: disconnecting PUT connection to peer `%s'\n",
                     s, s->client_put, GNUNET_i2s (&s->target));

    /* remove curl handle from multi handle */
    //XXX curl_easy_cleanup (s->client_put);
    s->client_put = NULL;
  }


  if (s->recv_wakeup_task != GNUNET_SCHEDULER_NO_TASK)
  {
    GNUNET_SCHEDULER_cancel (s->recv_wakeup_task);
    s->recv_wakeup_task = GNUNET_SCHEDULER_NO_TASK;
  }

  if (NULL != s->client_get)
  {
    GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, plugin->name,
                     "Session %p/connection %p: disconnecting GET connection to peer `%s'\n",
                     s, s->client_get,
                     GNUNET_i2s (&s->target));
    /* remove curl handle from multi handle */
    //XXX curl_easy_cleanup (s->client_get);
    s->client_get = NULL;
  }

  msg = s->msg_head;
  while (NULL != msg)
  {
    t = msg->next;
    if (NULL != msg->transmit_cont)
      msg->transmit_cont (msg->transmit_cont_cls, &s->target, GNUNET_SYSERR,
                          msg->size, msg->pos + s->overhead);
    s->overhead = 0;
    GNUNET_CONTAINER_DLL_remove (s->msg_head, s->msg_tail, msg);
    GNUNET_free (msg);
    msg = t;
  }

  GNUNET_assert (plugin->cur_connections >= 2);
  plugin->cur_connections -= 2;
  GNUNET_STATISTICS_set (plugin->env->stats,
                         HTTP_STAT_STR_CONNECTIONS,
                         plugin->cur_connections,
                         GNUNET_NO);
  GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, plugin->name,
                   "Session %p: notifying transport about ending session\n",s);

  plugin->env->session_end (plugin->env->cls, &s->target, s);
  client_delete_session (s);

  return res;
}


/**
 * Function that is called to get the keepalive factor.
 * GNUNET_CONSTANTS_IDLE_CONNECTION_TIMEOUT is divided by this number to
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
 * Function that can be used to force the plugin to disconnect
 * from the given peer and cancel all previous transmissions
 * (and their continuationc).
 *
 * @param cls closure
 * @param target peer from which to disconnect
 */
static void
http_client_peer_disconnect (void *cls,
                             const struct GNUNET_PeerIdentity *target)
{
  struct HTTP_Client_Plugin *plugin = cls;
  struct Session *next = NULL;
  struct Session *pos = NULL;

  GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, plugin->name,
                   "Transport tells me to disconnect `%s'\n",
                   GNUNET_i2s (target));

  next = plugin->head;
  while (NULL != (pos = next))
  {
    next = pos->next;
    if (0 == memcmp (target, &pos->target, sizeof (struct GNUNET_PeerIdentity)))
    {
      GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, plugin->name,
                       "Disconnecting session %p to `%pos'\n",
                       pos, GNUNET_i2s (target));
      GNUNET_assert (GNUNET_OK == http_client_session_disconnect (plugin,
                                                                  pos));
    }
  }
}


/**
 * Check if a sessions exists for an specific address
 *
 * @param plugin the plugin
 * @param address the address
 * @return the session or NULL
 */
static struct Session *
client_lookup_session (struct HTTP_Client_Plugin *plugin,
                       const struct GNUNET_HELLO_Address *address)
{
  struct Session *pos;

  for (pos = plugin->head; NULL != pos; pos = pos->next)
  {
    if ((0 == memcmp (&address->peer, &pos->target, sizeof (struct GNUNET_PeerIdentity))) &&
        (0 == GNUNET_HELLO_address_cmp(address, pos->address)))
      return pos;
  }
  return NULL;
}


/**
 * Wake up a curl handle which was suspended
 *
 * @param cls the session
 * @param tc task context
 */
static void
client_wake_up (void *cls,
                const struct GNUNET_SCHEDULER_TaskContext *tc)
{
  struct Session *s = cls;
  struct HTTP_Client_Plugin *p = s->plugin;

  if (GNUNET_YES != client_exist_session (p, s))
  {
    GNUNET_break (0);
    return;
  }
  s->recv_wakeup_task = GNUNET_SCHEDULER_NO_TASK;
  if (0 != (tc->reason & GNUNET_SCHEDULER_REASON_SHUTDOWN))
    return;
  GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, s->plugin->name,
                   "Session %p/connection %p: Waking up GET handle\n",
                   s,
                   s->client_get);
  s->put_paused = GNUNET_NO;
  if (NULL != s->client_get)
    ; //XXX curl_easy_pause (s->client_get, CURLPAUSE_CONT);

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
client_receive_mst_cb (void *cls, void *client,
                       const struct GNUNET_MessageHeader *message)
{
  struct Session *s = cls;
  struct HTTP_Client_Plugin *plugin;
  struct GNUNET_TIME_Relative delay;
  struct GNUNET_ATS_Information atsi;
  char *stat_txt;

  plugin = s->plugin;
  if (GNUNET_YES != client_exist_session (plugin, s))
  {
    GNUNET_break (0);
    return GNUNET_OK;
  }

  atsi.type = htonl (GNUNET_ATS_NETWORK_TYPE);
  atsi.value = s->ats_address_network_type;
  GNUNET_break (s->ats_address_network_type != ntohl (GNUNET_ATS_NET_UNSPECIFIED));

  delay = s->plugin->env->receive (plugin->env->cls, s->address, s, message);
  plugin->env->update_address_metrics (plugin->env->cls,
				       s->address, s,
				       &atsi, 1);

  GNUNET_asprintf (&stat_txt,
                   "# bytes received via %s_client",
                   plugin->protocol);
  GNUNET_STATISTICS_update (plugin->env->stats,
                            stat_txt, ntohs(message->size), GNUNET_NO);
  GNUNET_free (stat_txt);

  s->next_receive =
      GNUNET_TIME_absolute_add (GNUNET_TIME_absolute_get (), delay);

  if (GNUNET_TIME_absolute_get ().abs_value_us < s->next_receive.abs_value_us)
  {
    GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, plugin->name,
                     "Client: peer `%s' address `%s' next read delayed for %s\n",
                     GNUNET_i2s (&s->target),
                     http_common_plugin_address_to_string (NULL,
			s->plugin->protocol, s->address->address,
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
client_receive (void *stream, size_t size, size_t nmemb, void *cls)
{
  struct Session *s = cls;
  struct GNUNET_TIME_Absolute now;
  size_t len = size * nmemb;

  GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, s->plugin->name,
                   "Session %p / connection %p: Received %u bytes from peer `%s'\n",
                   s, s->client_get,
                   len, GNUNET_i2s (&s->target));
  now = GNUNET_TIME_absolute_get ();
  if (now.abs_value_us < s->next_receive.abs_value_us)
  {
    struct GNUNET_TIME_Absolute now = GNUNET_TIME_absolute_get ();
    struct GNUNET_TIME_Relative delta =
        GNUNET_TIME_absolute_get_difference (now, s->next_receive);
    GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, s->plugin->name,
                     "Session %p / connection %p: No inbound bandwidth available! Next read was delayed for %s\n",
                     s, s->client_get,
		     GNUNET_STRINGS_relative_time_to_string (delta,
							     GNUNET_YES));
    if (s->recv_wakeup_task != GNUNET_SCHEDULER_NO_TASK)
    {
      GNUNET_SCHEDULER_cancel (s->recv_wakeup_task);
      s->recv_wakeup_task = GNUNET_SCHEDULER_NO_TASK;
    }
    s->recv_wakeup_task =
        GNUNET_SCHEDULER_add_delayed (delta, &client_wake_up, s);
    return 0; //XXX CURL_WRITEFUNC_PAUSE;
  }
  if (NULL == s->msg_tk)
    s->msg_tk = GNUNET_SERVER_mst_create (&client_receive_mst_cb, s);
  GNUNET_SERVER_mst_receive (s->msg_tk, s, stream, len, GNUNET_NO, GNUNET_NO);
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
client_connect_get (struct Session *s)
{
  /* create get connection */
  s->client_get = (void *)next_xhr++;
  EM_ASM({
    var xhr = new goog.net.XhrIo();
    xhrs[$0] = xhr;
    xhr.setResponseType(goog.net.XhrIo.ResponseType.ARRAY_BUFFER);
    xhr.setTimeoutInterval($1);
    goog.events.listen(xhr, goog.net.EventType.COMPLETE, function(e) {
      var response = e.target.getResponse();
      ccall('client_receive', 'number', ['array', 'number', 'number', 'number'],
        [response, response.length, 1, $2]);
    });
    xhr.send($3);
  }, next_xhr, (long)(HTTP_CLIENT_NOT_VALIDATED_TIMEOUT.rel_value_us / 1000LL),
  s, s->url);
  return GNUNET_OK;
}


/**
 * Connect a HTTP put connection
 *
 * @param s the session to connect
 * @return #GNUNET_SYSERR for hard failure, #GNUNET_OK for ok
 */
static int
client_connect_put (struct Session *s)
{
  /* create put connection */
  s->client_put = NULL; //XXX curl_easy_init ();
  s->put_tmp_disconnected = GNUNET_NO;
  return GNUNET_OK;
}


/**
 * Connect both PUT and GET connection for a session
 *
 * @param s the session to connect
 * @return #GNUNET_OK on success, #GNUNET_SYSERR otherwise
 */
static int
client_connect (struct Session *s)
{

  struct HTTP_Client_Plugin *plugin = s->plugin;
  int res = GNUNET_OK;

  /* create url */
  if (NULL == http_common_plugin_address_to_string (NULL,
        plugin->protocol, s->address->address, s->address->address_length))
  {
    GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG,
                     plugin->name,
                     "Invalid address peer `%s'\n",
                     GNUNET_i2s (&s->target));
    return GNUNET_SYSERR;
  }

  GNUNET_asprintf (&s->url, "%s/%s;%u",
		   http_common_plugin_address_to_url (NULL, s->address->address,
		       s->address->address_length),
		   GNUNET_i2s_full (plugin->env->my_identity),
		   plugin->last_tag);

  plugin->last_tag++;

  GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, plugin->name,
                   "Initiating outbound session peer `%s' using address `%s'\n",
                   GNUNET_i2s (&s->target), s->url);

  if ((GNUNET_SYSERR == client_connect_get (s)) ||
      (GNUNET_SYSERR == client_connect_put (s)))
  {
    GNUNET_break (0);
    return GNUNET_SYSERR;
  }

  GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG,
                   plugin->name,
                   "Session %p: connected with connections GET %p and PUT %p\n",
                   s, s->client_get, s->client_put);

  /* Perform connect */
  plugin->cur_connections += 2;
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
http_client_get_network (void *cls,
			 struct Session *session)
{
  return ntohl (session->ats_address_network_type);
}


/**
 * Session was idle, so disconnect it
 *
 * @param cls the `struct Session` of the idle session
 * @param tc scheduler context
 */
static void
client_session_timeout (void *cls,
                        const struct GNUNET_SCHEDULER_TaskContext *tc)
{
  struct Session *s = cls;

  s->timeout_task = GNUNET_SCHEDULER_NO_TASK;
  GNUNET_log (TIMEOUT_LOG,
              "Session %p was idle for %s, disconnecting\n",
              s,
	      GNUNET_STRINGS_relative_time_to_string (HTTP_CLIENT_SESSION_TIMEOUT,
						      GNUNET_YES));

  /* call session destroy function */
  GNUNET_assert (GNUNET_OK == http_client_session_disconnect (s->plugin,
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
static struct Session *
http_client_plugin_get_session (void *cls,
                                const struct GNUNET_HELLO_Address *address)
{
  struct HTTP_Client_Plugin *plugin = cls;
  struct Session * s = NULL;
  struct sockaddr *sa;
  struct GNUNET_ATS_Information ats;
  size_t salen = 0;
  int res;

  GNUNET_assert (address->address != NULL);

  /* find existing session */
  s = client_lookup_session (plugin, address);
  if (s != NULL)
    return s;

  if (plugin->max_connections <= plugin->cur_connections)
  {
    GNUNET_log_from (GNUNET_ERROR_TYPE_WARNING, plugin->name,
                     "Maximum number of connections (%u) reached: "
                     "cannot connect to peer `%s'\n",
                     plugin->max_connections,
                     GNUNET_i2s (&address->peer));
    return NULL;
  }

  /* Determine network location */
  ats.type = htonl (GNUNET_ATS_NETWORK_TYPE);
  ats.value = htonl (GNUNET_ATS_NET_UNSPECIFIED);
  sa = http_common_socket_from_address (address->address, address->address_length, &res);
  if (GNUNET_SYSERR == res)
  {
    return NULL;
  }
  else if (GNUNET_YES == res)
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
    ats = plugin->env->get_address_type (plugin->env->cls, sa, salen);
    //fprintf (stderr, "Address %s is in %s\n", GNUNET_a2s (sa,salen), GNUNET_ATS_print_network_type(ntohl(ats.value)));
    GNUNET_free (sa);
  }
  else if (GNUNET_NO == res)
  {
    /* Cannot convert to sockaddr -> is external hostname */
    ats.value = htonl (GNUNET_ATS_NET_WAN);
  }
  if (GNUNET_ATS_NET_UNSPECIFIED == ntohl (ats.value))
  {
    GNUNET_break (0);
    return NULL;
  }

  s = GNUNET_new (struct Session);
  s->target = address->peer;
  s->plugin = plugin;
  s->address = GNUNET_HELLO_address_copy (address);
  s->ats_address_network_type = ats.value;
  s->put_paused = GNUNET_NO;
  s->put_tmp_disconnecting = GNUNET_NO;
  s->put_tmp_disconnected = GNUNET_NO;
  s->timeout_task =  GNUNET_SCHEDULER_add_delayed (HTTP_CLIENT_SESSION_TIMEOUT,
                                                   &client_session_timeout,
                                                   s);
  GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, plugin->name,
                   "Created new session %p for `%s' address `%s''\n",
                   s, http_common_plugin_address_to_string (NULL,
                       plugin->protocol, s->address->address,
                       s->address->address_length),
                   GNUNET_i2s (&s->target));

  /* add new session */
  GNUNET_CONTAINER_DLL_insert (plugin->head, plugin->tail, s);

  /* initiate new connection */
  if (GNUNET_SYSERR == client_connect (s))
  {
    GNUNET_log_from (GNUNET_ERROR_TYPE_ERROR, plugin->name,
                     "Cannot connect to peer `%s' address `%s''\n",
                     http_common_plugin_address_to_string (NULL,
                                            plugin->protocol, s->address->address,
                                            s->address->address_length),
                     GNUNET_i2s (&s->target));
    client_delete_session (s);
    return NULL;
  }
  return s;
}


/**
 * Setup http_client plugin
 *
 * @param plugin the plugin handle
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on error
 */
static int
client_start (struct HTTP_Client_Plugin *plugin)
{
  EM_ASM(goog.require('goog.net.XhrIo'));
  return GNUNET_OK;
}


/**
 * Increment session timeout due to activity for session s
 *
 * param s the session
 */
static void
client_reschedule_session_timeout (struct Session *s)
{
  GNUNET_assert (GNUNET_SCHEDULER_NO_TASK != s->timeout_task);
  GNUNET_SCHEDULER_cancel (s->timeout_task);
  s->timeout_task =  GNUNET_SCHEDULER_add_delayed (HTTP_CLIENT_SESSION_TIMEOUT,
                                                   &client_session_timeout,
                                                   s);
  GNUNET_log (TIMEOUT_LOG,
              "Timeout rescheduled for session %p set to %s\n",
              s,
              GNUNET_STRINGS_relative_time_to_string (HTTP_CLIENT_SESSION_TIMEOUT,
                                                      GNUNET_YES));
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
  /* struct Plugin *plugin = cls; */

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
  struct Session *pos;
  struct Session *next;

  if (NULL == api->cls)
  {
    /* Stub shutdown */
    GNUNET_free (api);
    return NULL;
  }

  GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, plugin->name,
                   _("Shutting down plugin `%s'\n"),
                   plugin->name);


  next = plugin->head;
  while (NULL != (pos = next))
  {
    next = pos->next;
    http_client_session_disconnect (plugin, pos);
  }

  GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, plugin->name,
                   _("Shutdown for plugin `%s' complete\n"),
                   plugin->name);

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

  GNUNET_log_from (GNUNET_ERROR_TYPE_DEBUG, plugin->name,
                   _("Maximum number of connections is %u\n"),
                   plugin->max_connections);
  return GNUNET_OK;
}


static const char *
http_plugin_address_to_string (void *cls,
                               const void *addr,
                               size_t addrlen)
{
  return http_common_plugin_address_to_string (cls, PLUGIN_NAME, addr, addrlen);
}


static void
http_client_plugin_update_session_timeout (void *cls,
                                  const struct GNUNET_PeerIdentity *peer,
                                  struct Session *session)
{
  struct HTTP_Client_Plugin *plugin = cls;

  /* lookup if session is really existing */
  if (GNUNET_YES != client_exist_session (plugin, session))
  {
    GNUNET_break (0);
    return;
  }
  client_reschedule_session_timeout (session);
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
    api->address_to_string = &http_plugin_address_to_string;
    api->string_to_address = &http_common_plugin_string_to_address;
    api->address_pretty_printer = &http_common_plugin_address_pretty_printer;
    return api;
  }

  plugin = GNUNET_new (struct HTTP_Client_Plugin);
  plugin->env = env;
  api = GNUNET_new (struct GNUNET_TRANSPORT_PluginFunctions);
  api->cls = plugin;
  api->send = &http_client_plugin_send;
  api->disconnect_session = &http_client_session_disconnect;
  api->query_keepalive_factor = &http_client_query_keepalive_factor;
  api->disconnect_peer = &http_client_peer_disconnect;
  api->check_address = &http_client_plugin_address_suggested;
  api->get_session = &http_client_plugin_get_session;
  api->address_to_string = &http_plugin_address_to_string;
  api->string_to_address = &http_common_plugin_string_to_address;
  api->address_pretty_printer = &http_common_plugin_address_pretty_printer;
  api->get_network = &http_client_get_network;
  api->update_session_timeout = &http_client_plugin_update_session_timeout;

#if BUILD_HTTPS
  plugin->name = "transport-https_client";
  plugin->protocol = "https";
#else
  plugin->name = "transport-http_client";
  plugin->protocol = "http";
#endif
  plugin->last_tag = 1;
  plugin->options = 0; /* Setup options */

  if (GNUNET_SYSERR == client_configure_plugin (plugin))
  {
    LIBGNUNET_PLUGIN_TRANSPORT_DONE (api);
    return NULL;
  }

  /* Start client */
  if (GNUNET_SYSERR == client_start (plugin))
  {
    LIBGNUNET_PLUGIN_TRANSPORT_DONE (api);
    return NULL;
  }
  return api;
}

/* vim: set expandtab ts=2 sw=2: */
