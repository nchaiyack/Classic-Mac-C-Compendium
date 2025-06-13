// File "floaters.h" - 

#ifndef ____FLOATERS_HEADER____
#define ____FLOATERS_HEADER____

// * ****************************************************************************** *
// * ****************************************************************************** *

// Declare a Guide for Passing the Filter Helper, for Universal Headers
typedef void (*FloaterEventProcPtr)(EventRecord *floatEvent, WindowPtr floatWindow);
typedef void (*FloaterCloseProcPtr)(WindowPtr floatWindow);
		
// * ****************************************************************************** *

typedef struct FloaterQElem {
	struct FloaterQElem *qLink;
	short qType;
	
	Boolean hideMe;
	long hotApplication;

	WindowPtr floatWindow;
	FloaterEventProcPtr eventHandler;
	FloaterCloseProcPtr closeHandler;
	} FloaterQElem, *FloaterQElemPtr;
	
// * ****************************************************************************** *
// * ****************************************************************************** *
// Function Prototypes

WindowPtr NewFloater(Ptr wStorage, Rect *bounds, Str255 title, Boolean visFlag,
		short wdefProc, WindowPtr behind, Boolean closeFlag, long refCon, long hotApp, 
		FloaterEventProcPtr eventHandler, FloaterCloseProcPtr closeHandler);
void CloseFloater(WindowPtr win);
void DisposeFloater(WindowPtr win);

FloaterQElemPtr GetOneFloater(WindowPtr win, Boolean dequeueIt);
FloaterQElemPtr GetIndFloater(short index, Boolean dequeueIt);

Boolean ShowHideFloater(WindowPtr win);
void UpdateFloater(WindowPtr win);
void EventDispatchFloaters(EventRecord *theEvent, WindowPtr win);
void CloseRemainingFloaters(void);

Boolean GetFloaterEvent(EventRecord *theEvent);
Boolean TestScreenSaver(void);

#endif ____FLOATERS_HEADER____
