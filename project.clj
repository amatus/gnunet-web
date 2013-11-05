(defproject gnunet-web "0.1.0-SNAPSHOT"
  :description "A GNUnet web application."
  :url "https://github.com/amatus/gnunet-web"
  :dependencies [[org.clojure/clojure "1.4.0"]
                 [ring "1.1.8"]]
  :plugins [[lein-cljsbuild "0.3.0"]]
  :hooks [leiningen.cljsbuild]
  :source-paths ["src/clj"]
  :cljsbuild { 
    :builds [{
      :source-paths ["src/cljs/webrtc_test"]
      :compiler {:output-to "resources/public/js/webrtc-test.js"
                 :optimizations :simple
                 :pretty-print true}
      :jar true}
     {
      :source-paths ["src/cljs/gnunet_web"]
      :compiler {:output-to "resources/public/js/gnunet-web.js"
                 :optimizations :simple
                 :pretty-print true}
      :jar true}]}
  :main gnunet-web.server)

