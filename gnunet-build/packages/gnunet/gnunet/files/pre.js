// pre.js - linked into each gnunet-web service
// Copyright (C) 2013-2016  David Barksdale <amatus@amat.us>
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

var xhrs = []; // plugin_transport_http_client

var dev_urandom_bytes = 0;
var random_bytes = [];
var random_offset = 0;
gnunet_prerun = function() {
  ENV.GNUNET_PREFIX = "/.";
  [
    'ats_proportional',
    'block_dht',
    'block_fs',
    'datacache_heap',
    'datastore_emscripten',
    'peerstore_emscripten',
    'transport_http_client',
  ].forEach(function(plugin) {
    FS.createPreloadedFile('/', 'libgnunet_plugin_' + plugin + '.js',
        'libgnunet_plugin_' + plugin + '.js', true, false);
  });

  // Create /dev/urandom that provides strong random bytes from the parent
  // window since workers don't have crypto
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
  FS.unlink('/dev/urandom');
  FS.mkdev('/dev/urandom', id);

  //  Mount IDBFS for services that use it
  var match = location.pathname.match('gnunet-service-(.*).js');
  if (match) {
    var service = match[1];
    var mounts = {peerinfo: true, fs: true, nse: true, datastore: true};
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
if (typeof(Module['preInit']) === "undefined") Module = { 'preInit': [] };
Module['preInit'].push(gnunet_prerun);
Module['arguments'] = ["-L", "DEBUG"];

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
  try {
    if ('init' == ev.data.type) {
      FS.writeFile('/private_key', ev.data['private-key'], {encoding: 'binary'});
      random_bytes = ev.data['random-bytes'];
      random_offset = 0;
      removeRunDependency('window-init');
    } else if ('connect' == ev.data.type) {
      console.debug("got connect: ", ev.data);
      SOCKETS.incoming.push(ev.data);
      if ("listening" in SOCKETS) {
        var socket = SOCKETS[SOCKETS.listening];
        if ("task" in socket) {
          delete SCHEDULER_TASKS[socket.task];
          delete socket["task"];
          console.debug("calling handler for listening socket");
          Runtime.dynCall('vi', socket["handler"], [socket["cls"]]);
        }
      }
    }
  } catch (e) {
    console.error('Rekt', e);
  }
}

// Ask a window to connect us to a service
function client_connect(service_name, message_port) {
  console.debug('I want to connect to', service_name);
  do_to_window(function(w) {
    console.debug('I am now connecting to', service_name);
    try {
      w.postMessage({
        type: 'client_connect',
        service_name: service_name,
        client_name: location.pathname,
        message_port: message_port}, [message_port]);
    } catch (e) {
      console.error('Failed to connect to', service_name);
    }
  });
}

function breakpoint() {
  var x = 1; // Something to break on
}

// emscripten removed this insanely useful function, wtf?
function ccallFunc(func, returnType, argTypes, args) {
  var toC = {'string' : function(str) {
      var ret = 0;
      if (str !== null && str !== undefined && str !== 0) { // null string
        // at most 4 bytes per UTF-8 code point, +1 for the trailing '\0'
        var len = (str.length << 2) + 1;
        ret = stackAlloc(len);
        stringToUTF8(str, ret, len);
      }
      return ret;
    }, 'array' : function(arr) {
      var ret = stackAlloc(arr.length);
      writeArrayToMemory(arr, ret);
      return ret;
    }};
  var cArgs = [];
  var stack = 0;
  if (args) {
    for (var i = 0; i < args.length; i++) {
      var converter = toC[argTypes[i]];
      if (converter) {
        if (stack === 0) stack = stackSave();
        cArgs[i] = converter(args[i]);
      } else {
        cArgs[i] = args[i];
      }
    }
  }
  var ret = func.apply(null, cArgs);
  if (returnType === 'string') ret = Pointer_stringify(ret);
  if (stack !== 0) {
    stackRestore(stack);
  }
  return ret;
}

// vim: set expandtab ts=2 sw=2:
