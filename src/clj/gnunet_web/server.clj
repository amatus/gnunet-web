;; server.clj - development web server
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

(ns gnunet-web.server
  (:require [ring.adapter.jetty :as jetty]
            [ring.middleware.resource :as resource]
            [ring.middleware.head :as head]
            [ring.middleware.content-type :as content-type]
            [ring.middleware.not-modified :as not-modified]
            [ring.util.response :as response])
  (:gen-class))

(defn handler [request]
  (response/redirect "/index.html"))

(def app
  (-> handler
    (head/wrap-head)
    (resource/wrap-resource "public")
    (content-type/wrap-content-type)
    (not-modified/wrap-not-modified)))

(defn -main [& args]
  (jetty/run-jetty app {:port 3000}))

