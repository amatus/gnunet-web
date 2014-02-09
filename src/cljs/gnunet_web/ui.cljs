;; ui.cljs - UI routines for gnunet-web website
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

(ns gnunet-web.ui
  (:use [gnunet-web.hostlist :only (fetch-and-process!)]
        [gnunet-web.peerinfo :only (start-peerinfo)]
        [gnunet-web.service :only (start-worker)]))

(defn by-id
  [id]
  (.getElementById js/document (name id)))

(set! *print-fn* (fn [string]
                   (let [output (by-id :output)]
                     (set! (.-textContent output)
                           (str (.-textContent output) string)))))
;;(set! *print-fn* #(.log js/console %))

(start-peerinfo)
(def topology-worker (start-worker "topology" "js/gnunet-daemon-topology.js"))

(.addEventListener
  (by-id :hostlist)
  "click"
  (fn [event]
    (fetch-and-process!)))
