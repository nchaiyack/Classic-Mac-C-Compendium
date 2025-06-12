(game-module "stdunit"
  (blurb "Standard unit types and tables")
)

;;; New format version of the standard game.  This file is just types and
;;; tables, is not a complete game (see "standard" for that).

(unit-type infantry (image-name "soldiers")
  (help "marches around and captures things"))
(unit-type armor (image-name "tank")
  (help "faster than infantry, limited to open terrain"))
(unit-type fighter (image-name "fighter")
  (help "interceptor to get those nasty bombers"))
(unit-type bomber (image-name "4e")
  (help "long range aircraft, carries infantry and bombs"))
(unit-type destroyer (image-name "dd")
  (help "fast, cheap, and sinks subs"))
(unit-type submarine (image-name "sub")
  (help "sneaks around and sinks ships"))
(unit-type troop-transport (name "troop transport") (image-name "ap")
  (help "carries infantry and armor across the pond"))
(unit-type carrier (image-name "cv") (char "C")
  (help "carries fighters and bombers around"))
(unit-type battleship (image-name "bb") (char "B")
  (help "the most powerful ship"))
(unit-type nuclear-bomb (name "nuclear bomb") (image-name "bomb") (char "N")
  (help "leveler of cities (and anything else)"))
(unit-type base (image-name "airbase") (char "/")
  (help "airbase plus port"))
(unit-type town (image-name "town20") (char "*")
  (help "smaller than a city"))
(unit-type city (image-name "city20") (char "@")
  (help "capital of a side"))

(define i infantry)
(define a armor)
(define f fighter)
(define b bomber)
(define d destroyer)
(define s submarine)
(define t troop-transport)
(define cv carrier)
(define bb battleship)
(define nuke nuclear-bomb)
(define / base)
(define * town)
(define @ city)

(material-type fuel)
(material-type ammo)

(include "stdterr")

(define makers (* @))
(define ground (i a))
(define aircraft (f b))
(define ship (d s t cv bb))
(define cities (/ * @))
(define movers (i a f b d s t cv bb nuke))
(define water (sea shallows))
(define land (swamp plains forest desert mountains))

;;; Static relationships.

(table vanishes-on
  ((i a) water true)
  (cities water true)
  (ship land true)
  (ship road true)
  (u* ice true)
  (aircraft ice false)
)

;; Unit-unit capacities.

(table unit-capacity-x
  (cv fighter 4)
  )

(table unit-size-as-occupant
  (u* u* 100) ; disables occupancy usually
  ((i nuke) b 1)
  (ground t 1)
  ((f b) cv (1 2))
  (u* / 2)
  (fighter / 1)
  (movers (* @) 1)
  ;; higher sizes for ships? used to be 6 for bbs, etc
  )

(add u* capacity (0 0 0 1 0 0 6 4 0 0 20 40 80))

(table occupant-max
  (u* u* 100)
  )

;;; Unit-terrain capacities.

;; Limit units to 16 in one cell, for the sake of playability and
;; and drawability.  Cities cover the entire cell, however.
  
(table unit-size-in-terrain
  (u* t* 1)
  (cities t* 16)
  )

(add t* capacity 16)

;;; Unit-material capacities.

(table unit-storage-x
  (u* fuel ( 6 10 18 36 100 100 200 400 200 100 200 500 900))
  (u* ammo ( 6  4  3  3  20  10  20  40  40 0 100 200 400))
)

;;; Vision.

; 10 s visibility
; 10 N visibility

(add (* @) see-always true)

(add @ vision-range 3)

;;; Actions.

(add u* acp-per-turn  (1 2 9 6 3 3 2 4 4 1 0 1 1))

;;; Movement.

(add (/ * @) speed 0)

(add i free-mp 1)

(table mp-to-enter-terrain
  ((cv bb) shallows 2)
  (a (swamp forest mountains) 10)
  ((i a) water 99)
  (ship land 99)
  (u* ice 99)
  (aircraft ice 1)
)

(table material-to-move
  (movers fuel 1)
  (nuke fuel 0)
)

(table mp-to-enter-unit
  (u* u* 1)
  ; aircraft can't sortie again until next turn
  (f u* 9)
  (b u* 6)
  (ship u* 0)
  (a (* @) 0)   ; travel quickly on surrounding roads.
)

(table consumption-per-move
  (movers fuel 1)
  ;; Nukes are "important", a country would never risk them running out.
  (nuke fuel 0)
  )

;;; Construction.

;; Nuclear weapons must be researched before they can be built.

(add nuke tech-max 60)
(add nuke tech-to-build 60)

;; Only cities can research nukes.

(table acp-to-research
  (@ nuke 1))

(table tech-per-research
  (@ nuke 1.00))

;; Limit the amount of gain possible due to a concentrated effort.
;; Note that this will only have an effect in games with many cities.

(add nuke tech-per-turn-max 3)

;; Basically, units take about as many turns to complete as their cp.

(add u* cp (4 7 8 16 10 16 12 30 40 20 5 1 1))

(table acp-to-create
  (i / 1)
  ((* @) movers 1)
  )

(table cp-on-creation
  ((i a) / (1 2))
  (* movers 1)
  (@ movers 1)
  )

(table acp-to-build
  (i / 1)
  ((* @) movers 1)
  )

(table cp-per-build
  ((i a) / (1 2))
  (* movers 1)
  (@ movers 1)
  )

(table supply-on-creation (u* m* 9999))

(table occupant-can-construct (u* u* false))

;;; Repair.

;;; Some types have sufficient people and redundancy to do repair work
;;; without affecting their readiness to act.

(add (cv bb /) hp-recovery 0.50)
(add (* @) hp-recovery 1.00)

;;; Explicit repair actions accelerate the recovery of lost hp.

(table acp-to-repair
  ((* @) u* 1)
  (/ u* 1)
  ((cv bb) (cv bb) 1)
  (i cities 1)
)

(table hp-per-repair
  ((* @) u* 1)
  (/ u* 3)
  ((cv bb) (cv bb) 10)
  (i cities (1 10 10))
)

;;; Production.

;50 ship t alter-mobility

(table base-production
  (ground fuel 1)
  (cities fuel (10 20 50))
  (cities ammo (5 10 20))
  (a fuel 2) ; this is less realistic, but problems otherwise
  )

(table productivity
  (i (plains forest) 100)
  (a plains 100)   ; in this case "plains" == "gas stations"
  (/ land (0 100 50 20 20))
  (* land (0 100 50 20 20))
  (@ land (0 100 50 20 20))
)

(table base-consumption
  ((i f b) fuel (1 3 2))  ; note that armor does *not* have a base consumption
  (ship fuel 1)
)

(table hp-per-starve
  ((i f b) fuel 1.00)
  (armor fuel 0.05)
  (ship fuel 0.10)
  (cities fuel 0.05)
  )

; 0 [ f b ] consume-as-occupant

;;; Combat.

(add u* hp-max (1 1 1 2 3 2 3 4 8 1 10 20 40))

;; Units are generally crippled at about 50% of hp-max.

(add movers hp-at-max-speed (1 1 1 2 2 2 2 2 4 1))
(add movers hp-at-min-speed (0 0 0 1 1 1 2 2 1 0))
(add movers speed-min       (0 0 0 3 3 3 2 1 2 1))

;;; The main hit table.

(table hit-chance
  (i u* ( 50  40  20  15  20  20  30  20   9  40  80  60  40))
  (a u* ( 60  50  30  30  30  20  30  20  20  50  90  70  50))
  (f u* ( 15  25  60  70  20  30  20  50  40  80 100 100 100))
  (b u* ( 20  20  10   9  30  50  50  70  60  50  90  95  99))
  (d u* (  5   5  10   5  60  70  60  40  20   0  99  90  80))
  (s u* (  0   0  10   5  40  10  60  40  50   0   0   0   0))
  (t u* ( 20   5  10   5  40  40  40  30   9   0   0   0   0))
 (cv u* ( 30  20  40  10  30  30  40  20  20   0   0   0   0))
 (bb u* ( 50  50  50  20  70  50  90  50  90   0 100 100 100))
  (nuke u* 100) ; doesn't really matter
  (/ u* ( 10  10  20  20  20  20  30  20  20   0   0   0   0))
  (* u* ( 30  20  50  40  40   0  30  20  20   0   0   0   0))
  (@ u* ( 50  40  70  60  50   0  30  20  50   0   0   0   0))
)

(table damage
  (u* u* 1)
  (a cities 2)
  (b ship 2)
  (b s 1)
  (b cities (2 2 3))
  (d s 2)
  (s ship 3)
  (s bb 4)
  (bb u* 2)
  (bb (* @) 3 4)
)

(table capture-chance
  (i cities (70 50 30))
  (a cities (90 70 50))
)

;; Attack and counterattack are completely symmetrical.

;(table counter-strength (u* u* 100))

;; infantry can capture cities even on water.

(table bridge (i cities true))

(table protection ; actually ablation?
  ;; cities offer some protection to occupants
  (cities movers 50)
  ;; but bases make aircraft sitting ducks, so no protection there.
  (base aircraft 100)
  ;; inf and armor protect the cities housing them.
  ;; can't make this too large or city can be
  ;; invulnerable.
  (i cities 80)
  (a cities 60)
  ; bases benefit more from protection.
  ((i a) / (50 40))
)

;; We need ammo to hit with, but no details about types etc.

(table consumption-per-attack (u* ammo 1))
(table hit-by (u* ammo 1))

;(set neutrality -50)

;(add ground destroy-message "defeats")
;(add ship destroy-message "sinks")
;(add aircraft destroy-message "shoots down")
;(add cities destroy-message "flattens")

;;; Detonation.

;; Nukes work by detonation rather than by conventional attack actions.

(add nuke acp-to-detonate 1)

(add nuke hp-per-detonation 1)

(table detonation-damage-at (nuke u* 60))

(table detonation-damage-adjacent (nuke u* 1))

;;; Disbanding.

;; This does everybody in one shot except battleships, which historically
;; are usually difficult to scuttle.

(add movers acp-to-disband 1)  ; should match speed?
(add bb acp-to-disband 2)

(add movers hp-per-disband 4)

;; Takes a while to dismantle a base.

(add / hp-per-disband 2)

;;; Changing sides.

(add u* acp-to-change-side 1)
(add i acp-to-change-side 0)

(add i possible-sides '(not "independent"))

;;; Automatic things.

;; Economy.

(table out-length
  ;; Net consumers of supply should never give any up automatically.
  ((i a b f nuke) m* -1)
  ;; Cities and towns can share things around.
  ((* @) m* 1)
  )

;;; Scoring.

(add u* point-value 0)
(add (i a nuke) point-value 1)
(add (* @) point-value (5 25))

;;; Initial setup.

(add @ start-with 1)
(add * start-with #| independent-near-start |# 5)
(set country-radius-min 3)
(set country-separation-min 16)
(set country-separation-max 48)
; Try to get countries on the coast.
(add (sea plains) country-terrain-min (1 4))

(table favored-terrain
  (u* t* 0)
  (@ plains 100)
  (* land 20)
  (* plains 40)
  )

(table independent-density (town plains 100))

(add land country-people-chance 90)
(add plains country-people-chance 100)

(add land independent-people-chance 50)

(table road-chance (city (town city) (50 100)))

;; Units always start out full.

(table unit-initial-supply (u* m* 9999))

;;; (junk that should be moved)

; help for machine strategy
;[ 200 150 100 100  10   8  40 1 10 0  50 500 500 ] u* attack-worth
;[ 200  80 200 130  20  15   0 0 10 0  50 500 500 ] u* defense-worth
;[ 200  50 100 100  20   0  40 0 10 0 100 500 500 ] u* explore-worth
;[   1  4   0   0  100 200 100 2000 400 0 ] movers min-region-size

(game-module (notes (
  "This game is the most commonly played one in Xconq.  It"
  "represents units of about 1945, from infantry to atomic bombs.  This is"
  "familiar, which makes it easier to play, but also more controversial,"
  "since historians have many conflicting theories about which kinds of"
  "units were most effective."
  ""
  "Opinions differ about optimal strategy for this game.  In general,"
  "blitzkrieg works, and can win the game in a hurry.  The problem is to"
  "muster enough force before striking.  One full troop transport is not"
  "enough; the invasion will melt away like ice cream on a hot sidewalk,"
  "unless"
  "reinforcements (either air or land) show up quickly.  Air cover is very"
  "important.  While building up an invasion force, airborne assaults using"
  "bombers and infantry can provide useful diversions, although it can be"
  "wasteful of bombers.  Human vs human games on a 60x30 world generally"
  "last about 100 turns, usually not enough time or units to build atomic"
  "bombs or battleships, and not a big enough world to really need carriers"
  "(although bases for staging are quite useful.)"
)))

(add i notes '(
  "The infantry army is the slowest of units, but it can go almost"
  "anywhere.  It is also quick to produce.  Infantry is the staple of"
  "campaigns - no special features, but essential to success."
))
(add a notes '(
  "The armor army is highly mobile and hits hard.  Unfortunately,"
  "it is limited to operating in open terrain - plains and desert.  It also"
  "takes longer to produce.  Armor can last twice as long in the  "
  "desert as infantry.  Both armor and infantry can"
  "assault and capture cities; they are the only units that can do so."
))
(add f notes '(
  "A fighter is a squadron or wing of high-speed armed aircraft."
  "Their fuel supply can be gotten only at units, towns, and bases, so they"
  "must continually be taking off and landing.  Fighters are not too effective"
  "against ground units or ships, but they eat bombers for lunch.  Fighters"
  "are very good for reconnaisance - important when you can't always"
  "see the enemy moving!"
))
(add b notes '(
  "Bombers are very powerful, since they can seriously damage"
  "or even flatten cities.  Loss rate in such activities is high, so they're"
  "not a shortcut to victory!"
  "Bombers can carry one infantry, which is very useful for raids."
))
(add d notes '(
  "Destroyers are fast small ships for both exploration and"
  "anti-submarine activities."
))
(add s notes '(
  "The favorite food of submarines is of course merchant shipping"
  "and troopships, and they can sink troop transports with one blow."
  "Subs are also invisible, but vulnerable to destroyers and aircraft."
))
(add t notes '(
  "This is how ground units get across the sea.  They can"
  "defend themselves against ships and aircraft, but are basically vulnerable."
  "They're not very fast either."
))
(add cv notes '(
  "Compensates for the fighter's limited range by providing"
  "a portable airport.  Carriers themselves are sitting ducks, particularly"
  "with respect to aircraft.  Fighter patrols are mandatory."
))
(add bb notes '(
  "The aptly named `Dread Naught' has little to fear from other"
  "units of this period.  Subs may sink them with enough effort, and a group"
  "of bombers and fighters are also deadly,"
  "but with eight hit points to start,"
  "a battleship can usually survive long enough to escape."
  "Battleships are very"
  "effective against cities and armies, at least the ones on the coast."
))
(add nuke notes '(
  "Atomic bombs.  The Final Solution; but they are not easy to use.  A bomb"
  "takes a long time to produce, moves very slowly by itself, and is easily"
  "destroyed by other units. The plus side is instant destruction for any unit"
  "of any size!  Bombs are imagined to be transported by a team of scientists,"
  "and can go on any sort of terrain without running out of supplies."
))
(add / notes '(
  "To simplify matters, this can serve as a camp, airbase, and port."
  "Bases cannot build units, although they can repair some damage."
))
(add * notes '(
  "Towns are the staple of territory.  They can build, repair, produce"
  "fuel and ammo, and serve as a safe haven for quite a few units."
))
(add @ notes '(
  "Cities are very large, powerful, and well defended.  They are"
  "basically capital cities, or something in a comparable range.  (New York"
  "and San Francisco are cities, Salt Lake City and San Antonio are towns."
  "Yeah, San Antonio has a lot of people, but it's still insignificant,"
  "nyah nyah.)  A city is worth five towns, point-wise."
))
