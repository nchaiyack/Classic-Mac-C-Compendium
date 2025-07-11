(game-module "modern"
  (title "Modern Times")
  (blurb "Economics and politics of today's world")
  (variants (see-all true))
  )

(unit-type engrs (image-name "soldiers"))

(unit-type oil-well (image-name "tower")) ; for now

(unit-type city (image-name "city20"))

(define places (oil-well city))

(material-type food)
(material-type oil)
(material-type people)

(include "stdterr")

;;; Static relationships.

(table unit-capacity-x
  (city engrs 1)
  (city oil-well 1)
  )

(table unit-storage-x
  (city food 900)
  (city oil 900)
  )

(table unit-size-in-terrain
  (u* t* 1)
  (city t* 16)
  )

(add t* capacity 16)

(table terrain-storage-x
  (t* oil 100)
  )

;;; Actions.

(add engrs acp-per-turn 1)

(add city acp-per-turn 1)

;;; Movement.

(add places speed 0)

;;; Construction.

(add engrs cp 2)

(add oil-well cp 6)

(table acp-to-create
  (engrs oil-well 1)
  (city engrs 1)
  (city oil-well 1)
  )

(table cp-on-creation
  (engrs oil-well 3)
  (city engrs 1)
  (city oil-well 1)
  )

(table separation-to-create-max
  (city oil-well 2)
  )

(table acp-to-build
  (engrs oil-well 1)
  (city engrs 1)
  (city oil-well 1)
  )

(table cp-per-build
  (engrs oil-well 3)
  (city engrs 1)
  (city oil-well 1)
  )

(table separation-to-build-max
  (city oil-well 2)
  )

;;; Automatic production.

(table base-production
  ((oil-well city) oil (20 10))
  )

(table base-consumption
  ((oil-well city) oil (1 10))
  )

(table hp-per-starve
  (u* oil 1.00)
  )

(table out-length
  (oil-well oil 5)
  )

(table in-length
  (city oil 5)
  )

;;; Initial random setup.

(add city start-with 1)
(add city start-with #| independent-near-start |# 5)
(set country-radius-min 3)
(set country-separation-min 16)
(set country-separation-max 48)
; Try to get countries on the coast.
(add (sea plains) country-terrain-min (1 4))

(table favored-terrain
  (u* t* 0)
  (city plains 100)
  (city land-t* 20)
  (city plains 40)
  )

(table independent-density (city plains 100))

(add land-t* country-people-chance 90)
(add plains country-people-chance 100)

(add land-t* independent-people-chance 50)

(table unit-initial-supply
  (u* m* 9999)
  (city oil 100)
  )

(table terrain-initial-supply
  (t* oil 100)
  )

(game-module (design-notes (
  "A detailed game of modern problems."
  ""
  "(should have a many-nukes-available option)"
  )))
