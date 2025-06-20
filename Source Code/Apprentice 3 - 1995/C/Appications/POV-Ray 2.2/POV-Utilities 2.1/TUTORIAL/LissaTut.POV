// Persistence of Vision RayTracer version 1.0 Scene description file
// File: LissaTut.POV
// Description: POV-Utilities/Mac Tutorial -- Lissajous
// Date: 2/1/93
// Author: Eduard [esp] Schwan
//


// ==== the standard tutorial surroundings (floor, sky, lighting)

// how far back from the object do we need to be
#declare Camera_ZDist = -5.0

#include "UtilTut.Inc"


// ==== the object (its textures need to be predeclared)

#declare LissaTut_CTDS_SphereTex  = texture {UtilTut_Texture}
#declare LissaTut_CTDS_ConeTex    = texture {UtilTut_Texture}

#include "LissaTut.inc"

object
{
  LissaTut_CTDS_Obj
}


// ==== [eof]

