(game-module "pelops"
  (title "Peloponnesian War")
  (blurb "Athens vs Sparta, for control of Greece")
  (base-module "greek")
  (variants
   (see-all false)
   ;; not realistic, more for debugging
   ("Wipeout" wipeout
     (true (scorekeeper (do last-side-wins))))
   )
)

(set terrain-seen true)

;; Suppress random startups (should hack synthesis list)
(add u* start-with 0)
(add u* independent-near-start 0)
(table independent-density (u* t* 0))

(area 101 49 (restrict 200 100 30 32))

;; One of the interesting properties of the Pelopponnesian War was that
;; it was really a three-sided contest, with Persia always waiting in
;; the wings, looking for opportunities to influence events.

(side 1 (noun "Athenian"))

(side 2 (noun "Spartan"))

(side 3 (noun "Persian"))

(include "u-greek")

;;; Initial alignments of cities.

(unit "Abdera" (s 1))
(unit "Abydos" (s 1))
(unit "Aegina" (s 1))
(unit "Amphipolis" (s 1))
(unit "Andros" (s 1))
(unit "Athens" (s 1))
(unit "Brauron" (s 1))
(unit "Byzantium" (s 1))
(unit "Calchedon" (s 1))
(unit "Catana" (s 1))
(unit "Chalcis" (s 1))
(unit "Chios" (s 1))
(unit "Corcyra" (s 1))
(unit "Cos" (s 1))
(unit "Elaea" (s 1))
(unit "Ephesus" (s 1))
(unit "Erythrae" (s 1))
(unit "Halicarnassus" (s 1))
(unit "Ialysus" (s 1))
(unit "Ilium" (s 1))
(unit "Lemnos" (s 1))
(unit "Leontini" (s 1))
(unit "Lindos" (s 1))
(unit "Melos" (s 1))
(unit "Methone" (s 1))
(unit "Methymna" (s 1))
(unit "Miletus" (s 1))
(unit "Myndus" (s 1))
(unit "Mytilene" (s 1))
(unit "Naupactus" (s 1))
(unit "Naxos" (s 1))
(unit "Olynthus" (s 1))
(unit "Parium" (s 1))
(unit "Paros" (s 1))
(unit "Perinthus" (s 1))
(unit "Plataeae" (s 1))
(unit "Same" (s 1))
(unit "Samos" (s 1))
(unit "Sestos" (s 1))
(unit "Thasos" (s 1))
(unit "Zacynthus" (s 1))
;; (should put army near Potidaea)

(unit "Sparta" (s 2))
(unit "Aegae" (s 2))
(unit "Ambracia" (s 2))
(unit "Anactorium" (s 2))
(unit "Corinth" (s 2))
(unit "Elis" (s 2))
(unit "Epidaurus" (s 2))
(unit "Gythium" (s 2))
(unit "Kythera" (s 2))
(unit "Lepreum" (s 2))
(unit "Megara" (s 2))
(unit "Olympia" (s 2))
(unit "Pella" (s 2))
(unit "Pharae" (s 2))
(unit "Potidaea" (s 2))
(unit "Pylos" (s 2))
(unit "Sicyon" (s 2))
(unit "Syracuse" (s 2))
(unit "Tegea" (s 2))
(unit "Thebes" (s 2))
(unit "Therma" (s 2))
(unit "Thyrea" (s 2))
(unit "Troezen" (s 2))

(unit "Anaea" (s 3))
(unit "Ancyra" (s 3))
(unit "Sardis" (s 3))
(unit "Magnesia" (s 3))
(unit "Nysa" (s 3))
(unit "Pergamum" (s 3))
;(unit "Thalles" (s 3))

;; Get rid of cities of later times.

(unit "Megalopolis" (hp 0))
