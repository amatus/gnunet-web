;; filestorage.cljs - functions for organizing data
;; Copyright (C) 2015  David Barksdale <amatus@amatus.name>
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

(ns amatus.filestorage
  (:require [cljs.core.async :refer [chan close!]])
  (:require-macros [cljs.core.async.macros :refer [go]]))

(def db (atom nil))
(let [request (js/indexedDB.open "filestorage" 1)]
  (set! (.-onupgradeneeded request)
        (fn [e]
            (.createObjectStore (.-result (.-target e)) "files"
                                (js-obj "autoIncrement" true))))
  (set! (.-onerror request)
        (fn [e] (js/console.error "Error creating downloads database" e)))
  (set! (.-onsuccess request)
        (fn [e] (reset! db (.-result (.-target e))))))

(defn create
  [type]
  (let [transaction (.transaction @db (array "files") "readwrite")
        store (.objectStore transaction "files")
        blob (js/Blob. (array) (js-obj "type" type))
        request (.add store blob)
        ch (chan 1)]
    (set! (.-onerror request)
          (fn [e]
            (js/console.error "add request failed" e)
            (close! ch)))
    (set! (.-onsuccess request)
          (fn [e]
            (go (>! ch (.-result (.-target e)))
                (close! ch))))
    ch))

(defn delete
  [handle]
  (let [transaction (.transaction @db (array "files") "readwrite")
        store (.objectStore transaction "files")
        request (.delete store handle)]
    (set! (.-onerror request)
          (fn [e] (js/console.error "delete request failed" e)))))

(defn write
  [handle offset data]
  (let [transaction (.transaction @db (array "files") "readwrite")
        store (.objectStore transaction "files")
        request (.get store handle)]
    (set! (.-onerror request)
          (fn [e] (js/console.error "get request failed" e)))
    (set! (.-onsuccess request)
          (fn [e]
            (let [blob (.-result (.-target e))
                  type (.-type blob)
                  header (.slice blob 0 offset)
                  trailer (.slice blob (+ offset (.-length data)))
                  fill (max 0 (- offset (.-size header)))
                  new-blob (js/Blob. (array header
                                            (js/Uint8Array. fill)
                                            data
                                            trailer)
                                     (js-obj "type" type))
                  request (.put store new-blob handle)]
              (set! (.-onerror request)
                    (fn [e] (js/console.error "put request failed" e))))))))

(defn create-object-url
  [handle]
  (let [transaction (.transaction @db (array "files") "readwrite")
        store (.objectStore transaction "files")
        request (.get store handle)
        ch (chan 1)]
    (set! (.-onerror request)
          (fn [e] (js/console.error "get request failed" e)))
    (set! (.-onsuccess request)
          (fn [e]
            (go (>! ch (js/URL.createObjectURL (.-result (.-target e))))
                (close! ch))))
    ch))
