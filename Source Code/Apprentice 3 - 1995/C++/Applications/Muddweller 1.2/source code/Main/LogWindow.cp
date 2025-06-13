/* LogWindow - Implementation of TLogWindow                                   */

#include "LogWindow.h"


		// ¥ Implementation use
#ifndef __MUDDOC__
#include "MUDDoc.h"
#endif

#ifndef __USizerView__
#include "USizerView.h"
#endif



//------------------------------------------------------------------------------

const short kMinHeight = 73;
const short kMinWidth = 80;
const short kMinLen = 14;

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal void TLogWindow::Activate (Boolean entering)
{
	if (((TMUDDoc *) fDocument)->fUseCTB &&
			(((TMUDDoc *) fDocument)->fConn != NULL))
		CMActivate (((TMUDDoc *) fDocument)->fConn, entering);
	inherited::Activate (entering);
}

//------------------------------------------------------------------------------

#pragma segment SMUDDocRes

pascal Boolean TLogWindow::DoIdle (IdlePhase )
{
	return FALSE;
}

//------------------------------------------------------------------------------

#pragma segment AOpen

pascal void TLogWindow::IRes (TDocument *itsDocument, TView *itsSuperView,
		Ptr *itsParams)
{
	fLogView = NULL;
	fCommandView = NULL;
	inherited::IRes (itsDocument, itsSuperView, itsParams);
}

//------------------------------------------------------------------------------

#pragma segment AOpen

pascal void TLogWindow::Open (void)
{
	Point minSize, xmaxSize;
	TSizerView *aSizerView;
	TextStyle aStyle;
	VRect sizerRect;
	long tileHeight;
	
	fLogView = (TLogView *) FindSubView ('logt');
	fCommandView = (TCommandView *) FindSubView ('cmdf');
	fLogView->SetMaxSize (((TMUDDoc *) fDocument)->fLogSize * 1024);
	aStyle.tsFont = ((TMUDDoc *) fDocument)->fFontNum;
	aStyle.tsFace = 0;
	aStyle.tsSize = ((TMUDDoc *) fDocument)->fFontSize;
	aStyle.tsColor = gRGBBlack;
	fLogView->SetBaseStyle (&aStyle, kDontRedraw);
	fCommandView->SetBaseStyle (&aStyle, kDontRedraw);
	minSize.h = kMinWidth;
	minSize.v = kMinHeight;
	xmaxSize.h = fResizeLimits.right;
	xmaxSize.v = fResizeLimits.bottom;
	SetResizeLimits (minSize, xmaxSize);
	aSizerView = (TSizerView *) FindSubView ('hors');
	aSizerView->SetMinPaneLength (kMinLen);
	aSizerView->FixupPanes (FALSE);
	tileHeight = ((TMUDDoc *) fDocument)->fTileHeight;
	if (tileHeight != 0) {
		sizerRect = aSizerView->GetSizerRect (1);
		sizerRect.bottom += tileHeight - sizerRect.top;
		sizerRect.top += tileHeight - sizerRect.top;
		aSizerView->SetPane (1, &sizerRect);
	}
	inherited::Open ();
}

//------------------------------------------------------------------------------

#pragma segment MAInit

void InitLogWindow (void)
{
	if (gDeadStripSuppression) {
		TLogWindow *aLogWindow = new TLogWindow;
	}
}
