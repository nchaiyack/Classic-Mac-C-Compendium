//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the implementation of the CPPoint class.  A CPPoint
//| is a graphics primitive.  It implements a colored point.
//|________________________________________________________________________________


#include "CPPoint.h"

#include <CList.h>


//============================ Prototypes ============================\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPPoint::IPPoint
//|
//| Purpose: Initialize a point.
//|
//| Parameters: colors:   the color list
//|_________________________________________________________

void CPPoint::IPPoint(CList *colors)
{

	CPrimitive::IPrimitive(colors);			//  Initialize superclass

}	//==== CPPoint::IPPoint() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPPoint::Dispose
//|
//| Purpose: Dispose of the point.
//|
//| Parameters: none
//|_________________________________________________________

void CPPoint::Dispose(void)
{

	CPrimitive::Dispose();

}	//==== CPPoint::Dispose() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Operator >> to extract CPPoint from stream
//|
//| Purpose: Read this point from the input stream
//|
//| Parameters: none
//|_________________________________________________________

istream& operator>> (istream& s, CPPoint& p)
{

	s >> p.color_index;											//  Get index of this point's color

	s >> p.vertex_index;										//  Get index of the vertex

	return s;
	
}	//==== Operator >> to extract CPPoint from stream ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPPoint::Draw
//|
//| Purpose: Draw this point.
//|
//| Parameters: override_color:  the color to use to draw the primitive
//|                              If this is NULL, use the primitive's default
//|             screen_vertices: the screen coordinates of the vertices
//|             clip_rect:       the clipping rectangle
//|             fAntialias:      TRUE if we should antialias
//|__________________________________________________________________________

void CPPoint::Draw(RGBColor *override_color, Point **screen_vertices,
					Rect *clip_rect, Boolean fAntialias)
{

	Point screen_point = *(*screen_vertices + vertex_index);	//  Get the 2D point
	
	RGBColor *color;
	if (override_color)
		color = override_color;									//  Use specified color, if any
	else
		color = *((RGBColor **) colors->NthItem(color_index));	//  Use default outline color

	SetCPixel(screen_point.h, screen_point.v, color);			//  Set the point to the proper color
		
}	//==== CPPoint::Draw() ====\\

