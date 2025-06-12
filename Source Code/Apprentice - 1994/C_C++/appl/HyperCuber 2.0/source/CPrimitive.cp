//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the implementation of the CPrimitive class.  A CPrimitive
//| is a graphics primitive (like a line, a point, or a polygon).
//|________________________________________________________________________________


#include "CPrimitive.h"


//============================ Prototypes ============================\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPrimitive::IPrimitive
//|
//| Purpose: Initialize a CPrimitive.
//|
//| Parameters: vertices: the 2D vertex list
//|             colors:   the color list
//|_________________________________________________________

void CPrimitive::IPrimitive(CList *colors)
{

	this->colors = colors;					//  Save the color list

}	//==== CPrimitive::IPrimitive() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPrimitive::Dispose
//|
//| Purpose: Dispose of the primitive.
//|
//| Parameters: none
//|_________________________________________________________

void CPrimitive::Dispose(void)
{


}	//==== CPrimitive::Dispose() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPrimitive::Draw
//|
//| Purpose: Draw this primitive.
//|
//| Parameters: override_color: the color to use to draw the primitive
//|                             If this is NULL, use the primitive's default
//|             screen_vertices: the screen coordinates of the vertices
//|             clip_rect:       the clipping rectangle
//|             fAntialias:      TRUE if we should antialias
//|__________________________________________________________________________

void CPrimitive::Draw(RGBColor *override_color, Point **screen_vertices,
						Rect *clip_rect, Boolean fAntialias)
{


}	//==== CPrimitive::Draw() ====\\

