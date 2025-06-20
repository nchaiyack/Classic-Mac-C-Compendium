(game-module "ww2-stdunit"
  (title "Standard WWII Types")
  (blurb "A simplified version of WWII (using standard types)")
  )

(include "stdunit")

;; Preset the sides, which restricts this to post-Dec 1941, when the
;; sides were clearly lined up.

(side 1 (name "Allies"))

(side 2 (name "Axis"))

(set sides-min 2)
(set sides-max 2)

(set advantage-min 1)
(set advantage-default 1)
(set advantage-max 1)

(game-module (design-notes
  "This is a set of modifications to stdunit that adapt it to the WWII era."
  "As such, it is quite lacking in accuracy, but has the advantage that it is"
  "simpler and more familiar."
  ""
  "This should be included in specific scenarios that have associated dates."
  ))
