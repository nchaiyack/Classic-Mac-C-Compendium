// Persistence Of Vision raytracer version 2.0 sample file.

// POV-Ray sample image mapper data file
// by Alexander Enzmann
// NOTE: Requires "test.gif"

#include "colors.inc"

#declare Bi = 2

#declare Texture0 = /* Planar image map */
texture {pigment{image_map { gif "test.gif" map_type 0 once interpolate Bi } } }

#declare Texture1 = /* Spherical image map */
texture {pigment{image_map { gif "test.gif" map_type 1 interpolate Bi } } }

sphere { <0, 0, 0>, 1
   texture { Texture1 }
   scale 10 
   rotate -90*y
   translate <-12, 0, 20>
}

plane {
   z, 0 
   clipped_by {box { <0, 0, -1>, <1, 1, 1> } }
   texture { Texture0 }
   translate <-0.5, -0.5, 0>
   scale 20
   rotate <20, 30, 0>
   translate <12, 0, 20>
}

camera {
   location  <0, 0, -60>
   direction <0, 0,   1>
   up        <0, 0.5, 0>
   right     <0.6666, 0, 0>
}

light_source { <0, 300, -200> colour White }
