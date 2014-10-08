;; parser.cljs - parser monad for gnunet-web website
;; Copyright (C) 2013,2014  David Barksdale <amatus@amatus.name>
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

(ns gnunet-web.parser
  (:require [goog.crypt :refer (utf8ByteArrayToString)]
            [monads.core :refer (bind get-state join maybe plus set-state
                                 state-t zero)])
  (:require-macros [monads.macros :as monadic]))

(extend-type monads.core/state-transformer
  IMeta
  (-meta [x] (.-meta x))
  IWithMeta
  (-with-meta [x meta]
    ;; XXX: return a new state-transformer, don't mutate this one
    (set! (.-meta x) meta)
    x))

(def parser (state-t maybe))

(defn m-until
  "While (p x) is false, replace x by the value returned by the
   monadic computation (f x). Return (parser x) for the first
   x for which (p x) is true."
  [p f x]
  (letfn [(until [p f x s]
            (if (p x)
              (bind (set-state s) (fn [_] (parser x)))
              (let [y ((f x) s)]
                (if (= y (zero y))
                  (zero (parser nil))
                  (let [[x s] (join y)]
                    (recur p f x s))))))]
    (bind (parser nil)
      (fn [_] (bind (get-state) (fn [s] (until p f x s)))))))

(defn optional
  "Makes a parser optional."
  [mv]
  (plus [mv (parser nil)]))

(defn none-or-more
  "Makes a parser repeat none or more times."
  [mv]
  (bind
    (m-until
      first
      (fn [[_ xs]]
        (plus
          [(bind mv (fn [x] (parser [false (conj xs x)])))
           (parser [true xs])]))
      [false []])
    (comp parser second)))

(defn one-or-more
  "Makes a parser repeat one or more times."
  [mv]
  (monadic/do parser
              [x mv
               xs (none-or-more mv)]
              (cons x xs)))

(defn repeat
  "Makes a parser repeat exactly n times."
  [mv n]
  (m-until
    #(= n (count %))
    #(bind mv (fn [x] (parser (conj % x))))
    []))

;; Parsing Typed Arrays
(def tail (get-state))

(defn items
  "Produces a parser which consumes n items from the input.
  Input must be a Uint8Array."
  [n]
  (monadic/do parser
              [array (get-state)
               :when (<= n (.-length array))
               _ (set-state (.subarray array n))]
              (.subarray array 0 n)))

(defn satisfy
  "Produces a parser that matches an item which satisfies the given predicate."
  [p]
  (monadic/do parser
              [x (items 1)
               :let [x (aget x 0)]
               :when (p x)]
              x))

(def parse-uint8
  "Parse an unsigned 8-bit integer.
   Input must be a Uint8Array."
  (monadic/do parser
              [array (get-state)
               :when (<= 1 (.-length array))
               _ (set-state (.subarray array 1))]
              (aget array 0)))

(def parse-uint16
  "Parse an unsigned 16-bit integer in network byte order (big endian).
   Input must be a Uint8Array."
  (monadic/do parser
              [msb parse-uint8
               lsb parse-uint8]
              (+ lsb (* 256 msb))))

(def parse-uint32
  "Parse an unsigned 32-bit integer in network byte order (big endian).
   Input must be a Uint8Array."
  (monadic/do parser
              [msh parse-uint16
               lsh parse-uint16]
              (+ lsh (* 65536 msh))))

(def parse-uint64
  "Parse an unsigned 64-bit integer in network byte order (big endian).
   Clojurescript cannot represent a 64-bit integer so we return a vector
   of numbers.
   Input must be a Uint8Array."
  (monadic/do parser
              [msw parse-uint32
               lsw parse-uint32]
              [msw lsw]))

(def parse-utf8
  (monadic/do parser
              [xs (none-or-more (satisfy #(not (== 0 %))))
               _ (items 1)]
              (utf8ByteArrayToString (to-array xs))))

(def parse-absolute-time parse-uint64)
