;; webrtc.cljs - WebRTC functions for gnunet-web
;; Copyright (C) 2018  David Barksdale <amatus@amat.us>
;;
;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

(ns gnunet-web.webrtc)

(def rtc-config
  (clj->js
    {:ice-servers [{:url "stun:stun.l.google.com:19302"}]}))

(def data-channel-config
  (clj->js
    {:ordered false
     :maxRetransmits 0
     :negotiated true
     :id 1}))

(defn peer-connect
  [message-port offer]
  (.debug js/console "peer-connect called with offer:" offer)
  (let [connection (js/RTCPeerConnection. rtc-config)
        channel (.createDataChannel connection
                 "data"
                 data-channel-config)]
    (set! (.-onopen channel)
      nil)
    (set! (.-onmessage channel)
      nil)
    (if (empty? offer)
      (.then (.createOffer connection)
        (fn [offer]
          (.debug js/console "created offer:" offer)
          (.setLocalDescription connection offer)
          (.postMessage message-port
            (js-obj "type" "offer"
                    "sdp" (.-sdp offer)))))
      (.then (.setRemoteDescription connection
                          (js-obj "type" "offer"
                            "sdp" offer))
        (fn []
          (.then (.createAnswer connection)
            (fn [answer]
              (.debug js/console "created answer:" answer)
              (.setLocalDescription connection answer)
              (.postMessage message-port
                (js-obj "type" "answer"
                  "sdp" (.-sdp answer))))))))
    (set! (.-onmessage message-port)
      (fn [e]
        (.debug js/console "got webrtc message-channel message" e)
        (let [data (.-data e)]
          (condp = (.-type data)
            "answer" (.then (.setRemoteDescription connection
                              (js-obj "type" "answer"
                                "sdp" (.-sdp data)))
                        (fn []))))))))
