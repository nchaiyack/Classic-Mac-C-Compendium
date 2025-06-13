//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D.in
//| 
//| This file contains the inline methods for the Color3D class.
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

#include "Color3D.h"



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D::RDouble
//| 
//| Purpose: This method find the red component of this color
//|
//| Parameters: returns the red component
//|
//| Created by Jeff Palmer
//| 
//| Modified:
//|________________________________________________________________

inline double Color3D::RDouble(void) const
{

	return REAL_TO_DOUBLE(r);

}  //==== Color3D::RDouble() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D::GDouble
//| 
//| Purpose: This method find the green component of this color
//|
//| Parameters: returns the green component
//|
//| Created by Jeff Palmer
//| 
//| Modified:
//|________________________________________________________________

inline double Color3D::GDouble(void) const
{

	return REAL_TO_DOUBLE(g);

}  //==== Color3D::GDouble() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D::BDouble
//| 
//| Purpose: This method find the blue component of this color
//|
//| Parameters: returns the blue component
//|
//| Created by Jeff Palmer
//| 
//| Modified:
//|________________________________________________________________

inline double Color3D::BDouble(void) const
{

	return REAL_TO_DOUBLE(b);

}  //==== Color3D::BDouble() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D::R
//| 
//| Purpose: This method find the red component of this color
//|
//| Parameters: returns the red component
//|
//| Created by Greg Ferrar, February 13, 1995
//|________________________________________________________________

inline Real Color3D::R(void) const
{

	return r;

}  //==== Color3D::R() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D::G
//| 
//| Purpose: This method find the green component of this color
//|
//| Parameters: returns the green component
//|
//| Created by Greg Ferrar, February 13, 1995
//|________________________________________________________________

inline Real Color3D::G(void) const
{

	return g;

}  //==== Color3D::G() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D::B
//| 
//| Purpose: This method find the blue component of this color
//|
//| Parameters: returns the blue component
//|
//| Created by Greg Ferrar, February 13, 1995
//|________________________________________________________________

inline Real Color3D::B(void) const
{

	return b;

}  //==== Color3D::B() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D::SetRDouble
//| 
//| Purpose: This method sets the red component of this color
//|
//| Parameters: newR: the new red component
//|
//| Created by Jeff Palmer
//| 
//| Modified:
//|________________________________________________________________

inline void Color3D::SetRDouble(double newR)
{

	r = DOUBLE_TO_REAL(newR);
	
}  //==== Color3D::SetR() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D::SetGDouble
//| 
//| Purpose: This method sets the green component of this color
//|
//| Parameters: newG: the new green component
//|
//| Created by Jeff Palmer
//| 
//| Modified:
//|________________________________________________________________

inline void Color3D::SetGDouble(double newG)
{

	g = DOUBLE_TO_REAL(newG);
	
}  //==== Color3D::SetGDouble() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Color3D::SetBDouble
//| 
//| Purpose: This method sets the blue component of this color
//|
//| Parameters: newB: the new blue component
//|
//| Created by Jeff Palmer
//| 
//| Modified:
//|________________________________________________________________

inline void Color3D::SetBDouble(double newB)
{

	b = DOUBLE_TO_REAL(newB);
	
}  //==== Color3D::SetBDouble() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| operator = for Color3D
//| 
//| Purpose: This method assigns a Color3D to this, making a copy
//|
//| Parameters: color: the color to make a copy of
//|
//| Created by Jeff Palmer
//| 
//| Modified:
//|________________________________________________________________

inline Color3D& Color3D::operator = (const Color3D& color)
{

	//  Make this a copy of color
	r = color.r;
	g = color.g;
	b = color.b;
	
	return *this;

}  //==== operator = for Color3D ====


