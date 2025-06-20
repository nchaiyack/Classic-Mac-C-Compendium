// Persistence of Vision RayTracer version 2 Scene description file
// File: Sponge1Tut.POV
// Description: POV-Utilities/Mac Tutorial -- Sponge
// Date: 2/1/93
// Author: Eduard [esp] Schwan
//


// ==== the standard tutorial surroundings (floor, sky, lighting)

// how far back from the object do we need to be
#declare Camera_ZDist = -4.0

#include "UtilTut.Inc"


// ==== the object

// You must define textures for each level:
#declare SpongeAtom_Tex0 = texture
  { UtilTut_Texture pigment { color red 1 } }

#declare SpongeAtom_Tex1 = texture
  { UtilTut_Texture pigment { color green 1 } }

// You may define the atomic shape to be anything:
// A simple atom
//   #declare SpongeAtomShape = object { sphere { <0,0,0> 1.0 } }
// A more interesting atom
#declare SpongeAtomShape = intersection
{
  box { <-1,-1,-1> <1,1,1> rotate <45,0,45> }
  sphere { 0*y, 1.3 }
}

#include "SpongeTut.inc"

object
{
  SpongeShape
  scale 1/3  // it is 3x3x3, scale down to 1x1x1
  bounded_by { box {<-1, -1, -1> <1, 1, 1>} }
  rotate 30*y
}


// ==== [eof]

