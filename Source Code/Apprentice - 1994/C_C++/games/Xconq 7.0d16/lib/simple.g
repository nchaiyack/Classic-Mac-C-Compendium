(game-module "simple"
  (blurb "trivial game")
)

(terrain-type plains (color "green") (char "+"))

(unit-type human (image-name "person") (char "@")
  (start-with 1)
  (acp-per-turn 4)
)
