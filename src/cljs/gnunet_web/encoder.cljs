;; encoder.cljs - network encodings for gnunet-web website
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

(ns gnunet-web.encoder
  (:require [goog.crypt]))

(defn encode-uint8
  [x]
  [(bit-and 0xff x)])

(defn encode-uint16
  [x]
  (concat
    (encode-uint8 (bit-shift-right x 8))
    (encode-uint8 x)))

(defn encode-uint32
  [x]
  (concat
    (encode-uint16 (bit-shift-right x 16))
    (encode-uint16 x)))

(defn encode-uint64
  [x]
  (concat
    (encode-uint32 (bit-shift-right x 32))
    (encode-uint32 x)))

(defn encode-utf8
  [x]
  (concat
    (goog.crypt/stringToUtf8ByteArray x)
    0))

(defn encode-date
  [x]
  (encode-uint64 (.valueOf x)))
