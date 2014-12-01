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
  (:require [cljs.core.async :refer [chan close!]]
            [gnunet-web.encoder :refer [encode-uint32]]
            [gnunet-web.hello :refer [encode-hello parse-hello]]
            [gnunet-web.message :refer [encode-message parse-message-types]]
            [gnunet-web.service :refer [client-connect]]
            [gnunet-web.util :refer [get-object read-memory register-object
                                     unregister-object]])
  (:require-macros [cljs.core.async.macros :refer [go go-loop]]))

(def message-type-start 360)

(defn encode-start-message
  [{:keys [options peer] :or {options 0 peer (repeat 32 0)}}]
  (encode-message
    {:message-type message-type-start
     :message
     (concat
       (encode-uint32 options)
       peer)}))

(defn monitor
  [callback]
  (let [message-channel (js/MessageChannel.)]
    (set! (.-onmessage (.-port1 message-channel))
          (fn [event]
            (let [message @((parse-message-types #{parse-hello})
                              (.-data event))]
              (if (coll? message)
                       (callback (:message (first message)))))))
    (client-connect "transport" "web app (monitor)"
                    (.-port2 message-channel))
    (.postMessage (.-port1 message-channel)
                  (into-array (encode-start-message {})))))

(def state-strings
  {0 "Not connected"
   1 "Waiting for ATS"
   2 "Waiting for SYN_ACK"
   3 "Waiting for ATS suggestion"
   4 "Waiting for ACK"
   5 "Connected"
   6 "ATS Reconnect"
   7 "Reconnecting"
   8 "Switching address"
   9 "Disconnecting"
   10 "Disconnect finished"})

(defn state->string
  [state]
  (get state-strings state))

(def transport-handle (js/_GNUNET_TRANSPORT_connect
                        0 ; const struct GNUNET_CONFIGURATION_Handle *cfg
                        0 ; const struct GNUNET_PeerIdentity *self
                        0 ; void *cls
                        0 ; GNUNET_TRANSPORT_ReceiveCallback rec
                        0 ; GNUNET_TRANSPORT_NotifyConnect nc
                        0)) ; GNUNET_TRANSPORT_NotifyDisconnect nd

(defn offer-hello
  [hello]
  (js/ccallFunc
    js/_GNUNET_TRANSPORT_offer_hello
    "number"
    (array "number" "array" "number" "number")
    (array transport-handle
           (into-array (encode-hello hello))
           0
           0)))

(defn address->string-callback
  [cls string-pointer res]
  (let [ch (get-object cls)]
    (if (zero? string-pointer)
      (close! ch)
      (go (>! ch (js/Pointer_stringify string-pointer))))))

(def address->string-callback-pointer
  (js/Runtime.addFunction address->string-callback))

(defn address->string
  [address-pointer]
  (let [ch (chan 1)
        ret-ch (chan 1)
        ch-key (register-object ch)
        context (js/_GNUNET_TRANSPORT_address_to_string_simple
                  address-pointer
                  address->string-callback-pointer
                  ch-key)]
    (go-loop []
             (let [result (<! ch)]
               (if (nil? result)
                 (do
                   (unregister-object ch-key)
                   (js/_GNUNET_TRANSPORT_address_to_string_cancel context)
                   (close! ret-ch))
                 (do
                   (>! ret-ch result)
                   (recur)))))
    ret-ch))

(defn monitor-callback
  [cls peer-pointer address-pointer state state-timeout]
  (when-not (zero? peer-pointer)
    (let [callback (get-object cls)
          peer (vec (read-memory peer-pointer 32))]
      (if (zero? address-pointer)
        (callback {:state state
                   :peer peer})
        (let [ch (address->string address-pointer)]
          (go (callback {:state state
                         :peer peer
                         :address (<! ch)})
              (close! ch)))))))

(def monitor-callback-pointer (js/Runtime.addFunction monitor-callback))

(defn monitor-peers
  [callback]
  (let [callback-key (register-object callback)]
    (js/_GNUNET_TRANSPORT_monitor_peers_simple monitor-callback-pointer
                                               callback-key)))
