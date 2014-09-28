;; filesharing.cljs - filesharing service for gnunet-web website
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

(ns gnunet-web.filesharing
  (:use [gnunet-web.encoder :only (encode-uint32)]
        [gnunet-web.message :only (encode-message parse-message-types)]
        [gnunet-web.parser :only (items parser parse-absolute-time parse-uint32
                                  parse-utf8 tail)]
        [gnunet-web.service :only (client-connect)])
  (:require [goog.crypt])
  (:require-macros [monads.macros :as monadic]))

(def message-type-start-search 136)
(def message-type-fs-put 138)
(def block-type-u 9)

(defn encode-start-search
  [{:keys [options type anonymity-level target query]
    :or {options 0
         type 0
         anonymity-level 0
         target (repeat 32 0)}}]
  (encode-message
    {:message-type message-type-start-search
     :message (concat
                (encode-uint32 options)
                (encode-uint32 type)
                (encode-uint32 anonymity-level)
                target
                query)}))

(def parse-u-block
  (monadic/do parser
              [signature (items 64)
               purpose (items 8)
               verification-key (items 32)
               data tail]
              {:signature signature
               :purpose purpose
               :verification-key verification-key
               :data data}))

(def parse-encrypted-u-block
  (monadic/do parser
              [update-identifier parse-utf8
               uri parse-utf8
               metadata tail]
              {:update-identifier update-identifier
               :uri uri
               :metadata metadata}))

(defn unwords
  [word-array]
  (take (.-sigBytes word-array)
        (mapcat #(list (bit-and 0xff (bit-shift-right % 24))
                       (bit-and 0xff (bit-shift-right % 16))
                       (bit-and 0xff (bit-shift-right % 8))
                       (bit-and 0xff %))
                (.-words word-array))))

(defn decrypt
  [data]
  (let [ciphertext (js/CryptoJS.lib.CipherParams.create
                     (clj->js
                       {:ciphertext (js/CryptoJS.lib.WordArray.create data)}))
        twofish-key (js/CryptoJS.enc.Hex.parse "9A89B29A3896E200279ADC010D7F70F7F9A0582190881A17825B87754CA95B81")
        twofish-iv (js/CryptoJS.enc.Hex.parse "9E71233934B556C9D9E97609CD22FBC1")
        plaintext (js/CryptoJS.TwoFish.decrypt ciphertext twofish-key
                                               (clj->js
                                                 {:iv twofish-iv
                                                  :mode js/CryptoJS.mode.CFB}))
        ciphertext (js/CryptoJS.lib.CipherParams.create
                     (clj->js {:ciphertext plaintext}))
        aes-key (js/CryptoJS.enc.Hex.parse "8D2E569BB26A7F45A933941996DC43C2F2C141F055AD2C863EE9A66379D59B61")
        aes-iv (js/CryptoJS.enc.Hex.parse "FBE06BD519F62556BE465A67B80777EC")
        plaintext (js/CryptoJS.AES.decrypt ciphertext aes-key
                                           (clj->js
                                             {:iv aes-iv
                                              :mode js/CryptoJS.mode.CFB}))]
    (unwords plaintext)))

(def parse-fs-put
  (with-meta
    (monadic/do parser
                [type parse-uint32
                 expiration parse-absolute-time
                 last-transmission parse-absolute-time
                 transmission-count parse-uint32
                 respect-offered parse-uint32
                 data tail]
                (conj
                  {:type type
                   :expiration expiration
                   :last-transmission last-transmission
                   :transmission-count transmission-count
                   :respect-offered respect-offered}
                  (condp = type
                    block-type-u
                    {:u-block (let [u-block @(parse-u-block data)]
                                (if (coll? u-block)
                                  (first u-block)))}
                    {:data data})))
    {:message-type message-type-fs-put}))

(defn search
  [{:keys [callback] :as args}]
  (let [message-channel (js/MessageChannel.)]
    (set! (.-onmessage (.-port1 message-channel))
          (fn [event]
            (let [message @((parse-message-types #{parse-fs-put})
                              (.-data event))]
              (if (coll? message)
                (callback (:message (first message)))))))
    (client-connect "fs" "web app (search)"
                    (.-port2 message-channel))
    (.postMessage (.-port1 message-channel)
                  (into-array (encode-start-search args)))))
