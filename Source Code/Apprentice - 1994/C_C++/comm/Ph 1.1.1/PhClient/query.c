/*_____________________________________________________________________

  	query.c - Query Window Manager
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
#include "query.h"
#include "oop.h"
#include "wstm.h"

#pragma segment query

/*_____________________________________________________________________

	Defines.
_____________________________________________________________________*/

#define headerRule		73			/* v coord of top line of header double rule */
#define headerHeight		(headerRule+4)		
											/* total height of window header area */
#define sBarWidth			16			/* width of scroll bar */
#define sBarWidthM1		(sBarWidth-1)
#define textMargin		4			/* left and right margins for text */
#define minWindHeight	250		/* min window height */
#define teSlop				3			/* extra pixels around textedit boxes */
#define teHeight			12			/* height of textedit boxes */
#define popupTop			11			/* top coord popup menu */
#define popupBot			(popupTop+16)			
											/* bot coord popup menu */
#define popupLeft			13			/* left coord popup menu */
#define popupRight		314		/* right coord popup menu */
#define domainTop			14			/* top coord domain name te box */
#define domainBot			(domainTop+11) 
											/* bot coord domain name te box */
#define domainLeft		(popupRight+15)
											/* left coord domain name te box */
#define domainRight		490		/* right coord domain name te box */
#define queryTop			46			/* top coord query te box */
#define queryBot			(queryTop+11)
											/* bot coord query te box */
#define queryLeft			(popupLeft+teSlop)
											/* left coord query te box */
#define queryRight		410		/* right coord query te box */

/*_____________________________________________________________________

	Types.
_____________________________________________________________________*/

typedef struct QueryInfo {
	ControlHandle		scrollBar;				/* handle to scroll bar control */
	ControlHandle		doitBtn;					/* handle to doit button */
	short					serverInx;				/* index of currently selected server */
	short					windNum;					/* window number */
	TEHandle				domain;					/* handle to domain name terec */
	TEHandle				query;					/* handle to query terec */
	TEHandle				reply;					/* handle to reply terec */
	TEHandle				curEdit;					/* handle to current active terec */
} QueryInfo;

/*_____________________________________________________________________

	Global Variables.
_____________________________________________________________________*/

static Rect				SizeRect;				/* size rectangle */
static Rect				PopupRect = {popupTop, popupLeft, popupBot, popupRight};
														/* popup menu rect */
static Rect				DomainRect = {domainTop-teSlop, domainLeft-teSlop, 
								domainBot+teSlop, domainRight+teSlop};
														/* domain name te box */
static Rect				DomainClickRect = {domainTop, domainLeft-teSlop,
								domainBot, domainRight+teSlop};
														/* domain name clickable box */
static Rect				QueryRect = {queryTop-teSlop, queryLeft-teSlop, 
								queryBot+teSlop, queryRight+teSlop};				
														/* query te box */
static Rect				QueryClickRect = {queryTop, queryLeft-teSlop,
								queryBot, queryRight+teSlop};
														/* query clickable box */

static oop_Dispatch dispatch = {
								query_DoPeriodic,
								query_DoClick,
								query_DoKey,
								query_DoUpdate,
								query_DoActivate,
								query_DoDeactivate,
								query_DoGrow,
								query_DoZoom,
								query_DoClose,
								query_DoCommand
							};

/*_____________________________________________________________________

	GetQueryInfo - Get QueryInfo Handle from Window Pointer
	
	Entry:	w = pointer to window record
	
	Exit:		Function result = handle to QueryInfo record.
_____________________________________________________________________*/

static QueryInfo **GetQueryInfo (WindowPtr w)

{
	return (QueryInfo**)oop_GetWindInfo(w);
}

/*_____________________________________________________________________

	GetViewRect - Get View Rectangle.
	
	Entry:	w = pointer to window record.
	
	Exit:		*viewRect = view rectangle.
_____________________________________________________________________*/

static void GetViewRect (WindowPtr w, Rect *viewRect)

{
	*viewRect = w->portRect;
	viewRect->top += headerHeight;
	viewRect->bottom -= sBarWidthM1;
	viewRect->right -= sBarWidthM1;
}

/*_____________________________________________________________________

	AdjustScrollMax - Adjust Scroll Bar Max Value.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

static void AdjustScrollMax (WindowPtr w)

{
	QueryInfo		**e;			/* handle to query info */
	TEHandle			reply;		/* handle to reply terec */
	Rect				viewRect;	/* view rect */
	short				vHeight;		/* height of view rect */
	short				tHeight;		/* height of reply text */
	short				max;			/* new max control value */
	
	e = GetQueryInfo(w);
	reply = (**e).reply;
	GetViewRect(w, &viewRect);
	vHeight = viewRect.bottom - viewRect.top;
	tHeight = (**reply).destRect.bottom - (**reply).destRect.top;
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
	QueryInfo		**e;			/* handle to query info */
	
	e = GetQueryInfo(w);
	SetPort(w);
	TEScroll(0, dv, (**e).reply);
	SetOrigin(0, 0);
}

/*_____________________________________________________________________

	ChangeSize - Process Window Size Change.
	
	Entry:	w = pointer to window record.
				height = new window height
_____________________________________________________________________*/

static void ChangeSize (WindowPtr w, short height)

{
	QueryInfo	**e;					/* handle to query info */
	Rect			viewRect;			/* view rectangle */
	TEHandle		reply;				/* handle to reply terec */
	short			offTop;				/* amount of text scrolled off top */
	short			textBot;				/* bottom text coord */
	short			viewBot;				/* bottom view coord */
	short			dv;					/* amount to scroll */

	e = GetQueryInfo(w);
	SizeControl((**e).scrollBar, sBarWidth, height-12-headerHeight);
	AdjustScrollMax(w);
	GetViewRect(w, &viewRect);
	reply = (**e).reply;
	InsetRect(&viewRect, textMargin, 0);
	(**reply).viewRect = viewRect;
	offTop = headerHeight - (**reply).destRect.top;
	textBot = (**reply).destRect.bottom;
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
	QueryInfo		**e;				/* handle to query info */
	short				val;				/* scroll bar value */
	short				min;				/* scroll bar minimum value */
	short				max;				/* scroll bar maximum value */
	short				viewHeight;		/* height of view rectangle */
	short				lineHeight;		/* height of one line */
	short				dv;				/* amount to scroll */
	
	w = FrontWindow();
	e = GetQueryInfo(w);
	val = (**scrollBar).contrlValue;
	min = (**scrollBar).contrlMin;
	max = (**scrollBar).contrlMax;
	lineHeight = (**(**e).reply).lineHeight;
	viewHeight = w->portRect.bottom - sBarWidthM1 - headerHeight - lineHeight;
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
	Rect			r;				/* clip rect */
	
	r = w->portRect;
	r.top += headerHeight-1;
	ClipRect(&r);
	DrawGrowIcon(w);
	SetClip(BigClipRgn);
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

	CloseQueryWindow - Close Window.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

static void CloseQueryWindow (WindowPtr w)

{
	QueryInfo			**e;			/* handle to query info */
	short					windNum;		/* window number */
	
	e = GetQueryInfo(w);
	windNum = (**e).windNum;
	if (windNum <= numPosSave) wstm_Save(w, &QueryStates[windNum]);
	TEDispose((**e).domain);
	TEDispose((**e).query);
	TEDispose((**e).reply);
	DisposHandle((Handle)e);
	oop_DoClose(w);
}

/*_____________________________________________________________________

	NewQueryWindow - Create New Query Window.
_____________________________________________________________________*/

static void NewQueryWindow (void)
	
{
	WindowPtr			v;						/* traverses window list */
	QueryInfo			**g;					/* handle to query info for window v */
	Boolean				haveWindNum;		/* true when window number found */
	WindState			windState;			/* window state record */
	QueryInfo			**e;					/* handle to query info */
	WindowPtr			w;						/* pointer to window */
	Str255				tmpl;					/* window title template */
	Str255				ttl;					/* window title */
	short					windNum;				/* window number */
	Str255				windNumStr;			/* window number string */
	Rect					r;						/* general purpose rect */
	Rect					d;						/* textedit dest rect */
	
	/* Create the new window record */

	windNum = 1;
	haveWindNum = false;
	while (true) {
		v = FrontWindow();
		if (!v) break;
		while (v) {
			if (oop_GetWindKind(v) == queryWind) {
				g = GetQueryInfo(v);
				if ((**g).windNum == windNum) break;
			}
			v = (WindowPtr)((WindowPeek)v)->nextWindow;
			if (!v) haveWindNum = true;
		}
		if (haveWindNum) break;
		windNum++;
	}
	windState.moved = false;
	w = wstm_Restore(false, queryWindID, nil, 
		windNum <= numPosSave ? &QueryStates[windNum] : &windState);
	SetPort(w);
	TextFont(FontNum);
	TextSize(fontSize);
	NumToString(windNum, windNumStr);
	GetIndString(tmpl, stringsID, queryWindTtl);
	utl_PlugParams(tmpl, ttl, windNumStr, nil, nil, nil);
	SetWTitle(w, ttl);
	
	/* Create the new QueryInfo record. */
	
	e = (QueryInfo**)NewHandle(sizeof(QueryInfo));
	(**e).scrollBar = GetNewControl(querySBarID, w);
	(**e).doitBtn = GetNewControl(queryDoitID, w);
	HiliteControl((**e).doitBtn, 255);
	r = DomainRect;
	InsetRect(&r, teSlop, teSlop);
	(**e).domain = TENew(&r, &r);
	TESetText(DefaultServer+1, *DefaultServer, (**e).domain);
	r = QueryRect;
	InsetRect(&r, teSlop, teSlop);
	(**e).query = TENew(&r, &r);
	(**e).curEdit = (**e).query;
	GetViewRect(w, &r);
	InsetRect(&r, textMargin, 0);
	d = r;
	d.top = d.bottom = headerHeight;
	(**e).reply = TENew(&d, &r);
	(**e).serverInx = glob_GetSiteIndex(DefaultServer);
	(**e).windNum = windNum;
	
	/* Create the new window object. */
	
	oop_NewWindow(w, queryWind, (Handle)e, &dispatch);
	
	/* Finish initializing and show the window. */
	
	ChangeSize(w, w->portRect.bottom);
	InvalRect(&w->portRect);
	ShowWindow(w);	
	
	return;
}

/*_____________________________________________________________________

	AutoScroll - Make Sure Selection Range is Visible.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

static void AutoScroll (WindowPtr w)

{
	QueryInfo		**e;				/* handle to query info */
	TEHandle			reply;			/* handle to reply terec */
	
	e = GetQueryInfo(w);
	reply = (**e).reply;
	TEAutoView(true, (**e).reply);
	TESelView(reply);
	TEAutoView(false, (**e).reply);
	SetCtlValue((**e).scrollBar, headerHeight - (**reply).destRect.top);
}

/*_____________________________________________________________________

	SelectEmailAddress - Select Next Email Address.
	
	Entry:	w = pointer to window record.
				autoScroll = true to scoll selection range into view.
				backup = true to select previous email address.
	
	Exit:		function result = true if email address selected, else false.
_____________________________________________________________________*/

static Boolean SelectEmailAddress (WindowPtr w, Boolean autoScroll, Boolean backup)

{
	QueryInfo		**e;				/* handle to query info */
	TEHandle			reply;			/* handle to reply terec */
	Handle			hText;			/* handle to reply text */
	short				teLength;		/* length of reply text */
	Str255			emailStr;		/* "email: " search string */
	short				emailStrLen;	/* length of search string */
	char				*p;				/* pointer into reply text */
	char				*pEnd;			/* pointer to last search anchor pos */
	char				*q;				/* pointer into reply text */
	char				*qEnd;			/* pointer to end of reply text */
	short				selStart;		/* selection start pos */
	short				selEnd;			/* selection end pos */
	
	e = GetQueryInfo(w);
	reply = (**e).reply;
	hText = (**reply).hText;
	teLength = (**reply).teLength;
	GetIndString(emailStr, fieldNames, emailFieldName);
	emailStrLen = *emailStr + 1;
	emailStr[emailStrLen++] = ':';
	emailStr[emailStrLen] = ' ';
	p = *hText + (backup ? (**reply).selStart-emailStrLen-1 : (**reply).selEnd);
	pEnd = *hText + teLength - emailStrLen;
	if (backup) {
		if (p < *hText) p = pEnd;
		while (p >= *hText) {
			if (!strncmp(p, emailStr+1, emailStrLen)) break;
			p--;
		}
		if (p < *hText) return false;
	} else {
		while (p < pEnd) {
			if (!strncmp(p, emailStr+1, emailStrLen)) break;
			p++;
		}
		if (p >= pEnd) return false;
	}
	p += emailStrLen;
	q = p;
	qEnd = *hText + teLength;
	while (q < qEnd && *q != ' ' && *q != '\n') q++;
	if (p == q) return false;
	selStart = p - *hText;
	selEnd = q - *hText;
	TESetSelect(selStart, selEnd, reply);
	if ((**e).curEdit != (**e).reply) {
		TEDeactivate((**e).curEdit);
		TEActivate((**e).reply);
		(**e).curEdit = (**e).reply;
	}
	if (autoScroll) AutoScroll(w);
	return true;
}

/*_____________________________________________________________________

	Doit - Do A Query.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void Doit (WindowPtr w)

{
	QueryInfo		**e;				/* handle to query info */
	TEHandle			domain;			/* handle to domain terec */
	TEHandle			query;			/* handle to query terec */
	TEHandle			reply;			/* handle to reply terec */
	short				domainLen;		/* length of domain name */
	Str255			server;			/* server */
	OSErr				rCode;			/* result code */
	Rect				viewRect;		/* view rect */
	short				i;					/* loop index */
	Str255			emailDomain;	/* email domain name */
	char				*p;				/* pointer to server domain name */
	char				*q;				/* pointer to email domain name */
	
	e = GetQueryInfo(w);
	domain = (**e).domain;
	query = (**e).query;
	reply = (**e).reply;
	TESetSelect(0, 0xffff, reply);
	TEDelete(reply);
	SetCtlValue((**e).scrollBar, 0);
	domainLen = (**domain).teLength;
	if (domainLen > 255) domainLen = 255;
	*server = domainLen;
	memcpy(server+1, *(**domain).hText, domainLen);
	p = *Servers;
	for (i = 1; i <= NumSites; i++) {
		if (EqualString(server, p, true, true)) break;
		p += *p+1;
	}
	if (i > NumSites) {
		*emailDomain = 0;
	} else {
		q = *Domains;
		while (--i) q += *q+1;
		utl_CopyPString(emailDomain, q);
	}
	if (rCode = serv_DoQuery(server, (**query).hText, (**query).teLength,
		emailDomain, (**reply).hText)) {
		if (GetHandleSize((**reply).hText) == 0) glob_ErrorCode(rCode);
	}
	SetPort(w);
	(**reply).teLength = GetHandleSize((**reply).hText);
	TECalText(reply);
	(**reply).destRect.top = headerHeight;
	(**reply).destRect.bottom = headerHeight + 
		(**reply).nLines * (**reply).lineHeight;
	AdjustScrollMax(w);
	GetViewRect(w, &viewRect);
	InvalRect(&viewRect);
	oop_UpdateAll();
	TESetSelect(0, 0, reply);
	if (!SelectEmailAddress(w, false, false)) {
		if ((**e).curEdit != query) {
			TEDeactivate((**e).curEdit);
			TEActivate(query);
			(**e).curEdit = query;
		}
		TESetSelect(0, 0xffff, query);
	}
}

/*_____________________________________________________________________

	query_DoPeriodic - Do Periodic Tasks.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void query_DoPeriodic (WindowPtr w)

{
	QueryInfo			**e;				/* handle to query info */
	Point					where;			/* current mouse loc in local coords */
	Rect					viewRect;		/* view rect */
	Boolean				okToDoit;		/* true if Doit button should be hilited */
	ControlHandle		doitBtn;			/* handle to Doit button */
	ControlHandle		scrollBar;		/* handle to scroll bar control */
	short					newBalloon;		/* index in STR# of balloon help msg, or 0 if
													none */
	Rect					hotRect;			/* balloon help hot rectangle */
	Point					tip;				/* balloon help tip location */
	HMMessageRecord	helpMsg;			/* help message record */
	
	static short		oldBalloon = 0;	/* index in STR# of previous balloon help msg,
														or 0 if none */

	e = GetQueryInfo(w);
	SetPort(w);
	TEIdle((**e).curEdit);
	GetViewRect(w, &viewRect);
	GetMouse(&where);
	SetCursor(PtInRect(where, &DomainClickRect) || PtInRect(where, &QueryClickRect)
		|| PtInRect(where, &viewRect) ? *IBeamHandle : &qd.arrow);
	okToDoit = (**(**e).domain).teLength && (**(**e).query).teLength;
	doitBtn = (**e).doitBtn;
	HiliteControl(doitBtn, okToDoit ? 0 : 255);
	if (HaveBalloons && HMGetBalloons()) {
		scrollBar = (**e).scrollBar;
		if (PtInRect(where, &DomainClickRect)) {
			newBalloon = hbQueryDomainName;
			hotRect = DomainClickRect;
		} else if (PtInRect(where, &QueryClickRect)) {
			newBalloon = hbQueryQuery;
			hotRect = QueryClickRect;
		} else if (PtInRect(where, &viewRect)) {
			newBalloon = hbQueryResult;
			hotRect = viewRect;
		} else if (SiteMenu && PtInRect(where, &PopupRect)) {
			newBalloon = hbQueryPopup;
			hotRect = PopupRect;
		} else if (PtInRect(where, &(**doitBtn).contrlRect)) {
			newBalloon = okToDoit ? 
				hbQueryDoitEnabled : hbQueryDoitDisabled;
				hotRect = (**doitBtn).contrlRect;
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

	query_DoClick - Process a Mouse Down Event.
	
	Entry:	w = pointer to window record.
				where = mouse click location in local coordinates.
				modifiers = modifiers from event record.
_____________________________________________________________________*/

void query_DoClick (WindowPtr w, Point where, short modifiers)

{
	QueryInfo		**e;					/* handle to query info */
	short				oldInx;				/* old index of server in menu */
	short				newInx;				/* new index of server in menu */
	Str255			newDomain;			/* new server domain name */
	TEHandle			teHandle;			/* handle to TextEdit record */
	ControlHandle	whichControl;		/* control clicked in */
	short				partCode;			/* control part code */
	short				oldVal;				/* old scroll bar control value */
	short				dv;					/* amount to scroll */
	Rect				viewRect;			/* view rect */
	
	e = GetQueryInfo(w);
	GetViewRect(w, &viewRect);
	if (SiteMenu && PtInRect(where, &PopupRect)) {
		oldInx = (**e).serverInx;
		glob_PopupSiteMenu(&PopupRect, oldInx, &newInx, newDomain); 
		if (newInx != -1) {
			(**e).serverInx = newInx;
			teHandle = (**e).domain;
			TESetText(newDomain+1, *newDomain, teHandle);
			InvalRect(&DomainRect);
		}
	} else if (PtInRect(where, &DomainClickRect)) {
		if ((**e).curEdit != (**e).domain) {
			TEDeactivate((**e).curEdit);
			(**e).curEdit = (**e).domain;
			TEActivate((**e).domain);
		}
		TEClick(where, modifiers & shiftKey ? true : false, (**e).domain);
	} else if (PtInRect(where, &QueryClickRect)) {
		if ((**e).curEdit != (**e).query) {
			TEDeactivate((**e).curEdit);
			(**e).curEdit = (**e).query;
			TEActivate((**e).query);
		}
		TEClick(where, modifiers & shiftKey ? true : false, (**e).query);
	} else if (PtInRect(where, &viewRect)) {
		if ((**e).curEdit != (**e).reply) {
			TEDeactivate((**e).curEdit);
			(**e).curEdit = (**e).reply;
			TEActivate((**e).reply);
		}
		TEAutoView(true, (**e).reply);
		TEClick(where, modifiers & shiftKey ? true : false, (**e).reply);
		TEAutoView(false, (**e).reply);
		SetCtlValue((**e).scrollBar, headerHeight - (**(**e).reply).destRect.top);
	} else {
		partCode = FindControl(where, w, &whichControl);
		if (whichControl == (**e).doitBtn) {
			if (TrackControl(whichControl, where, nil)) {
				Doit(w);
			}
		} else if (whichControl == (**e).scrollBar) {
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

	query_DoKey - Process a Key Down Event.
	
	Entry:	w = pointer to window record.
				key = ascii code of key.
				modifiers = modifiers from event record.
_____________________________________________________________________*/

void query_DoKey (WindowPtr w, char key, short modifiers)

{
#pragma unused (modifiers)

	QueryInfo		**e;				/* handle to query info */
	ControlHandle	scrollBar;		/* handle to scroll bar */
	short				val;				/* scroll bar value */
	short				min;				/* scroll bar minimum value */
	short				max;				/* scroll bar maximum value */

	e = GetQueryInfo(w);
	if (key == returnKey || key == enterKey) {
		if ((**(**e).doitBtn).contrlHilite) {
			SysBeep(0);
		} else {
			utl_FlashButton((**e).doitBtn);
			Doit(w);
		}
	} else if (key == tabKey) {
		if (modifiers & shiftKey) {
			if ((**e).curEdit == (**e).query) {
				TEDeactivate((**e).query);
				TESetSelect(0, 0xffff, (**e).domain);
				TEActivate((**e).domain);
				(**e).curEdit = (**e).domain;
			} else if ((**e).curEdit == (**e).domain) {
				TEDeactivate((**e).domain);
				(**e).curEdit = (**e).reply;
				TESetSelect(0, 0, (**e).reply);
				if (!SelectEmailAddress(w, true, true)) TESetSelect(0, 0xffff, (**e).reply);
				TEActivate((**e).reply);
			} else if ((**e).curEdit == (**e).reply) {
				if (!SelectEmailAddress(w, true, true)) {
					TEDeactivate((**e).reply);
					TESetSelect(0, 0xffff, (**e).query);
					TEActivate((**e).query);
					(**e).curEdit = (**e).query;
				}
			}
		} else {
			if ((**e).curEdit == (**e).domain) {
				TEDeactivate((**e).domain);
				TESetSelect(0, 0xffff, (**e).query);
				TEActivate((**e).query);
				(**e).curEdit = (**e).query;
			} else if ((**e).curEdit == (**e).query) {
				TEDeactivate((**e).query);
				(**e).curEdit = (**e).reply;
				TESetSelect(0, 0, (**e).reply);
				if (!SelectEmailAddress(w, true, false)) TESetSelect(0, 0xffff, (**e).reply);
				TEActivate((**e).reply);
			} else if ((**e).curEdit == (**e).reply) {
				if (!SelectEmailAddress(w, true, false)) {
					TEDeactivate((**e).reply);
					TESetSelect(0, 0xffff, (**e).domain);
					TEActivate((**e).domain);
					(**e).curEdit = (**e).domain;
				}
			}
		}
	} else if (utl_StandardAsciiChar(key) || key == deleteKey) {
		if ((**e).curEdit == (**e).reply) {
			TEDeactivate((**e).reply);
			TESetSelect(0, 0xfffff, (**e).query);
			TEActivate((**e).query);
			(**e).curEdit = (**e).query;
		}
		TEKey(key, (**e).curEdit);
	} else if (key == leftArrow || key == rightArrow ||
		(key == downArrow || key == upArrow) && (**e).curEdit == (**e).reply) {
		TEAutoView(true, (**e).curEdit);
		TEKey(key, (**e).curEdit);
		TEAutoView(false, (**e).curEdit);
		SetCtlValue((**e).scrollBar, headerHeight - (**(**e).reply).destRect.top);
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

	query_DoUpdate - Process an Update Event.
_____________________________________________________________________*/

void query_DoUpdate (WindowPtr w)

{
	QueryInfo	**e;					/* handle to query info */
	short			right;				/* right coord window portrect */
	Rect			frameRect;			/* framing rect */
	Rect			viewRect;			/* view rect */

	e = GetQueryInfo(w);
	EraseRect(&w->portRect);
	DrawControls(w);
	frameRect = (**(**e).doitBtn).contrlRect;
	InsetRect(&frameRect, -4, -4);
	PenSize(3, 3);
	FrameRoundRect(&frameRect, 16, 16);
	PenSize(1, 1);
	if (SiteMenu) glob_DrawSitePopup(&PopupRect, (**e).serverInx);
	FrameRect(&DomainRect);
	TEUpdate(&DomainRect, (**e).domain);
	FrameRect(&QueryRect);
	TEUpdate(&QueryRect, (**e).query);
	MoveTo(0, headerRule);
	right = w->portRect.right;
	Line(right, 0);
	MoveTo(0, headerRule+2);
	Line(right, 0);
	GetViewRect(w, &viewRect);
	TEUpdate(&viewRect, (**e).reply);
	DrawGrowBox(w);
}

/*_____________________________________________________________________

	query_DoActivate - Process an Activate Event.	
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void query_DoActivate (WindowPtr w)

{
	QueryInfo		**e;			/* handle to query info */
	
	e = GetQueryInfo(w);
	ShowControl((**e).scrollBar);
	TEActivate((**e).curEdit);
	DrawGrowBox(w);
}

/*_____________________________________________________________________

	query_DoDeactivate - Process a Deactivate Event.	
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void query_DoDeactivate (WindowPtr w)

{
	QueryInfo		**e;			/* handle to query info */
	
	e = GetQueryInfo(w);
	HideControl((**e).scrollBar);
	TEDeactivate((**e).curEdit);
	DrawGrowBox(w);
}

/*_____________________________________________________________________

	query_DoGrow - Process Window Grow Operation.
	
	Entry:	w = pointer to window record.
				where = location of mouse click in global coordinates.
_____________________________________________________________________*/

void query_DoGrow(WindowPtr w, Point where)

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

	query_DoZoom - Process Window Zoom Operation.
	
	Entry:	w = pointer to window record.
				where = location of mouse click in global coordinates.
				partCode = inZoomIn or inZoomOut
_____________________________________________________________________*/

void query_DoZoom (WindowPtr w, Point where, short partCode)

{
	if (!TrackBox(w, where, partCode)) return;
	SetPort(w);
	if (partCode == inZoomOut) wstm_ComputeStd(w);
	EraseRect(&w->portRect);
	ZoomWindow(w, partCode, false);
	ChangeSize(w, w->portRect.bottom - w->portRect.top);
}

/*_____________________________________________________________________

	query_DoClose - Close Window.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void query_DoClose (WindowPtr w)

{
	CloseQueryWindow(w);
}

/*_____________________________________________________________________

	query_DoCommand - Process a Command.
	
	Entry:	top = pointer to top window record.
				theMenu = menu number.
				theItem = menu item number.
_____________________________________________________________________*/

#pragma segment command

Boolean query_DoCommand (WindowPtr top, short theMenu, short theItem)

{
	QueryInfo		**e;			/* handle to query info */
	TEHandle			textH;		/* handle to active TextEdit record */
	Handle			scrap;		/* handle to scrap to be pasted */
	short				scrapLen;	/* length of scrap to be pasted */
	char				*p;			/* pointer to scrap char */
	
	if (top) e = GetQueryInfo(top);
	switch (theMenu) {
		case fileID:
			switch (theItem) {
				case newQueryCmd:
					NewQueryWindow();
					return true;
			}
			break;
		case editID:
			if (!top) break;
			textH = (**e).curEdit;
			switch (theItem) {
				case cutCmd:
				case copyCmd:
					if (theItem == cutCmd) {
						TECut(textH);
					} else {
						TECopy(textH);
					}
					ZeroScrap();
					TEToScrap();
					return true;
				case pasteCmd:
					TEFromScrap();
					scrap = TEScrapHandle();
					scrapLen = GetHandleSize(scrap);
					if (scrapLen > 255) {
						SysBeep(0);
						return true;
					}
					p = *scrap;
					while (scrapLen--) {
						if (!utl_StandardAsciiChar(*p)) {
							SysBeep(0);
							return true;
						};
						p++;
					}
					TEPaste(textH);
					return true;
				case clearCmd:
					TEDelete(textH);
					return true;
				case selectAllCmd:
					TESetSelect(0, 32767, textH);
					return true;
			}
			break;
	}
	return false;
}

#pragma segment query

/*_____________________________________________________________________

	query_Init - Initialize
_____________________________________________________________________*/

#pragma segment init

void query_Init (void)

{
	WindowPtr	w;					/* pointer to dummy window */
	
	w = GetNewWindow(queryWindID, nil, (WindowPtr)-1);
	SetPort(w);
	SetRect(&SizeRect, w->portRect.right+1, minWindHeight, 
		w->portRect.right+1, 0x7fff);
	DisposeWindow(w);
	oop_RegisterCommandHandler(query_DoCommand);
	if (SiteMenu) {
		CalcMenuSize(SiteMenu);
		PopupRect.left = PopupRect.right - (**SiteMenu).menuWidth;
		if (PopupRect.left < popupLeft) PopupRect.left = popupLeft;
	}
	NewQueryWindow();
}

#pragma segment query

/*_____________________________________________________________________

	query_CanEditField - Check to See if Current Field Can be Edited.
	
	Entry:	w = pointer to window record.
	
	Exit:		function result = true if current field can be edited.
_____________________________________________________________________*/

Boolean query_CanEditField (WindowPtr w)

{
	QueryInfo		**e;				/* handle to query info */
	
	e = GetQueryInfo(w);
	return (**e).curEdit != (**e).reply;
}

/*_____________________________________________________________________

	query_NewSiteList - Process New Site List.
_____________________________________________________________________*/

void query_NewSiteList (void)

{
	WindowPtr		v;				/* traverses window list */
	QueryInfo		**g;			/* handle to query info for window v */
	Rect				r;				/* rect to invalidate */
	TEHandle			domain;		/* handle to domain terec */
	short				domainLen;	/* length of domain name */
	Str255			server;		/* server */
	short				i;				/* loop index */
	char				*p;			/* pointer to server domain name */
	
	r = PopupRect;
	InsetRect(&r, -2, -2);
	r.left = 0;
	v = FrontWindow();
	while (v) {
		if (oop_GetWindKind(v) == queryWind) {
			g = GetQueryInfo(v);
			SetPort(v);
			if (SiteMenu) {
				domain = (**g).domain;
				domainLen = (**domain).teLength;
				if (domainLen > 255) domainLen = 255;
				*server = domainLen;
				memcpy(server+1, *(**domain).hText, domainLen);
				p = *Servers;
				for (i = 1; i <= NumSites; i++) {
					if (EqualString(server, p, true, true)) break;
					p += *p+1;
				}
				if (i > NumSites) i = 1;
				(**g).serverInx = i;
			}
			InvalRect(&r);
		}
		v = (WindowPtr)((WindowPeek)v)->nextWindow;
	}
	if (SiteMenu) {
		CalcMenuSize(SiteMenu);
		PopupRect.left = PopupRect.right - (**SiteMenu).menuWidth;
		if (PopupRect.left < popupLeft) PopupRect.left = popupLeft;
	}
}

/*_____________________________________________________________________

	query_NewDefaultSite - Process New Default Site.
_____________________________________________________________________*/

void query_NewDefaultSite (void)

{
	WindowPtr			v;				/* traverses window list */
	QueryInfo			**g;			/* handle to query info for window v */
	
	v = FrontWindow();
	while (v) {
		if (oop_GetWindKind(v) == queryWind) {
			g = GetQueryInfo(v);
			SetPort(v);
			TESetText(DefaultServer+1, *DefaultServer, (**g).domain);
			(**g).serverInx = glob_GetSiteIndex(DefaultServer);
			InvalRect(&PopupRect);
			InvalRect(&DomainRect);
		}
		v = (WindowPtr)((WindowPeek)v)->nextWindow;
	}
}

/*_____________________________________________________________________

	query_Terminate - Terminate.
_____________________________________________________________________*/

void query_Terminate (void)

{
	WindowPtr		v;				/* traverses window list */
	QueryInfo		**g;			/* handle to query info for window v */
	short				windNum;		/* window number */
	
	v = FrontWindow();
	while (v) {
		if (oop_GetWindKind(v) == queryWind) {
			g = GetQueryInfo(v);
			windNum = (**g).windNum;
			if (windNum <= numPosSave) wstm_Save(v, &QueryStates[windNum]);
		}
		v = (WindowPtr)((WindowPeek)v)->nextWindow;
	}
}
