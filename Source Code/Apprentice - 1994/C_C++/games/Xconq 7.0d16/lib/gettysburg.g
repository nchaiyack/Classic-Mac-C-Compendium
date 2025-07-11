(game-module "gettysburg"
  (title "Gettysburg")
  (blurb "The Battle of Gettysburg, 1-3 July 1863")
  (instructions (
   "Most of the brigades are off-board, will come in a few at a time; "
   "the Confederates from the north and west, the Federals from the south."
   ""
   "You have to try to hold your positions, while at the same time "
   "charging the enemy."
   ""
   "Artillery can be captured, so be sure to guard it."
   ))
  (variants
    (see-all true)
    ("Day/Night" eval (world (day-length 24)))
    )
  )

(unit-type infantry)
(unit-type cavalry)
(unit-type artillery (image-name "cannon"))
(unit-type supply-train (image-name "wagon"))

(define inf infantry)
(define cav cavalry)
(define arty artillery)

(terrain-type clear (image-name "straw") (char "+"))
(terrain-type rough (image-name "mountains") (char "^"))
(terrain-type light-woods (char "%"))
(terrain-type orchard)
(terrain-type woods (image-name "forest") (char "%"))
(terrain-type town (image-name "urban") (char "G"))
(terrain-type stream (image-name "water") (subtype border))
(terrain-type ravine (image-name "gully") (subtype border))
(terrain-type trail (subtype connection))
(terrain-type road (subtype connection))

(add t* elevation-min 0 #|550|#)
(add t* elevation-max 50 #|600|#)

(set grid-color "gray")

;;; Static relationships.

(add t* capacity 3)

(table unit-size-in-terrain
  (u* t* 1)
  (inf road 10)
  (supply-train t* 0)
  )

;(add u* possible-sides (not "independent"))

;;; Actions.

(add u* acp-per-turn (8 16 8 8))

;; Night shuts things down considerably.

(table night-acp-effect
  ;; 1/4 of normal abilities.
  (u* t* 25)
  ;; Infantry in open terrain can still move around somewhat.
  (infantry clear 50)
  ;; (should allow movement along roads at normal rate?)
  )
  
;;; Movement.

(table mp-to-enter-terrain
  (u* t* 1)
  (u* rough 99)
  (inf rough 2)
  ((cav arty) woods 4)
  ((inf cav) stream 0)
  ((cavalry arty) road 0)
  )

(table mp-to-leave-terrain
  (u* t* 1)
  (u* rough 99)
  (inf rough 2)
  ((cav arty) woods 4)
  ((cavalry arty) road 0)
  )

(table mp-to-traverse
  ((arty cavalry) road (2 1))
  (supply-train (trail road) (2 1))
  )

(table zoc-range
  (inf inf 1) ; should be 2
  )

(table mp-to-enter-zoc
  (inf inf 4)
  )

(table mp-to-traverse-zoc
  (inf inf 4)
  )

;;; Repair.

(table acp-to-repair
  (u* u* 4)
  )

(table hp-per-repair
  (u* u* 2)
  )

;;; Combat.

(add u* hp-max 8)

(table acp-to-attack
  (u* u* 4)
  (arty u* 0)
  (supply-train u* 0)
  )

(table acp-to-defend
  (u* u* 4))

(table hit-chance
  (u* u* 50)
  (supply-train u* 0)
  )

(table damage
  (u* u* 1d3)
  (supply-train u* 0)
  )

(add arty range 5)  ;  (5 or 3 - need two types?)

(add arty acp-to-fire 8)

(table capture-chance
  (infantry arty 30)
  )

;; Everybody bounces back over time.

(add u* hp-recovery 0.25)
 
;;; Vision.

(set terrain-seen true)

(add u* vision-range 5)

(table visibility
  (u* t* 100)
  (u* (orchard light-woods) 50)
  (u* woods 0)
  )

;;; The world.

(world 125000)

(area 50 36)

(area (terrain
  "50a"
  "aea2e5ae5a2e3ae9ae18a"
  "ae8a2e5aea2e2ae7ae18a"
  "9aeae5ae2ae2ae7ae18a"
  "10ae39a"
  "4ae23ae2ae4ae13a"
  "13ad6ae8a3e4ae13a"
  "ae4a2e5aed5ae3ae25a"
  "ae12ae6ae3ad6a2e3ad12a"
  "ae2ae2ae2ae3ae12a2e4ae16a"
  "5aeae2aea2ead11a2e21a"
  "10aead2ae2a3f6a3e8ae3ae7a"
  "3ae3ae2a2e4ae2a2fe5a3e8a2e3ae6a"
  "aeae3ae3ae4ae10a2e6ae2a3e3ae5a"
  "aea2e2a2e2aead3ad6ae2a2ead4ae7a3e4a"
  "2ae6a2e2ad3ae6a2ea3e15ae4a"
  "2a2e4ad4ae3ae6a3ea3e19a"
  "14ae2ae3ac4aea3eae10ae6a"
  "15a3e5aea7ea2e6ae2a3ed2a"
  "13ad2aeae5ae2a2ea2e2ae3a4e8a"
  "12a2e3ae13ae10ae7a"
  "11ae2ae2ae2ad3aea2eae3ae16a"
  "7a2e2aea3e2ae10ae3ae5ae10a"
  "7a2e2ae7a2d3a2e3ae3ae4a3e7aea"
  "8a2e14a3ea3e10aead3a2ea"
  "9a2e3aeaea2ead2e5a3eae14aea"
  "10ae3aeae3ae2ae3c6ae8a3e3aea"
  "14ae6ae2ae2c7aeae5ac3e4a"
  "21ae2a4e6ae9a2c4a"
  "21ad6e22a"
  "18a3e2a5e9ae2a3e2ad4a"
  "19ae4a2e2aead10a3e2ad3a"
  "18a3e7a2e20a"
  "19a2e3a3ea2e20a"
  "25a2e2ae3ae3ae3a3e6a"
  "50a"
))

(area (aux-terrain stream
  "50a"
  "6agq7acy33a"
  "6agL8ahq32a"
  "6acT8agL32a"
  "7ahq7acXi31a"
  "7acXi7ab?y30a"
  "8ab?y8ahq29a"
  "10ahq7acT29a"
  "10acT8ap}4aem22a"
  "11ady7adU<3agM<21a"
  "12ahq7ady3agL22a"
  "12agL8ahq2agL22a"
  "12agL8acT2agL22a"
  "12agL9adyagL22a"
  "12agL10adygL22a"
  "12agL11ad:L22a"
  "12acT12ahL22a"
  "13ahq11agL22a"
  "13agL11acT22a"
  "13agL12ady21a"
  "13acTgq11ady20a"
  "14ahwL12ady19a"
  "8ae2mi2acZL13ady18a"
  "8acM=?y2ahL14ahq17a"
  "12ad}igL14acT17a"
  "13abCwL15ahqae14a"
  "14agNT15acXicT13a"
  "14agLhq14aenKqhq12a"
  "14acTgL15a?YDgL12a"
  "15ad:T16abC:L12a"
  "16ab?y12ac}2moM<12a"
  "18ahq12ab=CYH2m11a"
  "18agL14acU3=<10a"
  "18agL15ady13a"
  "18acL16adq12a"
  "50a"
))

(area (aux-terrain ravine
  "50a"
  "7agq41a"
  "7agL14ai26a"
  "7acT14ad}3agq20a"
  "8ahq14ab<3a<20a"
  "9a<40a"
  "50a"
  "50a"
  "50a"
  "35ai14a"
  "35ady13a"
  "34agqdy12a"
  "18ae15agLab12a"
  "18agL14agL14a"
  "18agL14acL14a"
  "18agL30a"
  "19a<14agq14a"
  "34agL4ae9a"
  "34agL4acL8a"
  "34acT4ae9a"
  "35ahqeiaoL8a"
  "35agLgMab<8a"
  "36a<a<11a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
))

(area (aux-terrain trail
  "50a"
  "19ae30a"
  "20a@29a"
  "21a>u27a"
  "23a@3ac}21a"
  "24a@3aj>u19a"
  "25a>uaj2a>u17a"
  "27a@j4a@4ai11a"
  "28aE5a@3aj11a"
  "27akr6a@2aj11a"
  "27aj8a@aj11a"
  "27aj9a@j11a"
  "18ai8af10aE11a"
  "18aj9aD9aj11a"
  "18aj9aj9aj11a"
  "18ab9aj9aj11a"
  "28af9aj11a"
  "7ae21aD8af11a"
  "8aD20aj9aD10a"
  "8af20aj9aj10a"
  "9aD18akr9aj10a"
  "9af18aj10aj10a"
  "10a>u12ak3sv10aj10a"
  "12a@7acu2aj4aD9af10a"
  "13a>s2{su3a>uj4an10aD9a"
  "15a2j2a>q3a?suakr@6ai2aj9a"
  "15a2f10a>r2a@5aj2aj9a"
  "16a2D9acsu2a@4aj2ab9a"
  "16a2j12a>ua@3aj12a"
  "16ajn14a>sP2aj12a"
  "14acstr@2ae13a>wr12a"
  "19a@2a@14a@12a"
  "20a@2a@7aksu4aD11a"
  "21a<2a>6sr2a>3sr11a"
  "50a"
  "50a"
))

(area (aux-terrain road
  "50a"
  "7ae4ae2ai3ai10aksq17a"
  "8a>u3a@aj3aj8aksr19a"
  "acu7a@3a@f3aj7akr21a"
  "3a>u6a@3a@D2aj5aksr22a"
  "5a>u5a>u2aE2aj4akr24a"
  "7a>u5a@af2aj4aj25a"
  "9a>u4a@aDaj3akr25a"
  "11a>u3a@faj2akr26a"
  "13a>u2a@Djakr27a"
  "15a>uaAnkr28a"
  "17a>q=?2su26a"
  "15ak3syi3a>su23a"
  "13aksr3a2f6a>su20a"
  "12akr6aDH8a>3su15a"
  "10aksr7apr@12a>3su10a"
  "8aksr9aj@a@16a>2su6a"
  "6aksr11ajaDa@19a>3sqa"
  "4aksr13ajaf2a@24a"
  "3acr15aj2a@2a@23a"
  "20aj3a@2a@22a"
  "20aj4aD2a>su19a"
  "20aj4af5a@18a"
  "20aj5aD5a>u16a"
  "20aj5af7a>u14a"
  "20af6aD8a>u12a"
  "21aD5aj10a>u10a"
  "21aj5af12a>u8a"
  "21aj6a@13a>u6a"
  "21aj7aD14a>u4a"
  "21aj7af16a>u2a"
  "21aj8aD17a<a"
  "21aj8af19a"
  "21aj9aD18a"
  "21ab9ab18a"
  "50a"
))

(area (features (
   (17 "ridge" "Seminary *")
   (16 "town" "Gettysburg")
   (15 "hill" "Barlow's *")
   (14 "hill" "Granite *")
   (13 "hill" "Oak *")
   (12 "hill" "Benner's *")
   (11 "hill" "Wolf *")
   (10 "hill" "Culp's *")
   (9 "hill" "Power's *")
   (8 "hill" "Cemetery *")
   (7 "hill" "Little Round Top")
   (6 "hill" "Big Round Top")
   (5 "ridge" "Cress *")
   (4 "ridge" "Warfield *")
   (3 "ridge" "Cemetery *")
   (2 "ridge" "McPherson's *")
   (1 "ridge" "Herr *")
  )
  "50a"
  "50a"
  "50a"
  "33ao16a"
  "50a"
  "50a"
  "8ab41a"
  "8a2b40a"
  "7a3b40a"
  "7a3b3ac36a"
  "7a3b3ac36a"
  "7a2b4a2c3a3q3am25a"
  "8ab5ac4a2q29a"
  "8ab5ac35a"
  "8ab12ad28a"
  "8ab12aidak25a"
  "9ab11a2d6al20a"
  "21a2d27a"
  "21a2d27a"
  "22ad27a"
  "23ad26a"
  "24ad2ad22a"
  "25ad24a"
  "26ad23a"
  "25ab24a"
  "50a"
  "26ah23a"
  "26ah23a"
  "50a"
  "26ag23a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
))

(area (elevations
  "50a"
  "a5ba3bc12a6b21a"
  "a5ba2babc12a5b21a"
  "a2b2aba2babc12a5b21a"
  "a2b5a2babc11a2b24a"
  "ab6a2b2a2b10a2b24a"
  "8ab3a2b10a2b24a"
  "8a2b3ab10a2b24a"
  "7a3b3a2b9a2b24a"
  "7a3b3a2b9a2b24a"
  "7a3b3abab8a2b24a"
  "7a2b4abab8ab25a"
  "8ab5abab7ab25a"
  "8ab5abab33a"
  "8ab5ab2ab3ac28a"
  "8ab8ab3a2c27a"
  "9ab7ab3a2c27a"
  "21a2c27a"
  "21a2b27a"
  "22a2b26a"
  "23ab26a"
  "27ab22a"
  "25ab24a"
  "26ab23a"
  "25ab24a"
  "50a"
  "26ac23a"
  "26ac23a"
  "50a"
  "26ac23a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
))

(side 1 (name "US") (adjective "Union")
  (emblem-name "flag-usa") (names-locked true))

(side 2 (name "CSA") (adjective "Confederate")
  (emblem-name "flag-csa") (names-locked true))

(set sides-min 2)
(set sides-max 2)

(set advantage-default 1)
(set advantage-min 1)
(set advantage-max 1)

#|  ; McPherson's Ridge scenario
(infantry 13 25 1  (n "1 Meredith"))
(infantry 14 27 1  (n "2 Cutler"))
(infantry 13 29 1  (n "3 Paul"))
(infantry 13 28 1  (n "4 Baxter"))
(infantry 14 24 1  (n "5 Rowley"))
(infantry 13 26 1  (n "6 Stone"))
(artillery 14 26 1  (n "7 Wainwright a"))
(artillery 14 24 1  (n "8 Wainwright b"))
(infantry 14 24 1  (n "5 Rowley"))
(infantry 13 26 1  (n "6 Stone"))
|#

(inf -21 -1 1 (n "Meredith") (cp -1) (x (appear 3)))
(inf -21 -1 1 (n "Cutler") (cp -1) (x (appear 3)))
(inf -21 -1 1 (n "Paul") (cp -1) (x (appear 4)))
(inf -21 -1 1 (n "Baxter") (cp -1) (x ((appear 4))))
(inf -3 -16 1 (n "Rowley") (cp -1) (x (appear 6)))
(inf -3 -16 1 (n "Stone") (cp -1) (x (appear 6)))
(arty -21 -1 1 (n "Wainwright 1") (cp -1) (x (appear 5)))
(arty -21 -1 1 (n "Wainwright 2") (cp -1) (x (appear 5)))
(inf -31 -1 1 (n "Cross") (cp -1) (x (appear 18)))
(inf -31 -1 1 (n "Kelly") (cp -1) (x (appear 18)))
(inf -31 -1 1 (n "Zook") (cp -1) (x (appear 18)))
(inf -31 -1 1 (n "Brooke") (cp -1) (x (appear 19)))
(inf -31 -1 1 (n "Harrow") (cp -1) (x (appear 19)))
(inf -31 -1 1 (n "Webb") (cp -1) (x (appear 19)))
(inf -31 -1 1 (n "Hall") (cp -1) (x (appear 20)))
(inf -31 -1 1 (n "Carroll") (cp -1) (x (appear 20)))
(inf -31 -1 1 (n "Smyth") (cp -1) (x (appear 20)))
(inf -31 -1 1 (n "Willard") (cp -1) (x (appear 21)))
(arty -31 -1 1 (n "Hazard 1") (cp -1) (x (appear 21)))
(arty -31 -1 1 (n "Hazard 2") (cp -1) (x (appear 21)))
(inf -21 -1 1 (n "Graham") (cp -1) (x (appear 12)))
(inf -21 -1 1 (n "Ward") (cp -1) (x (appear 12)))
(inf -21 -1 1 (n "de Trobriand") (cp -1) (x (appear 27)))
(inf -21 -1 1 (n "Carr") (cp -1) (x (appear 13)))
(inf -21 -1 1 (n "Brewster") (cp -1) (x (appear 13)))
(inf -21 -1 1 (n "Burling") (cp -1) (x (appear 27)))
(arty -21 -1 1 (n "Rand a") (cp -1) (x (appear 12)))
(arty -21 -1 1 (n "Rand b") (cp -1) (x (appear 14)))
(inf -43 -19 1 (n "Tilton") (cp -1) (x (appear 27)))
(inf -43 -19 1 (n "Schweitzer") (cp -1) (x (appear 27)))
(inf -43 -19 1 (n "Vincent") (cp -1) (x (appear 27)))
(inf -43 -19 1 (n "Day") (cp -1) (x (appear 28)))
(inf -43 -19 1 (n "Burbank") (cp -1) (x (appear 28)))
(inf -43 -19 1 (n "Weed") (cp -1) (x (appear 28)))
(inf -43 -19 1 (n "McCandless") (cp -1) (x (appear 31)))
(inf -43 -19 1 (n "Fisher") (cp -1) (x (appear 32)))
(arty -43 -19 1 (n "Martin a") (cp -1) (x (appear 29)))
(arty -43 -19 1 (n "Martin b") (cp -1) (x (appear 29)))
(inf -48 -4 1 (n "Torbert") (cp -1) (x (appear 34)))
(inf -48 -4 1 (n "Bartlett") (cp -1) (x (appear 34)))
(inf -48 -4 1 (n "Russell") (cp -1) (x (appear 34)))
(inf -48 -4 1 (n "Grant") (cp -1) (x (appear 35)))
(inf -48 -4 1 (n "Neill") (cp -1) (x (appear 35)))
(inf -48 -4 1 (n "Shaler") (cp -1) (x (appear 35)))
(inf -48 -4 1 (n "Eustis") (cp -1) (x (appear 36)))
(inf -48 -4 1 (n "Wheaton") (cp -1) (x (appear 36)))
(arty -48 -4 1 (n "Tompkins a") (cp -1) (x (appear 37)))
(arty -48 -4 1 (n "Tompkins b") (cp -1) (x (appear 37)))
(arty -48 -4 1 (n "Tompkins c") (cp -1) (x (appear 37)))
(inf -21 -1 1 (n "von Gilsa") (cp -1) (x (appear 6)))
(inf -21 -1 1 (n "Ames") (cp -1) (x (appear 6)))
(inf -31 -1 1 (n "Coster") (cp -1) (x (appear 8)))
(inf -31 -1 1 (n "Smith") (cp -1) (x (appear 8)))
(inf -31 -1 1 (n "Schimmelfenning") (cp -1) (x (appear 6)))
(inf -31 -1 1 (n "Krzyzanowski") (cp -1) (x (appear 6)))
(arty -31 -1 1 (n "Osborn a") (cp -1) (x (appear 7)))
(arty -31 -1 1 (n "Osborn b") (cp -1) (x (appear 7)))
(inf -48 -4 1 (n "McDougall") (cp -1) (x (appear 10)))
(inf -48 -4 1 (n "Ruger") (cp -1) (x (appear 10)))
(inf -48 -4 1 (n "Candy") (cp -1) (x (appear 10)))
(inf -48 -4 1 (n "Kane") (cp -1) (x (appear 11)))
(inf -48 -4 1 (n "Greene") (cp -1) (x (appear 11)))
(arty -48 -4 1 (n "Muhlenburg 1") (cp -1) (x (appear 12)))
(arty -48 -4 1 (n "Muhlenburg 2") (cp -1) (x (appear 12)))
(inf -21 -1 1 (n "Stannard") (cp -1) (x (appear 11)))
(inf -48 -4 1 (n "Lockwood") (cp -1) (x (appear 25)))
(arty -31 -1 1 (n "Ransom") (cp -1) (x (appear 24)))
(arty -31 -1 1 (n "McGilvey") (cp -1) (x (appear 28)))
(arty -31 -1 1 (n "Taft") (cp -1) (x (appear 24)))
(arty -31 -1 1 (n "Huntington") (cp -1) (x (appear 25)))
(arty -31 -1 1 (n "Fitzhugh") (cp -1) (x (appear 25)))
(cav 20 24 1 (n "Gamble"))
(cav 20 23 1 (n "Devin"))
(cav -43 -19 1 (n "McIntosh") (cp -1) (x (appear 36)))
(cav -43 -19 1 (n "Gregg") (cp -1) (x (appear 36)))
(cav -43 -19 1 (n "Farnsworth") (cp -1) (x (appear 57)))
(cav -43 -19 1 (n "Custer") (cp -1) (x (appear 57)))
(supply-train -48 -4 1 (cp -1) (x (appear 16)))

(inf -1 -32 2 (n "Anderson") (cp -1) (x (appear 18)))
(inf -1 -32 2 (n " Benning") (cp -1) (x (appear 18)))
(inf -1 -32 2 (n "Law") (cp -1) (x (appear 31)))
(inf -1 -32 2 (n "Robertson") (cp -1) (x (appear 18)))
(arty -1 -32 2 (n "Henry") (cp -1) (x (appear 19)))
(inf -1 -32 2 (n "Barksdale") (cp -1) (x (appear 19)))
(inf -1 -32 2 (n "Kershaw") (cp -1) (x (appear 19)))
(inf -1 -32 2 (n "Semmes") (cp -1) (x (appear 20)))
(inf -1 -32 2 (n "Wofford") (cp -1) (x (appear 20)))
(arty -1 -32 2 (n "Cabell") (cp -1) (x (appear 20)))
(inf -1 -32 2 (n "Armisted") (cp -1) (x (appear 36)))
(inf -1 -32 2 (n "Garnett") (cp -1) (x (appear 36)))
(inf -1 -32 2 (n "Kemper") (cp -1) (x (appear 36)))
(arty -1 -32 2 (n "Dearing") (cp -1) (x (appear 37)))
(inf -1 -32 2 (n "Alexander") (cp -1) (x (appear 21)))
(inf -1 -32 2 (n "Eshleman") (cp -1) (x (appear 21)))
(inf -19 -34 2 (n "Gordon") (cp -1) (x (appear 9)))
(inf -19 -34 2 (n "Hays") (cp -1) (x (appear 9)))
(inf -19 -34 2 (n "Hoke") (cp -1) (x (appear 9)))
(inf -19 -34 2 (n "Smith") (cp -1) (x (appear 10)))
(arty -19 -34 2 (n "H. Jones") (cp -1) (x (appear 10)))
(inf -1 -32 2 (n "Jones") (cp -1) (x (appear 13)))
(inf -1 -32 2 (n "Nicholls") (cp -1) (x (appear 13)))
(inf -1 -32 2 (n "Steuart") (cp -1) (x (appear 13)))
(inf -1 -32 2 (n "Walker") (cp -1) (x (appear 14)))
(arty -1 -32 2 (n "Latimer") (cp -1) (x (appear 14)))
(inf -7 -34 2  (n "Daniel") (cp -1) (x (appear 7)))
(inf -7 -34 2 (n "Doles") (cp -1) (x (appear 7)))
(inf -7 -34 2 (n "Iverson") (cp -1) (x (appear 7)))
(inf -7 -34 2 (n "O'Neal") (cp -1) (x (appear 8)))
(inf -7 -34 2  (n "Ramseur") (cp -1) (x (appear 8)))
(arty -1 -32 2 (n "Carter") (cp -1) (x (appear 8)))
(arty -1 -32 2 (n "Dance") (cp -1) (x (appear 15)))
(arty -1 -32 2 (n "Nelson") (cp -1) (x (appear 15)))
(inf -1 -32 2 (n "Mahone") (cp -1) (x (appear 12)))
(inf -1 -32 2 (n "Perry") (cp -1) (x (appear 12)))
(inf -1 -32 2 (n "Posey") (cp -1) (x (appear 12)))
(inf -1 -32 2 (n "Wilcox") (cp -1) (x (appear 13)))
(inf -1 -32 2 (n "Wright") (cp -1) (x (appear 13)))
(arty -1 -32 2 (n "J. Lane") (cp -1) (x (appear 13)))
(inf 5 30 2 (n "Archer"))
(inf -1 -32 2 (n "Brockenbrough") (cp -1) (x (appear 4)))
(inf 5 30 2 (n "Davis"))
(inf -1 -32 2 (n "Pettigrew") (cp -1) (x (appear 4)))
(inf -1 -32 2 (n "J. Garnet") (cp -1) (x (appear 4)))
(inf -1 -32 2 (n "Lane") (cp -1) (x (appear 7)))
(inf -1 -32 2 (n "Perrin") (cp -1) (x (appear 7)))
(inf -1 -32 2 (n "Scales") (cp -1) (x (appear 7)))
(inf -1 -32 2 (n "Thomas") (cp -1) (x (appear 8)))
(arty -1 -32 2 (n "Poague") (cp -1) (x (appear 8)))
(arty -1 -32 2 (n "McIntosh") (cp -1) (x (appear 5)))
(arty -1 -32 2 (n "Pegram") (cp -1) (x (appear 3)))
(cav -19 -34 2 (n "Hampton") (cp -1) (x (appear 33)))
(cav -19 -34 2 (n "Jenkins") (cp -1) (x (appear 10)))
(cav -19 -34 2 (n "F. Lee") (cp -1) (x (appear 33)))
(cav -19 -34 2 (n "W. Lee") (cp -1) (x (appear 33)))

(supply-train -1 -32 2 (cp -1) (x (appear 16)))

;;; Should be more elaborate.

(scorekeeper (do last-side-wins))

;;; We get at least three days.

(set last-turn 72)

;;; ... and possibly another few hours.

(set extra-turn-chance 10)

(set calendar '(usual "hour"))

(set initial-date "06 1 Jul 1863")

;; The game starts at six hours before noon.

(set initial-day-part 0)

(game-module (design-notes (
  "This is a brigade-level simulation, inspired by the game described in"
  "the July-Aug 1992 issue of Command."
  ""
  "The scale is 5 hexes/mile, 1 hour/turn."
  )))

