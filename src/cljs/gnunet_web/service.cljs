;; service.cljs - service manager for gnunet-web website
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

(ns gnunet-web.service)

(def services (atom {}))

(defn add-service
  [service-name port]
  (swap! services assoc service-name port))

(defn worker-for-service
  [service-name]
  (js/SharedWorker. (str "js/gnunet-service-" service-name ".js")))

(defn client-connect
  [service-name message-port output]
  (let [service (get @services service-name)]
    (if (nil? service)
      (let [worker (worker-for-service service-name)
            port (.-port worker)]
        (add-service service-name port)
        (set! (.-onerror worker)
              (fn [event]
                (output (str service-name ":"
                             (.-filename event) ":" (.-lineno event) " "
                             (.-message event)))))
        (set! (.-onmessage port)
              (fn [event]
                (let [data (.-data event)]
                  (condp = (.-type data)
                    "stdout" (set! (.-onmessage (.-port data))
                                   (fn [event]
                                     (output (str service-name ":"
                                                  (.-data event)))))
                    "client_connect" (client-connect (.-service_name data)
                                                     (.-message_port data)
                                                     output)
                    (output (str service-name ":" (js/JSON.stringify data)))))))
        (.start port)
        (.postMessage port (clj->js {:type "stdout"}))
        (recur service-name message-port output))
      (.postMessage service (clj->js {:type "connect"
                                      :port message-port})
                    (array message-port)))))
