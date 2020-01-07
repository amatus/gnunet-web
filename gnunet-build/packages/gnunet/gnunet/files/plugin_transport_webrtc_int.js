// plugin_transport_webrtc_int.js - js for webrtc plugin
// Copyright (C) 2018  David Barksdale <amatus@amat.us>
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
  $CONNECTIONS: [],
  $NEXT_CONNECTION: 1,
  peer_connect__deps: ["$CONNECTIONS", "$NEXT_CONNECTION"],
  peer_connect: function(offer_cb, answer_cb, message_cb, offer_ptr, offer_size, cls) {
    var offer;
    if (0 != offer_ptr) {
      offer = Pointer_stringify(offer_ptr, offer_size);
    }
    var channel = new MessageChannel();
    var port = channel.port1;
    port.onmessage = function(e) {
      if ('offer' == e.data.type && 0 != offer_cb) {
        ccallFunc(
          getFuncWrapper(offer_cb, 'vii'),
          'void',
          ['number', 'string'],
          [cls, e.data.sdp]);
      } else if ('answer' == e.data.type && 0 != answer_cb) {
        ccallFunc(
          getFuncWrapper(answer_cb, 'vii'),
          'void',
          ['number', 'string'],
          [cls, e.data.sdp]);
      } else if ('message' == e.data.type) {
        console.error('got webrtc message:', e.data);
      } else {
        console.error('unhandled message on webrtc message channel', e.data);
      }
    };
    do_to_window(function(w) {
      w.postMessage({
        type: 'peer_connect',
        offer: offer,
        message_port: channel.port2}, [channel.port2]);
    });
    CONNECTIONS[NEXT_CONNECTION] = port;
    return NEXT_CONNECTION++;
  },
  set_remote_answer__deps: ["$CONNECTIONS"],
  set_remote_answer: function(num, answer_ptr, answer_size) {
    var port = CONNECTIONS[num];
    port.postMessage({type: 'answer', sdp: Pointer_stringify(answer_ptr, answer_size)});
  },
  peer_disconnect__deps: ["$CONNECTIONS"],
  peer_disconnect: function(num) {
    var port = CONNECTIONS[num];
    port.postMessage({type: 'disconnect'});
    port.close();
    delete CONNECTIONS[num];
  }
});

// vim: set expandtab ts=2 sw=2: