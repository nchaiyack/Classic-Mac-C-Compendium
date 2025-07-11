/************************
** draw.c
**
** routines for drawing stuff
*************************/

#include <math.h>
#include "main.h"


/****************** local variables ******************/


#define NUMPOINTS 20
static Point pt_array[NUMPOINTS];
static short currentPt=0;
static short lastPt=0;

#define NextPointIndex(x)	(((x)==(NUMPOINTS-1))?0:(x)+1)
#define PrevPointIndex(x)	(((x)==0)?(NUMPOINTS-1):(x)-1)

static double angle=0.0;          /* current direction, in radians */


void DrawStep(void)
{
	Point center, next, oldPt1, oldPt2;
	static int inited=0;

	GetMouse( &center);
	if (!inited)  /* initialize the position array */
	{
		register int i;
		for (i=0;i<NUMPOINTS; i++)
			pt_array[i] = center;
		lastPt = 1;
		inited=1;
	} /* if */
	
	
	/** erase the last line segment **/
	oldPt1 = pt_array[ lastPt];
	oldPt2 = pt_array[ NextPointIndex(lastPt)];
	PenPat(&qd.white);
	MoveTo( oldPt1.h, oldPt1.v);
	LineTo( oldPt2.h, oldPt2.v);
	lastPt = NextPointIndex(lastPt);
	
	if (gListening)
		gOffset = FetchSndOffset();   /* get the current sound level */
	else
		gOffset = 0;
	
	next.h = (short)((gRadius+gOffset) * cos(angle)) + center.h;
	next.v = (short)((gRadius+gOffset) * sin(angle)) + center.v;
	
	PenPat(&qd.black);
	MoveTo( pt_array[ currentPt].h, pt_array[ currentPt].v);
	currentPt = NextPointIndex( currentPt);
	pt_array[ currentPt] = next;
	LineTo( pt_array[ currentPt].h, pt_array[ currentPt].v);

	angle += gAngleStep;
	if (angle > 2*PI) angle -= 2*PI;
	return;
} /* DrawStep() */

