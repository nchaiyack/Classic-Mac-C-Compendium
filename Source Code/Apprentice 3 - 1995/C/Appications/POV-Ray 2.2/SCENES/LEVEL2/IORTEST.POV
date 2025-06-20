// Persistence of Vision Raytracer
// This file demonstrates the use of the file "ior.inc" and a few other
// interesting and useful tricks.  It can take a bit of time to render,
// (he said, understatingly), because of the transparency and because of
// the 7 element light bank (flourescent tube?).  Eliminating some of the
// lights (and adjusting the brightness color, "Watts", accordingly)
// will help quite a bit.

#include "colors.inc"
#include "shapes.inc"
#include "textures.inc"
#include "fov.inc"                  // Field of view constants
#include "ior.inc"                  // Index of refraction constants

//max_trace_level 2                  // Use for faster debugging only!
max_trace_level 4                  // This should be enough for these shapes


camera {
   location <0, 5, -20>
   direction <0.0, 0.0, FoV_60>     // Use a 60o field of view
   up  <0.0, 1.0, 0.0>
   right <4/3, 0.0, 0.0>
   look_at <0, 1, 0>
}

// Assemble a bank of lights here, on the ground...
#declare Watts = color Gray25
#declare Light_Distance = -50

union {
    light_source  { <-6, 0, Light_Distance>  color Watts  }
    light_source  { <-4, 0, Light_Distance>  color Watts  }
    light_source  { <-2, 0, Light_Distance>  color Watts  }
    light_source  { < 0, 0, Light_Distance>  color Watts  }
    light_source  { < 2, 0, Light_Distance>  color Watts  }
    light_source  { < 4, 0, Light_Distance>  color Watts  }
    light_source  { < 6, 0, Light_Distance>  color Watts  }

    rotate 60*x           // ... and hoist 'em up into the air

    pigment { White }  // Doesn't do anything but suppresses a parser warning
}


// Horozontally striped floor
plane { y, -1
   pigment {
      checker color HuntersGreen color SummerSky
      scale <32000, 1, 2>
   }
   finish {
      ambient 0.1
      diffuse 0.6
   }
}


#declare Hummer =
union {
   // Chris Young's shape from textures.pov
   union {
      object { UnitBox }
      object { Disk_Y translate 2*y }
      sphere { <0, 4, 0>, 1 }
      rotate 45*y
   }

   // Let's attach an orange sphere to this thing... off in the distance,
   // so it'll be automatically repeated as we repeat the rest of the
   // object (see below)
   sphere { <0, 5, 20>, 1 texture { finish { Shiny } pigment {Orange} } }
}

// Set up a default texture for all objects that follow that don't already
// have a texture of their own
default { texture { Glass2 } }


// Now lay out five of those Hummers
object { Hummer
   translate -6*x
   finish { ior Diamond_Ior }
}

object { Hummer
   translate -3*x
   finish { ior Flint_Glass_Ior }
}

object { Hummer
   translate 0*x
   finish { ior Crown_Glass_Ior }
}

object { Hummer
   translate 3*x
   finish { ior Water_Ior }
}

object { Hummer
   translate 6*x
   finish { ior Air_Ior }
}
// end of file iortest.pov
