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
        [gnunet-web.message :only (encode-message)]
        [gnunet-web.service :only (client-connect)]))

(def message-type-monitor-peer-request 380)

(defn encode-monitor-peer-request-message
  [{:keys [one-shot peer] :or {one-shot false peer (repeat 32 0)}}]
  (encode-message
    {:message-type message-type-monitor-peer-request
     :message
     (concat
       (encode-uint32 one-shot)
       peer)}))

(defn monitor-peers
  [callback]
  (let [message-channel (js/MessageChannel.)]
    (set! (.-onmessage (.-port1 message-channel))
          (fn [event]
            (.log js/console "monitor-peers" event)))
    (client-connect "transport" "web app" (.-port2 message-channel))
    (.postMessage (.-port1 message-channel)
                  (into-array (encode-monitor-peer-request-message {})))))
