/* CommandView - Implementation of TCommandView                               */

#include "CommandView.h"


		// ¥ Toolbox
#ifndef __VALUES__
#include "Values.h"
#endif


		// ¥ Implementation use
#ifndef __GLOBALS__
#include "Globals.h"
#endif

#ifndef __MUDDOC__
#include "MUDDoc.h"
#endif

#ifndef __USizerView__
#include "USizerView.h"
#endif


//------------------------------------------------------------------------------

#pragma segment MACommandRes

class TSendCmd: public TCommand {
public:
	virtual pascal void DoIt (void);
	virtual pascal void ISendCmd (TCommandView *itsView,CmdNumber itsCmdNumber);
};


pascal void TSendCmd::DoIt (void)
{
	switch (fCmdNumber) {
	case cReturnKey:
		((TCommandView *) fView)->SendLine ();
		break;
	case cUpKey:
		((TCommandView *) fView)->ScrollHist (TRUE);
		break;
	case cDownKey:
		((TCommandView *) fView)->ScrollHist (FALSE);
		break;
	}
}


pascal void TSendCmd::ISendCmd (TCommandView *itsView, CmdNumber itsCmdNumber)
{
	ICommand (itsCmdNumber, NULL, itsView, NULL);
	fCanUndo = FALSE;
	fCausesChange = FALSE;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

class TCmdScrollBar: public TSScrollBar {
public:
	virtual pascal void Activate (Boolean entering);
};


pascal void TCmdScrollBar::Activate (Boolean )
{
	inherited::Activate (FALSE);
}


//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

class TCmdScroller: public TScroller {
public:
	virtual pascal void CreateTemplateScrollBar (VHSelect itsDirection);
};


pascal void CreateHandler (short , long , void *thisObject)
{
	((TCmdScrollBar *) thisObject)->Free ();
}


pascal void TCmdScroller::CreateTemplateScrollBar (VHSelect itsDirection)
{
	FailInfo fi;
	TCmdScrollBar *aCmdScrollBar;

	CatchFailures (&fi, CreateHandler, this);
	aCmdScrollBar = (TCmdScrollBar *) DoCreateViews (fDocument, fSuperView,
		kCmdScrlID, &gZeroVPt);
	aCmdScrollBar->fDirection = itsDirection;
	aCmdScrollBar->fShown = IsShown ();
	aCmdScrollBar->AttachScroller (this);
	Success (&fi);
}


//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TCommandView::CalcMinSize (VPoint *minSize)
{
	inherited::CalcMinSize (minSize);
	if (minSize->v < fSuperView->fSize.v) minSize->v = fSuperView->fSize.v;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal Boolean TCommandView::ClikLoop (void)
{
	Boolean result;
	TextStyle aTextStyle;
	
	result = inherited::ClikLoop ();
	aTextStyle = fTextStyle;
	SetPortTextStyle (&aTextStyle);
	return result;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal TCommand *TCommandView::DoKeyCommand (short ch, short aKeyCode,
		EventInfo *info)
{
	TSendCmd *aSendCmd;
	TLogWindow *aLogWindow;
	MHandle mh;
	TMacro *mac;

	mh = ((TMUDDoc *) fMyDocument)->fMacroList;
	while (mh) {
		if (((aKeyCode & 0xff) == (**mh).mCode) &&
				(info->theShiftKey == (**mh).mShift) &&
				(info->theOptionKey == (**mh).mOption) &&
				(info->theControlKey == (**mh).mControl))
			break;
		mh = (**mh).mNext;
	}
	if (mh) {
		if (gDisableMenus || !((TMUDDoc *) fMyDocument)->fConnected ||
				((TMUDDoc *) fMyDocument)->fClosing)
			gApplication->Beep (0);
		else {
			mac = new TMacro;
			FailNIL (mac);
			mac->IMacro (fMyDocument, mh);
			if (((TMUDDoc *) fMyDocument)->fRunList == mac) mac->DoIdle ();
		}
		return NULL;
	} else {
		switch (ch) {
		case chReturn:
		case chEnter:
			if (info->theOptionKey)
				return inherited::DoKeyCommand (chReturn, aKeyCode, info);
			else if (gDisableMenus || !((TMUDDoc *) fMyDocument)->fConnected ||
					((TMUDDoc *) fMyDocument)->fClosing) {
				gApplication->Beep (0);
				return NULL;
			} else {
				aSendCmd = new TSendCmd;
				FailNIL (aSendCmd);
				aSendCmd->ISendCmd (this, cReturnKey);
				return aSendCmd;
			}
		case chUp:
			if (info->theOptionKey)
				return inherited::DoKeyCommand (ch, aKeyCode, info);
			else {
				aSendCmd = new TSendCmd;
				FailNIL (aSendCmd);
				aSendCmd->ISendCmd (this, cUpKey);
				return aSendCmd;
			}
		case chDown:
			if (info->theOptionKey)
				return inherited::DoKeyCommand (ch, aKeyCode, info);
			else {
				aSendCmd = new TSendCmd;
				FailNIL (aSendCmd);
				aSendCmd->ISendCmd (this, cDownKey);
				return aSendCmd;
			}
		case chHome:
		case chEnd:
		case chPageDown:
		case chPageUp:
			aLogWindow = (TLogWindow *) GetWindow ();
			return aLogWindow->fLogView->DoKeyCommand (ch, aKeyCode, info);
		default:
			return inherited::DoKeyCommand (ch, aKeyCode, info);
		}
	}
}


//------------------------------------------------------------------------------

#pragma segment TERes

pascal TCommand *TCommandView::DoMouseCommand (Point *theMouse, EventInfo *info,
		Point *hysteresis)
{
	TLogWindow *aLogWindow;
	
	aLogWindow = (TLogWindow *) GetWindow ();
	if (aLogWindow->fTarget != this) {
		if (!info->theShiftKey && Focus ()) TESetSelect (0, 0, fHTE);
		aLogWindow->SetTarget (this);
	}
	return inherited::DoMouseCommand (theMouse, info, hysteresis);
}

//------------------------------------------------------------------------------

#pragma segment MAClose

pascal void TCommandView::Free (void)
{
	Handle *hp;
	
	if (fHandList != NULL ) {
		hp = *fHandList;
		for (int i = fMaxLines; i > 0; i--) DisposIfHandle (*hp++);
		DisposIfHandle (fHandList);
	}
	inherited::Free ();
}

//------------------------------------------------------------------------------

#pragma segment AOpen

pascal void TCommandView::IRes (TDocument *itsDocument, TView *itsSuperView,
		Ptr *itsParams)
{
	Handle *hp;
	Rect dest;
	
	fHandList = NULL;
	fCurIndex = 0;
	fFirstIndex = 0;
	fMaxLines = 0;
	inherited::IRes (NULL, itsSuperView, itsParams);
		// fDocument = NULL prevents typing commands from setting fChangeCount
	fMyDocument = itsDocument;
	fInset.top = 2;
	fInset.left = 2;
	fInset.bottom = 1;
	fInset.right = 2;
	dest.top = fInset.top;
	dest.left = fInset.left;
	dest.bottom = (short) (fSize.v - fInset.bottom);
	dest.right = (short) (fSize.h - fInset.right);
	StuffTERects (&dest);
	fShowIndex = 0;
	fMaxLines = ((TMUDDoc *) itsDocument)->fHistSize + 1;
	fHandList = (Handle **) NewPermHandle ((long) fMaxLines * sizeof (Handle));
	FailNIL (fHandList);
	hp = *fHandList;
	for (int i = fMaxLines; i > 0; i--) *hp++ = NULL;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TCommandView::RevealRect (VRect *rectToReveal, Point minToSee,
		Boolean redraw)
{
	minToSee.h += 2;
	minToSee.v += 2;
	inherited::RevealRect (rectToReveal, minToSee, redraw);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TCommandView::ScrollHist (Boolean scrollUp)
{
	Boolean showIt;
	Handle theChars;
	long len;
	int oldShowIndex;
	Rect area;
	
	CommitLastCommand ();
	oldShowIndex = fShowIndex;
	showIt = FALSE;
	if (scrollUp) {
		if (fShowIndex != fFirstIndex) {
			fShowIndex = (fShowIndex + fMaxLines - 1) % fMaxLines;
			showIt = TRUE;
		}
	} else {
		if (fShowIndex != fCurIndex) {
			fShowIndex = (fShowIndex + 1) % fMaxLines;
			showIt = TRUE;
		}
	}
	if (showIt) {
		SetText ("\p"); // text moves to fSavedTEHandle
		if (oldShowIndex == fCurIndex) {
			DisposIfHandle (*(*fHandList + fCurIndex));
			*(*fHandList + fCurIndex) = fSavedTEHandle;
			fSavedTEHandle = NULL;
		} else {
			DisposIfHandle (fSavedTEHandle);
			fSavedTEHandle = NULL;
		}
		theChars = *(*fHandList + fShowIndex);
		if (theChars != NULL)
			FailOSErr (HandToHand (&theChars));
		else {
			theChars = NewPermHandle (0);
			FailNIL (theChars);
		}
		StuffText (theChars);
		len = GetHandleSize (theChars);
		SetSelect ((short) len, (short) len, fHTE);
		RecalcText ();
		SynchView (kDontRedraw);
		ScrollSelectionIntoView ();
		if (Focus () && IsVisible ()) {
			GetQDExtent (&area);
			EraseRect (&area);
			DrawContents ();
			ValidRect (&area);
		}
	}
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TCommandView::SendLine (void)
{
	Handle theChars;
	unsigned char *theLine, *oldLine, *cp, ch;
	SignedByte oldState;
	Boolean drop;
	long len, i;
	
	CommitLastCommand ();
	theChars = ExtractText ();
	len = GetHandleSize (theChars);
	oldState = HGetState (theChars);
	HLock (theChars);
	((TMUDDoc *) fMyDocument)->Send ((unsigned char *) *theChars, len);
	HSetState (theChars, oldState);
	ch = chReturn;
	((TMUDDoc *) fMyDocument)->Send (&ch, 1);
	drop = TRUE;
	cp = (unsigned char *) *theChars;
	i = len;
	while (drop && (i-- > 0)) drop = (*cp++ == ' ');
	if (!drop && (fCurIndex != fFirstIndex)) {
		drop = (len == GetHandleSize (*(*fHandList + (fCurIndex + fMaxLines -1)
			% fMaxLines)));
		if (drop) {
			theLine = (unsigned char *) *theChars;
			oldLine = (unsigned char *) **(*fHandList + (fCurIndex + fMaxLines
				- 1) % fMaxLines);
			i = len;
			while (drop && (i-- > 0)) drop = (*theLine++ == *oldLine++);
		}
	}
	SetText ("\p"); // text moves to fSavedTEHandle
	ForceRedraw ();
	if (Focus ())
		TESetSelect (0, 0, fHTE);
	else
		SetSelect (0, 0, fHTE);
	RecalcText ();
	SynchView (kDontRedraw);
	SetIdleFreq (0);
	if (!drop) {
		DisposIfHandle (*(*fHandList + fCurIndex));
		*(*fHandList + fCurIndex) = fSavedTEHandle;
		fSavedTEHandle = NULL;
		fCurIndex = (fCurIndex + 1) % fMaxLines;
		if (fCurIndex == fFirstIndex) {
			fFirstIndex = (fFirstIndex + 1) % fMaxLines;
			DisposIfHandle (*(*fHandList + fCurIndex));
			*(*fHandList + fCurIndex) = NULL;
		}
	} else {
		DisposIfHandle (fSavedTEHandle);
		fSavedTEHandle = NULL;
		DisposIfHandle (*(*fHandList + fCurIndex));
		*(*fHandList + fCurIndex) = NULL;
	}
	fShowIndex = fCurIndex;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TCommandView::SetBaseStyle (TextStyle *theStyle, Boolean redraw)
{
	SetOneStyle (0, (**fHTE).teLength, doAll, theStyle, redraw);
	fTextStyle = *theStyle;
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TCommandView::SetHistSize (long lines)
{
	Handle *hp, **newHList;
	int i, j;
	
	lines += 1;
	if (fMaxLines != lines) {
		newHList = (Handle **) NewPermHandle (lines * sizeof (Handle));
		FailNIL (newHList);
		hp = *newHList;
		for (i = (int) lines; i > 0; i--) *hp++ = NULL;
		if (lines <= (fCurIndex + fMaxLines - fFirstIndex) % fMaxLines) {
			i = (fCurIndex + fMaxLines - (int) lines + 1) % fMaxLines;
			while (fFirstIndex != i) {
				DisposIfHandle (*(*fHandList + fFirstIndex));
				fFirstIndex = (fFirstIndex + 1) % fMaxLines;
			}
		}
		i = fFirstIndex;
		j = 0;
		while (TRUE) {
			*(*newHList + j) = *(*fHandList + i);
			if (i == fCurIndex) break;
			i = (i + 1) % fMaxLines;
			j++;
		}
		DisposIfHandle (fHandList);
		fHandList = newHList;
		fShowIndex = j - (fCurIndex + fMaxLines - fShowIndex) % fMaxLines;
		if (fShowIndex < 0) fShowIndex = 0;
		fFirstIndex = 0;
		fCurIndex = j;
		fMaxLines = (int) lines;
	}
}

//------------------------------------------------------------------------------

#pragma segment MAInit

void InitCommandView (void)
{
	if (gDeadStripSuppression) {
		TCommandView *aCommandView = new TCommandView;
		TCmdScroller *aCmdScroller = new TCmdScroller;
		TCmdScrollBar *aCmdScrollBar = new TCmdScrollBar;
	}
	gNonPanes->AddElement (GetClassIDFromName ("\pTCmdScrollBar"));
}

//------------------------------------------------------------------------------
