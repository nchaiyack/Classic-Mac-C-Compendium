(game-module "hobbit"
  (title "Middle Earth")
  (blurb "Tolkien's Middle Earth, version 1.0")
  (variants (see-all true))
)

(unit-type hobbit (image-name "hobbit") (possible-sides "good"))
(unit-type dwarf (image-name "dwarf") (possible-sides "good"))
(unit-type elf (image-name "elf") (possible-sides "good"))
(unit-type man (image-name "swordman"))
(unit-type rider (image-name "cavalry"))
(unit-type ship (image-name "frigate") (possible-sides "evil"))
(unit-type troll (image-name "troll") (possible-sides "evil"))
(unit-type nazgul (image-name "nazgul") (possible-sides "evil"))
(unit-type bridges (image-name "bridge"))
(unit-type mine (image-name "mine2"))
(unit-type village (image-name "walltown"))
(unit-type town (image-name "city18")) ; or castle?

(define h hobbit)
(define d dwarf)
(define e elf)
(define m man)
(define R rider)
(define S ship)
(define T troll)
(define N nazgul)
(define b bridges)
(define / mine)
(define * village)
(define @ town)

(material-type food (help "everybody needs it"))
(material-type gold (help "everybody wants it"))

(define ! food)
(define $ gold)

(terrain-type sea (color "deep sky blue"))
(terrain-type river (color "cyan"))
(terrain-type swamp (color "medium aquamarine"))
(terrain-type plains (color "green"))
(terrain-type forest (color "forest green"))
(terrain-type wasteland (color "yellow") (image-name "desert"))
(terrain-type mountains (color "sienna"))
(terrain-type ice (color "gray"))

; lots of forests in this period:
;                            sea riv swa pla for des mou ice
(add t* alt-percentile-min (   0  53  55  55  55  55  93  99))
(add t* alt-percentile-max (  53  55  93  93  93  93  99 100))
(add t* wet-percentile-min (   0   0  97  12  47   0   0   0))
(add t* wet-percentile-max ( 100 100 100  47  97  12 100 100))

;65 alt-roughness
;70 wet-roughness

(set edge-terrain ice)

(define land (plains wasteland forest mountains))
(define land-forces (h d e m R))
(define movers (h d e m R S T N))
(define places (/ * @))

(table vanishes-on
  (u* (sea river) true)
  (nazgul t* false)
  (ship (sea river) false)
)

;true [ * @ ] named
;[ 1 2 10 25 ] [ N / * @ ] territory

(add (/ * @) start-with (0 0 1 #|1 5 1|#))

(table independent-density (places t* (10 80 20)))

(table favored-terrain
  (u* t* 0)
  (/ (plains forest mountains wasteland) (20 20 100 20))
  (* (plains forest mountains) (100 40 20))
  (@ (plains forest) (100 20))
)

(set country-radius-min 5)
(set country-separation-min 20)

#|

25 [ / ] visibility
60 [ h e ] visibility
25 forest [ h d e m R T ] conceal 
true [ b * @ ] always-seen
true u* already-seen


2 u* [ * @ ] repair

2 food land-forces produce
3 food R produce
1 food T produce
20 gold / produce
[ 10 1 ] [ food gold ] * produce
[ 20 2 ] [ food gold ] @ produce
100 plains land-forces productivity
 50 forest land-forces productivity
100 [ forest mountains ] T productivity
;  h   d  e  m   R   S  T  N b    /   *    @
[  4   8  8  8  15 100 20 20 0   10 500 1000 ] food u* storage
[ 10 100 20 50  50  50 30 20 0 1000 500 1000 ] gold u* storage
100 food u* stockpile
  0 gold u* stockpile
 10 gold m stockpile
1 food u* consume
2 food R consume
0 r* [ b / ]  consume
100 t* / productivity 
[ 100 50 20 20 ] land * productivity
[ 100 50 20 20 ] land @ productivity

|#

;                         h d e m R S T  N
(add movers acp-per-turn (1 1 2 1 5 5 1 10))

(add (* @) acp-per-turn 1)

(add (* @) speed 0)

(table mp-to-enter-terrain
  (u* t* 100)
  (land-forces land 1)
  (h swamp 1)
  (e (river swamp mountains) 2)
  (R (wasteland forest mountains) (2 4 4))
  (S (sea river) (1 3))
  (N t* 1)
  (T (forest mountains) 1)
)

(table unit-size-as-occupant
  (movers b 1)
  (movers (/ * @) 1)
  (movers S 1)
)

(add b capacity 1)
(add (/ * @) capacity 10)
(add S capacity 4)

;  h d e m R S T N
;;[  8 6 6 4 2 1 1 1 ] movers * capacity
;[ 12 9 9 6 3 2 1 2 ] movers @ capacity
;[  1 5 0 1 0 ] land-forces / capacity
;[  4 3 3 2 1 ] land-forces S capacity

;;; Construction parameters.

(table cp-on-creation
  (@ movers 1)
  (* movers 1)
)
;  h  d  e  m  R  S  T  N
;[  8 12 12 12 20 20 25 40 ] movers @ make
;[ 16 24 24 24 40 40 50 80 ] movers * make
;  R   S   T   N
;[ 20 100 200 500 ] gold [ R S T N ] to-make

;true [ * @ ] maker ; part of doctrine now

;100 / d make
;0 u* startup
;100 N research

;               h d e m R S T N b  /  *  @
(add u* hp-max (1 1 1 2 2 3 4 4 8 20 20 40))

;[ 0 0 0 1 1 2 2 2 4 10 10 20 ] u* crippled

(table hit-chance
;             h  d  e  m  R  S  T  N 
  (h movers (30 20 20 20 20  0 20 50))
  (d movers (50 50 40 40 40 10 30 10))
  (e movers (50 40 30 40 30 10 40 10))
  (m movers (50 40 40 50 50 15 25 20))
  (R movers (50 40 30 50 60 20 50 30))
  (T movers (30 50 20 50 50 30 50  0))
  (N movers (70 90 60 80 70 50 60 50))
  (S movers 0)
  (S S 40)
  ((b / * @) u* 0)
)

;true u* b bridge 

(table damage
  (u* u* 1)
  ((T N) u* 2)
  (h N 2)
)

(table capture-chance
  (h places (20 30 20))
  (d places (75 30 20))
  (e places ( 0 50 35))
  (m places (50 50 35))
  (R places ( 0 30 20))
  ;; Bridges are easy to capture
  ;; (should just allow everybody to use, leave indep)
  (movers b 100)
  ((S N) b 30)
  )

(add u* acp-to-disband 1)
(add places acp-to-disband 0)

;"kills" movers destroy-message
;"sinks" S destroy-message

;-20 neutrality

;; [ 90 60 80 ] [ R T N ] control

; disasters: evil creatures lurk in forests and mountains
; trolls and nazguls are immune, being themselves evil
;200 swamp land-forces accident
;100 wasteland land-forces accident
;100 forest [ h d m R ] accident ; elves can defend themselves in the forest
;;100 mountains land-forces accident
;400 ice land-forces accident
;"is killed by evil creatures" land-forces accident-message
; 50 [ sea river ] S accident
;"is lost in a storm" S accident-message

(set side-library '(
  ((noun "Numenorean") (class "good"))
  ((noun "Dunadan") (class "good"))
  ((noun "Ranger") (class "good"))	
  ((noun "Corsair") (class "evil"))
  ((noun "Gondorian") (class "good"))
  ((noun "Rhovanian"))	
  ((noun "Arnorian") (class "good"))
  ((noun "Rohirrim") (class "good"))
  ((noun "Lindonian"))	
  ((noun "Southron"))
  ((noun "Easterling"))
  ((noun "Mordorian") (class "evil"))	
  ((noun "Sindar") (class "good"))
  ((noun "Breelander"))
  ((noun "Variag"))		
  ((noun "Haradrim") (class "evil"))	
))

(game-module (notes
 "This period tries to reproduce Tolkien's Middle Earth."
 "Orcs, Wizards, Dragons and RINGS are not implemented (due to lack of ideas)."
 ""
 "Please send comments, suggestions, fixes, and nicer bitmaps to the author:"
 "Massimo Campostrini (campo@sunthpi1.difi.unipi.it)"
 "Dipartimento di Fisica,  Piazza Torricelli 2,"
 "I-56126 Pisa,  Italy"
))

(add hobbit notes (
  "Easily produced, hobbits are good explorers but not very good"
  "fighters."
))

(add dwarf notes (
  "Dwarves are irreplaceable in digging, running and conquering mines."
))

(add elf notes (
  "Good and fast explorers, elven can swim across rivers."
))

(add man notes (
  "Men are the solid core of armies."
))

(add rider notes (
  "The fast-moving riders can explore and fight (best used for"
  "shock-effect)."
))

(add ship notes (
  "Ships can quickly carry land forces across water and along rivers,"
  "but are no good for fighting (except by themselves)."
))

(add troll notes (
  "Trolls are powerful but cumbersome units, and cannot cross plains"
  "or embark in ships."
))

(add nazgul notes (
  "Ringwraiths or Nazguls are evil creatures moving very quickly upon"
  "winged steeds.  They are superb fighters, but very vulnerable to hobbits!"
))

(add bridges notes (
  "Very useful to cross rivers."
))

(add mine notes (
  "Gold is obtained in quantity only in mines.  A lot of gold is needed"
  "to produce trolls and nazguls.  Therefore try to build or capture mines." 
))

(add village notes (
  "Small population center."
))

(add town notes (
  "Larger population center."
))
