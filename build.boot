(set-env!
  :dependencies '[[adzerk/boot-cljs           "1.7.228-1"]
                  [cljsjs/jquery              "2.2.2-0"]
                  [com.cognitect/transit-cljs "0.8.239"]
                  [fence                      "0.2.0"]
                  [hoplon/boot-hoplon         "0.1.13"]
                  [hoplon/hoplon              "6.0.0-alpha16"]
                  [net.clojure/monads         "1.0.2"]
                  [org.clojure/clojure        "1.7.0"]
                  [org.clojure/clojurescript  "1.7.170"]
                  [org.clojure/core.async     "0.2.374"]
                  [pandeiro/boot-http         "0.7.3"]]
  :source-paths #{"src/cljs" "src/hl" "src/js"}
  :resource-paths #{"assets"})

(require
  '[adzerk.boot-cljs   :refer [cljs]]
  '[hoplon.boot-hoplon :refer [hoplon prerender]]
  '[pandeiro.boot-http :refer [serve]])

(deftask dev
  "Build gnunet-web for development."
  []
  (comp
    (serve :dir "target/" :port 8000)
    (watch)
    (speak)
    (hoplon)
    (cljs :pretty-print true)))

(deftask prod
  "Build gnunet-web for production."
  []
  (comp
    (hoplon)
    (cljs :optimizations :advanced)
    (prerender)))

;; vim: set filetype=clojure :
