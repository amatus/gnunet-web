// network.js - network routines for gnunet-web services
// Copyright (C) 2016  David Barksdale <amatus@amat.us>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

mergeInto(LibraryManager.library, {
  $SOCKETS: {incoming: []},
  $NEXT_SOCKET: 1,
  GNUNET_NETWORK_socket_create__deps: ['$SOCKETS', '$NEXT_SOCKET'],
  GNUNET_NETWORK_socket_create: function(domain, type, protocol) {
    console.debug("socket_create(", domain, type, protocol, ")");
    if (domain != 1 || type != 1 || protocol != 0) {
      return 0;
    }
    return NEXT_SOCKET++;
  },
  GNUNET_NETWORK_socket_connect__deps: ['$SOCKETS'],
  GNUNET_NETWORK_socket_connect: function(desc, address, address_len) {
    console.debug("socket_connect(", desc, address, address_len, ")");
    if (desc in SOCKETS) {
      console.error("socket already connected?");
      return -1;
    }
    if (110 != address_len) {
      return -1;
    }
    var path = Pointer_stringify(address + 2);
    console.debug("connecting to", path);
    var channel;
    try {
      channel = new MessageChannel();
    } catch (e) {
      console.error("No MessageChannel in this browser", e);
      return -1;
    }
    var socket = SOCKETS[desc] = {
      port: channel.port1,
      name: path,
      queue: [],
    };
    channel.port1.onmessage = function(ev) {
      console.debug("got message on socket", desc, ev);
      socket.queue.push(ev.data);
      if ("task" in socket) {
        console.debug("calling read handler");
        delete SCHEDULER_TASKS[socket.task];
        delete socket["task"];
        Runtime.dynCall('vi', socket.handler, [socket.cls]);
      }
    };
    if (typeof client_connect == 'function') {
      client_connect(path, channel.port2);
    } else {
      try {
        gnunet_web.service.client_connect(path, 'client.js', channel.port2);
      } catch(e) {
        console.error('Failed to connect to', path, e);
        return -1;
      }
    }
    return 1;
  },
  GNUNET_NETWORK_socket_send__deps: ['$SOCKETS'],
  GNUNET_NETWORK_socket_send: function(desc, buffer, length) {
    console.debug("socket_send(", desc, buffer, length, ")");
    if (!(desc in SOCKETS)) {
      console.error("socket not connected?");
      return -1;
    }
    var view = {{{ makeHEAPView('U8', 'buffer', 'buffer+length') }}};
    try {
      SOCKETS[desc].port.postMessage(new Uint8Array(view));
    } catch (e) {
      console.error("Failed to send");
      return -1;
    }
    return length;
  },
  GNUNET_NETWORK_socket_close__deps: ['$SOCKETS'],
  GNUNET_NETWORK_socket_close: function(desc) {
    console.debug("socket_close(", desc, ")");
    if (!(desc in SOCKETS)) {
      return 1;
    }
    var socket = SOCKETS[desc];
    delete SOCKETS[desc];
    if ("port" in socket) {
      socket.port.close();
    }
    return 1;
  },
  GNUNET_NETWORK_socket_bind__deps: ['$SOCKETS'],
  GNUNET_NETWORK_socket_bind: function(desc, address, address_len) {
    console.debug("socket_bind(", desc, address, address_len, ")");
    if (desc in SOCKETS) {
      console.error("socket already bound?");
      return -1;
    }
    if (110 != address_len) {
      return -1;
    }
    var path = Pointer_stringify(address + 2);
    console.debug("binding to", path);
    SOCKETS[desc] = {};
    return 1;
  },
  GNUNET_NETWORK_socket_listen__deps: ['$SOCKETS'],
  GNUNET_NETWORK_socket_listen: function(desc, backlog) {
    console.debug("socket_listen(", desc, backlog, ")");
    if ("listening" in SOCKETS) {
      console.error("only one listening socket is supported");
      return -1;
    }
    SOCKETS.listening = desc;
    return 1;
  },
  GNUNET_NETWORK_socket_accept__deps: ['$SOCKETS', '$NEXT_SOCKET'],
  GNUNET_NETWORK_socket_accept: function(desc, address, address_len) {
    console.debug("socket_accept(", desc, address, address_len, ")");
    if (desc != SOCKETS.listening) {
      console.error("socket is not listening");
      return 0;
    }
    if (0 == SOCKETS.incoming.length) {
      console.debug("no incoming connections");
      return 0;
    }
    var data = SOCKETS.incoming.shift();
    var sd = NEXT_SOCKET++;
    var socket = SOCKETS[sd] = {
      port: data.port,
      name: data['client-name'],
      queue: [],
    };
    data.port.onmessage = function(ev) {
      console.debug("got message on socket", sd, ev);
      socket.queue.push(ev.data);
      if ("task" in socket) {
        console.debug("calling read handler");
        delete SCHEDULER_TASKS[socket.task];
        delete socket["task"];
        Runtime.dynCall('vi', socket.handler, [socket.cls]);
      }
    };
    {{{ makeSetValue('address', '0', '1', 'i16') }}};
    {{{ makeSetValue('address_len', '0', '110', 'i32') }}};
    return sd;
  },
  GNUNET_NETWORK_socket_recv__deps: ['$SOCKETS'],
  GNUNET_NETWORK_socket_recv: function(desc, buffer, length) {
    console.debug("socket_recv(", desc, buffer, length, ")");
    if (!(desc in SOCKETS)) {
      console.error("socket not connected?");
      return -1;
    }
    var socket = SOCKETS[desc];
    if (0 == socket.queue.length) {
      console.debug("nothing to read");
      return 0;
    }
    var data = socket.queue[0];
    var sub = data.subarray(0, length);
    HEAP8.set(sub, buffer);
    if (sub.length == data.length) {
      socket.queue.shift();
    } else {
      socket.queue[0] = data.subarray(sub.length);
    }
    console.debug("read", sub.length, "bytes");
    return sub.length;
  },
  GNUNET_NETWORK_fdset_isset__deps: ['$SOCKETS'],
  GNUNET_NETWORK_fdset_isset: function(fds, desc) {
    if (fds == 2) {
      // always ready to write
      return 1;
    }
    if (desc == SOCKETS.listening) {
      return SOCKETS.incoming.length;
    }
    if (desc in SOCKETS) {
      return SOCKETS[desc].queue.length;
    }
    return 0;
  },
});

// vim: set expandtab ts=2 sw=2:
