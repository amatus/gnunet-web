;; datastructures.cljs - functions for organizing data
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

(ns amatus.datastructures)

(defn nested-group-by
  ([fs coll] (nested-group-by fs identity coll))
  ([fs inner-fn coll]
   (if (empty? fs)
     (inner-fn coll)
     (persistent!
       (reduce
         (fn [ret [k v]]
           (assoc! ret k (nested-group-by (rest fs) inner-fn v)))
         (transient {}) (group-by (first fs) coll))))))

(defn flatten-nested-maps
  ([nested-maps level-keys] (flatten-nested-maps nested-maps level-keys {}))
  ([nested-maps [level-key & level-keys] level-accumulator]
   (if (empty? level-keys)
     (for [v nested-maps]
       (assoc level-accumulator level-key v))
     (mapcat #(flatten-nested-maps
                (val %)
                level-keys
                (assoc level-accumulator level-key (key %)))
      nested-maps))))
