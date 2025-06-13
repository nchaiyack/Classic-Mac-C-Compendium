//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CPrimitive class.  A CPrimitive
//| is a graphics primitive (like a line, a point, or a polygon).
//|________________________________________________________________________________

#pragma once

#include <CPtrArray.h>

class CPrimitive
	{

  protected:
  
	CPtrArray<RGBColor>	*colors;			//  Pointer to the colors list

  public:

	short			color_index;			//  The color of this primitive (as index in color list)

	CPrimitive(CPtrArray<RGBColor> *colors);
	
	virtual void	Draw(RGBColor *override_color, Point *screen_vertices,
							Rect *clip_rect, Boolean fAntialias);

	};
