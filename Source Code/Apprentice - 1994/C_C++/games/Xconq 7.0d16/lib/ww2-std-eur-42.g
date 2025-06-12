(game-module "ww2-std-eur-42"
  (title "WWII (Std), Europe Jan 42")
  (blurb "The situation in Europe, January 1942 (using standard types)")
  (base-module "ww2-stdunit")
  (variants (see-all true))
  )

(add u* start-with 0)
(add u* independent-near-start 0)

(set synthesis-methods nil)

(include "eur-100km")

(include "u-eur-1942")

(set initial-date "Jan 1942")

(scorekeeper (do last-side-wins)) ; for now
