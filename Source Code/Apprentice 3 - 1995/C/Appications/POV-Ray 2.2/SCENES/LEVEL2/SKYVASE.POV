// Persistence Of Vision raytracer version 2.0 sample file.

// By Dan Farmer
//    Minneapolis, mn

//   skyvase.pov
//   Vase made with Hyperboloid and sphere {, sitting on a hexagonal
//   marble column.  Take note of the color and surface characteristics
//   of the gold band around the vase.  It seems to be a successful
//   combination for gold or brass.
//
// Contains a Disk_Y object which may have changed in shapes.dat


#include "shapes.inc"
#include "shapes2.inc"
#include "colors.inc"
#include "textures.inc"

#declare DMF_Hyperboloid = quadric {  /* Like Hyperboloid_Y, but more curvy */
   <1.0, -1.0,  1.0>,
   <0.0,  0.0,  0.0>,
   <0.0,  0.0,  0.0>,
   -0.5
}

camera {
   location <0.0, 28.0, -200.0>
   direction <0.0, 0.0, 2.0>
   up  <0.0, 1.0, 0.0>
   right <4/3, 0.0, 0.0>
   look_at <0.0, -12.0, 0.0>
}

/* Light behind viewer postion (pseudo-ambient light) */
light_source { <100.0, 500.0, -500.0> colour White }

union {
   union {
      intersection {
         plane { y, 0.7 }
         object { DMF_Hyperboloid scale <0.75, 1.25, 0.75> }
         object { DMF_Hyperboloid scale <0.70, 1.25, 0.70> inverse }
         plane { y, -1.0 inverse }
      }
      sphere { <0, 0, 0>, 1 scale <1.6, 0.75, 1.6 > translate <0, -1.15, 0> }

      scale <20, 25, 20>

      pigment {
         Bright_Blue_Sky
         turbulence 0.3
         quick_color Blue
         scale <8.0, 4.0, 4.0>
         rotate 15*z
      }
      finish {
         ambient 0.1
         diffuse 0.75
         phong 1
         phong_size 100
         reflection 0.35
      }
   }

   sphere {  /* Gold ridge around sphere portion of vase*/
      <0, 0, 0>, 1
      scale <1.6, 0.75, 1.6>
      translate -7*y
      scale <20.5, 4.0, 20.5>

      finish { Metal }
      pigment { OldGold }
   }

   bounded_by {
      object {
         Disk_Y
         translate -0.5*y  // Remove for new Disk_Y definition
         scale <34, 100, 34>
      }
   }
}

/* Stand for the vase */
object { Hexagon
   rotate -90.0*z             /* Stand it on end (vertical)*/
   rotate -45*y               /* Turn it to a pleasing angle */
   scale <40, 25, 40>
   translate -70*y

   pigment {
      Sapphire_Agate
      quick_color Red
      scale 10.0
   }
   finish {
      ambient 0.2
      diffuse 0.75
      reflection 0.85
   }
}

union {
   plane { z, 50  rotate -45*y }
   plane { z, 50  rotate +45*y }

   pigment { DimGray }
   finish {
      ambient 0.2
      diffuse 0.75
      reflection 0.5
   }
}
