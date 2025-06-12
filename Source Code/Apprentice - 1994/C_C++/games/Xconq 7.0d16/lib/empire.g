(game-module "empire"
  (title "True Empire")
  (blurb "The classic big economic/military game")
  )

(unit-type infantry)
(unit-type armor)
(unit-type engineers (image-name "soldiers"))

(define ground-types (infantry armor engineers))

(unit-type dd)
(unit-type sub)
(unit-type bb)

(define ship-types (dd sub bb))

(unit-type fighter)
(unit-type bomber)

(define air-types (fighter bomber))

(unit-type irbm)
(unit-type icbm)

(define missile-types (irbm icbm))

(unit-type minefield)

(unit-type farm)
(unit-type oil-field)
(unit-type iron-mine)
(unit-type gold-mine)
(unit-type uranium-mine)
(unit-type hydro-plant)

(define extraction-types
  (farm oil-field iron-mine gold-mine uranium-mine hydro-plant))

(unit-type radar)
(unit-type bank)
(unit-type lab)
(unit-type port)
(unit-type airfield)
(unit-type refinery)
(unit-type factory)
(unit-type warehouse)
(unit-type nuclear-plant)

(define facility-types
  (radar bank lab port airfield refinery factory warehouse nuclear-plant))

(unit-type city) ; make variable-size?
(unit-type capital)

(define city-types (city capital))

(material-type food)
(material-type oil)
(material-type petrol)
(material-type ore)
(material-type iron)
(material-type gold)
(material-type rads)
(material-type civs)
(material-type money)
(material-type manufactures)

(terrain-type sea)
(terrain-type wilderness (image-name "forest"))
(terrain-type settled (image-name "clear"))
(terrain-type mountains)
(terrain-type wasteland)

;;; Each side should be able to have its own type of money...

;;; Static relationships.

(table unit-capacity-x
  ;; Engineers can get inside most anything.
  (extraction-types engineers 1)
  (facility-types engineers 1)
  (city-types engineers 1)
  (city-types facility-types 1)
  )

(table unit-size-in-terrain
  (u* t* 1)
  (extraction-types t* 16)
  (facility-types t* 16)
  (city-types t* 16)
  )

(add t* capacity 16)

(table terrain-storage-x
  (t* oil 100)
  (t* ore 100)
  (t* gold 10)
  (t* rads 10)
  )

;;; Actions.

(add u* acp-per-turn 1)

(add extraction-types speed 0)
(add facility-types speed 0)
(add city-types speed 0)

;;; Construction.

(add u* cp 2)

(table acp-to-create
  (engineers u* 1)
  (city-types u* 1)
  )

(table cp-on-creation
  (engineers u* 1)
  (city-types u* 1)
  )

(table acp-to-build
  (engineers u* 1)
  (city-types u* 1)
  )

(table cp-per-build
  (engineers u* 1)
  (city-types u* 1)
  )

;;; Random setup.

(add capital start-with 1)

(add (sea wilderness) alt-percentile-min (0 70))
(add (sea wilderness) alt-percentile-max (70 100))
(add (sea wilderness) wet-percentile-min 0)
(add (sea wilderness) wet-percentile-max 100)

(table terrain-initial-supply
  (t* oil 100)
  (t* ore 100)
  (t* gold 1)  ; should be rare but high concentration
  (t* rads 1)
  )

(area 20 20)  ; small for now

(game-module (design-notes (
  "Scale is unimportant, this is an abstract game."
  )))
