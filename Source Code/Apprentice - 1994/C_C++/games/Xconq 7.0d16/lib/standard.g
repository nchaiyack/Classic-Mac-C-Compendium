(game-module "standard"
  (title "Standard Game")
  (blurb "The standard Xconq game, loosely based on WW II ca 1945")
  (instructions "Take over the world before you get taken over!")
  (variants
    (world-seen false)
    (see-all false)
    (world-size 60 30 360)
    ("Mostly Land" mostly-land
     (add sea alt-percentile-max 20)
     (add shallows alt-percentile-min 20)
     (add shallows alt-percentile-max 21)
     (add swamp alt-percentile-min 21)
     (add swamp alt-percentile-max 23)
     (add (desert plains forest) alt-percentile-min 21)
     )
    ("All Land" all-land
     (add sea alt-percentile-max 1)
     (add shallows alt-percentile-min 1)
     (add shallows alt-percentile-max 2)
     (add swamp alt-percentile-min 2)
     (add swamp alt-percentile-max 4)
     (add (desert plains forest) alt-percentile-min 2)
     )
    ("Large Countries" large
     (set country-radius-max 100)
     )
    )
)

(include "stdunit")

(include "nat-names")

(include "town-names")

(add (* @) namer "random-town-names")

(scorekeeper
  (title "")
  (do last-side-wins)
  )
