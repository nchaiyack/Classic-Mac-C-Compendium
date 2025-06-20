// Persistence of Vision Raytracer
// Updated to version 2, 4/30/93, Eduard Schwan
// Shape Test file

#include "colors.inc"
#include "textures.inc"

camera {
   location   <0.0, -4.0,  1.0>
   direction  y
   up         z
   right      4/3*x
   look_at    -1*y
}

background { color DarkSlateBlue }

plane {
   <0.0, 0.0, 1.0>, -2.0
   texture {
      pigment { colour Brown }
      finish { ambient 0.2 diffuse 0.8 }
   }
}

//Include Shape to be tested
#include "coil.inc"

object {
   Coil_Shape
   rotate 45.0*x
}

//Light Source
light_source {
   <50.0, -50.0, 100.0>
   colour White
}
