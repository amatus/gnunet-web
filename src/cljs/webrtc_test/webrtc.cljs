;; webrtc.cljs - webrtc routines for webrtc-test website
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

(ns webrtc-test.webrtc)

(defn new-peer-connection
  [& {:keys [servers options]
      :or {servers {:iceServers [{:url "stun:stun.l.google.com:19302"}]}
           options {:optional [{:RtpDataChannels true}]}}}]
  (js/webkitRTCPeerConnection. (clj->js servers) (clj->js options)))

