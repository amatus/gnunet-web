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
  (:use [tailrecursion.cljson :only (clj->cljson cljson->clj)]))

(def private-key
  ;; XXX This has no synchronization!
  (let [d (.getItem js/localStorage "private-key")]
    (if (nil? d)
      (let [d (repeatedly 32 #(int (* 0x100 (js/Math.random))))]
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
        port (.-port worker)]
    (set! (.-onerror worker)
          (fn [event]
            (println worker-name ":" (.-filename event) ":" (.-lineno event)
                     " " (.-message event))))
    (set! (.-onmessage port)
          (fn [event]
            (let [data (.-data event)]
              (condp = (.-type data)
                "stdout" (set! (.-onmessage (.-port data))
                               (fn [event]
                                 (println worker-name ":" (.-data event))))
                "client_connect" (client-connect (.-service_name data)
                                                 (.-message_port data))
                (println worker-name ":" (js/JSON.stringify data))))))
    (.start port)
    (.postMessage port (clj->js {:type "init"
                                 :private-key private-key}))
    worker))

(defn client-connect
  [service-name message-port]
  (let [service (get @services service-name)]
    (if (nil? service)
      (let [worker (start-worker service-name
                                 (str "js/gnunet-service-" service-name ".js"))
            port (.-port worker)]
        (add-service service-name port)
        (recur service-name message-port))
      (.postMessage service (clj->js {:type "connect"
                                      :port message-port})
                    (array message-port)))))
