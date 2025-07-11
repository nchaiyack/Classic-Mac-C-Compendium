/*
	File:		DTEditWind.h

	Written by:	Mark Gross

	Copyright:	� 1992 by Applied Technical Software, all rights reserved.
	Use at your own risk.

*/

// This is the class declaration of the DTEditWind subclass of
// DScrollWindow

#ifndef __DTEDITWIND__
#define __DTEDITWIND__

#include "DScrollWindow.h"

class DTEditWind : public DScrollWindow
{
protected:

	TEHandle fDocText;
		// a handy refrence to the TEditDocs fText field;
		
public:

	DTEditWind(void);
	~DTEditWind(void);
		// stub con/de structors... here just for style
		
	virtual Boolean Init(DDocument *doc, Boolean hasColorWindows);
	virtual void	Draw(Rect *r);

	virtual void HandleKeyDown(EventRecord *theEvent);					
	virtual void HandleActivateEvt(EventRecord *theEvent);
	virtual void HandleNullEvent(EventRecord *theEvent);
	virtual void HandleOSEvent(EventRecord *theEvent);
				// Supports the deacivation of the windows highlighted areas
				// on a context switch.

	virtual void	FocusOnContent(void);
		//over ridden to take into account that the TE scrolling
		// is different in that TEScroll needs the qd origen to not
		// change for it to work correctly.

	void	SynchScrollBars(void);
		// Make sure that the valus of the mins,maxs, and offestes
		// are right witht the TE world.
protected:

	void	ScrollContents(short dh, short dv);
		// TE scroll is requiers this to be over ridden
		
	virtual void	DoContent(EventRecord* theEvent);
	virtual	void	DoGrow(EventRecord *theEvent);
	virtual	void	DoZoom(short partCode);
};// end of class declaration


#endif __DTEDITWIND__