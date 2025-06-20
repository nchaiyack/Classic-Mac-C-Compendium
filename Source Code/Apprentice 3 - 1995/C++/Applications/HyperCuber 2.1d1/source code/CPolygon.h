//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CPolygon class.  A CPolygon
//| is a graphics primitive.  It is an optionally filled closed polygon,
//| with an optional border.
//|________________________________________________________________________________

#pragma once
#include "CPrimitive.h"
#include <iostream.h>

class CPane;

class CPolygon : public CPrimitive
	{
	
  public:

	short	boundary_color_index;					//  Boundary color of this polygon
	CPtrArray<long>	polygon_vertex_indices;			//  Vertices of this polygon


	CPolygon(CPtrArray<RGBColor> *colors);		//  Constructor
	
	void	Draw(RGBColor *override_color, Point *screen_vertices,
							Rect *clip_rect, Boolean fAntialias);

	friend istream& operator>> (istream& s, CPolygon& polygon);

	};
