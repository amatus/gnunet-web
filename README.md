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

### Build libgpg-error, libgcrypt, and libunistring with emscripten ###
0. Install [emscripten] and add it to your `PATH`.
1. Execute `./build-gnunet.sh`

  [gnunet]: https://gnunet.org
  [webrtc]: http://www.webrtc.org
  [emscripten]: https://github.com/kripken/emscripten
  [rfc3264]: http://www.ietf.org/rfc/rfc3264.txt
