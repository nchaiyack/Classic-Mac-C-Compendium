(game-module "ww2-eur-42"
  (title "WWII (Adv), Europe Jan 42")
  (blurb "The situation in Europe, January 1942")
  (base-module "ww2-adv")
  (variants (see-all true))
  )

;; No random syntheses needed.

(set synthesis-methods nil)

;(set terrain-already-seen true)

(include "eur-100km")

(area (people-sides
  "a67l22a"
  "26lb10l4d7a10lfl9f21a"
  "25lb11l4d6a7l4df2l9f20a"
  "25l3b9l3dl7a7l4d12f19a"
  "25l3b10l2d2l6a2la2ldl8d9f18a"
  "26l2b13ldl5a5ldl9d9f17a"
  "27l3b11l2dl2a7l12d9f16a"
  "24l2b2l3b11l3d8l13d9f15a"
  "24la2b2l4b9l2d2ld2l2dl14d10f14a"
  "24l3a2l5b5l28d10f13a"
  "24l3a2l6b3l30d10f12a"
  "25la5l4b2l32d10f11a"
  "30l2b4l33d11f10a"
  "33ldl35d11f9a"
  "31l42d9f8a"
  "c32l11d3a26dl9f7a"
  "c34l9d3a4e18dld3l8flf6a"
  "2c33l9d7el15d3l2dfl9flf5a"
  "3c33l9d6e2l14d4lf3l9flf4a"
  "4c26l8g8d3l3e3l12d10l8flf3a"
  "5c25l12g6ld2l3e3l9dh12l7flf2a"
  "6c25l2a9g7le2l4e2l7d3h3l4h5l7flfa"
  "7c24l2a8g8l2e3l4el5d2lhl17h3blf"
  "a6c24l2a7g3lg6le5le3l3d5l18h4b"
  "2a5c11la13l2a6g17le4l3d2lhl17h4b"
  "3a4c10la15la6g17le4l2d2lel19h3b"
  "4a3c28lb9l5a3e2l3e6ld5lhlhl2h2lh3a5h3b"
  "5a2c35l8a3e3l2e8l2dle8l5a7b"
  "6ac29l3g12a2e22l2b2l3a8b"
  "7a28l16a3e4lb20l2a9b"
  "8a27l17a3e9l3e12l11b"
  "9a26l17a6e6l5e5l2b3l11b"
  "10a19lg4l19a8e3l6e20b"
  "11a24l18a18e11b5l3b"
  "12a23l19a17e8blb8lb"
  "13a18lg3l19a18e8b10l"
  "14a16lg3l21a17e9b9l"
  "15a18l22a18e9b8l"
  "16a17l23a17e10b7l"
  "17a16l23a18e9b7l"
  "18a15l24a17e10b6l"
  "19a14l24a18e10b5l"
  "20a13l25a17e11b4l"
  "21a13l24a18e11b3l"
  "22a68l"
))

(side 1 (name "UK") (emblem-name "flag-uk") (trusts 2))
(side 2 (name "USA") (emblem-name "flag-usa"))
(side 3 (name "Germany") (emblem-name "flag-swastika"))
(side 4 (name "Italy") (emblem-name "flag-italy") (controlled-by 3))
(side 5 (name "USSR") (emblem-name "soviet-star"))
(side 6 (name "Spain") (emblem-name "flag-spain") (active false))
(side 7 (name "Turkey") (emblem-name "flag-turkey") (active false))

;; (add minor countries also?)

;; (set up agreements applying to particular dates)

(set initial-date "Jan 1942")

(infantry 37 32 3)
(infantry 37 33 3)
(infantry 31 30 3)
(infantry 72 29 3)
(infantry 71 29 3)
(infantry 71 27 3)
(infantry 68 32 3)
(infantry 67 34 3)
(infantry 65 36 3)
(infantry 64 38 3)
(infantry 62 40 3)
(infantry 61 40 3)
(armor 60 40 3)
(armor 63 39 3)
(armor 70 27 3)
(armor 72 30 3)
(armor 70 30 3)
(armor 69 31 3)
(armor 68 33 3)
(armor 66 35 3)
(armor 65 37 3)
(infantry 59 40 3)
(infantry 58 41 3)
(infantry 57 42 3)

(infantry 65 39 5)
(infantry 60 41 5)
(infantry 74 30 5)
(infantry 73 31 5)
(infantry 68 35 5)
(infantry 67 36 5)
(infantry 67 37 5)
(infantry 66 38 5)
(infantry 74 29 5)
(infantry 72 27 5)
(infantry 62 41 5)
(infantry 72 31 5)
(infantry 70 31 5)
(infantry 70 32 5)
(infantry 68 34 5)
(infantry 66 37 5)
(infantry 65 38 5)
(infantry 63 40 5)
(infantry 61 41 5)
(infantry 59 41 5)
(armor 64 39 5)
(armor 71 31 5)
(armor 69 33 5)
(armor 66 36 5)


(town 36 13 0 (n "Marrakech"))
(base 79 14 0 (n "Haifa"))
(town 80 14 0 (n "Jerusalem"))
(base 36 16 6 (n "Tangiers"))
(base 79 18 0 (n "Aleppo"))
(town 76 19 0 (n "Adana"))
(town 87 20 0 (n "Tabriz"))
(town 32 20 0 (n "Lisbon"))
(town 26 35 0 (n "Dublin"))
(town 35 15 0 (n "Casablanca"))
(town 42 17 0 (n "Oran"))
(town 45 18 0 (n "Algiers"))

(base 34 33 1 (n "Dover"))
(base 35 18 1 (n "Gibraltar"))
(base 58 15 1 (n "Malta"))
(base 26 43 1 (n "Scapa Flow"))
(base 34 34 1 (n "Southend"))
(base 77 12 1 (n "Port Said"))
(base 78 11 1 (n "Suez"))
(town 74 13 1 (n "Alexandria"))
(town 80 6 1 (n "Aswan"))
(town 78 8 1 (n "Asyut"))
(town 25 37 1 (n "Belfast"))
(town 31 34 1 (n "Bristol"))
(town 76 10 1 (n "Cairo"))
(town 30 34 1 (n "Cardiff"))
(town 28 38 1 (n "Edinburgh"))
(town 31 36 1 (n "Hull"))
(town 30 37 1 (n "Newcastle"))
(city 30 35 1 (n "Birmingham"))
(city 27 38 1 (n "Glasgow"))
(city 29 36 1 (n "Manchester"))
(capital 33 34 1 (n "London"))

(city 2 26 2 (n "Boston"))
(city 6 19 2 (n "Houston"))
(city 6 21 2 (n "New Orleans"))
(city 3 25 2 (n "New York"))
(city 1 25 2 (n "Chicago"))
(city 4 24 2 (n "Philadelphia"))
(city 3 24 2 (n "Pittsburgh"))
(city 2 24 2 (n "Detroit"))
(city 2 22 2 (n "St. Louis"))
(capital 5 23 2 (n "Washington"))

(town 64 20 3 (n "Athens"))
(town 58 26 3 (n "Belgrade"))
(town 36 27 3 (n "Bordeaux"))
(town 38 32 3 (n "Brussels"))
(town 64 26 3 (n "Bucharest"))
(town 55 29 3 (n "Budapest"))
(town 54 32 3 (n "Cracow"))
(town 51 35 3 (n "Danzig"))
(town 42 33 3 (n "Dortmund"))
(town 41 33 3 (n "Essen"))
(town 44 32 3 (n "Frankfurt"))
(town 53 36 3 (n "Konigsberg"))
(town 58 32 3 (n "L'vov"))
(town 36 32 3 (n "Le Havre"))
(town 47 32 3 (n "Leipzig"))
(town 41 27 3 (n "Lyon"))
(town 43 25 3 (n "Marseilles"))
(town 60 35 3 (n "Minsk"))
(town 47 30 3 (n "Munich"))
(town 34 29 3 (n "Nantes"))
(town 45 25 3 (n "Nice"))
(town 67 28 3 (n "Odessa"))
(town 40 42 3 (n "Oslo"))
(town 49 31 3 (n "Prague"))
(town 55 38 3 (n "Riga"))
(town 62 24 3 (n "Sofia"))
(town 45 30 3 (n "Stuttgart"))
(town 63 22 3 (n "Thessaloniki"))
(town 39 25 3 (n "Toulouse"))
(town 52 30 3 (n "Vienna"))
(town 58 36 3 (n "Vilnius"))
(town 55 34 3 (n "Warsaw"))
(town 53 27 3 (n "Zagreb"))
(city 44 37 3 (n "Copenhagen"))
(city 42 35 3 (n "Hamburg"))
(city 65 32 3 (n "Kiev"))
(city 38 34 3 (n "Rotterdam"))
(capital 47 34 3 (n "Berlin"))
(capital 37 30 3 (n "Paris"))

(town 64 13 4 (n "Benghazi"))
(town 46 26 4 (n "Genoa"))
(town 55 18 4 (n "Palermo"))
(town 44 27 4 (n "Torino"))
(town 60 12 4 (n "Tripoli"))
(town 52 18 4 (n "Tunis"))
(city 47 27 4 (n "Milan"))
(capital 52 23 4 (n "Rome"))

(town 81 28 5 (n "Astrakhan"))
(town 86 23 5 (n "Baku"))
(town 80 23 5 (n "Batum"))
(town 67 35 5 (n "Br'ansk"))
(town 69 30 3 (n "Dnepropetrovsk"))
(town 68 39 5 (n "Jaroslavi"))
(town 73 37 5 (n "Kazanh"))
(town 69 32 5 (n "Kharkov"))
(town 75 27 5 (n "Maikop"))
(town 71 26 5 (n "Sevastopol"))
(town 77 31 5 (n "Stalingrad"))
(town 71 30 5 (n "Stalino"))
(town 83 23 5 (n "Tbilisi"))
(city 71 38 5 (n "Gorky"))
(city 75 35 5 (n "Kuybyshev"))
(city 58 42 5 (n "Leningrad"))
(city 73 30 5 (n "Rostov"))
(capital 67 38 5 (n "Moscow"))

(base 35 20 6 (n "Cordoba"))
(base 30 8 6 (n "Las Palmas"))
(town 41 23 6 (n "Barcelona"))
(town 35 25 6 (n "Bilbao"))
(town 31 23 0 (n "Porto"))
(town 39 21 6 (n "Valencia"))
(town 38 23 6 (n "Zaragoza"))
(capital 36 22 6 (n "Madrid"))

(town 72 21 7 (n "Ankara"))
(town 81 21 7 (n "Erzurum"))
(town 68 20 7 (n "Izmir"))
(town 84 21 7 (n "Jerevan"))
(city 67 23 7 (n "Istanbul"))

;(town 88 3 0  (n "Mecca")) ; a distraction to Egyptian forces

