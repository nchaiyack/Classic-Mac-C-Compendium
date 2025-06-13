/*
	File:		DScrollWindow.cp

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <5>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <4>	11/14/92	MTG		Bringing the C++ version up to date WRT the ThinkC version.
		 <3>	  8/9/92	MTG		merging changes form the ThinkC version
		 <2>	  8/8/92	MTG		cleaning up unused variables

	To Do:
*/

#include "DScrollWindow.h"
#include <DApplication.h>



DScrollWindow::DScrollWindow(void)
{
//null
}

DScrollWindow::~DScrollWindow(void)
{
//null
}

		
Boolean DScrollWindow::Init(DDocument *doc, Boolean hasColorWindows)
{
	Boolean inheritedSuccess;
	Rect r;
	
	SetRect(&r, 0, 0, 0, 0);
	inheritedSuccess = inherited::Init(doc, hasColorWindows);
	
	fHorizScrollBar = NULL;
	fVertScrollBar = NULL;
	fVMin = 0;
	fVMax = 444;
	
	fHMin = 0;
	fHMax = 444;
	if (inheritedSuccess)
	{
		SetPort(fWindowPtr);
		fHorizScrollBar = NewControl(fWindowPtr,&r, "\p",TRUE,
								0, 0, 0, scrollBarProc, 0);
		fVertScrollBar = NewControl(fWindowPtr,&r, "\p",TRUE,
								0, 0, 0, scrollBarProc, 0);
		
		SizeScrollBars();
	
		SynchScrollBars();
	 	InvalRect(&(fWindowPtr->portRect));
	 }
	return ( (fHorizScrollBar != NULL)&&(fVertScrollBar != NULL) && inheritedSuccess);
}// end of init function



Boolean DScrollWindow::KillMeNext(void)
{
	Boolean inheritedSuccess = FALSE;

	if(fAlive)
	{
		if(inheritedSuccess = inherited::KillMeNext())
		{
			KillControls(fWindowPtr);
		}
	}
	return inheritedSuccess;
}




void	DScrollWindow::HandleUpdateEvt(EventRecord *theEvent)
{
	Rect r;
	
	SetPort (fWindowPtr);
	BeginUpdate(fWindowPtr);
		
	FocusOnContent();
	r = (**(fWindowPtr->visRgn)).rgnBBox;
	if( !EmptyRgn(fWindowPtr->visRgn) )
		Draw(&r);
		
	FocusOnWindow();
	DrawControls(fWindowPtr);
	DrawGrowIcon( fWindowPtr);
	EndUpdate(fWindowPtr);
	
}// end of HandleUpdateEvt member function

//
// Needed to fixe make the scroll bars behave correctly when a context change occurs
//
void DScrollWindow::HandleOSEvent(EventRecord *theEvent)
{
	if (gApplication->fInBackground)
	{
		FocusOnWindow();
		HideControl(fVertScrollBar);
		HideControl(fHorizScrollBar);
	}
	else
	{
		FocusOnWindow();
		ShowControl(fVertScrollBar);
		ShowControl(fHorizScrollBar);
		DrawControls(fWindowPtr);
		DrawGrowIcon(fWindowPtr);

		FocusOnContent();
	}
	inherited::HandleOSEvent(theEvent);
	
}// end of HandleOSEvent member function
		
	
void DScrollWindow::HandleActivateEvt(EventRecord *theEvent)
{
	Boolean	activating;
	
	activating = theEvent->modifiers & activeFlag;
	if(activating)
	{
		FocusOnWindow();
		ShowControl(fVertScrollBar);
		ShowControl(fHorizScrollBar);
		DrawControls(fWindowPtr);
		DrawGrowIcon(fWindowPtr);

		FocusOnContent();
		if( !EmptyRgn(fWindowPtr->visRgn) )
			;// makes comm tool box window go south (erases my window!!!!
			// if Draw( &(fWindowPtr->portRect)); is called
			// besides, DScrollWindow should only take care of its own
	}
	else
	{
		FocusOnWindow();
		HideControl(fVertScrollBar);
		HideControl(fHorizScrollBar);
	}
}// end of HandleActivateEvt member function
	
short	DScrollWindow::GetVertLineScrollAmount(void)
{
	return 16;
}//end of GetVertLineScrollAmount function


short	DScrollWindow::GetHorizLineScrollAmount(void)
{
	return 16;
}//end of GetHorizLineScrollAmount function

	
short	DScrollWindow::GetVertPageScrollAmount(void)
{
	Rect r;
	
	GetContentRect(&r);
	return r.bottom - r.top - kScrollOverlap;
}//end of GetVertPageScrollAmount function

	
short	DScrollWindow::GetHorizPageScrollAmount(void)
{
	Rect r;
	
	GetContentRect(&r);
	return r.right - r.left - kScrollOverlap;
}//end of GetHorizPageScrollAmount function


void	DScrollWindow::DoGrow(EventRecord *theEvent)
{
	FocusOnWindow();
	inherited::DoGrow(theEvent);
	SizeScrollBars();
	SynchScrollBars();
}//end of DoGrow function




void	DScrollWindow::DoZoom(short partCode)
{
	FocusOnWindow();
	inherited::DoZoom(partCode);
	SizeScrollBars();
	SynchScrollBars();
}//end of DoZoom function




void	DScrollWindow::DoContent(EventRecord *theEvent)
{
	Rect contents;
	
	FocusOnWindow();
	GlobalToLocal(&theEvent->where);
	GetContentRect(&contents);
	if(PtInRect(theEvent->where, &contents))
	{
		FocusOnContent();
		inherited::DoContent(theEvent);
	}
	else
		ScrollClick(theEvent);
}// end of DoContent member function

	

void	DScrollWindow::SizeScrollBars()
{
	Rect r, r_temp;
	
	FocusOnWindow();
	r = fWindowPtr->portRect;

	HideControl(fVertScrollBar);
	SizeControl(fVertScrollBar, kScrollBarWidth,
				(r.bottom - r.top - kScrollBarPos) + 2);		// the '2' is to make the bar end on the grow box boundry
	MoveControl(fVertScrollBar, r.right - kScrollBarPos, -1);
	ShowControl(fVertScrollBar);
	r_temp = (**fVertScrollBar).contrlRect;
	ValidRect(&r_temp);
	InvalRect(&r_temp);

	HideControl(fHorizScrollBar);
	SizeControl(fHorizScrollBar, (r.right - r.left - kScrollBarPos) + 2, 
									kScrollBarWidth);
	MoveControl(fHorizScrollBar, -1, r.bottom - r.top - kScrollBarPos);
	ShowControl(fHorizScrollBar);
	r_temp = (**fHorizScrollBar).contrlRect;
	ValidRect(&r_temp);
	InvalRect(&r_temp);

}// end of SizeScroll Bars member fuction



// When things happen which could have changed the
// scroll parameters and these routeens is used to get them back
// in the correct form.

void	DScrollWindow::ValidateScrollRange(void)
{
	Rect r;
	short dh, dv;
	short newMax;
	RgnHandle oldClip;
	
	GetContentRect(&r);
	dh = 0; dv = 0;
	newMax = (fVMax - fVMin) - (r.bottom - r.top); 
	// see .h file for comment on control ranges vrs visible ranges
		
	if(newMax < 0)
		newMax = 0;
	if(fVOffSet > newMax)
		dv = fVOffSet - newMax;
	SetCtlMax(fVertScrollBar, newMax);

	newMax = (fHMax - fHMin) - (r.right - r.left);
	if(newMax < 0)
		newMax = 0;
	if(fHOffSet > newMax)
		dh = fHOffSet - newMax;
	SetCtlMax(fHorizScrollBar, newMax);
	
	if(dh | dv)
	{
		FocusOnContent();
		
		GetContentRect(&r);
		InvalRect(&r);
		
		oldClip = NewRgn();
		GetClip(oldClip);
		SetRect(&r, 0, 0, 0, 0);
		ClipRect(&r);
		ScrollContents(dh,dv);
		
		SetClip(oldClip);
		DisposeRgn(oldClip);
	}
}// end of ValidateScrollRange member function


void	DScrollWindow::ValidateVertScrollRange(void)
{
	Rect r;
	short dv;
	short newMax;
	RgnHandle oldClip;
	
	GetContentRect(&r);
	dv = 0;
	newMax = (fVMax - fVMin) - (r.bottom - r.top);
	if(newMax < 0)
		newMax = 0;
	if(fVOffSet > newMax)
		dv = fVOffSet - newMax;
	SetCtlMax(fVertScrollBar, newMax);
	
	if(dv)
	{
		FocusOnContent();
		
		GetContentRect(&r);
		InvalRect(&r);
		
		oldClip = NewRgn();
		GetClip(oldClip);
		SetRect(&r, 0, 0, 0, 0);
		ClipRect(&r);
		ScrollContents(0,dv);
		
		SetClip(oldClip);
		DisposeRgn(oldClip);
	}
}// end of ValidateVertScrollRange member function



void	DScrollWindow::SynchScrollBars(void)
{
	ValidateScrollRange();
	FocusOnWindow();
	SetCtlValue(fHorizScrollBar, fHOffSet);
	SetCtlValue(fVertScrollBar, fVOffSet);
	
}// end of SynchScrollBars member function




void	DScrollWindow::ScrollClick(EventRecord *theEvent)
{
	ControlHandle whichControl;
	short part;
	
	FocusOnWindow();
	if(part = FindControl(theEvent->where, fWindowPtr, &whichControl) )
	{
		switch(part)
		{
			case inThumb:
				DoThumbScroll(whichControl, theEvent->where);
				break;
			case inUpButton:
			case inDownButton:
				DoButtonScroll(whichControl, theEvent->where);
				break;
			case inPageUp:
			case inPageDown:
				DoPageScroll(whichControl,part);
				break;
		}/*end of switch*/
	}
}// end of ScrollClick member function

	
void	DScrollWindow::DoThumbScroll(ControlHandle theControl, Point localPt)
{
	short oldValue, trackResult, newValue, diff;
	
	oldValue = GetCtlValue(theControl);
	trackResult = TrackControl(theControl, localPt, NULL);
	if(trackResult != 0)
	{
		newValue = GetCtlValue(theControl);
		diff = newValue - oldValue;

		if(theControl == fHorizScrollBar)
			DoHScroll(diff);
		if(theControl == fVertScrollBar)
			DoVScroll(diff);
	}
}// end of DoThumbScroll member function

	


void	DScrollWindow::DoPageScroll(ControlHandle theControl, short part)
{
	short scrollAmount, currentPart;
	Point thePt;
	
	if(theControl == fVertScrollBar)
	{
		scrollAmount = GetVertPageScrollAmount();
		do
		{
			GetMouse(&thePt);
			currentPart = TestControl(theControl, thePt);
			if(currentPart == part)
			{
				if(currentPart == inPageUp)
					DoVScroll(- scrollAmount);
				if(currentPart == inPageDown)
					DoVScroll(scrollAmount);
			}
		}while(Button());
	}// end if VeriticalScrollBar
	else if (theControl == fHorizScrollBar)
	{
		scrollAmount = GetHorizPageScrollAmount();
		do
		{
			GetMouse(&thePt);
			currentPart = TestControl(theControl, thePt);
			if(currentPart == part)
			{
				if(currentPart == inPageUp)
					DoHScroll(- scrollAmount);
				if(currentPart == inPageDown)
					DoHScroll(scrollAmount);
			}
		}while(Button());
	}// end ifHorizontal Scroll Bar
}// end of DoPageScroll function



void	DScrollWindow::DoButtonScroll(ControlHandle theControl, Point localPt)
{
	short result;
	
	result = TrackControl(theControl, localPt, (ProcPtr) ActionProc) ;
}// end of DoButtonScroll member function



	
void	DScrollWindow::ScrollContents(short dh, short dv)
{
	Rect r;
	RgnHandle updateRgn;
	
	GetContentRect(&r);
	updateRgn = NewRgn();
	
	ScrollRect(&r,dh,dv,updateRgn);
	
	fVOffSet -= dv;
	fHOffSet -= dh;
	
	InvalRgn(updateRgn);
	HandleUpdateEvt(NULL);
	DisposeRgn(updateRgn);
}// end of ScrollContents function



void	DScrollWindow::DoHScroll( short change)
{
	RgnHandle oldClip;
	short diff;
	short newValue;
	short oldMin, oldMax;
		// These variables are to allow the use of different
		// ranges in the fHOffSet, fHMax and fHMins than possible
		// in the controls.  It is a result of the fact that the settings
		// of the controls and the settings of the offset, max and min are
		// not equal, becuase the scroll is scrolling the origin suchthat
		// the entier range defined by fHMin and fHMax are accesibl
		// the windows settings.(its best to stay close to that state)

		// In particualar its the control's max value wich is not
		// equall to the fHMax/fVMax values,  The mins are equal in 
		// the applications I've created so far but just to be on the 
		// safe side, I'll check the mins too.

	oldClip = NewRgn();
	GetClip(oldClip);
	diff = 0;
	newValue = fHOffSet + change;
	
	if(change<0)
	{
		oldMin = GetCtlMin(fHorizScrollBar);
		if(newValue < oldMin)
			newValue = oldMin;
	}
	else
	{
		oldMax = GetCtlMax(fHorizScrollBar);
		if(newValue > oldMax)
			newValue = oldMax;
	}
	diff = fHOffSet - newValue;
	
	FocusOnContent();
	ScrollContents(diff,0);
		// changes the offsets
	
	FocusOnWindow();
	SetCtlValue(fHorizScrollBar, fHOffSet);
		// now bring the control up to date with the offsets
	
	SetClip(oldClip);
	DisposeRgn(oldClip);
}// end of DoHScroll member function


void	DScrollWindow::DoVScroll(short change)
{
	RgnHandle oldClip;
	short diff;
	short newValue;
	short oldMin, oldMax;
		// this change in theam is to allow the use of different
		// dinamic ranges in the fHOffSet, Max and Mins than possible
		// in the controls.  It also admits to the fact that the settings
		// of the controls and the settings of the offset, max and min are
		// not necessaraly identical (its best to stay close to that state)

	
	oldClip = NewRgn();
	GetClip(oldClip);
	diff = 0;

	newValue = fVOffSet + change;
	
	if(change<0)
	{
		oldMin = GetCtlMin(fVertScrollBar);
		if(newValue < oldMin)
			newValue = oldMin;
	}
	else
	{
		oldMax = GetCtlMax(fVertScrollBar);
		if(newValue > oldMax)
			newValue = oldMax;
	}
	diff = fVOffSet - newValue;
	
	FocusOnContent();
	ScrollContents(0, diff);
		// changes the offsets
	
	FocusOnWindow();
	SetCtlValue(fVertScrollBar, fVOffSet);
		// now bring the control up to date with the offsets

	
	SetClip(oldClip);
	DisposeRgn(oldClip);
}// DoVScroll member function


	
pascal void DScrollWindow::ActionProc(ControlHandle theControl, short partCode)
{
	short scrollAmount = 0;
	DScrollWindow *theCurrScrollWindow;
	
	theCurrScrollWindow = (DScrollWindow *) GetWRefCon( FrontWindow() );// isn't system7 great?
	
	if(theControl == theCurrScrollWindow->fVertScrollBar )
	{
		scrollAmount = theCurrScrollWindow->GetVertLineScrollAmount();
		if(partCode == inUpButton)
			theCurrScrollWindow->DoVScroll(-scrollAmount);
		if(partCode == inDownButton)
			theCurrScrollWindow->DoVScroll(scrollAmount);
	}
	else if( theControl == theCurrScrollWindow->fHorizScrollBar)
	{
		scrollAmount = theCurrScrollWindow->GetHorizLineScrollAmount();
		if(partCode == inUpButton)
			theCurrScrollWindow->DoHScroll(-scrollAmount);
		if(partCode == inDownButton)
			theCurrScrollWindow->DoHScroll(scrollAmount);
	}

}// end of ActionProc proc
