//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CPoint class.  A CPoint
//| is a graphics primitive.  It is a colored point.
//|________________________________________________________________________________

#pragma once
#include "CPrimitive.h"
#include <iostream.h>

class CPane;

class CPPoint : public CPrimitive
	{
	
  public:

	long			vertex_index;		//  The index of the vertex this point lies on
	
	CPPoint(CPtrArray<RGBColor> *colors);

	void			Draw(RGBColor *override_color, Point *screen_vertices,
							Rect *clip_rect, Boolean fAntialias);

	friend istream& operator>> (istream& s, CPPoint& path);

	};
