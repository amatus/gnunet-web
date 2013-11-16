;; hostlist.cljs - hostlist routines for gnunet-web website
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

(ns gnunet-web.hostlist
  (:require [gnunet-web.http :as http])
  (:require-macros [cljs.core.async.macros :refer [go]]))

(defn fetch-and-process!
  [peerinfo]
  (go
    (let [hostlist (<! (http/GET "hostlist"))]
      (.log js/console (str "Got hostlist: " hostlist)))))
