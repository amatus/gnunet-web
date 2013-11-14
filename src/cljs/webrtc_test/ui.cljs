;; ui.cljs - UI routines for webrtc-test website
;; Copyright (C) 2013  David Barksdale <amatus@amatus.name>
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

(ns webrtc-test.ui
  (:require [webrtc-test.webrtc :as webrtc]))

(defn by-id
  [id]
  (.getElementById js/document id))

(defn show-local-description
  [pc]
  (set! (.-value (by-id "local-description"))
        (.-sdp (.-localDescription pc))))

(defn format-sdp
  [sdp] sdp)
  ;(.join (.split sdp #"\n") "\r\n"))

(defn as-offer
  [sdp]
  (js/RTCSessionDescription. (clj->js {:sdp (format-sdp sdp) :type :offer})))

(defn as-answer
  [sdp]
  (js/RTCSessionDescription. (clj->js {:sdp (format-sdp sdp) :type :answer})))

(defn data-channel-events
  [data-channel]
  (set! (.-onmessage data-channel)
        (fn [message]
          (js/alert (.-data message)))))

(def peer-connection (webrtc/new-peer-connection))
(def data-channel nil)
(set! (.-onicecandidate peer-connection)
      (fn [event] (show-local-description peer-connection)))
(set! (.-ondatachannel peer-connection)
      (fn [event]
        (set! data-channel (.-channel event))
        (data-channel-events data-channel)))
(set! (.-onicechange peer-connection)
      (fn [event]
        (set! (.-innerHTML (by-id "ice-state"))
              (.-iceConnectionState peer-connection))))

(.addEventListener
  (by-id "create-offer")
  "click"
  (fn [event]
    (let [dc (.createDataChannel peer-connection "gnunet"
                                 (clj->js {:reliable false}))]
      (data-channel-events dc)
      (set! data-channel dc)
      (.createOffer peer-connection
                    (fn [description]
                      (.setLocalDescription peer-connection description))))))

(.addEventListener
  (by-id "create-answer")
  "click"
  (fn [event]
    (.createAnswer peer-connection
                   (fn [description]
                     (.setLocalDescription peer-connection description)))))

(.addEventListener
  (by-id "set-local-offer")
  "click"
  (fn [event]
    (.setLocalDescription peer-connection
                          (as-offer (.-value (by-id "local-description"))))))

(.addEventListener
  (by-id "set-local-answer")
  "click"
  (fn [event]
    (.setLocalDescription peer-connection
                          (as-answer (.-value (by-id "local-description"))))))

(.addEventListener
  (by-id "set-remote-offer")
  "click"
  (fn [event]
    (.setRemoteDescription peer-connection
                           (as-offer (.-value (by-id "remote-description"))))))

(.addEventListener
  (by-id "set-remote-answer")
  "click"
  (fn [event]
    (.setRemoteDescription peer-connection
                          (as-answer (.-value (by-id "remote-description"))))))

(.addEventListener
  (by-id "send")
  "click"
  (fn [event]
    (.send data-channel (.-value (by-id "message")))))

