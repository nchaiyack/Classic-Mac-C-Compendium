#include "mandel.h"


RestartProc()
{
	ExitToShell();
}



main()
{
EventRecord		theEvent;
WindowPtr		whichWindow;
char			theChar;

	InitGraf(&thePort);
	InitFonts();
	FlushEvents( everyEvent, 0 );
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(RestartProc);
	InitCursor();
	MaxApplZone();
	
	receiveRecord = (ABRecHandle) NewHandle(atpSize);
	
	if (receiveRecord == 0L)
		FatalError("\p Can't allocate memory for receiveRecord");
	
	nbpRegistered = FALSE;
	nbpName = 0L;
	
	if (OptionKey())
	{
		ErrorAlert("\p Master mode enabled");
		slaveMode = FALSE;
	}
	else
		slaveMode = TRUE;
	
	shading = TRUE;
	numSlaves = 0;
	coords = FALSE;
	meToo = TRUE;
	timeDelay = 3.0;
	
	theDisplayMessages[0][0] = 0;
	theDisplayMessages[1][0] = 0;
	theDisplayMessages[2][0] = 0;
	
	lims[xmin] = -2.0;
	lims[xmax] = 1.0;
	lims[ymin] = -1.5;
	lims[ymax] = 1.5;
	
	maxIterations = 10;
	pixelSize = 1;
	
	SetUpCursors();
	SetUpMenus();
	
	InitWindow();
	
	if (SetupAppleTalk() != noErr)
		FatalError("\p Can't set up AppleTalk!");
	
	DrawInfo();
	
	FlushEvents(everyEvent,0);
	
	AwaitRequest(receiveRecord,&theMessage);
	
	alive = TRUE;

	while (alive)
	{
		SystemTask();
		MaintainMenus();
		MaintainCursor();
		
		if (GetNextEvent(everyEvent,&theEvent))
		{
			switch (theEvent.what)
			{
				case networkEvt:
					if (slaveMode==TRUE && theEvent.message==(long) receiveRecord)
					{
						if (RespondToRequest(receiveRecord) == noErr)
							AwaitRequest(receiveRecord,&theMessage);
					}
					
					break;
				
				case mouseDown:
					switch (FindWindow(theEvent.where,&whichWindow)) 
					{
						case inMenuBar:
							DoCommand(MenuSelect(theEvent.where));
							break;
							
						case inSysWindow:
							SystemClick(&theEvent,whichWindow);
							break;
							
						case inContent:
							if (whichWindow != FrontWindow())
								SelectWindow(whichWindow);
							
							else if (whichWindow == theWPtr)
								DoContent(&theEvent);
							
							break;
						
						case inDrag:
							if (whichWindow != FrontWindow())
								SelectWindow(whichWindow);
							
							else if (whichWindow == theWPtr)
								DoDrag(whichWindow,&theEvent);
							
							break;
					}
					
					break;
				
			case keyDown:
			case autoKey: 
				
				theChar = theEvent.message & charCodeMask;
					
				if (theEvent.modifiers & cmdKey)
					DoCommand(MenuKey(theChar));
				
				else
					SysBeep(1);
				
				break;
				
			case activateEvt:
				if ((WindowPtr) theEvent.message == theWPtr) 
				{
					if (theEvent.modifiers & activeFlag) 
					{
						
					}
					else 
					{
						
					}
				}
				
				break;
				
			case updateEvt: 
				if ((WindowPtr) theEvent.message == theWPtr) 
				{
					UpdateWindow();
				}
				
				break;
			}
		}
	}
	
	
ExitPoint:
	ShutDownAppleTalk();
	
	DisposHandle(receiveRecord);
	DisposPtr(graphBits.baseAddr);
	
	return(0);
}