/*
	File:		DScribbleWind.c

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.
	Use at your own risk.

*/
// This is the class definition of DScribbleWind

#include "DScribbleWind.h"
#include "DScribbleDoc.h"



DScribbleWind::DScribbleWind(void)
{
	//stub
}

DScribbleWind::~DScribbleWind(void)
{
	//stub
}



Boolean DScribbleWind::Init(DDocument *doc, Boolean hasColorWindows)
{
	Boolean inheritedSuccess;
	PicHandle		thePict;

	thePict = ((DScribbleDoc *) doc)->fPict;
	
	fPenSize = 1;
	fPenPat = patBlack;
	
	inheritedSuccess = inherited :: Init(doc, hasColorWindows);
	if(thePict != NULL)
	{
		fVMin = 0;
		fVMax = (**thePict).picFrame.bottom - (**thePict).picFrame.top;
		
		fHMin = 0;
		fHMax = (**thePict).picFrame.right - (**thePict).picFrame.left;
	}

	return (inheritedSuccess);
}// end of init function


void	DScribbleWind::DoContent(EventRecord* theEvent)
{	
	Rect contents;
	Point	newPoint;
	PicHandle thePict;
	
	FocusOnWindow();
	GlobalToLocal(&theEvent->where);
	GetContentRect(&contents);
	if(PtInRect(theEvent->where, &contents))
	{
		PenSize(fPenSize, fPenSize);
		
		if(fPenPat == patBlack)
			PenPat(black);
		if(fPenPat == patGray)
			PenPat(gray);
		if(fPenPat == patWhite)
			PenPat(white);
		FocusOnContent();
		GetMouse( &newPoint);
		MoveTo(newPoint.h, newPoint.v);
		do
		{
			GetMouse( &newPoint);
				LineTo(newPoint.h, newPoint.v);

		} while(StillDown());
		
		fDoc->fNeedToSave = TRUE;
		thePict = ((DScribbleDoc *) fDoc)->fPict;
		if( thePict != NULL)
			KillPicture( thePict);
			
		thePict = OpenPicture(&fWindowPtr->portRect);
		CopyBits(&fWindowPtr->portBits, &fWindowPtr->portBits,
				&fWindowPtr->portRect, &fWindowPtr->portRect, srcCopy, NULL);
		ClosePicture();
		((DScribbleDoc *) fDoc)->fPict = thePict;
	}// end if in content rect
	else
				ScrollClick(theEvent);

}// end of DoContent function


void	DScribbleWind::Draw(Rect *r)
{
	if(((DScribbleDoc *) fDoc)->fPict != NULL)
		DrawPicture(((DScribbleDoc *) fDoc)->fPict, &( (*(((DScribbleDoc *) fDoc)->fPict))->picFrame ) );
}
