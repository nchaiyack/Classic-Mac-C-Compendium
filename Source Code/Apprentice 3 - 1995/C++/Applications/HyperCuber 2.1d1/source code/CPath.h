//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CPath class.  A CPath
//| is a graphics primitive.  It is path composed of line segments.
//|________________________________________________________________________________

#pragma once
#include "CPrimitive.h"
#include <iostream.h>

class CPane;

class CPath : public CPrimitive
	{
	

  public:

	CPtrArray<long>	path_vertex_indices;			//  Vertices of this path (by index number)

	CPath(CPtrArray<RGBColor> *colors);

	void	Read(void);
	void	Draw(RGBColor *override_color, Point *screen_vertices,
							Rect *clip_rect, Boolean fAntialias);

	friend istream& operator>> (istream& s, CPath& path);

	};
