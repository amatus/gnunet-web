;; ats.cljs - ATS service for gnunet-web website
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

(ns gnunet-web.ats
  (:use [gnunet-web.message :only (parse-message-types)]
        [gnunet-web.parser :only (parser parse-uint32)]
        [gnunet-web.service :only (add-service)])
  (:require-macros [monads.macros :as monadic]))

(def message-type-ats-start 340)
(def start-flag-scheduling 0)
(def start-flag-performance-with-pic 1)
(def start-flag-performance-no-pic 2)
(def parse-client-start
  (with-meta
    (monadic/do parser
                [start-flag parse-uint32]
                {:start-flag start-flag})
    {:message-type message-type-ats-start}))

;; scheduling client
(def my-client (atom nil))

(def ats-message-channel (js/MessageChannel.))
(def clients (atom #{}))

(defn client-get-message
  [output event]
  (let [message (first (.-v ((parse-message-types #{parse-client-start})
                               (.-data event))))]
    (output (str "ats-msg:" (js/JSON.stringify (clj->js message))))
    (condp = (:message-type message)
      message-type-ats-start
      (condp = (:start-flag (:message message))
        start-flag-scheduling
        (swap! my-client #(if (nil? %) (.-target event) %))))))

(defn start-ats
  [output]
  (set! (.-onmessage (.-port1 ats-message-channel))
        (fn [event]
          ;; This must be a connect message
          (let [port (.-port (.-data event))]
            (swap! clients conj port)
            (set! (.-onmessage port) (partial client-get-message output)))))
  (add-service "ats" (.-port2 ats-message-channel)))

