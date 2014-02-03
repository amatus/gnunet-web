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
  (:use [gnunet-web.encoder :only (encode-uint32)]
        [gnunet-web.hello :only (encode-hello equals-hello merge-hello
                                 message-type-hello parse-hello)]
        [gnunet-web.message :only (encode-message parse-message-types
                                   parse-peer-identity)]
        [gnunet-web.parser :only (parser parse-uint32)]
        [gnunet-web.service :only (add-service)]
        [tailrecursion.cljson :only (clj->cljson cljson->clj)])
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

(def parse-list-peer
  (with-meta
    (monadic/do parser
                [include-friend-only parse-uint32
                 peer parse-peer-identity]
                {:include-friend-only include-friend-only
                 :peer peer})
    {:message-type message-type-peerinfo-get}))

(def parse-list-peer-all
  (with-meta
    (monadic/do parser
                [include-friend-only parse-uint32]
                {:include-friend-only include-friend-only})
    {:message-type message-type-peerinfo-get-all}))

(defn encode-info-message
  [peer hello]
  (encode-message
    {:message-type message-type-peerinfo-info
     :message
     (concat
       (encode-uint32 0) ;; reserved
       peer
       (when hello (encode-hello hello)))}))

(def info-end-message
  (encode-message {:message-type message-type-peerinfo-info-end}))

(def hostmap (atom nil))
(def notify-clients (atom #{}))
(def notify-clients-friend-only (atom #{}))

(defn notify-all
  [entry]
  (let [msg-pub (encode-info-message (:identity entry)
                                     (:hello entry))
        msg-friend (encode-info-message (:identity entry)
                                        (:friend-only-hello entry))]
    (doseq [nc @notify-clients]
      (.postMessage nc (to-array msg-pub)))
    (doseq [nc @notify-clients-friend-only]
      (.postMessage nc (to-array msg-friend)))))

(defn update-friend-hello
  [hello friend-hello]
  (merge-hello hello (or friend-hello {:public-key (:public-key hello)
                                       :friend-only true})))

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
      (swap! hostmap assoc-in [peer dest] merged)
      (when-not (:friend-only hello)
        (swap! hostmap assoc-in [peer :friend-only-hello]
               (update-friend-hello merged (:friend-only-hello host))))
      (let [host (get @hostmap peer)]
        (.setItem js/localStorage
                  (str "hello:" peer)
                  (clj->cljson (:hello host)))
        (.setItem js/localStorage
                  (str "friend-only-hello:" peer)
                  (clj->cljson (:friend-only-hello host)))
        (notify-all host)))))

(defn add-host-to-known-hosts
  [public-key]
  (let [host {:identity public-key}]
    (swap! hostmap
           (fn [hostmap]
             (if (contains? hostmap public-key)
               hostmap
               (assoc hostmap public-key host))))
    (notify-all host)
    (when-let [hello (cljson->clj
                       (.getItem js/localStorage (str "hello:" public-key)))]
      (update-hello hello))
    (when-let [hello (cljson->clj
                       (.getItem js/localStorage (str "friend-only-hello:"
                                                      public-key)))]
      (update-hello hello))))

(defn process-hello
  [hello]
  (println "processing hello:" (clj->cljson hello))
  (add-host-to-known-hosts (:public-key hello))
  (update-hello hello))

(def peerinfo-message-channel (js/MessageChannel.))
(def clients (atom #{}))

(defn client-get-message
  [event]
  (let [message (first (.-v ((parse-message-types #{parse-hello
                                                    parse-peerinfo-notify
                                                    parse-list-peer
                                                    parse-list-peer-all})
                               (.-data event))))]
    (println "peerinfo-msg:" (js/JSON.stringify (clj->js message)))
    (condp = (:message-type message)
      message-type-hello (process-hello (:message message))
      message-type-peerinfo-notify
      (swap!
        (if (:include-friend-only (:message message))
          notify-clients-friend-only
          notify-clients)
        conj (.-target event))
      message-type-peerinfo-get
      (let [get-message {:message message}
            peer (:public-key get-message)
            hello-type (if (:include-friend-only get-message)
                         :friend-only-hello
                         :hello)
            hello (hello-type (get @hostmap peer))]
        (.postMessage (.-target event)
                      (encode-info-message peer hello))
        (.postMessage (.-target event) info-end-message))
      message-type-peerinfo-get-all
      (let [hello-type (if (:include-friend-only (:message message))
                         :friend-only-hello
                         :hello)]
        (doseq [entry @hostmap]
          (.postMessage (.-target event)
                        (encode-info-message (key entry)
                                             (hello-type (val entry)))))
        (.postMessage (.-target event) info-end-message)))))

(defn start-peerinfo
  []
  (set! (.-onmessage (.-port1 peerinfo-message-channel))
        (fn [event]
          ;; This must be a connect message
          (let [port (.-port (.-data event))]
            (swap! clients conj port)
            (set! (.-onmessage port) client-get-message))))
  (add-service "peerinfo" (.-port2 peerinfo-message-channel)))

