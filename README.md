gnunet-web
==========

This is a port of [GNUnet] to the browser using [WebRTC] for peer-to-peer
communication.

Roadmap
-------
* Compile GNUnet using [emscripten].
    * gnunet-service-peerinfo.js - Done.
    * gnunet-service-transport.js - Done, with HTTP(S) transport.
    * gnunet-service-ats.js - Done.
    * gnunet-daemon-topology.js - Done.
    * gnunet-service-core.js - Done.
    * gnunet-service-nse.js - Done.
    * gnunet-service-dht.js - Done.
    * gnunet-service-cadet.js - Done.
    * gnunet-service-datastore.js - Building with heap backend plugin.
        * Needs an [indexedDB] backend plugin.
    * gnunet-service-fs.js - Done.
* Write a minimal UI that allows publishing, searching, and downloading via the
  file-sharing service.
* Release alpha.
* Write a WebRTC transport plugin.
    * Implement [RFC3264] over GNUnet.

What You Can Do Now
-------------------

You will need to install [boot] to follow these instructions.

### Try out the RTCPeerConnection demo ###
0. Execute `boot production`
1. Open two browsers to resources/public/webrtc.html (let's call them Alice and Bob).
2. Alice presses "Create Offer" and waits a bit for ICE candidates to be
   collected.
3. Alice sends the Local Description to Bob.
4. Bob enters the description into the Remote Description box and presses
   "Set Remote Description as Offer".
5. Bob presses "Create Answer" and waits a bit for ICE candidates to be
   collected.
6. Bob sends the Local Description to Alice.
7. Alice enters the description into the Remote Description box and presses
   "Set Remote Description as Answer".
8. Alice and Bob wait for the ICE State to be connected.
9. Alice and Bob can send messages with the input box at the bottom of the page.

### Compile GNUnet with emscripten ###
0. Execute `./build-gnunet.sh`
1. Execute `boot production`
2. Open resources/public/index.html

Each GNUnet service is running in its own [Web Worker] thread. The APIs used by
the services to schedule tasks, communicate with each other, and load plugins
are implemented as emscripten js libraries.

To debug a shared worker in chrome open chrome://inspect and click the
"inspect" link next to an entry in the shared workers list.

  [gnunet]: https://gnunet.org
  [webrtc]: http://www.webrtc.org
  [emscripten]: https://github.com/kripken/emscripten
  [rfc3264]: http://www.ietf.org/rfc/rfc3264.txt
  [web worker]: http://www.w3.org/TR/workers/
  [indexeddb]: http://www.w3.org/TR/IndexedDB/
  [boot]: https://github.com/tailrecursion/boot

