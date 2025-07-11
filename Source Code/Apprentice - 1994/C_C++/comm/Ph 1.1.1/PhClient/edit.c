/*_____________________________________________________________________

  	edit.c - Edit Window Manager
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
#include "edit.h"
#include "login.h"
#include "pswd.h"
#include "open.h"
#include "oop.h"
#include "new.h"
#include "wstm.h"

#pragma segment edit

/*_____________________________________________________________________

	Defines.
_____________________________________________________________________*/

#define headerRule		26			/* v coord of top line of header double rule */
#define headerHeight		(headerRule+4)		
											/* total height of window header area */
#define sBarWidth			16			/* width of scroll bar */
#define sBarWidthM1		(sBarWidth-1)
#define fChangedBase		11			/* base line for �=Field Changed */
#define cantChangeBase	22			/* base line for �=Can�t Change Field */
#define textMargin		4			/* left and right margins for text */
#define minWindHeight	125		/* min window height */
#define infoWhite1		3			/* white space before first field info rule */
#define infoWhite2		3			/* white space after first field info rule */
#define infoWhite3		5			/* white space before second field info rule */
#define infoWhite4		5			/* white space after second field info rule */
#define infoWhite			(infoWhite1+infoWhite2+infoWhite3+infoWhite4)
#define infoHeight		(infoWhite + 2 + 2*LineHeight)	
											/* total height of field info - must be even! */

/*_____________________________________________________________________

	Types.
_____________________________________________________________________*/

typedef struct EditInfo {
	FieldInfo			**fields;				/* handle to field info */
	short					numFields;				/* number of fields */
	short					curField;				/* current active field */
	short					aliasFieldIndex;		/* index in Fields array of alias field */
	ControlHandle		scrollBar;				/* handle to scroll bar control */
	ControlHandle		showFieldInfoBox;		/* handle to show field info checkbox */
	ControlHandle		revertButton;			/* handle to revert field button */
	ControlHandle		revertAllButton;		/* handle to revert all fields button */
	RgnHandle			clipRgn;					/* window clipped to scrolling area */
	Boolean				showFieldInfo;			/* true to show field info */
	Boolean				dirty;					/* true if any changes made */
	Str255				alias;					/* alias */
	Str255				server;					/* login server */
	Str255				loginAlias;				/* login alias */
	Str255				pswd;						/* login password */
	short					windNum;					/* window number */
} EditInfo;

/*_____________________________________________________________________

	Global Variables.
_____________________________________________________________________*/

static Boolean			LoggedIn = false;		/* true if logged in */
static WindowPtr		LoginWindow;			/* pointer to login window */
static Boolean			Hero;						/* true if hero */
static Str255			Server;					/* login server */
static Str255			Alias;					/* login alias */
static Str255			Password;				/* login password */
static Boolean			Proxy;					/* true if server supports proxy menu */

static Rect				SizeRect;				/* size rectangle */
static short			Width;					/* font char width */
static short			NameRight;				/* field name right margin */
static short			TextLeft;				/* field text left margin */
static short			TextRight;				/* field text right margin */

static oop_Dispatch dispatch = {
								edit_DoPeriodic,
								edit_DoClick,
								edit_DoKey,
								edit_DoUpdate,
								edit_DoActivate,
								edit_DoDeactivate,
								edit_DoGrow,
								edit_DoZoom,
								edit_DoClose,
								edit_DoCommand
							};

/*_____________________________________________________________________

	GetEditInfo - Get EditInfo Handle from Window Pointer
	
	Entry:	w = pointer to window record
	
	Exit:		Function result = handle to EditInfo record.
_____________________________________________________________________*/

static EditInfo **GetEditInfo (WindowPtr w)

{
	return (EditInfo**)oop_GetWindInfo(w);
}

/*_____________________________________________________________________

	TextTop - Get Top Coord of Text.
	
	Entry:	e = handle to EditInfo record.
	
	Exit:		Function result = top coord of text.
_____________________________________________________________________*/

static short TextTop (EditInfo **e)

{
	return (**(*(**e).fields)[0].edit).viewRect.top -
		((**e).showFieldInfo ? infoHeight : 0);
}

/*_____________________________________________________________________

	TextBot - Get Bottom Coord of Text.
	
	Entry:	e = handle to EditInfo record.
	
	Exit:		Function result = bottom coord of text.
_____________________________________________________________________*/

static short TextBot (EditInfo **e)

{
	return (**(*(**e).fields)[(**e).numFields-1].edit).viewRect.bottom;
}

/*_____________________________________________________________________

	TextHeight - Compute Total Text Height.
	
	Entry:	e = handle to EditInfo record.
	
	Exit:		Function result = total text height in pixels.
_____________________________________________________________________*/

static short TextHeight (EditInfo **e)

{
	return TextBot(e) - TextTop(e);
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

	MakeIbeamRgn - Make Ibeam Region.
	
	Entry:	w = pointer to window.
_____________________________________________________________________*/

static void MakeIbeamRgn (WindowPtr w)

{
	EditInfo		**e;			/* handle to edit info */
	short			i;				/* loop index */
	FieldInfo	*f;			/* pointer to field info record */
	Rect			r;				/* textedit rectangle */
	RgnHandle	rgn;			/* scratch region */
	short			numFields;	/* number of fields */
	RgnHandle	ibeamRgn;	/* ibeam region */

	e = GetEditInfo(w);
	ibeamRgn = oop_GetWindIbeamRgn(w);
	SetEmptyRgn(ibeamRgn);
	rgn = NewRgn();
	numFields = (**e).numFields;
	HLock((Handle)(**e).fields);
	for (i = 0; i < numFields; i++) {
		f = &(*(**e).fields)[i];
		if (f->canEdit) {
			r = (**f->edit).viewRect;
			r.left -= textMargin;
			r.right += textMargin;
			RectRgn(rgn, &r);
			UnionRgn(ibeamRgn, rgn, ibeamRgn);
		}
	}
	HUnlock((Handle)(**e).fields);
	DisposeRgn(rgn);
}

/*_____________________________________________________________________

	AdjustScrollMax - Adjust Scroll Bar Max Value.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

static void AdjustScrollMax (WindowPtr w)

{
	EditInfo			**e;					/* handle to edit info */
	short				windHeight;			/* height of window text */
	short				max;					/* computed max scroll bar value */
	
	SetPort(w);
	e = GetEditInfo(w);
	windHeight = w->portRect.bottom - sBarWidthM1 - headerHeight;
	max = TextHeight(e) - windHeight;
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
	EditInfo			**e;			/* handle to edit info */
	Rect				viewRect;	/* view rectangle */
	short				i;				/* loop index */
	FieldInfo		*f;			/* pointer to field info */
	TEHandle			edit;			/* handle to TextEdit record */
	RgnHandle		rgn;			/* scratch region */
	short				numFields;	/* number of fields */
	RgnHandle		ibeamRgn;	/* ibeam region */

	e = GetEditInfo(w);
	SetPort(w);
	GetViewRect(w, &viewRect);
	rgn = NewRgn();
	ScrollRect(&viewRect, 0, dv, rgn);
	InvalRgn(rgn);
	DisposeRgn(rgn);
	SetOrigin(0, 0);
	numFields = (**e).numFields;
	for (i = 0; i < numFields; i++) {
		f = &(*(**e).fields)[i];
		edit = f->edit;
		OffsetRect(&(**edit).viewRect, 0, dv);
		OffsetRect(&(**edit).destRect, 0, dv);
	}
	ibeamRgn = oop_GetWindIbeamRgn(w);
	OffsetRgn(ibeamRgn, 0, dv);
	oop_UpdateAll();
}

/*_____________________________________________________________________

	GetLineNumber - Get Line Number of Character Position in TextEdit field.
	
	Entry:	pos = character position (0-based).
				h = handle to TextEdit record..
					
	Exit:		function result = line number (0-based).
_____________________________________________________________________*/

static short GetLineNumber (short pos, TEHandle h)

{
	short			*lineStarts;			/* ptr to lineStarts array */
	short			nLines;					/* number of lines */
	short			i;							/* loop index */

	lineStarts = (**h).lineStarts;
	nLines = (**h).nLines;
	i = 0;
	while (lineStarts[i] <= pos && i <= nLines) i++;
	return i-1;
}

/*_____________________________________________________________________

	AutoScroll - Make Sure Selection Range is Visible.
	
	Entry:	w = pointer to window record.
				fNum = field number.
_____________________________________________________________________*/

static void AutoScroll (WindowPtr w, short fNum)

{
	EditInfo			**e;			/* handle to edit info */
	TEHandle			edit;			/* handle to TextEdit record */
	short				lineStart;	/* selection range start line num */
	short				lineEnd;		/* selection range end line num */
	short				top;			/* top coord of field */
	short				vStart;		/* selection range start v coord */
	short				vEnd;			/* selection range end v coord */
	Rect				viewRect;	/* view rectangle */
	Boolean			tooBig;		/* true if selection is bigger than view */
	short				dv;			/* number of pixels to scroll */
	short				oldVal;		/* old scroll bar control value */
	short				max;			/* scroll bar max value */
	
	e = GetEditInfo(w);
	edit = (*(**e).fields)[fNum].edit;
	lineStart = GetLineNumber((**edit).selStart, edit);
	lineEnd = GetLineNumber((**edit).selEnd, edit);
	top = (**edit).viewRect.top;
	vStart = top + lineStart*LineHeight;
	vEnd = top + (lineEnd+1)*LineHeight;
	GetViewRect(w, &viewRect);
	tooBig = (vEnd - vStart) > (viewRect.bottom - viewRect.top);
	if (vEnd > viewRect.bottom) {
		if (tooBig && vStart <= viewRect.top) return;
		dv = viewRect.bottom - vEnd;
	} else if (vStart < viewRect.top) {
		if (tooBig && vEnd >= viewRect.bottom) return;
		dv = viewRect.top - vStart + ((**e).showFieldInfo ? infoHeight : 0);
	} else {
		return;
	}
	oldVal = GetCtlValue((**e).scrollBar);
	max = GetCtlMax((**e).scrollBar);
	if (oldVal - dv > max) dv = oldVal - max;
	Scroll(w, dv);
	AdjustScrollMax(w);
	SetCtlValue((**e).scrollBar, oldVal-dv);
}

/*_____________________________________________________________________

	ChangeSize - Process Window Size Change.
	
	Entry:	w = pointer to window record.
				height = new window height
_____________________________________________________________________*/

static void ChangeSize (WindowPtr w, short height)

{
	EditInfo		**e;					/* handle to edit info */
	short			offTop;				/* amount of text scrolled off top */
	short			textBot;				/* bottom text coord */
	short			viewBot;				/* bottom view coord */
	short			dv;					/* amount to scroll */
	Rect			viewRect;			/* view rectangle */

	e = GetEditInfo(w);
	SizeControl((**e).scrollBar, sBarWidth, height-12-headerHeight);
	AdjustScrollMax(w);
	offTop = headerHeight - TextTop(e);
	textBot = TextBot(e);
	viewBot = w->portRect.bottom - sBarWidthM1;
	if (textBot < viewBot && offTop) {
		dv = viewBot - textBot;
		if (dv > offTop) dv = offTop;
		Scroll(w, dv);
	}
	GetViewRect(w, &viewRect);
	RectRgn((**e).clipRgn, &viewRect);
}

/*_____________________________________________________________________

	ScrollAction - Scroll Bar Action Procedure
_____________________________________________________________________*/

static pascal void ScrollAction (ControlHandle scrollBar, short partCode)

{
	WindowPtr		w;					/* pointer to window */
	short				val;				/* scroll bar value */
	short				min;				/* scroll bar minimum value */
	short				max;				/* scroll bar maximum value */
	short				viewHeight;		/* height of view rectangle */
	short				dv;				/* amount to scroll */
	
	w = FrontWindow();
	val = (**scrollBar).contrlValue;
	min = (**scrollBar).contrlMin;
	max = (**scrollBar).contrlMax;
	viewHeight = w->portRect.bottom - sBarWidthM1 - headerHeight - LineHeight;
	dv = 0;
	switch (partCode) {
		case inUpButton:
			dv = (LineHeight < val-min) ? LineHeight : val-min;
			break;
		case inDownButton:
			dv = (LineHeight < max-val) ? -LineHeight : val-max;
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

	GetAliasFieldIndex - Get Index in Fields Array of Alias Field
	
	Entry:	fields = handle to fields array.
				numFields = number of fields.
					
	Exit:		Function result = index in fields array of alias field.
_____________________________________________________________________*/

static short GetAliasFieldIndex (FieldInfo **fields, short numFields)

{
	short			i;						/* loop index */
	FieldInfo	*f;					/* pointer to field info record */
	char			aliasStr[20];		/* "alias" field name */

	GetIndString(aliasStr, fieldNames, aliasFieldName);
	p2cstr(aliasStr);
	for (i = 0; i < numFields; i++) {
		f = &(*fields)[i];
		if (!strcmp(f->name+1, aliasStr)) break;
	}
	return i;
}

/*_____________________________________________________________________

	GetProxyFieldIndex - Get Index in Fields Array of Proxy Field
	
	Entry:	fields = handle to fields array.
				numFields = number of fields.
					
	Exit:		Function result = index in fields array of proxy field.
_____________________________________________________________________*/

static short GetProxyFieldIndex (FieldInfo **fields, short numFields)

{
	short			i;						/* loop index */
	FieldInfo	*f;					/* pointer to field info record */
	char			proxyStr[20];		/* "alias" field name */

	GetIndString(proxyStr, fieldNames, proxyFieldName);
	p2cstr(proxyStr);
	for (i = 0; i < numFields; i++) {
		f = &(*fields)[i];
		if (!strcmp(f->name+1, proxyStr)) break;
	}
	return i;
}

/*_____________________________________________________________________

	GetHero - Check to See if User is a Hero.
	
	Entry:	fields = handle to field info.
				numFields = number of fields.
					
	Exit:		Function result = true if user is a hero.
_____________________________________________________________________*/

static Boolean GetHero (FieldInfo **fields, short numFields)

{
	short			i;						/* loop index */
	FieldInfo	*f;					/* pointer to field info record */
	char			heroStr[20];		/* "hero" field name */
	
	GetIndString(heroStr, fieldNames, heroFieldName);
	p2cstr(heroStr);
	for (i = 0; i < numFields; i++) {
		f = &(*fields)[i];
		if (!strcmp(f->name+1, heroStr)) break;
	}
	return f->origSize;
}

/*_____________________________________________________________________

	CompleteFieldInitialization - Complete Field Initialiation.
	
	Entry:	e = handle to edit info.
				Following fields set in fields array:
				name, maxSize, attributes, description, original, origSize.
					
	Exit:		Remaining fields set in fields array:
				canEdit, dirty, nLines, edit.
					
	Special text is also stored in the password field and in all encrypted
	fields.
	
	Fields which cannot be displayed and are empty or encrypted are
	removed from the Fields array.
_____________________________________________________________________*/

static void CompleteFieldInitialization (EditInfo **e)

{
	short			top;					/* top coord of field */
	short			i;						/* loop index */
	short			j;						/* loop index */
	FieldInfo	*f;					/* pointer to field info record */
	Rect			vdRect;				/* view = destination rect */
	TEHandle		hText;				/* handle to TextEdit record */
	short			nLines;				/* number of lines in text */
	char			changeStr[20];		/* "Change" attribute string */
	char			encryptStr[20];	/* "Encrypt" attribute string */
	char			pswdStr[20];		/* "password" field name */
	Str255		msg;					/* special text */
	Boolean		display;				/* true if field should be displayed */
	short			numFields;			/* number of fields */
	
	GetIndString(changeStr, fieldAttrs, changeAttr);
	p2cstr(changeStr);
	GetIndString(encryptStr, fieldAttrs, encryptAttr);
	p2cstr(encryptStr);
	GetIndString(pswdStr, fieldNames, pswdFieldName);
	p2cstr(pswdStr);
	top = headerHeight;
	numFields = (**e).numFields;
	HLock((Handle)(**e).fields);
	for (i = 0; i < numFields; i++) {
		f = &(*(**e).fields)[i];
		f->hasChangeAttr = strstr(*f->attributes+1, changeStr);
		f->canEdit = Hero ? true : f->hasChangeAttr;
		display = f->canEdit || f->origSize;
		if (!strcmp(f->name+1, pswdStr)) {
			f->canEdit = false;
			display = true;
			GetIndString(msg, stringsID, pswdMsg);
			SetHandleSize(f->original, *msg);
			memcpy(*f->original, msg+1, *msg);
			f->origSize = *msg;
		} else if (strstr(*f->attributes+1, encryptStr)) {
			if (display = Hero) {
				f->canEdit = false;
				GetIndString(msg, stringsID, encryptMsg);
				SetHandleSize(f->original, *msg);
				memcpy(*f->original, msg+1, *msg);
				f->origSize = *msg;
			}
		}
		if (display) {
			f->dirty = false;
			SetRect(&vdRect, TextLeft, top, TextRight, 0x7fff);
			f->edit = hText = TENew(&vdRect, &vdRect);
			(**hText).txSize = 9;
			(**hText).txFont = FontNum;
			(**hText).lineHeight = LineHeight;
			(**hText).fontAscent = Ascent;
			HLock(f->original);
			TESetText(*f->original, f->origSize, hText);
			HUnlock(f->original);
			TECalText(hText);
			nLines = (**hText).nLines;
			if (!nLines) nLines = 1;
			f->nLines = nLines;
			top += nLines*LineHeight;
			(**hText).destRect.bottom = (**hText).viewRect.bottom = top;
			if (!i) {
				TESetSelect(0, 0x7fff, hText);
				TEActivate(hText);
			}
		} else {
			DisposHandle(f->original);
			for (j = i+1; j < numFields; j++) (*(**e).fields)[j-1] = (*(**e).fields)[j];
			i--;
			numFields--;
		}
	}
	HUnlock((Handle)(**e).fields);
	SetHandleSize((Handle)(**e).fields, numFields*sizeof(FieldInfo));
	(**e).numFields = numFields;
	(**e).aliasFieldIndex = GetAliasFieldIndex((**e).fields, (**e).numFields);
}

/*_____________________________________________________________________

	ToggleShowFieldInfo - Toggle Show Field Info Option.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

static void ToggleShowFieldInfo (WindowPtr w)

{
	EditInfo			**e;				/* handle to edit info */
	short				i;					/* index of field at top of window */
	short				j;					/* loop index */
	FieldInfo		*f;				/* pointer to field info */
	TEHandle			edit;				/* handle to TextEdit record */
	short				top;				/* top coord of cur field */
	short				dvi;				/* change in v coord of field i */
	short				dv;				/* increase or decrease in field height */
	short				offset;			/* amount to offset field */
	Rect				r;					/* rectangle to invalidate */
	short				numFields;		/* number of fields */
	Boolean			showFieldInfo;	/* true to show full field info */

	e = GetEditInfo(w);
	showFieldInfo = (**e).showFieldInfo = !(**e).showFieldInfo;
	SetCtlValue((**e).showFieldInfoBox, showFieldInfo ? 1 : 0);
	numFields = (**e).numFields;
	HLock((Handle)(**e).fields);
	i = (**e).curField;
	f = &(*(**e).fields)[i];
	edit = f->edit;
	top = (**edit).viewRect.top - (showFieldInfo ? infoHeight : 0);
	dvi = headerHeight - top;
	if (dvi) {
		OffsetRect(&(**edit).viewRect, 0, dvi);
		OffsetRect(&(**edit).destRect, 0, dvi);
	}
	dv = showFieldInfo ? -infoHeight : infoHeight;
	offset = dvi + dv;
	for (j = i-1; j >= 0; j--) {
		f = &(*(**e).fields)[j];
		edit = f->edit;
		OffsetRect(&(**edit).viewRect, 0, offset);
		OffsetRect(&(**edit).destRect, 0, offset);
		offset += dv;
	}
	dv = -dv;
	offset = dvi + dv;
	for (j = i+1; j < numFields; j++) {
		f = &(*(**e).fields)[j];
		edit = f->edit;
		OffsetRect(&(**edit).viewRect, 0, offset);
		OffsetRect(&(**edit).destRect, 0, offset);
		offset += dv;
	}
	HUnlock((Handle)(**e).fields);
	r = w->portRect;
	r.top += headerHeight;
	InvalRect(&r);
	ChangeSize(w, r.bottom);
	SetCtlValue((**e).scrollBar, headerHeight - TextTop(e));
	MakeIbeamRgn(w);
}

/*_____________________________________________________________________

	ChangeCurField - Change Current Active Field
	
	Entry:	e = handle to edit info.
				i = index of new current field.
_____________________________________________________________________*/

static void ChangeCurField (EditInfo **e, short i)

{
	FieldInfo		*f;			/* pointer to field info */
	TEHandle			edit;			/* handle to TextEdit record */
	
	SetClip((**e).clipRgn);
	HLock((Handle)(**e).fields);
	f = &(*(**e).fields)[(**e).curField];
	edit = f->edit;
	TESetSelect(0, 0, edit);
	TEDeactivate(edit);
	(**e).curField = i;
	f = &(*(**e).fields)[i];
	edit = f->edit;
	TEActivate(edit);
	SetClip(BigClipRgn);
	HiliteControl((**e).revertButton, f->dirty ? 0 : 255);
	HUnlock((Handle)(**e).fields);
}

/*_____________________________________________________________________

	SetWindowTitle - Set Window Title.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

static void SetWindowTitle (WindowPtr w)

{
	EditInfo			**e;			/* handle to edit info */
	Str255			alias;		/* alias */
	short				strIndex;	/* index of template string */
	Str255			tmpl;			/* window title template */
	Str255			ttl;			/* new window title */
	Str255			oldttl;		/* old window title */
	
	if (!w) return;
	e = GetEditInfo(w);
	utl_CopyPString(alias, (**e).alias);
	if (!LoggedIn || w != LoginWindow) {
		strIndex = wTitleRegular;
	} else if (Hero) {
		strIndex = wTitleHero;
	} else {
		strIndex = wTitleLogin;
	}
	GetIndString(tmpl, stringsID, strIndex);
	utl_PlugParams(tmpl, ttl, (**e).dirty ? "\p� " : "\p", alias, nil, nil);
	GetWTitle(w, oldttl);
	if (!EqualString(oldttl, ttl, true, true)) SetWTitle(w, ttl);
}

/*_____________________________________________________________________

	ChangeDirtyState - Change Dirty Window State.
	
	Entry:	w = pointer to window record.
				dirty = true if window is dirty.
				
	Exit:		Dirty flag set in edit info record.
				Revert All Fields button hilighted.
				Window title adjusted.
_____________________________________________________________________*/

static void ChangeDirtyState (WindowPtr w, Boolean dirty)

{
	EditInfo			**e;			/* handle to edit info */
	
	e = GetEditInfo(w);
	if ((**e).dirty == dirty) return;
	(**e).dirty = dirty;
	HiliteControl((**e).revertAllButton, dirty ? 0 : 255);
	SetWindowTitle(w);
}

/*_____________________________________________________________________

	ProcessFieldChange - Process Changed Field
	
	Entry:	w = pointer to window record.
				dirty = true if field should be marked dirty.
_____________________________________________________________________*/

static void ProcessFieldChange (WindowPtr w, Boolean dirty)

{
	EditInfo			**e;			/* handle to edit info */
	FieldInfo		*f;			/* pointer to field info */
	TEHandle			edit;			/* handle to TextEdit record */
	short				nLines;		/* number of lines in field */
	Handle			hText;		/* handle to field text */
	short				dv;			/* change in field height */
	short				i;				/* loop index */
	FieldInfo		*q;			/* pointer to field info */
	TEHandle			qEdit;		/* handle to TextEdit record */
	Rect				r;				/* rectangle to invalidate */
	short				top;			/* first field top coord */
	short				bot;			/* last field bot coord */
	Rect				viewRect;	/* view rectangle */
	short				numFields;	/* number of fields */
	short				curField;	/* current field number */

	e = GetEditInfo(w);
	curField = (**e).curField;
	HLock((Handle)(**e).fields);
	f = &(*(**e).fields)[curField];
	edit = f->edit;
	GetViewRect(w, &viewRect);
	if (dirty && !f->dirty) {
		f->dirty = true;
		SetClip((**e).clipRgn);
		r.left = TextLeft - Width;
		r.right = TextLeft;
		r.top = (**edit).viewRect.top;
		r.bottom = r.top + LineHeight;
		EraseRect(&r);
		MoveTo(r.left, r.top + Leading + Ascent);
		DrawChar('�');
		SetClip(BigClipRgn);
		HiliteControl((**e).revertButton, 0);
		ChangeDirtyState(w, true);
	}
	nLines = (**edit).nLines;
	if (!nLines) {
		nLines = 1;
	} else {
		hText = (**edit).hText;
		if (*(*hText + (**edit).teLength - 1) == '\n') nLines++;
	}
	if (nLines != f->nLines) {
		dv = (nLines - f->nLines)*LineHeight;
		(**edit).viewRect.bottom += dv;
		(**edit).destRect.bottom += dv;
		numFields = (**e).numFields;
		for (i = curField + 1; i < numFields; i++) {
			q = &(*(**e).fields)[i];
			qEdit = q->edit;
			OffsetRect(&(**qEdit).viewRect, 0, dv);
			OffsetRect(&(**qEdit).destRect, 0, dv);
		}
		AdjustScrollMax(w);
		MakeIbeamRgn(w);
		bot = TextBot(e);
		if (bot < viewRect.bottom) {
			dv = viewRect.bottom-bot;
			top = TextTop(e);
			if (top + dv > headerHeight) dv = headerHeight - top;
			if (dv) Scroll(w, dv);
		}
		AutoScroll(w, curField);
		r = viewRect;
		r.top = (**edit).viewRect.top;
		InvalRect(&r);
		f->nLines = nLines;
	}
	HUnlock((Handle)(**e).fields);
}

/*_____________________________________________________________________

	Revert - Revert Current Field to Original Value.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

static void Revert (WindowPtr w)

{
	EditInfo			**e;			/* handle to edit info */
	FieldInfo		*f;			/* pointer to field info */
	TEHandle			edit;			/* handle to TextEdit record */
	short				i;				/* loop index */
	Rect				r;				/* rectangle to erase */
	short				numFields;	/* number of fields */
	Boolean			newDirty;	/* new window dirty flag */

	e = GetEditInfo(w);
	HLock((Handle)(**e).fields);
	f = &(*(**e).fields)[(**e).curField];
	edit = f->edit;
	SetClip((**e).clipRgn);
	HLock(f->original);
	TESetText(*f->original, f->origSize, edit);
	HUnlock(f->original);
	InvalRect(&(**edit).viewRect);
	TECalText(edit);
	SetClip(BigClipRgn);
	ProcessFieldChange(w, false);
	f->dirty = false;
	newDirty = false;
	numFields = (**e).numFields;
	for (i = 0; i < numFields; i++) {
		if ((*(**e).fields)[i].dirty) {
			newDirty = true;
			break;
		}
	}
	SetClip((**e).clipRgn);
	r.left = TextLeft - Width;
	r.right = TextLeft;
	r.top = (**edit).viewRect.top;
	r.bottom = r.top + LineHeight;
	EraseRect(&r);
	if (!f->hasChangeAttr && f->canEdit) {
		MoveTo(r.left, r.top + Leading + Ascent);
		DrawChar('�');
	}
	SetClip(BigClipRgn);
	HiliteControl((**e).revertButton, 255);
	ChangeDirtyState(w, newDirty);
	AutoScroll(w, (**e).curField);
	HUnlock((Handle)(**e).fields);
}

/*_____________________________________________________________________

	RevertAll - Revert All Fields to Original Values.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

static void RevertAll (WindowPtr w)

{
	EditInfo			**e;			/* handle to edit info */
	short				i;						/* loop index */
	short				oldCurField;		/* saved current field */
	short				numFields;	/* number of fields */

	e = GetEditInfo(w);
	oldCurField = (**e).curField;
	numFields = (**e).numFields;
	for (i = 0; i < numFields; i++) {
		if ((*(**e).fields)[i].dirty) {
			ChangeCurField(e, i);
			Revert(w);
		}
	}
	ChangeCurField(e, oldCurField);
}

/*_____________________________________________________________________

	FieldTooBig - Check for Field Too Big.
	
	Entry:	e = handle to edit info.
				n = number of characters to be inserted into current field.
	
	Exit:		Function result = true if field too big after insertion.
_____________________________________________________________________*/

static Boolean FieldTooBig (EditInfo **e, short n)

{
	FieldInfo			*f;			/* pointer to field info */
	TEHandle				edit;			/* handle to TextEdit record */
	short					nSel;			/* selection size */
	short					teLength;	/* field size */
	char					lenStr[20];	/* max field size as a string */
	Boolean				tooBig;		/* true if fields is too big */

	HLock((Handle)(**e).fields);
	f = &(*(**e).fields)[(**e).curField];
	edit = f->edit;
	nSel = (**edit).selEnd - (**edit).selStart;
	teLength = (**edit).teLength;
	if (tooBig = (teLength - nSel + n > f->maxSize)) {
		NumToString(f->maxSize, lenStr);
		glob_Error(servErrors, msgFieldTooBig, lenStr);
	}
	HUnlock((Handle)(**e).fields);
	return tooBig;
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

	DoSave - Send Field Changes to Server.
	
	Entry:	w = pointer to window record.
	
	Exit:		function result = true if save operation cancelled by user or
					all the fields were not updated because of some other error.
_____________________________________________________________________*/

static Boolean DoSave (WindowPtr w)

{
	EditInfo			**e;				/* handle to edit info */
	short				i;					/* loop index */
	FieldInfo		*f;				/* pointer to field info */
	TEHandle			edit;				/* handle to TextEdit record */
	short				teLength;		/* length of text */
	Rect				viewRect;		/* view rect */
	OSErr				rCode;			/* result code */
	short				numFields;		/* number of fields */
	Str255			alias;			/* alias */
	Str255			server;			/* login server */
	Str255			loginAlias;		/* login alias */
	Str255			pswd;				/* login password */
	WindowPtr		v;					/* pointer to traverse window list */
	EditInfo			**g;				/* handle to edit info for window v */
	Boolean			aliasChanged;	/* trued if alias was changed */
	Boolean			someNotPut;		/* true if some fields not put ok */
	short				sCode;			/* server response code */

	e = GetEditInfo(w);
	numFields = (**e).numFields;
	i = (**e).aliasFieldIndex;
	f = &(*(**e).fields)[i];
	edit = f->edit;
	if (!(**edit).teLength) {
		glob_Error(servErrors, msgSaveNoAlias, nil);
		return true;
	}
	utl_CopyPString(alias, (**e).alias);
	utl_CopyPString(server, (**e).server);
	utl_CopyPString(loginAlias, (**e).loginAlias);
	utl_CopyPString(pswd, (**e).pswd);
	for (i = 0; i < numFields; i++) {
		f = &(*(**e).fields)[i];
		f->putOK = false;
		f->servErrMsg = nil;
	}
	rCode = serv_PutRecord(server, loginAlias, pswd, alias, (**e).fields, 
		numFields, &sCode);
	if (rCode) {
		glob_ErrorCode(rCode);
	} else if (sCode != phSuccess) {
		glob_ServErrorCode(sCode);
	}
	SetPort(w);
	aliasChanged = someNotPut = false;
	HLock((Handle)(**e).fields);
	for (i = 0; i < numFields; i++) {
		f = &(*(**e).fields)[i];
		if (!f->dirty) continue;
		if (f->putOK) {
			f->dirty = false;
			edit = f->edit;
			teLength = (**edit).teLength;
			SetHandleSize(f->original, teLength);
			memcpy(*f->original, *(**edit).hText, teLength);
			f->origSize = teLength;
			if (i == (**e).aliasFieldIndex) {
				*alias = teLength;
				memcpy(alias+1, *f->original, teLength);
				aliasChanged = true;
			}
			if (i == (**e).curField) HiliteControl((**e).revertButton, 255);
		} else {
			someNotPut = true;
		}
	}
	HUnlock((Handle)(**e).fields);
	if (aliasChanged) {
		v = FrontWindow();
		while (v) {
			if (oop_GetWindKind(v) == editWind) {
				g = GetEditInfo(v);
				if (EqualString((**g).server, (**e).server, true, true) && 
					EqualString((**g).loginAlias, (**e).alias, true, true)) {
					utl_CopyPString((**g).loginAlias, alias);
				}
			}
			v = (WindowPtr)((WindowPeek)v)->nextWindow;
		}
		if (EqualString(server, Server, true, true) &&
			EqualString((**e).alias, Alias, true, true))
			utl_CopyPString(Alias, alias);
		utl_CopyPString((**e).alias, alias);
	}
	SetWindowTitle(w);
	GetViewRect(w, &viewRect);
	InvalRect(&viewRect);
	oop_UpdateAll();
	if (someNotPut) {
		HLock((Handle)(**e).fields);
		for (i = 0; i < numFields; i++) {
			f = &(*(**e).fields)[i];
			if (f->putOK) continue;
			if (f->servErrMsg) {
				HLock(f->servErrMsg);
				glob_BringToFront();
				utl_ErrorAlertRez(servErrors, msgBadVal, errorMsgID,
					oop_ModalUpdate,
					f->name, *(f->servErrMsg), nil, nil);
				DisposHandle(f->servErrMsg);
				SetPort(w);
			}
		}
		HUnlock((Handle)(**e).fields);
	} else {
		ChangeDirtyState(w, false);
	}
	return someNotPut;
}

/*_____________________________________________________________________

	AlertIfDirty - Ask User if He Wants to Save Changes.
	
	Entry:	w = pointer to window record.
				index = index of "quitting" or "closing window" string.
	
	Exit:		Function result = true if alert canceled.
_____________________________________________________________________*/

static Boolean AlertIfDirty (WindowPtr w, short index)

{
	EditInfo			**e;				/* handle to edit info */
	Str255			alias;			/* alias */
	Str255			str;				/* "quitting" or "closing window" */

	e = GetEditInfo(w);
	if (!(**e).dirty) return false;
	utl_CopyPString(alias, (**e).alias);
	GetIndString(str, stringsID, index);
	glob_BringToFront();
	oop_UpdateAll();
	switch (utl_SaveChangesAlertRez(stringsID, msgSaveChanges, saveChangesID,
		oop_ModalUpdate, alias, str, nil, nil)) {
		case saveSave:
			oop_UpdateAll();
			return DoSave(w);
		case saveCancel:
			return true;
		case saveDont:
			return false;
	}
}

/*_____________________________________________________________________

	DisposeFields - Dispose Fields Array.
	
	Entry:	fields = handle to fields array.
				numFields = number of fields.
				intialized = true if field intitialization has been completed.
_____________________________________________________________________*/

static void DisposeFields (FieldInfo **fields, short numFields, Boolean initialized)

{
	short				i;				/* loop index */
	FieldInfo		*f;			/* pointer to field info */
	
	HLock((Handle)fields);
	for (i = 0; i < numFields; i++) {
		f = &(*fields)[i];
		DisposHandle(f->attributes);
		DisposHandle(f->description);
		DisposHandle(f->original);
		if (initialized) TEDispose(f->edit);
	}
	DisposHandle((Handle)fields);
}

/*_____________________________________________________________________

	CloseEditWindow - Close Window.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

static void CloseEditWindow (WindowPtr w)

{
	EditInfo			**e;			/* handle to edit info */
	short				windNum;		/* window number */
	
	if (w == LoginWindow) LoginWindow = nil;
	e = GetEditInfo(w);
	windNum = (**e).windNum;
	if (windNum <= numPosSave) wstm_Save(w, &EditStates[windNum]);
	DisposeFields((**e).fields, (**e).numFields, true);
	DisposeRgn((**e).clipRgn);
	DisposHandle((Handle)e);
	oop_DoClose(w);
}

/*_____________________________________________________________________

	NewEditWindow - Create New Edit Window.
	
	Entry:	fields = handle to field info.
				numFields = number of fields.
				alias = alias.
_____________________________________________________________________*/

static void NewEditWindow (FieldInfo **fields, short numFields, Str255 alias)
	
{
	WindowPtr			v;						/* traverses window list */
	EditInfo				**g;					/* handle to edit info for window v */
	Boolean				haveWindNum;		/* true when window number found */
	short					windNum;				/* window number */
	WindState			windState;			/* window state record */
	EditInfo				**e;					/* handle to edit info */
	Rect					viewRect;			/* view rectangle */
	WindowPtr			w;						/* pointer to window */
	
	/* Create the new window record */

	windNum = 1;
	haveWindNum = false;
	while (true) {
		v = FrontWindow();
		if (!v) break;
		while (v) {
			if (oop_GetWindKind(v) == editWind) {
				g = GetEditInfo(v);
				if ((**g).windNum == windNum) break;
			}
			v = (WindowPtr)((WindowPeek)v)->nextWindow;
			if (!v) haveWindNum = true;
		}
		if (haveWindNum) break;
		windNum++;
	}
	windState.moved = false;
	w = wstm_Restore(false, editWindID, nil, 
		windNum <= numPosSave ? &EditStates[windNum] : &windState);
	SetPort(w);
	TextFont(FontNum);
	TextSize(fontSize);
	
	/* Create the new EditInfo record. */
	
	e = (EditInfo**)NewHandle(sizeof(EditInfo));
	(**e).fields = fields;
	(**e).numFields = numFields;
	(**e).curField = 0;
	(**e).aliasFieldIndex = GetAliasFieldIndex((**e).fields, (**e).numFields);
	(**e).scrollBar = GetNewControl(sBarID, w);
	(**e).showFieldInfoBox = GetNewControl(showInfoID, w);
	(**e).revertButton = GetNewControl(revertID, w);
	HiliteControl((**e).revertButton, 255);
	(**e).revertAllButton = GetNewControl(revertAllID, w);
	HiliteControl((**e).revertAllButton, 255);
	(**e).showFieldInfo = false;
	(**e).dirty = false;
	utl_CopyPString((**e).alias, alias);
	utl_CopyPString((**e).server, Server);
	utl_CopyPString((**e).loginAlias, Alias);
	utl_CopyPString((**e).pswd, Password);
	(**e).windNum = windNum;
	
	/* Create the new window object. */
	
	oop_NewWindow(w, editWind, (Handle)e, &dispatch);
	
	/* Finish initializing and show the window. */
	
	CompleteFieldInitialization(e);
	MakeIbeamRgn(w);
	GetViewRect(w, &viewRect);
	(**e).clipRgn = NewRgn();
	RectRgn((**e).clipRgn, &viewRect);
	ChangeSize(w, w->portRect.bottom);
	AdjustScrollMax(w);
	InvalRect(&w->portRect);
	ShowWindow(w);	
	
	return;
}

/*_____________________________________________________________________

	AlreadyOpen - Check to See if Record is Already Open.
	
	Entry:	server = server.
				alias = alias.
				
	Exit:		function result = pointer to window record, or nil if none.
_____________________________________________________________________*/

static WindowPtr AlreadyOpen (Str255 server, Str255 alias)
	
{
	WindowPtr		v;						/* pointer to traverse window list */
	EditInfo			**g;					/* handle to edit info */
	
	v = FrontWindow();
	while (v) {
		if (oop_GetWindKind(v) == editWind) {
			g = GetEditInfo(v);
			if (EqualString((**g).server, server, true, true) &&
				EqualString((**g).alias, alias, true, true)) {
				return v;
			}
		}
		v = (WindowPtr)((WindowPeek)v)->nextWindow;
	}
	return nil;
}

/*_____________________________________________________________________

	GetRecord - Get a Ph Record and Open an Editing Window.
	
	Entry:	user = alias or username.
	
	Exit:		function result = true if record opened ok.
_____________________________________________________________________*/

static Boolean GetRecord (Str255 user)

{
	OSErr					rCode;				/* result code */
	Str255				alias;				/* alias for new record */
	FieldInfo			**fields;			/* handle to field info */
	short					numFields;			/* number of fields */
	short					sCode;				/* server response code */
	short					i;						/* index of field */
	FieldInfo			*f;					/* pointer to field info */
	Boolean				canPut;				/* true if login user permitted to edit
														this record */
	short					proxyLen;			/* length of proxy field */
	short					aliasLen;			/* length of login alias */
	short					j;						/* loop index */
	WindowPtr			w;						/* ptr to window if already open */

	fields = nil;
	if (rCode = serv_GetRecord(Server, Alias, Password, user,
		&fields, &numFields, &sCode)) {
		glob_ErrorCode(rCode);
		if (fields) DisposeFields(fields, numFields, false);
		return false;
	}
	if (sCode != phSuccess) {
		glob_ServErrorCode(sCode);
		if (fields) DisposeFields(fields, numFields, false);
		return false;
	}
	i = GetAliasFieldIndex(fields, numFields);
	f = &(*fields)[i];
	if (!f->origSize) {
		glob_Error (servErrors, msgOpenNoAlias, nil);
		DisposeFields(fields, numFields, false);
		return false;
	}
	*alias = f->origSize;
	memcpy(alias+1, *f->original, f->origSize);
	if (w = AlreadyOpen(Server, alias)) {
		if (fields) DisposeFields(fields, numFields, false);
		SelectWindow(w);
		return true;
	}
	if (!Hero && !EqualString(alias, Alias, true, true)) {
		i = GetProxyFieldIndex(fields, numFields);
		f = &(*fields)[i];
		canPut = false;
		proxyLen = f->origSize;
		aliasLen = *Alias;
		for (j = 0; j <= proxyLen-aliasLen; j++) {
			if (!strncmp(Alias+1, *f->original+j, aliasLen)) {
				canPut = true;
				break;
			}
		}
		if (!canPut) {
			glob_Error(servErrors, msgNotAuth, user);
			if (fields) DisposeFields(fields, numFields, false);
			return false;
		}
	}
	NewEditWindow(fields, numFields, alias);
	SetWindowTitle(FrontWindow());
	return true;
}

/*_____________________________________________________________________

	DoOpen - Open a Ph Record
_____________________________________________________________________*/

static void DoOpen (void)

{
	Str255			user;			/* username to open */
	
	HiliteMenu(0);
	*user = 0;
	while (true) {
		if (open_DoDialog(user)) return;
		if (!*user) {
			glob_Error(servErrors, msgNoAliasIdName, nil);
			continue;
		}
		if (GetRecord(user)) break;
	}
}

/*_____________________________________________________________________

	BuildProxyMenu - Build Proxy Menu.
	
	Entry:		proxyList = handle to proxy list (a sequence of Pascal
						strings).
					proxyCode = server response code to proxy query.
_____________________________________________________________________*/

static void BuildProxyMenu (Handle proxyList, short proxyCode)

{
	short			nItems;		/* number of items in menu */
	short			i;				/* loop index */
	char			*p;			/* pointer into proxy list */
	char			*pEnd;		/* pointer to end of proxy list */
	short			item;			/* menu item number */
	Str255		ttl;			/* menu command */

	Proxy = proxyList && proxyCode == phSuccess;
	nItems = CountMItems(ProxyMenu);
	for (i = nItems; i > 0; i--) DelMenuItem(ProxyMenu, i);
	if (Proxy) {
		HLock(proxyList);
		p = *proxyList;
		pEnd = p + GetHandleSize(proxyList);
		item = 0;
		while (p < pEnd) {
			if (*p) {
				item++;
				AppendMenu(ProxyMenu, "\p ");
				SetItem(ProxyMenu, item, p);
			}
			p += *p+1;
		}
	} else {
		GetIndString(ttl, stringsID, whyCmd);
		AppendMenu(ProxyMenu, ttl);
	}
	DisposHandle(proxyList);
}

/*_____________________________________________________________________

	DoNew - Create a New Ph Record.
_____________________________________________________________________*/

static void DoNew (void)

{
	OSErr					rCode;				/* result code */
	Str255				alias;				/* alias to create */
	Str255				name;					/* name field */
	Str255				type;					/* type field */
	Str255				pswd1;					/* pswd for new record */
	Str255				pswd2;				/* second pswd for new record */
	FieldInfo			**fields;			/* handle to field info */
	short					numFields;			/* number of fields */
	short					sCode;				/* server response code */
	short					whichField;			/* field to hilite in dialog =
														field containing illegal value */
	Str255				servErrMsg;			/* server error message */
	Str255				fName;				/* illegal field name */
	short					inx;					/* index in field names rsrc of field name */
	
	HiliteMenu(0);
	*alias = *name = *type = *pswd1 = *pswd2 = whichField = 0;
	while (true) {
		if (new_DoDialog(alias, name, type, pswd1, pswd2, whichField)) return;
		if (!*alias) {
			glob_Error(servErrors, msgNoAlias, nil);
			continue;
		}
		if (!EqualString(pswd1, pswd2, true, true)) {
			glob_Error(servErrors, msgPswdNotEqual, nil);
			continue;
		}
		fields = nil;
		if (rCode = serv_CreateRecord(Server, Alias, Password, alias, 
			name, type, pswd1, &fields, &numFields, &sCode, &whichField, 
			servErrMsg)) {
			glob_ErrorCode(rCode);
			if (fields) DisposeFields(fields, numFields, false);
			continue;
		}
		if (sCode != phSuccess) {
			if (whichField >= 0) {
				switch (whichField) {
					case 0: inx = aliasFieldName; break;
					case 1: inx = nameFieldName; break;
					case 2: inx = typeFieldName; break;
					case 3: inx = pswdFieldName; break;
				}
				GetIndString(fName, fieldNames, inx);
				glob_BringToFront();
				utl_ErrorAlertRez(servErrors, msgBadVal, errorMsgID,
					oop_ModalUpdate,
					fName, servErrMsg, nil, nil);
			} else {
				glob_ServErrorCode(sCode);
			}
			if (fields) DisposeFields(fields, numFields, false);
			continue;
		}
		break;
	}
	NewEditWindow(fields, numFields, alias);
	SetWindowTitle(FrontWindow());
}

/*_____________________________________________________________________

	DoLogin - Login to Server.
_____________________________________________________________________*/

static void DoLogin (void)

{
	OSErr					rCode;				/* result code */
	Str255				server;				/* server */
	Str255				user;					/* alias or username */
	Str255				pswd;					/* password */
	Str255				loginAlias;			/* login alias */
	FieldInfo			**fields;			/* handle to field info */
	short					numFields;			/* number of fields */
	Handle				proxyList;			/* handle to proxy list */
	short					proxyCode;			/* server response code to proxy query */
	short					sCode;				/* server response code */
	WindowPtr			w;						/* ptr to login window */

	HiliteMenu(0);
	utl_CopyPString(server, DefaultServer);
	*user = *pswd = 0;
	while (true) {
		if (login_DoDialog(server, user, pswd)) return;
		if (!*server) {
			glob_Error(servErrors, msgNoServer, nil);
			continue;
		}
		if (!*user) {
			glob_Error(servErrors, msgNoAliasName, nil);
			continue;
		}
		if (!*pswd) {
			glob_Error(servErrors, msgNoPswd, nil);
			continue;
		}
		proxyList = nil;
		fields = nil;
		if (rCode = serv_Login(server, user, pswd, loginAlias,
			&fields, &numFields, &sCode, &proxyList, &proxyCode)) {
			glob_ErrorCode(rCode);
			if (proxyList) DisposHandle(proxyList);
			if (fields) DisposeFields(fields, numFields, false);
			continue;
		}
		if (sCode != phSuccess) {
			glob_ServErrorCode(sCode);
			if (proxyList) DisposHandle(proxyList);
			if (fields) DisposeFields(fields, numFields, false);
			continue;
		}
		break;
	}
	utl_CopyPString(Server, server);
	utl_CopyPString(Alias, loginAlias);
	utl_CopyPString(Password, pswd);
	Hero = GetHero(fields, numFields);
	if (LoggedIn) {
		LoggedIn = false;
		SetWindowTitle(LoginWindow);
	}
	LoggedIn = true;
	if (w = AlreadyOpen(server, loginAlias)) {
		if (fields) DisposeFields(fields, numFields, false);
		SelectWindow(w);
	} else {
		NewEditWindow(fields, numFields, loginAlias);
		w = FrontWindow();
	}
	LoginWindow = w;
	SetWindowTitle(w);
	BuildProxyMenu(proxyList, proxyCode);
}

/*_____________________________________________________________________

	DoLogout - Logout.
_____________________________________________________________________*/

static void DoLogout (void)

{
	WindowPtr			v;			/* pointer to traverse window list. */
	
	v = FrontWindow();
	while (v) {
		if (oop_GetWindKind(v) == editWind) {
			if (AlertIfDirty(v, msgClosing)) return;
			CloseEditWindow(v);
		}
		v = (WindowPtr)((WindowPeek)v)->nextWindow;
	}
	LoggedIn = false;
	LoginWindow = nil;
	Hero = false;
	memset(Server, 0, 256);
	memset(Alias, 0, 256);
	memset(Password, 0, 256);
}

/*_____________________________________________________________________

	DoChangePassword - Change Server Password.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

static void DoChangePassword (WindowPtr w)

{
	EditInfo			**e;					/* handle to edit info */
	OSErr				rCode;				/* result code */
	Str255			alias;				/* alias of record to change */
	Str255			pswd1;				/* first password */
	Str255			pswd2;				/* second password */
	Str255			server;				/* login server */
	Str255			loginAlias;			/* login alias */
	Str255			loginPswd;			/* login password */
	WindowPtr		v;						/* pointer to traverse window list */
	EditInfo			**g;					/* handle to edit info for window v */
	short				sCode;				/* server response code */
	Str255			servErrMsg;			/* server error message */
	Boolean			emptyOK;				/* true if empty password is OK */

	HiliteMenu(0);
	e = GetEditInfo(w);
	utl_CopyPString(alias, (**e).alias);
	utl_CopyPString(server, (**e).server);
	utl_CopyPString(loginAlias, (**e).loginAlias);
	utl_CopyPString(loginPswd, (**e).pswd);
	emptyOK = false;
	if (Hero) {
		emptyOK = true;
		v = FrontWindow();
		while (v) {
			if (oop_GetWindKind(v) == editWind) {
				g = GetEditInfo(v);
				if (EqualString((**g).server, server, true, true) &&
					EqualString((**g).loginAlias, alias, true, true)) {
					emptyOK = false;
					break;
				}
			}
			v = (WindowPtr)((WindowPeek)v)->nextWindow;
		}
		if (emptyOK && EqualString(server, Server, true, true) &&
			EqualString(alias, Alias, true, true)) emptyOK = false;
	}
	*pswd1 = *pswd2 = 0;
	while (true) {
		if (pswd_DoDialog(pswd1, pswd2)) return;
		if (!emptyOK && !*pswd1 && !*pswd2) {
			glob_Error(servErrors, msgNoPswd, nil);
			continue;
		}
		if (!EqualString(pswd1, pswd2, true, true)) {
			glob_Error(servErrors, msgPswdNotEqual, nil);
			continue;
		}
		break;
	}
	rCode = serv_ChangePassword(server, loginAlias, loginPswd, alias, pswd1,
		&sCode, servErrMsg);
	SetPort(w);
	if (rCode) {
		glob_ErrorCode(rCode);
		return;
	}
	if (sCode != phSuccess) {
		if (*servErrMsg) {
			glob_Error(servErrors, msgBadPswd, servErrMsg);
		} else {
			glob_ServErrorCode(sCode);
		}
		return;
	}
	v = FrontWindow();
	while (v) {
		if (oop_GetWindKind(v) == editWind) {
			g = GetEditInfo(v);
			if (EqualString((**g).server, server, true, true) && 
				EqualString((**g).loginAlias, alias, true, true)) {
				utl_CopyPString((**g).pswd, pswd1);
			}
		}
		v = (WindowPtr)((WindowPeek)v)->nextWindow;
	}
	if (EqualString(server, Server, true, true) &&
		EqualString(alias, Alias, true, true))
		utl_CopyPString(Password, pswd1);
}

/*_____________________________________________________________________

	DoDelete - Delete a Ph Record.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

static void DoDelete (WindowPtr w)

{
	EditInfo			**e;					/* handle to edit info */
	Str255			alias;				/* alias to delete */
	OSErr				rCode;				/* result code */
	short				sCode;				/* server response code */
	Boolean			canDelete;			/* true if record can be deleted */
	WindowPtr		v;						/* traverses window list */
	EditInfo			**g;					/* handle to edit info for window v */

	e = GetEditInfo(w);
	utl_CopyPString(alias, (**e).alias);
	canDelete = !EqualString(Alias, alias, true, true) ||
		!EqualString(Server, (**e).server, true, true);
	if (canDelete) {
		v = (WindowPtr)((WindowPeek)w)->nextWindow;
		while (v) {
			if (oop_GetWindKind(v) == editWind) {
				g = GetEditInfo(v);
				if (EqualString((**g).server, (**e).server, true, true) &&
					EqualString((**g).loginAlias, alias, true, true)) {
					canDelete = false;
					break;
				}
			}
			v = (WindowPtr)((WindowPeek)v)->nextWindow;
		}
	}
	if (!canDelete) {
		glob_Error(stringsID, msgCantDelete, alias);
		return;
	}
	glob_BringToFront();
	if (deleteCancel == utl_TellMeTwiceAlertRez(stringsID, msgDelete, deleteID,
		oop_ModalUpdate, alias, nil, nil, nil)) return;
	if (rCode = serv_DeleteRecord(Server, Alias, Password, alias, &sCode)) {
		glob_ErrorCode(rCode);
		return;
	}
	if (sCode != phSuccess) {
		glob_ServErrorCode(sCode);
		return;
	}
	CloseEditWindow(w);
}

/*_____________________________________________________________________

	edit_DoPeriodic - Do Periodic Tasks.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void edit_DoPeriodic (WindowPtr w)

{
	EditInfo				**e;					/* handle to edit info */
	Point					where;				/* current mouse loc in local coords */
	Rect					viewRect;			/* view rectangle */
	RgnHandle			ibeamRgn;			/* ibeam region */
	ControlHandle		scrollBar;			/* handle to scroll bar control */
	ControlHandle		showFieldInfoBox;	/* handle to show field info checkbox */
	ControlHandle		revertButton;		/* handle to revert field button */
	ControlHandle		revertAllButton;	/* handle to revert all fields button */
	short					newBalloon;			/* index in STR# of balloon help msg, or 0 if
														none */
	Rect					hotRect;				/* balloon help hot rectangle */
	Point					tip;					/* balloon help tip location */
	HMMessageRecord	helpMsg;				/* help message record */
	
	static short		oldBalloon = 0;	/* index in STR# of previous balloon help msg,
														or 0 if none */

	e = GetEditInfo(w);
	SetPort(w);
	GetViewRect(w, &viewRect);
	SetClip((**e).clipRgn);
	TEIdle((*(**e).fields)[(**e).curField].edit);
	SetClip(BigClipRgn);
	GetMouse(&where);
	ibeamRgn = oop_GetWindIbeamRgn(w);
	SetCursor(PtInRect(where, &viewRect) && PtInRgn(where, ibeamRgn) ? 
		*IBeamHandle : &qd.arrow);
	if (HaveBalloons && HMGetBalloons()) {
		scrollBar = (**e).scrollBar;
		showFieldInfoBox = (**e).showFieldInfoBox;
		revertButton = (**e).revertButton;
		revertAllButton = (**e).revertAllButton;
		if (PtInRect(where, &viewRect)) {
			newBalloon = hbEditFields;
			hotRect = viewRect;
		} else if (PtInRect(where, &(**scrollBar).contrlRect)) {
			newBalloon = (**scrollBar).contrlMin < (**scrollBar).contrlMax ?
				hbScrollEnabled : hbScrollDisabled;
				hotRect = (**scrollBar).contrlRect;
		} else if (PtInRect(where, &(**showFieldInfoBox).contrlRect)) {
			newBalloon = (**showFieldInfoBox).contrlValue ? 
				hbEditInfoChk : hbEditInfoUnChk;
				hotRect = (**showFieldInfoBox).contrlRect;
		} else if (PtInRect(where, &(**revertButton).contrlRect)) {
			newBalloon = (**revertButton).contrlHilite ? 
				hbEditRvtDisabled : hbEditRvtEnabled;
				hotRect = (**revertButton).contrlRect;
		} else if (PtInRect(where, &(**revertAllButton).contrlRect)) {
			newBalloon = (**revertAllButton).contrlHilite ? 
				hbEditRvtAllDisabled : hbEditRvtAllEnabled;
				hotRect = (**revertAllButton).contrlRect;
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

	edit_DoClick - Process a Mouse Down Event.
	
	Entry:	w = pointer to window record.
				where = mouse click location in local coordinates.
				modifiers = modifiers from event record.
_____________________________________________________________________*/

void edit_DoClick (WindowPtr w, Point where, short modifiers)

{
	EditInfo			**e;				/* handle to edit info */
	ControlHandle	whichControl;	/* control handle returned from FindControl */
	short				oldVal;			/* old value of scroll bar */
	short				dv;				/* change in scroll bar value */
	short				partCode;		/* FindControl part code */
	short				i;					/* loop index */
	FieldInfo		*f;				/* pointer to field info */
	TEHandle			edit;				/* handle to TextEdit record */
	Rect				viewRect;		/* view rectangle */
	Rect				fieldRect;		/* field rectangle */
	short				numFields;		/* number of fields */

	e = GetEditInfo(w);
	partCode = FindControl(where, w, &whichControl);
	if (!whichControl) {
		GetViewRect(w, &viewRect);
		if (!PtInRect(where, &viewRect)) return;
		numFields = (**e).numFields;
		HLock((Handle)(**e).fields);
		for (i = 0; i < numFields; i++) {
			f = &(*(**e).fields)[i];
			if (!f->canEdit) continue;
			edit = f->edit;
			fieldRect = (**edit).viewRect;
			fieldRect.left -= textMargin;
			fieldRect.right += textMargin;
			if (!PtInRect(where, &fieldRect)) continue;
			if (i != (**e).curField) {
				ChangeCurField(e, i);
			}
			SetClip((**e).clipRgn);
			TEClick(where, modifiers & shiftKey ? true : false, edit);
			SetClip(BigClipRgn);
			return;
		}
		HUnlock((Handle)(**e).fields);
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
	} else {
		if (!TrackControl(whichControl, where, nil)) return;
		if (whichControl == (**e).showFieldInfoBox) {
			ToggleShowFieldInfo(w);
		} else if (whichControl == (**e).revertButton) {
			Revert(w);
		} else if (whichControl == (**e).revertAllButton) {
			RevertAll(w);
		}
	}
}

/*_____________________________________________________________________

	edit_DoKey - Process a Key Down Event.
	
	Entry:	w = pointer to window record.
				key = ascii code of key.
				modifiers = modifiers from event record.
_____________________________________________________________________*/

void edit_DoKey (WindowPtr w, char key, short modifiers)

{
	EditInfo			**e;				/* handle to edit info */
	TEHandle			edit;				/* handle to TextEdit record */
	short				i;					/* loop index */
	Boolean			legal;			/* true if legal char for this field */
	short				numFields;		/* number of fields */
	ControlHandle	scrollBar;		/* handle to scroll bar */
	short				val;				/* scroll bar value */
	short				min;				/* scroll bar minimum value */
	short				max;				/* scroll bar maximum value */

	e = GetEditInfo(w);
	numFields = (**e).numFields;
	SetPort(w);
	if (key == tabKey) {
		i = (**e).curField;
		if (modifiers & shiftKey) {
			while (true) {
				i--;
				if (i < 0) i = numFields - 1;
				if ((*(**e).fields)[i].canEdit) break;
			}
		} else {
			while (true) {
				i++;
				if (i >= numFields) i = 0;
				if ((*(**e).fields)[i].canEdit) break;
			}
		}
		edit = (*(**e).fields)[i].edit;
		TESetSelect(0, 0x7fff, edit);
		AutoScroll(w, i);
		ChangeCurField(e, i);
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
		legal = utl_StandardAsciiChar(key);
		if (legal || key == deleteKey || key == leftArrow ||
			key == rightArrow || key == upArrow || key == downArrow ||
			key == returnKey) {
			if ((legal || key == returnKey) && FieldTooBig(e, 1)) return;
			SetClip((**e).clipRgn);
			TEKey(key, (*(**e).fields)[(**e).curField].edit);
			SetClip(BigClipRgn);
			ProcessFieldChange(w, legal || key == deleteKey || key == returnKey);
			AutoScroll(w, (**e).curField);
			oop_UpdateAll();
		} else {
			glob_Error(servErrors, msgBadFieldChar, nil);
		}
	}
}

/*_____________________________________________________________________

	edit_DoUpdate - Process an Update Event.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void edit_DoUpdate (WindowPtr w)

{
	EditInfo		**e;					/* handle to edit info */
	Rect			viewRect;			/* view rectangle */
	short			i;						/* loop index */
	FieldInfo	*f;					/* pointer to field info */
	TEHandle		edit;					/* handle to TextEdit record */
	short			right;				/* coord of right window edge */
	char			str[80];				/* scratch string */
	short			fieldTop;			/* top coord of field */
	short			fieldBot;			/* bot coord of field */
	short			updateTop;			/* top coord of update region */
	short			updateBot;			/* bot coord of update region */
	short			v;						/* v coord */
	Rect			r;						/* rect to fill with light gray */
	short			numFields;			/* number of fields */
	Boolean		showFieldInfo;		/* true to show full field info */

	e = GetEditInfo(w);
	numFields = (**e).numFields;
	showFieldInfo = (**e).showFieldInfo;
	EraseRect(&w->portRect);
	DrawControls(w);
	GetIndString(str, stringsID, fieldChanged);
	right = w->portRect.right;
	MoveTo(right-textMargin-StringWidth(str), fChangedBase);
	DrawString(str);
	GetIndString(str, stringsID, cantChange);
	MoveTo(right-textMargin-StringWidth(str), cantChangeBase);
	DrawString(str);
	MoveTo(0, headerRule);
	Line(right, 0);
	MoveTo(0, headerRule+2);
	Line(right, 0);
	DrawGrowBox(w);
	GetViewRect(w, &viewRect);
	ClipRect(&viewRect);
	updateTop = (**w->visRgn).rgnBBox.top;
	updateBot = (**w->visRgn).rgnBBox.bottom;
	HLock((Handle)(**e).fields);
	for (i = 0; i < numFields; i++) {
		f = &(*(**e).fields)[i];
		edit = f->edit;
		fieldTop = (**edit).viewRect.top - (showFieldInfo ? infoHeight : 0);
		fieldBot = (**edit).viewRect.bottom + (showFieldInfo ? infoWhite1 : 0);
		if (fieldBot < updateTop || fieldTop > updateBot) continue;
		if (showFieldInfo) {
			v = fieldTop + infoWhite1 + 1;
			MoveTo(0, v);
			Line(right - sBarWidthM1, 0);
			v += infoWhite2 + LineHeight;
			MoveTo(textMargin, v);
			DrawString(f->name);
			DrawChar(':');
			Move(Width, 0);
			HLock(f->description);
			DrawString(*f->description);
			HUnlock(f->description);
			v += LineHeight;
			MoveTo(textMargin, v);
			HLock(f->attributes);
			DrawString(*f->attributes);
			HUnlock(f->attributes);
			if (**f->attributes) DrawString("\p, ");
			GetIndString(str, stringsID, maxSizeStr);
			DrawString(str);
			NumToString(f->maxSize, str);
			DrawString(str);
			v += infoWhite3 + 1;
			PenPat(qd.ltGray);
			MoveTo(0, v);
			Line(right - sBarWidthM1, 0);
			PenPat(qd.black);
			r = (**edit).viewRect;
			r.left = 0;
			r.right = NameRight + Width;
			r.top -= infoWhite4;
			r.bottom += infoWhite1 + 1;
			FillRect(&r, qd.ltGray);
		} else {
			v = fieldTop + Ascent + Leading;
			MoveTo(NameRight - StringWidth(f->name), v);
			DrawString(f->name);
			MoveTo(NameRight, v);
			DrawChar(':');
		}
		if (f->dirty || !f->hasChangeAttr) {
			MoveTo(TextLeft-Width, (**edit).viewRect.top + Ascent + Leading);
			if (f->dirty) {
				DrawChar('�');
			} else if (f->canEdit) {
				DrawChar('�');
			} else {
				DrawChar('�');
			}
		}
		TEUpdate(&viewRect, edit);
	}
	HUnlock((Handle)(**e).fields);
	SetClip(BigClipRgn);
}

/*_____________________________________________________________________

	edit_DoActivate - Process an Activate Event.	
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void edit_DoActivate (WindowPtr w)

{
	EditInfo		**e;			/* handle to edit info */
	
	e = GetEditInfo(w);
	TEActivate((*(**e).fields)[(**e).curField].edit);
	ShowControl((**e).scrollBar);
	DrawGrowBox(w);
}

/*_____________________________________________________________________

	edit_DoDeactivate - Process a Deactivate Event.	
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void edit_DoDeactivate (WindowPtr w)

{
	EditInfo		**e;			/* handle to edit info */
	
	e = GetEditInfo(w);
	TEDeactivate((*(**e).fields)[(**e).curField].edit);
	HideControl((**e).scrollBar);
	DrawGrowBox(w);
}

/*_____________________________________________________________________

	edit_DoGrow - Process Window Grow Operation.
	
	Entry:	w = pointer to window record.
				where = location of mouse click in global coordinates.
_____________________________________________________________________*/

void edit_DoGrow(WindowPtr w, Point where)

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

	edit_DoZoom - Process Window Zoom Operation.
	
	Entry:	w = pointer to window record.
				where = location of mouse click in global coordinates.
				partCode = inZoomIn or inZoomOut
_____________________________________________________________________*/

void edit_DoZoom (WindowPtr w, Point where, short partCode)

{
	if (!TrackBox(w, where, partCode)) return;
	SetPort(w);
	if (partCode == inZoomOut) wstm_ComputeStd(w);
	EraseRect(&w->portRect);
	ZoomWindow(w, partCode, false);
	ChangeSize(w, w->portRect.bottom - w->portRect.top);
}

/*_____________________________________________________________________

	edit_DoClose - Close Window.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void edit_DoClose (WindowPtr w)

{
	if (AlertIfDirty(w, msgClosing)) return;
	CloseEditWindow(w);
}

/*_____________________________________________________________________

	edit_DoCommand - Process a Command.
	
	Entry:	top = pointer to top window record.
				theMenu = menu number.
				theItem = menu item number.
_____________________________________________________________________*/

#pragma segment command

Boolean edit_DoCommand (WindowPtr top, short theMenu, short theItem)

{
	EditInfo			**e;			/* handle to edit info */
	TEHandle			textH;		/* handle to active TextEdit record */
	Handle			scrap;		/* handle to scrap to be pasted */
	short				scrapLen;	/* length of scrap to be pasted */
	char				*p;			/* pointer to scrap char */
	Str255			user;			/* proxy record to open */
	
	if (top) e = GetEditInfo(top);
	switch (theMenu) {
		case fileID:
			switch (theItem) {
				case newPhCmd:
					DoNew();
					return true;
				case openPhCmd:
					DoOpen();
					return true;
				case savePhCmd:
					if (top) DoSave(top);
					return true;
				case loginCmd:
					DoLogin();
					return true;
				case logoutCmd:
					DoLogout();
					return true;
				case paswdCmd:
					if (top) DoChangePassword(top);
					return true;
				case deletePhCmd:
					if (top) DoDelete(top);
					return true;
			}
			break;
		case editID:
			if (!top) break;
			textH = (*(**e).fields)[(**e).curField].edit;
			switch (theItem) {
				case cutCmd:
				case copyCmd:
					if (theItem == cutCmd) {
						SetClip((**e).clipRgn);
						TECut(textH);
						SetClip(BigClipRgn);
						ProcessFieldChange(top, true);
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
					if (FieldTooBig(e, scrapLen)) break;
					p = *scrap;
					while (scrapLen--) {
						if (!utl_StandardAsciiChar(*p) && *p != returnKey) {
							glob_Error(servErrors, msgBadFieldChar, nil);
							return true;
						};
						p++;
					}
					SetClip((**e).clipRgn);
					TEPaste(textH);
					SetClip(BigClipRgn);
					ProcessFieldChange(top, true);
					return true;
				case clearCmd:
					SetClip((**e).clipRgn);
					TEDelete(textH);
					SetClip(BigClipRgn);
					ProcessFieldChange(top, true);
					return true;
				case selectAllCmd:
					TESetSelect(0, 32767, textH);
					return true;
			}
			break;
		case proxyID:
			HiliteMenu(0);
			if (Proxy) {
				GetItem(ProxyMenu, theItem, user);
				GetRecord(user);
			} else {
				glob_Error(servErrors, msgNoProxy, nil);
			}
			return true;
	}
	return false;
}

#pragma segment edit

/*_____________________________________________________________________

	edit_Init - Initialize
_____________________________________________________________________*/

#pragma segment init

void edit_Init (void)

{
	WindowPtr	w;					/* pointer to dummy window */
	
	w = GetNewWindow(editWindID, nil, (WindowPtr)-1);
	SetPort(w);
	TextFont(FontNum);
	TextSize(fontSize);
	SetRect(&SizeRect, w->portRect.right+1, minWindHeight, 
		w->portRect.right+1, 0x7fff);
	Width = CharWidth('A');
	NameRight = textMargin + maxFieldName*Width;
	TextLeft = NameRight + 2*Width;
	TextRight = w->portRect.right - textMargin - sBarWidthM1;
	DisposeWindow(w);
	oop_RegisterCommandHandler(edit_DoCommand);
}

#pragma segment edit

/*_____________________________________________________________________

	edit_GetLoginStatus - Get Edit Login Status
	
	Exit:		function result = status
				server = login server, or nil if you don't care.
				alias = login alias, or nil if you don't care.
_____________________________________________________________________*/

EditLoginStatus edit_GetStatus (Str255 server, Str255 alias)

{
	if (LoggedIn) {
		if (server) utl_CopyPString(server, Server);
		if (alias) utl_CopyPString(alias, Alias);
		return Hero ? editHero : editLoggedIn;
	} else {
		return editNotLoggedIn;
	}
}

/*_____________________________________________________________________

	edit_IsDirty - Get Edit Window Dirty Status.
	
	Entry:	w = pointer to edit window record.
	
	Exit:		function result = true if window is dirty.
_____________________________________________________________________*/

Boolean edit_IsDirty (WindowPtr w)

{
	EditInfo			**e;			/* handle to edit info */
	
	e = GetEditInfo(w);
	return (**e).dirty;
}

/*_____________________________________________________________________

	edit_Terminate - Terminate.
	
	Entry:	interactionPermitted = true if interaction is permitted.
	
	Exit:		function result = true if save changes alert canceled.
_____________________________________________________________________*/

Boolean edit_Terminate (Boolean interactionPermitted)

{
	WindowPtr		v;						/* pointer to traverse window list */
	EditInfo			**g;					/* handle to edit info for window v */
	short				windNum;				/* window number */
	
	v = FrontWindow();
	while (v) {
		if (oop_GetWindKind(v) == editWind) {
			if (interactionPermitted && AlertIfDirty(v, msgQuitting)) 
				return true;
			g = GetEditInfo(v);
			windNum = (**g).windNum;
			if (windNum <= numPosSave) wstm_Save(v, &EditStates[windNum]);
		}
		v = (WindowPtr)((WindowPeek)v)->nextWindow;
	}
	return false;
}
