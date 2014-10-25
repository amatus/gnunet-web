;; filesharing.cljs - filesharing client for gnunet-web website
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

(ns gnunet-web.filesharing
  (:require [cljs.core.async :refer (chan close!)]
            [gnunet-web.extractor :as e])
  (:require-macros [cljs.core.async.macros :refer [go]]))

(defn -uri-ksk-create
  [query]
  (js/ccallFunc
    js/_GNUNET_FS_uri_ksk_create
    "number"
    (array "string" "number")
    (array query 0)))

(defn -uri-destroy
  [uri]
  (js/ccallFunc
    js/_GNUNET_FS_uri_destroy
    "void"
    (array "number")
    (array uri)))

(defn -start-search
  [ctx uri anonymity options cctx]
  (js/ccallFunc
    js/_GNUNET_FS_search_start
    "number"
    (array "number" "number" "number" "number" "number")
    (array ctx uri anonymity options cctx)))

(defn uri-pointer-to-string
  [uri-pointer]
  (let [uri-string-pointer (js/_GNUNET_FS_uri_to_string uri-pointer)
        uri-string (js/Pointer_stringify uri-string-pointer)]
    (js/_free uri-string-pointer)
    uri-string))

(defn metadata-iterator
  [metadata cls plugin-name type format mime-type data data-size]
  (swap! metadata conj
         {:plugin-name (js/Pointer_stringify plugin-name)
          :type type
          :format format
          :mime-type (js/Pointer_stringify mime-type)
          :data (if (or (= format e/format-utf8)
                        (= format e/format-string)) 
                  (js/Pointer_stringify data)
                  (.subarray js/HEAP8 data data-size))}))

(defn parse-progress-search
  [status info-pointer]
  (conj
    {:status status
     :cctx (js/getValue (+ 4 info-pointer) "i32")}
    (condp = status
      :search-result (let [metadata (atom [])
                           callback (partial metadata-iterator metadata)
                           callback-pointer (js/Runtime.addFunction callback)]
                       (js/_GNUNET_CONTAINER_meta_data_iterate
                         (js/getValue (+ 32 info-pointer) "i32")
                         callback-pointer)
                       (js/Runtime.removeFunction callback-pointer)
                       {:uri (uri-pointer-to-string
                               (js/getValue (+ 36 info-pointer) "i32"))
                        :metadata @metadata})
      nil)))

(def status-search-start 17)
(def status-search-result 21)
(defn parse-progress-info
  [info-pointer]
  (let [status (js/getValue (+ 112 info-pointer) "i32")]
    (condp = status
      status-search-start (parse-progress-search :search-start info-pointer)
      status-search-result (parse-progress-search :search-result info-pointer)
      (js/console.warn "ignored status:" status))))

(defn progress-callback
  [cls info-pointer]
  (when-let [info (parse-progress-info info-pointer)]
    ;; Since this is a function we registered we can call it with
    ;; non-C-compatible typed arguments and they get passed strait through.
    (js/Runtime.dynCall "vi" (:cctx info) (array info))
    (:cctx info)))

(def fs
  (js/ccallFunc
    js/_GNUNET_FS_start
    "number"
    (array "number" "string" "number" "number" "number" "array")
    (array 0 "gnunet-web" (js/Runtime.addFunction progress-callback) 0 0
           (array 0))))

(defn start-search
  [query anonymity]
  (let [uri (-uri-ksk-create query)
        ch (chan 1)
        callback (fn [info] (go (>! ch info)))
        callback-pointer (js/Runtime.addFunction callback)
        search (-start-search fs uri anonymity 0 callback-pointer)]
    (-uri-destroy uri)
    {:ch ch
     :callback-pointer callback-pointer
     :search search}))

(defn stop-search
  [{:keys [ch callback-pointer search]}]
  (js/ccallFunc
    js/_GNUNET_FS_search_stop
    "void"
    (array "number")
    (array search))
  (js/Runtime.removeFunction callback-pointer)
  (close! ch))
