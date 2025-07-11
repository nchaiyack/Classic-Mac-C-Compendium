//******************************************************************************
// Title:       Line3D.hpp
// Author:      Maarten Gribnau
// Date:        07-04-1995
// Purpose:     Inline methods for Line3D class.
// Descrption:  
// Project:     Graph3D
// Remarks:     
//
// Revision history:
// Who:
// Date:
// What:
//******************************************************************************

#include "Line3D.h"

#include "Vector3D.hpp"


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::SetOrigin
//|
//| Purpose: This changes the origin of the line
//|
//| Parameters: newOrigin: the new origin
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

inline void Line3D::SetOrigin(const Vector3D &newOrigin)
{

  origin = newOrigin;
  
}  //==== Line3D::SetOrigin ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::SetDirection
//|
//| Purpose: This changes the direction of the line
//|
//| Parameters: newOrigin: the new direction
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

inline void Line3D::SetDirection(const Vector3D &newDirection)
{

  direction = newDirection;
  
}  //==== Line3D::SetDirection ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::Origin
//|
//| Purpose: This returns the origin of the line
//|
//| Parameters: none
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

inline Vector3D Line3D::Origin() const
{

  return origin;
  
}  //==== Line3D::Origin ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::Direction
//|
//| Purpose: This returns the direction of the line
//|
//| Parameters: none
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

inline Vector3D Line3D::Direction() const
{

  return direction;
  
}  //==== Line3D::Direction ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::ParameterToPoint
//|
//| Purpose: This calculates calculates a point for the value of the 
//|          line parameter given.
//|
//| Parameters: parameter: value of the line parameter
//|             point:     where the result is stored
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

inline void Line3D::ParameterToPoint(Real parameter, Vector3D &point) const
{

  point  = direction;
  point *= parameter;
  point += origin;
  
} //==== Line3D::ParameterToPoint====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::PointToParameter
//|
//| Purpose: This calculates calculates the line parameter for a point
//|          on the line.
//|
//| Parameters: point:     where the result is stored
//|             parameter: value of the line parameter
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

inline Real Line3D::PointToParameter(const Vector3D &point) const
{

  Vector3D v (point);
  v -= origin;
  Real parameter = v.Magnitude();
  Real dirMag = direction.Magnitude();
  if (dirMag == 0)
    return 0;
  return parameter = REAL_DIV(parameter, dirMag);
  
} //==== Line3D::PointToParameter====