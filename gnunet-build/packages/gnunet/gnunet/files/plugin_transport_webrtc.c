/*
     This file is part of GNUnet
     Copyright (C) 2002-2018 GNUnet e.V.

     GNUnet is free software: you can redistribute it and/or modify it
     under the terms of the GNU Affero General Public License as published
     by the Free Software Foundation, either version 3 of the License,
     or (at your option) any later version.

     GNUnet is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Affero General Public License for more details.
    
     You should have received a copy of the GNU Affero General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file transport/plugin_transport_webrtc.c
 * @brief WebRTC transport plugin
 * @author Christian Grothoff
 * @author David Barksdale <amatus@amat.us>
 */

#include "platform.h"
#include "gnunet_cadet_service.h"
#include "gnunet_protocols.h"
#include "gnunet_statistics_service.h"
#include "gnunet_transport_plugin.h"
#include "gnunet_transport_service.h"
#include "gnunet_util_lib.h"

#define PLUGIN_NAME "webrtc"
#define LOG(kind,...) GNUNET_log_from (kind, PLUGIN_NAME,__VA_ARGS__)

#define MESSAGE_TYPE_WEBRTC_OFFER 1
#define MESSAGE_TYPE_WEBRTC_ANSWER 2

/**
 * Encapsulation of all of the state of the plugin.
 */
struct Plugin;


/**
 * Session handle for connections.
 */
struct GNUNET_ATS_Session
{
  /**
   * To whom are we talking.
   */
  struct GNUNET_PeerIdentity peer;

  /**
   * Pointer to the global plugin struct.
   */
  struct Plugin *plugin;

  /**
   * Cadet channel for SDP exchange.
   */
  struct GNUNET_CADET_Channel *channel;

  /**
   * Handle to RTCPeerConnection
   */
  int rtc_peer_connection;

  /**
   * The client (used to identify this connection)
   */
  /* void *client; */

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
   * At what time did we reset @e last_received last?
   */
  struct GNUNET_TIME_Absolute last_quota_update;

  /**
   * How many bytes have we received since the @e last_quota_update
   * timestamp?
   */
  uint64_t last_received;

  /**
   * Number of bytes per ms that this peer is allowed
   * to send to us.
   */
  uint32_t quota;

};

/**
 * Encapsulation of all of the state of the plugin.
 */
struct Plugin
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
   * Cadet service handle.
   */
  struct GNUNET_CADET_Handle *cadet;

  /**
   * Cadet port for incoming connections.
   */
  struct GNUNET_CADET_Port *in_port;

  /**
   * Pre-computed port "number".
   */
  struct GNUNET_HashCode port;
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
notify_session_monitor (struct Plugin *plugin,
                        struct GNUNET_ATS_Session *session,
                        enum GNUNET_TRANSPORT_SessionState state)
{
  struct GNUNET_TRANSPORT_SessionInfo info;

  if (NULL == plugin->sic)
    return;
  memset (&info, 0, sizeof (info));
  info.state = state;
  info.is_inbound = GNUNET_SYSERR; /* FIXME */
  // info.num_msg_pending =
  // info.num_bytes_pending =
  // info.receive_delay =
  // info.session_timeout = session->timeout;
  // info.address = session->address;
  plugin->sic (plugin->sic_cls,
               session,
               &info);
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
 * @param session which session must be used
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
 * @param cont_cls closure for @a cont
 * @return number of bytes used (on the physical network, with overheads);
 *         -1 on hard errors (i.e. address invalid); 0 is a legal value
 *         and does NOT mean that the message was not transmitted (DV)
 */
static ssize_t
webrtc_plugin_send (void *cls,
                    struct GNUNET_ATS_Session *session,
                    const char *msgbuf,
                    size_t msgbuf_size,
                    unsigned int priority,
                    struct GNUNET_TIME_Relative to,
                    GNUNET_TRANSPORT_TransmitContinuation cont,
                    void *cont_cls)
{
  /*  struct Plugin *plugin = cls; */
  ssize_t bytes_sent = 0;

  return bytes_sent;
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
webrtc_plugin_disconnect_peer (void *cls,
                               const struct GNUNET_PeerIdentity *target)
{
  struct Plugin *plugin = cls;
  
  GNUNET_break (0);
}


/**
 * Function that can be used to force the plugin to disconnect
 * from the given peer and cancel all previous transmissions
 * (and their continuationc).
 *
 * @param cls closure
 * @param session session from which to disconnect
 * @return #GNUNET_OK on success
 */
static int
webrtc_plugin_disconnect_session (void *cls,
                                  struct GNUNET_ATS_Session *session)
{
  struct Plugin *plugin = cls;
  
  GNUNET_break (0);
  return GNUNET_SYSERR;
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
webrtc_plugin_query_keepalive_factor (void *cls)
{
  return 5;
}


/**
 * Function obtain the network type for a session
 *
 * @param cls closure ('struct Plugin*')
 * @param session the session
 * @return the network type in HBO or #GNUNET_SYSERR
 */
static enum GNUNET_ATS_Network_Type
webrtc_plugin_get_network (void *cls,
                           struct GNUNET_ATS_Session *session)
{
  return GNUNET_ATS_NET_WAN;
}


/**
 * Function obtain the network type for an address.
 *
 * @param cls closure (`struct Plugin *`)
 * @param address the address
 * @return the network type
 */
static enum GNUNET_ATS_Network_Type
webrtc_plugin_get_network_for_address (void *cls,
                                       const struct GNUNET_HELLO_Address *address)
{
  return GNUNET_ATS_NET_WAN;
}


/**
 * Function called for a quick conversion of the binary address to
 * a numeric address.  Note that the caller must not free the
 * address and that the next call to this function is allowed
 * to override the address again.
 *
 * @param cls closure
 * @param addr binary address
 * @param addrlen length of the address
 * @return string representing the same address
 */
static const char *
webrtc_plugin_address_to_string (void *cls, const void *addr, size_t addrlen)
{
  uint32_t options;
  static char buf[7 + 10 + 1];

  if (4 != addrlen)
  {
    LOG (GNUNET_ERROR_TYPE_WARNING,
        _("Unexpected address length: %u bytes\n"),
        (unsigned int) addrlen);
    return NULL;
  }
  options = ntohl (*(uint32_t *)addr);
  GNUNET_snprintf (buf, sizeof(buf), "webrtc.%u", options);
  return buf;
}


/**
 * Convert the transports address to a nice, human-readable
 * format.
 *
 * @param cls closure
 * @param type name of the transport that generated the address
 * @param addr one of the addresses of the host, NULL for the last address
 *        the specific address format depends on the transport
 * @param addrlen length of the address
 * @param numeric should (IP) addresses be displayed in numeric form?
 * @param timeout after how long should we give up?
 * @param asc function to call on each string
 * @param asc_cls closure for @a asc
 */
static void
webrtc_plugin_address_pretty_printer (void *cls, const char *type,
                                      const void *addr, size_t addrlen,
                                      int numeric,
                                      struct GNUNET_TIME_Relative timeout,
                                      GNUNET_TRANSPORT_AddressStringCallback
                                      asc, void *asc_cls)
{
  const char *str = webrtc_plugin_address_to_string (cls, addr, addrlen);

  if (NULL == str)
  {
    asc (asc_cls, NULL, GNUNET_SYSERR); /* invalid address */
  }
  else
  {
    asc (asc_cls, str, GNUNET_OK); /* return address */
  }
  asc (asc_cls, NULL, GNUNET_OK); /* done */
}



/**
 * Another peer has suggested an address for this
 * peer and transport plugin.  Check that this could be a valid
 * address.  If so, consider adding it to the list
 * of addresses.
 *
 * @param cls closure
 * @param addr pointer to the address
 * @param addrlen length of addr
 * @return #GNUNET_OK if this is a plausible address for this peer
 *         and transport
 */
static int
webrtc_plugin_address_suggested (void *cls, const void *addr, size_t addrlen)
{
  return GNUNET_OK;
}


/**
 * Function called to convert a string address to
 * a binary address.
 *
 * @param cls closure ('struct Plugin*')
 * @param addr string address
 * @param addrlen length of the @a addr
 * @param buf location to store the buffer
 * @param added location to store the number of bytes in the buffer.
 *        If the function returns #GNUNET_SYSERR, its contents are undefined.
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on failure
 */
static int
webrtc_plugin_string_to_address (void *cls,
                                 const char *addr,
                                 uint16_t addrlen,
                                 void **buf, size_t *added)
{
  uint32_t options;
  uint32_t *buf2;

  if ((NULL == addr) || (0 == addrlen))
  {
    GNUNET_break (0);
    return GNUNET_SYSERR;
  }
  if ('\0' != addr[addrlen - 1])
  {
    GNUNET_break (0);
    return GNUNET_SYSERR;
  }
  if (strlen (addr) != addrlen - 1)
  {
    GNUNET_break (0);
    return GNUNET_SYSERR;
  }
  addr = strchr (addr, '.');
  if (NULL == addr)
  {
    GNUNET_break (0);
    return GNUNET_SYSERR;
  }
  options = atol (addr);
  buf2 = GNUNET_new (uint32_t);
  *buf2 = htonl (options);
  *buf = buf2;
  *added = sizeof(*buf2);
  return GNUNET_OK;
}


static void
offer_cb(void *cls,
         char *offer)
{
  struct GNUNET_ATS_Session *s = cls;
  struct GNUNET_MQ_Handle *mq = GNUNET_CADET_get_mq (s->channel);
  size_t offer_len = strlen (offer);
  struct GNUNET_MessageHeader *msg;
  struct GNUNET_MQ_Envelope *env = GNUNET_MQ_msg_extra (msg, offer_len, MESSAGE_TYPE_WEBRTC_OFFER);

  LOG (GNUNET_ERROR_TYPE_DEBUG,
      "Sending our offer `%s'\n",
      offer);
  memcpy (&msg[1], offer, offer_len);
  GNUNET_MQ_send (mq, env);
}


static void
message_cb(void *cls, uint8_t *data, int length)
{
  struct GNUNET_ATS_Session *s = cls;
  GNUNET_break (0);
}


/**
 * Check if payload is sane (size contains payload).
 *
 * @param cls should match #ch
 * @param message The actual message.
 * @return #GNUNET_OK to keep the channel open,
 *         #GNUNET_SYSERR to close it (signal serious error).
 */
static int
check_answer (void *cls,
              const struct GNUNET_MessageHeader *message)
{
  return GNUNET_OK;             /* all is well-formed */
}


extern void set_remote_answer(int, void *, int);


/**
 * Functions with this signature are called whenever a complete answer
 * is received.
 *
 * @param cls closure
 * @param srm the actual message
 */
static void
handle_answer (void *cls,
               const struct GNUNET_MessageHeader *message)
{
  struct GNUNET_ATS_Session *s = cls;
  uint16_t size = ntohs(message->size);
  set_remote_answer(s->rtc_peer_connection, &message[1], size - sizeof(*message));
  GNUNET_break (0);
}


/**
 * Function called whenever an MQ-channel's transmission window size changes.
 *
 * The first callback in an outgoing channel will be with a non-zero value
 * and will mean the channel is connected to the destination.
 *
 * For an incoming channel it will be called immediately after the
 * #GNUNET_CADET_ConnectEventHandler, also with a non-zero value.
 *
 * @param cls Channel closure.
 * @param channel Connection to the other end (henceforth invalid).
 * @param window_size New window size. If the is more messages than buffer size
 *                    this value will be negative..
 */
static void
out_window_change_cb (void *cls,
                      const struct GNUNET_CADET_Channel *channel,
                      int window_size)
{
  /* FIXME: could do flow control here... */
}


/**
 * Function called by cadet when a client disconnects.
 * Cleans up our `struct CadetClient` of that channel.
 *
 * @param cls  our `struct CadetClient`
 * @param channel channel of the disconnecting client
 * @param channel_ctx
 */
static void
out_disconnect_cb (void *cls,
                   const struct GNUNET_CADET_Channel *channel)
{
  struct GNUNET_ATS_Session *s = cls;

  GNUNET_break (0);
}

extern int peer_connect(void *, void *, void *, void *, int, void *);

/**
 * Create a new session to transmit data to the target
 * This session will used to send data to this peer and the plugin will
 * notify us by calling the env->session_end function
 *
 * @param cls closure
 * @param address pointer to the GNUNET_HELLO_Address
 * @return the session if the address is valid, NULL otherwise
 */
static struct GNUNET_ATS_Session *
webrtc_plugin_get_session (void *cls,
                           const struct GNUNET_HELLO_Address *address)
{
  struct Plugin *plugin = cls;
  struct GNUNET_ATS_Session *s;

  LOG (GNUNET_ERROR_TYPE_DEBUG,
      "Trying to get session for peer `%s'\n",
      GNUNET_i2s (&address->peer));
  /* find existing session */
  s = GNUNET_CONTAINER_multipeermap_get (plugin->sessions,
                                         &address->peer);
  if (NULL != s)
    return s;
  s = GNUNET_new (struct GNUNET_ATS_Session);
  s->plugin = plugin;
  s->peer = address->peer;
  /* add new session */
  (void) GNUNET_CONTAINER_multipeermap_put (plugin->sessions,
                                            &s->peer,
                                            s,
                                            GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST);
  struct GNUNET_MQ_MessageHandler handlers[] = {
    GNUNET_MQ_hd_var_size (answer,
                           MESSAGE_TYPE_WEBRTC_ANSWER,
                           struct GNUNET_MessageHeader,
                           plugin),
    GNUNET_MQ_handler_end ()
  };
  s->channel = GNUNET_CADET_channel_create (plugin->cadet,
                                            s,
                                            &address->peer,
                                            &plugin->port,
                                            GNUNET_CADET_OPTION_RELIABLE,
                                            out_window_change_cb,
                                            out_disconnect_cb,
                                            handlers);
  GNUNET_assert (s->channel != NULL);
  s->rtc_peer_connection = peer_connect(offer_cb, NULL, message_cb, NULL, 0, s);
  notify_session_monitor (plugin,
                          s,
                          GNUNET_TRANSPORT_SS_INIT);
  return NULL;
}


static void
webrtc_plugin_update_session_timeout (void *cls,
                                      const struct GNUNET_PeerIdentity *peer,
                                      struct GNUNET_ATS_Session *session)
{

}


#if 0
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
  struct Plugin *plugin = cls;
  struct GNUNET_ATS_Session *session = value;

  notify_session_monitor (plugin,
                          session,
                          GNUNET_TRANSPORT_SS_UP);
  return GNUNET_OK;
}
#endif


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
webrtc_plugin_setup_monitor (void *cls,
                             GNUNET_TRANSPORT_SessionInfoCallback sic,
                             void *sic_cls)
{
  struct Plugin *plugin = cls;

  plugin->sic = sic;
  plugin->sic_cls = sic_cls;
  if (NULL != sic)
  {
#if 0
    GNUNET_CONTAINER_multipeermap_iterate (NULL /* FIXME */,
                                           &send_session_info_iter,
                                           plugin);
#endif
    /* signal end of first iteration */
    sic (sic_cls, NULL, NULL);
  }
}


static void
answer_cb(void *cls,
          char *answer)
{
  struct GNUNET_ATS_Session *s = cls;
  struct GNUNET_MQ_Handle *mq = GNUNET_CADET_get_mq (s->channel);
  size_t answer_len = strlen (answer);
  struct GNUNET_MessageHeader *msg;
  struct GNUNET_MQ_Envelope *env = GNUNET_MQ_msg_extra (msg, answer_len, MESSAGE_TYPE_WEBRTC_ANSWER);

  LOG (GNUNET_ERROR_TYPE_DEBUG,
      "Sending our answer `%s'\n",
      answer);
  memcpy (&msg[1], answer, answer_len);
  GNUNET_MQ_send (mq, env);
}

/**
 * Check if payload is sane (size contains payload).
 *
 * @param cls should match #ch
 * @param message The actual message.
 * @return #GNUNET_OK to keep the channel open,
 *         #GNUNET_SYSERR to close it (signal serious error).
 */
static int
check_offer (void *cls,
             const struct GNUNET_MessageHeader *message)
{
  return GNUNET_OK;             /* all is well-formed */
}


/**
 * Functions with this signature are called whenever a complete offer
 * is received.
 *
 * @param cls closure
 * @param srm the actual message
 */
static void
handle_offer (void *cls,
              const struct GNUNET_MessageHeader *message)
{
  struct GNUNET_ATS_Session *s = cls; // XXX right?
  LOG (GNUNET_ERROR_TYPE_DEBUG,
      "handle_offer called with session %p\n",
      s);
  uint16_t size = ntohs(message->size);
  s->rtc_peer_connection = peer_connect(NULL, answer_cb, message_cb, &message[1], size - sizeof(*message), s);
  GNUNET_break (0);
}


/**
 * Functions of this type are called upon new cadet connection from other peers.
 *
 * @param cls the closure from GNUNET_CADET_connect
 * @param channel the channel representing the cadet
 * @param initiator the identity of the peer who wants to establish a cadet
 *            with us; NULL on binding error
 * @return initial channel context (our `struct GNUNET_ATS_Session`)
 */
static void *
connect_cb (void *cls,
            struct GNUNET_CADET_Channel *channel,
            const struct GNUNET_PeerIdentity *initiator)
{
  struct Plugin *plugin = cls;
  struct GNUNET_ATS_Session *s;

  LOG (GNUNET_ERROR_TYPE_DEBUG,
      "Got CADET connection from peer `%s'\n",
      GNUNET_i2s (initiator));
  /* find existing session */
  s = GNUNET_CONTAINER_multipeermap_get (plugin->sessions,
                                         initiator);
  if (NULL == s)
  {
    GNUNET_break (0);
    return s;
  }
  s = GNUNET_new (struct GNUNET_ATS_Session);
  s->plugin = plugin;
  s->peer = *initiator;
  s->channel = channel;
  /* add new session */
  (void) GNUNET_CONTAINER_multipeermap_put (plugin->sessions,
                                            &s->peer,
                                            s,
                                            GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST);
  LOG (GNUNET_ERROR_TYPE_DEBUG,
      "Returning session %p\n",
      s);
  return s;
}


/**
 * Function called by cadet when a client disconnects.
 * Cleans up our `struct CadetClient` of that channel.
 *
 * @param cls  our `struct CadetClient`
 * @param channel channel of the disconnecting client
 * @param channel_ctx
 */
static void
in_disconnect_cb (void *cls,
                  const struct GNUNET_CADET_Channel *channel)
{
  GNUNET_break (0);
}


/**
 * Function called whenever an MQ-channel's transmission window size changes.
 *
 * The first callback in an outgoing channel will be with a non-zero value
 * and will mean the channel is connected to the destination.
 *
 * For an incoming channel it will be called immediately after the
 * #GNUNET_CADET_ConnectEventHandler, also with a non-zero value.
 *
 * @param cls Channel closure.
 * @param channel Connection to the other end (henceforth invalid).
 * @param window_size New window size. If the is more messages than buffer size
 *                    this value will be negative..
 */
static void
in_window_change_cb (void *cls,
                     const struct GNUNET_CADET_Channel *channel,
                     int window_size)
{
  /* FIXME: could do flow control here... */
}


/**
 * Entry point for the plugin.
 */
void *
libgnunet_plugin_transport_webrtc_init (void *cls)
{
  struct GNUNET_TRANSPORT_PluginEnvironment *env = cls;
  struct GNUNET_TRANSPORT_PluginFunctions *api;
  struct Plugin *plugin;
  struct GNUNET_HELLO_Address *address;

  if (NULL == env->receive)
  {
    /* run in 'stub' mode (i.e. as part of gnunet-peerinfo), don't fully
       initialze the plugin or the API */
    api = GNUNET_new (struct GNUNET_TRANSPORT_PluginFunctions);
    api->cls = NULL;
    api->address_to_string = &webrtc_plugin_address_to_string;
    api->string_to_address = &webrtc_plugin_string_to_address;
    api->address_pretty_printer = &webrtc_plugin_address_pretty_printer;
    return api;
  }

  plugin = GNUNET_new (struct Plugin);
  plugin->env = env;
  plugin->sessions = GNUNET_CONTAINER_multipeermap_create (128,
                                                           GNUNET_YES);
  plugin->cadet = GNUNET_CADET_connect(env->cfg);
  GNUNET_assert (plugin->cadet != NULL);
  struct GNUNET_MQ_MessageHandler handlers[] = {
    GNUNET_MQ_hd_var_size (offer,
                           MESSAGE_TYPE_WEBRTC_OFFER,
                           struct GNUNET_MessageHeader,
                           plugin),
    GNUNET_MQ_handler_end ()
  };
  GNUNET_CRYPTO_hash ("webrtc", 6, &plugin->port);
  plugin->in_port = GNUNET_CADET_open_port (plugin->cadet,
                                            &plugin->port,
                                            &connect_cb,
                                            plugin,
                                            &in_window_change_cb,
                                            &in_disconnect_cb,
                                            handlers);
  GNUNET_assert (plugin->in_port != NULL);
  api = GNUNET_new (struct GNUNET_TRANSPORT_PluginFunctions);
  api->cls = plugin;
  api->send = &webrtc_plugin_send;
  api->disconnect_peer = &webrtc_plugin_disconnect_peer;
  api->disconnect_session = &webrtc_plugin_disconnect_session;
  api->query_keepalive_factor = &webrtc_plugin_query_keepalive_factor;
  api->address_pretty_printer = &webrtc_plugin_address_pretty_printer;
  api->check_address = &webrtc_plugin_address_suggested;
  api->address_to_string = &webrtc_plugin_address_to_string;
  api->string_to_address = &webrtc_plugin_string_to_address;
  api->get_session = &webrtc_plugin_get_session;
  api->get_network = &webrtc_plugin_get_network;
  api->get_network_for_address = &webrtc_plugin_get_network_for_address;
  api->update_session_timeout = &webrtc_plugin_update_session_timeout;
  api->setup_monitor = &webrtc_plugin_setup_monitor;

  uint32_t options = 0;
  address = GNUNET_HELLO_address_allocate (plugin->env->my_identity,
                                           PLUGIN_NAME,
                                           &options,
                                           sizeof (options),
                                           GNUNET_HELLO_ADDRESS_INFO_NONE);
  plugin->env->notify_address (plugin->env->cls,
      GNUNET_YES,
      address);
  GNUNET_HELLO_address_free (address);
  LOG (GNUNET_ERROR_TYPE_INFO, "WebRTC plugin successfully loaded\n");
  return api;
}


/**
 * Exit point from the plugin.
 */
void *
libgnunet_plugin_transport_webrtc_done (void *cls)
{
  struct GNUNET_TRANSPORT_PluginFunctions *api = cls;
  struct Plugin *plugin = api->cls;

  GNUNET_CADET_close_port (plugin->in_port);
  GNUNET_CADET_disconnect (plugin->cadet);
  GNUNET_free (plugin);
  GNUNET_free (api);
  return NULL;
}

// vim: set expandtab ts=2 sw=2:
