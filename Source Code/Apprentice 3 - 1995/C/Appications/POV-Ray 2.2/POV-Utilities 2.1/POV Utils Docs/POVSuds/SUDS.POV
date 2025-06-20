// POV-Ray 2.0 POV-SUDS test module
// Must generate "suds.inc" with POVSUDS.EXE before rendering this file.
#include "colors.inc"
#include "textures.inc"
#include "shapes.inc"

//background { color Gray20 }
sphere { <0,0,0> 1
    pigment { radial
        rotate z*90
        rotate x*45
    }
    finish {
        ambient 1
        diffuse 0
    }
    scale 15000
}

// sudsTex1 is used by the suds.inc objects
#declare sudsTex1 = texture {
    pigment { color White }
    finish  {
        specular 0.65
        roughness 0.0005
        brilliance 5
        ambient 0.125
        diffuse 0.6
        reflection 0.15
    }
}

// You may need to declare other sudsTexn textures,  depending on the
// number of textures you specify when generating the suds object.
//#declare sudsTex2 = texture {}
//#declare sudsTex3 = texture {}
//#declare sudsTex4 = texture {}
//#declare sudsTex5 = texture {}
//#declare sudsTex6 = texture {}
//#declare sudsTex7 = texture {}
//#declare sudsTex8 = texture {}
//#declare sudsTex9 = texture {}
//#declare sudsTex10 = texture {}

// Substiture any shape you wish here, but should keep it about unit size.
#declare SudsObject =  sphere { <0, 0, 0>, 1 }
//#declare SudsObject =  box { <0, 0, 0>, <1, 1, .25> }

// Read in the suds objects as a union of many multi-scaled SudsObjects
#include "suds.inc"

// Place the union
object { Suds }

// Set the camera and lights up, and away we go!
max_trace_level 5

camera {
   location <0,  50,  -320>
   direction <0.0, 0.0,  1.0>
   up  <0.0,  1.0,  0.0>
   right <1.33333, 0.0, 0.0>
   look_at <0, 0, 0>
}

// Light source
object { light_source {  <-150, 200,  -200> color Wheat  }}
object { light_source {  <250,  350,   -700> color LightSteelBlue }}


