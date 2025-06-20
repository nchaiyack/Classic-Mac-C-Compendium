(game-module "mars"
  (blurb "Mars")
  (variants
    (see-all false)
    (world-seen false)
	)
  )

(terrain-type plani (color "orange") (char "+"))
(terrain-type highlands (color "brown") (char "^"))
(terrain-type canyon (color "tan"))
(terrain-type co2-ice (color "white"))
(terrain-type water-ice (color "light blue"))

(terrain-type wall (subtype border))

(define cell-t* (plani highlands))

(unit-type rover (image-name "hovercraft") (char "c")
  (help "slow, long-range")
  )
(unit-type boulder-car (char "b")
  (help "fast, small, shorter-range, hard for satellites to see")
  )

(define ground-types (rover boulder-car))

(unit-type satellite
  (help "hovers overhead, observes all below")
  )

(unit-type base
  )
(unit-type underground-city
  )
(unit-type domed-city
  (help "large but vulnerable")
  )
(unit-type underice-city
  (help "nearly invisible, can only be placed under water ice")
  )

(material-type air)
(material-type water)
(material-type food)

;;; Static relationships.


;;; Actions.

(add ground-types acp-per-turn 4)

;;; Random game setup.

;;; Some defns for the fractal percentile generator.

(set alt-blob-density 10000)
(set alt-blob-height 500)
(set alt-blob-size 200)
(set alt-smoothing 4)
(set wet-blob-density 2000)
(set wet-blob-size 100)

(add cell-t* alt-percentile-min (  0  70))
(add cell-t* alt-percentile-max ( 69 100))
(add cell-t* wet-percentile-min (  0   0))
(add cell-t* wet-percentile-max (100 100))

(add u* start-with 1)

(area 60 30)

(world 400)

(scorekeeper (do last-side-wins))

(game-module (notes (
  "This game was inspired by Kim Stanley Robinson's `Red Mars',"
  "an excellent story of the near-future colonization of Mars."
  ""
  "The general idea of the game is to build up a colony and perhaps"
  "eventually contend for control of Mars, if players can't work out"
  "agreements."
  )))

(game-module (design-notes (
  "This game is far from complete."
  ""
  "Mars is 21240km in diameter, elevations range over 27km. To show"
  "canyons and such, would need a very large map."
  "10m intervals for elevation would be reasonable."
  "Scenarios could include solitaire game to establish a viable settlement,"
  "and a race for Mars that is competitive."
  ""
  "(should be able to mine for water(ice) and transport to cities)"
  )))

(add satellite notes (
  "Satellites are spy satellites that can see much on the ground,"
  "but require so much observing and processing time that they can"
  "only focus on small areas at a time.  So the position of a satellite"
  "designates its focus area rather than its actual physical position."
  ))
