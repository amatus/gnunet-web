// client.js - client routines for gnunet-web services
// Copyright (C) 2013  David Barksdale <amatus@amatus.name>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

mergeInto(LibraryManager.library, {
  $CLIENT_PORTS: {},
  $NEXT_PORT: 1,
  GNUNET_CLIENT_connect__deps: ['$CLIENT_PORTS', '$NEXT_PORT'],
  GNUNET_CLIENT_connect: function(service_name, cfg) {
    var service_name = Pointer_stringify(service_name);
    var channel = new MessageChannel();
    var port = NEXT_PORT;
    NEXT_PORT = port + 1;
    CLIENT_PORTS[port] = channel.port1;
    client_connect(service_name, channel.port2);
    return port;
  },
  GNUNET_CLIENT_receive__deps: ['$CLIENT_PORTS'],
  GNUNET_CLIENT_receive: function(client, handler, handler_cls, timeout) {
    CLIENT_PORTS[client].onmessage = function(ev) {
      var stack = Runtime.stackSave();
      var message = allocate(ev.data, 'i8', ALLOC_STACK);
      Runtime.dynCall('vii', handler, [handler_cls, message]);
      Runtime.stackRestore(stack);
    };
    var delay = getValue(timeout, 'i64');
    if (-1 != delay) {
      setTimeout(function() {
        CLIENT_PORTS[client].onmessage = null;
        Runtime.dynCall('vii', handler, [handler_cls, 0]);
      }, delay / 1000);
    }
  },
  GNUNET_CLIENT_notify_transmit_ready__deps: ['$CLIENT_PORTS'],
  GNUNET_CLIENT_notify_transmit_ready: function(client, size, timeout,
                                           auto_retry, notify, notify_cls) {
    // Supposedly we can call notify right now, but the current code never
    // does so let's emulate that.
    setTimeout(function() {
      Module.print('I want to send ' + size + ' bytes to service ' + client);
      var stack = Runtime.stackSave();
      var buffer = allocate(size, 'i8', ALLOC_STACK);
      var ret = Runtime.dynCall('iiii', notify, [notify_cls, size, buffer]);
      var view = {{{ makeHEAPView('U8', 'buffer', 'buffer+ret') }}};
      CLIENT_PORTS[client].postMessage(view);
      Runtime.stackRestore(stack);
    }, 0);
    return 1; // opaque GNUNET_CLIENT_TransmitHandle*
  },
});

// vim: set expandtab ts=2 sw=2:
