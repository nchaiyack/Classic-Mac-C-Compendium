(game-module "ring-quest"
  (title "")
  (blurb "")
  (variants 
    (see-all false)
    (world-seen true)
  )
)

(include "u-midearth")

;; Constrain construction.

(table tp-to-build
  (u* hobbit 1)
  (u* dwarf 1)
  (u* elf 1)
  )

(side 2 (trusts 4 1))

(side 4 (name "Gondor") (adjective "Gondorian") (class "good")
  (emblem-name "tree-and-crown"))

(side 4 (trusts (2 1)))

(side 5 (name "Isengard") (adjective "Isengardian") (class "evil")
  )

(side 5 (trusts (1 1)))

(unit-defaults (s 2))

(hobbit 24 51 (n "Frodo"))
(hobbit 24 51 (n "Sam"))
(hobbit 24 51 (n "Pippin"))
(hobbit 24 51 (n "Merry"))

(wizard 24 50 (n "Gandalf"))

(unit "Brandy Hall")

(unit "Hobbiton")

(the-ring (in "Frodo"))

(unit-defaults (s 4))

(unit "Calembel")
(unit "Dol Amroth")
(unit "Lamedon")
(unit "Langstrand")
(unit "Linhir")
(unit "Minas Tirith")
(unit "Morthond")
(unit "Pelargir")

(unit-defaults (s 5))

(unit "Isengard")

(wizard (n "Saruman") (in "Isengard"))
(unit-defaults (s 1))

;; 9 nazgul

(nazgul)
(nazgul)
(nazgul)
(nazgul)
(nazgul)
(nazgul)
(nazgul)
(nazgul)
(nazgul)

(unit "Minas Morgul")
(unit "Durthang")
(unit "Orodruin")
