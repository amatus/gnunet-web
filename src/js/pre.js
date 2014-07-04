// pre.js - linked into each gnunet-web service
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

var xhrs = [];

var WorkerMessageQueue = [];
function flush_worker_message_queue(f) {
  WorkerMessageQueue.forEach(f);
  WorkerMessageQueue = [];
}
gnunet_prerun = function() {
  ENV.GNUNET_PREFIX = "/.";
  Module['print'] = function(x) { WorkerMessageQueue.push(x); };
  Module['printErr'] = function(x) { Module.print(x); };
  [
    'ats_proportional',
    'block_dht',
    'block_fs',
    'datacache_heap',
    'datastore_heap',
    'transport_http_client',
  ].forEach(function(plugin) {
    FS.createPreloadedFile('/', 'libgnunet_plugin_' + plugin + '.js',
        'libgnunet_plugin_' + plugin + '.js', true, false);
  });
  var id = FS.makedev(1, 8);
  FS.registerDevice(id, {
    read: function(stream, buffer, offset, length, pos) {
      //var buf = new Uint8Array(length);
      //window.crypto.getRandomValues(buf);
      for (var i = 0; i < length; i++) {
        buffer[offset+i] = Math.floor(Math.random() * 256);
      }
      return length;
    },
  });
  FS.mkdev('/dev/random', id);
  FS.mkdev('/dev/urandom', id);
//  addRunDependency("randomness")
//  <gather randomness>
//  removeRunDependency("randomness")

  //  Mount IDBFS for services that use it
  var match;
  if (match = location.pathname.match('gnunet-service-(.*).js')) {
    var service = match[1];
    var mounts = {peerinfo: true, fs: true, nse: true};
    if (mounts[service]) {
      var mount = '/' + service;
      FS.mkdir(mount);
      FS.mount(IDBFS, {}, mount);
      addRunDependency('syncfs');
      FS.syncfs(true, function() { removeRunDependency('syncfs'); });
      var sync_callback = function() {
        setTimeout(function() { FS.syncfs(false, sync_callback); }, 5000);
      };
      sync_callback();
    }
  }
}
if (typeof(Module) === "undefined") Module = { preRun: [] };
Module.preRun.push(gnunet_prerun);

var init_continuation;

// Called by GNUNET_SERVICE_run and GNUNET_PROGRAM_run to perform
// asynchronous setup tasks. Calls continuation when finished.
worker_setup = function(continuation) {
  if (init_continuation === 'now')
    continuation();
  else
    init_continuation = continuation;
};

// a map of window index to port
var windows = {};
// next available index
var next_window = 1;
onconnect = function(ev) {
  ev.ports[0].onmessage = get_message;
  ev.ports[0]._name = next_window;
  windows[next_window] = ev.ports[0];
  next_window++;
};

// do to any window
function do_to_window(fn) {
  for (var w in windows) {
    fn(windows[w]);
    return;
  }
}

function get_message(ev) {
  if ('init' == ev.data.type) {
    var channel = new MessageChannel();
    Module['print'] = function(x) { channel.port1.postMessage(x); };
    flush_worker_message_queue(Module.print);
    ev.target.postMessage({type: 'stdout', port: channel.port2},
                          [channel.port2]);
    FS.writeFile('/private_key', ev.data['private-key'], {encoding: 'binary'});
    if (typeof (init_continuation) === 'undefined')
      init_continuation = 'now';
    else
      init_continuation();
  } else if ('connect' == ev.data.type) {
    SERVER.connect(ev.data.port);
  }
}

// Ask a window to connect us to a service
function client_connect(service_name, message_port) {
  do_to_window(function(w) {
    w.postMessage({'type': 'client_connect',
      'service_name': service_name,
      'message_port': message_port}, [message_port]);
  });
}

function breakpoint() {
  var x = 1; // Something to break on
}

// vim: set expandtab ts=2 sw=2:
