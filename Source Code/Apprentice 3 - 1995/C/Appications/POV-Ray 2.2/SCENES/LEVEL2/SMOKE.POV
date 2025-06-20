// Persistence Of Vision Raytracer version 2.0 sample file.
// Dark, smokey glass ball 
// Quick example of how to make a "black" filter.
// Just use dark shades of gray.

#declare Gray90 = color red .9 green .9 blue .9
#declare Gray80 = color red .8 green .8 blue .8
#declare Gray70 = color red .7 green .7 blue .7
#declare Gray60 = color red .6 green .6 blue .6
#declare Gray40 = color red .4 green .4 blue .4
#declare Gray30 = color red .3 green .3 blue .3

#declare White = color red 1 green 1 blue 1
#declare Blue = color blue 1
#declare Red = color red 1
#declare Yellow = color red 1 green 1

#declare Black_Smoke =
   pigment {
      bozo
      turbulence 0.2
      color_map {
         [0 0.1  color Gray80 filter 1    color Gray60 filter 0.7]
         [0.1 1  color Gray60 filter 0.7  color Gray30 filter 0.6]
      }
   }

#declare Dark_Smoke = 
   pigment {
      bozo
      turbulence 0.2
      color_map {
         [ 0 .1  color Gray90 filter 1    color Gray70 filter 0.8 ]
         [ .1 1  color Gray70 filter 0.8  color Gray40 filter 0.7 ]
      }
   }

camera {
   location <0.0, 3.0, -10.0>
   direction <0.0, 0.0, 1.0>
   up  <0.0, 1.0, 0.0>
   right <4/3, 0.0, 0.0>
   look_at <0, 2, 0>
}

// Light source
light_source { <10, 20, -30> color White }

// Leopard Sky  
sphere { <0, 0, 0>, 100
   pigment {
      leopard
      color_map { [ 0 1 color Yellow color Red] }
      scale 5
   }
   finish {
      ambient 0.8 // make it "glow"
      diffuse 0
   }
}

// Black Glass Sphere  
sphere { <0, 3, 0>, 3
   // Try with no refraction or specular 
   pigment { Dark_Smoke }
   finish {
      specular 0.9
      roughness 0.001
      ior 1.51
      refraction 1
   }
}


// Bright and Hokey checkered floor
plane { y, 0
   pigment {
      checker color White color Blue
      scale 5
   }
   finish { diffuse 0.8 }
}

