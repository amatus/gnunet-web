(ns gnunet-web.ui)

(defn by-id
  [id]
  (.getElementById js/document (name id)))

(def peerinfo (js/Worker. "js/gnunet-service-peerinfo.js"))
(set! (.-onmessage peerinfo)
      (fn [event]
        (let [output (by-id :output)]
          (set! (.-textContent output)
                (str (.-textContent output) "\n" (.-data event))))))
