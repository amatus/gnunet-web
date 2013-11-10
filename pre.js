// vim: set expandtab ts=2 sw=2:
var WorkerMessageQueue = [];
flush_worker_message_queue = function(f) {
  WorkerMessageQueue.forEach(f);
  WorkerMessageQueue = [];
}
gnunet_prerun = function() {
  ENV.GNUNET_PREFIX = "/.";
  if (ENVIRONMENT_IS_WORKER) {
    Module["print"] = function(x) {
      WorkerMessageQueue.push(x);
    };
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
    var size = getValue(message, 'i16');
    var type = getValue(message + 2, 'i16');
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
