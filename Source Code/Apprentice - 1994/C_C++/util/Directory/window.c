//-- Window.c --//

// Generic window management routines.

// The window management routines to create and distroy drawing windows and their data
// structures, to redraw windows, and to append lines to the window line buffer, as well
// to managing scrolling and such.  This is the total of all the routines to manage windows.
// This also contains the code to manage the project window as such. */

#include <stdio.h>
#include "struct.h"
#include "res.h"
#include "error.h"

//-- GLOBALS --//
 
//-- The globals used for the window structure.

struct DrawWindow *drawList;
static struct DrawWindow *whichScroll;	// What window is being scrolled?
Rect openSize;
extern unsigned char MultiWidget;		// State flags.

//-- CODE --//

//-- The different routines
 
//-- Basic allocation/deallocation of windows.

//-- AllocateWindow --//

//-- This allocs a new window.  
//-- WARNING:  Errors are returned using the 'Throw' mechanism.

struct DrawWindow *AllocateWindow()
{
	int i;
	
	for (i = 0; i < MAXWINDOWS; i++) if (drawList[i].inuse == 0) break;
	if (i == MAXWINDOWS) Throw(OUTWINDOWS);

	drawList[i].inuse = 1;
	openSize.top = 38 + i * 20;
	openSize.left = 30 - i * 3;
	openSize.right = -i * 3;
	openSize.bottom = - 10;
	if (MultiWidget) openSize.right -= 75;
	return &(drawList[i]);
}


//-- FreeWind

// This frees a window.  Only do this AFTER disposing of the window.

FreeWind(w)
struct DrawWindow *w;
{
	w->inuse = 0;
}

//-- Basic window math. --//

//-- CalcScroll --//

// Given a window, this computes where the scroll bars are to go..

CalcScroll(w,x,y)
WindowPtr w;
Rect *x,*y;
{
	Rect r;
	
	r = w->portRect;
	x->top = r.bottom - 15;
	x->bottom = r.bottom + 1;
	x->right = r.right - 14;
	x->left = -1;
	
	y->top = -1;
	y->bottom = r.bottom - 14;
	y->left = r.right - 15;
	y->right = r.right + 1;
}


//-- CalcClip --//

// Calculate the clip area of the window (where to draw, which doesn't include scroll
// bars.

CalcClip(w,c)
WindowPtr w;
Rect *c;
{
	Rect r;

	r = w->portRect;
	c->top = r.top;
	c->left = r.left;
	c->right = r.right - 15;
	c->bottom = r.bottom - 15;
}

//-- Window management. --//
 

//-- NewPlan --//

// Open a new window.

struct DrawWindow *NewPlan(name)
short name;								/* vRefNum of this object */
{
	int i;
	struct DrawWindow *w;
	short step;
	GrafPtr foo;
	Rect r;
	Rect s;
	int t;

// Step 1:  Prepare for disaster.
	
	step = 1;
	if (i = Catch()) {
	
// According to the step number, do the appropriate shutting down.

		switch (step) {
			case 2:
				CloseWindow(w);
				FreeWind(w);
			case 1:
				break;
		}
		
// Post the error message, and return.

		PostError(i);
		return NULL;
	}
	
// Step 2:  Allocate and open the window.

	w = AllocateWindow();
	GetWMgrPort(&foo);
	r = foo->portRect;
	r.right += openSize.right;
	r.bottom += openSize.bottom;
	r.top = openSize.top;
	r.left = openSize.left;
	InsetRect(&r,4,4);
	NewWindow(w,&r,"\pUntitled",1,8,(char *)-1,CanEject(name),0L);
	step = 2;
	
	/*
	 *	Step 3:  Initialize different data structures
	 */

	w->state = 0;						/* Directories only */
	w->w.windowKind = WK_PLAN;
	ComputePict(name,w);

	CalcScroll(w,&r,&s);
	if (NULL == (w->yScroll = NewControl(w,&s,"",1,0,0,0,16,0L))) 
		Throw(OUTMEM);
	
	t = (GetHandleSize(w->data) / sizeof(struct DirectData)) - 1;
	if (t < 0) t = 0;
	SetCtlMax(w->yScroll,t);

// All done.  Return.
	
	Uncatch();
	return w;
}




//-- ClosePlan --//

// What to do when the 'close' option is selected for a particular plan.


int ClosePlan(w)
struct DrawWindow *w;
{

// Dispose of the window and quit.

	if (!CanEject(w->vRefNum)) return 0;	/* Failure */
	Eject("",w->vRefNum);
	UnmountVol("",w->vRefNum);
	DisposHandle(w->data);
	CloseWindow(w);
	FreeWind(w);
	return 1;								/* Success */
}



//-- UpdatePlan --//

// This updates the plan (redraws the contents of the window).

UpdatePlan(w)
struct DrawWindow *w;
{
	DrawGrowIcon(w);
	DrawControls(w);
	DrawPlanWind(w);
}



//-- DrawPlanWind --//

// Actually draw the contents of this drawing window.

DrawPlanWind(w)
struct DrawWindow *w;
{
	int x,y;
	Rect r;
	RgnHandle rgn;
	Rect s;
	long l,i;
	struct DirectData *ptr;
	short ind;
	
//-- Properly initialize clipping for redrawing the screen

	SetPort(w);
	CalcClip(w,&r);
	rgn = NewRgn();
	GetClip(rgn);
	ClipRect(&r);
	
//-- Draw the contents of the screen

	HLock(w->data);
	ptr = *(w->data);
	TextFont(4);
	TextSize(9);
	y = GetCtlValue(w->yScroll);
	l = GetHandleSize(w->data) / sizeof(struct DirectData);
	for (i = y, x = 12; i < l; i++, x += 12) {
		for (ind = 0; ind < ptr[i].indent; ind++) {
			MoveTo(ind*12+15,x-10);
			Line(0,12);
		}
		MoveTo(ptr[i].indent*12+10,x);
		DrawString(ptr[i].data);
		if (ptr[i].auxdata[0] != '\0') {
			MoveTo(ptr[i].indent*12+15+StringWidth(ptr[i].data),x);
			LineTo(295,x);
			MoveTo(360 - StringWidth(ptr[i].auxdata),x);
			DrawString(ptr[i].auxdata);
			
			MoveTo(380,x);
			DrawString(ptr[i].auxdata2);
		}
		if (x > r.bottom) break;
	}
	HUnlock(w->data);
		
// Reset the clipper properly.

	SetClip(rgn);
	DisposeRgn(rgn);
}



//-- ActivatePlan --//

// What to do when to activate/deactivate windows.

ActivatePlan(w,i)
struct DrawWindow *w;
int i;
{
	SetPort(w);
	DrawGrowIcon(w);

	if (!i) {
		HideControl(w->yScroll);
	} else {
		ShowControl(w->yScroll);
	}
}


//-- ResizeInitPlan --//

// Do resizing initialization for the plans.  This is so that the object in the
// centre of the screen remains in the centre.

ResizeInitPlan(w)
struct DrawWindow *w;
{

// Handle the scroll bars (they should be invisible during movement).

	HideControl(w->yScroll);
}


//-- ResizePlan --//

// What to do when the window is resized.

ResizePlan(w)
struct DrawWindow *w;
{
	Rect x,y;

	SetPort(w);
	
// Resize the scroll bars for display again.

	CalcScroll(w,&x,&y);
	MoveControl(w->yScroll,y.left,y.top);
	SizeControl(w->yScroll,y.right - y.left,y.bottom - y.top);
	ShowControl(w->yScroll);

// Prevent the scroll bars from flickering.

	ValidRect(&x);
	ValidRect(&y);
}

//-- ScrollPlan --//

// How to scroll the plan window.

ScrollPlan(w,yo,yn)
struct DrawWindow *w;
int yo,yn;
{
	Rect r;
	RgnHandle update;
	
	update = NewRgn();
	SetPort(w);
	CalcClip(w,&r);
	ScrollRect(&r,0,12 * (yo - yn),update);
	DisposeRgn(update);
	DrawPlanWind(w);
}

//-- MyScrollPlan --//

// How scrolling is handled in the end.

pascal void MyScrollPlan(c,partCode)
ControlHandle c;
short partCode;
{
	short whichDir;
	short delta;
	Rect r;
	int oldval,newval;
	int pin;

	r = whichScroll->w.port.portRect;
	whichDir = 0;

	switch (partCode) {
		case inUpButton:
			delta = 1;
			break;
		case inPageUp:
			delta = (r.bottom - r.top - 5) / 12;
			break;
		case inDownButton:
			delta = -1;
			break;
		case inPageDown:
			delta = - (r.bottom - r.top - 5) / 12;
			break;
		default:
			delta = 0;
			break;
	}

	oldval = GetCtlValue(c);
	newval = oldval - delta;
	if (newval > (pin = GetCtlMax(c))) newval = pin;
	if (newval < (pin = GetCtlMin(c))) newval = pin;
	if (newval == oldval) return;

	
	SetCtlValue(c,newval);
	ScrollPlan(whichScroll,oldval,newval);
}



//-- MousePlan --//

// What to do when the mouse goes down in the window.

MousePlan(w,e,p)
struct DrawWindow *w;
EventRecord *e;
Point *p;
{
	int i;
	ControlHandle c;
	int oldval,newval;

	i = FindControl(*p,w,&c);
	if (i) {							/* down in a control:  scroll bar? */
		SetPort(w);
		if (i == inThumb) {
			oldval = GetCtlValue(c);
			TrackControl(c,*p,NULL);
			newval = GetCtlValue(c);
			ScrollPlan(w,oldval,newval);
		} else {
			whichScroll = w;
			TrackControl(c,*p,MyScrollPlan);
		}
	} else {
	}
}


//-- SetBtn --//

// Set button.

SetBtn(dlog,i,val)
DialogPtr dlog;
int i;
int val;
{
	short i1;
	Handle i2;
	Rect i3;

	GetDItem(dlog,i,&i1,&i2,&i3);
	SetCtlValue(i2,val);
}


//-- OptionDialog --//

// Set options.

OptionDialog()
{
	DialogPtr dlog;
	short i1;
	Handle i2;
	Rect i3;
	short x;
	short nv;
	short t;
	short index;
	struct DrawWindow *w;
	
	w = (struct DrawWindow *)FrontWindow();
	if (w == NULL) return;							/* not a window to do this to */
	
	nv = w->state;
	dlog = GetNewDialog(OPTIONDLOG,NULL,(char *)-1);
	SetPort(dlog);
	GetDItem(dlog,1,&i1,&i2,&i3);
	PenSize(3,3);
	InsetRect(&i3,-4,-4);
	FrameRoundRect(&i3,16,16);

	SetBtn(dlog,4+nv,1);
	for (;;) {
		ModalDialog(NULL,&x);
		switch (x) {
			case 1:
				DisposDialog(dlog);
				if (nv != w->state) {
					w->state = nv;

					SetPort(w);
					EraseRect(&(w->w.port.portRect));
					InvalRect(&(w->w.port.portRect));
					DisposHandle(w->data);
					ComputePict(w->vRefNum,w);
					t = (GetHandleSize(w->data) / sizeof(struct DirectData)) - 1;
					if (t < 0) t = 0;
					SetCtlMax(w->yScroll,t);
				}
				return;
			case 2:
				DisposDialog(dlog);
				return;
			case 4:
			case 5:
			case 6:
				SetBtn(dlog,4+nv,0);
				nv = x - 4;
				SetBtn(dlog,4+nv,1);
				break;
		}
	}
}
