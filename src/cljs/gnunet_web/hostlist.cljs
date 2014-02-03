;; hostlist.cljs - hostlist routines for gnunet-web website
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

(ns gnunet-web.hostlist
  (:use [gnunet-web.http :only (GET)]
        [gnunet-web.parser :only (none-or-more parser)]
        [gnunet-web.peerinfo :only (process-hello)]
        [gnunet-web.hello :only (parse-hello)]
        [gnunet-web.message :only (parse-message-types)])
  (:require-macros [cljs.core.async.macros :refer [go]]
                   [monads.macros :as monadic]))

(def parse-hostlist
  (monadic/do parser
              [hellos (none-or-more (parse-message-types #{parse-hello}))]
              (doseq [hello hellos]
                (process-hello (:message hello)))))

(defn fetch-and-process!
  []
  (go
    (let [buf (<! (GET "hostlist"))
          hostlist (js/Uint8Array. buf)]
      (parse-hostlist hostlist))))
