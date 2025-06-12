(game-module "ww2-42"
  (title "WWII Jan 42")
  (blurb "Full WWII, January 1942.")
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
;;; (should include the "minor powers" eventually)
;;; (should use symbolic names)

(side 1 (name "UK") (emblem-name "flag-uk"))
(side 2 (name "France") (emblem-name "flag-france") (active false))
(side 3 (name "USA") (emblem-name "flag-usa"))
(side 4 (name "Germany") (emblem-name "flag-swastika"))
(side 5 (name "Italy") (emblem-name "flag-italy"))
(side 6 (name "Japan") (emblem-name "flag-japan"))
(side 7 (name "USSR") (emblem-name "soviet-star"))
(side 8 (name "China") (emblem-name "flag-china"))
(side 9 (name "Spain") (emblem-name "flag-spain") (active false))

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

;;; Define the people.

;(include "p-e1-1938")

;;; Define the cities.

(include "u-e1-1938")

;;; Modify for territorial changes from 1938.

(unit "Copenhagen" (s de))
(unit "Rotterdam" (s de))
(unit "Warsaw" (s de))
(unit "Brussels" (s de))
(unit "Prague" (s de))
(unit "Vienna" (s de))
(unit "Budapest" (s de))
(unit "Zagreb" (s de))
(unit "Bucharest" (s de))
(unit "Thessaloniki" (s de))
(unit "Athens" (s de))
(unit "Bordeaux" (s de))
(unit "Le Havre" (s de))
(unit "Lyon" (s de))
(unit "Marseilles" (s de))
(unit "Nantes" (s de))
(unit "Nice" (s de))
(unit "Toulouse" (s de))
(unit "Paris" (s de))

;; Need Pacific theater changes also.

;; (add minor countries also?)

;; (set up agreements applying to particular dates)

(scorekeeper (do last-side-wins))

(set initial-date "Jan 1942")

;;; Set up January 1942 alliances.
