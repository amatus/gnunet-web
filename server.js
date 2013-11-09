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
  }
});

var clients = {};
var next_client = 1;
onconnect = function(event) {
  debug_port = event.ports[0];
  event.ports[0].onmessage = get_message;
  event.ports[0]._name = next_client;
  clients[next_client] = event.port;
  next_client++;
};

function get_message(event) {
  if ('stdout' == event.data.type) {
    var channel = new MessageChannel();
    Module['print'] = function(x) { channel.port1.postMessage(x); };
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
