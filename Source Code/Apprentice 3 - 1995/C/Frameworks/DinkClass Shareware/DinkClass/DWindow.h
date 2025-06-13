/*
	File:		DWindow.h

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <4>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <3>	11/14/92	MTG		Bringing the C++ version up to date WRT the ThinkC version.
		 <2>	 9/20/92	MTG		Bringing C++ version up to date with the THINK C version

	To Do:
*/


// This is the class declaration of the DWindow object

#ifndef __DWINDOW__
#define __DWINDOW__

#include "DEventHandler.h"
#include <Printing.h>

class DDocument;	// forard declaration of the DDocument class
					// its refrence is needed for the getting of
					// the data needed to render the window
					

class DWindow : public DEventHandler
{
protected:

	short			fVOffSet;
	short			fHOffSet;
		// These offests are used in the "Focusing" opperations
		// which are used when multiple coordinate systems
		// are needed (like when scrolling)
		
	THPrint			fPrintRecord;
	Rect			fGrowWindowLimits;
		// the max and mins that the window is allowd to grow.

public:

	DDocument *fDoc;

	WindowPtr fWindowPtr;
		
	DWindow(void);

	~DWindow(void);
	
	virtual Boolean Init(DDocument *doc, Boolean hasColorWindows);
	
	virtual void	FocusOnContent(void);	
		// set the QuickDraw globals to use the offsets
		// to define the drawing coordinate system
		
	void	FocusOnWindow(void);
		// Set the drawing coordinate system to be screen coordinates
	
	void	GetContentRect(Rect *r);
		// return the rectangle defined by the intierior of a window

	virtual void Draw( Rect *area);
			// This gets called from the upDate Event handler
			// override for this subclass of DEventHandler
	
	virtual Boolean KillMeNext(void);
			//  in responce to 
			// a mouseDown event in the close box of the window or the
			// prober menu selection.

	virtual void HandleMouseDown(EventRecord *theEvent, short thePart, WindowPtr theWindow);

	virtual void HandleUpdateEvt(EventRecord *theEvent);
	virtual void HandleActivateEvt(EventRecord *theEvent);
	
	void SetWindowTitle(void);
	
protected:
	virtual void DoDrag(EventRecord *theEvent);
	virtual void DoGrow(EventRecord *theEvent);
	virtual void DoZoom(short thePart);
	
	virtual void DoContent(EventRecord *theEvent);

public:

	virtual void DoPageSetUp(void);
	virtual void DoPrint(void);


};// end of class declaration for the DWindow Class of objects

#define rWindowID 1000
#define kStagger	20
#define MINHIGHT	75
#define MINWIDTH	75

#define rHScroll	1
#define rVScroll	2
#define kScrollOverlap	16

#define kScrollBarWidth	16
#define kScrollBarPos	15

#endif __DWINDOW__
