/*
	File:		DScrollWindow.h

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	� 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <3>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <2>	11/14/92	MTG		Bringing the C++ version up to date WRT the         ThinkC
									version.

	To Do:
*/


// This is the implementation of TScrollDoc subclass in my 
// class library.  

#ifndef __DSCROLLWINDOW__
#define __DSCROLLWINDOW__

#include "DWindow.h"


class DScrollWindow : public DWindow
{
protected:

	short			fVMin;
	short			fVMax;
	
	short			fHMin;
	short			fHMax;
		// these Max and Min's are the extreams of all that (the bounds of visibility)
		// is visable by working the scroll bar control.  This is not the
		// same as the ranges used in the scroll bar controls (fHMax will
		// be bigger than the fHorizScrollBar MaxValue by the number 
		// of pixles across the content rect for most scrollbar applications)
	
	ControlHandle	fHorizScrollBar;
	ControlHandle	fVertScrollBar;

public:
	DScrollWindow(void);
	~DScrollWindow(void);
		
		//DWindow OverRides
	virtual Boolean	Init(DDocument *doc, Boolean hasColorWindows);

	virtual Boolean 	KillMeNext(void);
	virtual	void	HandleUpdateEvt(EventRecord *theEvent);
	virtual void 	HandleActivateEvt(EventRecord *theEvent);
	virtual void 	HandleOSEvent(EventRecord *theEvent);
		// needed to hide the scroll bars when a context switch happens 
	
		//New To DScrollWindow
	virtual	short	GetVertLineScrollAmount(void);
	virtual	short	GetHorizLineScrollAmount(void);
	
	virtual	short	GetVertPageScrollAmount(void);
	virtual	short	GetHorizPageScrollAmount(void);
	
protected:

		//DWindow OverRides
	virtual	void	DoGrow(EventRecord *theEvent);
	virtual	void	DoZoom(short partCode);
	virtual	void	DoContent(EventRecord *theEvent);

		//New To DScrollWindow
	
	void	SizeScrollBars(void);

public:

	void	ValidateScrollRange(void);
			// Call whenever possible scroll range or window size change.
			
	void	ValidateVertScrollRange(void);
			// Call for small changes common in TextEdit type applications
			// which only need to wory about the vertical changes to the
			// scroll parameters, Like when some text is added/deleted
			// from a document.

	void	SynchScrollBars(void);
			// validate ranges and set thumb setting correctly
	
protected:

	void	ScrollClick(EventRecord *theEvent);
	void	DoThumbScroll(ControlHandle theControl, Point localPt);
	void	DoPageScroll(ControlHandle theControl, short part);
	void	DoButtonScroll(ControlHandle theControl, Point localPt);
	
	virtual void	ScrollContents(short dh, short dv);
			// The scroll operation bottelneck procedure for doing
			// the scroll.  It pressently implements the scrolling
			// common for image types of applications. (TE apps will 
			// need to override this)

public:
	
	void	DoHScroll(short change);
	void	DoVScroll(short change);
	
	static pascal void ActionProc(ControlHandle theControl, short partCont);
	
};// end of class declaration for TDocRootd

	


#endif __DSCROLLWINDOW__