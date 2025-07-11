(game-module "napoleon"
  (title "Napoleon")
  (blurb "definitions for the Napoleonic wars")
  (variants
   (see-all true)
   (world-seen true)
   (world-size)
   ("Winds" winds
    (add t* wind-force-min 1)
    (add t* wind-force-average 1)
    (add t* wind-force-max 4)
    (add t* wind-force-variability 50.00)
    (add t* wind-variability 50.00)
    )
   ("Scoring" scoring
	(add places point-value (0 5 25))
	(scorekeeper (do last-side-wins))
	)
   )
)

(unit-type inf (name "infantry") (image-name "soldiers")
  (help "the backbone of the army"))
(unit-type guards (image-name "soldiers")
  (help "the elite of the army"))
(unit-type cav (name "cavalry") (image-name "cavalry")
  (help "zips around for shock effect"))
(unit-type ldr (name "leader") (image-name "flag")
  (help "controls the movements of land forces"))
(unit-type transport (image-name "brig")
  (help "how armies get across water"))
(unit-type frigate (image-name "frigate")
  (help "the eyes of the fleet"))
(unit-type fleet (image-name "twodecker")
  (help "a full line of battle fleet"))
(unit-type balloon (image-name "balloon")
  (help "fragile but good for reconnaissance"))
(unit-type / (name "entrenchments") (image-name "camp")
  (help "temporary protection for armies"))
(unit-type city (name "city") (image-name "town20")
  (help "a typical city"))
(unit-type capital (name "capital") (image-name "city18")
  (help "the major city of a country"))

(material-type food)
(material-type shot)

(terrain-type sea
  (color "sky blue") (image-name "sea") (char ".")
  (help "deep ocean"))
(terrain-type shallows
  (color "cyan") (image-name "shallows") (char ",")
  (help "coastal waters and lakes"))
(terrain-type swamp
  (color "yellow green") (image-name "swamp") (char "=")
  )
(terrain-type desert
  (color "yellow") (image-name "desert") (char "~")
  )
(terrain-type plains
  (color "green") (image-name "plains") (char "+")
  )
(terrain-type forest
  (color "forest green") (image-name "forest") (char "%")
  )
(terrain-type mountains
  (color "sienna") (image-name "mountains") (char "^")
  )

(define land-t* (desert plains forest mountains))

(define cell-t* (sea shallows swamp desert plains forest mountains))

(terrain-type road
  (color "gray") (char ">")
  (subtype connection) (subtype-x road-x))

(terrain-type river
  (color "blue") (char "<")
  (subtype border) (subtype-x river-x))

(terrain-type snow
  (color "white")
  (subtype coating))

(add t* elevation-min -100)
(add t* elevation-max 2000)
(add (sea shallows swamp) elevation-min 0)
(add (sea shallows swamp) elevation-max (0 0 10))
(add (mountains) elevation-min 2000)
(add (mountains) elevation-max 9000)

(add (sea shallows snow) liquid true)

#|
;;; Some defns for the fractal percentile generator.

(set alt-blob-density 10000)
(set alt-blob-height 500)
(set alt-blob-size 200)
(set alt-smoothing 4)
(set wet-blob-density 2000)
(set wet-blob-size 100)

|#

;;; River generation.

;; Rivers are most likely to start in the mountains or forests.

(add (plains forest mountains) river-chance (20.00 30.00 30.00))

;; Rivers empty into lakes if they don't reach the sea.

(set river-sink-terrain shallows)

;;; Road generation.

(table road-into-chance
  (land-t* land-t* 100)
  ;; Try to get a road back out into the plains.
  (cell-t* plains 100)
  ;; Be reluctant to run through hostile terrain.
  (plains (desert forest mountains) (40 30 20))
  )

;(set edge-terrain ice)

(define water (sea shallows))
(define land (plains desert forest mountains))

(define army-types (inf guards))
(define land-forces (inf guards cav))

(define ships (transport frigate fleet))

(define movers (append land-forces ldr ships balloon))

(define places (/ city capital))

;;; Temperature characteristics of terrain.

(add t* temperature-min -20)
(add water temperature-min 0)
(add desert temperature-min 0)

(add t* temperature-max 30)
(add desert temperature-max 50)
(add mountains temperature-max 20)

(add t* temperature-average 20)

(add land temperature-variability 5)

(add u* already-seen true)

;;; Static relationships.

;;; Unit-unit capacities.

(table unit-capacity-x
  ;; Armies can have only one commander.
  ((inf guards) ldr 1)
  ;; (how can leaders carry others as subordinates?)
  )

(table unit-size-as-occupant
  ;; Disable occupancy normally.
  (u* u* 99)
  (land-forces (transport fleet) 1)
  (ldr u* 1)
  (movers (city capital) 1)
  )

(add (transport fleet / city capital) capacity (4 2 10 80 80))

(table occupant-max
  (u* u* 99)
  )

;;; Unit-terrain interaction.

(table vanishes-on
  (land-forces sea true)
  (ships land true)
  (places sea true)
  )

;;; Unit-terrain capacities.

;; Allow effectively infinite capacity everywhere.

(table unit-size-in-terrain
  (u* t* 1)
  )

(add t* capacity 100)

;;; Actions.

(add u* acp-per-turn (1 1 3 10 2 5 3 5 0 1 1))
;; capitals should be more capable?

(table occupant-acp-effect
  (ldr inf 800)
  (ldr guards 1000)
  )

;;; Movement.

(add places speed 0)

(table mp-to-enter-terrain
  (u* t* 1)
  (land-forces sea 99)
  ;; Cavalry is slower in rough terrain.
  (cav (forest mountains) 2)
  ;; Leaders move about as fast as a frigate, when at sea.
  (ldr (sea shallows) 2)
  ;; Leaders are also slower in the mountains.
  (ldr (mountains) 2)
  (ships land 99)
  ;; (no effect for shallows, ships were small then)
  )

;;; Construction.

(add u* cp (20 40 8 12 10 20 40 4 1 1 1))

(table acp-to-create
  (army-types / 1)
  ((city capital) movers 1)
  (city balloon 0)  ; only let capitals build balloons
  )

(table cp-on-creation
  (army-types / 1)
  ((city capital) movers 1)
)

(table acp-to-build
  ((city capital) movers 1)
  (city balloon 0)  ; only let capitals build balloons
  )

(table cp-per-build
  ((city capital) movers 1)
  )

;;; (ships should need lots of tooling up to build)

;;; Cities can repair anything.

(table acp-to-repair
  ((city capital) u* 1)
  )

;;; Navy ships can repair themselves automatically.

(add (frigate fleet) hp-recovery 1)

;1 food land-forces produce
;1 food / produce
;10 [ food shot ] [ city capital ] produce
;100 [ plains forest ] land-forces productivity
;100 [ plains forest mountains ] / productivity
;0 desert / productivity
;100 plains [ city capital ] productivity
;70 [ forest mountains ] [ city capital ] productivity
;30 desert [ city capital ] productivity
;[ 4 10  4 300 300 300  0 10 500 1000 ] food u* storage
;[ 1  0 30  30  30  50  0 100 200 400 ] shot u* storage
;1 food land-forces consume
;1 food ships consume
;-1 r* land-forces out-length

;;; Combat.

(add u* hp-max (10 10 2 1 3 6 1 1 10 20 40))

(add army-types parts-max 10)

(table hit-chance
  (u* u* 50)
  ;; Leaders should never expose themselves to combat directly.
  (u* ldr 90)
  (ldr u* 0)
  ;; Balloons are somewhat protected by their altitude.
  (u* balloon 50)
  (balloon u* 0)
  )

(table damage
  (u* u* 1)
  (fleet u* 3)
  )

(table capture-chance
  (army-types city (20 30))
  (army-types capital (10 15))
  )

;;; Scuttling and disbanding was easy in those days.

(add movers acp-to-disband 1)

;;; Environmental effects.

(add land-forces survival-zone-min -30)
(add land-forces comfort-zone-min    5)
(add land-forces comfort-zone-max   30)
(add land-forces survival-zone-max  50)

;;; The major participants.

(set side-library '(
  ((name "France") (adjective "French"))
  ((name "England") (adjective "English"))
  ((name "Spain") (adjective "Spanish"))
  ((name "Austria") (adjective "Austrian"))
  ((name "Prussia") (adjective "Prussian"))
  ((name "Russia") (adjective "Russian"))
  ))

(world 1000 (year-length 12))

(set calendar '(usual "month"))

;;; Random setup stuff.  Sort of meaningless for a historical game, but useful.

(add cell-t* alt-percentile-min (  0  50  50  51  51  51  95))
(add cell-t* alt-percentile-max ( 50  51  51  95  95  95 100))
(add cell-t* wet-percentile-min (  0   0  50   0  10  90   0))
(add cell-t* wet-percentile-max (100 100 100  10  90 100 100))

(add plains country-terrain-min 7)
(set country-radius-min 3)
(set country-separation-min 8)

(add (city capital) start-with (5 1))

;(add city independent-density 1000)

(table favored-terrain add
  (u* t* 0)
  ((city capital) plains 100)
  )

(game-module (notes "player notes here"
  ))

(game-module (design-notes (
  "Map scale is 50 km/hex, game time is 1 month/turn."
  ""
  "Balloons are more fun than realistic."
  )))

