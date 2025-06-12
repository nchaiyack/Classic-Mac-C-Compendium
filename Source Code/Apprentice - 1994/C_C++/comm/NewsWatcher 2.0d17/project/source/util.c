/*----------------------------------------------------------------------------

	util.c

	This module contains miscellaneous utility routines.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>
#include <Traps.h>

#include "glob.h"
#include "util.h"
#include "dlgutil.h"
#include "mouse.h"
#include "activate.h"
#include "draw.h"
#include "menus.h"



static CStr255 gStatusMsg;
static OSErr gMemError = noErr;



/*	IsEqualFSSpec compares two canonical FSSpec records for equality.
*/

Boolean IsEqualFSSpec (FSSpec *file1, FSSpec *file2)
{
	return
		file1->vRefNum == file2->vRefNum &&
		file1->parID == file2->parID &&
		EqualString(file1->name, file2->name, false, true);
}



/*	GiveTime is called whenever the application is waiting for a slow
	process to complete.  The routine calls SpinCursor and WaitNextEvent
	to give time to currently running background applications.
*/

Boolean GiveTime (void)
{
	EventRecord ev;
	Boolean gotEvt;
	static long nextTime = 0;
	char keyPressed, charPressed;
	WindowPtr statusWind;
	GrafPtr savePort;
	ControlHandle cancel;
	long myticks;
	short part;
	WindowPtr theWindow;

	HiliteMenu(0);
	SetMenusTo(false, 0, 0, 0, 0, 0, 0);
	ShowCursor();

	if (TickCount() >= nextTime) {
			
		SpinCursor(16);

		gotEvt = WaitNextEvent(everyEvent,&ev,0,nil);
		
		if ( gotEvt )
			switch (ev.what) {
				case mouseDown:
					part = FindWindow(ev.where, &theWindow);
					if (part == inMenuBar) {
						MenuSelect(ev.where);
					} else if (IsStatusWindow(FrontWindow())) {
						HandleMouseDown(&ev);
					}
					break;
				case activateEvt:
					HandleActivate((WindowPtr)ev.message,
						((ev.modifiers & activeFlag) != 0)); 
					break;
				case updateEvt:
					HandleUpdate((WindowPtr)(ev.message));
					break;
				case app4Evt:
					HandleSuspendResume(ev.message);
					break;
				case keyDown:
				case autoKey:
					FlushEvents(keyDownMask+keyUpMask+autoKeyMask,0);
					charPressed = ev.message & charCodeMask;
					keyPressed = (ev.message & keyCodeMask) >> 8;
					if (keyPressed == escapeKeyCode || 
						(ev.modifiers & cmdKey) != 0 && charPressed == '.') {
						statusWind = FrontWindow();
						if (IsStatusWindow(statusWind)) {
							cancel = ((WindowPeek)statusWind)->controlList;
							GetPort(&savePort);
							SetPort(statusWind);
							HiliteControl(cancel,1);
							Delay(8,&myticks);
							HiliteControl(cancel,0);
							SetPort(savePort);
						}
						gCancel = true;
					}
					break;
			}
		nextTime = TickCount() + 5;
	}

	return !gCancel;
}


/*	StatusWindow displays a movable-modal status window indicating
	the current state of the program.
*/

Boolean StatusWindow (char *text)
{
	WindowPtr statusWind;
	Rect bounds = {0,0,75,420};
	Rect buttonBounds = {42,347,62,407};
	Rect msgBounds = {13, 13, 40, 420};
	TWindow **info;
	
	if (!IsStatusWindow(statusWind = FrontWindow())) {
		if (PtInRect(gPrefs.statusWindowLocn,&gDesktopExtent))
			OffsetRect(&bounds,
				gPrefs.statusWindowLocn.h,
				gPrefs.statusWindowLocn.v);
		else
			OffsetRect(&bounds,100,100);
		info = (TWindow**) MyNewHandle(sizeof(TWindow));
		if (MyMemErr() != noErr)
			return false;
		strcpy(gStatusMsg,text);
		(**info).kind = kStatus;
		statusWind = NewWindow(nil, &bounds, "\pStatus", true,
			movableDBoxProc, (WindowPtr)-1,
			false, (unsigned long)info);
		SetPort(statusWind);
		NewControl(statusWind, &buttonBounds, "\pCancel", true, 0, 0, 0, 
			pushButProc, 0);
		ValidRect(&statusWind->portRect);
		SpinCursor(0);
	} else {
		SetPort(statusWind);
		strcpy(gStatusMsg,text);
		EraseRect(&msgBounds);
	}
	TextFont(systemFont);
	TextSize(12);
	MoveTo(13,29);
	DrawText(gStatusMsg,0,strlen(gStatusMsg));
	return true;
}


/*	UpdateStatus is called in response to update events for the status
	window.  This routine will redraw sections of the window as necessary.
*/

void UpdateStatus (void)
{
	GrafPtr savePort;
	WindowPtr statusWind;
	
	statusWind = FrontWindow();
	if (!IsStatusWindow(statusWind)) return;
	GetPort(&savePort);
	SetPort(statusWind);
	EraseRect(&statusWind->portRect);
	DrawControls(statusWind);
	TextFont(systemFont);
	TextSize(12);
	MoveTo(13,29);
	DrawText(gStatusMsg,0,strlen(gStatusMsg));
	SetPort(savePort);
}


/*	CloseStatusWindow is called when the status window should be removed.
*/

void CloseStatusWindow (void)
{
	WindowPtr statusWind;
	GrafPtr savePort;
	TWindow **info;
	
	if (IsStatusWindow(statusWind = FrontWindow())) {
		SetPt(&gPrefs.statusWindowLocn,0,0);
		GetPort(&savePort);
		SetPort(statusWind);
		LocalToGlobal(&gPrefs.statusWindowLocn);
		info = (TWindow**)GetWRefCon(statusWind);
		SetPort(savePort);
		MyDisposHandle((Handle)info );
		DisposeWindow(statusWind);
	}
}

/*	MyIOCheck can be called to display the result of a routine returning
	an OSErr.  If the value in err is zero, the routine simply terminates.
*/

OSErr MyIOCheck (OSErr err)
{
	if (err != noErr) {
		UnexpectedErrorMessage(err);
	}
	return err;
}


/*	LowMemory is called when the program runs out of useable memory.
	If this is the first time this has happened, the program de-allocates
	lifeboat memory which was allocated when the program was launched.
	Otherwise, the user had better quit.
*/

static Boolean LowMemory (void)
{
	Boolean result;
	
	if (MyMemErr() != memFullErr) {
		MyIOCheck(MyMemErr());
		return false;
	}
		
	if (gSinking) {
		result = false;
		gOutOfMemory = true;
		ErrorMessage("You have run out of memory");
	}
	else {
		HUnlock(gLifeBoat);
		DisposHandle(gLifeBoat);
		gSinking = true;
		result = true;
		ErrorMessage("Memory is getting low.  Some operations may fail.");
	}
	return result;
}


/*	This is a wrapper for the NewPtr routine which automatically checks
	the result of the call and takes appropriate action.
*/

Ptr MyNewPtr (Size byteCount)
{
	Ptr thePtr;
	
	thePtr = NewPtrClear(byteCount);
	if ((gMemError = MemError()) != noErr) {
		if (LowMemory())
			thePtr = MyNewPtr(byteCount);
		else
			thePtr = nil;
	}
	return thePtr;
}


/*	This is a wrapper for the NewHandle routine which automatically checks
	the result of the call and takes appropriate action.
*/

Handle MyNewHandle (Size byteCount)
{
	Handle theHndl;
	
	theHndl = NewHandleClear(byteCount);
	if ((gMemError = MemError()) != noErr) {
		if (LowMemory())
			theHndl = MyNewHandle(byteCount);
		else
			theHndl = nil;
	}
	return theHndl;
}

/*	This is a wrapper for the SetHandleSize routine which automatically checks
	the result of the call and takes appropriate action.
*/

void MySetHandleSize (Handle h, Size newSize)
{
	long oldSize;

	oldSize = GetHandleSize(h);
	SetHandleSize(h,newSize);
	if ((gMemError = MemError()) != noErr) {
		if (LowMemory())
			MySetHandleSize(h,newSize);
	} else if (oldSize < newSize) {
		memset(*h+oldSize, 0, newSize-oldSize);
	}
}


/*	This is a wrapper for the HandToHand routine which automatically checks
	the result of the call and takes appropriate action.
*/

OSErr MyHandToHand (Handle *theHndl)
{
	Handle oldHndl;
	OSErr result;
	
	oldHndl = *theHndl;
	result = gMemError = HandToHand(theHndl);
	if (result != noErr) {
		*theHndl = oldHndl;
		if (LowMemory())
			result = MyHandToHand(theHndl);
	}
	return result;
}


/*	This is a wrapper for the DisposPtr routine which automatically checks
	the result of the call and takes appropriate action.
	NT: extra errorchecking for stability incorporated
*/

OSErr MyDisposPtr (Ptr thePtr)
{
	if(thePtr != nil)
	{
		DisposPtr(thePtr);
		gMemError = MemError();
	}
	else
	{
		gMemError = noErr;
	}
	return MyIOCheck(gMemError);
}


/*	This is a wrapper for the DisposHandle routine which automatically checks
	the result of the call and takes appropriate action.
	NT: extra errorchecking for stability incorporated
*/

OSErr MyDisposHandle (Handle theHndl)
{
	if(theHndl != nil)
	{
		DisposHandle(theHndl);
		gMemError = MemError();
	}
	else
	{
		gMemError = noErr;
	}
	return MyIOCheck(gMemError);
}


/*	This is a wrapper for the MemError routine which automatically checks
	the result of the call and takes appropriate action.
*/

OSErr MyMemErr (void)
{
	return gMemError;
}


/*	IsAppWindow returns true if the window belongs to the application
*/

Boolean IsAppWindow (WindowPtr wind)
{
	short		windowKind;
	
	if (wind == nil)
		return false;
	else {
		windowKind = ((WindowPeek)wind)->windowKind;
		return windowKind >= userKind;
	}
}


/*	IsDAWindow returns true if the window is a DA window
*/

Boolean IsDAWindow (WindowPtr wind)
{
	if (wind == nil)
		return false;
	else	/* DA windows have negative windowKinds */
		return ((WindowPeek) wind)->windowKind < 0;
}


/*	IsStatusWindow returns true if the window is the status window.
*/

Boolean IsStatusWindow (WindowPtr wind)
{
	TWindow **info;

	if (!IsAppWindow(wind)) return false;
	info = (TWindow**)GetWRefCon(wind);
	return (**info).kind==kStatus;
}


/*	pstrcpy copies Pascal format strings.
*/

void pstrcpy (StringPtr to, StringPtr from)
{
	BlockMove(from, to, *from+1);
}


/*	GetPixelDepth gets the pixel depth of the monitor with the 
	maximum intersection with a given rectangle.
	
	Entry:	*r = rectangle in local coord system of current port.
	
	Exit:	function result = pixel depth.
*/

short GetPixelDepth (Rect *r)
{
	Rect tempRect;
	GDHandle theDev;
	PixMapHandle theMap;
	
	if (!gHasColorQD) return 1;
	tempRect = *r;
	LocalToGlobal((Point*)&tempRect.top);
	LocalToGlobal((Point*)&tempRect.bottom);
	theDev = GetMaxDevice(&tempRect);
	if (theDev == nil) return 1;
	theMap = (**theDev).gdPMap;
	if (theMap == nil) return 1;
	return (**theMap).pixelSize;
}

/*	VolNameToVRefNum returns a volume reference number given a volume name. */

OSErr VolNameToVRefNum (StringPtr name, short *vRefNum)
{
	HParamBlockRec pb;
	Str31 volNameWithColon;
	short len;
	OSErr err;
	
	pstrcpy(volNameWithColon, name);
	len = *volNameWithColon;
	if (volNameWithColon[len] != ':') {
		len = ++(*volNameWithColon);
		volNameWithColon[len] = ':';
	}
	pb.volumeParam.ioNamePtr = volNameWithColon;
	pb.volumeParam.ioVolIndex = -1;
	err = PBHGetVInfo(&pb, false);
	*vRefNum = pb.volumeParam.ioVRefNum;
	return err;
}



/*	GetFontNumber - Get font number corresponding to font name. 
	Returns false if font does not exist. (From Think Ref) */


Boolean GetFontNumber (Str255 fontName, short *fontNum)
{
	Str255 systemFontName;

	GetFNum(fontName, fontNum);
	if (*fontNum == 0) {
		/* Either we didn't find the font, or we were looking for the system
		  * font. */
		GetFontName(0, systemFontName);
		return EqualString(fontName, systemFontName, false, false);
	} else {
		return true;
	}
}
		


/*	The InitCursorCtl, SpinCursor, strcasecmp and strncasecmp functions 
	are missing in Think, so we include them here	*/

Handle gAcur = nil;

pascal void InitCursorCtl (Handle id)
{
	short NoFrames, i, CursId;
	CursHandle	TheCursHndl;

	if (id == nil) {
		gAcur = (Handle) GetResource('acur',0);
		if(gAcur == nil) return;
		HLock(gAcur);
		NoFrames = ((short *)(*gAcur))[0];
		for(i = 0; i < NoFrames; i++) {
			CursId = ((short *)(*gAcur))[(2*i)+2];
			TheCursHndl = GetCursor(CursId);
			((CursHandle *)(*gAcur))[i+1] = TheCursHndl;
			HLock((Handle)TheCursHndl);
		}
	} else {
		gAcur = id;
		HLock(gAcur);
		NoFrames = ((short *)(*gAcur))[0];
		for(i = 0; i < NoFrames; i++) {
			CursId = ((short *)(*gAcur))[(2*i)+2];
			TheCursHndl = GetCursor(CursId);
			((CursHandle *)(*gAcur))[i+1] = TheCursHndl;
			HLock((Handle)TheCursHndl);
		}
	}
	((short *)(*gAcur))[1] = 0;
	DetachResource(gAcur);
}


pascal void SpinCursor (short num)
{
	short	NoFrames, CurrentFrame, CurrentCounter;
	Cursor	CurrentCursor;

	if (gAcur == nil) InitCursorCtl(nil);
	NoFrames = ((short *)(*gAcur))[0];
	CurrentCounter = ((((short *)(*gAcur))[1] + num) % (NoFrames * 32));
	((short *)(*gAcur))[1] = CurrentCounter;
	CurrentFrame = CurrentCounter / 32;
	CurrentCursor = **(((CursHandle *)(*gAcur))[CurrentFrame+1]);
	SetCursor(&CurrentCursor);
}


/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific written prior permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#ifndef u_char
	#define u_char  unsigned char
#endif

/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison.  The mappings are
 * based upon ascii character sequences.
 */
static u_char charmap[] = {
	'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
	'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
	'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
	'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
	'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
	'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
	'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
	'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
	'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
	'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
	'\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
	'\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
	'\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
	'\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
	'\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
	'\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
	'\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
	'\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
	'\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
	'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};


short strcasecmp (const char *s1, const char *s2)
{
	u_char			*cm = charmap;
	const u_char	*us1 = (const u_char *)s1;
	const u_char	*us2 = (const u_char *)s2;

	while (cm[*us1] == cm[*us2++])
		if (*us1++ == 0)
			return(0);
	return(cm[*us1] - cm[*--us2]);
}


short strncasecmp (const char *s1, const char *s2, short n)
{
	u_char			*cm = charmap;
	const u_char	*us1 = (const u_char *)s1;
	const u_char	*us2 = (const u_char *)s2;

	while (--n >= 0 && cm[*us1] == cm[*us2++])
		if (*us1++ == 0)
			return(0);
	return(n < 0 ? 0 : cm[*us1] - cm[*--us2]);
}

/*------------------------------------------------------------------------
 * SetPortTextStyle
 * Set the text font, size, style, etc. of the current port based
 * on a TextStyle record.
 * NOTE: color field is ignored for now.
 */
void SetPortTextStyle(const TextStyle *style)
{
	TextFont(style->tsFont);
	TextFace(style->tsFace);
	TextSize(style->tsSize);
}

/*------------------------------------------------------------------------
 * GetPortTextStyle
 * Get the text font, size, style, etc. of the current port.
 * NOTE: color field is ignored for now.
 */
void GetPortTextStyle(TextStyle *style)
{
	style->tsFont = qd.thePort->txFont;
	style->tsFace = qd.thePort->txFont;
	style->tsSize = qd.thePort->txFont;
}

