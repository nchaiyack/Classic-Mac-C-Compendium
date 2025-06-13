/* Help.c 
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
#include "RandomDotMain.h"
#include "RandomDotRes.h"

#include "Help.h"
#include "Utils.h"

#define kHelpItem	(kOK+1)

static TEHandle			helpTE;
static ControlHandle	helpScroll;

static void InitVThumb(ControlHandle);
static pascal void ScrollDown(ControlHandle, Integer);
static pascal void ScrollUp(ControlHandle, Integer);
static pascal void PageDown(ControlHandle, Integer);
static pascal void PageUp(ControlHandle, Integer);
static void VDThumb(ControlHandle, Integer);
static pascal void VThumb(void);
static void ScrollDraw(Integer x, Integer y);
static void DoScrollDraw(Integer x, Integer y);
static Integer PageSize(void);

static ScrollBarClassRec vProcs = {
	ScrollUp,		/* inUpButton */
	ScrollDown,	/* inDownButton */
	PageUp,		/* inPageUp */
	PageDown,		/* inPageDn */
	VThumb,		/* inThumb */
	InitVThumb,	/* InitThumb */
	VDThumb		/* DThumb */
};	

void InitVThumb(ControlHandle ch){
}

pascal void ScrollDown(ControlHandle ch, Integer i){
	ScrollDraw(0, 1);
}

pascal void ScrollUp(ControlHandle ch, Integer i){
	ScrollDraw(0, -1);
}

pascal void PageDown(ControlHandle ch, Integer i){
	ScrollDraw(0, PageSize());
}

pascal void PageUp(ControlHandle ch, Integer i){
	ScrollDraw(0, -PageSize());
}

void VDThumb(ControlHandle ch, Integer oldVal){
	if(oldVal != GetCtlValue(ch)){
		DoScrollDraw(0, GetCtlValue(ch) - oldVal);
	}
}

pascal void VThumb(void){
}

static Integer PageSize(void){
	Rect r;

	GetDIRect(kHelpItem, &r);
	return ((r.bottom-r.top)*9)/((**helpTE).lineHeight*10);
}

/* TrimScroll - adjust number to preserve min < val < max relationship
 */
static Integer TrimScroll(Integer delta, ControlHandle c){
	Integer newVal;
	newVal = GetCtlValue(c) + delta;
	if(newVal > GetCtlMax(c))
		return GetCtlMax(c) - GetCtlValue(c);
	if(newVal < GetCtlMin(c))
		return GetCtlMin(c) - GetCtlValue(c);
	return delta;
}

/* ScrollDraw - scroll the window and change the controls.
 */
static void ScrollDraw(Integer x, Integer y){
	ControlHandle theH, theV;
	
	theH = NIL;
	theV = helpScroll;

	y = TrimScroll(y, theV);

	if(y == 0){
		return;
	}
	if(y != 0){
		SetCtlValue(theV, GetCtlValue(theV) + y);
	}
	DoScrollDraw(x, y);
}

/* DoScrollDraw - actually scrolls the window.
 * doesn't set the controls. Use this when the user directly sets the
 * control thumb.
 */
static void DoScrollDraw(Integer x, Integer y){
	TEScroll(0, -y*(**helpTE).lineHeight, helpTE);
}

static pascal Boolean HelpFilter(DialogPtr, EventRecord *, Integer *);

/* HelpMouse - 
 */
static Boolean HelpMouse(Point where){
	Rect r;

	GlobalToLocal(&where);
	GetDIRect(kHelpItem, &r);
	r.left = r.right - kScrollBarWidth;
	if(PtInRect(where, &r)){
		TrackScroll(where);
		return TRUE;
	}
	return FALSE;
}

/* HelpPageHome - 
 */
static Boolean HelpPageHome(void){
	ScrollDraw(0, -GetCtlValue(helpScroll));
	return FALSE;
}

/* HelpPageEnd - 
 */
static Boolean HelpPageEnd(void){
	ScrollDraw(0, GetCtlMax(helpScroll)-GetCtlValue(helpScroll));
	return FALSE;
}

/* HelpPageUp - 
 */
static Boolean HelpPageUp(void){
	PageUp(NIL, 0);
	return FALSE;
}

/* HelpPageDown - 
 */
static Boolean HelpPageDown(void){
	PageDown(NIL, 0);
	return FALSE;
}



/* HelpKey - 
 */
static Boolean HelpKey(EventRecord *event, Integer *item){
	Boolean val;

	val = FALSE;
	if(IsReturnEnter((char) event->message) ||
		IsCancel(event)){
		MirrorIt(*item = kOK);
		return TRUE;
	}
	switch((char) event->message){
	case kHomeChar:		val = HelpPageHome();	break;
	case kBackspaceChar:val = HelpPageEnd();	break;
	case kUpChar:
	case kPageUpChar:	val = HelpPageUp();		break;
	case kDownChar:
	case kReturnChar:	val = HelpPageDown();	break;
	}
	event->what = nullEvent;
	return val;
}

/* HelpFilter - 
 */
static pascal Boolean HelpFilter(DialogPtr dp,EventRecord *event,Integer *itemp){
	Rect r;
	GrafPtr savePort;
	Boolean val;
	static ModalFilterUPP	stdFilerProc = NIL;

	val = FALSE;
	GetPort(&savePort);
	SetPort(dp);
	switch(event->what){
	case updateEvt:
		if((LongInt) dp == event->message){
			GetDIRect(kHelpItem, &r);
			PenPat(&qd.ltGray);
			InsetRect(&r, -1, -1);
			FrameRect(&r);
			PenPat(&qd.black);
			r = (**qd.thePort->visRgn).rgnBBox;
			TEUpdate(&r, helpTE);
			if(NIL == stdFilerProc){
				GetStdFilterProc(&stdFilerProc);
			}
			CallModalFilterProc(stdFilerProc, dp, event, itemp);
		}
		break;
	case mouseDown:	
		if(HelpMouse(event->where)){
			event->what = nullEvent;
		}
		break;
	case autoKey:
	case keyDown:	val = HelpKey(event, itemp);	break;
	}
	SetPort(savePort);
	return val;
}

/* DoHelp - Dialogs.h
 */
void DoHelp(void){
	DialogPtr	dp;
	Integer		n, item;
	Handle		h;
	Rect		r;
	GrafPtr		savePort;
	static ModalFilterUPP helpFilter = NIL;

	GetPort(&savePort);
	dp = GetNewDialog(rHelp, NIL, (WindowPtr) -1L);
	SetPort(dp);
	SetDialogDefaultItem(dp, kOK);
	GetDItem(qd.thePort, kHelpItem, &item, &h, &r);
	if(NIL == (h = GetResource('TEXT', 129))){
		return;
	}
	DetachResource(h);
	r.right -= kScrollBarWidth;
	GetFNum("\pGeneva", &n);
	TextFont(n);
	TextSize(9);
	InsetRect(&r, 1, 1);
	helpTE = TENew(&r, &r);
	InsetRect(&r, -1, -1);
	DisposHandle( (**helpTE).hText);
	(**helpTE).hText = h;
	TECalText(helpTE);
	helpScroll = GetNewControl(128, qd.thePort);
	SetCtlMax(helpScroll,  (**helpTE).nLines - (r.bottom - r.top)/(**helpTE).lineHeight);
	r.left = r.right;
	r.right += kScrollBarWidth + 1;
	InitScrollBarClass(&vProcs);
	SetCRefCon(helpScroll, (LongInt) &vProcs);
	MoveControl(helpScroll, r.left-1, r.top);
	SizeControl(helpScroll, r.right-r.left, r.bottom-r.top);
	ShowWindow(dp);
	SelectWindow(dp);
	if(NIL == helpFilter){
		helpFilter = NewModalFilterProc(HelpFilter);
	}
	ModalDialog(helpFilter, &item);
	TEDispose(helpTE);
	DisposDialog(dp);
	SetPort(savePort);
}
