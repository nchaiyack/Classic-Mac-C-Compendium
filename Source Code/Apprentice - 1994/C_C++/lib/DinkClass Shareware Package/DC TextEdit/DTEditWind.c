/*
	File:		DTEditWindow.c

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.
	Use at your own risk.

*/

// This is the class definition of DTEditWind

#include "DTEditDoc.h"
#include "DTEditWind.h"
#include "DApplication.h"
#include <DinkUtils.h>


DTEditWind::DTEditWind(void)
{
	//stub
}

DTEditWind::~DTEditWind(void)
{
	//stub
}



Boolean DTEditWind::Init(DDocument *doc, Boolean hasColorWindows)
{
	Boolean inheritedSuccess;
	Rect vRect;
	
	inheritedSuccess = inherited::Init(doc, hasColorWindows);

	fDocText = ((DTEditDoc*)doc)->fText;
	
	fNextHandler = doc;
	SetPort(fWindowPtr);
	( *fDocText)->inPort = fWindowPtr;
	( *fDocText)->txSize = 9;
	( *fDocText)->txFont = monaco;
	
	fVMax = (*fDocText)->nLines * (*fDocText)->lineHeight;
	fVMin = 0;
	
	fHMax = ( (*fDocText)->destRect.right - (*fDocText)->destRect.left );
	fHMin = 0;
	
	return (inheritedSuccess);
}// end of init function



void DTEditWind::SynchScrollBars(void)
{
	Rect visible, dest;
	
	visible = (*fDocText)->viewRect;
	dest = (*fDocText)->destRect;

	fVMax = (*fDocText)->nLines * (*fDocText)->lineHeight;
	fVMin = 0;
	
	fVOffSet = visible.top - dest.top;
	fHOffSet = visible.left - dest.left;
		
	inherited::SynchScrollBars();
}// end of SynchScrollBars member function



void DTEditWind::HandleKeyDown(EventRecord *theEvent)				
{
	TEKey(LoWrd(theEvent->message), fDocText);
	fDoc->fNeedToSave = TRUE;
	SynchScrollBars();

} // end of HandleKeyDown member function


void DTEditWind::HandleActivateEvt(EventRecord *theEvent)
{
	Boolean	activating;
	
	inherited:: HandleActivateEvt(theEvent);
	
	activating = (theEvent->modifiers & activeFlag);
	if(activating)
	{
		TEActivate( fDocText );
	}
	else
	{
		TEDeactivate( fDocText );
	}
	
	inherited:: HandleActivateEvt(theEvent);

}// end of HandleActivateEvt member function


void DTEditWind::HandleOSEvent(EventRecord *theEvent)
{
	if(gApplication->fInBackground)
	{
		TEDeactivate( fDocText );
	}
	else
	{
		TEActivate( fDocText );
	}
	
	inherited::HandleOSEvent(theEvent);
		
}// end of HandleActivateEvt member function



void DTEditWind::HandleNullEvent(EventRecord *theEvent)
{
	Rect	r;
	Point whereIsMouse;
	GrafPtr	oldPort;
	CursHandle	IBeam;

	if (!gApplication->fInBackground)
	{
		GetPort(&oldPort);
		SetPort(fWindowPtr);
		GetMouse(&whereIsMouse);
	
		GetContentRect(&r);
		if( PtInRect(whereIsMouse, &r) )
		{
			IBeam = GetCursor(iBeamCursor);
			if(IBeam != NULL)
				SetCursor(*IBeam);
			
		}
		else
			InitCursor();
	
		SetPort(oldPort);
		TEIdle( fDocText );
	}// end if in forground
}



void	DTEditWind::DoContent(EventRecord* theEvent)
{	
	Rect contents;
	Boolean shiftOn;
	Point	newPoint;
	
	shiftOn = ( (theEvent->modifiers & shiftKey) != 0);
	FocusOnWindow();
	GlobalToLocal(&theEvent->where);
	GetContentRect(&contents);
	if(PtInRect(theEvent->where, &contents))
	{		
		TEClick(theEvent->where, shiftOn, fDocText);
	}// end if in content rect
	else
		ScrollClick(theEvent);

}// end of DoContent function


void	DTEditWind::Draw(Rect *r)
{
	if( fDocText)
	{
		EraseRect(r);
		TEUpdate(r, fDocText);
	}
	
}// end of Draw member function


// the grow methods only need to update the viewRect for the TEHandle
void	DTEditWind::DoGrow(EventRecord *theEvent)
{
	Rect r;
	
	inherited::DoGrow(theEvent);
	
	GetContentRect(&r);
	(**fDocText).viewRect = r;
	
}// end of DoGrow override

void	DTEditWind ::DoZoom(short partCode)
{
	Rect r;

	inherited::DoZoom(partCode);
	
	GetContentRect(&r);
	(**fDocText).viewRect = r;
	
}// end of DoZoom member function



void	DTEditWind::ScrollContents(short dh, short dv)
{

	TEScroll(dh, dv, fDocText );
	
	fVOffSet = (*fDocText)->viewRect.top - (*fDocText)->destRect.top;
	fHOffSet = (*fDocText)->viewRect.left - (*fDocText)->destRect.left;
	
}// end of ScrollContents function


void	DTEditWind::FocusOnContent(void)
{
	Rect r;
	
	SetPort(fWindowPtr);
	GetContentRect(&r);
	ClipRect(&r);
	
}// end of FocusOnContent member
