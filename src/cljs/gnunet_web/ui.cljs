(ns gnunet-web.ui)

(.addEventListener
  (.getElementById js/document "new")
  "click"
  (fn []
    (js/alert "whoozle?")))

