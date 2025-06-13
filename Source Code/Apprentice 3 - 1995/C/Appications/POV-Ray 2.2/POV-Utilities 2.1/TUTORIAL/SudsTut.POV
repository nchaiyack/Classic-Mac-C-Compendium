// Persistence of Vision RayTracer version 2 Scene description file
// File: SudsTut.POV
// Description: POV-Utilities/Mac Tutorial -- Suds
// Date: 5/5/94
// Author: Eduard [esp] Schwan
//


// ==== the standard tutorial surroundings (floor, sky, lighting)

// how far back from the object do we need to be
#declare Camera_ZDist = -7.0

#include "UtilTut.Inc"


// ==== the object

// ---- You must define some textures:

#declare sudsTex1  = texture {
  pigment {
    wood turbulence 0.2 scale 0.2
  }
  // normal { wrinkles, bumps, etc. }
  finish {
    ambient 0.2 diffuse 0.7
    specular 0.3 roughness 0.1
  }
} // sudsTex1 

#declare sudsTex2  = texture {
  pigment {
    agate
  }
  // normal { wrinkles, bumps, etc. }
  finish {
    ambient 0.2 diffuse 0.7
    specular 0.8 roughness 0.01
  }
} // sudsTex2 

#declare sudsTex3  = texture {
  pigment {
    marble turbulence 1 scale 0.5
  }
  // normal { wrinkles, bumps, etc. }
  finish {
    ambient 0.2 diffuse 0.7
    specular 0.8 roughness 0.01
  }
} // sudsTex3 

#declare sudsTex4  = texture {
  pigment {
    granite
  }
  // normal { wrinkles, bumps, etc. }
  finish {
    ambient 0.2 diffuse 0.7
    specular 0.8 roughness 0.01
  }
} // sudsTex4 

#declare sudsTex5  = texture {
  pigment {
    color rgbf <0.8,1,1,1>
  }
  // normal { wrinkles, bumps, etc. }
  finish {
    ambient 0.2 diffuse 0.7
    refraction 1 ior 1.4
    specular 0.8 roughness 0.01
  }
} // sudsTex5 

#declare sudsTex6  = texture {
  pigment {
    color rgb <1,0.2,0.6>
  }
  normal { bumps 0.6 scale 0.5 }
  finish {
    ambient 0.2 diffuse 0.5
    reflection 0.7 metallic
    specular 0.8 roughness 0.01
  }
} // sudsTex6 

#declare sudsTex7  = texture {
  pigment {
    color rgb <1,0,1>
  }
  // normal { wrinkles, bumps, etc. }
  finish {
    ambient 0.2 diffuse 0.7
    specular 0.8 roughness 0.2
  }
} // sudsTex7 

#declare sudsTex8  = texture {
  pigment {
    color rgb <0,1,0>
  }
  // normal { wrinkles, bumps, etc. }
  finish {
    ambient 0.2 diffuse 0.7
    specular 0.8 roughness 0.01
  }
} // sudsTex8 

#declare sudsTex9  = texture {
  pigment {
    gradient y
    color_map {
      [ 0.1  color rgb <1,0,0> ]
      [ 0.3  color rgb <1,1,1> ]
      [ 1.0  color rgb <0,0,1> ]
    }
    scale 0.5
  }
  // normal { wrinkles, bumps, etc. }
  finish {
    ambient 0.2 diffuse 0.7
    specular 0.8 roughness 0.01
  }
} // sudsTex9 

#declare sudsTex10 = texture {
  pigment {
    radial frequency 10 rotate 90*x
  }
  // normal { wrinkles, bumps, etc. }
  finish {
    ambient 0.2 diffuse 0.7
    specular 0.8 roughness 0.01
  }
} // sudsTex10


// ---- read the suds declaration
#include "SudsTut.inc"

// ---- create the suds object
object
{
  Suds
  scale 1/10  // it is 100x50x10, scale down to 10x5x1
//  translate <0 0.5 0> // move up off floor slightly
}

// ==== [eof]

