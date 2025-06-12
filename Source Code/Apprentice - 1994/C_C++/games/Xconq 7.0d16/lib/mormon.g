(game-module "mormon"
  (title "1847")
  (blurb "The heroic age of the heroic Mormon pioneers")
  (variants (world-seen) (see-all true))
  )

(unit-type m (name "mormon") (char "m") (help ""))
(unit-type A (name "avenging angel") (char "A") (help ""))
(unit-type P (name "prophet") (char "P") (help ""))
(unit-type a (name "army") (char "a") (help ""))
(unit-type u (name "Ute") (char "u") (help ""))
(unit-type / (name "fort") (char "/") (help "where the army hangs out"))
(unit-type * (name "settlement") (char "*") (help ""))
(unit-type @ (name "temple") (char "@") (help ""))

(material-type water (help "very scarce in Utah"))

(terrain-type lake (char ".") (color "sky blue"))
(terrain-type river (char ",") (color "cyan"))
(terrain-type valley (char "+") (color "green"))
(terrain-type forest (char "%") (color "forest green"))
(terrain-type desert (char "~") (color "yellow"))
(terrain-type mountains (char "^") (color "sienna"))
(terrain-type |salt flat| (char "_") (color "white"))
(terrain-type |not-Utah| (char ":") (color "black"))

(add m image-name "man")
(add A image-name "trooper")
(add P image-name "man")
(add a image-name "cavalry")
(add u image-name "archer")
(add / image-name "walltown")
(add * image-name "walltown")
(add @ image-name "city18")

;;; Static relationships.

(table vanishes-on
  (u* (lake not-Utah) true)
  )

(add (* @) capacity 10)

(table unit-capacity-x
  (/ a 6)
  )

;; no random init, since sides asymmetrical

;5000 a siege

(table acp-to-create
  (P m 6)
  (m m 2)
  ;; work on these
;  (m * 20)
;  (a / 20)
  )

;1 water (m A a) produce
;10 water (/ * @) produce
;100 valley u* productivity
;100 mountains a productivity
;10 water (m A a) storage
;100 water (/ * @) storage
;1 water (m A a) consume

;"dies of thirst" (m A a) starve-message

;;;; scale should be 15 km?

(add (m A a u P) acp-per-turn (2 4 4 4 6))

(table mp-to-enter-terrain
  (u* (lake not-Utah) 99)
  )

(table hit-chance
  ((a u) m 95)  ; the army misses occasionally
  ((a u) P 50)  ; prophet is wily
  (a A 30)
  (A a 50)      ; avenging angels are only way to hit back
  )

(table damage (u* u* 1))

(add u* initial-seen-radius 4)

(scorekeeper (do last-side-wins))

(world 50)  ; hexagon would be better

(area 50 42)

(area (terrain
  "50h"
  "6ecfcfc2af33h3e"
  "2f5ecfcf2a2f32h2ef"
  "8ecfcfafd33h2e"
  "4e2a3ecfc2fdf32hef"
  "e2g2e2a2ecf2c2fd33he"
  "fe2ge3aeacf2cfdf32he"
  "2e3ge4aecf2cfd33h"
  "fe4ge3aecf3cdf32h"
  "he3gfge3aecf4c10dc3e18h"
  "he3gfge2ae2acf3c2d7f2dc2e18h"
  "2he5g2e3aecf2cd10fce2b17h"
  "2h2e4g3ea2ecf3c3d6fc2ebe17h"
  "3he3g3ef2efcf2cf3dc5ece2b2e16h"
  "3h2e3g3efefecfc2d3c7eb3e16h"
  "4h2e2g3ef2efacfcfdf9eb3e15h"
  "4h4eg3efefeacfcfd2f7eb3e15h"
  "5h7ef3eacfc2fd4f4ebe3f14h"
  "5h4ef7ecfc2efce4f2ebefdf14h"
  "6h4ef4efecfefefece4febefd2f13h"
  "6h2ef2ef4efcef2efe2c3efebef2df13h"
  "7h2efef6efefef5ec2ebefd3f12h"
  "7h2ef2ef2e2cef2efef8ebe2f2e12h"
  "8h2efef2e3c2ef2e2f5ecebe2f2eb11h"
  "8h2ef4e3cecfce2f7ebe2f2eb11h"
  "9h5ea6ece2f7ecbefe2be10h"
  "9h3efea5efc3ef7eb3eb2e10h"
  "10h2efea4ecfecfef8eb2eb3e9h"
  "10hefef2ef5ec11eb2ebefe9h"
  "11hefefef4e2f6ef5ebeb2efe8h"
  "11hef2ef4ec2ef5e2f5e2b2efe8h"
  "12hefe2f4ef2e2f4ef5eb6e7h"
  "12h2e2f6efef2ef2e2f4eb6e7h"
  "13h2f5ec2f5ef2ef3e2b3e2f2e6h"
  "13h5ec2e2f5ef2ef2e2b4e3fe6h"
  "14h6ec2f11eb6e2f2e5h"
  "14h3ef3ef11e2b10e5h"
  "15h2efe2cf11ebe5b6e4h"
  "15h2efec11e3b6e6b4h"
  "16h3ec11eb15e3h"
  "16h10ec4eb2e2f11e3h"
  "50h"
))
#|
(area (terrain (by-char ".;+%~^_:")
  "50:"
  "6~+^+^+..^33:3~"
  "^^5~+^+^..^^32:~~1^"
  "8~+^+^.^%33:2~"
  "4~..3~+^+^^%^32:~1^"
  "~__~~..~~+^++^^%33:1~"
  "^~__~3.~.+^++^%^32:1~"
  "~~3_~4.~+^++^%33:"
  "^~4_~3.~+^3+%^32:"
  ":~3_^_~3.~+^4+10%+3~18:"
  ":~3_^_~..~..+^3+%%7^%%+~~18:"
  "::~5_~~3.~+^++%10^+~..17:"
  "::~~4_3~.~~+^3+3%6^+~~.~17:"
  "3:~3_3~^~~^+^++^3%+5~+~..~~16:"
  "3:~~3_3~^~^~+^+%%3+7~.3~16:"
  "4:~~__3~^~~^.+^+^%^9~.3~15:"
  "4:4~_3~^~^~.+^+^%^^7~.3~15:"
  "5:7~^3~.+^+^^%4^4~.~3^14:"
  "5:4~^7~+^+~~^+~4^~~.~^%^14:"
  "6:4~^4~^~+^~^~^~+~4^~.~^%^^13:"
  "6:~~^~~^4~^+~^~~^~++3~^~.~^%%^13:"
  "7:~~^~^6~^~^~^5~+~~.~^%3^12:"
  "7:~~^~~^~~++~^~~^~^8~.~^^~~12:"
  "8:~~^~^~~3+~~^~~^^5~+~.~^^~~.11:"
  "8:~~^4~3+~+^+~^^7~.~^^~~.11:"
  "9:5~;6~+~^^7~+.~^~..~10:"
  "9:3~^~;5~^+3~^7~.3~.~~10:"
  "10:~~^~;4~+^~+^~^8~.~~.3~9:"
  "10:~^~^~~^5~+11~.~~.~^~9:"
  "11:~^~^~^4~^^6~^5~.~.~~^~8:"
  "11:~^~~^4~+~~^5~^^5~..~~^~8:"
  "12:~^~^^4~^~~^^4~^5~.6~7:"
  "12:~~^^6~^~^~~^~~^^4~.6~7:"
  "13:^^5~+^^5~^~~^3~..3~^^~~6:"
  "13:5~+~~^^5~^~~^~~..4~3^~6:"
  "14:6~+^^11~.6~^^~~5:"
  "14:3~^3~^11~..10~5:"
  "15:~~^~++^11~.~5.6~4:"
  "15:~~^~+11~3.6~6.4:"
  "16:3~+11~.15~3:"
  "16:10~+4~.~~^^11~3:"
  "50:"
))
|#

(side 1 (name "US"))

(side 2 (noun "Mormon"))

(/ 14 31 1 (n "Ft Douglas"))
(a 14 31 1)
(a 14 31 1)
(a 14 31 1)
(a 14 31 1)
(a 14 31 1)
(a 14 31 1)

(@ 14 30 2 (n "Salt Lake City"))
(P 14 30 2 (n "Brigham Young"))
(A 14 30 2)
(m 14 30 2)

(* 15 27 2 (n "Orem"))
(* 17 25 2 (n "Provo"))
(* 14 29 2 (n "Sandy"))
(* 12 32 2 (n "Ogden"))

(game-module (instructions (
  "The Mormons should try to reproduce themselves "
  "and spread out as much as possible "
  "before the cavalry catches up with them."
  "Use the Avenging Angels to ambush the cavs."
  ""
  "The US cavalry just has to kill as many Mormons "
  "as fast as they can."
  )))

(game-module (notes (
  "Can the US army kill all the Mormons before they overpopulate Utah?"
  ""
  "(This is all a joke of course.)"
  )))

(game-module (design-notes (
  "Needs some cleanup and balancing work"
  )))
