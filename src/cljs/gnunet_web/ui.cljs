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
  (:require [gnunet-web.hostlist :as hostlist]))

(defn by-id
  [id]
  (.getElementById js/document (name id)))

(defn output
  [string]
  (let [output (by-id :output)]
    (set! (.-textContent output)
          (str (.-textContent output) "\n" string))))

(def transport (js/SharedWorker. "js/gnunet-service-transport.js"))

(set! (.-onerror transport)
      (fn [event]
        (output (str "transport:"
                     (.-filename event) ":" (.-lineno event) " "
                     (.-message event)))))

(set! (.-onmessage (.-port transport))
      (fn [event]
        (let [data (.-data event)]
          (if (= "stdout" (.-type data))
            (set! (.-onmessage (.-port data))
                  (fn [event] (output (str "transport:" (.-data event)))))
            (output (str "transport:" (js/JSON.stringify (.-message data))))))))

(.start (.-port transport))
(.postMessage (.-port transport) (clj->js {:type "stdout"}))

(.addEventListener
  (by-id :send)
  "click"
  (fn [event]
    (let [message (js/Object.)]
      (set! (.-type message) "message")
      (set! (.-array message) (js/JSON.parse (.-value (by-id :message))))
      (.postMessage (.-port transport) message))))

(.addEventListener
  (by-id :hostlist)
  "click"
  (fn [event]
    (hostlist/fetch-and-process!)))
