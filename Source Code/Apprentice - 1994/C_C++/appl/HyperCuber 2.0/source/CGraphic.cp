//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the implementation of the Graphic class.  A Graphic is a
//| collection of graphics primitives.
//|________________________________________________________________________________


#include "CGraphic.h"
#include "CHyperCuberPane.h"
#include "CHyperCuberPrefs.h"
#include "CPath.h"
#include "CPolygon.h"
#include "CPPoint.h"
#include "CPrimitive.h"
#include <LongQD.h>

#include <math.h>
#include <string.h>
#include <stdlib.h>


//============================= Protypes ==============================\\

extern double **CreateRotMatrix(long dim, double *angles);
extern void UpdateRotMatrix(long i, double old_value, double new_value, long dim, 
                            double **matrix_handle);
extern void RotateVector(double *vector, double *rotated_vector, long dim, double *matrix);
extern void BuildRotMatrix(long dim, double *angles, double **matrix_handle);
void ProjectVector(double *point, double *proj_point, long dimension, short persp_index);


//============================= Constants ==============================\\

#define	Pi				3.14159265358979323846
#define EYE_SEPARATION	Pi/30


//===================================== Globals =====================================\\

extern CHyperCuberPrefs		*gPrefs;		//  The preferences



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CGraphic::IGraphic
//|
//| Purpose: Initialize a Graphic.
//|
//| Parameters: none
//|_________________________________________________________

void	CGraphic::IGraphic(void)
{

	primitives = new(CList);							//  Initialize the primitives list
	primitives->IList();

	colors = new(CList);								//  Initialize the colors list
	colors->IList();

	vertices = new(CList);								//  Initialize the vertices lists
	vertices->IList();
	stereo_vertices = new(CList);	
	stereo_vertices->IList();

	rotation_matrices = new(CList);						//  Initialize the rotation matrices lists
	rotation_matrices->IList();
	stereo_matrices = new(CList);
	stereo_matrices->IList();

	angles = new(CList);
	angles->IList();

}	//==== CGraphic::IGraphic() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CGraphic::Dispose
//|
//| Purpose: Dispose of a Graphic
//|
//| Parameters: none
//|_________________________________________________________

	void dispose_handle(CObject *object)
	{
		DisposeHandle((Handle) object);
	}


void	CGraphic::Dispose(void)
{

	primitives->DisposeAll();							//  Dispose of the primitives

	long i;
	for (i=1; i <= num_vertices; i++)
		DisposeHandle((Handle) vertices->NthItem(i));	//  Dispose of the vertices lists
	
	for (i=1; i <= 2; i++)
		DisposeHandle(
				(Handle) stereo_vertices->NthItem(i));	//  Dispose of the stereo vertices lists

	vertices->Dispose();								//  Dispose of the lists of vertices lists
	stereo_vertices->Dispose();

	for (i=3; i <= dimension; i++)
		DisposeHandle(
				(Handle) rotation_matrices->NthItem(i));//  Dispose of the rotation matrices

	DisposeHandle((Handle) stereo_matrices->NthItem(3));//  Dispose of the stereo rotation matrix

	rotation_matrices->Dispose();						//  Dispose of the rotation matrices lists
	stereo_matrices->Dispose();

	angles->DoForEach(dispose_handle);
	angles->Dispose();

	DisposHandle((Handle) perspective_params);			//  Dispose of the perspective parameters

}	//==== CGraphic::Dispose() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CGraphic::SetupGraphic
//|
//| Purpose: Set up a those parts of the graphic which depend on the dimension.
//|
//| Parameters: n: the dimension of this graphic
//|_____________________________________________________________________________

void CGraphic::SetupGraphic(long n)
{

	dimension = n;

	rotation_matrices->Append((CObject *) NULL);	//  Fill in null fields in the sines,
	rotation_matrices->Append((CObject *) NULL);	//    cosines, or rotation matrix for
	stereo_matrices->Append((CObject *) NULL);		//    1D and 2D, since we don't do 1D or
	stereo_matrices->Append((CObject *) NULL);		//    2D rotations
	angles->Append((CObject *) NULL);
	angles->Append((CObject *) NULL);
	
	long i;
	for (i=3; i <= dimension; i++)
		{

		double **angles_array =
			(double **) NewHandle(sizeof(double)*(i-1));//  Allocate space for angles
		
		long j;
		for (j = 0; j <= i-2; j++)						//  Fill in the sines and cosines array
			(*angles_array)[j] = 0;

		if (i == 3)
			{
			(*angles_array)[0] = 5*Pi/6;				//  Special view for 3D
			(*angles_array)[1] = 5*Pi/6;
			}
			
		HLock((Handle) angles_array);
		double **matrix = CreateRotMatrix(i,
										*angles_array);	//  Create this rotation matrix
		                            
		angles->Append((CObject *) angles_array);		//  Add this angles array to the list
		
		rotation_matrices->Append((CObject *) matrix);	//  Add this matrix to the list

		if (i == 3)
			{
			(*angles_array)[0] = 5*Pi/6 + EYE_SEPARATION;	//  Offset angle to provide other stereo view
			
			matrix = CreateRotMatrix(i, *angles_array);	//  Create another matrix for 3D stereo

			stereo_matrices->Append((CObject *) matrix);//  This matrix is used for stereo
		
			(*angles_array)[0] = 5*Pi/6;				//  Restore angle to normal
			
			}
		else
		
			stereo_matrices->Append((CObject *) matrix);//  Add this matrix to the stereo list

		HUnlock((Handle) angles_array);

		}

	perspective_params =								//  Allocate space for the perspective parms
			(long **) NewHandle(sizeof(long)*dimension);

	for (i = 0; i < dimension; i++)
		(*perspective_params)[i] = 3;					//  Make all perspective parameters 3
	
	(*perspective_params)[2] =
		(*perspective_params)[3] = 7;					//  ...except dimensions 3 and 4; then use 7
	
	
}	//==== CGraphic::SetupGraphic() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Operator >> for extracting a CGraphic
//|
//| Purpose: Read the graphic from the stream
//|
//| Parameters: none
//|__________________________________________________________________________

istream& operator>> (istream& s, CGraphic& graphic)
{

enum {
	point = 1,
	path,
	polygon
	};

	if (!s.ipfx)
		return s;

	long version;
	s >> version;										//  Get the version number
	
	long dimension;
	s >> dimension;										//  Get the dimensions of the image
	
	graphic.SetupGraphic(dimension);
		
	short reserved;
	s >> reserved;										//  Ignore the reserved fields
	s >> reserved;
	
	long num_vertices;
	s >> num_vertices;									//  Get the number of vertices
	graphic.num_vertices = num_vertices;

	Point **screen_point_array =					//  Allocate space for screen point array
			(Point **) NewHandle(
						num_vertices*sizeof(Point));
	
	graphic.vertices->Append(
					(CObject *) screen_point_array);	//  Put the screen point array in the
														//    1D vertices spot (since there are no
														//    1D vertices to store there)
	
	Point **stereo_screen_point_array =					//  Allocate space for the other screen point
						(Point **) NewHandle(			//    array (used for stereo).
							num_vertices*sizeof(Point));
	
	graphic.stereo_vertices->Append(
				(CObject *) stereo_screen_point_array);	//  Put the other screen point array in 1D
	
	double **vertex_list;
	long i;
	for (i = 2; i <= graphic.dimension; i++)			//  Allocate space for vertex arrays
		{
		
		vertex_list = (double **)NewHandle(				//  Create an array of verticies of dimension i.
						num_vertices*sizeof(double)*i);	//  Each vertex is an array of coordinates
		
		graphic.vertices->Append(						//  Add this array to the list of vertex arrays
					(CObject *) vertex_list);
		
		if (i == 2)
			vertex_list = (double **)NewHandle(			//  Create another array for 2D vertices.
						num_vertices*sizeof(double)*i);	//    (used for stereo)
			
		graphic.stereo_vertices->Append(				//  Add this to the list of stereo vertex arrays
							(CObject *) vertex_list);
		}
	
	//  ASSERTION: vertex_list points to space for "real" (unprojected) vertices
	
	double *coordinate = *vertex_list;					//  Point to first coordinate of
														//    first point
		
	for (i = 1; i <= num_vertices; i++)
		{

		char left_paren;
		s >> left_paren;								//	Get left parenthesis
		
		char comma;
		long j;
		for (j = 1; j<= graphic.dimension; j++)
			{
			s >> *coordinate;							//  Read in this coordinate
			s >> comma;									//  Read the comma (or ")" for last coordinate)
			coordinate++;								//  Point to next coordinate
			}
		
		// ASSERTION: comma = ")"
		// ASSERTION: coordinate points to first coordinate of next point
		
		}
	
	short num_colors;
	s >> num_colors;									//  Get the number of colors

	for (i = 1; i <= num_colors; i++)
		{
		RGBColor **color;
		
		color = (RGBColor **)
				NewHandle(sizeof(RGBColor));			//  Allocate space for color;
		
		char comma;
		s >> (*color)->red >> comma;					//	Get this color
		s >> (*color)->green >> comma;		
		s >> (*color)->blue;
		
		graphic.colors->Append((CObject *) color);		//  Add this color to the end of the colors list
		}
	
	short num_primitives;
	s >> num_primitives;								//  Get the number of primitives

	for (i = 1; i <= num_primitives; i++)
		{
		
		short primitive_type;
		s >> primitive_type;							//  Get the primitive type
		
		CPrimitive *primitive;							//  Create a new primitive of the correct type
		switch(primitive_type)
			{
			case point:
				primitive = (CPrimitive *) new (CPPoint);
				((CPPoint *) primitive)->IPPoint(graphic.colors);
				s >> *((CPPoint *) primitive);			//  Read the point in from the input stream
				break;
				
			case path:
				primitive = (CPrimitive *) new (CPath);
				((CPath *) primitive)->IPath(graphic.colors);
				s >> *((CPath *) primitive);			//  Read the path in from the input stream
				break;
				
			case polygon:
				primitive = (CPrimitive *) new (CPolygon);
				((CPolygon *) primitive)->IPolygon(graphic.colors);
				s >> *((CPolygon *) primitive);			//  Read the polygon in from the input stream
				break;
				
			}
		
		graphic.primitives->Append(primitive);			//  Add this primitive to the end of the list
		
		}
	
	return s;

}	//==== operator>> for reading CGraphic from file ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CGraphic::ClearDrawingArea
//|
//| Purpose: Clear the drawing area, in preparation for a Draw
//|
//| Parameters: color: the color to use
//|__________________________________________________________________________

void	CGraphic::ClearDrawingArea(RGBColor *color)
{

	GrafPtr temp_port;

	pane->Prepare();									//  Prepare to draw into pane

	if (pane->fDrawOffscreen)
		{
		GetPort(&temp_port);
		SetPort((GrafPtr) &(pane->OffscreenPort));		//  Prepare to draw off-screen
		}

	Rect pane_rect;										//  Find the pane rect
	LongRect long_pane_rect;
	pane->GetFrame(&long_pane_rect);
	LongToQDRect(&long_pane_rect, &pane_rect);
	ClipRect(&pane_rect);

	RGBColor temp_color;
	GetBackColor(&temp_color);
	RGBBackColor(color);								//  Erase the drawing area
	EraseRect(&pane_rect);
	RGBBackColor(&temp_color);

	if (pane->fDrawOffscreen)
		SetPort(temp_port);								//  Restore the old port

}	//==== CGraphic::ClearDrawingArea() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CGraphic::Draw
//|
//| Purpose: Draw the graphic
//|
//| Parameters: none
//|__________________________________________________________________________

void	CGraphic::Draw(void)
{

	GrafPtr temp_port;

	pane->Prepare();											//  Prepare to draw into pane

	if (pane->fDrawOffscreen)
		{
		GetPort(&temp_port);
		SetPort((GrafPtr) (&pane->OffscreenPort));				//  Prepare to draw off-screen
		}

	if (pane->StereoMode == two_color_stereo)					//  Draw in AddOver mode for 2-img
		PenMode(addOver);

	long num_primitives = primitives->GetNumItems();			//  Find the number of primitives

	RGBColor *draw_color =
			(pane->fUseNativeColors) ? NULL :
										&(pane->GraphicColor);	//  Use same color for all primitives
																//    if desired
	Boolean fAntialias = pane->fAntialias;
	
	LongRect long_frame_rect;
	Rect frame_rect;
	pane->GetFrame(&long_frame_rect);
	LongToQDRect(&long_frame_rect, &frame_rect);
	
	Point **screen_points = (Point **) vertices->FirstItem();	//  Get screen points list
	
	long i;
	Point **stereo_screen_points;
	Rect left_frame_rect, right_frame_rect;
	switch(pane->StereoMode)
		{
		case mono:
			break;
		
		case two_image_stereo:

			left_frame_rect = frame_rect;						//  Split frame into two square rects
			right_frame_rect = frame_rect;
			
			long frame_height = frame_rect.bottom - frame_rect.top;
			
			left_frame_rect.right -= frame_height;
			right_frame_rect.left += frame_height;		

			stereo_screen_points =
					(Point **) stereo_vertices->FirstItem();	//  Get stereo screen points list
			break;
			
		case two_color_stereo:

			stereo_screen_points =
					(Point **) stereo_vertices->FirstItem();	//  Get stereo screen points list
			break;
			
		}
					
	for (i = 1; i <= num_primitives; i++)						//  Draw the primitives
		{
		CPrimitive *primitive =
					(CPrimitive *) primitives->NthItem(i);		//  Get this primitive
		
		
		switch(pane->StereoMode)
			{
			case mono:
				((CPrimitive *) primitives->NthItem(i))->
								Draw(draw_color, screen_points,
									&frame_rect, fAntialias);	//  Draw the primitive
				break;
				
			case two_image_stereo:
				((CPrimitive *) primitives->NthItem(i))->
							Draw(draw_color, screen_points,
								&right_frame_rect, fAntialias);	//  Draw primitive (left view)
	
				((CPrimitive *) primitives->NthItem(i))->
							Draw(draw_color, stereo_screen_points,
								&left_frame_rect, fAntialias);	//  Draw primitive (right view)
				break;
				
			case two_color_stereo:
				((CPrimitive *) primitives->NthItem(i))->
							Draw(&(gPrefs->prefs.right_eye_color),
								screen_points,
									&frame_rect, fAntialias);	//  Draw primitive (left view)
	
				((CPrimitive *) primitives->NthItem(i))->
							Draw(&(gPrefs->prefs.left_eye_color),
								stereo_screen_points,
									&frame_rect, fAntialias);	//  Draw primitive (right view)
				break;
				
			}
		}

	if (pane->StereoMode == two_color_stereo)					//  Switch back to SrcCopy mode
		PenMode(srcCopy);

	if (pane->fDrawOffscreen)									//  If we drew offscreen, update the
																//    pane now
		{
		SetPort(temp_port);										//  Restore the old port

		pane->Prepare();										//  Prepare to update pane
	
		Rect dest_rect;
		LongToQDRect(&(pane->frame), &dest_rect);				//  Get area of pane
	
		RGBColor rgb_white = {0xFFFF, 0xFFFF, 0xFFFF};
		RGBBackColor(&rgb_white);								//  Make sure background color is white

		CopyBits((BitMapPtr) *(pane->OffscreenPort.portPixMap),
					(BitMapPtr) *(((CGrafPtr)thePort)->portPixMap), 
					&dest_rect, &dest_rect, srcCopy, NULL);
		}


}	//==== CGraphic::Draw() ====\\



#if 0

//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CGraphic::DrawAxes
//|
//| Purpose: Draw the n-dimensional axes
//|
//| Parameters: none
//|__________________________________________________________________________

void	CGraphic::DrawAxes(void)
{

	

}	//==== CGraphic::DrawAxes() ====\\

#endif



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CGraphic::OffsetAngle
//|
//| Purpose: Change one of the angles by some offset.
//|
//| Parameters: dimension: dimension this angle controls
//|             angle:     the angle to change
//|             offset:    the amount to change the angle by
//|__________________________________________________________________________

void	CGraphic::OffsetAngle(long dimension, long angle, double offset)
{

	double *angles_array =
		*((double **) angles->NthItem(dimension));		//  Find the array of angles
	
	ChangeAngle(dimension, angle,
					angles_array[angle-1] + offset);	//  Change angle to old value + offset
	
}	//==== CGraphic::OffsetAngle() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CGraphic::ChangeAngle
//|
//| Purpose: Change one of the angles.
//|
//| Parameters: dimension: dimension this angle controls
//|             angle:     the angle to change
//|             value:     new value of the angle
//|__________________________________________________________________________

void	CGraphic::ChangeAngle(long dimension, long angle, double value)
{

	double *angles_array =
		*((double **) angles->NthItem(dimension));		//  Find the array of angles
	
	double current_value = angles_array[angle-1];		//  Get the current value of the angle
	angles_array[angle-1] = value;						//  Change the value to the new value  

	double **rotation_matrix =
			((double **) rotation_matrices->
								NthItem(dimension));	//  Get the rotation matrix

	// DEBUG: this is a "safer" way to build the matrix

//	BuildRotMatrix(dimension, angles_array, rotation_matrix);		//  Build the matrix


	UpdateRotMatrix(angle-1, current_value, value,
				dimension, rotation_matrix);			//  Update the rotation matrix
	
	if ((pane->StereoMode != mono) && (dimension == 3))
		{
		
		if (angle == 1)
			{
			value += EYE_SEPARATION;					//  Offset value slightly for stereo angle
			current_value += EYE_SEPARATION;
			}
				
		rotation_matrix =
				((double **) stereo_matrices->
									NthItem(dimension));//  Get the stereo rotation matrix
	
		UpdateRotMatrix(angle-1, current_value, value,
							dimension, rotation_matrix);//  Update the rotation matrix
	
		}

	Project(dimension);									//  Project object with new angle

}	//==== CGraphic::ChangeAngle() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CGraphic::SwitchToStereo
//|
//| Purpose: Prepare to stereo viewing mode.
//|
//| Parameters: none
//|______________________________________________________________________

void	CGraphic::SwitchToStereo(void)
{

	double **rotation_matrix =
				((double **) rotation_matrices->
									NthItem(3));	//  Get address of 3D rotation matrix
	double **stereo_rotation_matrix =
				((double **) stereo_matrices->
									NthItem(3));	//  Get address of stereo 3D rotation matrix
	
	BlockMove(*rotation_matrix,
				*stereo_rotation_matrix,
					9*sizeof(double));				//  Copy normal rotation matrix to stereo
	
	double *angles_array =
		*((double **) angles->NthItem(3));			//  Find the array of angles

	UpdateRotMatrix(0, angles_array[0],
					angles_array[0] + EYE_SEPARATION,
					3, stereo_rotation_matrix);		//  Change stereo matrix to stereo view

	Project(3);										//  Project the object in stereo

}	//==== CGraphic::SwitchToStereo() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CGraphic::ChangePerspective
//|
//| Purpose: Change the amount of perspective.
//|
//| Parameters: dimension:   dimension to change perspective in
//|             perspective: new amount of perspective
//|__________________________________________________________________________

void	CGraphic::ChangePerspective(long dimension, long perspective)
{

	(*perspective_params)[dimension-1] = perspective;	//  Change the parameter
	
	Project(dimension);									//  Project object with new perspective

}	//==== ChangePerspective() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CGraphic::Project
//|
//| Purpose: Project the the vertices from the n-dimensional vertex list 
//|          to the (n-1)-dimensional vertex list.  This also propagates
//|          downward to (n-2), etc, down to 2D and the screen points.
//|
//| Parameters: n: dimension to project from
//|__________________________________________________________________________

void	CGraphic::Project(long n)
{

	double *point =										//  Get handle to first point in the n-D list
				*((double **) vertices->NthItem(n));	//    vertex list
	double *proj_point =								//  Get handle to the first point in the
				*((double **) vertices->NthItem(n-1));	//    (n-1)-D vertex list
	
	double *stereo_point;
	double *stereo_proj_point;
	double *stereo_rotmatrix;
	if ((pane->StereoMode != mono) && (n == 3))
		{
		stereo_point = *((double **)
						stereo_vertices->NthItem(3));	//  Handle to the first point in 3D list

		stereo_proj_point = *((double **)
						stereo_vertices->NthItem(2));	//    2D stereo vertex list

		stereo_rotmatrix = *((double **)
						stereo_matrices->NthItem(n));	//  Get the rotation matrix
		}
	
	double **rotated_point_handle =
			(double **) NewHandle(sizeof(double) * n);
	double *rotated_point = *rotated_point_handle;

	double *rotmatrix =									//  Get the rotation matrix
			*((double **) rotation_matrices->NthItem(n));

	long vertex;
	for (vertex = 1; vertex <= num_vertices; vertex++)
		{
		
		RotateVector(point, rotated_point, n, rotmatrix);	//  Rotate this point
		ProjectVector(rotated_point, proj_point, n,
						(*perspective_params)[n-1]);		//  Project this point

		point += n;											//  Point to next n-D point
		proj_point += (n-1);								//  Point to next (n-1)-D point

		if ((pane->StereoMode != mono) && (n == 3))			//  Do the same for the stereo view
			{
	
			RotateVector(stereo_point, rotated_point, n,
							stereo_rotmatrix);				//  Rotate this point
			ProjectVector(rotated_point, stereo_proj_point,
							n, (*perspective_params)[n-1]);	//  Project this point
			
			stereo_point += n;								//  Point to next n-D point
			stereo_proj_point += (n-1);						//  Point to next (n-1)-D point
	
			}
		}

	DisposHandle((Handle) rotated_point_handle);			//  Free up memory used by rotated point

	if (n > 3)
		Project(n-1);										//  Recursively project downward to 3D

	else
		FitToPane();										//  Map 2D points to screen points

}	//==== CGraphic::Project() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure: ProjectVector
//|
//| Purpose: Project a vector from n-space to (n-1)-space, with perspective
//|          if desired.
//|
//| Parameters: point:       the point to project
//|             proj_point:  receives the projected point
//|             dimension:   the dimension of point
//|             persp_index: the perspective coefficient (0 for no perspective)
//|_____________________________________________________________________________

void ProjectVector(double *point, double *proj_point, long dimension, short persp_index)
{

	const double persp_table[11] = {0.0, 50.0, 25.5, 15.5, 12.0, 9.0, 6.5, 4.5, 3.0, 2.5, 2.0};

	double perspective = persp_table[persp_index];

	if (perspective != 0)							//  Project point with perspective
		{
		double t = perspective /
				(perspective - point[0]);
		
		if (t<0) t = -t;
		
		long i;
		for (i = 1; i < dimension; i++)
			proj_point[i-1] = t*point[i];
		}
		
	else											//  No perspective-- just project it straight
		BlockMove(point+1, proj_point,
							sizeof(double)*(dimension-1));
		
}	//==== ProjectVector() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CGraphic::FitToPane
//|
//| Purpose: Map the 2D vertices to their pane equivalents
//|
//| Parameters: fOtherView: none
//|__________________________________________________________________________

void	CGraphic::FitToPane(void)
{

	double ymin = -3;
	double ymax = 3;
	double xmin = -3;
	double xmax = 3;
	
	Point *screen_points = *((Point **) vertices->FirstItem());	//  Get pointer to screen points list
	double *points2D = *((double **) vertices->NthItem(2));		//  Get pointer to 2D points list

	Point *stereo_screen_points = *((Point **)
								stereo_vertices->FirstItem());	//  Get pointer to screen points list
	double *stereo_point2D = *((double **)
						stereo_vertices->NthItem(2));			//  2D stereo vertex list

	short pane_left;
	short pane_right;
	short pane_top;
	short pane_bottom;
	short other_pane_left;
	short other_pane_right;
	short other_pane_top;
	short other_pane_bottom;

	short pane_width;
	switch (pane->StereoMode)
		{
		case mono:												//  Use area once
		
			pane_left = pane->frame.left;
			pane_right = pane->frame.right;
			pane_top = pane->frame.top;
			pane_bottom = pane->frame.bottom;
		
			break;
		
		case two_image_stereo:									//  Split pane into two areas
	
			pane_width = pane->frame.right - pane->frame.left;

			pane_right = pane->frame.right;
			pane_left = pane_right - pane_width/2;
			
			other_pane_top = pane_top = pane->frame.top;
			other_pane_bottom = pane_bottom = pane->frame.bottom;
		
			other_pane_right = pane_left;
			other_pane_left = pane_left - pane_width/2;
			
			break;

		case two_color_stereo:									//  Use same area twice

			other_pane_left = pane_left = pane->frame.left;
			other_pane_right = pane_right = pane->frame.right;
			other_pane_top = pane_top = pane->frame.top;
			other_pane_bottom = pane_bottom = pane->frame.bottom;
		
			break;
		}
	
	long i;
	for (i = 1; i <= num_vertices; i++)
		{

		screen_points->h = pane_left +
							(pane_right - pane_left) *
								(*points2D - xmin) / (xmax - xmin);
		
		points2D++;
		
		screen_points->v = pane_bottom -
							(pane_bottom - pane_top) *
								(*points2D - ymin) / (ymax - ymin);
		
		points2D++;
		screen_points++;

		}

	if (pane->StereoMode != mono)									//  Fit the other stereo image
		{

		screen_points = *((Point **) stereo_vertices->FirstItem());	//  Get pointer to screen points list
		points2D = *((double **) stereo_vertices->NthItem(2));		//  Get pointer to 2D points list
	
		for (i = 1; i <= num_vertices; i++)
			{
	
			screen_points->h = other_pane_left +
								(other_pane_right - other_pane_left) *
									(*points2D - xmin) / (xmax - xmin);
			
			points2D++;
			
			screen_points->v = other_pane_bottom -
								(other_pane_bottom - other_pane_top) *
									(*points2D - ymin) / (ymax - ymin);
			
			points2D++;
			screen_points++;
	
			}		
		}

}	//==== FitToPane() ====\\

