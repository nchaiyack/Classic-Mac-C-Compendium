(game-module "1805"
  (title "Napoleon's Austrian campaign, 1805")
  (base-module "napoleon")
  (variants (see-all true))
  (notes (
  "The French must try to capture Vienna, the Austrians must try"
  "to keep it, all within 5 months (turns)."
  ))
  )

;; No unknown terrain in Europe at this time.

(set terrain-seen true)

(include "eur-50km")

(add u* assign-number false)

(table acp-to-create (u* u* 0))

(add u* start-with 0)

(side 1 (name "France") (adjective "French") (emblem-name "flag-france"))
(side 2 (name "England") (adjective "English") (emblem-name "flag-uk")
  (active false))
(side 3 (name "Spain") (adjective "Spanish") (emblem-name "flag-spain")
  (active false))
(side 4 (name "Austria") (adjective "Austrian") (emblem-name "arms-austria"))
(side 5 (name "Prussia") (adjective "Prussian") (emblem-name "arms-prussia")
  (active false))
(side 6 (name "Russia") (adjective "Russian") (emblem-name "arms-russia"))

(set sides-min 6)
(set sides-max 6)

(set initial-date "July 1805")

(set last-turn 5)

(add u* point-value 0)
(add capital point-value 1)

;;; Owner of Vienna only should be the winner.

(scorekeeper 
  (title "VP")
  (initial 0)
  (do (set (sum point-value)))
  )

(capital 70 31 0 (n "Rome"))
(capital 56 69 0 (n "Stockholm"))
(inf 56 55 1)
(inf 49 55 1)
(inf 49 55 1)
(ldr 49 55 1)
(ldr 61 49 1)
(ldr 61 50 1)
(inf 61 50 1)
(cav 61 50 1)
(ldr 72 44 1)
(ldr 65 43 1)
(inf 57 42 1)
(inf 57 49 1)
(inf 38 48 1  (hp 4))
(inf 67 34 1 (hp 4))
(inf 53 50 1)
(inf 63 38 1(hp 3))
(inf 55 49 1 (hp 8))
(inf 53 53 1 (hp 3))
(inf 56 51 1 (hp 3))
(guards 56 49 1  (hp 2))
(cav 54 51 1)
(cav 55 49 1)
(cav 55 49 1)
(ldr 53 50 1 (n "Davout"))
(ldr 53 51 1 (n "Soult"))
(ldr 63 38 1 (n "Massena"))
(ldr 53 53 1 (n "Bernadotte"))
(ldr 55 49 1 (n "Lannes"))
(ldr 55 48 1 (n "Napoleon"))
(ldr 56 49 1 (n "Ney"))
(ldr 56 51 1 (n "Marmont"))
(city 57 45 1 (n "Basel"))
(city 50 36 1 (n "Toulouse"))
(city 43 45 1 (n "Nantes"))
(city 38 48 1 (n "Brest"))
(city 46 52 1 (n "Calais"))
(city 45 49 1 (n "Le Havre"))
(city 41 50 1 (n "Cherbourg"))
(city 50 52 1 (n "Brussels"))
(city 57 49 1 (n "Stuttgart"))
(city 57 42 1 (n "Geneva"))
(city 61 50 1 (n "Wurzburg"))
(city 57 54 1 (n "Hanover"))
(city 67 34 1 (n "Florence"))
(city 54 50 1 (n "Metz"))
(city 63 38 1 (n "Milan"))
(city 48 51 1 (n "Lille"))
(city 52 49 1 (n "Rheims"))
(city 49 55 1 (n "Amsterdam"))
(city 56 48 1 (n "Strasbourg"))
(city 53 53 1 (n "Cologne"))
(city 56 51 1 (n "Coblenz"))
(capital 49 48 1 (n "Paris"))
(city 41 58 2 (n "Norwich"))
(city 43 53 2 (n "Dover"))
(city 38 53 2 (n "Plymouth"))
(city 41 53 2 (n "Portsmouth"))
(capital 42 54 2 (n "London"))
(capital 42 31 3 (n "Madrid"))
(cav 59 47 4)
(cav 60 47 4)
(cav 65 43 4)
(inf 65 43 4 (hp 3))
(inf 72 44 4)
(inf 68 48 4 (hp 1))
(inf 66 47 4 (hp 1))
(inf 66 38 4)
(inf 66 38 4 (hp 4))
(inf 60 48 4)
(cav 60 48 4)
(ldr 66 38 4 (n "Charles"))
(ldr 66 38 4 (n "Hiller"))
(ldr 60 48 4 (n "Ferdinand"))
(ldr 60 48 4 (n "Mack"))
(city 67 38 4 (n "Venice"))
(city 68 48 4 (n "Prague"))
(city 65 43 4 (n "Innsbruck"))
(city 66 47 4 (n "Ratisbon"))
(city 59 48 4 (n "Ulm"))
(capital 63 47 4 (n "Munich"))
(capital 78 42 4 (n "Budapest"))
(capital 72 44 4 (n "Vienna"))
(inf 62 54 5 (hp 1))
(capital 62 54 5 (n "Berlin"))
(guards 74 55 6 (hp 2))
(cav 75 51 6)
(ldr 74 51 6 (n "Kutuzov"))
(inf 74 51 6 (hp 7))
(ldr 74 51 6 (n "Bagration"))
(cav 74 57 6)
(ldr 73 57 6 (n "Benningsen"))
(inf 73 57 6 (hp 3))
(cav 75 54 6)
(ldr 74 54 6 (n "Buxhowden"))
(ldr 74 54 6 (n "Constantine"))
(inf 74 54 6)
(city 78 47 6 (n "Lvov"))
(city 70 59 6 (n "Kovno"))
(city 68 60 6 (n "Memel"))
(city 73 57 6 (n "Grodno"))
(city 74 51 6 (n "Lublin"))
(city 74 54 6 (n "Brest-Litovsk"))
(capital 86 62 6 (n "Moscow"))
#|
(capital 56 69 0 (n "Stockholm"))
(capital 70 31 0 (n "Rome"))
(city 57 45 1 (n "Basel"))
(city 50 36 1 (n "Toulouse"))
(city 43 45 1 (n "Nantes"))
(city 38 48 1 (n "Brest"))
(inf 38 48 1 (hp 4))
(city 46 52 1 (n "Calais"))
(city 45 49 1 (n "Le Havre"))
(city 41 50 1 (n "Cherbourg"))
(city 50 52 1 (n "Brussels"))
(city 57 49 1 (n "Stuttgart"))
(city 57 42 1 (n "Geneva"))
(city 61 50 1 (n "Wurzburg"))
(city 57 54 1 (n "Hanover"))
(city 67 34 1 (n "Florence"))
(inf 67 34 1 (hp 4))
(city 54 50 1 (n "Metz"))
(inf 53 50 1)
(cavalry 54 51 1)
(ldr 53 50 1 (n "Davout"))
(ldr 53 50 1 (n "Soult"))
(city 63 38 1 (n "Milan"))
(inf 63 38 1 (hp 3))
(ldr 63 38 1 (n "Massena"))
(ldr 53 54 1 (n "Bernadotte"))
(ldr 55 49 1 (n "Lannes"))
(ldr 55 49 1 (n "Napoleon"))
(ldr 56 49 1 (n "Ney"))
(inf 55 49 1 (hp 8))
(guards 56 49 1 (hp 2))
(city 48 51 1 (n "Lille"))
(city 52 49 1 (n "Rheims"))
(city 49 55 1 (n "Amsterdam"))
(city 56 48 1 (n "Strasbourg"))
(cavalry 55 49 1)
(cavalry 55 49 1)
(city 53 53 1 (n "Cologne"))
(inf 53 53 1 (hp 3))
(city 56 51 1 (n "Coblenz"))
(ldr 56 51 1 (n "Marmont"))
(inf 56 51 1 (hp 3))
(capital 49 48 1 (n "Paris"))

(city 41 58 2 (n "Norwich"))
(city 43 53 2 (n "Dover"))
(city 38 53 2 (n "Plymouth"))
(city 41 53 2 (n "Portsmouth"))
(capital 42 54 2 (n "London"))
(capital 42 31 3 (n "Madrid"))
(capital 63 47 4 (n "Munich"))
(capital 78 42 4 (n "Budapest"))
(city 67 38 4 (n "Venice"))
(inf 66 38 4)
(inf 66 38 4 (hp 4))
(ldr 66 38 4 (n "Charles"))
(ldr 66 38 4 (n "Hiller"))
(city 68 48 4 (n "Prague"))
(city 65 43 4 (n "Innsbruck"))
(city 66 47 4 (n "Ratisbon"))
(city 59 48 4 (n "Ulm"))
(inf 60 48 4)
(cavalry 60 48 4)
(ldr 60 48 4 (n "Ferdinand"))
(ldr 60 48 4 (n "Mack"))
(capital 72 44 4 (n "Vienna"))
(capital 62 54 5 (n "Berlin"))
(city 78 47 6 (n "Lvov"))
(city 70 59 6 (n "Kovno"))
(city 68 60 6 (n "Memel"))
(city 73 57 6 (n "Grodno"))
(city 74 51 6 (n "Lublin"))
(city 74 54 6 (n "Brest-Litovsk"))
(capital 86 62 6 (n "Moscow"))
|#
