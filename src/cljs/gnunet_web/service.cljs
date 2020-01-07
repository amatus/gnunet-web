;; service.cljs - service manager for gnunet-web website
;; Copyright (C) 2013-2018  David Barksdale <amatus@amat.us>
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

(ns gnunet-web.service
  (:require [cognitect.transit :as t]
            [gnunet-web.webrtc :refer [peer-connect]]))

(def private-key
  ;; XXX This has no synchronization!
  (let [d (.getItem js/localStorage "private-key")]
    (if (nil? d)
      (let [d (js/Uint8Array. 32)
            _ (js/window.crypto.getRandomValues d)
            d (vec (.apply js/Array (array) d))]
        (.setItem js/localStorage "private-key" (t/write (t/writer :json) d))
        d)
      (t/read (t/reader :json) d))))

(def services (atom {}))

(defn add-service
  [service-name port]
  (swap! services assoc service-name port))

(def client-connect)

(defn start-worker
  [worker-name uri]
  (let [worker (js/SharedWorker. uri)
        port (.-port worker)
        random-bytes (js/Uint8Array. 4080)
        _ (js/window.crypto.getRandomValues random-bytes)]
    (set! (.-onerror worker)
          (fn [event]
            (.error js/console
                    worker-name
                    (.-filename event)
                    (.-lineno event)
                    (.-message event))))
    (set! (.-onmessage port)
          (try
            (fn [event]
              (let [data (.-data event)]
                (condp = (.-type data)
                  "init" (do
                           (set! (.-onmessage (aget data "stdout"))
                                 (fn [event]
                                   (.debug js/console
                                           worker-name
                                           (.-data event))))
                           (set! (.-onmessage (aget data "stderr"))
                                 (fn [event]
                                   (.debug js/console
                                           worker-name
                                           (.-data event)))))
                  "client_connect" (client-connect (aget data "service_name")
                                                   (aget data "client_name")
                                                   (aget data "message_port"))
                  "peer_connect" (peer-connect (aget data "message_port") (aget data "offer"))
                  (.warn js/console worker-name data))))
          (catch :default e
            (js/console.error "REKT" e))))
    (.start port)
    (.postMessage port (js-obj "type" "init"
                               "private-key" (to-array private-key)
                               "random-bytes" random-bytes))
    worker))

(defn ^:export client-connect
  [service-name client-name message-port]
  (js/console.debug "client" client-name "wants to connect to" service-name)
  (let [service (get @services service-name)]
    (if (nil? service)
      (let [worker (start-worker service-name
                                 (str "js/gnunet-service-" service-name ".js"))
            port (.-port worker)]
        (add-service service-name port)
        (recur service-name client-name message-port))
      (.postMessage service (js-obj "type" "connect"
                                    "client-name" client-name
                                    "port" message-port)
                    (array message-port)))))
