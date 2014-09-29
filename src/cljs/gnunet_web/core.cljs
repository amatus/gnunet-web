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
  (:require [gnunet-web.hello :refer (parse-hello)]
            [gnunet-web.message :refer (encode-message parse-message-types
                                        parse-peer-identity)]
            [gnunet-web.parser :refer (parser parse-absolute-time parse-uint32)]
            [gnunet-web.service :refer (client-connect)])
  (:require-macros [monads.macros :as monadic]))

(def message-type-monitor-peers 78)
(def message-type-monitor-notify 79)

(def monitor-peers-message
  (encode-message
    {:message-type message-type-monitor-peers
     :message []}))

(def parse-monitor-notify-message
  (with-meta
    (monadic/do parser
                [state parse-uint32
                 peer parse-peer-identity
                 timeout parse-absolute-time]
                {:state state
                 :peer peer
                 :timeout timeout})
    {:message-type message-type-monitor-notify}))

(defn monitor-peers
  [callback]
  (let [message-channel (js/MessageChannel.)]
    (set! (.-onmessage (.-port1 message-channel))
          (fn [event]
            (let [message @((parse-message-types
                              #{parse-monitor-notify-message})
                              (.-data event))]
              (if (coll? message)
                (callback (:message (first message)))))))
    (client-connect "core" "web app (monitor-peers)"
                    (.-port2 message-channel))
    (.postMessage (.-port1 message-channel)
                  (into-array monitor-peers-message))))

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
