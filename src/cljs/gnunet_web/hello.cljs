;; hello.cljs - HELLO parser for gnunet-web website
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

(ns gnunet-web.hello
  (:use [amatus.datastructures :only (nested-group-by)]
        [gnunet-web.parser :only (items none-or-more parser parse-date
                                        parse-uint16 parse-uint32 parse-utf8)])
  (:require-macros [monads.macros :as monadic]))

(def message-type-hello 17)

(def parse-transport-address
  (monadic/do parser
              [transport parse-utf8
               address-length parse-uint16
               expiration parse-date
               encoded-address (items address-length)]
              {:transport transport
               :expiration expiration
               :encoded-address (.apply js/Array nil encoded-address)}))

(def parse-hello
  (with-meta
    (monadic/do parser
                [friend-only parse-uint32
                 public-key (items 32)
                 addresses (none-or-more parse-transport-address)]
                {:friend-only friend-only
                 :public-key (.apply js/Array nil public-key)
                 :transport-addresses
                 (nested-group-by [:transport :encoded-address] addresses
                                  (partial map :expiration))})
    {:message-type message-type-hello}))

(defn merge-hello
  [a b]
  )

(defn equals-hello
  [a b expiration]
  )

(defn update-friend-hello
  [a b]
  )
