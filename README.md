gnunet-web
==========

This is a port of [GNUnet] to the browser using [WebRTC] for peer-to-peer
communication.

Roadmap
-------
* Compile GNUnet using [emscripten].
* Implement [RFC3264] over GNUnet.
* Implement WebRTC transport.
* Write a kick-ass browser UI in clojurescript.

What You Can Do Now
-------------------

### Try out the RTCPeerConnection demo ###
0. Execute `lein run`
1. Open two browsers to http://localhost:3000/ (let's call them Alice and Bob).
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
0. Install [emscripten] and add it to your `PATH`.
1. Execute `./build-gnunet.sh`
2. Execute `lein run`
3. Open http://localhost:3000/gnunet.html

We're a long way from running GNUnet in the browser. Currently we have the
peerinfo service linked into a javascript file which will load but has no
way of receiving commands.

Eventually we will have each GNUnet service running in its own [Web Worker]
thread. The APIs used by the services to schedule tasks and communicate with
each other is being implemented in javascript.

  [gnunet]: https://gnunet.org
  [webrtc]: http://www.webrtc.org
  [emscripten]: https://github.com/kripken/emscripten
  [rfc3264]: http://www.ietf.org/rfc/rfc3264.txt
  [web worker]: http://www.w3.org/TR/workers/
