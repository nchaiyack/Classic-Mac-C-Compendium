(game-module "quest"
  (title "Quest for XP")
  (blurb "Type definitions for fantasy role-playing")
  (variants (see-all false))
)

(unit-type human (image-name "person") (char "@"))

(unit-type orc)
(unit-type elf)
(unit-type dwarf)
(unit-type bugbear)
(unit-type dragon)

(define monster (orc elf dwarf bugbear dragon))
(define animate (append human monster))

(material-type food)
(material-type water)
(material-type gold)

(include "stdterr")

;;; Static relationships.

(include "ng-weird")

(add human namer "generic-names")

(add human possible-sides "human")

(add monster possible-sides "monster")

(table unit-storage-x
  (human m* (10 5 100))
  )

;;; Action parameters.

(add u* acp-per-turn 4)

;;; Movement parameters.

(table mp-to-enter-terrain
  (u* mountains 10)
  (dragon mountains 0)
  )

;;; Combat parameters.

(add u* hp-max 10)

(table hit-chance
  (u* u* 50)
  )

(table damage
  (u* u* 1)
  )

;;; Backdrop activities.

(add u* hp-recovery 100)

;;; Random setup.

;;; One adventurer on a side.

(add human start-with 1)

(add monster start-with 2)

(set country-radius-min 4)
(set country-separation-min 15)
(set country-separation-max 20)

(table favored-terrain
  (u* (sea shallows) 0) 
  (u* plains 100)
  )

;(set synthesis-methods
;  '(make-maze-terrain make-countries make-independent-units))

(set sides-min 1)

(side 1 (name "You") (class "human"))

(side 2 (noun "Monster") (class "monster") (emblem-name "none"))

(scorekeeper (do last-side-wins))

(game-module (notes (
  "This is a sort of outdoors adventure for individual adventurers."
  )))

;;; Can live off friendly terrain, else must carry n days of food/water.
;;; Various pieces of equipment available.

;;; Have nobrains machine-run sides for countries, towns, castles, etc.
;;; (or quiescent unless player becomes outlaw?)

;;; Monsters belong to sides defined by alignment.  Sides of matching
;;; alignment friendly, etc.  No brains to side, but player could run
;;; a monster side, just for fun.
;;; Monsters can breed periodically, but limit total # somehow.

;;; Win by collecting the most treasure after <n> turns.
;;; Treasure should be hidden inside towns, carried by monsters.

