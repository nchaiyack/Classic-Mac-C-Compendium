(game-module "ww2-div-eur"
  (title "ww2-division-europe")
  (blurb "Defns for division-level WWII in Europe")
  (variants
   (world-seen true)
   (see-all true)
   ("Keep Score" eval (scorekeeper (do last-side-wins)))
   )
  )

(unit-type a-inf (name "infantry(a)") (image-name "inf-div")
  (help "Allied infantry division")
  )
(unit-type a-armor (name "armor(a)") (image-name "armor-div")
  (help "Allied armored division")
  )
(unit-type a-para (name "paratroops(a)") (image-name "para-div")
  )

(define a-ground-types (a-inf a-armor a-para))

(add a-ground-types possible-sides "allies")

(add a-ground-types acp-per-turn (3 6 3))

(unit-type g-inf (name "infantry(g)") (image-name "inf-div")
  )
(unit-type g-armor (name "armor(g)") (image-name "armor-div")
  )
(unit-type g-para (name "paratroops(g)") (image-name "para-div")
  )
(unit-type pzgren (name "panzergrenadiers") (image-name "pzgren-div")
  )
(unit-type metro (image-name "metro-div") (image-name "inf-div")
  )
(unit-type cd (image-name "cd-div") (image-name "inf-div")
  )

(define g-ground-types (g-inf g-armor g-para pzgren metro cd))

(add g-ground-types possible-sides "axis")

(add g-ground-types acp-per-turn (3 5 3 3 3 2))

(unit-type hq (image-name "hq-div") (image-name "flag")
  )

(add hq acp-per-turn 6)

(define ground-types (append a-ground-types g-ground-types hq))

(define non-mot-types (a-inf g-inf))

(define mot-types (a-armor g-armor))

(unit-type train)

;; should have assault craft distinct from transports.
(unit-type transport (image-name "ap"))
(unit-type battleship (image-name "bb"))

(define ship-types (transport battleship))

(unit-type airbase)
(unit-type v2-base (image-name "v2"))
(unit-type port)
(unit-type coast-defense (image-name "walltown"))
(unit-type town (image-name "town20"))

(define place-types (airbase v2-base port coast-defense town))

;;; Abstracted aircraft. (should have diff types?)

(material-type org
  (help "organization - important to do things, acquired from HQs"))
(material-type aircraft
  (help "attached aircraft used for support"))

; sea clr brkn/bocage rough/hills mtn urb road rail river rhine

(terrain-type sea)
(terrain-type swamp)
(terrain-type clear)
(terrain-type broken (image-name "bocage"))
(terrain-type hills (image-name "forest"))
(terrain-type mountains)
(terrain-type urban)
(terrain-type road (subtype connection) (subtype-x road-x))
(terrain-type railroad (subtype connection) (subtype-x road-x))
(terrain-type river (subtype border) (subtype-x river-x))
(terrain-type Rhine-river (subtype border))

(define land-t* (swamp clear broken hills mountains urban))

(define cell-t* (sea swamp clear broken hills mountains urban))

;;; Static relationships.

(table vanishes-on
  (ground-types sea true)
  (ship-types land-t* true)
  (train t* true)
  (train railroad false)
  (place-types sea true)
  )

;; Unit-in-terrain capacities.

(add t* capacity 3)

;; Unit-in-unit capacities.

(table unit-size-as-occupant
  (u* u* 99)
  (ground-types (transport town) 1)
  (ground-types train 1)
  (train town 1)
  )

(add transport capacity 4)

(add train capacity 1)

(add town capacity 6)

(table unit-capacity-x
  (port ship-types (6 2))
  )

;;; Vision.

;;; Static units can't do anything without the world's reporters finding
;;; out within the week.

(add place-types see-always true)

;;; Action parameters.

(add ship-types acp-per-turn 14) ; 28 would be better
(add train acp-per-turn 28)
(add place-types acp-per-turn 0)
(add (airbase town) acp-per-turn 7) ; for airstrikes

;;; Movement.

(add ship-types speed 70.00)
(add place-types speed 0)

(table mp-to-enter-terrain
  (mot-types hills 2)
  (ground-types mountains 2)
  (mot-types mountains 4)
  (hq mountains 1)
  (ground-types river 2)
  (ground-types Rhine-river 3)
  (ground-types sea 9999)
  (train t* 9999)
  (train railroad 1)
  (ship-types land-t* 9999)
  )

(table zoc-range
  (ground-types ground-types 1)
  (hq u* 0)
  )

(table zoc-into-terrain
  (ground-types t* false)
  (ground-types land-t* true)
  )

(table mp-to-enter-zoc
  (ground-types ground-types 7)
  )

;;; Repair.

(add u* hp-recovery 0.50)

;;; (should have towns repair units also, but would need to be able to act then!)
;;; (could say nonzero acp-to-xxx with zero acp-max means auto-actions)

;;; Combat.

(add ground-types hp-max 12)
(add place-types hp-max 48)

(table hit-chance
  (u* u* 50)
  (a-inf u* 60)
  (a-armor u* 40)
  (a-para u* 50)
  (g-inf u* 40)
  (g-armor u* 40)
  (g-para u* 70)
  (pzgren u* 60)
  (cd u* 30)
  ;; HQs are not for combat.
  (hq u* 0)
  ;; Transport combat is non-existent at this scale.
  (u* transport 0)
  (transport u* 0)
  ;; Battleships are very good at shelling everything.
  (battleship u* 90)
  ;; ...but not so good with each other.
  (battleship battleship 50)
  (u* train 0)
  )

(table damage
  (u* u* 2d4)
  (hq u* 0)
  (u* transport 0)
  (transport u* 0)
  (u* train 0)
  )

;; Battleships can reach inland a little ways.

(add battleship acp-to-fire 1)
(add battleship range 2)

;; Aircraft give towns and bases an immense reach.

(add (airbase town) range 30)

(table consumption-per-attack
  ((airbase town) aircraft 1)
  )

(table capture-chance
  (ground-types train 50)
  ;; Towns don't usually have much integral defense.
  (ground-types place-types 90)
  )

;;; Definitions used for testing.

;;; Some defns for the fractal percentile generator.

(set alt-blob-density 10000)
(set alt-blob-size 20)
(set alt-smoothing 6)
(set wet-blob-density 2000)
(set wet-blob-size 100)

; sea low clr brkn rough mtn urb

(add cell-t* alt-percentile-min (  0  30  30  30  70  90  30))
(add cell-t* alt-percentile-max ( 30  35  70  70  90 100  70))
(add cell-t* wet-percentile-min (  0  50   0  50   0   0  45))
(add cell-t* wet-percentile-max (100 100  45 100 100 100  50))

;;; River generation.

(add (clear hills mountains) river-chance (1000 1000 2000))

(set river-sink-terrain sea)

;;; Road generation.

(table road-chance
  (town town 100)
  (town (port airbase) 100)
  )

(table road-into-chance
  (land-t* land-t* 100)
  )

(set country-radius-min 6)

(add (sea clear) country-terrain-min (5 5))

(add u* start-with 1)  ; one of everything

(table favored-terrain
  (u* t* 100)
  (u* (sea swamp) 0)
  (ship-types sea 100)
  (ship-types land-t* 0)
  )

;;; No political detailing needed at the divisional level.

(set side-library '(
  ((name "Allies") (adjective "Allied") (class "allies")
   )
  ((name "Axis") (adjective "Axis") (class "axis")
   (emblem-name "german-cross"))
  ))

(world 2500 (year-length 52))

(game-module (design-notes (
  "The map scale is 10 miles/hex, game time is 1 week/turn."
  ""
  "This game design covers division-level operations in Europe during WWII."
  "Players take on the roles of army commanders but not political leaders;"
  "the game does not include political factors."
  )))

#|

Organization is consumed by movement/combat, supplied by HQs
Have organization material increase effectiveness of combat (and movement).

HQs should auto-retreat.  Inability to retreat results in capture.

Parachutists have restrictions on initial placement (why?), won't
drop further than 10 hexes from friendlies (even on invasion?)
(use control radius to impl)

Mulberry is like a stationary transport, placed in the water by ships,
has better unit/supply transfer parms.

Can build fortresses (very slowly), units can be inside or outside.

Admin movement should use trains/trucks (introduce as units with
huge move allowance?).

All units have high prob to surrender - use ZOC to calculate.

|#
