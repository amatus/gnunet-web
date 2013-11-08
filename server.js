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
  }
}
// vim: set expandtab ts=2 sw=2:
