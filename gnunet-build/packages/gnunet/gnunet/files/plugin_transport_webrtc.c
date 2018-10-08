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
#include "gnunet_util_lib.h"
#include "gnunet_protocols.h"
#include "gnunet_statistics_service.h"
#include "gnunet_transport_service.h"
#include "gnunet_transport_plugin.h"

#define LOG(kind,...) GNUNET_log_from (kind, PLUGIN_NAME,__VA_ARGS__)

/**
 * After how long do we expire an address that we
 * learned from another peer if it is not reconfirmed
 * by anyone?
 */
#define LEARNED_ADDRESS_EXPIRATION GNUNET_TIME_relative_multiply (GNUNET_TIME_UNIT_HOURS, 1)

#define PLUGIN_NAME "webrtc"

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
   * To whom are we talking to (set to our identity
   * if we are still waiting for the welcome message)
   */
  struct GNUNET_PeerIdentity sender;

  /**
   * Stored in a linked list (or a peer map, or ...)
   */
  struct GNUNET_ATS_Session *next;

  /**
   * Pointer to the global plugin struct.
   */
  struct Plugin *plugin;

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
   * List of open sessions (or peer map, or...)
   */
  struct GNUNET_ATS_Session *sessions;

  /**
   * Function to call about session status changes.
   */
  GNUNET_TRANSPORT_SessionInfoCallback sic;

  /**
   * Closure for @e sic.
   */
  void *sic_cls;
};


#if 0
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
#endif


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
  // struct Plugin *plugin = cls;
  // FIXME
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
  // struct Plugin *plugin = cls;
  // FIXME
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
  return 3;
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
  GNUNET_assert (NULL != session);
  return GNUNET_ATS_NET_UNSPECIFIED; /* Change to correct network type */
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
  return GNUNET_ATS_NET_WAN; /* FOR NOW */
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
  asc (asc_cls, "converted address", GNUNET_OK); /* return address */
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
  /* struct Plugin *plugin = cls; */

  /* check if the address is belonging to the plugin*/
  return GNUNET_OK;
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
  /*
   * Print address in format webrtc.options.address
   */

  if (0 == addrlen)
  {
    return TRANSPORT_SESSION_INBOUND_STRING;
  }

  GNUNET_break (0);
  return NULL;
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
  /*
   * Parse string in format webrtc.options.address
   */
  GNUNET_break (0);
  return GNUNET_SYSERR;
}


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
  GNUNET_break (0);
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


/**
 * Entry point for the plugin.
 */
void *
libgnunet_plugin_transport_webrtc_init (void *cls)
{
  struct GNUNET_TRANSPORT_PluginEnvironment *env = cls;
  struct GNUNET_TRANSPORT_PluginFunctions *api;
  struct Plugin *plugin;

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
  LOG (GNUNET_ERROR_TYPE_INFO, "Template plugin successfully loaded\n");
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

  GNUNET_free (plugin);
  GNUNET_free (api);
  return NULL;
}

// vim: set expandtab ts=2 sw=2:
