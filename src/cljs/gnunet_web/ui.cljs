(ns gnunet-web.ui)

(defn by-id
  [id]
  (.getElementById js/document (name id)))

(defn output
  [string]
  (let [output (by-id :output)]
    (set! (.-textContent output)
          (str (.-textContent output) "\n" string))))

(def peerinfo (js/SharedWorker. "js/gnunet-service-peerinfo.js"))

(set! (.-onerror peerinfo)
      (fn [event]
        (output (str "peerinfo:"
                     (.-filename event) ":" (.-lineno event) " "
                     (.-message event)))))

(set! (.-onmessage (.-port peerinfo))
      (fn [event]
        (let [data (.-data event)]
          (if (= "stdout" (.-type data))
            (set! (.-onmessage (.-port data))
                  (fn [event] (output (.-data event))))
            (output (.-data event))))))

(.start (.-port peerinfo))
(.postMessage (.-port peerinfo) (clj->js {:type "stdout"}))
