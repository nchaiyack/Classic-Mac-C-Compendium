// Persistence Of Vision raytracer version 2.0, sample file.

// Bump map test file by CdW!
// NOTE: Calls for "bumpmap_.gif", but any 320x200 gif will work.

#include "colors.inc"

camera {
   location  <0, 0, -120>
   direction <0, 0,  1.5>
   up        <0, 1,  0>
   right   <4/3, 0,  0>
   look_at   <0, 0,  0>
}

sphere { <0, 0, 0>, 25
   pigment {Blue}

   normal {
      bump_map {
         gif "bumpmap_.gif"  
         bump_size 5
         interpolate 2
         once
      }
      scale 50              /* scaled and translated into position  */
      translate <-25, -25, 0>
   }
   finish {ambient 0.2 diffuse 0.7 specular 0.6}
}

plane { y, -25
   pigment {Gold}
   finish {ambient 0.1 diffuse 0.5}
}

light_source {<100,120,-130> colour White}
