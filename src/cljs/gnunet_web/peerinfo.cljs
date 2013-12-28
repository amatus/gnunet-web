;; peerinfo.cljs - peerinfo service for gnunet-web website
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

(ns gnunet-web.peerinfo
  (:use [gnunet-web.service :only (add-service)]))

(def peerinfo-message-channel (js/MessageChannel.))
(def clients (atom #{}))

(defn client-get-message
  [output event]
  (output (str "peerinfo-msg:" (js/JSON.stringify (.-data event)))))

(defn start-peerinfo
  [output]
  (set! (.-onmessage (.-port1 peerinfo-message-channel))
        (fn [event]
          ;; This must be a connect message
          (let [port (.-port (.-data event))]
            (swap! clients conj port)
            (set! (.-onmessage port) (partial client-get-message output)))))
  (add-service "peerinfo" (.-port2 peerinfo-message-channel)))

