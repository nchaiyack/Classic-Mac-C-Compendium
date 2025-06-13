//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D.cc
//|
//| This file contains the implementation of the Light3D class.
//| A Light3D is a colored point light source in 3-space.
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

#include <iostream.h>
#include "Color3D.hpp"
#include "Vector3D.hpp"
#include "Scene3D.h"
#include "View3D.h"
#include "Light3D.h"
#include "Matrix3D.hpp"



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::Light3D
//|
//| Purpose: These methods construct new lights
//|
//| Parameters: 
//|
//| Created by Greg Ferrar, June 21, 1994
//|
//| Modified:
//|________________________________________________________________

static Color3D defaultColor(REAL_TO_DOUBLE(REAL_COLOR_MAX),
                            REAL_TO_DOUBLE(REAL_COLOR_MAX),
                            REAL_TO_DOUBLE(REAL_COLOR_MAX));

Light3D::Light3D(const Vector3D& newPos)
{

  Initialize(newPos, defaultColor, 0.5);

}  //==== Light3D::Light3D() ====


Light3D::Light3D(Vector3D *newPos)
{

  Initialize(*newPos, defaultColor, 0.5);

}  //==== Light3D::Light3D() ====


Light3D::Light3D(const Vector3D& newPos, const Color3D& newColor)
{

  Initialize(newPos, newColor, 0.5);

}  //==== Light3D::Light3D() ====


Light3D::Light3D(Vector3D *newPos, Color3D *newColor)
{

  Initialize(*newPos, *newColor, 0.5);

}  //==== Light3D::Light3D() ====


Light3D::Light3D(const Vector3D& newPos, const Color3D& newColor, double newDiffuse)
{

  Initialize(newPos, newColor, newDiffuse);

}  //==== Light3D::Light3D() ====


Light3D::Light3D(Vector3D *newPos, Color3D *newColor, double newDiffuse)
{

  Initialize(*newPos, *newColor, newDiffuse);

}  //==== Light3D::Light3D() ====


Light3D::Light3D(const Light3D& l)
{

  Initialize(l.pos, l.color, l.kDiffuse);

}  //==== Light3D::Light3D() ====


Light3D::Light3D(Light3D *l)
{

  Initialize(l->pos, l->color, l->kDiffuse);

}  //==== Light3D::Light3D() ====


void Light3D::Initialize(const Vector3D& newPos, const Color3D& newColor, double newDiffuse)
{

  // Set diffuseness
  kDiffuse = DOUBLE_TO_REAL(newDiffuse);

  // Set the position
  pos = newPos;

  // Normalize the position
  pos.Normalize();

  // Set the color
  color = newColor;

  // Set up the color multipliers
  UpdateColorMultipliers();
  
  //  Light doesn't belong to a scene yet
  scene = NULL;

}    //==== Initialize() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::~Light3D
//|
//| Purpose: This destroys this light
//|
//| Parameters: none
//|
//| Created by Greg Ferrar, June 21, 1994
//|
//| Modified:
//|________________________________________________________________

Light3D::~Light3D(void)
{


}  //==== Light3D::~Light3D() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::Diffuse
//|
//| Purpose: This returns the diffuseness of this light
//|
//| Parameters: returns diffuseness
//|
//| Created by Greg Ferrar, June 21, 1994
//|
//| Modified:
//|________________________________________________________________

double Light3D::Diffuse(void)
{

  return (REAL_TO_DOUBLE(kDiffuse));	    // return the diffuseness constant (as a double)

}  //==== Light3D::Diffuse() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::SetPosition
//|
//| Purpose: This changes the position of the light
//|
//| Parameters: newPos: the new position
//|
//| Created by Greg Ferrar, June 21, 1994
//|
//| Modified:
//|________________________________________________________________

void Light3D::SetPosition(const Vector3D& newPos)
{

  pos = newPos;				     // Set the position
  pos.Normalize();			     // Normalize the position

  //  If we're attached to a scene, this change has made the lighting invalid
  if (scene)
    scene->ShadingInvalid();

}  //==== Light3D::SetPosition() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::GetPosition
//|
//| Purpose: This returns the position of the light
//|
//| Parameters: newPos: the new position
//|
//| Created by Greg Ferrar, June 21, 1994
//|
//| Modified:
//|________________________________________________________________

Vector3D &Light3D::CGetPosition(void)
{

  return pos;				  // return the position of the light

}  //==== Light3D::GetPosition() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::SetColor
//|
//| Purpose: This changes the color of the light
//|
//| Parameters: newColor: the new color
//|
//| Created by Greg Ferrar, August 31, 1994
//|
//| Modified:
//|________________________________________________________________

void Light3D::SetColor(const Color3D& newColor)
{

  color = newColor;			    // Save the new color

  UpdateColorMultipliers();		    // Set up the color multipliers

  //  If we're attached to a scene, this change has made the lighting invalid
  if (scene)
    scene->ShadingInvalid();

}  //==== Light3D::SetColor() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::UpdateColorMultipliers
//|
//| Purpose: This sets the color multipliers (used to do fast
//|          color mapping) according to the current values of
//|          color and kDiffuse.
//|
//| Parameters: none
//|
//| Created by Greg Ferrar, December 15, 1994
//|
//| Modified:
//|________________________________________________________________

void Light3D::UpdateColorMultipliers(void)
{

  RedColorMult = REAL_MULT(color.r, kDiffuse);
  GreenColorMult = REAL_MULT(color.g, kDiffuse);
  BlueColorMult = REAL_MULT(color.b, kDiffuse);

}  //==== Light3D::UpdateColorMultipliers() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::TransformPos
//|
//| Purpose: This applies a transformation to the position of this
//|          Light3D
//|
//| Parameters: transform: the transformation
//|
//| Created by Greg Ferrar, June 21, 1994
//|
//| Modified:
//|  May, 1, 1995 by Maarten Gribnau
//|    updated for new matrix methods
//|    included normalisation to avoid scaling
//|________________________________________________________________

void Light3D::TransformPos(Matrix3D& transform)
{

  transformedPos = pos.TransformDirectionTo(transform);
  transformedPos.Normalize();

}  //==== Light3D::TransformPos() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Light3D::GetTransformedPos
//|
//| Purpose: This returns the transformed position of the light
//|
//| Parameters: returns the position
//|
//| Created by Greg Ferrar, June 21, 1994
//|
//| Modified:
//|________________________________________________________________

Vector3D &Light3D::GetTransformedPos(void)
{

  return transformedPos;

}  //==== Light3D::GetTransformedPos() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| operator << for light
//|
//| Purpose: This sends a light to an output stream
//|
//| Parameters: o: the output stream
//|             l: the light to output
//|             returns o, with l output to it
//|
//| Created by Greg Ferrar, June 21, 1994
//|
//| Modified:
//|________________________________________________________________

ostream &operator << (ostream &o, Light3D &light)
{

  o << "lightPosition: " << light.pos << '\n';
  o << "lightColor: " << light.color << '\n';
  o << "lightDiffuse: " << light.kDiffuse << '\n';
  o << '\n';

  return o;

}  //==== operator << for Light3D ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| operator >> for Light
//|
//| Purpose: This reads a Light3D from an input stream
//|
//| Parameters: in: the input stream
//|             l: the light to input
//|             returns in, after reading l from it
//|
//| Created by Greg Ferrar, June 21, 1994
//|
//| Modified:
//|________________________________________________________________

istream &operator >> (istream &in, Light3D &light)
{

  char dummy[50];
  in >> dummy >> light.pos;
  in >> dummy >> light.color;
  in >> dummy >> light.kDiffuse;

  //  If we're attached to a scene, this change has made the lighting invalid
  if (light.scene)
    light.scene->ShadingInvalid();

  return in;

}  //==== operator << for Light3D ====


