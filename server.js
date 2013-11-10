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
  },
});

// vim: set expandtab ts=2 sw=2:
