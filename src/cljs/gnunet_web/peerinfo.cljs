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
        [gnunet-web.service :only (client-connect)]
        [gnunet-web.util :only (now)]
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

(def peerinfo-message-channel (js/MessageChannel.))
(client-connect "peerinfo" (.-port2 peerinfo-message-channel))

(defn process-hello
  [hello]
  (.postMessage (.-port1 peerinfo-message-channel)
                (into-array (encode-hello hello))))
