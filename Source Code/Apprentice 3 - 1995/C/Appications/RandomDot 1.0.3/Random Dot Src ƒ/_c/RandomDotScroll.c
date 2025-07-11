/* RandomDotScroll.c
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
#include "RandomDotMain.h"
#include "RandomDotRes.h"

#include "RandomDotWin.h"
#include "Utils.h"
#include "RandomDotScroll.h"

static void ScrollDraw(Integer x, Integer y);
static void	DoScrollDraw(Integer, Integer);

static void	HDThumb(ControlHandle, Integer);
static pascal void HThumb(void);
static void	InitHThumb(ControlHandle);
static void	InitVThumb(ControlHandle);
static pascal void PageDown(ControlHandle, Integer);
static pascal void PageLeft(ControlHandle, Integer);
static pascal void PageRight(ControlHandle, Integer);
static pascal void PageUp(ControlHandle, Integer);
static Integer PageWidth(void);
static pascal void ScrollDown(ControlHandle, Integer);
static pascal void ScrollLeft(ControlHandle, Integer);
static pascal void ScrollRight(ControlHandle, Integer);
static pascal void ScrollUp(ControlHandle, Integer);
static void VDThumb(ControlHandle, Integer);
static pascal void VThumb(void);

ScrollBarClassRec VProcs = {
	ScrollUp,	/* inUpButton */
	ScrollDown,	/* inDownButton */
	PageUp,		/* inPageUp */
	PageDown,	/* inPageDown */
	VThumb,		/* inThumb */
	InitVThumb,	/* InitThumb */
	VDThumb		/* DThumb */
};

ScrollBarClassRec HProcs = {
	ScrollLeft,	/* inUpButton */
	ScrollRight,/* inDownButton */
	PageLeft,	/* inPageUp */
	PageRight,	/* inPageDown */
	HThumb,		/* inThumb */
	InitHThumb,	/* InitThumb */
	HDThumb		/* DThumb */
};


/* ScrollUp - control proc calls us
 */
static pascal void ScrollUp(ControlHandle c, Integer p){
	if(inUpButton == p){
		ScrollDraw(0, -1);
	}
}

/* ScrollDown - control proc calls us
 */
static pascal void ScrollDown(ControlHandle c, Integer p){
	if(inDownButton == p){
		ScrollDraw(0, 1);
	}
}

/* PageHeight - return 9/10s of the window size
 */
static Integer PageHeight(void){
	return	9L*(qd.thePort->portRect.bottom - qd.thePort->portRect.top - kScrollBarWidth) / 10L;
}


/* PageWidth - return 9/10s of the window size rounded down to the nearest
   multiple of scale.
 */
static Integer PageWidth(void){
	return	9L*(qd.thePort->portRect.right - qd.thePort->portRect.left - kScrollBarWidth) / 10L;
}

/* PageUp -
 */
static pascal void PageUp(ControlHandle c, Integer p){
	if(inPageUp == p){
		ScrollDraw(0, -PageHeight());
	}
}

/* PageDown -
 */
static pascal void PageDown(ControlHandle c, Integer p){
	if(inPageDown == p){
		ScrollDraw(0, PageHeight());
	}
}

/* VThumb - thumb tracking. nothing special.
 */
static pascal void VThumb(){
}

/* VDThumb called C style by ContentDoc
   vertical done thumb
 */
static void VDThumb(ControlHandle c, Integer oldVal){
	if(oldVal != GetCtlValue(c)){
		DoScrollDraw(0, GetCtlValue(c) - oldVal);
	}
}

/* InitVThumb - set up for VDThumb
 */
static void InitVThumb(ControlHandle c){
}

/******* horizontal scroll bar functions ******/
/* these get executed repeatedly during the control activation */


/* ScrollLeft - called from dispatch proc
 */
static pascal void ScrollLeft(ControlHandle c, Integer p){
	if(inUpButton == p){
		ScrollDraw(-1, 0);
	}
}

/* ScrollRight - called from dispatch proc
 */
static pascal void ScrollRight(ControlHandle c, Integer p){
	if(inDownButton == p){
		ScrollDraw(1, 0);
	}
}

/* PageLeft - called from dispatch proc
 */
static pascal void PageLeft(ControlHandle c, Integer p){
	if(inPageUp == p){
		ScrollDraw(-PageWidth(), 0);
	}
}

/* PageRight - called from dispatch proc
 */
static pascal void PageRight(ControlHandle c, Integer p){
	if(inPageDown == p){
		ScrollDraw(PageWidth(), 0);
	}
}

/* HThumb - no special thumb tracking
 */
static pascal void HThumb(){
}

/* HDThumb called C style by ContentDoc
	horizontal done thumb
 */
static void HDThumb(ControlHandle c, Integer oldVal){
	if(oldVal != GetCtlValue(c)){
		DoScrollDraw(GetCtlValue(c) - oldVal, 0);
	}
}

/* InitHThumb - set up for HDThumb
 */
static void InitHThumb(ControlHandle c){
}

/* DoScrollDraw - actually scrolls the window.
	doesn't set the controls. Use this when the user directly sets the control thumb.

	If we are going to scroll the window, we must empty the update event queue, since
	the update region will be bogus after the scroll.
 */
static void DoScrollDraw(Integer x, Integer y){
	RgnHandle	saveClip, scrollUpdate;
	Rect		r;

	GetContentsRect(&r);
	saveClip = NewRgn();
	GetClip(saveClip);
	scrollUpdate = NewRgn();
	ScrollRect(&r, -x, -y, scrollUpdate);
	SetClip(scrollUpdate);
	RandomDotUpdate();
	DisposeRgn(scrollUpdate);
	SetClip(saveClip);
	DisposeRgn(saveClip);
}

/* TrimScroll - adjust number to preserve min < val < max relationship
 */
static Integer  TrimScroll(LongInt delta, ControlHandle c){
	Integer newVal, oldVal, maxVal;

	oldVal = GetCtlValue(c);
	maxVal = GetCtlMax(c);
	newVal = oldVal + delta;
	if(newVal > maxVal)
		return maxVal - oldVal;
	if(newVal < 0)
		return  -oldVal;
	return delta;
}

/* ScrollDraw - scroll the window and change the controls.
 */
static void ScrollDraw(Integer x, Integer y){
	ControlHandle		theH;
	ControlHandle		theV;
	
	theH = ((RandomDotWindowPtr) qd.thePort)->hScroll;
	x = TrimScroll(x, theH);

	theV = ((RandomDotWindowPtr) qd.thePort)->vScroll;
	y = TrimScroll(y, theV);

	if(x == 0 && y == 0){
		return;
	}
	if(x != 0){
		SetCtlValue(theH, GetCtlValue(theH) + x);
	}
	if(y != 0){
		SetCtlValue(theV, GetCtlValue(theV) + y);
	}
	DoScrollDraw(x, y);
}
