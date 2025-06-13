// -*- C++ -*-
//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D.in
//|
//| These are the inline methods of the Light3D class
//|
//| This code was created by Greg Ferrar, based on original code
//| created by Jeff Palmer.  There are no restrictions on the use
//| or distribution of this library, except that you may not
//| restrict its use or distribution.  Any project which is
//| created from this code, or any significant modification of
//| this code, may be distributed in any way you choose; you may
//| even sell it or its source code.  However, any limitations
//| placed on the distribution or use of such a project or
//| modification may not in any way limit the distribution or use
//| of this original code.
//|________________________________________________________________

#include "Light3D.h"
#include "Types3D.h"


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::CalculateFlatRedIntensity
//|
//| Purpose: This finds the flat red intensity of this light, used
//|          to compute the red component of a polygon for display.
//|
//| Parameters: red:  basic red intensity of a polygon
//|             view: the view we're in
//|             dot:  the dot product of the normal of the polygon
//|                   with the direction vector to this light.
//|             returns the flat red intensity
//|
//| Created by Greg Ferrar, June 21, 1994
//|
//| Modified:
//|________________________________________________________________

inline Real Light3D::CalculateFlatRedIntensity(Real red, Real dot)
{

  register Real redIntensity =
      REAL_MULT(REAL_MULT(RedColorMult, red), dot);

  if (redIntensity > REAL_ONE)
    redIntensity = REAL_ONE;				  // Truncate to 1.0
  
  else if (redIntensity < 0)
    redIntensity = 0;					  // Truncate to 0.0

  return redIntensity;
   
}  //==== Light3D::CalculateFlatRedIntensity() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::CalculateFlatGreenIntensity
//|
//| Purpose: This finds the flat green intensity of this light, used
//|          to compute the green component of a polygon for display.
//|
//| Parameters: green: basic greem intensity of a polygon
//|             scene:  the scene we're in
//|             dot:   the dot product of the normal of the polygon
//|                    with the direction vector to this light.
//|             returns the flat green intensity
//|
//| Created by Greg Ferrar, June 21, 1994
//|
//| Modified:
//|________________________________________________________________

inline Real Light3D::CalculateFlatGreenIntensity(Real green, Real dot)
{

  register Real greenIntensity =
      REAL_MULT(REAL_MULT(GreenColorMult, green), dot);

  if (greenIntensity > REAL_ONE)
    greenIntensity = REAL_ONE;			  // Truncate to 1.0
  
  else if (greenIntensity < 0)
    greenIntensity = 0;					  // Truncate to 0.0

  return greenIntensity;
  
}  //==== Light3D::CalculateFlatGreenIntensity() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::CalculateFlatBlueIntensity
//|
//| Purpose: This finds the flat blue intensity of this light, used
//|          to compute the blue component of a polygon for display.
//|
//| Parameters: blue: basic blue intensity of a polygon
//|             scene: the scene we're in
//|             dot:  the dot product of the normal of the polygon
//|                   with the direction vector to this light.
//|             returns the flat blue intensity
//|
//| Created by Greg Ferrar, June 21, 1994
//|
//| Modified:
//|________________________________________________________________

inline Real Light3D::CalculateFlatBlueIntensity(Real blue, Real dot)
{

  register Real blueIntensity =
      REAL_MULT(REAL_MULT(BlueColorMult, blue), dot);

  if (blueIntensity > REAL_ONE)
    blueIntensity = REAL_ONE;				  // Truncate to 1.0
  
  else if (blueIntensity < 0)
    blueIntensity = 0;					  // Truncate to 0.0

  return blueIntensity;
  
}  //==== Light3D::CalculateFlatBlueIntensity() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::GetColor
//|
//| Purpose: This gets the color of the light
//|
//| Parameters: returns the color of the light
//|
//| Created by Greg Ferrar, August 31, 1994
//|
//| Modified:
//|________________________________________________________________

inline Color3D &Light3D::CGetColor(void)
{

  return color;				  // return the color of the light

}  //==== Light3D::GetColor() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::GetColor
//|
//| Purpose: This gets the color of the light
//|
//| Parameters: returns the color of the light
//|
//| Created by Greg Ferrar, August 31, 1994
//|
//| Modified:
//|________________________________________________________________

inline Color3D *Light3D::GetColor()
{

  return &color; 

}



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::GetPosition
//|
//| Purpose: This gets the position of the light
//|
//| Parameters: returns the position of the light
//|
//| Created by Greg Ferrar, Decemeber 15, 1994
//|
//| Modified:
//|________________________________________________________________

inline Vector3D *Light3D::GetPosition(void)
{

  return &pos; 

}
