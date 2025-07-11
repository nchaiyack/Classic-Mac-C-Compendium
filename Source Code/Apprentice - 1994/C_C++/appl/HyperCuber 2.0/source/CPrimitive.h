//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CPrimitive class.  A CPrimitive
//| is a graphics primitive (like a line, a point, or a polygon).
//|________________________________________________________________________________

#pragma once
#include <CObject.h>

CLASS CList;

class CPrimitive : public CObject
	{
	
	CList			*colors;				//  The colors list

  public:

	short			color_index;			//  The color of this primitive (as index in color list)

	void			IPrimitive(CList *colors);
	virtual void	Dispose(void);
	virtual void	Draw(RGBColor *override_color, Point **screen_vertices,
							Rect *clip_rect, Boolean fAntialias);

	};
