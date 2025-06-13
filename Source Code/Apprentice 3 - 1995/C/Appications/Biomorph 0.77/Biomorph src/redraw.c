#include <MacHeaders>
#include "constants.h"
#include "mathtype.h"
#include "globals.h"

#define RECURSE 		// define this for recursive descent method

static MathType xgap;			// distance between pixels
static MathType ygap;


static Boolean CheckCancel(void);	// has the user pressed Cmd-. ?
static void DrawImage(Rect *rect);
static int CalcPoint(int x, int y);
#ifdef RECURSE
static void DrawRecurse( int xmin, int xmax, int ymin, int ymax);

static int sDone;		// 's' is for statically declared var -- local
static int count;		// counter for event checking
#endif

void Redraw(void)
{
	ReadCoords();

	DrawImage( &gMainWindow->portRect);
} // Redraw()


static void DrawImage(Rect *rect)
{
	register int x;
	register int y;
	GrafPtr savedPort;
	EventRecord theEvent;
	
	count = kEventCount;
	xgap = (gXmax - gXmin) / (MathType)rect->right;
	ygap = (gYmax - gYmin) / (MathType)rect->bottom;
	
	GetPort( &savedPort);   // get whichever window was last current
	SetPort( &gOffGP);			// clean up the offscreen bitmap
	EraseRect( &gOffGP.portRect);
	PenPat(black);
	PenMode(srcCopy);
	SetPort( gMainWindow);		// set port to the drawing area
	EraseRect( &gMainWindow->portRect);
	PenPat(black);
	PenMode(srcCopy);
	
#ifdef RECURSE

	// draw the image using divide & conquer
	sDone = 0;
	
	DrawRecurse( 0, rect->right -1, 0, rect->bottom -1);
	
	SysBeep(5);		// beep twice when done
	SysBeep(5);

#else
// ...else do not do recursive drawing.  Do a normal
// raster scan left to right, top to bottom.
//
	for (y=0; y< rect->bottom; y++)
	{
		for (x=0; x< rect->right; x++)
		{
			if (CalcPoint(x,y))
			{
				SetPort(gMainWindow);	// plot the main window's image
				MoveTo(x, y);
				Line(0,0);				// plot the pixel
	
				SetPort( &gOffGP);		// plot the offscreen bitmap's image
				MoveTo(x, y);
				Line(0,0);				// plot the pixel
			}

			if ( ! (--count))  // check for a Cmd-period to abort.
			{
				count = kEventCount;
				if (CheckCancel())	// did user cancel our pretty picture?
					goto end;
			} // if
		} // for x
	} // for y
	SysBeep(5);		// beep twice when done
	SysBeep(5);
#endif

end:
	SetPort(savedPort);
	return;
} // DrawImage()


static int CalcPoint(int x, int y)
{
	register int n;
	ImagPt	z, z0, C;
	Pattern thePat;
	
	C.r = gCreal;
	C.i = gCimag;
	z0.r = gXmin + xgap*x;
	z0.i = gYmin + ygap*y;
	z = z0;
	
	n=1;
	do {
		z0 = z;
		(**gMorphProcH)( &z0, &C, &z);	// iterate this function
		n++;
	}
	while ( (n <=10) && (dabs(z.r)<=10.0) &&
			(dabs(z.i)<=10.0) && (z.mag <=100));
	
	if ( (dabs(z.r) < 10.0) || (dabs(z.i) < 10.0) )
		return 1;	// pixel needs to be drawn
	else
		return 0;	// pixel is not drawn
	
} // CalcPoint()


static Boolean CheckCancel()
{
	EventRecord theEvent;
	
	// handle all events pending...
	
	while ( WaitNextEvent( everyEvent, &theEvent, gSleepTime, NULL))
		switch( theEvent.what)
		{
			case keyDown:
				if ((theEvent.modifiers & cmdKey) &&
					((theEvent.message & charCodeMask) == '.'))
				{
					SysBeep(1);
					return 1;
				}
				break;
			case mouseDown:
				{	WindowPtr whichWindow;
					int part;
					part = FindWindow(theEvent.where, &whichWindow);
					if (part == inMenuBar)
						(void)MenuSelect(theEvent.where);
				}
				break;
		}
	return 0;
} // CheckCancel()


void ZoomIn(void) // magnify by 2 == decrease range by half.
{
	ReadCoords();
	gXmax -= (gXmax - gCenterX)/2.0;  // liposuck the boundaries
	gXmin -= (gXmin - gCenterX)/2.0;
	gYmax -= (gYmax - gCenterY)/2.0;
	gYmin -= (gYmin - gCenterY)/2.0;
	WriteCoords();
} // ZoomIn()


void ZoomOut(void) // magnify by 0.5 == increase range by 100%.
{
	ReadCoords();
	gXmax = gCenterX + 2.0*(gXmax - gCenterX);
	gXmin = gCenterX + 2.0*(gXmin - gCenterX);
	gYmax = gCenterY + 2.0*(gYmax - gCenterY);
	gYmin = gCenterY + 2.0*(gYmin - gCenterY);
	WriteCoords();
} // ZoomOut()


#ifdef RECURSE
static void DrawRecurse( int xmin, int xmax, int ymin, int ymax)
{
	int xmid, ymid;
	
	if ( sDone) return;
	
	xmid = xmax + xmin + 1;  xmid >>= 1;  // take the inclusive average
	ymid = ymax + ymin + 1;  ymid >>= 1;  // take the inclusive average

// The returns inside each condition are so the code doesn't
// have to BRA outside it all on every call to this function.
// Having the return there eliminates costly jumps.

	if ( xmax == xmin)	// then don't split x
	{
		if (ymax == ymin)  // then single pixel -- do calculations
		{
			if (CalcPoint(xmin,ymin))
			{
				SetPort(gMainWindow);	// plot the main window's image
				MoveTo(xmin,ymin);
				Line(0,0);				// plot the pixel
	
				SetPort( &gOffGP);		// plot the offscreen bitmap's image
				MoveTo(xmin,ymin);
				Line(0,0);				// plot the pixel
			}
			
			if ( ! --count )  // check for a Cmd-period to abort periodically.
			{
				count = kEventCount;
				if (CheckCancel())	// did user cancel our pretty picture?
					sDone = 1;
			} // if
			return;
		}
		else
		{
			DrawRecurse( xmin, xmin, ymin, ymid-1);  // split y
			DrawRecurse( xmin, xmin, ymid, ymax);
			return;
		}
	}
	else // split X
	{
		if (ymax == ymin) // then split along X only
		{
			DrawRecurse( xmin, xmid-1, ymin, ymin);
			DrawRecurse( xmid, xmax, ymin, ymin);
			return;
		}
		else	// split both X and Y
		{
			DrawRecurse( xmin, xmid-1, ymin, ymid-1);	// top left
			DrawRecurse( xmid, xmax, ymin, ymid-1);		// top right
			DrawRecurse( xmin, xmid, ymid, ymax);		// bot left
			DrawRecurse( xmid, xmax, ymid, ymax);		// bot right
			return;
		}
	}
} // DrawRecurse()
#endif

