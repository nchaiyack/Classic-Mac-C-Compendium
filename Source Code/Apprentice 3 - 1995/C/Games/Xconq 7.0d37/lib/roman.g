(game-module "roman"
  (title "Ancient Rome")
  (blurb "Ancient Rome, ca 100 BC to 200 AD")
  (notes 
   ("Period under construction."
    ""
    "The Roman empire from 100 BC to 100 AD."
    ""
    "Massimo Campostrini (campo@sunthpi3.difi.unipi.it)"
    ))
  (variants
   (world-seen false)
   (see-all false)
   (world-size (60 30 360))
   ("Large Countries" large
    (true (set country-radius-max 100))
    )
   )
  )


(unit-type infantry (char "i") (image-name "pikeman")
  (help "light infantry")
)
(unit-type cavalry (char "c") (image-name "cavalry")
  (help "light cavalry")
)
(unit-type archer (char "a") (image-name "archer")
  (help "archers and slingers")
  (notes "pay attention to arrow supply")
)
(unit-type legion (char "L") (image-name "legion")
  (possible-sides "roman")
  (help "\"division\" of armored soldiers")
)
(unit-type bireme (char "B") (image-name "bireme")
  (help "small two-decked ship")
)
(unit-type trireme (char "T") (image-name "trireme")
  (possible-sides (not "barbarian"))
  (help "three-decked ship - mainstay of the navies")
)
(unit-type barge (char "U") (image-name "barge")
  (help "troop transport, slow and weak")
)
(unit-type siege-engine (name "siege engine")
  (char "S") (image-name "catapult")
  (possible-sides (not "barbarian"))
  (help "built by infantry for attacking cities")
)
(unit-type camp (char "/") (image-name "tents")
  (help "fortified camp")
)
(unit-type tribe(char "X") (image-name "tribe")
; (possible-sides (or "barbarian" "independent")) ; is this a good idea?
  (help "wandering barbarian tribe")
)
(unit-type oppidum(char "V") (image-name "village2")
  (help "village")
)
(unit-type civitas (char "*") (image-name "walltown")
  (help "typical city")
)
(unit-type urbs (char "@") (image-name "parthenon")
  (possible-sides (not "barbarian"))
  (help "large city")
)

(material-type food (help "what everybody has to eat"))
(material-type wood (help "raw materials for building things"))
(material-type stones (help "ammo for siege engines"))
(material-type arrows (help "ammo for archers"))

(define i infantry)
(define c cavalry)
(define a archer)
(define L legion)
(define B bireme)
(define T trireme)
(define U barge)
(define S siege-engine)
(define / camp)
(define X tribe)
(define V oppidum)
(define * civitas)
(define @ urbs)

(include "stdterr")
; more land, more forest
;                                sea sha swa des pla for mou ice
(add cell-t* alt-percentile-min (  0  48  50  50  51  51  90  99))
(add cell-t* alt-percentile-max ( 48  50  51  90  90  90  99 100))
(add cell-t* wet-percentile-min (  0   0  20   0  20  60   0   0))
(add cell-t* wet-percentile-max (100 100 100  20  60 100 100 100))

(define cities (V * @))
(define makers (X V * @))
(define places (/ V * @))
(define troops (i c a L S))
(define ships (B T U))
(define movers (i c a L B T U S X))
(define water (sea shallows))
(define land (swamp plains forest desert mountains))

(table road-chance ((* @) (* @) (1 5) (10 90)))

;;; Static relationships.

(table vanishes-on
  (troops water true)
  (places water true)
  (ships land true)
  (ships road true)
  (u* ice true)
)

;; Unit-unit capacities.

(table unit-size-as-occupant
  (u* u* 500)
;             i c a L B T U  S   X
  (movers u* (1 2 1 4 2 4 6 500 500))
)
(add (B T U / V X * @) capacity (1 2 4 8 8 10 25 100))
(table occupant-max
  (u* u* 100)
  (ships ships 0)
)

;;; Unit-terrain capacities.

;; Limit units to 4 in one cell.
;;  Places cover the entire cell, however.
  
(table unit-size-in-terrain
  (u* t* 4)
  (L t* 8)
  (places t* 16)
)
(add t* capacity 16)

;;; Unit-material capacities.

(table unit-storage-x
  (u* m* 0)
;              i c a  L  B  T  U S  /  X  V  *   @ 
  (u* food   ( 5 8 5 10 10 20 30 5 10 30 30 50 200))
  (u* wood   ( 8 4 4 16 12 16 25 0 20 20 30 50 150))
  (u* stones ( 2 1 1  4  4  6 12 5 10 10 30 50 150))
  (u* arrows ( 2 1 4  4  4  6 10 0 10 15 20 30 100))
)

;; A game's starting units will be full by default.
(table unit-initial-supply (u* m* 9999))

; i c a L B T U S / X V * @ 

(add places already-seen true)
(add X already-seen true)
(add cities see-always true)
(table visibility
  (u* t* 100)
  (movers (mountains forest) 10)
  ((i a) (swamp plains desert) 30)
  ((S X) (mountains forest) 30)
)
;                    i c a L B T U S /  X  V   * @ 
(add u* stack-order (1 3 2 4 5 7 6 8 9 10 11 12 13))

;;; Actions.

;                      i  c  a  L  B  T  U  S / X V * @ 
(add u* acp-per-turn ( 2  8  2  4  4  4  2  1 1 2 1 2 4))
(add movers acp-min  (-2 -8 -2 -4 -4 -4 -3 -3  -2))

;;; Movement.

(add places speed 0)
(add movers speed 100)
(table mp-to-enter-unit
   (u* u* 0)
)

(table mp-to-enter-terrain 
  (u* t* 99)
  (troops land 2)
  (L land 4)
  (c (forest mountains) 8)
  ((S X) land 2)
  (ships water 1)
  (troops road 0)
  (troops river 4)
  (L river 8)
  (c river 16)
  (X river 4)
)
(table mp-to-traverse
  (u* (river road) 99)
  (ships river 2)
  (troops road 1)
  ((L c S X) road 2)
)

;;; Construction.

;           i  c  a  L  B  T  U  S / X V * @ 
(add u* cp (6 15 10 20 16 24 12 10 8 1 1 1 1))
(table acp-to-create
  ((i L) (S /) 1)
  (cities troops 1)
  (cities ships 1)
  (X troops 1)
  ((X V) (L T) 0)
  (X (i a B U) 2)
  (V (c B U) 2)
)
(table cp-on-creation
  ((i L) (S /) 1)
  (cities troops 1)
  (cities ships 1)
)
(table acp-to-build
  ((i L) (S /) 1)
  (cities troops 1)
  (cities ships 1)
  ((X V) (L T) 0)
  (X (i a B U) 2)
  (V (c B U) 2)
)
(table cp-per-build
  ((i L) (S /) 1)
  (cities troops 1)
  (cities ships 1)
)
(table occupant-can-construct
  (u* u* false)
)

(table consumption-on-creation
  ((B T U S /) wood (15 25 15 8 8))
)

;; Automatic repair work.
(add troops hp-recovery 0.25)
(add ships  hp-recovery 0.25)
(add cities hp-recovery 0.50)

;;; Production.

;; should actually get food and wood from terrain
(table base-production
  (troops food 1)
  (c food 2)
  ((/ X V * @) food   (5 5 10 20 40))
  ((/ X V * @) wood   (1 3  2  3  5))
  ((/ X V * @) arrows (1 1  1  2  3)) 
  ((/ X V * @) stones (1 1  1  2  3)) 
)

(table productivity
  (u* t* 0)
  (troops (plains forest) 100)
  (c forest 50)
  (/ land-t* (30 100 50 30))
  (X land-t* (30 100 50 30))
  (V land-t* (30 100 50 30))
  (* land-t* (30 100 50 30))
  (@ land-t* (30 100 50 30))
)

(table base-consumption
  (movers food 1)
  ((c T) food (2 0))
)
(table hp-per-starve
  (movers food 0.5)
)

;;; Combat.

(table acp-to-attack
  (u* u* 1)
  ((a S) u* 0)
)
(table acp-to-defend
  (u* u* 1))
(add (a S) range 1)
(add (a S) acp-to-fire 1)

;               i c a L B T U S /  X  V  *  @ 
(add u* hp-max (2 3 2 8 3 6 2 2 5 20 20 50 200))

(table hit-chance 
   (u* u* 50) ; for now
   (troops ships 20)
   (ships troops 20)
   (S movers 0)
   (U u* 0)
)

(table damage
  (u* u* 1)
  (S (V * @) 4)
)

(table capture-chance
  (i (/ X V * @) (20 15 15 10  5))
  (c (/ X V * @) (20 20 10  5  2))
  (L (/ X V * @) (40 40 40 20 10))
)
(table independent-capture-chance
  (i (/ X V * @) (40 20 30 20 10))
  (c (/ X V * @) (40 30 20 10  5))
  (L (/ X V * @) (70 50 70 35 20))
)

(table ferry-on-entry
   (u* u* over-own)
   (ships movers over-border)
)
(table ferry-on-departure
   (u* u* over-own)
   (ships movers over-border)
)

(table protection 
  (/ movers 80)
  (V movers 65)
  (* movers 50)
  (@ movers 25)
)

(table consumption-per-attack 
  (a arrows 1)
  (S stones 1)
)
(table hit-by
  (u* arrows 1)
  (places stones 1)
  (ships stones 1)
)

(table surrender-range (u* u* 1))
(add u* acp-to-change-side 1)

(add u* point-value 0)
(add makers point-value (1 1 3 9))

(table favored-terrain
  (u* t* 0)
  (u* plains 100)
  (u* forest 20)
  (ships t* 0)
  (ships water 100)
  (X (plains desert forest mountains) (30 20 100 30))
  (V (desert forest mountains) (20 50 30))
  (* (desert forest mountains) (10 30 20))
)
(table independent-density
;                     X  V  *  @
  (makers plains    (20 60 40 10))
  (makers forest    (80 40 20  1))
  (makers mountains (20 20 10  1))
  (makers desert    (10 10  5  1))
)

(add makers start-with 1)

;; Naming.

(include "town-names")
(add places namer "random-town-names")

;; Sides.

(set sides-min 2)

(scorekeeper (do last-side-wins)) ; for now

; the side library really needs work (what about emblems?)
(set side-library '(
  ((name "Marius")
   (class "roman"))
  ((name "Sulla")
   (class "roman"))
  ((name "Parthia") (long-name "Regnum Parthorum") (adjective "Parthian")
   (class "civilized"))
  ((name "Macedonia") (long-name "Regnum Macedoniae") (adjective "Macedonian")
    (class "civilized"))
  ((name "Carthago") (adjective "Punic")
   (class "civilized"))
  ((name "Aedui") (adjective "Aeduan")
   (class "barbarian"))
  ((name "Teutones") (adjective "Teutonian")
   (class "barbarian"))
  ))
