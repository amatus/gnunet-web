// pre.js - linked into each gnunet-web service
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

var WorkerMessageQueue = [];
flush_worker_message_queue = function(f) {
  WorkerMessageQueue.forEach(f);
  WorkerMessageQueue = [];
}
gnunet_prerun = function() {
  ENV.GNUNET_PREFIX = "/.";
  if (ENVIRONMENT_IS_WORKER) {
    Module['print'] = function(x) {
      WorkerMessageQueue.push(x);
    };
    Module['printErr'] = Module['print'];
  }
}
if (typeof(Module) === "undefined") Module = { preRun: [] };
Module.preRun.push(gnunet_prerun);

var clients = {};
var next_client = 1;
onconnect = function(event) {
  debug_port = event.ports[0];
  event.ports[0].onmessage = get_message;
  event.ports[0]._name = next_client;
  clients[next_client] = event.ports[0];
  next_client++;
};

function get_message(event) {
  if ('stdout' == event.data.type) {
    var channel = new MessageChannel();
    Module['print'] = function(x) { channel.port1.postMessage(x); };
    flush_worker_message_queue(Module.print);
    event.target.postMessage({type:'stdout', port:channel.port2},
                             [channel.port2]);
  } else if ('message' == event.data.type) {
    var stack = Runtime.stackSave();
    var message = allocate(event.data.array, 'i8', ALLOC_STACK);
    var size = getValue(message, 'i8') << 8 | getValue(message + 1, 'i8');
    var type = getValue(message + 2, 'i8') << 8 | getValue(message + 3, 'i8');
    var handler = SERVERS.handlers[type];
    Module.print("Got message of type " + type + " size " + size + " from "
        + event.target._name);
    if (typeof handler === 'undefined') {
      Module.print("But I don't know what to do with it");
    } else {
      if (handler.expected_size == 0 || handler.expected_size == size) {
        Runtime.dynCall('viii', handler.callback,
            [handler.callback_cls, event.target._name, message]);
      } else {
        Module.print("But I was expecting size " + handler.expected_size);
      }
    }
    Runtime.stackRestore(stack);
  }
}
// vim: set expandtab ts=2 sw=2:
