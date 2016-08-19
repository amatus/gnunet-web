;; transport.cljs - transport service for gnunet-web website
;; Copyright (C) 2014,2015  David Barksdale <amatus@amatus.name>
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
  (:require [client-lib]
            [cljs.core.async :refer [chan close!]]
            [gnunet-web.hello :refer [encode-hello]]
            [gnunet-web.service :as service] ;; leave this here
            [gnunet-web.util :refer [get-object read-memory register-object
                                     unregister-object]]
            [goog.crypt :as gcrypt])
  (:require-macros [cljs.core.async.macros :refer [go go-loop]]
                   [fence.core :refer [+++]]))

(def state-strings
  {0 "Not connected"
   1 "Selecting transport"
   2 "Waiting for SYN_ACK"
   3 "Selecting address"
   4 "Waiting for ACK"
   5 "Connected"
   6 "Error, reconnecting"
   7 "Reconnecting"
   8 "Switching address"
   9 "Disconnecting"
   10 "Disconnect finished"})

(def state-connected 5)
(def state-disconnected 10)

(defn state->string
  [state]
  (get state-strings state))

(defn offer-hello
  [hello]
  (js/ccallFunc
    js/_GNUNET_TRANSPORT_offer_hello
    "number"
    (array "number" "array" "number" "number")
    (array 0
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
    (js/_GNUNET_TRANSPORT_hello_get_cancel @(:handle closure))
    (unregister-object cls)))

(def get-hello-callback-pointer
  (+++ (.addFunction js/Runtime get-hello-callback)))

(defn get-my-peer-id
  [callback]
  (let [closure {:callback callback
                 :handle (atom nil)}
        closure-key (register-object closure)]
    (reset! (:handle closure)
            (js/_GNUNET_TRANSPORT_hello_get
              0
              0
              get-hello-callback-pointer
              closure-key))))

(defn monitor-callback
  [cls peer-pointer state transport-pointer address-pointer address-length]
  (let [callback (get-object cls)
        peer (vec (read-memory peer-pointer 32))]
    (callback
      {:state state
       :peer peer
       :address (when-not (zero? transport-pointer)
                  (let [transport (js/Pointer_stringify transport-pointer)
                        address (read-memory address-pointer address-length)]
                    (when (= "http_client" transport)
                      (gcrypt/utf8ByteArrayToString
                        (to-array (drop 8 address))))))})))

(def monitor-callback-pointer (+++ (.addFunction js/Runtime monitor-callback)))

(defn monitor-peers
  [callback]
  (let [callback-key (register-object callback)]
    (js/_GNUNET_TRANSPORT_monitor_peers_simple monitor-callback-pointer
                                               callback-key)))
