(game-module "old-empire"
  (title "Old Empire")
  (blurb "An emulation of `old Empire', a distant ancestor of Xconq.")
  ;; No variants were available.
)

;;; Types.

(unit-type army (char "A") (image-name "soldiers"))
(unit-type fighter (char "F") (image-name "jets"))
(unit-type destroyer (char "D") (image-name "dd"))
(unit-type submarine (char "S") (image-name "sub"))
(unit-type troop-transport (char "T") (image-name "ap"))
(unit-type cruiser (char "R") (image-name "ca"))
(unit-type carrier (char "C") (image-name "cv"))
(unit-type battleship (char "B") (image-name "bb"))
(unit-type city (char "@") (image-name "city20"))

;;; avgas is used only to constrain fighter range.

(material-type avgas)

(terrain-type sea (color "sky blue") (char "."))
(terrain-type land (color "green") (char "+"))

(define A army)
(define F fighter)
(define D destroyer)
(define S submarine)
(define T troop-transport)
(define R cruiser)
(define C carrier)
(define B battleship)
(define @ city)

(define ship (D S T R C B))

;;; Static relationships.

(table vanishes-on
  ((A @) sea true)
  (ship land true)
  )

;;; Unit-unit capacities.

(table unit-capacity-x
  (T A 6)
  (C F 8)
  )

(add @ capacity 100)

(table unit-size-as-occupant
  (A @ 50)
  (F @ 1)
  (ship @ 1)
  ;; Cities can't occupy each other.
  (@ @ 200)
  )

;;; Unit-terrain capacity is the default of 1 unit, 1 cell.

;;; Unit-material capacities.

(table unit-storage-x
  (F avgas 20)
  ;; Carriers and cities have effectively infinite capacity.
  ((C @) avgas 9999)
  )

;;; Actions.

(add (A F) acp-per-turn (1 4))
(add ship acp-per-turn 2)
(add @ acp-per-turn 1)

;;; Movement.

(add @ speed 0)

(table mp-to-enter-terrain
  (army sea 10)
  (ship land 10)
  )

;4 u* F enter-time

(table consumption-per-move
  (F avgas 1)
  )

;;; Construction.

(add u* cp (5 10 20 30 25 50 60 75 1))

(table acp-to-create
  (@ u* 1)
  (@ @ 0)
  )

(table cp-on-creation
  (@ u* 1)
  )

(table acp-to-build
  (@ u* 1)
  )

(table cp-per-build
  (@ u* 1)
  )

(table acp-to-toolup
  (@ u* 1)
  )

(table tp-to-build
  (@ u* (1 2 4 6 5 10 12 15 0))
  )

(table tp-max
  (@ u* (1 2 4 6 5 10 12 15 0))
  )

;1 ship @ repair

;;; Combat.

(add u* hp-max (1 1 3 3 2 8 8 12 1))

;[ 0 0 2 2 1 4 4  6 0 ] u* crippled

(table hit-chance
  (u* u* 50)
  ;; Cities don't participate in combat.
  (u* @ 0)
  (@ u* 0)
  ;; except vs armies.
  (A @ 50)
  (@ A 50)
  )

(table damage
  (u* u* 1)
  ;; Subs hit ships harder.
  (S ship 3)
  ;; Armies don't damage cities.
  (A @ 0)
  )

(table capture-chance
  (A @ 50)
  )

(table hp-to-garrison
  ;; Use up the capturing army.
  (A @ 1)
  )

;"shoots down" F destroy-message
;"sinks" ship destroy-message
;"runs out of fuel and crashes" F starve-message

;;; Backdrop.

(table base-consumption
  (F avgas 4)
  )

(table base-production
  ((C @) avgas 9999)
  )

(table hp-per-starve
  (F avgas 1.00)
  )

;;; Scoring.

(add u* point-value 0)
(add @ point-value 1)

(scorekeeper (do last-side-wins))

;;; Random setup.

(add t* alt-percentile-min (0 70))
(add t* alt-percentile-max (70 100))
(add t* wet-percentile-min 0)
(add t* wet-percentile-max 100)

(set country-radius-min 3)

(set country-separation-min 15)

(add t* country-terrain-min (2 1))

(add city start-with 1)
(add city independent-near-start 3)

(table favored-terrain
  (city (sea land) (0 100))
  )

(table independent-density
  (city land 200)
  )

(table unit-initial-supply
  (@ avgas 9999)
  )

(game-module (notes (
  "Of course, this will not be exactly like old Empire - for one thing, Xconq maps"
  "are composed of hexes and not squares!  However, we can reproduce the details"
  "of the playing pieces fairly well."
  ""
  "The chief defects are that combat is always mutual, rather than"
  "one-side-at-a-time hits, and that cities must have capacity for armies."
  "In the latter case, this period fixes an annoying defect of the original"
  "empire game!  Also, some of the messages are a little weird."
  ""
  "There is also an imbalance in that ships defeating armies don't run"
  "aground, so they are more useful than in the original."
)))
