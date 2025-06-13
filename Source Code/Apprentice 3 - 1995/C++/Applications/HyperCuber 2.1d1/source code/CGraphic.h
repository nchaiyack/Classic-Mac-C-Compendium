//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the Graphic class.  A Graphic
//| is a graphics object made of graphics primitives.
//|________________________________________________________________________________

#pragma once
#include <CPtrArray.h>
#include <iostream.h>

class CHyperCuberPane;
class CPrimitive;

class CGraphic
	{
	

	CPtrArray<double> rotation_matrices;	//  The rotation matrices.  The nth element in this
											//    list is a handle to the rotation matrix used
											//    to rotate n-space.

	CPtrArray<double> stereo_matrices;		//  This is the list of rotation matrices used for
											//    the second image used in stereo.  Note that all
											//    the matrices except the 3D->2D matrix are aliases
											//    of the matrices in the "rotation_matrices" list.

	long	*perspective_params;			//  The perspective paramaters.  The nth element in
											//    this array is the perspective parameter for
											//    n-space (a long).
											
	CPtrArray<double>	angles;				//  The sines of the viewing angles.  These are lists

  public:

	CPtrArray<CPrimitive>	primitives;		//  The primitives.  This is a list of primitives
	CPtrArray<RGBColor>		colors;			//  The colors.  This is a list of pointers to RGBColors

	long	num_vertices;					//  Number of vertices in the graphic
	CPtrArray<double> 	vertices;			//  The vertices.  This is a list of handles to matrices;
											//    each of the matrices contains a sequence of points
											//    of dimension n, where n is the index of the
											//    matrix into the vertices list.
											
	CPtrArray<double> stereo_vertices;		//  This is the vertices list used to project and draw
											//    the second image used in stereo.  Note that all
											//    vertex lists except the 2D and screen lists are
											//    aliases of those in the "vertices" list.
											
	long	dimension;						//  Dimension of this graphic

	CHyperCuberPane	*pane;					//  The pane this graphic is in

	CGraphic(void);							//  constructor
	~CGraphic(void);						//  destructor

	void	Draw(void);
	void	SetupGraphic(long n);
	void	ClearDrawingArea(RGBColor *color);
	
	void	OffsetAngle(long dimension, long angle, double offset);
	void	ChangeAngle(long dimension, long angle, double value);
	void	ChangePerspective(long dimension, long perspective);
	
	void	SwitchToStereo(void);

	void	Project(long n);
	void	FitToPane(void);
	
	friend istream& operator>> (istream& s, CGraphic& graphic);

	};
