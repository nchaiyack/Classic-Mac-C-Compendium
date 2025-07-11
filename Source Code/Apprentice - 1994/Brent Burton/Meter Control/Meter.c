/**************************************************************************
** Meter.c
**
** Implements an analog meter-style dial control.
**
**
** Brent Burton,   12/10/92
** brentb@math.tamu.edu
**
***************************************************************************/

#include <MacHeaders>
#include <Sane.h>
#include "Meter.h"

/*********************************************************************/

/***********
**  The following structure is used to store some information
**  concerning the current state of the control - like where the
**  last point was we drew, the last value, etc.  The value and
**  position of the needle are recorded so that when we draw new
**  values, we can use a fast XOR operation rather than clearing
**  the area with EraseRect() or EraseOval() or whatever.  The
**  other information about center and radius is updated whenever
**  the control is to be redrawn - it might have moved or changed shape.
************/

typedef struct {
	PicHandle	thePict;		/* picture for basic control shape */
	Point		center;			/* Center of meter's rectangle     */
	Point		radius;			/* radius of needle sweep area     */
	Point		lastPt;			/* last point needle was drawn to. */
	int			lastValue;		/* last value of control           */
} CI, *CIP, **CIH;				/* Control Information record      */


typedef struct {		/* Record passed via param during thumbCntl msg */
	Rect limitRect, slopRect;
	int		axis;
} TR, *TRP;


/**********************************************************************/

pascal long		main(int, ControlHandle, int, long);
static void		Draw( ControlHandle, long);
static long		Test( ControlHandle, long);
static void		DragNeedle(ControlHandle);

static void		DrawNeedle(ControlHandle, Point, Point);
static void		DrawValue(ControlHandle, int, int);
static void		ResetPoints(ControlHandle);
static void		MapValue2Pt(ControlHandle, Point *);
static int		MapValue2Angle(ControlHandle );
static int 		MapAngle2Value(ControlHandle , int );
static void		MapAngle2Pt(ControlHandle , int , Point *);
static int		MapPt2Angle(ControlHandle , Point);

/**********************************************************************/
pascal long main(int varCode, ControlHandle theControl,
				 int message, long param)
{
	Handle h;

	switch (message) {
	
		case drawCntl:
			Draw( theControl, param);
			return(0);
			break;
			
		case testCntl:
			return Test( theControl, param);
			break;
			
		case calcCRgns:		/* 24-bit mode way of getting regions */
			if ( param & 0x80000000 )
			{
				param = (long)StripAddress( (Ptr)param);
				SetEmptyRgn( (Handle)param);
			}
			else
			{
				param = (long)StripAddress( (Ptr)param);
				RectRgn( (Handle)param, &(**theControl).contrlRect);
			}
			return param;
			break;
		
		case calcThumbRgn:	/* used in 32-bit mode instead */
			{
				param = (long)StripAddress( (Ptr)param);
				SetEmptyRgn( (Handle)param);
			}
			return param;
			break;
			
		case calcCntlRgn:	/* used in 32-bit mode instead */
			{
				param = (long)StripAddress( (Handle)param);
				RectRgn( (Handle)param, &(**theControl).contrlRect);
			}
			return param;
			break;
		
		case initCntl:	/* load the picture resource */
			{
				Rect r = (**theControl).contrlRect;
				
				h = (**theControl).contrlData = NewHandle( sizeof(CI));
	
				(**(CIH)h).thePict = (PicHandle)GetPicture( kMeterPict);
				ResetPoints( theControl);
				(**(CIH)h).lastValue = (**theControl).contrlValue;
				MapValue2Pt( theControl, &(**(CIH)h).lastPt);
							
				(**theControl).contrlAction = NULL;	/*  */
				return(0);
				break;
			}
			
		case dispCntl:	/* dispose of picture resource */
			h = (**theControl).contrlData;
			ReleaseResource( (**(CIH)h).thePict);	/* delete picture */
			DisposHandle( h);					/* delete data block */
			(**theControl).contrlData = NULL;
			return(0);
			break;
			
		case posCntl:		/* define this to position ind after drag */
			return(0);
			break;
			
		case thumbCntl:
			{
				((TRP)param)->limitRect	= (**theControl).contrlRect;
				((TRP)param)->slopRect	= (**theControl).contrlRect;
				((TRP)param)->axis		= noConstraint;
			}
			return(0);
			break;
			
		case dragCntl:
			if (param != 0L)		/* drag just indicator */
			{
				DragNeedle( theControl);
				return(1);
			}
			else			/* drag entire control */
			{
				return(0);
			}
			break;
			
		case autoTrack:
			return(0);
			break;
	} /* switch() */
	return(0);		/* just in case; should not be reached */
} /* main() */


/******************
** Draw()
**
** Draws at least the needle of the control, and also
** draws the entire control when told.
*******************/

static void Draw(ControlHandle theControl, long param)
{
	Rect r = (**theControl).contrlRect;
	GrafPtr savedPort;
	CIH cData = (CIH)(**theControl).contrlData;
	Point newpt;
	

	if ( (**theControl).contrlVis == 0)
		return;		/* do nothing */

	GetPort(&savedPort);
	SetPort( (**theControl).contrlOwner);	/* owner window */
	
	/*** which parts to draw... if param=0, draw them all */
	
	if ( param == 0 )	/* draw basic control shape */
	{
		CIH	cData = (CIH)(**theControl).contrlData;
		ResetPoints(theControl);
		DrawPicture( (**cData).thePict, &r);
		FrameRect(&r);
	}
	
	/********* Draw the indicator - the meter's "needle" *************/
	MapValue2Pt( theControl, &newpt);
	DrawNeedle( theControl, newpt, (**cData).lastPt);
	(**cData).lastPt = newpt;			/* update the last point drawn */
	
	/********* Now print the value of the control at the bottom. ****/
	DrawValue( theControl, (**theControl).contrlValue,
				(**cData).lastValue);
	(**cData).lastValue = (**theControl).contrlValue;
	
	SetPort( savedPort);
} /* Draw() */



/***********************
** Test()
**
** This is called when the function
** receives the testCntl message.
** It determines where in the control
** the point lies, and returns the
** corresponding part code.
*************************/

static long Test(ControlHandle theControl, long param)
{
	Point testPt, current;
	int currang, testang;	/* current and test points' angles */
	
	testPt = *(Point*)&param;	/* fool C typechecking */

	if ( !PtInRect( testPt, &(**theControl).contrlRect) )
		return 0L;
	
	MapValue2Pt( theControl, &current);
	currang = MapPt2Angle(theControl, current);
	testang = MapPt2Angle(theControl, testPt);	/* angles are in [0,270] */

	if (testang == -1)			/* then it's outside needle sweep */
		return 0L;
	
	if ( testang < currang - 3)
		return inTurnDown;
	else if (testang > currang + 3)
		return inTurnUp;
	else
		return inNeedle;		/* angle offsets allow a total range of 7� */

} /* Test() */


/***********************
** DragNeedle()
**
** This watches the mouse position and whenever
** the position changes, it updates the needle
** position and the control's value.  Everything
** is redrawn.  When the mouse button is
** released and it's in the legal control region,
** the control's value is finally updated and redrawn.
************************/

static void DragNeedle(ControlHandle theControl)
{
	Point newPos, oldPos, oldMouse;
	int angle, newValue, oldValue, valid=0;
	CIH	cData = (CIH)(**theControl).contrlData;
	
	oldValue = (**cData).lastValue;
	
	GetMouse( &oldMouse);
	oldPos = (**cData).lastPt;

	while ( StillDown() )
	{
		GetMouse( &newPos);
		
		/** Did the mouse move?  If so, get new position and update. **/
		if	( (newPos.h != oldMouse.h) || (newPos.v != oldMouse.v))
		{
			oldMouse = newPos;
			valid = PtInRect( newPos, &(**theControl).contrlRect) &&
					((angle = MapPt2Angle( theControl, newPos)) != -1);
			
			if (valid)
			{
				MapAngle2Pt( theControl, angle - 135, &newPos);
				DrawNeedle( theControl, newPos, oldPos);
	
				newValue = MapAngle2Value( theControl, angle);
				DrawValue( theControl, newValue, oldValue);
	
				oldValue = newValue;
				oldPos = newPos;
			}
			else	/* it's not in sweep region */
			{
				/* draw needle's current value */
				DrawNeedle( theControl, (**cData).lastPt, oldPos);
				oldPos = (**cData).lastPt;
				DrawValue(  theControl, (**cData).lastValue, oldValue);
				oldValue = (**cData).lastValue;
			} /* if */
		} /* if mouse position changed */
	} /* while button is pressed */
	
	if (valid)	/* then update control to new value */
	{
		(**theControl).contrlValue = newValue;
		MapAngle2Pt( theControl, angle - 135, &newPos);
		(**cData).lastPt = newPos;
		
		newValue = MapAngle2Value( theControl, angle);
		(**cData).lastValue = newValue;

	}
	Draw( theControl, 0);	/* simulate a Ctrl Mgr call. */
	return;
} /* DragNeedle() */



/**********************************************************************/
/** Following are some miscellaneous routines we need for utility    **/
/** purposes.                                                        **/
/**********************************************************************/


/***********************
** DrawNeedle()
**
** Draws the new needle position.  All
** this drawing makes for a fast refresh since
** we don't have to erase a large area explicitly.
************************/

static void DrawNeedle(ControlHandle theControl, Point newPos, Point oldPos)
{
	CIH cData = (CIH)(**theControl).contrlData;

	/*** draw the new needle ****/
	PenSize(1,1);
	MoveTo( (**cData).center.h, (**cData).center.v);
	LineTo( newPos.h, newPos.v);
	
	/********** Erase last needle **********/
	PenMode( notPatCopy);
	MoveTo( (**cData).center.h, (**cData).center.v);
	LineTo( oldPos.h, oldPos.v);

	/********** Draw the new needle to be sure *********************/
	/*** This ISN'T wasteful.  If the control is moved, bad      ***/
	/*** things happen, like needle doesn't reappear if we don't ***/
	/*** draw it again.                                          ***/
	
	PenMode( patCopy);
	MoveTo( (**cData).center.h, (**cData).center.v);
	LineTo( newPos.h, newPos.v);

} /* DrawNeedle() */

/***************************
** DrawValue()
**
** Updates the value readout at the bottom of the meter
** to reflect this value/corresponding needle position.
****************************/
static void DrawValue(ControlHandle theControl, int newVal, int oldVal)
{
	Str255	vstrnew, vstrold;
	int oldTmode, newwid, oldwid, xold, xnew, y;
	CIH cData = (CIH)(**theControl).contrlData;

	NumToString( (long)newVal, vstrnew);
	newwid = StringWidth( vstrnew);
	NumToString( (long)oldVal, vstrold);
	oldwid = StringWidth( vstrold);
	
	oldTmode = ((**theControl).contrlOwner)->txMode;	/* save old mode */
	xold = (**cData).center.h - oldwid/2;
	xnew = (**cData).center.h - newwid/2;
	y    = (**theControl).contrlRect.bottom - 2;
	
	TextMode( srcXor);
	MoveTo( xnew, y);
	DrawString( vstrnew);

	MoveTo( xold, y);
	DrawString( vstrold);

	TextMode( srcOr);
	MoveTo( xnew, y);
	DrawString( vstrnew);
	TextMode( oldTmode);				/* restore original mode */

} /* DrawValue() */


/***************************
** ResetPoints()
**
** It calculates the new center of the control
** and updates radius and center
****************************/
static void ResetPoints(ControlHandle theControl)
{
	Rect r = (**theControl).contrlRect;
	Point newctr, oldctr, offset;
	CIH h = (CIH)(**theControl).contrlData;	/* get our data */
	
	oldctr = (**(CIH)h).center;
	
	newctr.h = (r.left + r.right) /2;
	newctr.v = (r.top + r.bottom) /2;
	(**(CIH)h).center = newctr;	/* new values */
	(**(CIH)h).radius.h = 3 * (newctr.h - r.left) /4;
	(**(CIH)h).radius.v = 3 * (newctr.v - r.top) /4;

	offset.h = newctr.h - oldctr.h;
	offset.v = newctr.v - oldctr.v;
	
	(**(CIH)h).lastPt.h += offset.h;		/* Also move last needle point */
	(**(CIH)h).lastPt.v += offset.v;		/* to match new meter location */
} /* ResetPoints() */


/****************
** MapValue2Pt()
**
** Given a point and a control, this proc
** calculates the point to plot given the
** value of the control.  The point is 
** the location on the edge of the needle sweep area
** to connect to the center. (pt contains new point to plot.)
*****************/
static void MapValue2Pt(ControlHandle theControl, Point *pt)
{
	MapAngle2Pt( theControl, MapValue2Angle( theControl),  pt);
} /* MapValue2Pt() */


/**********************
** MapValue2Angle()
**
** given the control, return the angle
** represented by the current value.
***********************/
static int MapValue2Angle(ControlHandle theControl)
{
	int setting, cmin, cmax;
	setting = ( **theControl).contrlValue;
	cmin =    ( **theControl).contrlMin;
	cmax =    ( **theControl).contrlMax;
	return (int)((double)(setting - cmin)/(double)(cmax - cmin) * 
			(double)270.0) - 135;
}


/***********************
** MapAngle2Value()
**
** This calculates the meter's value that corresponds to the given
** needle angle.
************************/
static int MapAngle2Value(ControlHandle theControl, int angle)
{
	int crange;
	double percent;
	
	crange = (**theControl).contrlMax - (**theControl).contrlMin;
	percent = (double)angle/(double)270.0;
	return (int)(percent * (double)crange) + (**theControl).contrlMin;
} /* MapAngle2Value() */


/***********************
** MapAngle2Pt()
**
** Given a handle to a Control,
** and the angle of the current value, calculate and
** set pt to the point along the edge of the needle sweep
** area. NOTE: the angle passed here is in degrees and we
** have to convert it to radians.
************************/
static void MapAngle2Pt(ControlHandle theControl, int angle, Point *pt)
{
	CIH cData = (CIH)(**theControl).contrlData;
	double dangle;
	dangle = (double)angle * (double)(3.1415926)/(double)180.0;
	pt->h = (**cData).center.h +
			  (int)( ((**cData).radius.h - 1) * sin(dangle));
	pt->v = (**cData).center.v - 
			  (int)( ((**cData).radius.v - 1) * cos(dangle));
}


/************************
** MapPt2Angle()
**
** Given the control's rectangle and point
** in that rectangle, return the angle of
** that point normalized in the range [0, 270].
** If point is outside the desired area, return -1.
*************************/
static int MapPt2Angle(ControlHandle theControl, Point pt)
{
	int iangle;
	
	PtToAngle( &(**theControl).contrlRect, pt, &iangle);
	if ( !((136 <= iangle) && (iangle <= 224)) )
	{	iangle += 135;		/* put it in [0, 270] range */
		iangle %= 360;
	}
	else
		return(-1);		/* out of area, so return -1 */
	
	return (iangle);
} /* MapPt2Angle() */


/* fin */
