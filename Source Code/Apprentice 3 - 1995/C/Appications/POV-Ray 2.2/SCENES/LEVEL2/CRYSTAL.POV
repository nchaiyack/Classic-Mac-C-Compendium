// Persistence Of Vision raytracer version 2.0 sample file.

//---------------------------------------------------------------------------
// This scene file was designed to emulate the digitized photographic image
// of a crystal sphere { on a checkerboard that David Buck took, and to
// verify or refute the correctness of the current refractive functions
// in POV-Ray.  The original image is available on CompuServe
// (go comart), Library 17 by the name of crysta.gif.
// Experiment with the index-of-refraction value for the crystal ball.
//---------------------------------------------------------------------------

#include "colors.inc"
#include "shapes.inc"
#include "textures.inc"

max_trace_level 5

camera {
   location <-0.85, 12.5, -28>
   direction <0, 0, 4.125>
   up  <0, 1, 0>
   right <4/3, 0, 0>
   look_at <0.25, 0.15, 0>
}

light_source { <-5, 50, -5> colour red 0.85 green 0.85 blue 0.85 }
light_source { <-500, 500, -500> colour DimGray }
// light (under checkerboard, for background
light_source { <10, -50, 5> colour White }

// The background.  Designed to give the shaded quality of the photo
sphere { <0, 0, 0>, 1
   scale <10000, 500, 500>
   rotate 60*y

   finish {
      ambient 0.2
      diffuse 0.75
      crand 0.025
   }
   pigment { color Gray }
}

union {
   object { Cube
      scale <5, 0.001, 7>

      pigment {
         checker color Black color White
         translate <1, 0, 7>
      }
      finish {
         ambient 0.35
         diffuse 0.65
         crand 0.015
      }
   }

   sphere { <-0.25, 2.15,-4.25>, 2.15
      pigment { White filter 0.75 }
      finish {
         ambient 0.2
         diffuse 0.0
         reflection 0.12
         refraction 1.0
         ior 1.45
         specular 1.0
         roughness 0.001
      }
   }

   rotate -6*z       /* Simulate the slight camera tilt in the photo */
}
