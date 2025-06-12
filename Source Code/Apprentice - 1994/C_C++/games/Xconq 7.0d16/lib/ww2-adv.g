(game-module "ww2-adv"
  (title "Advanced WWII")
  (blurb "Types for global-scale simulation of WWII, at national level")
  )

(scorekeeper (do last-side-wins))  ; to get mplayers to do something

;;; Types.

(unit-type infantry (image-name "infantry")
  (help "a primarily foot-powered army"))
(unit-type armor (image-name "armor")
  (help "a primarily mechanized army"))

(define ground-types (infantry armor))

(unit-type air-force (image-name "4e")
  (help "strategic bombing capability"))
(unit-type bomber
  (help "strategic bombing capability"))
(unit-type interceptor (image-name "1e")
  (help "anti-bomber capability"))

(define air-types (air-force bomber interceptor))

(unit-type convoy (image-name "ap"))
(unit-type fleet (image-name "bb"))
(unit-type cv-fleet (image-name "cv"))
(unit-type sub-fleet (image-name "sub"))
(unit-type asw-fleet (image-name "dd"#|"asw"|#))

(define ship-types (convoy fleet cv-fleet sub-fleet asw-fleet))

(unit-type base (image-name "airbase")
  (help ""))
(unit-type town (image-name "town20")
  (help ""))
(unit-type city (image-name "city20")
  (help "major cities only"))
(unit-type port-city (image-name "port")
  (help "major port cities only"))
(unit-type capital (image-name "city20")
  (help "capital of a country or region"))

(define place-types (base town city port-city capital))

(define builders         (town city port-city capital))

(define inf infantry)

(material-type oil)

(terrain-type sea (color "sky blue") (image-name "sea") (char ".")
  (help "deep water"))
(terrain-type shallows (color "cyan") (image-name "shallows") (char ","))
(terrain-type swamp (color "yellow green") (image-name "swamp") (char "="))
(terrain-type desert (color "yellow") (image-name "desert") (char "~")
  (help "dry open terrain"))
(terrain-type land (color "green") (image-name "plains") (char "+"))
(terrain-type forest (color "forest green") (image-name "forest") (char "%"))
(terrain-type mountains (color "sienna") (image-name "mountains") (char "^"))
(terrain-type ice (color "white") (image-name "ice") (char "_"))
(terrain-type neutral (color "gray") #|(image-name "ice")|# (char "_"))
(terrain-type river (color "blue") (char "<")
  (subtype border) (subtype-x river-x))
(terrain-type cliffs (color "blue") (char "|")
  (subtype border))
(terrain-type marsh (color "blue") (char "=")
  (subtype border))
(terrain-type road (color "gray") (char ">")
  (subtype connection) (subtype-x road-x))

; need a coating for snow

;(table terrain-interaction (river (sea shallows) non-terrain))

(add (sea shallows) liquid true)

(define sea-t* (sea shallows))

(define land-t* (desert land forest mountains))

;;; Static relationships.

(table vanishes-on
  (ground-types sea-t* true)
  (ground-types ice true)
  (ship-types land-t* true)
  (ship-types ice true)
  (place-types sea-t* true)
  (place-types ice true)
  (u* neutral true)
  )

;;; Unit-unit capacities.

;(table unit-capacity-x
;  (cv fighter 4)
;  )

(table unit-size-as-occupant
  (u* u* 99)  ; disable occupancy by default
  (ground-types (convoy fleet) 1)
  (ground-types place-types 20)
  (air-types (convoy fleet cv-fleet) 1)
  (air-types place-types 1)
  (ship-types place-types 1)
  )

(add (convoy fleet cv-fleet) capacity (2 1 2))

(add place-types capacity 40)

;;; Unit-terrain capacities.

(table unit-size-in-terrain
  (u* t* 1)
  (ground-types t* 50)
  (place-types t* 100)
  )

(add t* capacity 100)

;;; Vision.

(set terrain-seen true)

(add place-types see-always true)

;;; Actions.

(add u* acp-per-turn 1)

(add ground-types acp-per-turn (4 5))

(add ship-types acp-per-turn 40)

;;; Movement.

(add place-types speed 0)

(table mp-to-enter-terrain
;  ((cv bb) shallows 2)
;  (a (swamp forest mountains) 10)
  (ground-types sea-t* 99)
  (ship-types land-t* 99)
  (u* ice 99)
  (air-types ice 1)
  (u* neutral 99)
)

;;; Marshes and cliffs should be basically uncrossable when landing.

;;; Construction.

(add u* cp 3)

(table acp-to-create
  (builders ground-types 1)
;  (builders air-types 1)
  (builders ship-types 1)
  )

(table cp-on-creation
  (builders ground-types 1)
  (builders air-types 1)
  (builders ship-types 1)
  )

(table acp-to-build
  (builders ground-types 1)
  (builders air-types 1)
  (builders ship-types 1)
  )

(table cp-per-build
  (builders ground-types 1)
  (builders air-types 1)
  (builders ship-types 1)
  )

#|
(table supply-on-creation
  (u* fuel ( 6 10 18 36 100 100 200 400 200 1 200 500 900))
  (u* ammo ( 6  4  3  3  20  10  20  40  40 1 100 200 400))
)

(table unit-initial-supply
  (u* fuel ( 6 10 18 36 100 100 200 400 200 1 200 500 900))
  (u* ammo ( 6  4  3  3  20  10  20  40  40 1 100 200 400))
)
|#
;;; Combat.

(table acp-to-attack
  (u* u* 1)
  (convoy u* 0)
  )

(add place-types hp-max (3 36 48 48 48))

;; Bombers actually attack at a distance.

(add bomber range 8)

(table hit-chance
  (u* u* 50)
  ;; Convoys are not combat units.
  (convoy u* 0)
  (place-types infantry 20)
  (place-types armor 10)
  )

(table damage
  (u* u* 1)
  (convoy u* 0)
  )

(table capture-chance
  ;; Armor can be captured and made use of.
  (ground-types armor 20)
  ;; Cities offer basically zippo resistance to armies.
  (ground-types place-types 90)
  )

;;; Backdrop.

(table people-surrender-chance
  (ground-types t* 100)
  (place-types t* 100)
  )

;;; The world.

(world 360 (year-length 12))

;;; Sides.

(set side-library '(
  ((name "America") (short-name "USA"))
  ((name "England") (short-name "UK"))
  ((name "Russia"))
  ((name "France"))
  ((name "Germany"))
  ((name "Italy"))
  ((name "Japan"))
  ;; and other minor countries...
  ))

(set advantage-min 1)
(set advantage-default 1)
(set advantage-max 1)

(set calendar ("usual" "month"))

;;; Random setup parameters (for testing).

(add t* alt-percentile-min   0)
(add t* alt-percentile-max   0)
(add (sea land) alt-percentile-min ( 0  70))
(add (sea land) alt-percentile-max (70 100))
(add t* wet-percentile-min   0)
(add t* wet-percentile-max 100)

;;; River generation.

(add (land forest mountains) river-chance (5000 10000 10000))

(set river-sink-terrain shallows)

;;; Road generation.

(table road-into-chance
  (t* land 100)
  (land (desert forest mountains) (50 40 20))
  )

(set edge-terrain ice)

(set country-radius-min 4)

;(add u* start-with 1)

(add capital start-with 1)

;(add (sea land) country-terrain-min (4 4))

(table favored-terrain
  (u* t* 0)
  (ground-types land 100)
  (air-types land 100)
  (ship-types sea 100)
  (place-types land 100)
  )

(add land country-people-chance 100)

(game-module (design-notes
  "The basic idea of this game is to model the grand strategy of the WWII era."
  "Scale is 1 deg or 90-100km/hex, 1 month/turn."
  ""
  "At this scale the details of maneuver become less important than managing"
  "production and logistics."
  ""
  "Land units are infantry and armor armies, basically the same except that armor"
  "is more highly mechanized."
  ""
  "Air forces represent strategic bombing abilities."
  ""
  "Convoys are primarily transport ships with some protection."
  ""
  "Fleets are battleship/cruiser fleets, while carrier fleets have a long-range"
  "strike capability due to their aircraft."
  "Submarine fleets represent a large number of subs operating over a wide area."
  ""
  "City production is very hard to change."
  ))

