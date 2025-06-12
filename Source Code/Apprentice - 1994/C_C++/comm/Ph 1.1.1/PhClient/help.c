/*_____________________________________________________________________

  	help.c - Help Window Manager
____________________________________________________________________*/

/*_____________________________________________________________________

	Header Files.
_____________________________________________________________________*/

#pragma load "precompile"
#include "MacTCPCommonTypes.h"
#include "rez.h"
#include "utl.h"
#include "mtcp.h"
#include "serv.h"
#include "glob.h"
#include "help.h"
#include "oop.h"
#include "wstm.h"
#include "site.h"

#pragma segment help

/*_____________________________________________________________________

	Defines.
_____________________________________________________________________*/

#define sBarWidth			16			/* width of scroll bar */
#define sBarWidthM1		(sBarWidth-1)
#define textMargin		4			/* left and right margins for text */
#define minWindHeight	150		/* min window height */
/*_____________________________________________________________________

	Types.
_____________________________________________________________________*/

typedef struct HelpInfo {
	ControlHandle		scrollBar;			/* handle to scroll bar control */
	TEHandle				help;					/* handle to help terec */
	short					windNum;				/* window number */
	Str255				server;				/* help server */
} HelpInfo;

/*_____________________________________________________________________

	Global Variables.
_____________________________________________________________________*/

static Rect				SizeRect;				/* size rectangle */

static oop_Dispatch dispatch = {
								help_DoPeriodic,
								help_DoClick,
								help_DoKey,
								help_DoUpdate,
								help_DoActivate,
								help_DoDeactivate,
								help_DoGrow,
								help_DoZoom,
								help_DoClose,
								help_DoCommand
							};

/*_____________________________________________________________________

	GetHelpInfo - Get HelpInfo Handle from Window Pointer
	
	Entry:	w = pointer to window record
	
	Exit:		Function result = handle to HelpInfo record.
_____________________________________________________________________*/

static HelpInfo **GetHelpInfo (WindowPtr w)

{
	return (HelpInfo**)oop_GetWindInfo(w);
}

/*_____________________________________________________________________

	GetViewRect - Get View Rectangle.
	
	Entry:	w = pointer to window record.
	
	Exit:		*viewRect = view rectangle.
_____________________________________________________________________*/

static void GetViewRect (WindowPtr w, Rect *viewRect)

{
	*viewRect = w->portRect;
	viewRect->bottom -= sBarWidthM1;
	viewRect->right -= sBarWidthM1;
}

/*_____________________________________________________________________

	AdjustScrollMax - Adjust Scroll Bar Max Value.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

static void AdjustScrollMax (WindowPtr w)

{
	HelpInfo			**e;			/* handle to help info */
	TEHandle			help;			/* handle to help terec */
	Rect				viewRect;	/* view rect */
	short				vHeight;		/* height of view rect */
	short				tHeight;		/* height of reply text */
	short				max;			/* new max control value */
	
	e = GetHelpInfo(w);
	help = (**e).help;
	GetViewRect(w, &viewRect);
	vHeight = viewRect.bottom - viewRect.top;
	tHeight = (**help).destRect.bottom - (**help).destRect.top;
	max = tHeight - vHeight;
	if (max < 0) max = 0;
	SetCtlMax((**e).scrollBar, max);
}

/*_____________________________________________________________________

	Scroll - Scroll the Window.
	
	Entry:	w = pointer to window record.
				dv = number of pixels to scroll.
_____________________________________________________________________*/

static void Scroll (WindowPtr w, short dv)

{
	HelpInfo		**e;			/* handle to help info */
	
	e = GetHelpInfo(w);
	SetPort(w);
	TEScroll(0, dv, (**e).help);
	SetOrigin(0, 0);
}

/*_____________________________________________________________________

	ChangeSize - Process Window Size Change.
	
	Entry:	w = pointer to window record.
				height = new window height
_____________________________________________________________________*/

static void ChangeSize (WindowPtr w, short height)

{
	HelpInfo		**e;					/* handle to help info */
	Rect			viewRect;			/* view rectangle */
	TEHandle		help;					/* handle to help terec */
	short			offTop;				/* amount of text scrolled off top */
	short			textBot;				/* bottom text coord */
	short			viewBot;				/* bottom view coord */
	short			dv;					/* amount to scroll */

	e = GetHelpInfo(w);
	SizeControl((**e).scrollBar, sBarWidth, height-13);
	AdjustScrollMax(w);
	GetViewRect(w, &viewRect);
	help = (**e).help;
	InsetRect(&viewRect, textMargin, 0);
	(**help).viewRect = viewRect;
	offTop = -(**help).destRect.top;
	textBot = (**help).destRect.bottom;
	viewBot = w->portRect.bottom - sBarWidthM1;
	if (textBot < viewBot && offTop) {
		dv = viewBot - textBot;
		if (dv > offTop) dv = offTop;
		Scroll(w, dv);
	}
}

/*_____________________________________________________________________

	ScrollAction - Scroll Bar Action Procedure
_____________________________________________________________________*/

static pascal void ScrollAction (ControlHandle scrollBar, short partCode)

{
	WindowPtr		w;					/* pointer to window */
	HelpInfo			**e;				/* handle to help info */
	short				val;				/* scroll bar value */
	short				min;				/* scroll bar minimum value */
	short				max;				/* scroll bar maximum value */
	short				viewHeight;		/* height of view rectangle */
	short				lineHeight;		/* height of one line */
	short				dv;				/* amount to scroll */
	
	w = FrontWindow();
	e = GetHelpInfo(w);
	val = (**scrollBar).contrlValue;
	min = (**scrollBar).contrlMin;
	max = (**scrollBar).contrlMax;
	lineHeight = (**(**e).help).lineHeight;
	viewHeight = w->portRect.bottom - sBarWidthM1 - lineHeight;
	dv = 0;
	switch (partCode) {
		case inUpButton:
			dv = (lineHeight < val-min) ? lineHeight : val-min;
			break;
		case inDownButton:
			dv = (lineHeight < max-val) ? -lineHeight : val-max;
			break;
		case inPageUp:
			dv = (viewHeight < val-min) ? viewHeight : val-min;
			break;
		case inPageDown:
			dv = (viewHeight < max-val) ? -viewHeight : val-max;
			break;
	}
	if (dv) {
		SetCtlValue(scrollBar, val-dv);
		Scroll(w, dv);
	}
}

/*_____________________________________________________________________

	DrawGrowBox - Draw Grow Box.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

static void DrawGrowBox (WindowPtr w)

{
	DrawGrowIcon(w);
}

/*_____________________________________________________________________

	InvalScroll - Invalidate Scroll Bars.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/


static void InvalScroll (WindowPtr w)

{
	Rect				r;					/* rectangle to invalidate */
	
	r = w->portRect;
	r.left = r.right - sBarWidthM1;
	InvalRect(&r);
	r = w->portRect;
	r.top = r.bottom - sBarWidthM1;
	InvalRect(&r);
}

/*_____________________________________________________________________

	CloseHelpWindow - Close Window.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

static void CloseHelpWindow (WindowPtr w)

{
	HelpInfo			**e;			/* handle to help info */
	short				windNum;		/* window number */
	
	e = GetHelpInfo(w);
	windNum = (**e).windNum;
	if (windNum <= numPosSave) wstm_Save(w, &HelpStates[windNum]);
	TEDispose((**e).help);
	DisposHandle((Handle)e);
	oop_DoClose(w);
}

/*_____________________________________________________________________

	NewHelpWindow - Create New Help Window.
	
	Entry:	theItem = item number in Help menu.
_____________________________________________________________________*/

static void NewHelpWindow (short theItem)
	
{
	Str255				topic;				/* help topic */
	WindowPtr			v;						/* traverses window list */
	HelpInfo				**g;					/* handle to help info for window v */
	Boolean				haveWindNum;		/* true when window number found */
	short					windNum;				/* window number */
	WindState			windState;			/* window state record */
	HelpInfo				**e;					/* handle to help info */
	WindowPtr			w;						/* pointer to window */
	Rect					r;						/* general purpose rect */
	Str255				ttl;					/* title of window v */
	TEHandle				help;					/* handle to help terec */
	OSErr					rCode;				/* result code */
	char					*p;					/* pointer into help topic */
	short					i;						/* loop index */
	
	/* Get the help topic. */
	
	GetItem(HelpMenu, theItem, topic);
	
	/* Check to see if this topic help window is already open. */
	
	v = FrontWindow();
	while (v) {
		if (oop_GetWindKind(v) == helpWind) {
			g = GetHelpInfo(v);
			GetWTitle(v, ttl);
			if (EqualString(ttl, topic, true, true) &&
				EqualString(HelpServer, (**g).server, true, true)) {
				SelectWindow(v);
				return;
			}
		}
		v = (WindowPtr)(((WindowPeek)v)->nextWindow);
	}
	
	/* Create the new window record */

	windNum = 1;
	haveWindNum = false;
	while (true) {
		v = FrontWindow();
		if (!v) break;
		while (v) {
			if (oop_GetWindKind(v) == helpWind) {
				g = GetHelpInfo(v);
				if ((**g).windNum == windNum) break;
			}
			v = (WindowPtr)((WindowPeek)v)->nextWindow;
			if (!v) haveWindNum = true;
		}
		if (haveWindNum) break;
		windNum++;
	}
	windState.moved = false;
	w = wstm_Restore(false, helpWindID, nil, 
		windNum <= numPosSave ? &HelpStates[windNum] : &windState);
	SetPort(w);
	TextFont(FontNum);
	TextSize(fontSize);
	SetWTitle(w, topic);
	
	/* Create the new HelpInfo record. */
	
	e = (HelpInfo**)NewHandle(sizeof(HelpInfo));
	(**e).scrollBar = GetNewControl(helpSBarID, w);
	GetViewRect(w, &r);
	InsetRect(&r, textMargin, 0);
	(**e).help = help = TENew(&r, &r);
	(**e).windNum = windNum;
	utl_CopyPString((**e).server, HelpServer);
	
	/* Create the new window object. */
	
	oop_NewWindow(w, helpWind, (Handle)e, &dispatch);
	
	/* Fetch the help text. */
	
	HLock(HelpTopics);
	p = *HelpTopics;
	theItem -= HelpMenuLength;
	while (--theItem) p += *p+1;
	utl_CopyPString(topic, p);
	for (i = 1; i <= *topic; i++) if (topic[i] == ' ') topic[i] = '_';
	HUnlock(HelpTopics);
	if (rCode = serv_GetHelp(HelpServer, topic, (**help).hText)) {
		glob_ErrorCode(rCode);
		SetPort(w);
		help_DoClose(w);
		return;
	}
	SetPort(w);
	(**help).teLength = GetHandleSize((**help).hText);
	TECalText(help);
	(**help).destRect.top = 0;
	(**help).destRect.bottom = (**help).nLines * (**help).lineHeight;
	AdjustScrollMax(w);
	
	/* Show the window. */
	
	ChangeSize(w, w->portRect.bottom);
	InvalRect(&w->portRect);
	ShowWindow(w);	
	
	return;
}

/*_____________________________________________________________________

	help_DoPeriodic - Do Periodic Tasks.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void help_DoPeriodic (WindowPtr w)

{
	HelpInfo				**e;				/* handle to help info */
	Point					where;			/* current mouse loc in local coords */
	Rect					viewRect;		/* view rect */
	ControlHandle		scrollBar;		/* handle to scroll bar control */
	short					newBalloon;		/* index in STR# of balloon help msg, or 0 if
													none */
	Rect					hotRect;			/* balloon help hot rectangle */
	Point					tip;				/* balloon help tip location */
	HMMessageRecord	helpMsg;			/* help message record */
	
	static short		oldBalloon = 0;	/* index in STR# of previous balloon help msg,
														or 0 if none */

	e = GetHelpInfo(w);
	SetPort(w);
	TEIdle((**e).help);
	GetViewRect(w, &viewRect);
	GetMouse(&where);
	SetCursor(PtInRect(where, &viewRect) ? *IBeamHandle : &qd.arrow);
	if (HaveBalloons && HMGetBalloons()) {
		scrollBar = (**e).scrollBar;
		if (PtInRect(where, &viewRect)) {
			newBalloon = hbHelpText;
			hotRect = viewRect;
		} else if (PtInRect(where, &(**scrollBar).contrlRect)) {
			newBalloon = (**scrollBar).contrlMin < (**scrollBar).contrlMax ?
				hbScrollEnabled : hbScrollDisabled;
				hotRect = (**scrollBar).contrlRect;
		} else {
			hotRect = w->portRect;
			hotRect.top = hotRect.bottom - sBarWidthM1;
			hotRect.left = hotRect.right - sBarWidthM1;
			if (PtInRect(where, &hotRect)) {
				newBalloon = hbGrow;
			} else {
				newBalloon = 0;
			}
		}
		if (!HMIsBalloon()) oldBalloon = 0;
		if (newBalloon != oldBalloon) {
			if (newBalloon) {
				LocalToGlobal((Point*)&hotRect);
				LocalToGlobal((Point*)&hotRect.bottom);
				tip.h = (hotRect.left + hotRect.right) >> 1;
				tip.v = (hotRect.top + hotRect.bottom) >> 1;
				helpMsg.hmmHelpType = khmmStringRes;
				helpMsg.u.hmmStringRes.hmmResID = hbStringsId;
				helpMsg.u.hmmStringRes.hmmIndex = newBalloon;
				if (!HMShowBalloon(&helpMsg, tip, &hotRect, 
					nil, 0, 0, kHMRegularWindow)) oldBalloon = newBalloon;
			} else {
				oldBalloon = 0;
			}
		}
	}
}

/*_____________________________________________________________________

	help_DoClick - Process a Mouse Down Event.
	
	Entry:	w = pointer to window record.
				where = mouse click location in local coordinates.
				modifiers = modifiers from event record.
_____________________________________________________________________*/

void help_DoClick (WindowPtr w, Point where, short modifiers)

{
	HelpInfo			**e;					/* handle to help info */
	ControlHandle	whichControl;		/* control clicked in */
	short				partCode;			/* control part code */
	short				oldVal;				/* old scroll bar control value */
	short				dv;					/* amount to scroll */
	Rect				viewRect;			/* view rect */
	
	e = GetHelpInfo(w);
	GetViewRect(w, &viewRect);
	if (PtInRect(where, &viewRect)) {
		TEAutoView(true, (**e).help);
		TEClick(where, modifiers & shiftKey ? true : false, (**e).help);
		TEAutoView(false, (**e).help);
		SetCtlValue((**e).scrollBar, -(**(**e).help).destRect.top);
	} else {
		partCode = FindControl(where, w, &whichControl);
		if (whichControl == (**e).scrollBar) {
			if (partCode == inThumb) {
				oldVal = GetCtlValue(whichControl);
				TrackControl(whichControl, where, nil);
				dv = GetCtlValue(whichControl) - oldVal;
				if (dv) Scroll(w, -dv);
			} else {
				TrackControl(whichControl, where, (ProcPtr)ScrollAction);
			}
			AdjustScrollMax(w);
		}
	}
}

/*_____________________________________________________________________

	help_DoKey - Process a Key Down Event.
	
	Entry:	w = pointer to window record.
				key = ascii code of key.
				modifiers = modifiers from event record.
_____________________________________________________________________*/

void help_DoKey (WindowPtr w, char key, short modifiers)

{
#pragma unused (modifiers)

	HelpInfo			**e;				/* handle to query info */
	TEHandle			help;				/* handle to terec */
	ControlHandle	scrollBar;		/* handle to scroll bar */
	short				val;				/* scroll bar value */
	short				min;				/* scroll bar minimum value */
	short				max;				/* scroll bar maximum value */

	e = GetHelpInfo(w);
	if (key == leftArrow || key == rightArrow ||
		key == downArrow || key == upArrow) {
		help = (**e).help;
		TEAutoView(true, help);
		TEKey(key, help);
		TEAutoView(false, help);
		SetCtlValue((**e).scrollBar, -(**help).destRect.top);
	} else if (key == pageUpKey || key == pageDownKey ||
		key == homeKey || key == endKey) {
		scrollBar = (**e).scrollBar;
		val = (**scrollBar).contrlValue;
		min = (**scrollBar).contrlMin;
		max = (**scrollBar).contrlMax;
		if (key == pageUpKey) {
			ScrollAction(scrollBar, inPageUp);
		} else if (key == pageDownKey) {
			ScrollAction(scrollBar, inPageDown);
		} else if (key == homeKey) {
			SetCtlValue(scrollBar, min);
			Scroll(w, val-min);
		} else {
			SetCtlValue(scrollBar, max);
			Scroll(w, val-max);
		}
	} else {
		SysBeep(0);
	}
}

/*_____________________________________________________________________

	help_DoUpdate - Process an Update Event.
_____________________________________________________________________*/

void help_DoUpdate (WindowPtr w)

{
	HelpInfo		**e;					/* handle to help info */
	Rect			viewRect;			/* view rect */

	e = GetHelpInfo(w);
	EraseRect(&w->portRect);
	DrawControls(w);
	GetViewRect(w, &viewRect);
	TEUpdate(&viewRect, (**e).help);
	DrawGrowBox(w);
}

/*_____________________________________________________________________

	help_DoActivate - Process an Activate Event.	
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void help_DoActivate (WindowPtr w)

{
	HelpInfo		**e;			/* handle to help info */
	
	e = GetHelpInfo(w);
	ShowControl((**e).scrollBar);
	TEActivate((**e).help);
	DrawGrowBox(w);
}

/*_____________________________________________________________________

	help_DoDeactivate - Process a Deactivate Event.	
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void help_DoDeactivate (WindowPtr w)

{
	HelpInfo		**e;			/* handle to help info */
	
	e = GetHelpInfo(w);
	HideControl((**e).scrollBar);
	TEDeactivate((**e).help);
	DrawGrowBox(w);
}

/*_____________________________________________________________________

	help_DoGrow - Process Window Grow Operation.
	
	Entry:	w = pointer to window record.
				where = location of mouse click in global coordinates.
_____________________________________________________________________*/

void help_DoGrow(WindowPtr w, Point where)

{
	unsigned long			newSize;			/* new window size */
	short						height;			/* new window height */
	short						width;			/* new window width */

	SetPort(w);
	newSize = GrowWindow(w, where, &SizeRect);
	if (!newSize) return;
	height = newSize >> 16;
	if (!(height & 1)) height--;
	width = newSize & 0xffff;
	InvalScroll(w);
	SizeWindow(w, width, height, true);
	InvalScroll(w);
	ChangeSize(w, height);
}

/*_____________________________________________________________________

	help_DoZoom - Process Window Zoom Operation.
	
	Entry:	w = pointer to window record.
				where = location of mouse click in global coordinates.
				partCode = inZoomIn or inZoomOut
_____________________________________________________________________*/

void help_DoZoom (WindowPtr w, Point where, short partCode)

{
	if (!TrackBox(w, where, partCode)) return;
	SetPort(w);
	if (partCode == inZoomOut) wstm_ComputeStd(w);
	EraseRect(&w->portRect);
	ZoomWindow(w, partCode, false);
	ChangeSize(w, w->portRect.bottom - w->portRect.top);
}

/*_____________________________________________________________________

	help_DoClose - Close Window.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void help_DoClose (WindowPtr w)

{
	CloseHelpWindow(w);
}

/*_____________________________________________________________________

	help_DoCommand - Process a Command.
	
	Entry:	top = pointer to top window record.
				theMenu = menu number.
				theItem = menu item number.
_____________________________________________________________________*/

#pragma segment command

Boolean help_DoCommand (WindowPtr top, short theMenu, short theItem)

{
	HelpInfo			**e;			/* handle to help info */
	TEHandle			help;			/* handle to help terec */
	OSErr				rCode;		/* error code */
	Str255			server;		/* help server domain name */
	
	switch (theMenu) {
		case editID:
			if (!top) break;
			e = GetHelpInfo(top);
			help = (**e).help;
			switch (theItem) {
				case copyCmd:
					TECopy(help);
					ZeroScrap();
					TEToScrap();
					return true;
				case selectAllCmd:
					TESetSelect(0, 32767, help);
					return true;
			}
			break;
		case helpID:
		case kHMHelpMenuID:
			HiliteMenu(0);
			if (theItem > HelpMenuLength) {
				if (HelpTopics) {
					NewHelpWindow(theItem);
				} else {
					glob_Error(servErrors, msgNoHelp, nil);
				}
			} else {
				utl_CopyPString(server, HelpServer);
				while (true) {
					if (site_DoDialog(server, helpLabelMsg1, helpLabelMsg2)) break;
					if (rCode = serv_GetHelpList(server)) {
						glob_ErrorCode(rCode);
						continue;
					}
					utl_CopyPString(HelpServer, server);
					break;
				}
			}
			return true;
	}
	return false;
}

#pragma segment help

/*_____________________________________________________________________

	help_Init - Initialize
_____________________________________________________________________*/

#pragma segment init

void help_Init (void)

{
	WindowPtr	w;					/* pointer to dummy window */
	
	w = GetNewWindow(helpWindID, nil, (WindowPtr)-1);
	SetPort(w);
	SetRect(&SizeRect, w->portRect.right+1, minWindHeight, 
		w->portRect.right+1, 0x7fff);
	DisposeWindow(w);
	oop_RegisterCommandHandler(help_DoCommand);
}

#pragma segment help

/*_____________________________________________________________________

	help_Terminate - Terminate
_____________________________________________________________________*/

void help_Terminate (void)

{
	WindowPtr		v;				/* traverses window list */
	HelpInfo			**g;			/* handle to help info for window v */
	short				windNum;		/* window number */
	
	v = FrontWindow();
	while (v) {
		if (oop_GetWindKind(v) == helpWind) {
			g = GetHelpInfo(v);
			windNum = (**g).windNum;
			if (windNum <= numPosSave) wstm_Save(v, &HelpStates[windNum]);
		}
		v = (WindowPtr)((WindowPeek)v)->nextWindow;
	}
}

/*_____________________________________________________________________

	help_BuildHelpMenu - Build Help Menu.
	
	Entry:	HelpTopics = help topic list.
	
	Exit:		HelpMenu = rebuilt help menu.
	
	If a help topic contains the substring "..", only the part of the
	help topic following the first occurence of the substring ".." is
	displayed in the help menu. All occurences of '_' in help topics
	are also replaced by ' '.
_____________________________________________________________________*/

void help_BuildHelpMenu (void)

{
	short				nItems;			/* number of menu items */
	short				item;				/* menu item number */
	Str255			ttl;				/* menu command */
	short				len;				/* length of menu command */
	char				*p;				/* pointer into help topic */
	char				*pEnd;			/* pointer to end of help topic list */
	char 				*q;				/* pointer into help topic */
	short				i;					/* loop index */

	nItems = CountMItems(HelpMenu);
	for (item = nItems; item > HelpMenuLength; item--) DelMenuItem(HelpMenu, item);
	if (HelpTopics) {
		HLock(HelpTopics);
		p = *HelpTopics;
		pEnd = p + GetHandleSize(HelpTopics);
		item = HelpMenuLength;
		while (p < pEnd) {
			len = *p;
			q = p+1;
			while (len > 2 && (*q != '.' || *(q+1) != '.')) {
				q++;
				len--;
			}
			if (len > 2) {
				q += 2;
				len -= 2;
			} else {
				q = p+1;
				len = *p;
			}
			if (len) {
				item++;
				memcpy(ttl+1, q, len);
				*ttl = len;
				for (i = 1; i <= len; i++) if (ttl[i] == '_') ttl[i] = ' ';
				AppendMenu(HelpMenu, "\p ");
				SetItem(HelpMenu, item, ttl);
			}
			p += *p+1;
		}
		HUnlock(HelpTopics);
	} else {
		GetIndString(ttl, stringsID, whyCmd);
		AppendMenu(HelpMenu, ttl);
	}
	CalcMenuSize(HelpMenu);
}
