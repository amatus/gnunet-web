(ns gnunet-web.webrtc)

(defn description-callback
  [pc]
  (fn [description]
    (.setLocalDescription pc description)))

(defn new-peer-connection
  []
  (let [servers {:iceServers [{:url "stun:stun.l.google.com:19302"}]}
        options {:optional [{:RtpDataChannels true}]}
        pc (js/webkitRTCPeerConnection. (clj->js servers) (clj->js options))
        dc (.createDataChannel pc "a label" (clj->js {:reliable false}))]
    (set! (.-onicecandidate pc)
          (fn [event]
            (println (str "Candidate: " (js->clj (.-candidate event))))))
    {:pc pc :dc dc}))
