(game-module "ww2-bn"
  (blurb "Types for battalion-level WW2 Europe")
  (variants (see-all true))
  )

;;; Non-motorized infantry.

(unit-type inf (long-name "infantry") (image-name "infantry"))
(unit-type inf-bicycle (long-name "bicycle infantry"))
(unit-type inf-para (long-name "parachute infantry") (possible-sides "allied"))
(unit-type inf-glider (long-name "glider infantry") (possible-sides "allied"))
(unit-type inf-sec (long-name "security company") (possible-sides "allied"))
(unit-type inf-ranger (long-name "rangers") (possible-sides "allied"))
(unit-type inf-cmdo (long-name "commando") (possible-sides "allied"))
(unit-type inf-fusil (long-name "fusilier") (possible-sides "german"))
(unit-type inf-ost (long-name "ost") (possible-sides "german"))
(unit-type inf-pion (long-name "pioniere") (possible-sides "german"))

(define non-mot-inf-bn-types
  (inf inf-bicycle inf-para inf-glider inf-ranger inf-cmdo inf-fusil inf-ost inf-pion))

(define non-mot-inf-co-types (inf-sec))

(define non-mot-inf-types (append non-mot-inf-bn-types non-mot-inf-co-types))

(add non-mot-inf-types image-name "inf-bn")

(add non-mot-inf-types acp-per-turn 8)
(add inf-sec acp-per-turn 4)
(add (inf-bicycle inf-ranger inf-cmdo) acp-per-turn 12)

(define allied-inf-types (inf-para inf-glider inf-sec inf-ranger inf-cmdo))

;;; Motorized infantry.

(unit-type inf-mot (long-name "motorized infantry"))
(unit-type inf-heavy (long-name "heavy weapon infantry"))
(unit-type inf-mg-co (long-name "machinegun company"))
(unit-type inf-mot-sec (long-name "motorized security"))
(unit-type inf-recon (long-name "recon") (possible-sides "allied"))
(unit-type inf-fsj (long-name "Fallschirmjaeger") (possible-sides "german"))
(unit-type inf-mot-pion (long-name "Pioniere") (possible-sides "german"))
(unit-type inf-mech-cav (long-name "mechanized cavalry"))

(define mot-inf-bn-types (inf-mot inf-fsj inf-recon))

(define mot-inf-co-types
  (inf-heavy inf-mg-co inf-mot-sec inf-recon inf-mot-pion inf-mech-cav))

(define mot-inf-types (append mot-inf-bn-types mot-inf-co-types))

(add mot-inf-types image-name "infantry-mot") ; for now

(add mot-inf-types acp-per-turn 16)
(add inf-mg-co acp-per-turn 8)

;;; Tracked infantry.

(unit-type inf-armor (long-name "armored infantry"))
(unit-type inf-pz-pion (long-name "Panzer pioniere") (possible-sides "german"))
(unit-type inf-armor-recon (long-name "armored recon") (possible-sides "german"))
(unit-type inf-lw-recon (long-name "Luftwaffe recon") (possible-sides "german"))

(define tracked-inf-types (inf-armor inf-pz-pion inf-armor-recon inf-lw-recon))

(add tracked-inf-types image-name "inf-bn-tracked")

(add (inf-armor inf-pz-pion) acp-per-turn 12)
(add (inf-armor-recon inf-lw-recon) acp-per-turn 16)

(define inf-co-types (append non-mot-inf-co-types mot-inf-co-types inf-lw-recon))

;;; All infantry types together.

(define inf-types (append non-mot-inf-types mot-inf-types tracked-inf-types))

(add inf-types hp-max 12)
(add inf-co-types hp-max 3)

;;; Artillery types.

(unit-type arty-cd-cased (long-name "casemated coastal artillery"))
(unit-type arty-cd-open (long-name "open coastal artillery"))
(unit-type arty-cd-rr (long-name "railroad coastal artillery"))
(unit-type arty-horse (long-name "horse-drawn artillery battalion"))
(unit-type arty-horse-bty (long-name "horse-drawn artillery battery"))
(unit-type arty-para (long-name "parachute artillery"))
(unit-type arty-glider (long-name "glider artillery"))
(unit-type arty-rr (long-name "railroad artillery"))

(define german-non-mot-arty-types
  (arty-cd-cased arty-cd-open arty-cd-rr))

(add german-non-mot-arty-types possible-sides "german")

(add german-non-mot-arty-types acp-per-turn 2)

(add german-non-mot-arty-types speed 0)

(add german-non-mot-arty-types acp-to-fire 1)

(add german-non-mot-arty-types range (9 8 15))

(add arty-cd-rr range-min 7)

(define allied-non-mot-arty-types
  (arty-para arty-glider))

(add allied-non-mot-arty-types possible-sides "allied")

(add allied-non-mot-arty-types acp-per-turn 16)

(add allied-non-mot-arty-types acp-to-fire 8)

(add allied-non-mot-arty-types range (6 4))

(define horse-arty-types
  (arty-horse arty-horse-bty))
 
(add horse-arty-types acp-per-turn 8)

(add horse-arty-types acp-to-fire 4)

(add horse-arty-types range (6 10))

(define non-mot-arty-types
  (append german-non-mot-arty-types allied-non-mot-arty-types
          horse-arty-types arty-rr))

(add non-mot-arty-types image-name "field-gun")

(add arty-cd-cased image-name "arty-cd-cased")

(unit-type arty-group (long-name "artillery group") (possible-sides "allied"))
(unit-type arty-hq (long-name "HQ/artillery") (possible-sides "allied"))
(unit-type arty-towed (long-name "towed artillery"))
(unit-type flak (possible-sides "german"))
(unit-type flak-bty (long-name "Flak battery") (possible-sides "german"))
(unit-type at (long-name "anti-tank battalion"))
(unit-type nebel (long-name "Nebelwerfer battalion") (possible-sides "german"))
(unit-type mortar)
(unit-type mortar4.2 (possible-sides "allied"))
(unit-type fortress-bn (long-name "fortress battalion"))

(define mot-arty-types
  (arty-group arty-hq arty-towed flak flak-bty
   at nebel mortar mortar4.2 fortress-bn))

(add mot-arty-types image-name "arty-bn-mot")

(add (mortar mortar4.2) image-name "mortar")

(add (flak flak-bty) image-name "flak")

(add mot-arty-types acp-per-turn 16)

(add mot-arty-types acp-to-fire 8)
(add mot-arty-types range (6 7 6 1 1 1 4 3 2 2))

(unit-type nebel-bty (long-name "Nebelwerfer battery") (possible-sides "german"))
(unit-type arty-armored (long-name "armored artillery"))
(unit-type arty-hq-armored (long-name "HQ/artillery, tracked") (possible-sides "allied"))
(unit-type sp-at (long-name "self-propelled anti-tank battalion"))
(unit-type sp-at-co (long-name "self-propelled anti-tank company"))

(define tracked-arty-types (nebel-bty arty-armored arty-hq-armored sp-at sp-at-co))

(add tracked-arty-types image-name "armor")

(add tracked-arty-types acp-per-turn 12)

(add tracked-arty-types acp-to-fire 6)
(add tracked-arty-types range (6 3 6 1 1))

(define arty-types (append non-mot-arty-types mot-arty-types tracked-arty-types))

(add arty-types hp-max 12)

;;; Armor types.

(unit-type tank (long-name "tank battalion"))
(unit-type ag (long-name "assault gun battalion"))
(unit-type tank-co (long-name "tank company"))
(unit-type ag-co (long-name "assault gun company"))
(unit-type croc-co (long-name "crocodile company"))

(define armor-bn-types (tank ag))

(define armor-co-types (tank-co ag-co croc-co))

(define armor-types (append armor-bn-types armor-co-types))

(add armor-bn-types image-name "armor-bn")
(add armor-co-types image-name "armor-co")

(add armor-types acp-per-turn 12)
(add croc-co acp-per-turn 8)

(add armor-types hp-max 12)

;; Headquarters types.

(unit-type hq-nebel-brg (long-name "nebelwerfer brigade HQ"))
(unit-type hq-flak-reg (long-name "sturmflak regiment HQ"))

(unit-type hq-inf-div (long-name "inf div HQ"))
(unit-type hq-mot-div (long-name "mot inf div HQ"))
(unit-type hq-air-div (long-name "air div HQ"))
(unit-type hq-gren-div (long-name "panzergrenadier div HQ"))
(unit-type hq-armor-div (long-name "armor div HQ"))

(unit-type hq-flak-korps (long-name "sturmflak korps HQ"))
(unit-type hq-pz-korps (long-name "panzer korps HQ"))
(unit-type hq-fsj-korps (long-name "fallschirmjaeger korps HQ"))

(unit-type hq-inf-corps (long-name "infantry corps HQ"))

(define german-hq-types
  (hq-inf-div hq-nebel-brg hq-flak-reg hq-gren-div
   hq-flak-korps hq-pz-korps hq-fsj-korps))

(add german-hq-types acp-per-turn (12 16 16 12 16 16 16))

(add german-hq-types possible-sides "german")

(define allied-hq-types
  (hq-mot-div hq-air-div))

(add allied-hq-types possible-sides "allied")

(add allied-hq-types acp-per-turn (16 8))

(define generic-hq-types
  (hq-armor-div hq-inf-corps))

(add generic-hq-types acp-per-turn (12 16))

(define hq-types (append german-hq-types allied-hq-types generic-hq-types))

(add hq-types image-name "inf-div-hq")
(add (hq-flak-korps hq-pz-korps hq-fsj-korps) image-name "inf-corps-hq")
(add hq-inf-corps image-name "inf-corps-hq")

(add hq-types hp-max 2)

(define ground-unit-types (append inf-types arty-types armor-types hq-types))

(define mot-types (append mot-inf-types mot-arty-types hq-types))

(define tracked-types (append tracked-inf-types tracked-arty-types armor-types))

(define veh-types (append mot-types tracked-types))

;;; Trains.

(unit-type train (possible-sides "german"))

;;; Ships.

(unit-type transport (image-name "ap"))
(unit-type destroyer (image-name "dd"))
(unit-type cruiser (image-name "ca"))
(unit-type battleship (image-name "bb"))

(define ship-types (transport destroyer cruiser battleship))

(add ship-types possible-sides "allied")

;(add ship-types acp-per-turn 4)
;(add ship-types speed 99.00)
(add ship-types acp-per-turn 200)

(add (destroyer cruiser battleship) acp-to-fire 2)
(add (destroyer cruiser battleship) range (6 8 12))

(add ship-types hp-max (6 6 12 18))

;;; Aircraft.

(unit-type fighter-bomber (image-name "1e"))
(unit-type light-bomber (image-name "4e"))
(unit-type medium-bomber (image-name "4e"))
(unit-type heavy-bomber (image-name "4e"))

(define aircraft (fighter-bomber light-bomber medium-bomber heavy-bomber))

(add aircraft possible-sides "allied")

(add aircraft acp-per-turn 4)
(add aircraft speed 99.00)

;;; Places.

(unit-type fort (image-name "fortress"))
(unit-type fortification (image-name "walltown"))
(unit-type coast-strongpoint (image-name "fortress"))

(define fort-types (fort fortification coast-strongpoint))

(add fort-types hp-max 8)

;;; Supply-related units.

(unit-type mulberry (possible-sides "allied"))
(unit-type port)
(unit-type supply-depot)

(define supply-types (mulberry port supply-depot))

(unit-type village (image-name "town20"))
(unit-type town (image-name "city18"))

(define town-types (village town))

(add town-types see-always true)

(add town-types hp-max (24 48))

(define place-types (append fort-types supply-types town-types))

;;; Materials

(material-type fuel)

;;; Terrain and its properties.

(terrain-type sea (color "dodgerblue") (char ".")
  (occurrence 10))
(terrain-type clear (color "white") (char "+")
  (occurrence 60))
(terrain-type bocage (color "green") (char "#")
  (occurrence 20))
(terrain-type hill (color "sienna") (image-name "mountains") (char "^")
  (occurrence 1))
(terrain-type forest (color "forest green") (char "%")
  (occurrence 8))
(terrain-type swamp (color "yellow green")
  )
(terrain-type flooded (color "light blue")
  )
(terrain-type city (color "gray") (image-name "urban") (char "*")
  (occurrence 1))
(terrain-type river (color "blue")
  (subtype border))
(terrain-type road (color "gray")
  (subtype connection) (subtype-x road-x))
(terrain-type sec-road (color "gray") (image-name "road")
  (subtype connection))
(terrain-type railroad (color "black")
  (subtype connection))

(define land (clear bocage hill forest swamp flooded city))

(define roads (road sec-road))

(add t* elevation-min 0)
(add t* elevation-max 100)
(add (sea swamp) elevation-max 0) 

;;; Static relationships.

(table vanishes-on
  (ground-unit-types sea true)
  (train t* true)
  (train railroad false)
  (ship-types land true)
  (place-types sea true)
  ;; ...except for mulberries.
  (mulberry sea false)
  )

;; A cell has a capacity for basically six companies or two battalions.

(add t* capacity 6)

(table unit-size-in-terrain
  (inf-types t* 3)
  (arty-types t* 1)
  (sp-at t* 3) ; only battalion-sized arty type
  (armor-co-types t* 1)
  (armor-bn-types t* 2)
  (hq-types t* 1)
  ;; A port takes up no additional space.
  (port t* 0)
  ;; A town fills up the cell.
  (town t* 6)
  )

(add transport capacity 9)
(add fort-types capacity 12)
(add town-types capacity 12)

(table unit-size-as-occupant
  (u* u* 100)
  (inf-types transport 3)
  (arty-types transport 1)
  (sp-at transport 3) ; only battalion-sized arty type
  (armor-co-types transport 1)
  (armor-bn-types transport 2)
  (hq-types transport 1)
  ;; Stacking in towns is similar to stacking in terrain.
  (inf-types fort-types 3)
  (arty-types fort-types 1)
  (sp-at fort-types 3) ; only battalion-sized arty type
  (armor-co-types fort-types 1)
  (armor-bn-types fort-types 2)
  (hq-types fort-types 1)
  (inf-types town-types 3)
  (arty-types town-types 1)
  (sp-at town-types 3) ; only battalion-sized arty type
  (armor-co-types town-types 1)
  (armor-bn-types town-types 2)
  (hq-types town-types 1)
  )

(table unit-capacity-x
  (port transport 6)
  )

;;; Action parameters.

;; (Acp is defined with each subset of types.)

;;; Movement parameters.

(table mp-to-leave-terrain
  (u* t* 1)
  (u* swamp 2)
  (u* flooded 2)
  (veh-types t* 2)
  (veh-types city 1)
  ;; Standing water is a problem for vehicles
  (veh-types swamp 4)
  (veh-types flooded 3)
  ;; Advantage of roads
  (veh-types roads 0)
  )

(table mp-to-enter-terrain
  (u* t* 1)
  (u* swamp 2)
  (u* flooded 2)
  (veh-types t* 2)
  (veh-types city 1)
  ;; Standing water is a problem for vehicles
  (veh-types swamp 4)
  (veh-types flooded 3)
  ;; Extra cost to cross rivers
  (u* river 1)
  (veh-types river 4)
  (tracked-types river 3)
  ;; Advantage of roads
  (veh-types roads 0)
  ;; Accident prevention
  (ground-unit-types sea 999)
  (ship-types land 999)
  )

(table mp-to-traverse
  (veh-types roads 1)
  )

;;; Combat parameters.

;;; (Hp and range is defined with each set of types.)

(table acp-to-attack
  (u* u* 2)
  (arty-types inf-types 0)
  (arty-types armor-types 0)
  (arty-types fort-types 0)
  (arty-types town-types 0)
  (transport u* 0)
  )

(table hit-chance
  (u* u* 50)
  (hq-types u* 0)
  (u* hq-types 25)
  (u* fort-types 5)
  (u* town-types 100)
  (transport u* 0)
  )

(table damage
  (u* u* 1d5)
  (hq-types u* 0)
  (u* fort-types 1)
  (u* town-types 1)
  (transport u* 0)
  )

(table retreat-chance
  (inf-types inf-types 10)
  (armor-types inf-types 20)
  (inf-types arty-types 20)
  (armor-types arty-types 40)
  (u* hq-types 50)
  )

;;; Capture.

(table capture-chance
  ;; This is the chance to capture a mostly-abandoned fort.
  (inf-types fort-types 50)
  (armor-types fort-types 50)
  ;; Towns never offer any sort of resistance.
  (u* town-types 100)
  )

(table protection
  (arty-types fort-types 50)
  (inf-types fort-types 50)
  (armor-types fort-types 80)
  )

;; Infantry bounces back if given a chance to rest.

(add inf-types hp-recovery 0.50)

;; Damage to forts is hard to repair.

(add fort-types hp-recovery 0.10)

;;; The Germans may want to destroy ports etc.

(add port acp-to-disband 1)

;;; Random game synthesis (for testing mostly, will usually use fixed map and units).

(set synthesis-methods
  '(make-random-terrain make-countries make-independent-units make-roads))

;; need a French name generator for villages.
;(add village independent-density 1000)
;; (or set all villages to be German?)

(set country-radius-min 8)

(add sea country-terrain-min 4)

(add sea country-terrain-max 10)

;; One of everything.
(add u* start-with 1)
;(add (inf tank town) start-with 1)

(table favored-terrain
  (u* t* 100)
  (u* sea 0)
  (ship-types t* 0)
  (ship-types sea 100)
)

(table road-chance
  (town (village town) (50 100))
  (village village 20)
  )

(table road-into-chance
  (land land 100))

;;; Scenario fillins.

;;; Entire Normandy area is known.

;;; Getting the dates right.

(set calendar '(usual "day"))

(set initial-date "6 Jun 1944")

(world 360 (day-length 1))

;;; The usual sides.

(set side-library '(
  ((name "Allies") (adjective "Allied") (class "allied")
   (color-scheme "blue") (emblem-name "white-star"))
  ((name "Germany") (adjective "German") (class "german")
   (color-scheme "black") (emblem-name "german-cross"))
  ))

(game-module (notes (
  "This is an extremely detailed game about the D-Day invasion of Normandy "
  "and the months of slugfest that followed."
  )))

(game-module (design-notes (
  "The basic map scale is 2km/hex, 1 day/turn."
  )))

