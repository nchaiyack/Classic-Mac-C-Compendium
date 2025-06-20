/*
	File:		DScribbleWind.h

	Written by:	Mark Gross

	Copyright:	� 1992 by Applied Technical Software, all rights reserved.
	Use at your own risk.

*/

// This is the class declaration of the DScribbleWindow
// sub-class to ScrollWindow for the Scribble application
#ifndef __DSCRIBBLEWIND__
#define __DSCRIBBLEWIND__

#ifndef penPat
	typedef enum {patBlack, patGray, patWhite} penPat;
#endif

//class DScrollWindow;
#include "DScrollWindow.h"

class DScribbleWind : public DScrollWindow
{

public:
	short			fPenSize;
	penPat			fPenPat;
	
	DScribbleWind(void);
	~DScribbleWind(void);
		// stub con/de structors... here just for style
		
		
	virtual Boolean Init(DDocument *doc, Boolean hasColorWindows);
		// Set up the scroll parameters for the window, and the above memebers
		
	virtual void	Draw(Rect *r);
		// This method simply draws the pict in the DScribbleDoc
		// member variabble DScribbleDoc::fPict

protected:

	virtual void	DoContent(EventRecord* theEvent);
		// This member dose the drawing and marks the document 
		// as needing saving.
		
};// end of class declaration


#endif __DSCRIBBLEWIND__