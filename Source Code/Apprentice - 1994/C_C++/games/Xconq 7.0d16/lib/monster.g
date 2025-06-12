;;; Reliving those old movies...

(game-module "monster"
  (blurb "Type defns for Tokyo 1962")
)

(set see-all true)

(unit-type monster (image-name "monster")
  (point-value 100)
  (help "breathes fire and stomps on buildings"))
(unit-type |fire breath| (image-name "breath")
  (help "burns things up"))
(unit-type fire (image-name "fire")
  (help "burns things up"))
(unit-type mob (name "panic-stricken mob") (image-name "horde")
  (help "helpless civilians"))
(unit-type |fire department| (image-name "fireman")
  (help "puts out fires"))
(unit-type |national guard| (image-name "soldiers")
  (help "does battle with the monster"))
(unit-type building (image-name "city20")
  (point-value 1)
  (help "good for hiding, but crushed by monster"))

(define m monster)
(define b |fire breath|)
(define f fire)
(define p mob)
(define F |fire department|)
(define g |national guard|)
(define B building)

(material-type energy (help "required to make fires move"))

(terrain-type sea (color "sky blue") (char "."))
(terrain-type beach (color "yellow") (image-name "desert") (char ","))
(terrain-type street (color "light gray") (image-name "road") (char "+"))
(terrain-type junkheap (color "sienna") (image-name "hills") (char "^"))
(terrain-type fields (color "green") (image-name "plains") (char "="))
(terrain-type trees (color "forest green") (image-name "forest") (char "%"))

(define movers (m b f p F g))
(define water (sea))
(define land (beach street junkheap fields trees))

(define fires (b f))

(add (m b f) possible-sides "monster")
(add (p F g B) possible-sides "human")


(table vanishes-on
  (u* water true)
  (monster water false)
  ;; fire trucks and mobs can only go along the streets
  ((p F) t* true)
  ((p F) street false)
  )

;;; Fires sometimes die down on their own.
(table attrition
  (fires t* 3000)
  (fires street 5000) ; not much fuel in the middle of the street
  (fires junkheap 2000)  ; but junkheaps are good
)

;1 e m produce
;100 t* m productivity
;1 e m storage
;1 e b storage
;1 f b make
;100 t* f productivity

(add movers acp-per-turn (1 1 1 1 2 2))

(table mp-to-enter-terrain
  (u* water 99)
  (monster water 0)
  ((p F) t* 99)
  ((p F) street 0)
  )

(table acp-to-create
  (m b 1)
  (b f 1)
  )

;1 e b to-move

(table unit-capacity-x
  (B (p F g) 1)
  (m b 2)
  (b f 2)
)

(add u* hp-max (100 5 5 1 2 5 6))

(table hit-chance
  (m u* (50 50 50 50 100 80 100))
  (b u* ( 0  0  0 40 40 40 90))
  (f u* ( 0  0  0 40  0 40 90))
  (p u* ( 0 10 10  0  0  0  0))
  (F u* ( 0 90 90  0  0  0  0))
  (g u* (80 40 40  0  0  0  0))
  (B u* (10 20 20  0  0  0  0))
)

(table damage
  (u* u* 1)
  (g m 4)
  (F (b f) (2 5))
  ((m b f) B (3 6 3))
)

;(add u* destroy-message
;  ("kills" "extinguishes" "extinguishes" "massacres" "wipes out" "wipes out" "flattens"))

;[ 05 0 90 50 20 ] movers retreat

;0 p control
;0 f control

;50 [ p F g ] B protect


(add m namer "monster-names")

(namer monster-names (random
  "Godzilla" "Rodan" "Mothra" "Megalon" "Gajira" "Aspidra"
  "Reptilicus" "Gamera"
  ))

;;; Random setup, for testing.

(add t* alt-percentile-min (  0  20  25  35  80  90 ))
(add t* alt-percentile-max ( 20  25  35  80  90 100 ))
(add t* wet-percentile-min 0)
(add t* wet-percentile-max 100)

(set country-radius-min 5)

(add u* start-with (1 0 0 10 3 3 10))

(table favored-terrain
  (u* sea 0)
  )

(game-module (notes (
  "Typically, one would set up a scenario with one or more monsters on"
  "one side, and mobs, fire departments, and national guards on the"
  "other.  Note"
  "that the monster can easily defeat national guards one after another,"
  "and that the most successful strategy for the "human" side is to"
  "attack the monster with several units at once.  The monster can use"
  "fires as a barricade to keep the national guards from getting close"
  "enough to attack.  Destroying buildings is fun but not very useful."
  ""
  "Sandra Loosemore (sandra@cs.utah.edu) is the person to blame for this"
  "piece of silliness (well, Stan aided and abetted)."
)))

