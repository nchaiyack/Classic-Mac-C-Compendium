// Persistence of Vision RayTracer version 2 Scene description file
// File: CoilTut.POV
// Description: POV-Utilities/Mac Tutorial -- Coil
// Date: 5/1/94
// Author: Eduard [esp] Schwan
//


// ==== the standard tutorial surroundings (floor, sky, lighting)

// how far back from the object do we need to be
#declare Camera_ZDist = -5.0

#include "UtilTut.Inc"


// ==== the object (its texture overridden by UtilTut_Texture)

#include "CoilTut.inc"

object
{
  CoilTut_Shape
  // override CoilTut_Texture with our own.
  texture { UtilTut_Texture }
}


// ==== [eof]

