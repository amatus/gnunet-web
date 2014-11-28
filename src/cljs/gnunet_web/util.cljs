;; util.cljs - utility routines for gnunet-web website
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

(ns gnunet-web.util)

(defn now
  []
  (-> (js/Date.) (.valueOf) (* 1000)))

(def objects (atom {:next 0}))

(defn register-object
  [object]
  (swap! objects
         (fn [{:keys [next] :as objects}]
           (conj objects
                 {next object
                  :next (inc next)})))
  ;; This would normally be cheating
  (dec (:next @objects)))

(defn unregister-object
  [object-key]
  (swap! objects dissoc object-key))

(defn get-object
  [object-key]
  (get @objects object-key))

(defn real-to-i64
  [x]
  [(unsigned-bit-shift-right x 0)
   (Math/min 4294967295 (Math/floor (/ x 4294967296)))])

(defn i64-to-real
  [[lw hw]]
  (+ lw (* 4294967296 hw)))
