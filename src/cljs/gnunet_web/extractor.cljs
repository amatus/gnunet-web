;; extractor.cljs - libextractor used by gnunet-web website
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

(ns gnunet-web.extractor)

(def format-unknown 0)
(def format-utf8 1)
(def format-data 2)
(def format-string 3) ; unknown character encoding

(defn metatype-to-string
  [type]
  (js/Pointer_stringify
    (js/_EXTRACTOR_metatype_to_string type)))

(defn metatype-to-description
  [type]
  (js/Pointer_stringify
    (js/_EXTRACTOR_metatype_to_description type)))
