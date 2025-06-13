(game-module "empire"
  (title "Empire")
  (blurb "The classic big economic/military game")
  (variants
   (see-all true)
   ("Only One Winner" wipeout true (true (scorekeeper (do last-side-wins))))
   ("More Starters" more-starters true
     (true
      (add harbor start-with 1)
      )
     )
   )
  )

(unit-type infantry (image-name "infantry"))
(unit-type motor-infantry (image-name "infantry"))
(unit-type mech-infantry (image-name "infantry"))
(unit-type marines (image-name "infantry"))
(unit-type security (image-name "infantry"))
(unit-type cavalry (image-name "cavalry"))
(unit-type light-armor (image-name "tank"))
(unit-type armor (image-name "tank"))
(unit-type heavy-armor (image-name "tank"))
(unit-type artillery (image-name "artillery"))
(unit-type light-artillery (image-name "artillery"))
(unit-type heavy-artillery (image-name "artillery"))
(unit-type mech-artillery (image-name "artillery"))
(unit-type aa (image-name "flak"))
(unit-type supply (image-name "truck"))
(unit-type engineers (image-name "engineers"))
(unit-type mech-engineers (image-name "engineers"))
(unit-type mobile-radar (image-name "radar"))

(define infantry-types (infantry motor-infantry mech-infantry marines security))

(define armor-types (light-armor armor heavy-armor))

(define artillery-types (artillery light-artillery heavy-artillery mech-artillery aa))

(define ground-types
  (append infantry-types cavalry armor-types artillery-types
   supply engineers mech-engineers mobile-radar
   ))

(add ground-types acp-per-turn 2)

(unit-type fishing-boat (image-name "ap"))

(unit-type cargo-ship (image-name "ap"))
(unit-type tanker (image-name "ap"))
(unit-type ore-ship (image-name "ap"))
(unit-type slave-ship (image-name "ap"))

(unit-type landing-craft (image-name "dd"))

(unit-type torpedo-boat (image-name "dd"))
(unit-type destroyer (image-name "dd"))
(unit-type frigate (image-name "dd"))
(unit-type light-cruiser (image-name "ca"))
(unit-type heavy-cruiser (image-name "ca"))
(unit-type battleship (image-name "bb"))

(unit-type light-carrier (image-name "cv"))
(unit-type escort-carrier (image-name "cv"))
(unit-type carrier (image-name "cv"))
(unit-type missile-frigate (image-name "dd"))
(unit-type missile-cruiser (image-name "ca"))

(unit-type submarine (image-name "sub"))
(unit-type asw-cruiser (image-name "ca"))
(unit-type minesweeper (image-name "ap"))

(define production-ship-types (fishing-boat))

(define material-transport-ship-types (cargo-ship tanker ore-ship slave-ship))

(define transport-ship-types (landing-craft))

(define surface-combat-ship-types
  (torpedo-boat destroyer frigate light-cruiser heavy-cruiser battleship))

(define air-combat-ship-types
  (light-carrier escort-carrier carrier missile-frigate missile-cruiser))

(define ship-types
  (append production-ship-types material-transport-ship-types
   transport-ship-types surface-combat-ship-types air-combat-ship-types
   submarine asw-cruiser minesweeper
   ))

(add ship-types acp-per-turn 4)

(unit-type fighter-1 (image-name "fighter"))
(unit-type fighter-2 (image-name "fighter"))
(unit-type escort (image-name "fighter"))
(unit-type naval-fighter (image-name "fighter"))
(unit-type light-bomber (image-name "4e"))
(unit-type medium-bomber (image-name "4e"))
(unit-type heavy-bomber (image-name "4e"))
(unit-type transport (image-name "4e"))
(unit-type attack-helicopter (image-name "helicopter"))
(unit-type transport-helicopter (image-name "cargo-chopper"))
(unit-type recon-plane (image-name "4e"))
(unit-type asw-plane (image-name "4e"))
(unit-type missile (image-name "missile"))
(unit-type icbm (image-name "icbm"))
(unit-type sam (image-name "missile"))
(unit-type abm (image-name "missile"))

(define fighter-plane-types (fighter-1 fighter-2 escort naval-fighter))

(define missile-types (missile icbm sam abm))

(define plane-types
  (append fighter-plane-types
   light-bomber medium-bomber heavy-bomber
   transport
   attack-helicopter transport-helicopter
   recon-plane asw-plane
   missile-types
   ))

(add plane-types acp-per-turn 8)

(add missile-types acp-per-turn 1)

(unit-type small-nuke (image-name "bomb"))
(unit-type medium-nuke (image-name "bomb"))
(unit-type large-nuke (image-name "bomb"))

(define nuke-types (small-nuke medium-nuke large-nuke))

(add nuke-types acp-per-turn 1)

;; The remaining types are actually "sector types" in Empire, but
;; they work better as cell-filling unit types in Xconq.

(unit-type bridge (image-name "bridge")
  (help "allows ground units to be over water"))
(unit-type radar (image-name "radar") (char ")")
  (help "fixed radar station - has a long-range view"))
(unit-type mines (image-name "minefield")
  (help "useful for blocking land and sea passages"))

(define inert-types (bridge radar mines))

(add inert-types acp-per-turn 0)

(unit-type agribusiness (image-name "farm") (char "a"))
(unit-type oil-field (image-name "oil-derrick") (char "o"))
(unit-type oil-platform (image-name "oil-derrick"))
(unit-type mine (image-name "iron-mine") (char "m"))
(unit-type gold-mine (image-name "gold-mine") (char "g"))
(unit-type uranium-mine (image-name "uranium-mine") (char "u"))

(unit-type technical-center (image-name "facility") (char "t"))
(unit-type fortress  (image-name "facility") (char "f"))
(unit-type research-lab (image-name "facility") (char "r"))
(unit-type nuclear-plant (image-name "facility") (char "n"))
(unit-type library/school (image-name "facility") (char "l"))
(unit-type enlistment (image-name "facility") (char "e"))
(unit-type headquarters (image-name "facility") (char "!"))

(unit-type harbor (image-name "port") (char "h"))
(unit-type airfield (image-name "airbase") (char "*"))
(unit-type refinery (image-name "facility") (char "%"))
(unit-type lcm-factory (image-name "facility") (char "j"))
(unit-type hcm-factory (image-name "facility") (char "k"))
(unit-type defense-plant (image-name "facility") (char "d"))
(unit-type shell-industry (image-name "facility") (char "i"))
(unit-type warehouse (image-name "facility") (char "w"))

(unit-type bank (image-name "bank") (char "b"))

(define facility-types
  (agribusiness oil-field oil-platform mine gold-mine uranium-mine
   bank
   technical-center fortress research-lab nuclear-plant library/school enlistment headquarters
   harbor airfield refinery lcm-factory hcm-factory defense-plant shell-industry warehouse))

(add facility-types acp-per-turn 1)

(unit-type capital (image-name "city20") (char "c")
  (help "center of the country"))

(add capital acp-per-turn 3)

;; Hit points are like effectiveness.

(add u* hp-max 100)

(material-type food (notes #|long-name|# "food"))
(material-type sh (notes #|long-name|# "shell"))
(material-type gun (notes #|long-name|# "guns"))
(material-type pet (notes #|long-name|# "petroleum"))
(material-type iron (notes #|long-name|# "iron ore"))
(material-type dust (notes #|long-name|# "gold dust"))
(material-type oil (notes #|long-name|# "crude oil"))
(material-type lcm (notes #|long-name|# "light construction materials"))
(material-type hcm (notes #|long-name|# "heavy construction materials"))
(material-type rad (notes #|long-name|# "rads"))
; also education and happiness here?
(material-type civ (notes #|long-name|# "civilians"))
(material-type mil (notes #|long-name|# "military"))
(material-type uw (notes #|long-name|# "uncompensated workers")
  (help "slaves, really"))
(material-type gold (notes #|long-name|# "gold"))

(define raw (iron dust oil rad))
(define manufactures (food sh gun pet lcm hcm gold))
(define peoples (civ mil uw))

(add peoples people 1)

(terrain-type sea (char "."))
(terrain-type settled (image-name "clear") (char "-"))
(terrain-type wilderness (image-name "forest") (char "-"))
(terrain-type mountains (char "^"))
(terrain-type wasteland (char "/")
  (help "uninhabitable due to radioactivity"))
(terrain-type highway (image-name "road") (char "+")
  (subtype connection))

(define land (settled wilderness mountains wasteland highway))

;;; Static relationships.

;; Unit vs unit.

(table unit-capacity-x
  ;; Engineers can get inside most anything.
  (facility-types (engineers mech-engineers) 1)
  (headquarters ground-types 8)
  (harbor ship-types 8)
  (airfield plane-types 8)
  (capital ground-types 2)
  (capital ship-types 8)
  (capital plane-types 8)
  )

;; Unit vs terrain.

(table vanishes-on
  (ground-types sea true)
  (ship-types land true)
  (facility-types sea true)
  (capital sea true)
  ;; Bridges and oil platforms can only be placed in sea cells.
  ((bridge oil-platform) sea false)
  ((bridge oil-platform) land true)
  ;; Units normally die in wasteland.
  (u* wasteland true)
  ;; Planes can fly over wasteland safely.
  (plane-types wasteland false)
)

(add t* capacity 16)

(table unit-size-in-terrain
  (u* t* 1)
  (facility-types t* 16)
  (capital t* 16)
  )

(table terrain-storage-x
  ;; Room for raw materials
  (t* oil 100)
  (t* iron 100)
  (t* dust 10)
  (t* rad 10)
  ;; Room for peoples
  (settled peoples 999)
  )

(table unit-storage-x
  (ground-types food 10)
  (artillery-types sh 100)
  (ground-types mil 1)
  (ship-types food 100)
  ;; Special-function ships.
  (fishing-boat food 200)
  (tanker oil 100)
  (ore-ship iron 100)
  (cargo-ship (sh gun lcm hcm) 100)
  (facility-types food 30)
  (facility-types civ 10)
  ;; Special-function facilities
  (agribusiness food 150)
  (ground-types pet 10)
  (ship-types pet 100)
  ;; Warehouses have lots of room, but only for manufactured goods.
  (warehouse manufactures 999)
  ;; Everything centers on the capital.
  (capital m* 999)
  )

;;; (really need generic capacities for cargo ships)

;;; Actions.

(table material-to-act
  ;; Ground units need soldiers to run them.
  (ground-types mil 1)
  ;; Facilities all need civilians to run them.
  (facility-types civ 1)
  )

;;; Movement.

(add missile-types speed 30.00)

(add inert-types speed 0)
(add facility-types speed 0)
(add capital speed 0)

;;; Construction.

(add u* cp 6)

(table acp-to-create
  (engineers bridge 1)
  (engineers facility-types 1)
  (headquarters ground-types 1)
  (harbor ship-types 1)
  (airfield plane-types 1)
  (capital engineers 1)
  )

(table cp-on-creation
  (engineers bridge 1)
  (engineers facility-types 1)
  (headquarters ground-types 1)
  (harbor ship-types 1)
  (airfield plane-types 1)
  (capital engineers 1)
  )

(table acp-to-build
  (engineers bridge 1)
  (engineers facility-types 1)
  (headquarters ground-types 1)
  (harbor ship-types 1)
  (airfield plane-types 1)
  (capital engineers 1)
  )

(table cp-per-build
  (engineers bridge 1)
  (engineers facility-types 1)
  (headquarters ground-types 1)
  (harbor ship-types 1)
  (airfield plane-types 1)
  (capital engineers 1)
  )

;; need pet/lcm/hcm to build things also.

;;; Production (and consumption).

(table base-production
  ((agribusiness fishing-boat capital) food (100 50 20))
  ((oil-field oil-platform) oil 1)  ;  should be extraction really
  ((enlistment capital) mil 1)
  (capital civ 1)
  (refinery pet 1)
  )

(table base-consumption
  ;; Everything eats food.
  (u* food 1)
  (agribusiness food 0) ; a hack
  ;; Capital is self-supporting, but has no excess.
  (capital food 20)
  ;; Most hardware eats petroleum products.
  (ship-types pet 1)
  (plane-types pet 1)
  ;; Inert types don't consume anything at all.
  (inert-types m* 0)
  )

(table hp-per-starve
  ;; Going without food is potentially deadly.
  (u* food 1.00)
  ;; Technology types really need their petroleum products.
  (ship-types pet 1.00)
  (plane-types pet 1.00)
  )

(table out-length
  ;; Farming has a default supply infrastructure.
  (agribusiness food 5)
  ;; Fishing fleets must rely on land systems to redistribute.
  (fishing-boat food 1)
  ;; The oil business' support machinery can get the oil sent around.
  (oil-field oil 5)
  (oil-platform oil 5)
  ;; Civilians will tend to go where they're needed (looking for jobs).
  (facility-types civ 2)
  ;; Warehouses are redistribution points.
  (warehouse m* 5)
  ;; Capital will be generous with nearby needy units.
  (capital m* 2)
  )

(table in-length
  ;; Every type of unit that needs food can get it from an adjacent cell.
  (u* food 1)
  ;; Civilians will tend to go where they're needed (looking for jobs).
  (facility-types civ 2)
  ;; Warehouses are redistribution points.
  (warehouse m* 5)
  ;; The capital always gets special service if it needs something.
  (capital m* 5)
  )

;;; Combat.

(table hit-chance
  (infantry-types facility-types 100)
  (infantry-types fortress 50)
  (infantry-types capital 100)
  )

(table damage
  (infantry-types facility-types 1)
  (infantry-types capital 1)
  )

(table capture-chance
  ;; Most facilities can't prevent their own capture.
  (infantry-types facility-types 100)
  ;; Fortresses are tough.
  (infantry-types fortress 10)
  (infantry-types capital 50)
  (armor-types capital 70)
  )

;; Nuclear detonation results in the destruction of units and the
;; creation of wasteland.

(add nuke-types acp-to-detonate 1)

;;; Terrain alteration.

(table acp-to-add-terrain
  (engineers settled 1)
  (engineers highway 1)
  )

(table acp-to-remove-terrain
  (engineers wilderness 1)
  )

;;; Vision.

(add mobile-radar vision-range 6)
(add radar vision-range 8)

;;; Random setup.

;;; Everybody starts with just the one capital.

(add capital start-with 1)

(add (sea settled wilderness mountains) alt-percentile-min (  0  70 70  95))
(add (sea settled wilderness mountains) alt-percentile-max ( 70  75 95 100))
(add t* wet-percentile-min 0)
(add t* wet-percentile-max 100)
(add settled wet-percentile-min 40)
(add settled wet-percentile-max 60)

(table favored-terrain
  (u* t* 0)
  (capital settled 100)
  )

;; A game's starting units will be full by default.

(table unit-initial-supply (u* m* 9999))

(table terrain-initial-supply
  (t* oil 100)
  (t* iron 100)
  (t* dust 1)  ; should be rare but high concentration randomly
  (t* rad 1)
  )

(set sides-min 1)
(set sides-max 30)

(game-module (instructions (
  "Build up your country and materials."
  "If you can't live in peace with your neighbors,"
  "then be prepared to fight with them."
  )))

(game-module (design-notes (
  "Scale is unimportant, this is an abstract game."
  )))
