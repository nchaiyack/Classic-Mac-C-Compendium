//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D.cc
//|
//| This is the implementation of the Color3D class.  A Color3D
//| represents a color.
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


#include "Color3D.hpp"
#include <iostream.h>




//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D::Color3D
//| 
//| Purpose: This method creates a new white Color3D
//|
//| Parameters: none
//|
//| Created by Jeff Palmer
//| 
//| Modified:
//|________________________________________________________________

Color3D::Color3D(void)
{

	//  Set the color to white
	r = REAL_COLOR_MAX;
	g = REAL_COLOR_MAX;
	b = REAL_COLOR_MAX;

}  //==== Color3D::Color3D() ====





//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D::Color3D
//| 
//| Purpose: This method creates a new Color3D
//|
//| Parameters: newR, newG, newB: the color to make this Color3D
//|
//| Created by Jeff Palmer
//| 
//| Modified:
//|________________________________________________________________

Color3D::Color3D(double newR, double newG, double newB)
{

	//  Set the color to white
	r = DOUBLE_TO_REAL(newR);
	g = DOUBLE_TO_REAL(newG);
	b = DOUBLE_TO_REAL(newB);

}  //==== Color3D::Color3D() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D::Color3D
//| 
//| Purpose: This method creates a new Color3D which is a copy
//|          of an existing Color3D
//|
//| Parameters: color: the color to copy
//|
//| Created by Jeff Palmer
//| 
//| Modified:
//|________________________________________________________________

Color3D::Color3D(const Color3D& c)
{

	//  Set the color to the same as c
	r = c.r;
	g = c.g;
	b = c.b;

}  //==== Color3D::Color3D() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D::~Color3D
//| 
//| Purpose: This method disposes of a Color3D
//|
//| Parameters: none
//|
//| Created by Jeff Palmer
//| 
//| Modified:
//|________________________________________________________________

Color3D::~Color3D(void)
{

}  //==== Color3D::~Color3D() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Operator << for Color3D
//| 
//| Purpose: This operator outputs a Color3D to an output stream
//|
//| Parameters: out: the output stream
//|             color: the color
//|
//| Created by Greg Ferrar, August 31, 1994
//| 
//| Modified:
//|________________________________________________________________

ostream &operator<<(ostream &out, const Color3D& color)
{

  out << "( " << color.RDouble() << ' '
              << color.GDouble() << ' '
              << color.BDouble() << " )";

  return out;

}  //==== operator << for Color3D ====





//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Operator >> for Color3D
//| 
//| Purpose: This operator inputs a Color3D from an input stream
//|
//| Parameters: in: the input stream
//|             color: the color
//|
//| Created by Greg Ferrar, August 31, 1994
//| 
//| Modified:
//|________________________________________________________________

istream &operator>>(istream &in, Color3D &color)
{

  char dummy[5];
  double r, g, b;
  in >> dummy >> r >> g >> b >> dummy;

  color.SetRDouble(b);
  color.SetGDouble(g);
  color.SetBDouble(b);

  return in;

}  //==== operator << for Color3D ====



