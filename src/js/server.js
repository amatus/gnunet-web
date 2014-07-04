// server.js - server routines for gnunet-web services
// Copyright (C) 2013,2014  David Barksdale <amatus@amatus.name>
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
  $SERVER: {
    handlers: {},
    connect_notify_list: [],
    disconnect_notify_list: [],
    clients: {},
    next_client: 1,
    connect: function(port) {
      port.onmessage = SERVER.client_get_message;
      port._name = SERVER.next_client++;
      SERVER.clients[port._name] = {
        port: port,
        ref_count: 0,
        shtudown_now: false,
      };
      SERVER.connect_notify_list.map(function(notify) {
        ccallFunc(Runtime.getFuncWrapper(notify.callback, 'vii'), 'void',
          ['number', 'number'],
          [notify.callback_cls, port._name]);
      });
    },
    client_get_message: function(ev) {
      var size = ev.data[0] << 8 | ev.data[1];
      var type = ev.data[2] << 8 | ev.data[3];
      var handler = SERVER.handlers[type];
      //Module.print("Got message of type " + type + " size " + size + " from "
      //    + ev.target._name);
      if (typeof handler === 'undefined') {
        //Module.print("But I don't know what to do with it");
      } else {
        if (handler.expected_size == 0 || handler.expected_size == size) {
          ccallFunc(Runtime.getFuncWrapper(handler.callback, 'viii'), 'void',
              ['number', 'number', 'array'],
              [handler.callback_cls, ev.target._name, ev.data]);
        } else {
          //Module.print("But I was expecting size " + handler.expected_size);
        }
      }
    }
  },
  GNUNET_SERVER_add_handlers__deps: ['$SERVER'],
  GNUNET_SERVER_add_handlers: function(server, handlers) {
    for (var i = 0; ; i += 12 /* sizeof GNUNET_SERVER_MessageHandler */) {
      var callback = getValue(handlers + i, 'i32');
      var callback_cls = getValue(handlers + i + 4, 'i32');
      var type = getValue(handlers + i + 8, 'i16');
      var expected_size = getValue(handlers + i + 10, 'i16');
      if (callback === 0)
        break;
      //Module.print('Adding handler for message type: ' + type);
      SERVER.handlers[type] = {
        'callback': callback,
        'callback_cls': callback_cls,
        'expected_size': expected_size
      };
    }
  },
  GNUNET_SERVER_receive_done__deps: [
    '$SERVER',
    'GNUNET_SERVER_client_disconnect',
  ],
  GNUNET_SERVER_receive_done: function(client, success) {
    var c = SERVER.clients[client];
    if (!success) {
      if (c.ref_count > 0)
        c.shutdown_now = true;
      else
        _GNUNET_SERVER_client_disconnect(client);
      return;
    }
    if (c.shutdown_now)
      _GNUNET_SERVER_client_disconnect(client);
  },
  GNUNET_SERVER_client_keep__deps: ['$SERVER'],
  GNUNET_SERVER_client_keep: function(client) {
    SERVER.clients[client].ref_count++;
  },
  GNUNET_SERVER_client_drop__deps: [
    '$SERVER',
    'GNUNET_SERVER_client_disconnect',
  ],
  GNUNET_SERVER_client_drop: function(client) {
    var c = SERVER.clients[client];
    c.ref_count--;
    if (c.ref_count == 0 && c.shutdown_now)
      _GNUNET_SERVER_client_disconnect(client);
  },
  GNUNET_SERVER_client_disconnect: function(client) {
    // TODO
  },
  GNUNET_SERVER_notify_transmit_ready: function(client, size, timeout, callback,
                                           callback_cls) {
    setTimeout(function() {
      //Module.print('I want to send ' + size + ' bytes to client ' + client);
      var stack = Runtime.stackSave();
      var buffer = Runtime.stackAlloc(size);
      var ret = Runtime.dynCall('iiii', callback, [callback_cls, size, buffer]);
      var view = {{{ makeHEAPView('U8', 'buffer', 'buffer+ret') }}};
      // See http://code.google.com/p/chromium/issues/detail?id=169705
      SERVER.clients[client].port.postMessage(new Uint8Array(view));
      Runtime.stackRestore(stack);
    }, 0);
    return 1; // opaque GNUNET_SERVER_TransmitHandle*
  },
  GNUNET_SERVER_connect_notify__deps: ['$SERVER'],
  GNUNET_SERVER_connect_notify: function(server, callback, callback_cls) {
    SERVER.connect_notify_list.push({
      callback: callback,
      callback_cls: callback_cls
    });
  },
  GNUNET_SERVER_disconnect_notify__deps: ['$SERVER'],
  GNUNET_SERVER_disconnect_notify: function(server, callback, callback_cls) {
    SERVER.disconnect_notify_list.push({
      callback: callback,
      callback_cls: callback_cls
    });
  },
  GNUNET_SERVER_suspend: function(server) {
    // TODO: We either need to fail client connections when the server is
    // suspended or we could allow connections but queue messages and
    // connection notifications for delivery when the server is resumed.
  },
  GNUNET_SERVER_resume: function(server) {
    // TODO: see GNUNET_SERVER_suspend
  },
  GNUNET_SERVER_client_mark_monitor: function(client) {
    // Mark the client as a 'monitor' so we don't wait for it to disconnect
    // when we are shutting down.
    // Since we don't handle shutdown this is a noop for now.
  },
});

// vim: set expandtab ts=2 sw=2:
