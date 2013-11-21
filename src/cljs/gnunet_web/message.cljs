;; message.cljs - message parser for gnunet-web website
;; Copyright (C) 2013  David Barksdale <amatus@amatus.name>
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

(ns gnunet-web.message
  (:use [gnunet-web.parser :only (items parser parse-uint16)])
  (:require [monads.core :as m])
  (:require-macros [monads.macros :as monadic]))

(def parse-header
  (monadic/do parser
              [size parse-uint16
               message-type parse-uint16]
              {:size size
               :message-type message-type}))

(defn parse-message-types
  "Produces a parser for the specific message types given in the parser-map.
   The parser does not fail if the message-type specific parser does not
   consume the entire input."
  [parser-map]
  (monadic/do parser
              [{message-type :message-type} parse-header
               message (parser-map message-type)]
              {:message-type message-type :message message}))
