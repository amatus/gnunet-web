;; transport.cljs - transport service for gnunet-web website
;; Copyright (C) 2014  David Barksdale <amatus@amatus.name>
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

(ns gnunet-web.transport
  (:use [gnunet-web.encoder :only (encode-uint32)]
        [gnunet-web.hello :only (parse-hello)]
        [gnunet-web.message :only (encode-message parse-message-types
                                   parse-peer-identity)]
        [gnunet-web.parser :only (items optional parser parse-absolute-time
                                  parse-uint32)]
        [gnunet-web.service :only (client-connect)])
  (:require [goog.crypt])
  (:require-macros [monads.macros :as monadic]))

(def message-type-start 360)
(def message-type-monitor-peer-request 380)
(def message-type-peer-iterate-reply 383)

(defn encode-start-message
  [{:keys [options peer] :or {options 0 peer (repeat 32 0)}}]
  (encode-message
    {:message-type message-type-start
     :message
     (concat
       (encode-uint32 options)
       peer)}))

(defn encode-monitor-peer-request-message
  [{:keys [one-shot peer] :or {one-shot false peer (repeat 32 0)}}]
  (encode-message
    {:message-type message-type-monitor-peer-request
     :message
     (concat
       (encode-uint32 one-shot)
       peer)}))

(def parse-peer-iterate-reply
  (with-meta
    (optional
      (monadic/do parser
                  [reserved parse-uint32
                   peer parse-peer-identity
                   state-timeout parse-absolute-time
                   local-address-info parse-uint32
                   state parse-uint32
                   address-length parse-uint32
                   plugin-length parse-uint32
                   address (items address-length)
                   plugin (items plugin-length)]
                  {:peer peer
                   :state-timeout state-timeout
                   :local-address-info local-address-info
                   :state state
                   :address (vec (.apply js/Array (array) address))
                   :plugin (goog.crypt/utf8ByteArrayToString
                             (.apply js/Array (array) plugin))}))
    {:message-type message-type-peer-iterate-reply}))

(defn monitor
  [callback]
  (let [message-channel (js/MessageChannel.)]
    (set! (.-onmessage (.-port1 message-channel))
          (fn [event]
            (let [message (.-v ((parse-message-types #{parse-hello})
                                  (.-data event)))]
              (if (not (= :nothing message))
                       (callback (:message (first message)))))))
    (client-connect "transport" "web app (monitor)"
                    (.-port2 message-channel))
    (.postMessage (.-port1 message-channel)
                  (into-array (encode-start-message {})))))

(defn monitor-peers
  [callback]
  (let [message-channel (js/MessageChannel.)]
    (set! (.-onmessage (.-port1 message-channel))
          (fn [event]
            (let [message (first (.-v ((parse-message-types
                                         #{parse-peer-iterate-reply})
                                         (.-data event))))]
              (callback (:message message)))))
    (client-connect "transport" "web app (monitor-peers)"
                    (.-port2 message-channel))
    (.postMessage (.-port1 message-channel)
                  (into-array (encode-monitor-peer-request-message {})))))

(def state-strings
  {0 "Not connected"
   1 "Waiting for ATS"
   2 "Waiting for CONNECT_ACK"
   3 "Waiting for ATS suggestion"
   4 "Waiting for SESSION_ACK"
   5 "Connected"
   6 "ATS Reconnect"
   7 "Reconnecting"
   8 "Switching address"
   9 "Disconnecting"
   10 "Disconnect finished"})

(defn state->string
  [state]
  (get state-strings state))

(defn addr->string
  [address]
  (goog.crypt/utf8ByteArrayToString (to-array (drop 8 address))))
