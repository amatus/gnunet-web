(set-env!
  :dependencies '[[adzerk/boot-cljs          "0.0-3308-0"]
                  [adzerk/boot-reload        "0.3.0"]
                  [fence                     "0.2.0"]
                  [net.clojure/monads        "1.0.2"]
                  [org.clojure/clojure       "1.7.0"]
                  [org.clojure/clojurescript "0.0-3308"]
                  [org.clojure/core.async    "0.1.346.0-17112a-alpha"]
                  [pandeiro/boot-http        "0.6.3"]
                  [tailrecursion/boot-hoplon "0.1.0"]
                  [tailrecursion/cljson      "1.0.7"]
                  [tailrecursion/hoplon      "6.0.0-alpha4"]]
  :source-paths #{"src/cljs" "src/hl" "src/js"}
  :resource-paths #{"assets"})

(require
  '[adzerk.boot-cljs          :refer [cljs]]
  '[adzerk.boot-reload        :refer [reload]]
  '[pandeiro.boot-http        :refer [serve]]
  '[tailrecursion.boot-hoplon :refer [hoplon prerender]])

(deftask dev
  "Build gnunet-web for development."
  []
  (comp
    (serve :dir "target/" :port 8000)
    (watch)
    (speak)
    (hoplon)
    (reload)
    (cljs :pretty-print true)))

(deftask prod
  "Build gnunet-web for production."
  []
  (comp
    (hoplon)
    (cljs :optimizations :advanced)
    (prerender)))

;; vim: set filetype=clojure :
