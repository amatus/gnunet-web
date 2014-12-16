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

var xhrs = []; // plugin_transport_http_client

var WorkerMessageQueue = [];
function flush_worker_message_queue(f) {
  WorkerMessageQueue.forEach(f);
  WorkerMessageQueue = [];
}
var dev_urandom_bytes = 0;
var random_bytes = [];
var random_offset = 0;
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
    'peerstore_emscripten',
    'transport_http_client',
  ].forEach(function(plugin) {
    FS.createPreloadedFile('/', 'libgnunet_plugin_' + plugin + '.js',
        'libgnunet_plugin_' + plugin + '.js', true, false);
  });

  // Create /dev/random but it doesn't need to do anything
  var id = FS.makedev(1, 8);
  FS.registerDevice(id, {});
  FS.mkdev('/dev/random', id);

  // Create /dev/urandom
  var id = FS.makedev(1, 9);
  FS.registerDevice(id, {
    read: function(stream, buffer, offset, length, pos) {
      dev_urandom_bytes += length;
      for (var i = 0; i < length && random_offset < random_bytes.length; i++) {
        buffer[offset+i] = random_bytes[random_offset];
        random_bytes[random_offset++] = 0;
      }
      if (i < length)
        Module.printErr('Random bytes exausted!');
      return i;
    },
  });
  FS.mkdev('/dev/urandom', id);

  //  Mount IDBFS for services that use it
  var match = location.pathname.match('gnunet-service-(.*).js');
  if (match) {
    var service = match[1];
    var mounts = {peerinfo: true, fs: true, nse: true};
    if (mounts[service]) {
      var mount = '/' + service;
      FS.mkdir(mount);
      FS.mount(IDBFS, {}, mount);
      addRunDependency('syncfs');
      FS.syncfs(true, function() { removeRunDependency('syncfs'); });
      var syncfs_task;
      var sync_callback = function() {
        clearTimeout(syncfs_task);
        syncfs_task = setTimeout(function() {
          FS.syncfs(false, sync_callback);
        }, 5000);
      };
      sync_callback();
    }
  }
  addRunDependency('window-init');
}
if (typeof(Module) === "undefined") Module = { preRun: [] };
Module.preRun.push(gnunet_prerun);

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
    var stdout = new MessageChannel();
    var stderr = new MessageChannel();
    Module['print'] = function(x) { stdout.port1.postMessage(x); };
    Module['printErr'] = function(x) { stderr.port1.postMessage(x); };
    flush_worker_message_queue(Module.print);
    ev.target.postMessage(
      {type: 'init', stdout: stdout.port2, stderr: stderr.port2},
      [stdout.port2, stderr.port2]);
    FS.writeFile('/private_key', ev.data['private-key'], {encoding: 'binary'});
    random_bytes = ev.data['random-bytes'];
    random_offset = 0;
    removeRunDependency('window-init');
  } else if ('connect' == ev.data.type) {
    SERVER.connect(ev.data.port, ev.data['client-name']);
  }
}

// Ask a window to connect us to a service
function client_connect(service_name, message_port) {
  //Module.print('I want to connect to ' + service_name);
  do_to_window(function(w) {
    //Module.print('I am now connecting to ' + service_name);
    w.postMessage({
      type: 'client_connect',
      service_name: service_name,
      client_name: location.pathname,
      message_port: message_port}, [message_port]);
  });
}

function breakpoint() {
  var x = 1; // Something to break on
}

// vim: set expandtab ts=2 sw=2:
