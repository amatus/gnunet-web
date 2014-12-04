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
            [gnunet-web.extractor :as e]
            [gnunet-web.service :as service] ;; leave this here
            [gnunet-web.util :refer [get-object i64-to-real read-memory
                                     real-to-i64 register-object
                                     unregister-object]])
  (:require-macros [cljs.core.async.macros :refer [go]]))

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

(defn keywords-from-metadata
  [metadata]
  (let [uri-pointer (js/_GNUNET_FS_uri_ksk_create_from_meta_data metadata)
        keywords-pointer (js/_GNUNET_FS_uri_ksk_to_string_fancy uri-pointer)
        keywords (js/Pointer_stringify keywords-pointer)]
    (js/_free keywords-pointer)
    (js/_GNUNET_FS_uri_destroy uri-pointer)
    keywords))

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
                  (read-memory data data-size))}))

(defn parse-progress-publish
  [status info-pointer]
  (conj
    {:status status
     :cctx (js/_GNUNET_FS_ProgressInfo_get_publish_cctx info-pointer)
     :size (js/_GNUNET_FS_ProgressInfo_get_publish_size info-pointer)
     :completed (js/_GNUNET_FS_ProgressInfo_get_publish_completed info-pointer)}
    (condp = status
      :publish-completed
      {:uri (uri-pointer-to-string
              (js/_GNUNET_FS_ProgressInfo_get_publish_completed_chk_uri
                info-pointer))}
      nil)))

(defn parse-progress-download
  [status info-pointer]
  (conj
    {:status status
     :cctx (js/_GNUNET_FS_ProgressInfo_get_download_cctx info-pointer)
     :size (js/_GNUNET_FS_ProgressInfo_get_download_size info-pointer)
     :completed (js/_GNUNET_FS_ProgressInfo_get_download_completed
                  info-pointer)}
    (condp = status
      :download-progress
      (let [data-pointer (js/_GNUNET_FS_ProgressInfo_get_download_progress_data
                           info-pointer)
            offset (js/_GNUNET_FS_ProgressInfo_get_download_progress_offset
                     info-pointer)
            data-len (js/_GNUNET_FS_ProgressInfo_get_download_progress_data_len
                       info-pointer)
            depth (js/_GNUNET_FS_ProgressInfo_get_download_progress_depth
                    info-pointer)
            data (js/Uint8Array.
                   (js/HEAPU8.subarray data-pointer (+ data-pointer data-len)))]
        {:data data
         :offset offset
         :depth depth})
      nil)))

(defn parse-progress-search
  [status info-pointer]
  (conj
    {:status status
     :cctx (js/_GNUNET_FS_ProgressInfo_get_search_cctx info-pointer)}
    (condp = status
      :search-result (let [metadata (atom [])
                           callback (partial metadata-iterator metadata)
                           callback-pointer (js/Runtime.addFunction callback)]
                       (js/_GNUNET_CONTAINER_meta_data_iterate
                         (js/_GNUNET_FS_ProgressInfo_get_search_result_meta
                           info-pointer)
                         callback-pointer)
                       (js/Runtime.removeFunction callback-pointer)
                       {:uri (uri-pointer-to-string
                               (js/_GNUNET_FS_ProgressInfo_get_search_result_uri
                                 info-pointer))
                        :metadata @metadata})
      nil)))

(def status-publish-start 0)
(def status-publish-progress 3)
(def status-publish-completed 5)
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
  (let [status (js/_GNUNET_FS_ProgressInfo_get_status info-pointer)]
    (condp = status
      status-publish-start (parse-progress-publish :publish-start info-pointer)
      status-publish-progress (parse-progress-publish :publish-progress
                                                      info-pointer)
      status-publish-completed (parse-progress-publish :publish-completed
                                                       info-pointer)
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
    ((get-object (:cctx info)) info)
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
        callback-key (register-object callback)
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
  (js/_GNUNET_FS_search_stop search)
  (unregister-object callback-key)
  (close! ch))

(defn start-download
  [uri anonymity]
  (let [uri-pointer (string-to-uri-pointer uri)
        length (js/_GNUNET_FS_uri_chk_get_file_size2 uri-pointer)
        ch (chan 1)
        callback (fn [info] (go (>! ch info)))
        callback-key (register-object callback)
        download (js/_GNUNET_FS_download_start_simple fs uri-pointer anonymity
                                                      callback-key)]
    (js/_GNUNET_FS_uri_destroy uri-pointer)
    {:download download
     :size length
     :ch ch
     :callback-key callback-key}))

(defn publish-reader-callback
  [cls offset-lw offset-hw size buf emsg]
  (if (zero? size)
    (do (unregister-object cls) 0)
    (let [file (get-object cls)
          offset (i64-to-real [offset-lw offset-hw])
          file-array (js/Uint8Array. file offset size)]
      (js/writeArrayToMemory file-array buf)
      size)))

(def publish-reader-callback-pointer
  (js/Runtime.addFunction publish-reader-callback))

(defn new-block-options
  [{:keys [expiration anonymity priority replication]}]
  (js/_GNUNET_FS_BlockOptions_new expiration anonymity priority replication))

(defn start-publish
  [file keywords metadata block-options]
  (let [file-key (register-object file)
        length (real-to-i64 (.-byteLength file))
        ch (chan 1)
        callback (fn [info] (go (>! ch info)))
        callback-key (register-object callback)
        uri-pointer (uri-ksk-create keywords)
        bo-pointer (new-block-options block-options)
        fi (js/_GNUNET_FS_file_information_create_from_reader
             fs
             callback-key ; void *client_info
             (first length) (second length) ; uint64_t length
             publish-reader-callback-pointer ; GNUNET_FS_DataReader reader
             file-key ; void *reader_cls
             uri-pointer ; struct GNUNET_FS_Uri *keywords
             metadata ; struct GNUNET_CONTAINER_MetaData *meta
             0 ; int do_index
             bo-pointer); struct GNUNET_FS_BlockOptions *bo
        publish (js/_GNUNET_FS_publish_start
                  fs
                  fi
                  0 ; ns
                  0 ; nid
                  0 ; nuid
                  0)] ; options
    (js/_GNUNET_FS_uri_destroy uri-pointer)
    (js/_free bo-pointer)
    {:publish publish
     :ch ch
     :callback-key callback-key}))

