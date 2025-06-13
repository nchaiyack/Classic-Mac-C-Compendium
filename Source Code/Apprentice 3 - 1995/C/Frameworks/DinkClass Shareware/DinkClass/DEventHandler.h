/*
	File:		DEventHandler.h

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <3>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <2>	11/14/92	MTG		Bringing the C++ version up to date WRT the         ThinkC
									version.

	To Do:
*/


// This is the class declaration file for the 
// DEventHandler class  responceble for the implemenation of the 
// menu and event responce modle developed on the train.

// The passing on of events to the nextHandler is determend
// by the state of a global variable gPassItOn if false it won't
// otherwise it will pass the event on to the next DEventHandler
// object.  This global is set true inside the event loop of the 
// application.  It is up to the subclasses to change its value it 
// requiered.  It may happen that theEvent may become a global for the
// sake of speed.
// 
#ifndef __DEVENTHANDLER__
#define __DEVENTHANDLER__

#include "DObject.h"

class DApplication; // forward declaration of application


class DEventHandler : public DObject
{
public:

	static DApplication *gApplication;
		// this is needed to support the cueing up of deletions of
		// event handlers from the list of event Handlers kept by the 
		// application
		                               
	static Boolean gPassItOn;
		// gPassItOn is initialized to true in the event loop.
		// The flag by which the event is passed on or not.	
	
	DEventHandler *fNextHandler;
		// Defines the linked list of DEventHandlers
		
	Boolean fAlive;
		// Needed to Avoid the problem associated with the 
		// clean up process which kills all the fAlive
		// DEventHandlers where some dieing handlers tell
		// depended handlers to die too BUT those dependents STILL get
		// a KillMeNext message because they are still in the
		// list of "Alive" Handlers. This normaly dosen't cause
		// problems except for Document and DWindow objects where
		// it causes trouble in multiple file savings and
		// disposes (like what happens when quitting with lots of open
		// windows) of window related items(crash!).
	
	DEventHandler(void);
		// Instal itself into the event handler list of the
		// application
		
	~DEventHandler(void);	// empty destructor

	
	virtual Boolean KillMeNext(void);
		// frees up all memory and notifies the gApplication that
		// its ready to go and should use the delete opperator on
		// it as soon as its conviniant.  It also has the gApplication
		// object reset linked list, by searching the list of eventHandlers
		// for the ones with THIS as its nextHandler and fixes that guys 
		// nextHandler.
				
//
// The following are the default event handling methods
//		
	virtual void HandleNullEvent(EventRecord *theEvent);
	virtual void HandleActivateEvt(EventRecord *theEvent);

	virtual void HandleAutoKey(EventRecord *theEvent);
	virtual void HandleKeyDown(EventRecord *theEvent);					

	virtual void HandleDiskEvt(EventRecord *theEvent);

	virtual void HandleHighLevelEvent(EventRecord *theEvent);
	virtual void HandleOSEvent(EventRecord *theEvent);
	virtual void HandleUpdateEvt(EventRecord *theEvent);
	
	virtual void HandleMouseDown(EventRecord *theEvent, 
							short thePart, WindowPtr theWindow);


//
// The following are the default MENU handling methods
//		
	virtual void HandleMenuChoice(short menuID, short menuItem);
			// called by application or other eventHandler object
			
	void EnableMenuItem(MenuHandle menu, short item, Boolean enable);
	
	virtual void SetUpMenues(void);
			// sets up menues in responce to a MouseDown event.
			// it passes on this measeage to the entier chain of
			// eventHandlers

}; // end of Class declaration

#endif __DEVENTHANDLER__

