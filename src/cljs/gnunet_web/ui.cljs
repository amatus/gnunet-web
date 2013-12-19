;; ui.cljs - UI routines for gnunet-web website
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

(ns gnunet-web.ui
  (:use [gnunet-web.service :only (client-connect)]
        [gnunet-web.hostlist :only (fetch-and-process!)]))

(defn by-id
  [id]
  (.getElementById js/document (name id)))

(defn output
  [string]
  (let [output (by-id :output)]
    (set! (.-textContent output)
          (str (.-textContent output) "\n" string))))

(def transport-message-channel (js/MessageChannel.))
(def transport-port (.-port1 transport-message-channel))
(set! (.-onmessage transport-port)
      (fn [event]
        (output (str "transport-msg:" (js/JSON.stringify (.-data event))))))
(client-connect "transport" (.-port2 transport-message-channel) output)

(.addEventListener
  (by-id :send)
  "click"
  (fn [event]
    (let [message (js/Object.)]
      (set! (.-type message) "message")
      (set! (.-array message) (js/JSON.parse (.-value (by-id :message))))
      (.postMessage transport-port message))))

(.addEventListener
  (by-id :hostlist)
  "click"
  (fn [event]
    (fetch-and-process!)))
