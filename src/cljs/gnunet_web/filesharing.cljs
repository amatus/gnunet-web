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
  (:require [cljs.core.async :refer [chan close!]]
            [gnunet-web.extractor :as e])
  (:require-macros [cljs.core.async.macros :refer [go]]))

(def callbacks (atom {:next 0}))

(defn register-callback
  [f]
  (swap! callbacks
         (fn [{:keys [next] :as callbacks}]
           (conj callbacks
                 {next f
                  :next (inc next)})))
  ;; This would normally be cheating
  (dec (:next @callbacks)))

(defn unregister-callback
  [callback]
  (swap! callbacks dissoc callback))

(defn get-callback
  [callback]
  (get @callbacks callback))

(defn uri-ksk-create
  [query]
  (js/ccallFunc
    js/_GNUNET_FS_uri_ksk_create
    "number"
    (array "string" "number")
    (array query 0)))

(defn uri-pointer-to-string
  [uri-pointer]
  (let [uri-string-pointer (js/_GNUNET_FS_uri_to_string uri-pointer)
        uri-string (js/Pointer_stringify uri-string-pointer)]
    (js/_free uri-string-pointer)
    uri-string))

(defn string-to-uri-pointer
  [uri]
  (js/ccallFunc
    js/_GNUNET_FS_uri_parse
    "number"
    (array "string" "number")
    (array uri 0)))

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

(defn parse-progress-download
  [status info-pointer]
  (conj
    {:status status
     :cctx (js/getValue (+ 4 info-pointer) "i32")
     :size (js/getValue (+ 24 info-pointer) "i64")
     :completed (js/getValue (+ 48 info-pointer) "i64")}
    (condp = status
      :download-progress (let [data-pointer (js/getValue (+ 64 info-pointer)
                                                         "i32")
                               offset (js/getValue (+ 72 info-pointer) "i64")
                               data-len (js/getValue (+ 80 info-pointer) "i64")
                               depth (js/getValue (+ 96 info-pointer) "i32")
                               data (js/Uint8Array.
                                      (js/HEAPU8.subarray data-pointer
                                                          (+ data-pointer
                                                             data-len)))]
                           {:data data
                            :offset offset
                            :depth depth})
      nil)))

(def status-download-start 7)
(def status-download-progress 10)
(def status-download-completed 12)
(def status-download-active 14)
(def status-download-inactive 15)
(def status-search-start 17)
(def status-search-result 21)
(def status-search-result-stopped 27)
(def status-search-stopped 29)
(defn parse-progress-info
  [info-pointer]
  (let [status (js/getValue (+ 112 info-pointer) "i32")]
    (condp = status
      status-download-start (parse-progress-download :download-start
                                                     info-pointer)
      status-download-progress (parse-progress-download :download-progress
                                                        info-pointer)
      status-download-completed (parse-progress-download :download-completed
                                                         info-pointer)
      status-download-active (parse-progress-download :download-active
                                                      info-pointer)
      status-download-inactive (parse-progress-download :download-inactive
                                                        info-pointer)
      status-search-start (parse-progress-search :search-start info-pointer)
      status-search-result (parse-progress-search :search-result info-pointer)
      status-search-result-stopped nil
      status-search-stopped nil
      (js/console.warn "ignored status:" status))))

(defn progress-callback
  [cls info-pointer]
  (when-let [info (parse-progress-info info-pointer)]
    ((get-callback (:cctx info)) info)
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
  (let [uri-pointer (uri-ksk-create query)
        ch (chan 1)
        callback (fn [info] (go (>! ch info)))
        callback-key (register-callback callback)
        search (js/_GNUNET_FS_search_start
                 fs
                 uri-pointer
                 anonymity
                 0
                 callback-key)]
    (js/_GNUNET_FS_uri_destroy uri-pointer)
    {:ch ch
     :callback-key callback-key
     :search search}))

(defn stop-search
  [{:keys [ch callback-key search]}]
  (js/ccallFunc
    js/_GNUNET_FS_search_stop
    "void"
    (array "number")
    (array search))
  (unregister-callback callback-key)
  (close! ch))

(defn guess-filename
  [metadata]
  (let [preference ["original filename"
                    "title"
                    ;; ...
                    ]]
    (first
      (for [type preference
            x metadata
            :when (= type (e/metatype-to-string (:type x)))]
        (:data x)))))

(defn start-download
  [uri anonymity]
  (let [uri-pointer (string-to-uri-pointer uri)
        length-lw (js/_GNUNET_FS_uri_chk_get_file_size uri-pointer)
        length-hw js/tempRet0
        ch (chan 1)
        callback (fn [info] (go (>! ch info)))
        callback-key (register-callback callback)
        download (js/_GNUNET_FS_download_start
                   fs
                   uri-pointer
                   0 ; meta
                   0 ; filename
                   0 ; tempname
                   0 ; offset-lw
                   0 ; offset-hw
                   length-lw
                   length-hw
                   anonymity
                   0 ; options
                   callback-key
                   0)] ; parent
    (js/_GNUNET_FS_uri_destroy uri-pointer)
    {:download download
     :size length-lw ;; XXX length truncated for now
     :ch ch
     :callback-key callback-key}))
