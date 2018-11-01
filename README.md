gnunet-web
==========

This is a port of the [GNUnet] secure peer-to-peer network to the browser using
HTTP for browser-to-native communication and soon [WebRTC] for
browser-to-browser communication.

Roadmap
-------
* Compile GNUnet using [emscripten].
    * gnunet-service-peerinfo.js - Done.
    * gnunet-service-transport.js - Done, with HTTP transport.
    * gnunet-service-ats.js - Done.
    * gnunet-daemon-topology.js - Done.
    * gnunet-service-core.js - Done.
    * gnunet-service-nse.js - Done.
    * gnunet-service-dht.js - Done.
    * gnunet-service-cadet.js - Done.
    * gnunet-service-datastore.js - Done.
    * gnunet-service-peerstore.js - Done.
    * gnunet-service-fs.js - Done.
* Write a minimal UI for the file-sharing service.
    * Search - Done.
    * Publish.
        * The user can only select one file at a time.
        * No directory support.
    * Download.
        * No URI support.
        * No directory support.
* Release alpha.
* Write a WebRTC transport plugin.
    * This is not currently possible, the Web Worker running the transport
      service cannot access WebRTC, see [bug 4700].

What You Can Do Now
-------------------

You will need to install [boot] to follow these instructions.

### Try gnunet-web pre-alpha ###
0. Execute `./build-gnunet.sh`
1. Execute `boot dev`
2. Open http://localhost:8000/

Each GNUnet service is running in its own [Web Worker] thread. The APIs used by
the services to schedule tasks, communicate with each other, and load plugins
are implemented as emscripten js libraries.

To debug a shared worker in chrome open chrome://inspect and click the
"inspect" link next to an entry in the shared workers list.

### Try out the RTCPeerConnection demo ###
0. Execute `boot dev`
1. Open two browsers to http://localhost:8000/webrtc.html (let's call them Alice and Bob).
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

  [gnunet]: https://gnunet.org
  [webrtc]: http://www.webrtc.org
  [emscripten]: https://github.com/kripken/emscripten
  [web worker]: http://www.w3.org/TR/workers/
  [indexeddb]: http://www.w3.org/TR/IndexedDB/
  [boot]: https://github.com/boot-clj/boot#install
  [bug 4700]: https://bugs.chromium.org/p/webrtc/issues/detail?id=4700

