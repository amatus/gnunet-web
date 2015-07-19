;; extractor.cljs - libextractor used by gnunet-web website
;; Copyright (C) 2014,2015  David Barksdale <amatus@amatus.name>
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

(ns gnunet-web.extractor
  (:require [client-lib]))

(def format-unknown 0)
(def format-utf8 1)
(def format-data 2)
(def format-string 3) ; unknown character encoding

(def metatype-mimetype 1)
(def metatype-filename 2)
(def metatype-title 4)
(def metatype-original-filename 180)

(defn metatype-to-string
  [type]
  (js/Pointer_stringify
    (js/_EXTRACTOR_metatype_to_string type)))

(defn metatype-to-description
  [type]
  (js/Pointer_stringify
    (js/_EXTRACTOR_metatype_to_description type)))

(defn metadata-destroy
  [metadata]
  (js/_GNUNET_CONTAINER_meta_data_destroy metadata))

(defn metadata-extract
  [file]
  (let [metadata (js/_GNUNET_CONTAINER_meta_data_create)]
    (js/ccallFunc
      js/_GNUNET_CONTAINER_meta_data_insert
      "number"
      (array "number" "string" "number" "number" "string" "string" "number")
      (array
        metadata
        "<gnunet-web>"
        metatype-mimetype
        format-utf8
        "text/plain"
        (.-type file)
        (inc (count (.-type file)))))
    (js/ccallFunc
      js/_GNUNET_CONTAINER_meta_data_insert
      "number"
      (array "number" "string" "number" "number" "string" "string" "number")
      (array
        metadata
        "<gnunet-web>"
        metatype-original-filename
        format-utf8
        "text/plain"
        (.-name file)
        (inc (count (.-name file)))))
    metadata))

(defn guess-filename
  [metadata]
  (let [preference [metatype-original-filename
                    metatype-filename
                    metatype-title
                    ;; ...
                    ]]
    (first
      (for [type preference
            x metadata
            :when (= type (:type x))]
        (:data x)))))
