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
            [gnunet-web.hello :refer [encode-hello]]
            [gnunet-web.service :as service] ;; leave this here
            [gnunet-web.util :refer [get-object read-memory register-object
                                     unregister-object]])
  (:require-macros [cljs.core.async.macros :refer [go go-loop]]
                   [fence.core :refer [+++]]))

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

(defn get-hello-callback
  [cls hello-pointer]
  (let [closure (get-object cls)
        peer-id-pointer (js/_malloc 32)]
    (js/_GNUNET_HELLO_get_id hello-pointer peer-id-pointer)
    ((:callback closure) (vec (read-memory peer-id-pointer 32)))
    (js/_free peer-id-pointer)
    (js/_GNUNET_TRANSPORT_get_hello_cancel @(:handle closure))
    (unregister-object cls)))

(def get-hello-callback-pointer
  (+++ (.addFunction js/Runtime get-hello-callback)))

(defn get-my-peer-id
  [callback]
  (let [closure {:callback callback
                 :handle (atom nil)}
        closure-key (register-object closure)]
    (reset! (:handle closure)
            (js/_GNUNET_TRANSPORT_get_hello
              transport-handle
              get-hello-callback-pointer
              closure-key))))

(defn address->string-callback
  [cls string-pointer res]
  (let [ch (get-object cls)]
    (if (zero? string-pointer)
      (do
        (unregister-object cls)
        (close! ch))
      (when (= 1 res)
        (go (>! ch (js/Pointer_stringify string-pointer)))))))

(def address->string-callback-pointer
  (+++ (.addFunction js/Runtime address->string-callback)))

(defn address->string
  [address-pointer]
  (let [ch (chan 1)
        ch-key (register-object ch)]
    (js/_GNUNET_TRANSPORT_address_to_string_simple
      address-pointer
      address->string-callback-pointer
      ch-key)
    ch))

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

(def monitor-callback-pointer (+++ (.addFunction js/Runtime monitor-callback)))

(defn monitor-peers
  [callback]
  (let [callback-key (register-object callback)]
    (js/_GNUNET_TRANSPORT_monitor_peers_simple monitor-callback-pointer
                                               callback-key)))
