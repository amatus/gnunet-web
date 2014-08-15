;; code.cljs - functions for encoding and decoding data
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

(ns amatus.code)

(defn pad
  [n padding xs]
  (take n (concat xs (repeat padding))))

(defn base32-hex
  [xs]
  (.toUpperCase
    (apply str
           (mapcat (fn [x]
                     (pad (Math/ceil (/ (* 8 (count x)) 5)) "0"
                          (.toString (reduce (fn [a b] (+ b (* a 256)))
                                             0
                                             (pad 5 0 x))
                                     32)))
                   (partition 5 5 [] xs)))))

(defn base32-crockford
  [xs]
  (apply str
         (map #(->> %
                 (.indexOf "0123456789ABCDEFGHIJKLMNOPQRSTUV")
                 (.charAt  "0123456789ABCDEFGHJKMNPQRSTVWXYZ"))
              (base32-hex xs))))

