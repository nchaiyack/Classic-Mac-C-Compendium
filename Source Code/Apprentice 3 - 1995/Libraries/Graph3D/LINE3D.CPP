//******************************************************************************
// Title:       Line3D.cpp
// Author:      Maarten Gribnau
// Date:        07-04-1995
// Purpose:     Implementation of Line3D class.
// Descrption: 
// Project:     Graph3D
// Remarks:     
//
// Revision history:
// Who:
// Date:
// What:
//******************************************************************************

#include "Line3D.hpp"

#include <iostream.h>
#include "Util3D.hpp"
#include "Plane3D.h"


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::Line3D
//|
//| Purpose: Default constructor
//|
//| Parameters: none
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Line3D::Line3D(void)
{


}  //==== Line3D::Line3D() ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::Line3D
//|
//| Purpose: This method creates a Line3D from origin and direction
//|
//| Parameters: theOrigin:    origin of line
//|             theDirection: direction of line
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Line3D::Line3D(const Vector3D *theOrigin, const Vector3D *theDirection)
{

  origin    = *theOrigin;
  direction = *theDirection;
  
}  //==== Line3D::Line3D() ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::Line3D
//|
//| Purpose: This method creates a Line3D from origin and direction
//|
//| Parameters: theOrigin:    origin of line
//|             theDirection: direction of line
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Line3D::Line3D(const Vector3D &theOrigin, const Vector3D &theDirection)
{

  origin    = theOrigin;
  direction = theDirection;
  
}  //==== Line3D::Line3D() ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::Line3D
//|
//| Purpose: This method creates a Line3D by copying another
//|
//| Parameters: theLine: Line3D to copy
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Line3D::Line3D(const Line3D *theLine)
{

  origin    = theLine->origin;
  direction = theLine->direction;
  
} //==== Line3D::Line3D() ====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::Line3D
//|
//| Purpose: This method creates a Line3D by copying another
//|
//| Parameters: theLine: Line3D to copy
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Line3D::Line3D(const Line3D &theLine)
{

  origin    = theLine.origin;
  direction = theLine.direction;
  
}  //==== Line3D::Line3D() ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::~Line3D
//|
//| Purpose: This method destroys a Line3D
//|
//| Parameters: none
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Line3D::~Line3D(void)
{


}  //====Line3D::~Line3D====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::Intersect
//|
//| Purpose: This calculates calculates the point of closest intersec-
//|          tion of this line with the line given.
//|          Explanation in Graphics Gems page 304.
//|
//| Parameters: line: line to intersect with
//|             par1: value of parameter of this line at intersection
//|             par2: same, but now for the line given
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Flag Line3D::Intersect(const Line3D &line, Real &par1, Real &par2)
{
  Real     lenSq;
  Vector3D cross;
  Vector3D point;

  point = line.origin - origin; 
  cross = direction.CrossTo(line.direction);
  lenSq = cross.Dot(cross);
  if (lenSq == 0)
    // Parallel lines 
    return FALSE;

  par1 = Det3D(point.X(), line.direction.X(), cross.X(),
               point.Y(), line.direction.Y(), cross.Y(),
               point.Z(), line.direction.Z(), cross.Z());
  par1 = REAL_DIV(par1, lenSq);

  par2 = Det3D(point.X(), direction.X(), cross.X(),
               point.Y(), direction.Y(), cross.Y(),
               point.Z(), direction.Z(), cross.Z());          
  par2 = REAL_DIV(par2, lenSq);

  return TRUE;
  
} //==== Line3D::Intersect====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::Intersect
//|
//| Purpose: This calculates calculates the point of closest intersec-
//|          tion of this line with the line given.
//|
//| Parameters: line:   line to intersect with
//|             point1: point of closest intersection on this line
//|             point2: same, but now for the line given
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Flag Line3D::Intersect(const Line3D &line, Vector3D &point1, Vector3D &point2)
{

  Flag result;
  Real par1, par2;

  result = Intersect(line, par1, par2);
  if (result)
    {
    ParameterToPoint(par1, point1);
    line.ParameterToPoint(par2, point2);
    }

  return result;
  
} //==== Line3D::Intersect====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::ParameterProject
//|
//| Purpose: This calculates the parameter of the point on the line
//|          closest to the point given.
//|          Creates a plane with normal equal to line direction,
//|          through origin. Then calculates the intersection of the
//|          plane and the line. There is always a intersection 'cause
//|          the plane normal is made perpendicular to the line's
//|          direction.
//|
//| Parameters: point: point to project
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Real Line3D::ParameterProject(const Vector3D &point)
{

  Real result;
  Plane3D  plane (direction, point);
  plane.Intersect(*this, result);
  return result;

} //==== Line3D::ParameterProject====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Line3D::PointProject
//|
//| Purpose: This calculates the point on the line closest to the 
//|          point given.
//|          Creates a plane with normal equal to line direction,
//|          through origin. Then calculates the intersection of the
//|          plane and the line. There is always a intersection 'cause
//|          the plane normal is made perpendicular to the line's
//|          direction.
//|
//| Parameters: point: point to project
//|
//| Created by Maarten Gribnau
//|
//| Modified:
//|___________________________________________________________________

Vector3D& Line3D::PointProject(const Vector3D &point)
{

  static Vector3D result = point;
  Plane3D  plane (direction, point);
  plane.Intersect(*this, result);
  return result;

} //==== Line3D::PointProject====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Transform
//|
//| Purpose: This applies a transformation to this line.
//|
//| Parameters: M:      the matrix of the transformation
//|             result: the line to put the result in
//|
//| Created by Maarten Gribnau
//|___________________________________________________________________

void Line3D::Transform(const Matrix3D &M)
{

  origin *= M;
  direction.TransformDirection(M);
  
}  //====Line3D::Transform====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| TransformTo
//|
//| Purpose: This applies a transformation to this line, and puts
//|          the result in another line
//|
//| Parameters: M:      the matrix of the transformation
//|             result: the line to put the result in
//|
//| Created by Maarten Gribnau
//|___________________________________________________________________

void Line3D::TransformTo(const Matrix3D &M, Line3D &result)
{

  result.origin = origin * M;
  result.direction = direction * M;
  
}  //====Line3D::TransformTo====


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Operator << for Line3D
//| 
//| Purpose: This operator outputs a Line3D to an output stream
//|
//| Parameters: o: the output stream
//|             l: the line to output
//|
//| Created by Maarten Gribnau
//| 
//| Modified:
//|________________________________________________________________

ostream &operator << (ostream &o, const Line3D& l)
{

  o << "lineOrigin: " << l.origin << '\n';
  o << "lineDirection: " << l.direction << '\n';
  o << '\n';

  return o;

}  //==== operator << for Line3D ====



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Operator >> for Line3D
//| 
//| Purpose: This operator inputs a Line3D from an input stream
//|
//| Parameters: i: the output stream
//|             l: the line
//|
//| Created by Maarten Gribnau
//| 
//| Modified:
//|________________________________________________________________

istream &operator >> (istream &in, Line3D &l)
{

  char dummy[25];
			   
  in >> dummy >> l.origin;
  in >> dummy >> l.direction;

  return in;

}  //==== operator << for Line3D ====



