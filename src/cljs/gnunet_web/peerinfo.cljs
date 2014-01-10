;; peerinfo.cljs - peerinfo service for gnunet-web website
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

(ns gnunet-web.peerinfo
  (:use [gnunet-web.hello :only (equals-hello merge-hello message-type-hello
                                 parse-hello update-friend-hello)]
        [gnunet-web.message :only (parse-message-types)]
        [gnunet-web.parser :only (parser parse-uint32)]
        [gnunet-web.service :only (add-service)])
  (:require-macros [monads.macros :as monadic]))

(def message-type-peerinfo-get 330)
(def message-type-peerinfo-get-all 331)
(def message-type-peerinfo-info 332)
(def message-type-peerinfo-info-end 333)
(def message-type-peerinfo-notify 334)

(def parse-peerinfo-notify
  (with-meta
    (monadic/do parser
                [include-friend-only parse-uint32]
                {:include-friend-only include-friend-only})
    {:message-type message-type-peerinfo-notify}))

(def hostmap (atom nil))

(defn notify-all
  [entry]
  )

(defn add-host-to-known-hosts
  [public-key]
  (let [host {:identity public-key}]
    (swap! hostmap
           (fn [hostmap]
             (if (contains? hostmap public-key)
               hostmap
               (assoc hostmap public-key host))))
    ;; TODO load host
    (notify-all host)))

(defn update-hello
  [hello]
  (let [peer (:public-key hello)
        host (get @hostmap peer)
        dest (if (:friend-only hello)
               :friend-only-hello
               :hello)
        merged (merge-hello hello (dest host))
        delta (equals-hello merged (dest host) (js/Date.))]
    (when-not (= :equal delta)
      (swap! hostmap assoc-in [peer dest] merged))
    (if (:friend-only hello)
      (swap! hostmap assoc-in [peer :friend-only]
             (update-friend-hello merged (:friend-only host))))
    ;; TODO save host
    (notify-all host)))

(def peerinfo-message-channel (js/MessageChannel.))
(def clients (atom #{}))

(defn client-get-message
  [event]
  (let [message (first (.-v ((parse-message-types #{parse-hello
                                                    parse-peerinfo-notify})
                               (.-data event))))]
    (println "peerinfo-msg:" (js/JSON.stringify (clj->js message)))
    (condp = (:message-type message)
      message-type-hello
      (do
        (add-host-to-known-hosts (:public-key (:message message)))
        (update-hello (:message message)))
      message-type-peerinfo-notify
      nil)))

(defn start-peerinfo
  []
  (set! (.-onmessage (.-port1 peerinfo-message-channel))
        (fn [event]
          ;; This must be a connect message
          (let [port (.-port (.-data event))]
            (swap! clients conj port)
            (set! (.-onmessage port) client-get-message))))
  (add-service "peerinfo" (.-port2 peerinfo-message-channel)))

