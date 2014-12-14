;; core.cljs - core service for gnunet-web website
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

(ns gnunet-web.core
  (:require [gnunet-web.service :as service] ;; leave this here
            [gnunet-web.util :refer [get-object read-memory register-object
                                     unregister-object]])
  (:require-macros [fence.core :refer [+++]]))

(def core-handle (js/_GNUNET_CORE_connect
                   1 ; const struct GNUNET_CONFIGURATION_Handle *cfg
                   0 ; void *cls
                   0 ; GNUNET_CORE_StartupCallback init
                   0 ; GNUNET_CORE_ConnectEventHandler connects
                   0 ; GNUNET_CORE_DisconnectEventHandler disconnects
                   0 ; GNUNET_CORE_MessageCallback inbound_notify
                   0 ; int inbound_hdr_only
                   0 ; GNUNET_CORE_MessageCallback outbound_notify
                   0 ; int outbound_hdr_only
                   0)) ; const struct GNUNET_CORE_MessageHandler *handlers

(defn monitor-callback
  [cls peer-id-pointer state timeout-pointer]
  (let [callback (get-object cls)]
    (callback {:peer (vec (read-memory peer-id-pointer 32))
               :state state})))

(def monitor-callback-pointer (+++ (.addFunction js/Runtime monitor-callback)))

(defn monitor-peers
  [callback]
  (let [callback-key (register-object callback)]
    (js/_GNUNET_CORE_monitor_start
      core-handle
      monitor-callback-pointer
      callback-key)))

(def state-strings
  {0 "Down"
   1 "Key Sent"
   2 "Key Received"
   3 "Connected"
   4 "Re-keying"
   5 "Disconnected"})

(defn state->string
  [state]
  (get state-strings state))
