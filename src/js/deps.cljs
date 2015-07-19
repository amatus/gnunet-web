{:foreign-libs [{:file "client-lib.js"
                 :provides ["client-lib"]}
                {:file "semantic.js"
                 :file-min "semantic.min.js"
                 :provides ["semantic-ui"]
                 :requires ["cljsjs.jquery"]}]
 :externs ["window.crypto.ext.js"]}
