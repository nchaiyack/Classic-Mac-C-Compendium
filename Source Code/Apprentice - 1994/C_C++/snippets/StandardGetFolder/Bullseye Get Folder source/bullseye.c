/*****
 * bullseye.c
 *
 *	A simple demonstration program to play with the debugger
 *
 *
 *****/

#include "bullMenus.h"
#include "bullWindow.h"

extern	WindowPtr	bullseyeWindow;
extern	Rect		dragRect;

void InitMacintosh(void);
void HandleMouseDown (EventRecord	*theEvent);
void HandleEvent(void);

/****
 * InitMacintosh()
 *
 * Initialize all the managers & memory
 *
 ****/

void InitMacintosh(void)

{
	MaxApplZone();
	
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();

}
/* end InitMacintosh */


/****
 * HandleMouseDown (theEvent)
 *
 *	Take care of mouseDown events.
 *
 ****/

void HandleMouseDown (EventRecord	*theEvent)

{
	WindowPtr	theWindow;
	int			windowCode = FindWindow (theEvent->where, &theWindow);
	
    switch (windowCode)
      {
	  case inSysWindow: 
	    SystemClick (theEvent, theWindow);
	    break;
	    
	  case inMenuBar:
	  	AdjustMenus();
	    HandleMenu(MenuSelect(theEvent->where));
	    break;
	    
	  case inDrag:
	  	if (theWindow == bullseyeWindow)
	  	  DragWindow(bullseyeWindow, theEvent->where, &dragRect);
	  	  break;
	  	  
	  case inContent:
	  	if (theWindow == bullseyeWindow)
	  	  {
	  	  if (theWindow != FrontWindow())
	  	    SelectWindow(bullseyeWindow);
	  	  else
	  	    InvalRect(&bullseyeWindow->portRect);
	  	  }
	  	break;
	  	
	  case inGoAway:
	  	if (theWindow == bullseyeWindow && 
	  		TrackGoAway(bullseyeWindow, theEvent->where))
		  HideWindow(bullseyeWindow);
	  	  break;
      }
}
/* end HandleMouseDown */


/****
 * HandleEvent()
 *
 *		The main event dispatcher. This routine should be called
 *		repeatedly (it  handles only one event).
 *
 *****/

void HandleEvent(void)

{
	int			ok;
	EventRecord	theEvent;

	HiliteMenu(0);
	SystemTask ();		/* Handle desk accessories */
	
	ok = GetNextEvent (everyEvent, &theEvent);
	if (ok)
	  switch (theEvent.what)
	    {
		case mouseDown:
			HandleMouseDown(&theEvent);
			break;
			
		case keyDown: 
		case autoKey:
		    if ((theEvent.modifiers & cmdKey) != 0)
		      {
		      AdjustMenus();
			  HandleMenu(MenuKey((char) (theEvent.message & charCodeMask)));
			  }
			break;
			
		case updateEvt:
			BeginUpdate(bullseyeWindow);
			DrawBullseye(((WindowPeek) bullseyeWindow)->hilited);
			EndUpdate(bullseyeWindow);
		    break;
		    
		case activateEvt:
			InvalRect(&bullseyeWindow->portRect);
			break;
	    }
}
/* end HandleEvent */


/*****
 * main()
 *
 *	This is where everything happens
 *
 *****/


main()

{
	InitMacintosh();
	SetUpMenus();
	SetUpWindow();
	
	for (;;)
		HandleEvent();
}
/* end main */