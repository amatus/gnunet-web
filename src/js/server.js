// server.js - server routines for gnunet-web services
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
  $SERVERS: { handlers: {} },
  GNUNET_SERVER_add_handlers__deps: ['$SERVERS'],
  GNUNET_SERVER_add_handlers: function(server, handlers) {
    for (var i = 0; ; i += 12 /* sizeof GNUNET_SERVER_MessageHandler */) {
      var callback = getValue(handlers + i, 'i32');
      var callback_cls = getValue(handlers + i + 4, 'i32');
      var type = getValue(handlers + i + 8, 'i16');
      var expected_size = getValue(handlers + i + 10, 'i16');
      if (callback === 0)
        break;
      Module.print('Adding handler for message type: ' + type);
      SERVERS.handlers[type] = {
        'callback': callback,
        'callback_cls': callback_cls,
        'expected_size': expected_size
      };
    }
  },
  GNUNET_SERVER_disconnect_notify: function(server, callback, callback_cls) {
  },
  GNUNET_SERVER_receive_done: function(client, success) {
  },
  GNUNET_SERVER_client_keep: function(client) {
  },
  GNUNET_SERVER_client_drop: function(client) {
  },
  GNUNET_SERVER_notify_transmit_ready: function(client, size, timeout, callback,
                                           callback_cls) {
    setTimeout(function() {
      Module.print('I want to send ' + size + ' bytes to client ' + client);
      var stack = Runtime.stackSave();
      var buffer = allocate(size, 'i8', ALLOC_STACK);
      var ret = Runtime.dynCall('iiii', callback, [callback_cls, size, buffer]);
      var view = {{{ makeHEAPView('U8', 'buffer', 'buffer+ret') }}};
      clients[client].postMessage({type: 'message', message: view});
      Runtime.stackRestore(stack);
    }, 0);
    return 1; // opaque GNUNET_SERVER_TransmitHandle*
  },
});

// vim: set expandtab ts=2 sw=2:
