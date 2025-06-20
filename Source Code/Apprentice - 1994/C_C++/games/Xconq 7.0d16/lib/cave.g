(game-module "cave"
  (title "Cave")
  (blurb "Cave Exploration")
;  (variants (see-all false))
)

(terrain-type floor (color "light gray") (char "."))
(terrain-type passage (color "light gray") (char "."))
(terrain-type rock (color "dark gray")  (char "#"))

(unit-type human (image-name "person") (char "@"))

(unit-type orc)
(unit-type elf)
(unit-type dwarf)

(unit-type dragon)
(unit-type pit-demon)

(unit-type diamond (name "huge diamond") (char "*"))

(define wimps (orc elf dwarf))
(define nasties (dragon pit-demon))
(define monster (append wimps nasties))
(define animate (append human monster))
(define items (diamond))

(material-type food)
(material-type water)
(material-type gold)

;;; Static relationships.

(include "ng-weird")

(add human namer "generic-names")

(add human possible-sides "human")

(add monster possible-sides "monster")

(add human storage 100)

(table material-size-in-unit
  (human m* (10 20 1))
  )

;;; Vision.

(add u* vision-range 4)

;;; Action parameters.

(add animate acp-per-turn 4)

;;; Movement parameters.

(table mp-to-enter-terrain
  (u* rock 99)
  )

;;; Combat parameters.

(add u* hp-max 10)

(table hit-chance
  (u* u* 50)
  )

(table damage
  (u* u* 1)
  )

;; Dwarves can dig.

(table acp-to-add-terrain
  (dwarf (floor passage) 1)
  )

(table acp-to-remove-terrain
  (dwarf rock 1)
  )

;;; Backdrop activities.

(add u* hp-recovery 100)

;;; Random setup.

(add floor maze-room-occurrence 1)

(add passage maze-passage-occurrence 1)

(add t* occurrence 0)

(add rock occurrence 1)

;;; One adventurer on a side.

(add human start-with 1)
(add wimps start-with 3)
(add nasties start-with 1)

(set country-radius-min 4)
(set country-separation-min 15)
(set country-separation-max 20)

(table favored-terrain
  (u* floor 100)
  (u* rock 0)
  )

(set synthesis-methods
  '(make-maze-terrain make-countries make-independent-units))

(set sides-min 1)

(side 1 (name "You") (class "human"))

(side 2 (noun "Monster") (class "monster") (emblem-name "none"))

(scorekeeper (do last-side-wins))

(game-module (notes (
  "Cave exploration."
  )))
