(ns gnunet-web.webrtc)

(defn new-peer-connection
  [& {:keys [servers options]
      :or {servers {:iceServers [{:url "stun:stun.l.google.com:19302"}]}
           options {:optional [{:RtpDataChannels true}]}}}]
  (js/webkitRTCPeerConnection. (clj->js servers) (clj->js options)))

