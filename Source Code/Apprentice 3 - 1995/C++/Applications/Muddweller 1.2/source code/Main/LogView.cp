/* LogView - Implementation of TLogView                                       */

#include "LogView.h"

		// � Toolbox
#ifndef __FONTS__
#include "Fonts.h"
#endif

#ifndef __TOOLUTILS__
#include "ToolUtils.h"
#endif

		// � Implementation use
#ifndef __GLOBALS__
#include "Globals.h"
#endif

#ifndef __LOGWINDOW__
#include "LogWindow.h"
#endif

#ifndef __MUDDOC__
#include "MUDDoc.h"
#endif

#ifndef __UMUDDWELLER__
#include "UMUDDweller.h"
#endif


//------------------------------------------------------------------------------

const short kLeftInset = 4;
const short kRightInset = 19;
const short kScrlWidth = 15;
const short kMaxScrapTE = 8000;

//------------------------------------------------------------------------------

#pragma segment MACommandRes

class TScrapTEView: public TTEView {
public:
	Handle fRealText;
	
	virtual pascal void Free (void);
	virtual pascal long GivePasteData (Handle aDataHandle, ResType dataType);
	virtual pascal void ITEView (TDocument *itsDocument, TView *itsSuperView,
		VPoint *itsLocation, VPoint *itsSize, SizeDeterminer itsHDeterminer,
		SizeDeterminer itsVDeterminer, Rect *itsInset, TextStyle *itsTextStyle,
		short itsJustification, Boolean itsStyleType, Boolean itsAutoWrap);
	virtual pascal void WriteToDeskScrap (void);
};

pascal void TScrapTEView::Free (void)
{
	DisposIfHandle (fRealText);
	inherited::Free ();
}

pascal long TScrapTEView::GivePasteData (Handle aDataHandle, ResType dataType)
{
	long size;
	
	if (fRealText && (dataType == 'TEXT')) {
		size = GetHandleSize (fRealText);
		if (aDataHandle) {
			SetPermHandleSize (aDataHandle, size);
			BlockMove (*fRealText, *aDataHandle, size);
		}
		return size;
	} else
		return inherited::GivePasteData (aDataHandle, dataType);
}

pascal void TScrapTEView::ITEView (TDocument *itsDocument, TView *itsSuperView,
		VPoint *itsLocation, VPoint *itsSize, SizeDeterminer itsHDeterminer,
		SizeDeterminer itsVDeterminer, Rect *itsInset, TextStyle *itsTextStyle,
		short itsJustification, Boolean itsStyleType, Boolean itsAutoWrap)
{
	fRealText = NULL;
	inherited::ITEView (itsDocument, itsSuperView, itsLocation, itsSize,
		itsHDeterminer, itsVDeterminer, itsInset, itsTextStyle,
		itsJustification, itsStyleType, itsAutoWrap);
}

pascal void TScrapTEView::WriteToDeskScrap (void)
{
	if (fRealText)
		FailOSErr (PutDeskScrapData ('TEXT', fRealText));
	else
		inherited::WriteToDeskScrap ();
}


//------------------------------------------------------------------------------

#pragma segment MACommandRes

class TLogCmd: public TCommand {
public:
	virtual pascal void DoIt (void);
	virtual pascal void ILogCmd (TLogView *itsView, CmdNumber itsCmdNumber);
};


pascal void TLogCmd::DoIt (void)
{
	switch (fCmdNumber) {
	case cCopy:
		((TLogView *) fView)->DoCopy ();
		break;
	}
}


pascal void TLogCmd::ILogCmd (TLogView *itsView, CmdNumber itsCmdNumber)
{
	ICommand (itsCmdNumber, NULL, itsView, NULL);
	fCanUndo = FALSE;
	fCausesChange = FALSE;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void CallActionProc (ControlHandle aCMgrControl, short partCode)
{
	TScrollBar *aScrollBar;

	aScrollBar = (TScrollBar *) GetCRefCon (aCMgrControl);
	FailNIL (aScrollBar);
	aScrollBar->ActionProc (partCode);
}


class TLogScrollBar: public TScrollBar {
public:
	virtual pascal void Activate (Boolean entering);
	virtual pascal struct TCommand *DoMouseCommand (Point *theMouse,
		EventInfo *info, Point *hysteresis);
	virtual pascal void SuperViewChangedSize (VPoint *delta,
		Boolean invalidate);
	virtual pascal void TrackScrollBar (short partCode);
};


pascal void TLogScrollBar::Activate (Boolean entering)
{
	Rect itsRect;
	Boolean wasVisible;
	
	itsRect = (**fCMgrControl).contrlRect;
	if (fCMgrControl) {
		if (Focus ()) {
			if (entering)
				ShowControl (fCMgrControl);
			else {
				HideControl (fCMgrControl);
				Draw (&itsRect);
			}
			ValidRect (&itsRect);
		} else {
			wasVisible = IsCMgrVisible ();
			SetCMgrVisibility (FALSE);
			if (entering)
				ShowControl (fCMgrControl);
			else
				HideControl (fCMgrControl);
			if (wasVisible && (!IsCMgrVisible ()))
				SetCMgrVisibility (wasVisible);
		}
	}
}


pascal struct TCommand *TLogScrollBar::DoMouseCommand (Point *theMouse,
		EventInfo *, Point *)
{
	short partCode;
	VCoordinate oldLongVal, newLongVal;
	
	oldLongVal = fLongVal;
	switch (TestControl (fCMgrControl, *theMouse)) {
	case inUpButton:
	case inDownButton:
	case inPageUp:
	case inPageDown:
		partCode = TrackControl (fCMgrControl, *theMouse,
			(ProcPtr) CallActionProc);
		if (fLongVal != oldLongVal) DoChoice (this, fDefChoice);
		break;
	case inThumb:
		if (TrackControl(fCMgrControl, *theMouse, NULL) == inThumb) {
			if (GetVal () == GetMax ())
				newLongVal = ((long) GetMax ()) << fBitsToShift;
			else
				newLongVal = ((long) GetVal ()) << fBitsToShift;
			if (newLongVal != oldLongVal) {
				((TLogView *) fSuperView)->Focus ();
				((TLogView *) fSuperView)->fBT->BTScroll (0, - newLongVal -
					((TLogView *) fSuperView)->fBT->fBounds.top, kRedraw);
				((TLogView *) fSuperView)->SynchScrollBar (kRedraw);
			}
		}
		break;
	}
	return NULL;
}

pascal void TLogScrollBar::SuperViewChangedSize (VPoint *delta,
		Boolean invalidate)
{
	Locate (fLocation.h + delta->h, fLocation.v, invalidate);
	Resize (fSize.h, fSize.v + delta->v, invalidate);
}


pascal void TLogScrollBar::TrackScrollBar (short partCode)
{
	TBigText *bt;
	long delta;
	
	bt = ((TLogView *) fSuperView)->fBT;
	switch (partCode) {
	case inPageUp:
		delta = fSuperView->fSize.v;
		break;
	case inPageDown:
		delta = -fSuperView->fSize.v;
		break;
	case inUpButton:
		delta = bt->fLineHeight;
		break;
	case inDownButton:
		delta = -bt->fLineHeight;
		break;
	default:
		return;
	}
	((TLogView *) fSuperView)->Focus ();
	bt->BTScroll (0, delta, kRedraw);
	((TLogView *) fSuperView)->SynchScrollBar (kRedraw);
}


//------------------------------------------------------------------------------

#pragma segment MAActivate

pascal void TLogView::Activate (Boolean entering)
{
	TLogWindow *aLogWindow;
	
	aLogWindow = (TLogWindow *) GetWindow ();
	if (aLogWindow->fTarget == this) {
		if (Focus ()) fBT->BTActivate (entering, kRedraw);
	}
	inherited::Activate (entering);
}


//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal long TLogView::AddText (unsigned char *textBuf, long count)
{
	if (Focus ()) {
		fBT->BTAppend (textBuf, count, kRedraw);
		if (fBT->fBounds.bottom > fBT->fDisplay.bottom)
			fBT->BTScroll (0, fSize.v - fBT->fBounds.bottom, kRedraw);
		SynchScrollBar (kRedraw);
	}
	return count;
}


//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TLogView::ClikLoop (Point pt)
{
	static unsigned long ticks = 0;
	
	if (((pt.v < fBT->fDisplay.top) || (pt.v > fBT->fDisplay.bottom)) &&
			((unsigned long) (TickCount () - ticks) >= kClikDelay)) {
		ticks = TickCount ();
		fBT->BTScroll (0, (pt.v < 0) ? fBT->fLineHeight: -fBT->fLineHeight,
			kRedraw);
		SynchScrollBar (kRedraw);
		Focus ();
	}
}


//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

static Handle txtHdl;
static TScrapTEView *clipTEView;

pascal void DoCopyHandler (short , long , void * )
{
	DisposIfHandle (txtHdl);
	txtHdl = NULL;
	FreeIfObject (clipTEView);
	clipTEView = NULL;
}

pascal void TLogView::DoCopy (void)
{
	TextStyle clipStyle;
	VPoint clipSize;
	Rect clipMargins;
	FailInfo fi;
	long size;
	Handle h;
	
	size = fBT->fSelEnd - fBT->fSelStart;
	if (size > kMaxScrapTE) size += kMaxScrapTE;
	h = NewPermHandle (size);
	while (!h) {
		if (!((TMUDDwellerApp *) gApplication)->DropSome (fDocument)) break;
		h = NewPermHandle (size);
	}
	FailNIL (h);
	((TMUDDwellerApp *) gApplication)->BuildReserves (fDocument);
	DisposIfHandle (h);
	txtHdl = NULL;
	clipTEView = NULL;
	CatchFailures (&fi, DoCopyHandler, kDummyLink);
	txtHdl = fBT->BTGetText ();
	FailNIL (txtHdl);
	SetTextStyle (&clipStyle, applFont, 0, 12, &gRGBBlack);
	SetVPt (&clipSize, 100, 50);
	SetRect (&clipMargins, 0, 0, 10, 0);
	clipTEView = new TScrapTEView;
	FailNIL (clipTEView);
	clipTEView->ITEView (NULL, NULL, &gZeroVPt, &clipSize, sizeSuperView,
		sizeVariable, &clipMargins, &clipStyle, teJustSystem, TRUE, FALSE);
	clipTEView->fAcceptsChanges = FALSE;
	FailSpaceIsLow ();
	if (size > kMaxScrapTE) {
		clipTEView->fRealText = txtHdl;
		txtHdl = NewPermHandle (kMaxScrapTE);
		FailNIL (txtHdl);
		BlockMove (*(clipTEView->fRealText), *txtHdl, kMaxScrapTE);
	}
	FailSpaceIsLow ();
	clipTEView->StuffText (txtHdl);
	FailSpaceIsLow ();
	clipTEView->fFreeText = TRUE;
	Success (&fi);
	gApplication->ClaimClipboard (clipTEView);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal TCommand *TLogView::DoKeyCommand (short ch, short aKeyCode,
		EventInfo *info)
{
	TLogWindow *aLogWindow;
	long delta;
	
	if (Focus ()) {
		switch (ch) {
		case chHome:
			delta = -fBT->fBounds.top;
			break;
		case chEnd:
			delta = fSize.v - fBT->fBounds.bottom;
			break;
		case chPageUp:
			delta = fSize.v;
			break;
		case chPageDown:
			delta = -fSize.v;
			break;
		default:
			aLogWindow = (TLogWindow *) GetWindow ();
			aLogWindow->SetTarget (aLogWindow->fCommandView);
			if (gTarget == this) // to prevent endless recursion
				return inherited::DoKeyCommand (ch, aKeyCode, info);
			else
				return gTarget->DoKeyCommand (ch, aKeyCode, info);
		}
		fBT->BTScroll (0, delta, kRedraw);
		SynchScrollBar (kRedraw);
	}
	return inherited::DoKeyCommand (ch, aKeyCode, info);
}


//------------------------------------------------------------------------------

#pragma segment MASelCommand

pascal TCommand *TLogView::DoMenuCommand (CmdNumber aCmdNumber)
{
	TLogCmd *aLogCmd;
	TLogWindow *aLogWindow;

	switch (aCmdNumber) {
	case cCopy:
		aLogCmd = new TLogCmd;
		FailNIL (aLogCmd);
		aLogCmd->ILogCmd (this, aCmdNumber);
		return aLogCmd;
	case cUndo:
	case cPaste:
	case cSelectAll:
		aLogWindow = (TLogWindow *) GetWindow ();
		aLogWindow->SetTarget (aLogWindow->fCommandView);
		if (gTarget == this) // to prevent endless recursion
			return inherited::DoMenuCommand (aCmdNumber);
		else
			return gTarget->DoMenuCommand (aCmdNumber);
		break;
	default:
		return inherited::DoMenuCommand (aCmdNumber);
	}
}


//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal struct TCommand *TLogView::DoMouseCommand (Point *theMouse,
		EventInfo *info, Point *)
{
	TLogWindow *aLogWindow;

	aLogWindow = (TLogWindow *) GetWindow ();
	if (!info->theShiftKey && Focus ()) fBT->BTSetSelect (0, 0, kRedraw);
	aLogWindow->SetTarget (this);
	if (Focus () && IsVisible ())
		fBT->BTClick (theMouse, info->theClickCount, info->theShiftKey);
	if (fBT->fSelStart == fBT->fSelEnd)
		aLogWindow->SetTarget (aLogWindow->fCommandView);
	return NULL;
}


//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal Boolean TLogView::DoSetCursor (Point localPoint, RgnHandle cursorRgn)
{
	GetDefaultCursorRgn (localPoint, cursorRgn);
	UseROMMap (TRUE);
	SetCursor (*GetCursor (iBeamCursor));
	return TRUE;
}


//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TLogView::DoSetupMenus (void)
{
	TLogWindow *aLogWindow;

	aLogWindow = (TLogWindow *) GetWindow ();
	inherited::DoSetupMenus ();
	if (fBT->fSelStart != fBT->fSelEnd) Enable (cCopy, TRUE);
	CanPaste ('TEXT');
	Enable (cSelectAll, (**(aLogWindow->fCommandView->fHTE)).teLength > 0);
}


//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TLogView::Draw (Rect *area)
{
	fBT->Draw (area);
}


//------------------------------------------------------------------------------

#pragma segment MAClose

pascal void TLogView::Free (void)
{
	if (fBT) {
		fBT->Free ();
		fBT = NULL;
	}
	inherited::Free ();
}


//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TLogView::InstallSelection (Boolean , Boolean beActive)
{
	if (Focus ()) fBT->BTActivate (beActive, kRedraw);
}


//------------------------------------------------------------------------------

#pragma segment AOpen

pascal void TLogView::IRes (TDocument *itsDocument, TView *itsSuperView,
		Ptr *itsParams)
{
	TBigText *aBT;
	TextStyle aStyle;
	Rect r;
	
	fBT = NULL;
	fScrollBar = NULL;
	inherited::IRes (itsDocument, itsSuperView, itsParams);
	aBT = new TBigText;
	FailNIL (aBT);
	r.top = 0;
	r.left = kLeftInset;
	r.right = (short) (fSize.h - kRightInset);
	r.bottom = (short) fSize.v;
	aStyle.tsFont = ((TMUDDoc *) fDocument)->fFontNum;
	aStyle.tsFace = 0;
	aStyle.tsSize = ((TMUDDoc *) fDocument)->fFontSize;
	aStyle.tsColor = gRGBBlack;
	aBT->IBigText (&r, &aStyle, ((TMUDDoc *) fDocument)->fTabSize, this);
	fBT = aBT;
	FailSpaceIsLow ();
}

//------------------------------------------------------------------------------

#pragma segment MAOpen

pascal void TLogView::Open (void)
{
	TScrollBar *aScrollBar;
	VRect extent;
	
	aScrollBar = (TScrollBar *) FindSubView ('scrl');
	fScrollBar = aScrollBar;
	GetExtent (&extent); /* avoid flashing */
	extent.right -= kScrlWidth; /* but draw scrollbar */
	ValidVRect (&extent);
	inherited::Open ();
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TLogView::Resize (VCoordinate width, VCoordinate height,
		Boolean invalidate)
{
	Rect r;
	VRect extent;
	Boolean atend;
	
	inherited::Resize (width, height, invalidate);
	r.top = 0;
	r.left = kLeftInset;
	r.right = (short) (fSize.h - kRightInset);
	r.bottom = (short) fSize.v;
	Focus ();
	atend = fBT->fBounds.bottom <= fBT->fDisplay.bottom;
	fBT->BTResize (&r, r.left, r.right, kDontRedraw);
	if (atend && (fBT->fBounds.bottom > fBT->fDisplay.bottom))
		fBT->BTScroll (0, fSize.v - fBT->fBounds.bottom, kDontRedraw);
	SynchScrollBar (kDontRedraw);
	GetExtent (&extent);
	extent.right -= kScrlWidth;
	InvalidVRect (&extent);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TLogView::SetBaseStyle (TextStyle *theStyle, Boolean redraw)
{
	Boolean atend;
	VRect extent;

	Focus ();
	atend = fBT->fBounds.bottom <= fBT->fDisplay.bottom;
	fBT->BTSetStyle (theStyle, ((TMUDDoc *) fDocument)->fTabSize, kDontRedraw);
	if (atend && (fBT->fBounds.bottom > fBT->fDisplay.bottom))
		fBT->BTScroll (0, fSize.v - fBT->fBounds.bottom, kDontRedraw);
	SynchScrollBar (kDontRedraw);
	if (redraw) {
		GetExtent (&extent);
		InvalidVRect (&extent);
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TLogView::SetMaxSize (long logSize)
{
	long oldbot;
	VRect extent;
	
	fBT->fMaxLength = logSize;
	if ((fBT->fLength > logSize) && Focus ()) {
		oldbot = fBT->fBounds.bottom;
		fBT->BTDrop (fBT->fLength - logSize, kDontRedraw);
		SynchScrollBar (kDontRedraw);
		GetExtent (&extent);
		if (oldbot == fBT->fBounds.bottom)
			extent.left = extent.right - kScrlWidth;
		InvalidVRect (&extent);
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TLogView::SynchScrollBar (Boolean redraw)
{
	VRect bounds;
	long val, max, oldval, oldmax;
	short bts;
	
	if (!fScrollBar) return;
	oldmax = fScrollBar->GetMax ();
	oldval = fScrollBar->GetVal ();
	bounds = fBT->fBounds;
	max = bounds.bottom - bounds.top - fSize.v;
	if (max < 0) max = 0;
	val = -bounds.top;
	fScrollBar->fLongMax = max;
	fScrollBar->fLongVal = val;
	bts = 0;
	while (max > 32767) {
		max >>= 1;
		val >>= 1;
		bts += 1;
	}
	fScrollBar->fBitsToShift = bts;
	if (oldval > max) oldval = max;
	if ((oldmax != max) || (oldval != val)) {
		if ((!max && oldmax) || (oldval == val)) {
			fScrollBar->SetMax ((short) max, redraw);
			fScrollBar->SetVal ((short) val, kDontRedraw);
		} else {
			fScrollBar->SetMax ((short) max, kDontRedraw);
			fScrollBar->SetVal ((short) val, redraw);
		}
	}
}

//------------------------------------------------------------------------------

#pragma segment MAInit

void InitLogView (void)
{
	if (gDeadStripSuppression) {
		TLogView *aLogView = new TLogView;
		TLogScrollBar *aLogScrollBar = new TLogScrollBar;
	}
}

//------------------------------------------------------------------------------
