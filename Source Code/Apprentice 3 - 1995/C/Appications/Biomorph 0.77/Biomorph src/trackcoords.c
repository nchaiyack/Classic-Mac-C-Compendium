/***************************
** trackcoords.c
**
** This source file contains TrackCoords(), which
** follows the mouse as it is dragged in the gMainWindow
** and updates the X & Y min/max image coords.
****************************/

#include <MacHeaders>
#include <stdio.h>
#include "constants.h"
#include "globals.h"

void TrackCoords(Point start)
{
	EventRecord theEvent;
	Handle	h;
	MathType xgap, ygap, mtemp;
static Point end2;
	Point end;
	Rect box;
static Rect selRect;	// selection rectangle
	Str255	s;
	int xmin, xmax, ymin, ymax, iType;
	
	SetPort(gMainWindow);
	PenPat(black);
	PenMode(patXor);
	
	GlobalToLocal(&start);
		
	xgap = (gXmax - gXmin) / (MathType)(gMainWindow->portRect.right -1);
	ygap = (gYmax - gYmin) / (MathType)(gMainWindow->portRect.bottom -1);

	while ( Button() )
	{
		SetPort(gMainWindow);
		GetMouse( &end);	// local coords already
		
		if ( (end2.h == end.h) &&
			(end2.v == end.v) )  // same point, so skip
		{
			(void)WaitNextEvent( everyEvent, &theEvent, 0, NULL);
			continue;
		}
		else				// different points, so do calcs
			end2 = end;
			
		if (start.h < end.h)	// set x min and max
		{
			xmin = start.h;
			xmax = end.h;
		}
		else
		{
			xmin = end.h;
			xmax = start.h;
		}
		
		if ( start.v < end.v)	// set y min and max
		{
			ymin = start.v;
			ymax = end.v;
		}
		else
		{
			ymin = end.v;
			ymax = start.v;
		}
		
		FrameRect( &selRect);
		selRect.left =xmin;
		selRect.right=xmax+1;
		selRect.top = ymin;
		selRect.bottom = ymax+1;
		FrameRect( &selRect);
		
		Print(NULL);	// Clears the message window
		NumToString((long)end.h, s);
		PtoCstr(s);
		Print( (char*)s);
		NumToString((long)end.v, s);
		PtoCstr(s);
		Print( (char*)s);
		
		// we now have x/y min/max values.
		// Convert pixel values to coordinate values.
		
		mtemp = xgap * (MathType)xmin + gXmin;
		sprintf( (char*)s, kMathFormat, mtemp);
		CtoPstr(s);
		GetDItem( gControlDialog, kETXMin, &iType, &h, &box);
		SetIText( h, s);
		
		mtemp = gXmin +  xgap * (MathType)xmax;
		sprintf( (char*)s, kMathFormat, mtemp);
		CtoPstr(s);
		GetDItem( gControlDialog, kETXMax, &iType, &h, &box);
		SetIText( h, s);
		
		mtemp = gYmin +  ygap * (MathType)ymin;
		sprintf( (char*)s, kMathFormat, mtemp);
		CtoPstr(s);
		GetDItem( gControlDialog, kETYMin, &iType, &h, &box);
		SetIText( h, s);
		
		mtemp = gYmin +  ygap * (MathType)ymax;
		sprintf( (char*)s, kMathFormat, mtemp);
		CtoPstr(s);
		GetDItem( gControlDialog, kETYMax, &iType, &h, &box);
		SetIText( h, s);
		
	} // while
	return;
	
} // TrackCoords()


