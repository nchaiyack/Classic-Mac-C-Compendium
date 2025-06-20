(game-module "tokyo"
  (title "Tokyo 1962")
  (blurb "Save Tokyo from the fire-breathing monsters!")
  (base-module "monster")
  (variants (see-all true))
)

(area 50 20)

(area (terrain
  "ec5ec3ec3ec7ec5e2c10a9e"
  "2ec5ec2ec4e2c5e7c12a8e"
  "3ec5e3c4ecd6c21a5c"
  "4ec4ec2d5cdc3dcdc20ac2dc1d"
  "c4ec3ec2dc5d2c2dc2d5c15ac3d1c"
  "ec4e8c6dcdc2dc3d2c15a4c"
  "2ec3ecdcdefe6c2d5c3dcfc15ace1f"
  "3ec2ecdcdf2ecdc2dc2dcdc2e4cefc15ac1e"
  "3ec2e3c2d3c2dc2d3c2dcec2dcefec15a1c"
  "7c2d3cdc2dc2d2cdc2d2c2dc2efec15a"
  "7ecdcdc2d7c2d3cdcdce2fec15a"
  "8e2cdc3dc2dc2ec2dc3d2cf3ec15a"
  "3e3c2e8c2dcefe3c3dc2f4c15a"
  "6e3cec3d5cfe4f7c18a"
  "6ec4e2cd3c2dcfe3fc5b19a"
  "6ec6e2c2dcd2c4ec24a"
  "14ec2dcdcd6c23a"
  "15ecd3c2dc4ac22a"
  "15e3c2b3c27a"
  "10e8c32a"
))

(side 1 (noun "monster") (emblem-name "none")
  (class "monster")
)

(side 2 (noun "Japanese") (emblem-name "none")
  (class "human")
)

(add u* point-value 1)

(scorekeeper (do last-side-wins))

;; Suppress default unit creation.

(add u* start-with 0)

;; Our hero

(monster 14 13 1 (n "Godzilla"))

;; Everything else

(building 11 6 0)
(building 10 8 0)
(mob 11 7 2)
(building 12 6 0)
(building 13 5 0)
(mob 14 4 2)
(building 15 3 0)
(building 16 2 0 (n "City Hall"))
(building 10 9 0)
(building 13 6 0)
(building 15 4 0 (n "Courthouse"))
(building 16 3 0)
(F 11 9 2)
(building 12 8 0)
(building 16 4 0)
(g 18 2 2)
(building 12 9 0)
(building 13 8 0 (n "Capitalists Trust"))
(F 15 6 2)
(building 18 3 0)
(g 19 2 2)
(g 20 1 2)
(mob 10 12 2)
(building 12 10 0)
(building 13 9 0)
(building 14 8 0 (n "Price Club"))
(building 17 5 0)
(building 18 4 0)
(building 20 2 0)
(g 21 1 2)
(building 16 7 0)
(building 18 5 0)
(building 20 3 0 (n "Sears"))
(building 21 2 0)
(g 22 1 2)
(mob 11 13 2)
(building 14 10 0)
(building 16 8 0)
(building 17 7 0)
(building 10 15 0)
(building 14 11 0)
(building 15 10 0 (n "Mormon Temple"))
(building 17 8 0)
(mob 19 6 2)
(building 10 16 0)
(building 11 15 0 (n "Hi-Rise Apts"))
(F 12 14 2)
(building 14 12 0)
(building 15 11 0)
(mob 16 10 2)
(F 23 3 2)
(building 11 16 0)
(building 16 12 0)
(building 17 11 0 (n "Last National Bank"))
(building 18 10 0)
(mob 19 9 2)
(mob 16 13 2)
(building 17 12 0)
(building 18 11 0)
(building 21 9 0)
(building 22 8 0)
(mob 23 7 2)
(building 19 12 0)
(mob 20 11 2)
(building 21 10 0 (n "Smiths"))
(building 22 9 0)
(building 23 8 0)
(mob 18 14 2)
(building 19 13 0)
(building 20 12 0)
(building 25 7 0)
(building 19 14 0)
(building 20 13 0)
(building 23 10 0)
(mob 24 9 2)
(building 25 8 0)
(building 26 7 0)
(building 22 12 0 (n "K-Mart"))
(building 23 11 0)
(building 24 10 0)
(building 26 8 0)
(building 27 7 0 (n "Apple Japan HQ"))
(building 9 16 0 (n "IBM"))
(building 20 15 0)
(building 21 14 0)
(building 26 9 0)
(building 27 8 0)
(building 20 16 0)
(building 21 15 0)
(mob 28 8 2)
(mob 20 17 2)
(building 21 16 0)
(building 23 14 0)
(building 27 10 0 (n "Fawlty Towers"))
(building 28 9 0 (n "Hilton Tokyo"))
(building 23 15 0)
(building 24 14 0)
(building 27 11 0)
(building 28 10 0)
(building 23 16 0)
(building 24 15 0)
(building 26 13 0)
(building 28 11 0)
(building 26 14 0)
(building 27 13 0)
(building 27 14 0)
(building 28 13 0)
(mob 29 12 2)
(building 28 14 0)
(F 37 14 2)

