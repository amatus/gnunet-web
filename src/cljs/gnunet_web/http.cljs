;; http.cljs - http routines for gnunet-web website
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

(ns gnunet-web.http
  (:require [goog.net.XhrIo]
            [goog.net.XhrIo.ResponseType :as ResponseType]
            [goog.events :as events]
            [goog.net.EventType :as EventType]
            [cljs.core.async :refer [chan close!]])
  (:require-macros [cljs.core.async.macros :refer [go]]))

(defn GET
  [url]
  (let [ch (chan 1)
        xhr (goog.net.XhrIo.)]
    (events/listen xhr EventType/COMPLETE
            (fn [event]
              (let [res (.getResponse xhr)]
                (go (>! ch res)
                    (close! ch)))))
    (events/listen xhr EventType/READY #(.dispose xhr))
    (.setResponseType xhr ResponseType/ARRAY_BUFFER)
    (.send xhr url)
    ch))
