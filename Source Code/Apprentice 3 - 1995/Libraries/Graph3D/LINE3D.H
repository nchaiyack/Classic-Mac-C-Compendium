//******************************************************************************
// Title:       Line3D.h
// Author:      Maarten Gribnau
// Date:        07-04-1995
// Purpose:     Interface definition for Line3D class.
// Description: 
// Project:     Graph3D
// Remarks:     
//
// Revision history:
// Who:
// Date:
// What:
//******************************************************************************


#ifndef _Line3D_h
#define _Line3D_h
 
#include "Vector3D.h"
 

class Line3D {

public:

    Line3D(void);
	Line3D(const Vector3D *theOrigin, const Vector3D *theDirection);
	Line3D(const Vector3D &theOrigin, const Vector3D &theDirection);
	Line3D(const Line3D *theLine);
	Line3D(const Line3D &theLine);
	~Line3D(void);

	inline void SetOrigin(const Vector3D &newOrigin);
	inline void SetDirection(const Vector3D &newDirection);
	inline Vector3D Origin() const;
	inline Vector3D Direction() const;

	inline void ParameterToPoint(Real parameter, Vector3D &point) const; 
	inline Real PointToParameter(const Vector3D &point) const; 
	
	Flag Intersect(const Line3D &line, Real &par1, Real &par2);
	Flag Intersect(const Line3D &line, Vector3D &point1, Vector3D &point2);
	
    Vector3D& PointProject(const Vector3D &point);
    Real      ParameterProject(const Vector3D &point);
    
	void Transform(const Matrix3D &M);
	void TransformTo(const Matrix3D &M, Line3D &result);

	friend ostream &operator<< (ostream &o, const Line3D& l);
	friend istream &operator>> (istream &i, Line3D &l);

private:

	Vector3D origin; 
	Vector3D direction;

};
 
#endif  // #ifndef _Line3D_h
