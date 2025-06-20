//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the implementation of the CPath class.  A CPath
//| is a graphics primitive.  It implements a path composed of line segments.
//|________________________________________________________________________________


#include "CPath.h"
#include "CHyperCuberPrefs.h"
#include <CList.h>


//============================ External Globals ============================\\

extern CHyperCuberPrefs	*gPrefs;


//============================ Prototypes ============================\\

void DrawAntialiasedLine(long x1, long y1, long x2, long y2, RGBColor *color,
								Rect *clip_rect);


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPath::IPath
//|
//| Purpose: Initialize a path.
//|
//| Parameters: colors:   the color list
//|_________________________________________________________

void CPath::IPath(CList *colors)
{

	CPrimitive::IPrimitive(colors);					//  Initialize superclass

	path_vertex_indices = new(CList);				//  Initialize the path vertex index list
	path_vertex_indices->IList();
	
}	//==== CPath::IPath() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPath::Dispose
//|
//| Purpose: Dispose of the polygon.
//|
//| Parameters: none
//|_________________________________________________________

void CPath::Dispose(void)
{

	path_vertex_indices->Dispose();					//  Get rid of the path vertex index list

	CPrimitive::Dispose();							//  Dispose of superclass

}	//==== CPath::Dispose() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Operator >> to extract CPath from stream
//|
//| Purpose: Read this path from the input stream
//|
//| Parameters: none
//|_________________________________________________________

istream& operator>> (istream& s, CPath& p)
{

	s >> p.color_index;											//  Get index of this point's color

	short num_vertices;											//  Get number of vertices
	s >> num_vertices;	

	short i;
	for (i = 1; i <= num_vertices; i++)
		{
		long vertex_index;										//  Get index of the vertex
		s >> vertex_index;	
		
		p.path_vertex_indices->Append((CObject *)vertex_index);	//  Add the vertex to the path
		}
	
	return s;
	
}	//==== Operator >> to extract CPath from stream ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPath::Draw
//|
//| Purpose: Draw this path.
//|
//| Parameters: override_color:  the color to use to draw the primitive
//|                              If this is NULL, use the primitive's default
//|             screen_vertices: the screen coordinates of the vertices
//|             clip_rect:       the clipping rectangle
//|             fAntialias:      TRUE if we should antialias
//|__________________________________________________________________________

void CPath::Draw(RGBColor *override_color, Point **screen_vertices,
					Rect *clip_rect, Boolean fAntialias)
{
	
	short num_vertices = path_vertex_indices->GetNumItems();		//  Get number of points

	RGBColor *draw_color = (override_color) ?
							override_color :
							*((RGBColor **) colors->NthItem(color_index));

	long vertex_index = (long) path_vertex_indices->NthItem(1)-1;	//  Get index of the first point

	Point screen_point = *(*screen_vertices + vertex_index);	//  Get the screen point
		
	short i;
	if (fAntialias)		//  Antialias line segments
		{
		
		Point last_screen_point = screen_point;
		
		for (i = 2; i <= num_vertices; i++)
			{
			
			vertex_index = (long) path_vertex_indices->NthItem(i)-1;	//  Get index of this point
			screen_point = *(*screen_vertices + vertex_index);			//  Get this screen point

			DrawAntialiasedLine(screen_point.h, screen_point.v,
								last_screen_point.h, last_screen_point.v,
								draw_color,
								clip_rect);
			
			last_screen_point = screen_point;						//  Remember this point

			}
		}
	
	else								//  Don't antialias line segments
		{
		RGBForeColor(draw_color);									//  Use specified color, if any
	
		MoveTo(screen_point.h, screen_point.v);						//  Start here
	
		for (i = 2; i <= num_vertices; i++)
			{
			
			vertex_index = (long) path_vertex_indices->NthItem(i)-1;//  Get index of this point
			screen_point = *(*screen_vertices + vertex_index);		//  Get this screen point
	
			LineTo(screen_point.h, screen_point.v);					//  Draw the line segment
	
			}
		}

}	//==== CPath::Draw() ====\\
