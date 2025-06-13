/* Utils.c - 
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
#include "RandomDotMain.h"
#include "Utils.h"

/* *** arithmetic functions.
 */
Integer Min(Integer a, Integer b){ return a < b ? a : b; }

Integer Max(Integer a, Integer b){ return a > b ? a : b; }

/* *** memory functions
 */

/* HSetLockState - lock a handle, and return the previous state
 */
SignedByte HSetLockState(Handle h){
	SignedByte	state;

	state = HGetState(h);
	HLock(h);
	return state;
}

/* *** string functions.
 */

/* StrMove - 
 */
void StrMove(const StringPtr src, StringPtr dest){
	BlockMove(src, dest, Length(src) + 1);
}

/* StrHandle - copy a string into the heap as a handle, and return it.
 */
StringHandle StrHandle(const StringPtr s){
	StringHandle	h;

	h = NIL;
	PtrToHand(s, (Handle *) &h, Length(s) + 1);
	return h;
}


/* AppendChar - safe add char to string
 */
void AppendChar(StringPtr s, char c){
	if(Length(s) < 254){
		s[0] = 1 + Length(s);
		s[Length(s)] = c;
	}
}

/* Concat - stick the tail on the string
 */
void Concat(StringPtr s, StringPtr tail){
	Integer newLen;

	newLen = Min(255, Length(s) + Length(tail));
	BlockMove((Ptr) &tail[1] ,(Ptr) &s[1+Length(s)], newLen - Length(s));
	s[0] = newLen;
}

/* BlockClear - clear a block of memory
 */
void BlockClear(void *xp, LongInt len){
	char *p;

	p = (char *) xp;
	while(len-- > 0){
		*p++ = 0;
	}
}

/* EqualBlock - true if two blocks of mem are eqivalent
 */
Boolean EqualBlock(void * ax, void *bx, LongInt len){
	char *a, *b;

	a = (char *) ax;
	b = (char *) bx;
	for( ; len > 0; len--, a++, b++){
		if(*a != *b){
			return FALSE;
		}
	}
	return TRUE;
}

/* EqualHandle - true if two handles are equivalent
 */
Boolean EqualHandle(Handle a, Handle b){
	LongInt	len;

	len = GetHandleSize(b);
	if(GetHandleSize(a) != len){
		return FALSE;
	}
	return EqualBlock(*a, *b, len);
}


/* NullUpdate - the empty update routine.
 */
pascal void NullUpdate(DialogPtr dp, Integer i){
}


/* SetWDIHandle - 
 */
void SetWDIHandle(WindowPtr win, Integer item, Handle h){
	Integer theType;
	Handle theHandle;
	Rect theRect;

	GetDItem(win, item, &theType, &theHandle, &theRect);
	SetDItem(win, item, theType, h, &theRect);
}

/* SetDIHandle - 
 */
void SetDIHandle(Integer item, Handle h){
	SetWDIHandle(qd.thePort, item, h);
}

/* GetWDIHandle - 
 */
Handle GetWDIHandle(WindowPtr win, Integer item){
	Integer theType;
	Handle theHandle;
	Rect theRect;

	GetDItem(win, item, &theType, &theHandle, &theRect);
	return theHandle;
}

/* GetDIHandle - 
 */
Handle GetDIHandle(Integer item){
	Integer theType;
	Handle theHandle;
	Rect theRect;

	GetDItem(qd.thePort, item, &theType, &theHandle, &theRect);
	return theHandle;
}

/* GetCIHandle - 
 */
ControlHandle GetCIHandle(Integer item){
	Integer theType;
	Handle theHandle;
	Rect theRect;

	GetDItem(qd.thePort, item, &theType, &theHandle, &theRect);
	return (ControlHandle) theHandle;
}

/* GetDIRect - 
 */
void GetDIRect(Integer item, Rect *rp){
	Integer theType;
	Handle theHandle;

	GetDItem(qd.thePort, item, &theType, &theHandle, rp);	
}

/* IsCancel - return true if Control-. or Escape
 */
Boolean IsCancel(EventRecord *theEvent){
	if(NOT (theEvent->what == autoKey || theEvent->what == keyDown)){
		return FALSE;
	}
	return	(theEvent->modifiers & cmdKey) ?
			((theEvent->message & charCodeMask) == '.') :
			((theEvent->message & charCodeMask) == kEscapeChar);
}

/* IsReturnEnter - true is c == either return or enter
 */
Boolean IsReturnEnter(char c){
	return (c == kReturnChar) || (c == kEnterChar);
}



/* MirrorIt - show the button changing state
 */
void MirrorIt(Integer itemHit){
	LongInt dontcare;
 
	HiliteControl((ControlHandle) GetDIHandle(itemHit), inButton);
	Delay(10, &dontcare);
	HiliteControl((ControlHandle) GetDIHandle(itemHit), kEnable);
}

/* *** window functions.
 */

/* GetContentsRect - return the contents area of the window.
 */
void GetContentsRect(Rect *rp){
	*rp = qd.thePort->portRect;
	rp->bottom -= kScrollBarWidth;
	rp->right -= kScrollBarWidth;
}


/* RestrictClipRect - set clip to intersection of old and new, and return old.
 */
RgnHandle RestrictClipRect(const Rect *r){
	RgnHandle oldClip, smallClip;

	oldClip = NewRgn();
	smallClip = NewRgn();
	GetClip(oldClip);
	RectRgn(smallClip, r);
	SectRgn(qd.thePort->clipRgn, smallClip, smallClip);
	SetClip(smallClip);
	DisposeRgn(smallClip);
	return oldClip;
}

/* RestoreClip - match a previous RestrictClipRect, and save the clip.
 */
void RestoreClip(RgnHandle rgn){
	SetClip(rgn);
	DisposeRgn(rgn);
}

/* InitScrollBarClass - if this is the first time, then initialize the UPP fields.
 */
void InitScrollBarClass(ScrollBarClassPtr p){
	if(NIL == p->up && NIL != p->upProc){
		p->up = NewControlActionProc(p->upProc);
	}
	if(NIL == p->down && NIL != p->downProc){
		p->down = NewControlActionProc(p->downProc);
	}
	if(NIL == p->pageUp && NIL != p->pageUpProc){
		p->pageUp = NewControlActionProc(p->pageUpProc);
	}
	if(NIL == p->pageDown && NIL != p->pageDownProc){
		p->pageDown = NewControlActionProc(p->pageDownProc);
	}
	if(NIL == p->thumb && NIL != p->thumbProc){
		p->thumb = NewControlIndicatorProc(p->thumbProc);
	}
}

/* ControlProc - return the appropriate Pascal procedure
	if mouse leaves part, do nothing.
 */

static ControlActionUPP ControlProc(ControlHandle theControl, Integer part){
	ScrollBarClassPtr p;

	if(NIL == (p = (ScrollBarClassPtr) GetCRefCon(theControl))){
		return NIL;
	}
	switch(part){
		case inUpButton:	return p->up;
		case inDownButton:	return p->down;
		case inPageUp:		return p->pageUp;
		case inPageDown:	return p->pageDown;
		case inThumb:		return (ControlActionUPP) p->thumb;
	}
	return NIL;
}

/* TrackScroll - track a scroll bar. takes a pointer to a vector of
 * procedures in the refcon of the scroll bar, and calls the appropriate
 * one repeatedly, as appropriate.
 */
Boolean TrackScroll(Point where){
	ControlHandle theControl;
	Integer part, oldVal;
	ScrollBarClassPtr p;

	if(0 != (part = FindControl(where, qd.thePort, &theControl)) ){
		if(inThumb == part){
			oldVal = GetCtlValue(theControl);
			p = (ScrollBarClassPtr) GetCRefCon(theControl);
			(*p->thumbInit)(theControl);
		}
		if(inThumb == TrackControl(theControl, where, ControlProc(theControl, part))){
			p = (ScrollBarClassPtr) GetCRefCon(theControl);
			(* p->thumbDone)(theControl, oldVal);
		}
		return TRUE;
	}
	return FALSE;
}

/* TitleBarOnScreen - return TRUE if the title bar that has rp for an
	area has some pixels that are visible.
 */
Boolean TitleBarOnScreen(const Rect *rp){
	Rect		r;
	RgnHandle	rgn;
	Boolean		val;

	r.left = rp->left;
	r.top = r.bottom - 20;
	r.right = rp->right;
	r.bottom = rp->top;
	InsetRect(&r, 4, 4);
	rgn = NewRgn();
	RectRgn(rgn, &r);
	SectRgn(rgn, LMGetGrayRgn(), rgn);
	val = NOT EmptyRgn(rgn);
	DisposeRgn(rgn);
	return val;
}


/* *** File functions
 */

/* SavePreferencesResource - copy a handle into the preferences file.
	(argument is unchanged)
 */
void SavePreferencesResource(Handle h, OSType type, Integer id){
	Integer	saveRes;
	Handle	saveH;
	OSErr	errCode;

	if(-1 != prefResFile){
		saveRes = CurResFile();
		UseResFile(prefResFile);
		if(NIL == (saveH = Get1Resource(type, id))){
			saveH = NewHandle(GetHandleSize(h));
			AddResource(saveH, type, id, "\p");
		}
		if(NOT EqualHandle(h, saveH)){
			HNoPurge(saveH);
			SetHandleSize(saveH, GetHandleSize(h));
			if(noErr == (errCode = MemError())){ BlockMove(*h, *saveH, Min(GetHandleSize(h), GetHandleSize(saveH))); }
			if(noErr == errCode){ ChangedResource(saveH); errCode = ResError(); }
			if(noErr == errCode){ WriteResource(saveH); errCode = ResError(); }
			HPurge(saveH);
		}
		UseResFile(saveRes);
	}
}

/* GetPreferencesHandle - get a handle from the prefrences file.
	returns NIL on not found. Note: this is a handle, not a resource.
	the caller must Dispose it.
 */
Handle GetPreferencesHandle(OSType type, Integer id){
	Integer	saveRes;
	Handle	h;

	if(-1 == prefResFile){
		return NIL;
	}
	saveRes = CurResFile();
	UseResFile(prefResFile);
	if(NIL != (h = Get1Resource(type, id))){
		DetachResource(h);
		HNoPurge(h);
	}
	UseResFile(saveRes);
	return h;
}

/* EqualFSSpec - return TRUE if two file specs are equivalent. 
 */
Boolean EqualFSSpec(FSSpecPtr a, FSSpecPtr b){
	return a->vRefNum == b->vRefNum && a->parID == b->parID && 
		EqualString(a->name, b->name, FALSE, TRUE);
}

