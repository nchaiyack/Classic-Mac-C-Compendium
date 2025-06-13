(game-module "ww2-38"
  (title "WWII Jan 38")
  (blurb "Full WWII, January 1938.  This is a monster game, based on the advanced ww2 unit types.")
  (base-module "ww2-adv")
  (variants
   (see-all true)
   )
  (instructions (
   "In this game you play the national leader of your country during WWII."
   ))
  )

;;; Define basic terrain.

(include "earth-1deg")

(set synthesis-methods nil)

;;; All the participants and major neutrals.
;;; (should include all the "minor powers" eventually)

(side 1 (name "UK") (emblem-name "flag-uk"))
(side 2 (name "France") (emblem-name "flag-france"))
(side 3 (name "USA") (emblem-name "flag-usa"))
(side 4 (name "Germany") (emblem-name "flag-swastika"))
(side 5 (name "Italy") (emblem-name "flag-italy"))
(side 6 (name "Japan") (emblem-name "flag-japan"))
(side 7 (name "USSR") (emblem-name "soviet-star"))
(side 8 (name "China") (emblem-name "flag-china"))
(side 9 (name "Spain") (emblem-name "flag-spain"))

;;; Two-letter codes for sides.

(define uk 1)
(define fr 2)
(define us 3)
(define de 4)
(define it 5)
(define jp 6)
(define su 7)
(define zh 8)
(define es 9)

;;; Define the nationalities of the people.

(include "p-e1-1938")

;;; Define the cities.

(include "u-e1-1938")

;; (add minor countries also?)

;; (set up agreements applying to particular dates)

;; not really correct

(scorekeeper (do last-side-wins))

;; Default to setting up at the beginning of 1938.

(set initial-date "Jan 1938")
