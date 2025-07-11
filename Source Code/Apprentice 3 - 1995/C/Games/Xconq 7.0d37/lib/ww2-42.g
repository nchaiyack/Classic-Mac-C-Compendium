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

;;; Define the people.

(area (people-sides
  "360l"
  "360l"
  "360l"
  "136l9a138h77l"
  "136l9a139h76l"
  "134l12a139hl2h25l18d27b2l"
  "22l4b106l2e12a145h22l18dl27bl"
  "b20l6b103l3e6a2l6a147h20l10dldl5d28b"
  "27b102l4e5a3l6a148h20l5dld2l8d28b"
  "27b101l4e6a4l6a147h18l20d27b"
  "27b100l5e5a4l7a143h22l6dl4dl8d27b"
  "25b15l6b80l6e5a4l7a134h4l4h24l11dl6dl4dl22b"
  "25b16l7b78l6e6a4l6a128h2l5h4l3h25l25d3bl16b"
  "25b17l8b45la30l6e6a10le125h5l4h33l11d3l12d18b"
  "25b18l8b4lb38l3a29l7e5a13l124h5l3h35l7d3l3d9l5d15b"
  "26b17l10b2l3b18la18l2a19l2b9l3e3l5a5lel9e108h16l5h41l4d16ldld13b"
  "28b16l15b56l4b12lel5a5lel9e105h18l7h25ld13l2d20ld2l12b"
  "34b8lb2l16b56l2b12l2e2l3a5l16e98h21l7h37l4d2l2d16ldld11b"
  "36b8l18b52l2bl4b10l4e8l16e98h2lh17l7h35l4d25l2dl3bl5b"
  "41b3l21b49la2b2l4b9le6l3el16e100h19l6h4lhlh27l2d31l9b"
  "41b3l23b47l3a3l3b6l32e103hlh12l5h31ldld33lb2l7b"
  "43b2l23b46l3a2l5b4l34e103hlh12l3h15ld11ldldld38lb2l5b"
  "44bl22b48la4l5b2l35e74h2a27hlh13lh22ldldld48l4b"
  "66b2lb50l3b5l36e70h6a10g16h2l2h11lh76l3b"
  "36b3l22b6l4b52lel38e68h8a17g9h2l2h89l2b"
  "4l31d5l22blb3l5b50l41e64h12a19g8h2lh91l"
  "2ldl31d2ld4l20b5l6b50l11e3a27e63h12a19g7h3lh9lh81l"
  "3l38d3l19b3lc58l3e8a3f20el2e3l12h3l47h12a19g7h4lh90l"
  "3l37d2l2d3l9bd9b60l2e6a6fl15e4l3e2l10h5l47h12agl17g6h13lg82l"
  "4l36d2l2d2l10b2d2b2l2b63le7a6f2l14e4le2l11h5l39h4i15a2i18g6h5l2g4l2g83l"
  "4l37dl3dl4b3l5d4lb57l8j7a4l3f2l13e10l9h4l33hl2h8i13a3i19g4h6l5g86l"
  "5l37dl3d2b2l8d62l11ja5lc3l3f3l10e12l8h5l29h4l10i11a5i19g10l3g87l"
  "5l37dl3d3l10d62l2a9j10l4f2l8e2a3l4a5l8h5l28h3l11i11a4i19g101l"
  "6l51d64l2a8j7l2f4l4flf4e2l20a3h4l27h18i7a5i10g2l7g11l2g88l"
  "6l50d65l2a7j2lj6l2f5lf3lf2e5l20a2h4l26h31i8g2l2g2l3g12l2g88l"
  "8l48d47la17l2a8j16lf4l3e2l23a5l18h2l5h31i8g7l3g11l2g88l"
  "8l48d49la17l7j17lf4l2e2lfl10a3ca3b5a5l7a12h2l4h31i9glg5l3g9l3g88l"
  "10l46d69lb8l9c2l2f7le5lalal2a2l4c5b17a10h4l2h13il18i13g3l3g7l5g87l"
  "11l45d76l11c13l2elf8l2c9b16a6h2b7l17il14i10g6l3g2l10g87l"
  "12l44d70l17c5lb16l2b2l11b16a5h5b6lil7il20i11g6lg5l2gl2g90l"
  "15l39d71l19c25l12b17a11b21l19i5g9l2gl2g92l"
  "14ld2l37d15lb47lj7l19c5f5l3f12l4b3a6b16a12b22l18i5g9l2g94l"
  "18ldl33d71l21c4f5l5f5l2b3l3b2a2l2a5b15a15b20l15i2gi5g8lg95l"
  "18l2a2la31d71l8c8l4c16f10b2a5l2a4b15a17b18l14il7g45la58l"
  "19l2a2l9a8d3l2d7l2d63lj7l7c10l3c16f10ba8l2ab2l14a20b16l13i9g103l"
  "21l2a2l8a7d13l2d64ljlj3lj4c13l3c3f11l2f7blbla9l2a3l13a22b13l13il9g7lg52ld41l"
  "21l3a2l10a3d15l2d67l3j3c15l3c2f12lfb2l5b2l2a10la4l11a27bl4b4l21i103l"
  "24l2al11a2d14l3d66l3j2c17l2c2f17l5b2l2a10l2a7l7a33bl21ig7lg95l"
  "25la2l2al8ad15l2dlb63l3j2c19l2cf18l5b2la11lala3la3l6a33b22i73ld29l"
  "26l2a2l10a20lb61l2j3c40l6b2l2a10l3a2l3a10l31b18igigl2g2lg16lg80l"
  "27l2a2l9a22lb59l2jc43l5b3l3a9lal7a9l31b14igl2g3l2g100l"
  "33l8a14l5a5lb57ljc43l6b3l3a14l5a9l22bl7b3ic10ilb5lg79l2d19l"
  "34l8a17l6a58l2c43l7b3l3a17la10l2bl15b4l7b3a5ci2li92ld17l"
  "35l8a6l4a10l3a58l2c43l7b4l2a16la14l14b5l6b4a5c97ld15l"
  "29la5l9a5l4a14l3a54l2c44l7b4la15l2a14l13b7l5b5a3c3l2i94l2d13l"
  "37l9a3l3ab10l2b2l6al2dla47l2c29l2c12l9b3l2a12l2a16l11b10l4b5a3c3l2i10ld82l3d12l"
  "40l12ab72l6c23l10c5l11b3l2a11la17l10b11l4b6a3c13l3d96l"
  "43l9ab27lb36la8l10c13l3c2l13c14b4l7al4a18l9b12l2blb6a4c12l2d96l"
  "49l9a67l42c15b3l7b2a22l6b18lb6a4c11l2d22lg73l"
  "51l9a21lc37la5l42c17b2l6b25l6b18l6a5c11l2d95l"
  "53l7a23lb42l3b18c9b12c17bl3b29l5b18l5a6c13l2d20lg72l"
  "57l4a14la6la44l20c10b11c18b33l5b13la4l2ala7c12l2dld91l"
  "58l3a10lala10lb43l3a16c10b12c18b6l2b3lb20l5b18l2a3l6c16ld90l"
  "59l3a7l4al5a2lala46l3c2b7c4b2c9b12c20b2l5b20lb4l3b14la4la5l4c11ldldl2d90l"
  "61la4la2l4al10a47l2c3b7c4bc9b13c26b25l3b2lb16la6lc12l2d5ld12la76l"
  "62l4al19a47l3ba6c4b2c8b14c25b27lb2l2b12la3l2a17ld4l4d88l"
  "65la2l18ab47l2b2a6c4bc8b15c24b30l2b17l2b21l5d25la61l"
  "70l16a3b47l3a6c4bc7b15c24b31l2b18l2b15la6l2d32la54l"
  "70l17a3b2c45l2a7cb5l5b16c23b48la2l3b12l4a94l"
  "71l17a2b2cl3a44lc12l2b2l13c5l21b48l3al3b11la2l2a93l"
  "71l17a2b7a61l10c8l19b50l3al3b9l2a2la94l"
  "71l27a61l8c10l18b52l2a2l2b8l2a3la93l"
  "70l29a60l8c2a10l15b54l3alb5l4a11la48la37l"
  "70l30a60l2j5c3a10l13b54lal3a7la7la2l5a36la48l"
  "58lala9l31ala53lj3l3j4c3a12l10b58l4a5l2a6la2la5la5l2ala74l"
  "70l36a54l3j4c2a12l2abl7b59l5a5la4l2a2l5a3la3l3a54la20l"
  "71l37a48lj4l2j4c2a12l2ab2l5b61l5ala2l6a3l3a10l2a2l5a6la2la58l"
  "71l41a51l3c4a11l2ab2l6b62l4a8l2a3lala6l2a3l3a4l3a65l"
  "71l43a5la44l2cl3a11l2a2bl6b63l3a13lal2a13l3a3l3a6la56l"
  "72l46a47l5a11l3a8b16lb48l2a13la3la14la4l3a2l4a3la51l"
  "73l45a49l3a11lala8b68la3l2a31lb3a7lala21la27l"
  "74l45a48l4a10l2a2l8b68l7a20la9l4b30la27l"
  "75l44a19lb28l4a10l2a3l8b73l2alala3la2l2a4la14l4b7la48l"
  "16la59l43a49l4a9l3abl9b80la3la23l2b10la18la25l"
  "77l42a50l3a9l4a4bl6b109lb11la42l"
  "78l41a51l2a7lbl4a5bl6b56lb32lblb9lb61l"
  "79l39a24la27l2a7l11bl6b3lc4lc81l6b5l2b60l"
  "81l37a52l2a7l12bl6a7l2c80l6b6l2b43lal2a12l"
  "82l36a52l3a6l9b3a2b5a7l2c76l2bl7b6l3b58l"
  "27la55l36a33lb18l2a7l7b5a2b4a6l4c75l11b6l3b21la35l"
  "85l34a52l3a6l9b4ab4a5l5c73l15b4l3b22la34l"
  "9la78l31a53l2a7l10b5a7l5c73l17b2l5b22la32l"
  "89l2al27a53l2a7l11b3a8l5c72l25b55l"
  "90l2a2l26a53l19b3a9l5c70l28b22la30l"
  "90l2a2l26a54l18b3a9l5c8lb60l31b15l2c34l"
  "la89l3al25a55l18b3a8l5c7lb59l34b17la32l"
  "91l2al25a56l18b4a8l4c67l35b49l"
  "91l3al22a59l18b3a9l3c67l37b47l"
  "91l3al20a62l17b3a10l2c67l38b46l"
  "92l2a2l19a63l16b2a12lc68l38b45l"
  "92l2al19a64l17ba81l39b44l"
  "54la38l2al19a64l17b82l39b43l"
  "93l22a65l16b82l40b42l"
  "94l21a67l14b84l38b14la27l"
  "94l20a68l14b84l38b42l"
  "94l20a70l12b85l38b41l"
  "94l19a71l11b87l9b9l19b41l"
  "87la7l14al3a72l10b87l9b11l16b42l"
  "95l15a76l2b95l3b16lbl14b42l"
  "95l16a196l11b23lb18l"
  "95l16a197l10b24lb17l"
  "95l16a198l9b25l2b15l"
  "95l12a205lbl2b27l4b13l"
  "96l12a235l4b13l"
  "96l10a207lb31l2b13l"
  "97l10a208l4b24l2blb13l"
  "97lal4al3a209l4b22l4b14l"
  "99l8a210l3b21l4b15l"
  "99l8a211l2b20l4b16l"
  "99lal5a234l4b16l"
  "98lal6a235l2b17l"
  "100l7a234lb18l"
  "99l8a253l"
  "99lal6a253l"
  "100l6a254l"
  "100lal5a8lblb242l"
  "102l5a253l"
  "105la2la251l"
  "104la2l4a249l"
  "106l4a30lb219l"
  "360l"
  "360l"
  "360l"
  "360l"
  "360l"
  "360l"
  "360l"
  "360l"
  "360l"
  "360l"
))

;;; Define the cities.

;; (should have more British bases - for instance at Bermuda and in West Indies)

;(include "u-e1-1938")
(base 152 78 0 (n "Kano"))
(base 95 124 0 (n "Reykjavik"))
(town 110 29 0 (n "Mar del Plata"))
(town 110 32 0 (n "Montevideo"))
(town 108 32 0 (n "Buenos Aires"))
(town 96 32 0 (n "Santiago"))
(town 97 33 0 (n "Mendoza"))
(town 94 33 0 (n "Valparaiso"))
(town 113 36 0 (n "Porto Alegre"))
(town 97 39 0 (n "Cordoba"))
(town 113 40 0 (n "Curitiba"))
(town 105 40 0 (n "Asuncion"))
(town 115 42 0 (n "Sao Paulo"))
(town 91 42 0 (n "Antofagasta"))
(town 117 43 0 (n "Rio de Janeiro"))
(town 116 46 0 (n "Belo Horizonte"))
(town 108 49 0 (n "Goiania"))
(town 89 49 0 (n "La Paz"))
(town 117 53 0 (n "Salvador"))
(town 79 53 0 (n "Lima"))
(base 183 54 0 (n "Elizabethville"))
(base 167 57 0 (n "Luanda"))
(town 118 57 0 (n "Recife"))
(town 266 58 6 (n "Surabaya"))
(town 260 59 6 (n "Jakarta"))
(base 272 60 6 (n "Ujung Pendang"))
(base 166 60 0 (n "Matadi"))
(base 167 61 0 (n "Kinshasa"))
(town 113 61 0 (n "Fortaleza"))
(base 266 62 6 (n "Banjarmasin"))
(base 91 62 0 (n "Manaus"))
(base 78 62 0 (n "Iquitos"))
(town 71 63 0 (n "Guayaquil"))
(base 253 64 0 (n "Palembang"))
(town 102 64 0 (n "Belem"))
(town 71 65 0 (n "Quito"))
(base 168 66 0 (n "Mboudaka"))
(base 246 69 0 (n "Medan"))
(town 71 69 0 (n "Cali"))
(town 73 70 0 (n "Bogota"))
(town 71 72 0 (n "Medellin"))
(base 143 73 0 (n "Kumasi"))
(base 73 73 0 (n "Cucuta"))
(base 66 74 3 (n "Panama Canal"))
(base 131 75 0 (n "Conakry"))
(town 78 76 0 (n "Caracas"))
(town 72 76 0 (n "Maracaibo"))
(town 69 76 0 (n "Baranquilla"))
(base 57 78 0 (n "Managua"))
(base 243 79 0 (n "Bangkok"))
(base 53 79 0 (n "San Salvador"))
(base 55 80 0 (n "Tegucigalpa"))
(base 71 84 0 (n "Santo Domingo"))
(base 68 84 0 (n "Port-au-Prince"))
(town 43 85 0 (n "Veracruz"))
(town 40 85 0 (n "Mexico City"))
(town 64 86 0 (n "Santiago de Cuba"))
(town 36 86 0 (n "Guadalajara"))
(base 178 87 0 (n "Mecca"))
(town 56 88 0 (n "La Habana"))
(town 37 88 0 (n "San Luis Potosi"))
(town 36 91 0 (n "Monterrey"))
(base 119 94 0 (n "Las Palmas"))
(town 29 94 0 (n "Chihuahua"))
(town 206 96 0 (n "Multan"))
(town 126 97 2 (n "Marrakech"))
(town 185 98 0 (n "Esfahan"))
(town 183 101 0 (n "Tehran"))
(town 191 102 0 (n "Mashhad"))
(base 166 103 0 (n "Adana"))
(base 177 104 0 (n "Tabriz"))
(town 158 104 0 (n "Izmir"))
(town 154 104 4 (n "Athens"))
(town 125 104 0 (n "Cordoba"))
(town 122 104 0 (n "Lisbon"))
(town 174 105 0 (n "Jerevan"))
(town 171 105 0 (n "Erzurum"))
(town 162 105 0 (n "Ankara"))
(town 153 106 4 (n "Thessaloniki"))
(town 198 107 0 (n "Tashkent"))
(city 157 107 0 (n "Istanbul"))
(town 121 107 0 (n "Porto"))
(town 152 108 4 (n "Sofia"))
(town 154 110 4 (n "Bucharest"))
(town 148 110 4 (n "Belgrade"))
(town 143 111 4 (n "Zagreb"))
(base 232 113 0 (n "Ulaan Bataar"))
(town 145 113 4 (n "Budapest"))
(town 142 114 4 (n "Vienna"))
(town 139 116 4 (n "Prague"))
(town 128 116 4 (n "Brussels"))
(town 145 118 4 (n "Warsaw"))
(city 128 118 4 (n "Rotterdam"))
(town 116 119 0 (n "Dublin"))
(town 148 120 4 (n "Vilnius"))
(city 134 121 4 (n "Copenhagen"))
(town 146 122 4 (n "Riga"))
(town 137 125 0 (n "Stockholm"))

(infantry 121 120 1)
(infantry 121 119 1)
(infantry 121 118 1)
(infantry 122 118 1)
(infantry 121 117 1)
(infantry 122 117 1)
(infantry 123 117 1)
(infantry 165 96 1)
(infantry 225 89 1)
(infantry 214 85 1)
(infantry 219 81 1)
(infantry 317 33 1)
(armor 122 119 1)
(armor 165 95 1)
(bomber 119 121 1 (in 208))
(bomber 119 121 1 (in 208))
(bomber 120 117 1 (in 181))
(interceptor 122 120 1 (in 193))
(interceptor 122 120 1 (in 193))
(interceptor 120 119 1 (in 222))
(interceptor 120 119 1 (in 222))
(interceptor 123 118 1 (in 225))
(interceptor 123 118 1 (in 225))
(interceptor 120 117 1 (in 181))
(interceptor 124 117 1 (in 164))
(interceptor 124 117 1 (in 164))
(fleet 117 125 1)
(fleet 116 122 1)
(fleet 121 121 1)
(fleet 119 119 1)
(fleet 119 117 1)
(base 146 72 1 (n "Accra"))
(base 187 79 1 (n "Aden"))
(base 170 90 1 (n "Aswan"))
(base 168 92 1 (n "Asyut"))
(base 179 98 1 (n "Baghdad"))
(base 192 50 1 (n "Blantyre"))
(base 303 48 1 (n "Cairns"))
(base 227 72 1 (n "Colombo"))
(base 192 58 1 (n "Dar-es-Salaam"))
(base 287 53 1 (n "Darwin"))
(base 124 117 1 (n "Dover") (# 164))
(base 183 64 1 (n "Entebbe"))
(base 125 102 1 (n "Gibraltar"))
(base 319 22 1 (n "Hobart"))
(base 170 98 1 (n "Jerusalem"))
(base 202 100 1 (n "Kabul"))
(base 174 81 1 (n "Khartoum"))
(base 64 84 1 (n "Kingston"))
(base 249 69 1 (n "Kuala Lumpur"))
(base 150 72 1 (n "Lagos"))
(base 222 79 1 (n "Madras"))
(base 148 100 1 (n "Malta"))
(base 235 88 1 (n "Mandalay"))
(base 191 61 1 (n "Mombasa"))
(base 187 64 1 (n "Nairobi"))
(base 192 32 1 (n "Port Elizabeth"))
(base 155 70 1 (n "Port Harcourt"))
(base 302 56 1 (n "Port Moresby"))
(base 167 96 1 (n "Port Said"))
(base 237 83 1 (n "Rangoon"))
(base 190 47 1 (n "Salisbury"))
(base 168 95 1 (n "Suez"))
(base 212 87 1 (n "Surat"))
(town 306 31 1 (n "Adelaide"))
(town 210 89 1 (n "Ahmadabad"))
(town 164 97 1 (n "Alexandria"))
(town 218 91 1 (n "Allahabad"))
(town 342 29 1 (n "Auckland"))
(town 220 79 1 (n "Bangalore"))
(town 115 121 1 (n "Belfast"))
(town 215 89 1 (n "Bhopal"))
(town 316 38 1 (n "Brisbane"))
(town 120 117 1 (n "Bristol") (# 181))
(town 166 95 1 (n "Cairo"))
(town 9 117 1 (n "Calgary"))
(town 316 30 1 (n "Canberra"))
(town 185 32 1 (n "Cape Town"))
(town 344 22 1 (n "Christchurch"))
(town 220 77 1 (n "Coimbatore"))
(town 228 89 1 (n "Dacca"))
(town 213 94 1 (n "Delhi"))
(town 195 35 1 (n "Durban"))
(town 9 119 1 (n "Edmonton"))
(town 253 88 1 (n "Hong Kong"))
(town 122 120 1 (n "Hull") (# 193))
(town 219 83 1 (n "Hyderabad"))
(town 212 92 1 (n "Jaipur"))
(town 191 39 1 (n "Johannesburg"))
(town 216 92 1 (n "Kanpur"))
(town 203 91 1 (n "Karachi"))
(town 208 97 1 (n "Lahore"))
(town 314 28 1 (n "Melbourne"))
(town 53 111 1 (n "Montreal"))
(town 218 87 1 (n "Nagpur"))
(town 119 121 1 (n "Newcastle") (# 208))
(town 222 91 1 (n "Patna"))
(town 282 33 1 (n "Perth"))
(town 216 83 1 (n "Poona"))
(town 191 36 1 (n "Pretoria"))
(town 199 97 1 (n "Qandahar"))
(town 252 67 1 (n "Singapore"))
(town 72 113 1 (n "St. John's"))
(town 317 32 1 (n "Sydney"))
(town 49 110 1 (n "Toronto"))
(town 2 115 1 (n "Vancouver"))
(town 27 115 1 (n "Winnipeg"))
(city 214 84 1 (n "Bombay"))
(city 227 88 1 (n "Calcutta"))
(city 117 121 1 (n "Glasgow"))
(city 120 119 1 (n "Manchester") (# 222))
(capital 123 118 1 (n "London") (# 225))

(base 142 71 2 (n "Abidjan"))
(base 169 102 2 (n "Aleppo"))
(base 206 47 2 (n "Antananarivo"))
(base 135 78 2 (n "Bamako"))
(base 249 81 6 (n "Da-nang"))
(base 125 80 2 (n "Dakar"))
(base 158 70 2 (n "Douala"))
(base 244 87 6 (n "Ha-noi"))
(base 204 50 2 (n "Majanga"))
(base 248 77 6 (n "Phnom-Penh"))
(base 252 76 6 (n "Sai-gon"))
(base 142 102 2 (n "Tunis"))
(town 135 102 2 (n "Algiers"))
(town 125 99 2 (n "Casablanca"))
(town 127 99 2 (n "Fez"))
(town 132 101 2 (n "Oran"))
(town 129 109 2 (n "Toulouse"))
(town 134 110 2 (n "Nice"))
(town 124 113 2 (n "Nantes"))
(town 132 109 2 (n "Marseilles"))
(town 131 111 2 (n "Lyon"))
(town 125 115 2 (n "Le Havre"))
(town 127 110 2 (n "Bordeaux"))
(capital 128 114 2 (n "Paris"))

(infantry 56 108 3)
(infantry 41 105 3)
(infantry 10 103 3)
(infantry 54 101 3)
(infantry 263 81 3)
(fleet 3 113 3)
(fleet 58 108 3)
(fleet 7 104 3)
(fleet 54 104 3)
(fleet 14 99 3)
(base 268 76 3 (n "Cebu"))
(base 65 75 3 (n "Panama City"))
(town 25 101 3 (n "Albuquerque"))
(town 48 99 3 (n "Atlanta"))
(town 46 99 3 (n "Birmingham"))
(town 50 108 3 (n "Buffalo"))
(town 45 105 3 (n "Cincinnati"))
(town 48 106 3 (n "Cleveland"))
(town 27 97 3 (n "El Paso"))
(town 341 87 3 (n "Honolulu"))
(town 43 105 3 (n "Indianapolis"))
(town 53 96 3 (n "Jacksonville"))
(town 35 105 3 (n "Kansas City"))
(town 263 80 3 (n "Manila"))
(town 56 91 3 (n "Miami"))
(town 39 109 3 (n "Milwaukee"))
(town 34 110 3 (n "Minneapolis-St. Paul"))
(town 45 102 3 (n "Nashville"))
(town 43 95 3 (n "New Orleans"))
(town 54 102 3 (n "Norfolk"))
(town 34 101 3 (n "Oklahoma City"))
(town 33 107 3 (n "Omaha"))
(town 20 99 3 (n "Phoenix"))
(town 50 106 3 (n "Pittsburgh"))
(town 3 111 3 (n "Portland"))
(town 52 108 3 (n "Rochester"))
(town 9 104 3 (n "Sacramento"))
(town 18 106 3 (n "Salt Lake City"))
(town 36 95 3 (n "San Antonio"))
(town 17 98 3 (n "San Diego"))
(town 74 84 3 (n "San Juan"))
(town 39 104 3 (n "St. Louis"))
(town 54 93 3 (n "Tampa"))
(town 23 97 3 (n "Tucson"))
(town 36 102 3 (n "Tulsa"))
(town 33 103 3 (n "Wichita"))
(city 57 108 3 (n "Boston"))
(city 41 107 3 (n "Chicago"))
(city 37 98 3 (n "Dallas/Ft Worth"))
(city 25 105 3 (n "Denver"))
(city 45 108 3 (n "Detroit"))
(city 39 95 3 (n "Houston"))
(city 14 100 3 (n "Los Angeles"))
(city 56 106 3 (n "New York"))
(city 54 105 3 (n "Philadelphia"))
(city 8 103 3 (n "San Francisco"))
(city 4 113 3 (n "Seattle"))
(capital 53 104 3 (n "Washington DC"))

(infantry 148 126 4)
(infantry 149 124 4)
(infantry 150 124 4)
(infantry 151 124 4)
(infantry 153 124 4)
(infantry 153 123 4)
(infantry 156 122 4)
(infantry 157 122 4)
(infantry 158 122 4)
(infantry 158 121 4)
(infantry 158 120 4)
(infantry 159 119 4)
(infantry 161 118 4)
(infantry 161 117 4)
(infantry 128 115 4)
(infantry 162 115 4)
(infantry 127 114 4)
(infantry 162 114 4)
(infantry 125 113 4)
(infantry 162 113 4)
(infantry 163 113 4)
(infantry 127 111 4)
(armor 152 124 4)
(armor 160 119 4)
(armor 162 116 4)
(air-force 132 118 4)
(air-force 129 116 4)
(air-force 126 114 4)
(air-force 129 114 4)
(air-force 129 113 4)
(air-force 126 112 4)
(bomber 142 119 4)
(bomber 138 117 4)
(bomber 128 113 4)
(interceptor 131 119 4)
(interceptor 129 118 4)
(interceptor 131 118 4)
(interceptor 127 115 4)
(interceptor 134 114 4)
(convoy 132 124 4)
(convoy 135 120 4)
(convoy 142 120 4)
(convoy 124 115 4)
(sub-fleet 131 133 4)
(sub-fleet 125 127 4)
(sub-fleet 130 121 4)
(sub-fleet 130 120 4)
(sub-fleet 131 120 4)
(sub-fleet 127 118 4)
(sub-fleet 124 112 4)
(sub-fleet 126 110 4)
(base 126 127 4 (n "Bergen"))
(base 132 132 4 (n "Narvik"))
(town 131 117 4 (n "Dortmund"))
(town 132 117 4 (n "Essen"))
(town 134 116 4 (n "Frankfurt"))
(town 143 120 4 (n "Konigsberg"))
(town 137 114 4 (n "Munich"))
(town 131 125 4 (n "Oslo"))
(town 135 114 4 (n "Stuttgart"))
(city 141 120 4 (n "Danzig"))
(city 132 119 4 (n "Hamburg"))
(capital 137 118 4 (n "Berlin"))

(infantry 138 111 5)
(infantry 143 107 5)
(infantry 146 102 5)
(infantry 158 97 5)
(convoy 137 109 5)
(convoy 143 106 5)
(convoy 145 103 5)
(convoy 150 97 5)
(fleet 147 105 5)
(base 184 74 5 (n "Adis Adeba"))
(base 180 81 5 (n "Asmera"))
(base 194 68 5 (n "Mogadishu"))
(town 154 97 5 (n "Benghazi"))
(town 136 110 5 (n "Genoa"))
(town 145 102 5 (n "Palermo"))
(town 134 111 5 (n "Torino"))
(town 151 96 5 (n "Tripoli"))
(city 136 111 5 (n "Milan"))
(capital 142 107 5 (n "Rome"))

(infantry 251 111 6)
(infantry 251 107 6)
(infantry 247 106 6)
(infantry 247 103 6)
(infantry 270 102 6)
(infantry 271 102 6)
(infantry 242 101 6)
(infantry 244 100 6)
(infantry 247 100 6)
(infantry 249 97 6)
(infantry 249 94 6)
(infantry 253 94 6)
(infantry 252 89 6)
(convoy 272 103 6)
(convoy 271 100 6)
(convoy 262 99 6)
(convoy 269 99 6)
(fleet 267 100 6)
(fleet 270 100 6)
(fleet 272 100 6)
(fleet 268 99 6)
(cv-fleet 264 100 6)
(cv-fleet 263 98 6)
(sub-fleet 270 107 6)
(sub-fleet 263 100 6)
(sub-fleet 265 99 6)
(town 270 106 6 (n "Aomori"))
(town 253 109 6 (n "Changchun"))
(town 258 107 6 (n "Changjin"))
(town 251 105 6 (n "Dairen"))
(town 263 99 6 (n "Fukuoka"))
(town 253 111 6 (n "Haerbin"))
(town 266 100 6 (n "Hiroshima"))
(town 264 97 6 (n "Kagoshima"))
(town 268 102 6 (n "Kanazawa"))
(town 258 101 6 (n "Kwangjin"))
(town 264 92 6 (n "Naha"))
(town 269 103 6 (n "Niigata"))
(town 260 101 6 (n "Pusan"))
(town 255 105 6 (n "Pyongyang"))
(town 249 113 6 (n "Qiqihaer"))
(town 269 109 6 (n "Sapporo"))
(town 271 104 6 (n "Sendai"))
(town 257 103 6 (n "Seoul"))
(town 252 107 6 (n "Shengyang"))
(town 259 88 6 (n "Kaohsiun"))
(town 259 90 6 (n "T'aipei"))
(city 269 101 6 (n "Nagoya"))
(city 268 100 6 (n "Osaka"))
(capital 271 101 6 (n "Tokyo"))

(infantry 149 126 7)
(infantry 150 126 7)
(infantry 151 125 7)
(infantry 152 125 7)
(infantry 154 124 7)
(infantry 154 123 7)
(infantry 155 123 7)
(infantry 156 123 7)
(infantry 157 123 7)
(infantry 158 123 7)
(infantry 159 122 7)
(infantry 160 121 7)
(infantry 159 120 7)
(infantry 160 120 7)
(infantry 161 119 7)
(infantry 162 118 7)
(infantry 238 118 7)
(infantry 162 117 7)
(infantry 252 117 7)
(infantry 163 116 7)
(infantry 163 115 7)
(infantry 164 114 7)
(infantry 164 113 7)
(infantry 260 113 7)
(infantry 165 112 7)
(infantry 259 110 7)
(base 204 109 7 (n "Alma-Ata"))
(base 175 112 7 (n "Astrachan'"))
(base 252 116 7 (n "Blagoveshchenk"))
(base 237 118 7 (n "Chita"))
(base 203 108 7 (n "Frunze"))
(base 228 118 7 (n "Irkutsk"))
(base 180 124 7 (n "Niznij Tagil"))
(base 282 119 7 (n "P. Kamchatkij"))
(base 232 117 7 (n "Ulan Ude"))
(base 207 115 7 (n "Ust' Kamenogorsk"))
(base 259 109 7 (n "Vladivostok"))
(town 179 106 7 (n "Baku"))
(town 205 119 7 (n "Barnaul"))
(town 157 119 7 (n "Br'ansk"))
(town 261 114 7 (n "Chabarovsk"))
(town 184 120 7 (n "Chelabinsk"))
(town 161 114 7 (n "Dnepropetrovsk"))
(town 199 104 7 (n "Dusanbe"))
(town 165 122 7 (n "Gorky"))
(town 160 123 7 (n "Jaroslavi"))
(town 198 115 7 (n "Karaganda"))
(town 208 120 7 (n "Kemerovo"))
(town 161 116 7 (n "Kharkov"))
(town 261 116 7 (n "Komsomol'sk"))
(town 213 123 7 (n "Krasnojarsk"))
(town 173 119 7 (n "Kujbyshev"))
(town 188 120 7 (n "Kurgan"))
(town 149 125 7 (n "Leningrad"))
(town 182 119 7 (n "Magnitogorsk"))
(town 150 119 7 (n "Minsk"))
(town 210 119 7 (n "Novokuzneck"))
(town 157 112 7 (n "Odessa"))
(town 196 120 7 (n "Omsk"))
(town 179 117 7 (n "Orenburg"))
(town 192 120 7 (n "Petropavlovsk"))
(town 182 122 7 (n "Sverdlovsk"))
(town 187 122 7 (n "T'umen'"))
(town 173 107 7 (n "Tbilisi"))
(town 206 122 7 (n "Tomsk"))
(town 170 114 7 (n "Stalingrad"))
(city 163 114 7 (n "Doneck"))
(city 155 116 7 (n "Kiev"))
(city 204 121 7 (n "Novosibirsk"))
(capital 159 121 7 (n "Moscow"))

(infantry 237 105 8)
(infantry 240 104 8)
(infantry 239 103 8)
(infantry 241 102 8)
(infantry 241 99 8)
(infantry 242 99 8)
(infantry 245 98 8)
(infantry 247 98 8)
(infantry 247 97 8)
(infantry 245 95 8)
(infantry 247 95 8)
(infantry 249 93 8)
(infantry 252 93 8)
(infantry 239 92 8)
(infantry 255 92 8)
(infantry 240 91 8)
(infantry 255 91 8)
(infantry 249 90 8)
(town 244 106 8 (n "Baoding"))
(town 246 105 8 (n "Beijing"))
(town 248 94 8 (n "Changsha"))
(town 239 96 8 (n "Chengdu"))
(town 241 95 8 (n "Chonqing"))
(town 256 92 8 (n "Fuzhou"))
(town 250 89 8 (n "Guangzhou"))
(town 250 85 8 (n "Haikou"))
(town 250 98 8 (n "Huainan"))
(town 239 91 8 (n "Kunming"))
(town 251 94 8 (n "Nanchang"))
(town 252 98 8 (n "Nanjing"))
(town 256 95 8 (n "Ningbao"))
(town 255 97 8 (n "Shanghai"))
(town 243 103 8 (n "Taiyuan"))
(town 248 104 8 (n "Tianjin"))
(town 249 96 8 (n "Wuhan"))
(town 241 100 8 (n "Xi'an"))
(town 233 102 8 (n "Xining"))
(town 250 100 8 (n "Xuzhou"))
(town 245 101 8 (n "Zhengzhou"))

(town 131 107 9 (n "Barcelona"))
(town 125 109 9 (n "Bilbao"))
(town 129 105 9 (n "Valencia"))
(town 128 107 9 (n "Zaragoza"))
(city 126 106 9 (n "Madrid"))

;;; Modify for territorial changes from 1938.

;; British gains in East Africa.

(unit "Adis Adeba" (s gb))
(unit "Asmera" (s gb))
(unit "Mogadishu" (s gb))

;; German gains in Europe.

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
(unit "Kiev" (s de))
(unit "Minsk" (s de))
(unit "Odessa" (s de))
(unit "Riga" (s de))
(unit "Vilnius" (s de))

;; Japanese gains in China.

(unit "Baoding" (s jp))
(unit "Beijing" (s jp))
(unit "Tianjin" (s jp))

;; (add minor countries also?)

;; (set up agreements applying to particular dates)

(scorekeeper (do last-side-wins))

(set initial-date "Jan 1942")

;;; Set up January 1942 alliances.
