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
  $RTC_CONFIG: {iceServers: [{url: "stun:stun.l.google.com:19302"}]},
  $CONNECTIONS: [],
  $NEXT_CONNECTION: 1,
  create_connection__deps: ["$RTC_CONFIG", "$CONNECTIONS", "$NEXT_CONNECTION"],
  create_connection: function(offer_cb, cls) {
    var conn = new RTCPeerConnection(RTC_CONFIG);
    chan = conn.createDataChannel("data", {ordered: false,
                                           maxRetransmits: 0,
                                           negotiated: true,
                                           id: 1});
    chan.onopen = function(e) {
      console.warn("channel open");
    };
    chan.onmessage = function(e) {
      console.warn("channel got message:", e);
    };
    offer = conn.createOffer();
    offer.then(function(e) {
      console.warn("created offer:", e);
      ccallFunc(
        getFuncWrapper(offer_cb, 'vii'),
        'void',
        ['number', 'string'],
        [cls, e.sdp]);
    });
    CONNECTIONS[NEXT_CONNECTION] = {conn: conn, chan: chan};
    return NEXT_CONNECTION++;
  }
});

// vim: set expandtab ts=2 sw=2: