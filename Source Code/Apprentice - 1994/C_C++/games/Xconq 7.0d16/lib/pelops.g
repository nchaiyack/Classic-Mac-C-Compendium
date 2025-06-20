(game-module "pelops"
  (title "Peloponnesian War")
  (blurb "Athens vs Sparta, for control of Greece")
  (base-module "greek")
  (variants
   (see-all true)
   ;; not realistic, more for debugging
   ("Wipeout" eval (scorekeeper (do last-side-wins)))
   )
)

;;; Suppress random startups (should hack synthesis list)
(add u* start-with 0)
(add u* independent-near-start 0)
(table independent-density (u* t* 0))

(set advantage-min 1)
(set advantage-default 1)
(set advantage-max 1)

;;; One of the interesting properties of the Pelopponnesian War was that
;;; it was really a three-sided contest, with Persia always waiting in
;;; the wings, looking for opportunities to influence events.

(side 1 (noun "Athenian"))

(side 2 (noun "Spartan"))

(side 3 (noun "Persian"))

(include "u-greek")

;;; Initial alignments of cities.

(unit "Athens" (s 1))
(unit "Abydos" (s 1))
(unit "Amphipolis" (s 1))
(unit "Brauron" (s 1))
(unit "Byzantium" (s 1))
(unit "Chalcis" (s 1))
(unit "Chios" (s 1))
(unit "Corcyra" (s 1))
;(unit "Larisa" (s 1))
(unit "Marathon" (s 1))
(unit "Naupactus" (s 1))
(unit "Naxos" (s 1))
(unit "Olynthus" (s 1))
(unit "Paros" (s 1))
;(unit "Pela" (s 1))
(unit "Sestos" (s 1))
(unit "Thoricus" (s 1))
;; (should put army near Potidaea)

(unit "Sparta" (s 2))
(unit "Elis" (s 2))
(unit "Corinth" (s 2))
(unit "Gythium" (s 2))
(unit "Kythera" (s 2))
(unit "Megara" (s 2))
(unit "Olympia" (s 2))
;(unit "Potidaea" (s 2))
(unit "Pylos" (s 2))
(unit "Tegea" (s 2))
(unit "Thebes" (s 2))

(unit "Sardis" (s 3))
(unit "Ancyra" (s 3))
(unit "Magnesia" (s 3))
(unit "Pergamum" (s 3))

;; Cities to get rid of.

(unit "Alexandria" (hp 0))
(unit "Antioch" (hp 0))
(unit "Carthage" (hp 0))
(unit "Gaza" (hp 0))
(unit "Heliopolis" (hp 0))
(unit "Leptis" (hp 0))
(unit "Memphis" (hp 0))
(unit "Naucratis" (hp 0))
(unit "Pelusium" (hp 0))
(unit "Rome" (hp 0))
(unit "Ruspe" (hp 0))
;(unit "" (hp 0))
