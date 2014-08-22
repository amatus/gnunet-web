#!/usr/bin/env boot

#tailrecursion.boot.core/version "2.5.0"

(set-env!
  :project      'gnunet-web
  :version      "0.1.0-SNAPSHOT"
  :dependencies '[[net.clojure/monads        "1.0.2"]
                  [org.clojure/core.async    "0.1.303.0-886421-alpha"]
                  [tailrecursion/boot.task   "2.2.1"]
                  [tailrecursion/hoplon      "5.10.22"]]
  :out-path     "resources/public"
  :src-paths    #{"src"})

;; Static resources (css, images, etc.):
(add-sync! (get-env :out-path) #{"assets"})

(require '[tailrecursion.hoplon.boot :refer :all])

(deftask development
  "Build gnunet-web for development."
  []
  (comp (watch) (hoplon {:prerender false :pretty-print true})))

(deftask production
  "Build gnunet-web for production."
  []
  (hoplon {:optimizations :advanced}))
