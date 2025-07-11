#include "stuff.h"
#include <GestaltEqu.h>

static long progress;
static Boolean autoquit = true;

TEHandle	TEH;
WindowPtr	progressWindow;
Rect		dragRect;
Rect		windowBounds = { 40, 40, 200, 300 };

void ChkOsErr(OSErr errcode)
	{
	char errno[9],*p = &errno[sizeof(errno)];
	if (errcode < 0)
		{
		errcode = -errcode;
		while (errcode > 0)
			{
			*--p = errcode%10+'0';
			errcode /= 10;
			}
		*--p = '-';
		*--p = &errno[sizeof(errno)]-p;
		ParamText((void *)p, 0, 0, 0);
		Alert(129, 0);
		ExitToShell();
		}
	}

OSErr GotRequiredParams (AppleEvent *theEvent)
	{
	DescType	returnedType;
	long			actualSize;
	OSErr		err;
	
	err = AEGetAttributePtr (theEvent, keyMissedKeywordAttr, typeWildCard, &returnedType, NULL, 0, &actualSize);
	if (err == errAEDescNotFound)
		return noErr;
	else if (err == noErr)
		return errAEEventNotHandled;
	else
		return err;
	}

pascal OSErr HandleOapp (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon)
	{
	autoquit = false;
	return noErr;
	}

pascal OSErr HandleOdoc (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon)
	{
	register OSErr		err;
	AEDescList		docList;
	
	err = AEGetParamDesc (theAppleEvent, keyDirectObject, typeAEList, &docList);
	if (err == noErr) 
		{
		err = GotRequiredParams (theAppleEvent);
		if (err == noErr)
			{
			long			numDocs, i, actualSize;
			Boolean		wasAlias,isFolder;
			OSErr		err;
			AEKeyword	keyword;
			DescType	returnedType;
	
			err = AECountItems (&docList, &numDocs);
			if (err == noErr) 
				{
				BeginProcessing ();
				for (i = 1; i <= numDocs; i++) 
					{
					FSSpec curDocFSS;
					err = AEGetNthPtr (&docList, i, typeFSS, &keyword, &returnedType,
								(Ptr) &curDocFSS, sizeof (FSSpec), &actualSize);
					if (err == noErr) 
						{
						err = ResolveAliasFile (&curDocFSS, true, &isFolder, &wasAlias);
						if (err == noErr) 
							{
							if (isFolder) scanfolder(&curDocFSS);
							else ProcessDoc(&curDocFSS, 0x0);
							}
						}
					}
				err = noErr;
				EndProcessing (numDocs);
				if (autoquit) ExitToShell();
				}
			}
		(void) AEDisposeDesc (&docList);
		}
		
	return err;
	}

pascal OSErr HandlePdoc (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon)
{
	return errAEEventNotHandled;
}

pascal OSErr HandleQuit (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon)
{
	ExitToShell();
	return noErr;
}

pascal OSErr ReturnEventNotHandled (AppleEvent *theAppleEvent, AppleEvent *theReply, long refcon)
{
	return errAEEventNotHandled;
}

#define TrapMask 0x0800

static short NumToolboxTraps( void )
{
	if (NGetTrapAddress(0xA86E, ToolTrap) ==
			NGetTrapAddress(0xAA6E, ToolTrap))
		return(0x0200);
	else
		return(0x0400);
}

static TrapType GetTrapType(short theTrap)
{

	if ((theTrap & TrapMask) > 0)
		return(ToolTrap);
	else
		return(OSTrap);

}

static Boolean TrapAvailable(short theTrap)
{

	TrapType	tType;

	tType = GetTrapType(theTrap);
	if (tType == ToolTrap)
	theTrap = theTrap & 0x07FF;
	if (theTrap >= NumToolboxTraps())
		theTrap = 0xA89F;

	return (NGetTrapAddress(theTrap, tType) !=
			NGetTrapAddress(0xA89F, ToolTrap));
}

void main(void)
{
	int major = 5,minor;
	long gestaltAnswer;
	InitMacintosh();
	if (TrapAvailable(0xA1AD))
		{
		Gestalt(gestaltSystemVersion, &gestaltAnswer);
		major = gestaltAnswer>>8;
		minor = gestaltAnswer&255;
		}
	if (major < 7)
		{
		ParamText("\p: System 7 required",0,0,0);
		Alert(129, 0);
		ExitToShell();		
		}
	SetUpMenus();
	SetUpWindow();

/* maintain compatibility with non universal headers */
//#ifndef NewAEEventHandlerProc
//#define NewAEEventHandlerProc(x) x
//#endif

	AEInstallEventHandler (kCoreEventClass, kAEOpenApplication, NewAEEventHandlerProc(HandleOapp), 0, false);
	AEInstallEventHandler (kCoreEventClass, kAEOpenDocuments, NewAEEventHandlerProc(HandleOdoc), 0, false);
	AEInstallEventHandler (kCoreEventClass, kAEPrintDocuments, NewAEEventHandlerProc(HandlePdoc), 0, false);
	AEInstallEventHandler (kCoreEventClass, kAEQuitApplication, NewAEEventHandlerProc(HandleQuit), 0, false);
	for (;;)
		HandleEvent();
}
/* end main */



MenuHandle	appleMenu, fileMenu, editMenu;

enum	{
	appleID = 1,
	fileID,
	editID
	};

enum	{
	openItem = 1,
	closeItem,
	quitItem = 4
	};

void SetUpMenus(void)
{
	InsertMenu(appleMenu = NewMenu(appleID, "\p\024"), 0);
	InsertMenu(fileMenu = NewMenu(fileID, "\pFile"), 0);
	DrawMenuBar();
	AddResMenu(appleMenu, 'DRVR');
	AppendMenu(fileMenu, "\pQuit/Q");
}
/* end SetUpMenus */

void HandleMenu (long mSelect)
{
	int			menuID = HiWord(mSelect);
	int			menuItem = LoWord(mSelect);
	Str255		name;
	GrafPtr		savePort;
	WindowPeek	frontWindow;
	
	switch (menuID)
	  {
	  case	appleID:
		GetPort(&savePort);
		GetItem(appleMenu, menuItem, name);
		OpenDeskAcc(name);
		SetPort(savePort);
		break;
	
	  case	fileID:
		ExitToShell();
		break;
  				
	  }
}
/* end HandleMenu */


void SetUpWindow(void)

{
	dragRect = qd.screenBits.bounds;
	
	progressWindow = NewWindow(0L, &windowBounds, "\pprogress", true, noGrowDocProc, (WindowPtr) -1L, true, 0);
	SetPort(progressWindow);
	TEH = TENew(&windowBounds, &windowBounds);
}
/* end SetUpWindow */


void InitMacintosh(void)

{
	MaxApplZone();
	
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();

}
/* end InitMacintosh */

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
	    HandleMenu(MenuSelect(theEvent->where));
	    break;
	    
	  case inDrag:
	  	if (theWindow == progressWindow)
	  	  DragWindow(progressWindow, theEvent->where, &dragRect);
	  	  break;
	  	  
	  case inContent:
	  	if (theWindow == progressWindow)
	  	  {
	  	  if (theWindow != FrontWindow())
	  	    SelectWindow(progressWindow);
	  	  else
	  	    InvalRect(&progressWindow->portRect);
	  	  }
	  	break;
	  	
	  case inGoAway:
	  	if (theWindow == progressWindow && 
	  		TrackGoAway(progressWindow, theEvent->where))
		  HideWindow(progressWindow);
	  	  break;
      }
}
/* end HandleMouseDown */

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
			  HandleMenu(MenuKey((char) (theEvent.message & charCodeMask)));
			  }
			break;
			
		case updateEvt:
			{
			Rect upper = progressWindow->portRect;
			Rect lower = progressWindow->portRect;
			upper.bottom = upper.top + 32;
			lower.top = upper.bottom;
			BeginUpdate(progressWindow);
			SetPort(progressWindow);
			EraseRect(&lower);
			upper.right = ((upper.right-upper.left)*progress>>16)+upper.left;
			FillRect(&upper,(void *)&qd.gray);
			upper.left = upper.right;
			upper.right = lower.right;
			FillRect(&upper,(void *)&qd.white);
			TEUpdate(&progressWindow->portRect, TEH);
			EndUpdate(progressWindow);
		    break;
		    }
		case activateEvt:
			InvalRect(&progressWindow->portRect);
			break;

		case kHighLevelEvent:
			AEProcessAppleEvent (&theEvent);
			break;
	    }
}
/* end HandleEvent */

void	UpdateProgress(long my_progress)
	{
	Rect upper = progressWindow->portRect;
	upper.bottom = upper.top + 32;
	progress += my_progress;
	if (progress > 0x10000L) progress = 0x10000L;
	if (!my_progress) progress = 0;
	InvalRect(&upper);
	HandleEvent();
	}

void	UpdateFileName(FSSpec *new)
	{
	Rect upper = progressWindow->portRect;
	Rect lower = progressWindow->portRect;
	upper.bottom = upper.top + 32;
	lower.top = upper.bottom;
	TESetText(&(new->name[1]),new->name[0],TEH);
	InvalRect(&lower);
	HandleEvent();
	}

