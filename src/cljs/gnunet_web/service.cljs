;; service.cljs - service manager for gnunet-web website
;; Copyright (C) 2013,2014  David Barksdale <amatus@amatus.name>
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
  (:require [tailrecursion.cljson :refer (clj->cljson cljson->clj)]))

(def private-key
  ;; XXX This has no synchronization!
  (let [d (.getItem js/localStorage "private-key")]
    (if (nil? d)
      (let [d (js/Uint8Array. 32)
            _ (js/window.crypto.getRandomValues d)
            d (vec (.apply js/Array (array) d))]
        (.setItem js/localStorage "private-key" (clj->cljson d))
        d)
      (cljson->clj d))))

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
                (.warn js/console worker-name data)))))
    (.start port)
    (.postMessage port (clj->js {:type "init"
                                 :private-key private-key
                                 :random-bytes random-bytes}))
    worker))

(defn client-connect
  [service-name client-name message-port]
  (let [service (get @services service-name)]
    (if (nil? service)
      (let [worker (start-worker service-name
                                 (str "js/gnunet-service-" service-name ".js"))
            port (.-port worker)]
        (add-service service-name port)
        (recur service-name client-name message-port))
      (.postMessage service (clj->js {:type "connect"
                                      :client-name client-name
                                      :port message-port})
                    (array message-port)))))
