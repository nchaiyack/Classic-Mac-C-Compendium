/*
	File:		DWindow.cp

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <6>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <5>	11/14/92	MTG		Bringing the C++ version up to date WRT the ThinkC version.
		 <4>	 9/20/92	MTG		Bringing the C++ code up to date with the THINK C version
		 <3>	  8/9/92	MTG		merging changes from the ThinkC version
		 <2>	  8/8/92	MTG		clearing up un-used variables

	To Do:
*/

// the is the Class definition for the DWindow class


#include "DWindow.h"
#include "DDocument.h"
#include "DEventHandler.h"
#include <DinkUtils.h>

DWindow::DWindow(void)
{
	//stub!!
}


DWindow::~DWindow(void)
{
	DisposHandle( (Handle) fPrintRecord);
}
	
	
Boolean DWindow::Init(DDocument *doc, Boolean hasColorWindows)
{
	WindowPtr prevFrontWindow;
	Point corner;
	RgnHandle	theGrayRgn;
	Rect r;

	SetRect(&r, 0,0,0,0);
	
	fVOffSet = 0;
	fHOffSet = 0;

	theGrayRgn = GetGrayRgn();
	fGrowWindowLimits = (**theGrayRgn).rgnBBox;
	fGrowWindowLimits.top = MINHIGHT; 
	fGrowWindowLimits.left = MINWIDTH;
	
	fPrintRecord = (THPrint) NewHandle(sizeof(TPrint) );
		//set up the print record with the last printer settings
	if(fPrintRecord != NULL)
	{
		PrOpen();
		PrintDefault(fPrintRecord);
		PrClose();
	}

	prevFrontWindow =  FrontWindow();
	if(prevFrontWindow != NULL)
	{
		SetPort( prevFrontWindow);
		corner.v = prevFrontWindow->portRect.top;
		corner.h = prevFrontWindow->portRect.left;
		LocalToGlobal(&corner);
	}	
	
	fDoc = doc;
	
	if(hasColorWindows)
		fWindowPtr = GetNewCWindow(rWindowID, NULL, (WindowPtr)-1L);
	else
		fWindowPtr = GetNewWindow(rWindowID, NULL, (WindowPtr)-1L);
	
	if(fWindowPtr)
	{
		fNextHandler = fDoc;
		SetWRefCon(fWindowPtr, (long)this);	// tie this to window for activate events!!
		if(prevFrontWindow != NULL)
			MoveWindow(fWindowPtr, corner.h + kStagger, corner.v + kStagger, TRUE);
		ShowWindow(fWindowPtr);

		return TRUE;
	}
	else
	{
		KillMeNext();
		return FALSE;
	}
		
}// end of Init member function
	



void DWindow::Draw( Rect *area)
{
#ifdef THINK_C
	FillRect( area, dkGray);
#else
	FillRect( area, &qd.dkGray);
#endif
		// This lets you know that you forgot a 
		// virtual override of this guy....
}



void DWindow::HandleUpdateEvt(EventRecord *theEvent)
{
	SetPort (fWindowPtr);
	BeginUpdate (fWindowPtr);
	if( !EmptyRgn(fWindowPtr->visRgn) )
		Draw( &(fWindowPtr->portRect));
	DrawGrowIcon( fWindowPtr);
	EndUpdate(fWindowPtr);
	
	inherited::HandleUpdateEvt(theEvent);
}

	
void DWindow::HandleActivateEvt(EventRecord *theEvent)
{
	FocusOnContent();
	if( !EmptyRgn(fWindowPtr->visRgn) )
		Draw( &(fWindowPtr->portRect));
	DrawGrowIcon(fWindowPtr);
	
	inherited::HandleActivateEvt(theEvent);
}

	
Boolean DWindow::KillMeNext(void)
{
	Boolean inheritedSuccess = FALSE;

	if(fAlive && fDoc->WindowClosed(this))// to give the user a chance to back out of the close...
	{
		if (inheritedSuccess = inherited::KillMeNext() )
		{
			
			//
			// These have to be after the inherited call
			// to avoid getting stuck in a recursive
			// stack smashing mess.  I'm looking for a better way.
			//
			
			DisposeWindow( fWindowPtr );
		}
	}
	return inheritedSuccess;
}

void DWindow::HandleMouseDown(EventRecord *theEvent, short thePart, WindowPtr theWindow)
{
	if (theWindow == fWindowPtr)
	{
		switch( thePart)
		{
			case 	inGoAway:
				if (TrackGoAway(theWindow, theEvent->where) )
					KillMeNext();
				break;
			
			case	inDrag:
					DoDrag(theEvent);
				break;
			
			case	inGrow:
					DoGrow(theEvent);
				break;
			
			case	inZoomIn:
			case	inZoomOut:
				if( TrackBox( theWindow, theEvent->where, thePart) ) 
					DoZoom(thePart);
				break;
			
			case	inContent:
				DoContent(theEvent);
				break;
		}// end switch
	}
	else
		; // do nothing but pass it on
	
	inherited::HandleMouseDown(theEvent, thePart, theWindow);
}


void DWindow::DoDrag(EventRecord *theEvent)
{
	RgnHandle	theGrayRgn;
	Rect		r;
	
	theGrayRgn = GetGrayRgn();
	r = (**theGrayRgn).rgnBBox;
	
	DragWindow(fWindowPtr, theEvent->where, &r);

}// end member function DoDrag


void DWindow::DoGrow(EventRecord *theEvent)
{
	long		result;
	RgnHandle	theGrayRgn;
	Rect		r;
	
	theGrayRgn = GetGrayRgn();
	r = (**theGrayRgn).rgnBBox;
	r.top = MINHIGHT; r.left = MINWIDTH;
	r = fGrowWindowLimits;
	SetPort(fWindowPtr);
	result = GrowWindow(fWindowPtr, theEvent->where, &r);
	
	if(result != 0)
	{
		r = fWindowPtr->portRect;
		r.left = r.right - kScrollBarPos;
		InvalRect(&r);
		EraseRect(&r);
		r = fWindowPtr->portRect;
		r.top = r.bottom - kScrollBarPos;
		InvalRect(&r);
		EraseRect(&r);
		
		SizeWindow(fWindowPtr, LoWrd(result), HiWrd(result), TRUE);
		
		r = fWindowPtr->portRect;
		r.left = r.right - kScrollBarPos;
		InvalRect(&r);
		EraseRect(&r);
		r = fWindowPtr->portRect;
		r.top = r.bottom - kScrollBarPos;
		InvalRect(&r);
		EraseRect(&r);
	}// end if 
}// end of member function DoGrow


void DWindow::DoZoom(short thePart)
{	
	SetPort(fWindowPtr);
	EraseRect(&fWindowPtr->portRect);
	ZoomWindow(fWindowPtr, thePart, fWindowPtr == FrontWindow());
	
	InvalRect(&fWindowPtr->portRect);
}// end member function DoZoom
	

void DWindow::DoContent(EventRecord *theEvent)
{

	; // stub!!

}// end member function DoContent



void	DWindow::DoPageSetUp(void)
{
	PrOpen();
	(void)PrStlDialog(fPrintRecord);
	PrClose();
}


void	DWindow::DoPrint(void)
{
	TPPrPort	printPort;
	Rect		r;
	TPrStatus	status;
	THPrint		tmpPrintRecord;
	
	tmpPrintRecord = fPrintRecord;
	PrOpen();
	if (PrValidate(tmpPrintRecord) )
		if (!PrStlDialog(tmpPrintRecord) )
		{
			PrClose();
			return;
		}
		
	if (!PrJobDialog(tmpPrintRecord) )
	{
		PrClose();
		return;
	}
	
	printPort = PrOpenDoc(tmpPrintRecord, NULL, NULL);
	
	PrOpenPage(printPort, NULL);
	
	SetRect(&r, 0, 0, 0, 0);
	Draw(&r);
	
	PrClosePage(printPort);
	
	PrCloseDoc(printPort);
	
	if ((**tmpPrintRecord).prJob.bJDocLoop != 0)
		PrPicFile(tmpPrintRecord, NULL, NULL, NULL, &status);
		
	PrClose();
	// now clean up the mess we made on the screen!!!
	
	FocusOnWindow();
	InvalRgn( ((WindowPeek)fWindowPtr)->contRgn);
}


void DWindow::SetWindowTitle(void)
{
	StringPtr title;
	
	if(fDoc->fFileRef != 0) //fFileRef is the file open flag.
	{
		title = (fDoc->fFileReply->sfFile).name;
		SetWTitle(fWindowPtr, title);
	}
}

void	DWindow::FocusOnContent(void)
{
	Rect r;
	
	SetPort(fWindowPtr);
	SetOrigin(fHOffSet, fVOffSet);
	GetContentRect(&r);
	ClipRect(&r);
}/*end of function*/


void	DWindow::FocusOnWindow(void)
{
	Rect r;
	
	SetPort(fWindowPtr);
	SetOrigin(0,0);
	r = fWindowPtr->portRect;
	ClipRect(&r);
}/*end of function*/
	

void	DWindow::GetContentRect(Rect *r)
{
	*r = fWindowPtr->portRect;
	r->right -= kScrollBarPos;
	r->bottom -= kScrollBarPos;
}/*end of function*/

	
	
	
	
