// Persistence of Vision RayTracer version 1.0 Scene description file
// File: ShellGenTut.POV
// Description: POV-Utilities/Mac Tutorial -- Shell Generator
// Date: 2/1/93
// Author: Eduard [esp] Schwan
//


// ==== the standard tutorial surroundings (floor, sky, lighting)

// how far back from the object do we need to be
#declare Camera_ZDist = -7.0

#include "UtilTut.Inc"


// ==== the object

// Eduard's snail shell texture
#declare ShellTexture = texture
{
  pigment
  {
    marble
    turbulence 0.2 rotate -15*z
    color_map
    {
      [ 0.1  color rgb <0.7, 1.0, 0.7> ]
      [ 0.4  color rgb <0.7, 0.4, 0.2> ]
      [ 0.5  color rgb <0.0, 0.1, 0.0> ]
      [ 0.6  color rgb <0.5, 0.5, 0.5> ]
      [ 1.0  color rgb <0.8, 0.8, 1.0> ]
    }
    scale 0.5 // several lines per sphere
  }
  normal { wrinkles 0.2 scale 0.1 }
  finish
  {
    ambient 0.2 diffuse 0.6
    reflection 0.1
    specular 0.3 roughness 0.2
    phong 0.2
  }
} // ShellTexture

#include "ShellGenTut.inc"

object
{
  ShellShape
  bounded_by {
    box { <-27.2984, -49.2925, -21.4182> <16.7352, -0.0031, 32.5706> }
  }
  // FYI, Center of object = <-5.2816, -24.6481, 5.57621>
  scale     0.1   // it is 25x35x25, scale down near 2x3x2
  translate 1.5*y   // move up slightly
}


// ==== [eof]

