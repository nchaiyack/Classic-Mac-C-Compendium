(game-module "hobbit-towns"
  (blurb "towns etc. for Western Lands in Middle Earth, version 1.0")
  (notes "a side is defeated if it doesn't hold a town or two villages")
)

;(include "hobbit")
(include "midearth")

;Globals 0 1000 1 0 1 0 
;0 0 1 1000 -1
;20

(side (name "Mordor") (adjective "Sauron's slave") (class "evil"))
(side (name "The Shire") (plural-noun "Shire folk") (class "good"))
(side (plural-noun "Lake people") (class "good"))
(side (name "Rohan") (noun "Rohirrim") (class "good"))
(side (name "Gondor") (noun "Gondorian") (class "good"))
(side (adjective "Pirate") (class "evil"))

(@ (n "Barad-dur") (@ 44 28) (s 1))
(@ (n "Michel Delving") (@ 75 51) (s 2))
(@ (n "Esgaroth") (@ 25 57) (s 3))
(@ (n "Edoras") (@ 24 30) (s 4))
(@ (n "Minas Tirith") (@ 37 24) (s 5))
(@ (n "Umbar") (@ 39 2) (s 6))

(@ (n "Bree") (@ 3 52))
(@ (n "Caras Galadhon") (@ 20 42))
(@ (n "Dol Amroth") (@ 27 20))
(@ (n "Minas Morgul") (@ 40 24))
(@ (n "Mithlond") (@ 70 53))
(@ (n "Nurnen") (@ 53 21))
(@ (n "Pelargir") (@ 38 19))
(@ (n "Rhun") (@ 46 46))
(@ (n "Rivendell") (@ 13 53))
(@ (n "Tharbad") (@ 7 45))
(* (n "Arnach") (@ 10 29))
(* (n "Brandy Hall") (@ 0 52))
(* (n "Calembel") (@ 28 23))
(* (n "Carn Dum") (@ 1 66))
(* (n "Dale") (@ 24 59))
(* (n "Dol Guldur") (@ 25 43))
(* (n "Dunharrow") (@ 25 27))
(* (n "Durthang") (@ 39 28))
(* (n "Egladil") (@ 19 44))
(* (n "Elvenking's Halls") (@ 22 58))
(* (n "Fornost") (@ 79 57))
(* (n "Grimslade") (@ 11 37))
(* (n "Harad Outpost") (@ 59 2))
(* (n "Helm's Deep") (@ 21 31))
(* (n "Hobbiton") (@ 77 52))
(* (n "Imloth Melui") (@ 37 12))
(* (n "Isengard") (@ 19 34))
(* (n "Khand") (@ 61 14))
(* (n "Lamedon") (@ 14 19))
(* (n "Langstrand") (@ 22 21))
(* (n "Linhir") (@ 33 19))
(* (n "Morthond") (@ 24 25))
(* (n "Rhosgobel") (@ 27 49))
(* (n "Staddle") (@ 3 54))
(* (n "Tuckborough") (@ 77 50))
(* (n "Undertowers") (@ 74 53))
(* (n "Woodmen's  ") (@ 21 52))
(/ (n "Erebor") (@ 23 59))
(/ (n "Ered Luin") (@ 71 50))
(/ (n "Ered Mithrin") (@ 19 61))
(/ (n "Iron Hills") (@ 35 58))
(/ (n "Moria") (@ 16 44))
(/ (n "Moria West Gate") (@ 15 45))
(/ (n "Orodruin") (@ 42 27))
(b (n "Brandywine Bridge") (@ 79 53))
(b (n "Carrock Ford") (@ 17 54))
(b (n "Ford of Bruinen") (@ 12 52))
(b (n "Harad Bridge") (@ 49 9))
(b (n "Osgiliath") (@ 38 24))
(b (n "Sarn Ford") (@ 2 48))
(b (n "Tharbad Bridge") (@ 8 44))
(b (n "The Last Bridge") (@ 9 52))
