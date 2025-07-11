//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the implementation of the CPolygon class.  A CPolygon
//| is a graphics primitive.  It implements an optionally filled closed polygon,
//| with an optional border.
//|________________________________________________________________________________


#include "CPolygon.h"


//============================ Prototypes ============================\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPolygon::CPolygon
//|
//| Purpose: Initialize a CPolygon.
//|
//| Parameters: colors:   the color list
//|_________________________________________________________

CPolygon::CPolygon(CPtrArray<RGBColor> *colors) : CPrimitive(colors)
{
	
}	//==== CPolygon::CPolygon() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Operator >> to extract CPolygon from stream
//|
//| Purpose: Read this polygon from the input stream
//|
//| Parameters: none
//|_________________________________________________________

istream& operator>> (istream& s, CPolygon& p)
{

	s >> p.color_index;											//  Get index of interior color
	s >> p.boundary_color_index;								//  Get index of boundary color
	
	short num_vertices;											//  Get number of vertices
	s >> num_vertices;	

	short i;
	for (i = 1; i <= num_vertices; i++)
		{
		long vertex_index;										//  Get index of the vertex
		s >> vertex_index;	
		
		p.polygon_vertex_indices.Append((long *) vertex_index);	//  Add this point to the polygon
		}
	
	return s;
	
}	//==== Operator >> to extract CPolygon from stream ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPolygon::Draw
//|
//| Purpose: Draw this polygon.
//|
//| Parameters: override_color:  the color to use to draw the primitive
//|                              If this is NULL, use the primitive's default
//|             screen_vertices: the screen coordinates of the vertices
//|             clip_rect:       the clipping rectangle
//|             fAntialias:      TRUE if we should antialias
//|__________________________________________________________________________

void CPolygon::Draw(RGBColor *override_color, Point *screen_vertices,
					Rect *clip_rect, Boolean fAntialias)
{

	PolyHandle polygon = OpenPoly();							//  Start recording a polygon

	long vertex_index =
					(long) polygon_vertex_indices.NthItem(1);	//  Get index of first point
	
	Point screen_point = *(screen_vertices + vertex_index);	//  Get the screen point

	MoveTo(screen_point.h, screen_point.v);						//  Start here

	short num_vertices = polygon_vertex_indices.GetNumItems();	//  Get number of points

	short i;
	for (i = 1; i < num_vertices; i++)
		{
		
		vertex_index =
					(long) polygon_vertex_indices.NthItem(i);	//  Get index of this point
		
		Point screen_point = *(screen_vertices + vertex_index);	//  Get the screen point

		LineTo(screen_point.h, screen_point.v);					//  Draw the line segment

		}

	ClosePoly();												//  Stop recording a polygon

	if (override_color)
		RGBForeColor(override_color);							//  Use specified color, if any

	else
		RGBForeColor(colors->NthItem(boundary_color_index));	//  Use default outline color

	FramePoly(polygon);											//  Draw the polygon's outline

	if (!override_color)
		RGBForeColor(colors->NthItem(color_index));				//  Use default fill color

	PaintPoly(polygon);											//  Fill the polygon

	KillPoly(polygon);											//  Get rid of polygon

}	//==== CPolygon::Draw() ====\\