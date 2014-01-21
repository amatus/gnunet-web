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
  (:use [amatus.datastructures :only (flatten-nested-maps nested-group-by)]
        [clojure.set :only (difference union)]
        [gnunet-web.encoder :only (encode-date encode-uint16 encode-uint32
                                   encode-utf8)]
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
               :encoded-address (vec (.apply js/Array nil encoded-address))}))

(defn encode-transport-address
  [{transport :transport
    expiration :expiration
    encoded-address :encoded-address}]
  (concat
    (encode-utf8 transport)
    (encode-uint16 (count encoded-address))
    (encode-date expiration)
    encoded-address))

(defn latest-expiration
  [transport-addresses]
  (first (reverse (sort (map :expiration transport-addresses)))))

(defn transport-addresses-map
  [transport-addresses-list]
  (nested-group-by [:transport :encoded-address]
                   latest-expiration
                   transport-addresses-list))

(defn flatten-transport-addresses
  [transport-addresses-nested-map]
  (flatten-nested-maps transport-addresses-nested-map
                       [:transport :encoded-address :expiration]))

(def parse-hello
  (with-meta
    (monadic/do parser
                [friend-only parse-uint32
                 public-key (items 32)
                 addresses (none-or-more parse-transport-address)]
                {:friend-only (not (zero? friend-only))
                 :public-key (vec (.apply js/Array nil public-key))
                 :transport-addresses (transport-addresses-map addresses)})
    {:message-type message-type-hello}))

(defn encode-hello
  [{friend-only :friend-only
    public-key :public-key
    transport-addresses :transport-addresses}]
  (concat
    (encode-uint32 (if friend-only 1 0))
    public-key
    (mapcat encode-transport-address
            (flatten-transport-addresses transport-addresses))))

(defn merge-hello
  [a b]
  {:public-key (:public-key a)
   :friend-only (or (:friend-only a) (:friend-only b))
   :transport-addresses
   (transport-addresses-map
     (concat
       (flatten-transport-addresses (:transport-addresses a))
       (flatten-transport-addresses (:transport-addresses b))))})

(defn equals-hello
  [a b expiration]
  (when (= (:public-key a) (:public-key b))
    (let [as (set (filter
                    #(>= (:expiration %) expiration)
                    (flatten-transport-addresses (:transport-addresses a))))
          bs (set (filter
                    #(>= (:expiration %) expiration)
                    (flatten-transport-addresses (:transport-addresses b))))]
      (if (empty? (union (difference as bs) (difference bs as)))
        :equal
        (let [eas (set (map #(dissoc % :expiration) as))
              ebs (set (map #(dissoc % :expiration) bs))]
          (when (empty? (union (difference eas ebs) (difference ebs eas)))
            (:expiration (first (sort-by :expiration (union as bs))))))))))
