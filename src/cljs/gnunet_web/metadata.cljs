;; metadata.cljs - metadata parser for gnunet-web website
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

(ns gnunet-web.metadata
  (:require [gnunet-web.extractor :as e :refer (type-descriptions)]
            [gnunet-web.parser :as p :refer (items parser parse-uint32)]
            [goog.crypt :refer (utf8ByteArrayToString)]
            [monads.core :as m :refer (get-state update-state)])
  (:require-macros [monads.macros :as monadic]))

(defn decompress
  [x]
  (js/pako.inflate x))

(def parse-entry
  (monadic/do parser
              [type parse-uint32
               format parse-uint32
               data-size parse-uint32
               plugin-name-length parse-uint32
               mime-type-length parse-uint32]
              {:type type
               :format format
               :data-size data-size
               :plugin-name-length plugin-name-length
               :mime-type-length mime-type-length}))

(defn parse-entry-data
  [{:keys [type format data-size plugin-name-length mime-type-length]}]
  (monadic/do parser
              [mime-type (items mime-type-length)
               plugin-name (items plugin-name-length)
               data (items data-size)]
              {:type type
               :format format
               :data data
               :plugin-name (utf8ByteArrayToString plugin-name)
               :mime-type (utf8ByteArrayToString mime-type)}))

(def compressed   0x80000000)
(def version-mask 0x7FFFFFFF)
(def entry-size 20)

(def parse-metadata
  (monadic/do parser
              [version parse-uint32
               :when (= 2 (bit-and version-mask version))
               entry-count parse-uint32
               size parse-uint32
               _ (if (bit-and compressed version)
                   (update-state decompress)
                   (parser nil))
               entries (p/repeat entry-count parse-entry)
               ;; GNUNET_CONTAINER_meta_data_deserialize() allows extra data
               ;; between entries and entry-data so we consume it here.
               :let [residue (- size
                               (* entry-size entry-count)
                               (reduce + (map #(+ (:data-size %)
                                                  (:plugin-name-length %)
                                                  (:mime-type-length %))
                                              entries)))]
               _ (items residue)
               entry-data (m/map parse-entry-data (reverse entries))]
              entry-data))

(defn pretty-print
  [metadata]
  (apply str
         (interpose
           "\n"
           (map (fn [{:keys [format type data]}]
                  (when (or (= e/format-utf8 format)
                            (= e/format-string format))
                    (str
                      (first (get type-descriptions type))
                      ": "
                      (utf8ByteArrayToString data))))
                metadata))))
