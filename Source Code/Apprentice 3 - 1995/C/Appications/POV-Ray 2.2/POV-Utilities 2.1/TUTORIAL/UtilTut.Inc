// Persistence of Vision RayTracer version 2 Scene description file
// File:   UtilTut.Inc
// Desc:   POV-Utilities/Mac Tutorial
//         Standard surroundings include file
// Date:   2/1/93, updated to 2.0 5/1/94
// Author: Eduard [esp] Schwan
//


// ==== set viewer's position

camera
{
  location  Camera_ZDist*z  // <X Y Z>
  direction 1.4*z           // which way are we looking <X Y Z>
  up        y               // which way is +up <X Y Z>
  right     4/3*x           // which way is +right <X Y Z>
  look_at   0*y             // point center of view at this point <X Y Z>
}


// ==== illuminate the scene

light_source
{
  0*y
  color red 1.0  green 1.0  blue 1.0  // light's color
  translate <-20, 30, -40>            // light's position
}


// ==== the sky

sphere
{
  // a sphere centered origin
  0*y, 1.0
  texture
  {
    pigment
    {
      gradient y // up/down (Y)
      // dark blue at horizon, lightening to light blue overhead
      color_map
      {
	    [  0.0  color  red 0.0  green 0.0  blue 0.2 ]
        [  1.0  color  red 0.8  green 0.6  blue 1.0 ]
      } // color_map
    }
    finish
    {
      ambient 1 diffuse 0 // all color, no shadows
    }
  } // texture

  scale 100 // stretch the texture and the sphere

} // sphere


// ==== the floor

plane
{
  // in the XZ plane, two units down
  y, -2
  texture
  {
    pigment
    {
      marble // triangle-gradient to the horizon (along X)
      // dark stripes on light gray floor
      color_map
      {
	    [  0.0  color red 0.2 green 0.2 blue 0.4 ]
	    [  0.2  color red 0.4 green 0.4 blue 0.5 ]
	    [  1.0  color red 0.8 green 0.7 blue 0.6 ]
      } // color_map
    }
    finish { ambient 0.2 diffuse 0.7 }
    // stretch the stripes out to every 2 units
    scale 2
  }
}


// ==== default object texture

#declare UtilTut_Texture = texture
{
  pigment
  {
    color red 0.2 green 0.6 blue 0.1
  }
  finish
  {
    ambient 0.2   diffuse 0.7
    phong    0.8  phong_size 80
    specular 0.5  roughness 0.01
   	reflection 0.3
  }
}

