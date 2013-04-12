(defproject gnunet-web "0.1.0-SNAPSHOT"
  :description "A GNUnet web application."
  :url "https://github.com/amatus/gnunet-web"
  :dependencies [[org.clojure/clojure "1.4.0"]
                 [ring "1.1.8"]]
  :plugins [[lein-cljsbuild "0.3.0"]]
  :hooks [leiningen.cljsbuild]
  :source-paths ["src/clj"]
  :cljsbuild { 
    :builds {
      :main {
        :source-paths ["src/cljs"]
        :compiler {:output-to "resources/public/js/cljs.js"
                   :optimizations :simple
                   :pretty-print true}
        :jar true}}}
  :main gnunet-web.server)

