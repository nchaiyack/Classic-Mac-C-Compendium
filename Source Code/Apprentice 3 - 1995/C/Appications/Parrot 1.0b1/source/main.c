/*------------------------------------------------------------------------------
# Main for Confusion
#
#	Copyright © 1992 Bernard Bernstein
#
#	taken from some background-only sample code and other sample code shells
------------------------------------------------------------------------------*/

#include <memory.h>
#include <appleevents.h>
#include <events.h>
#include <types.h>
#include <gestaltequ.h>

#include <SoundInput.h>
#include <Sound.h>

#include "Confusion.h"

/* variables */
Boolean	gInBackground;
Boolean gQuit = false;
EventRecord gERecord;
Boolean gHasAppleEvents;
unsigned long gMySleep = 60;

struct AEinstalls {
    AEEventClass theClass;
    AEEventID theEvent;
    EventHandlerProcPtr theProc;
};
typedef struct AEinstalls AEinstalls;

/* prototypes for this file */
pascal OSErr AEOpenHandler(AppleEvent *messagein, AppleEvent *reply, long refIn);
pascal OSErr AEOpenDocHandler(AppleEvent *messagein, AppleEvent *reply, long refIn);
pascal OSErr AEPrintHandler(AppleEvent *messagein, AppleEvent *reply, long refIn);
pascal OSErr AEQuitHandler(AppleEvent *messagein, AppleEvent *reply, long refIn);
void DoHighLevel(EventRecord *AERecord);
void InitAEStuff(void);
void DoEvent(EventRecord *event);
void DoMenuCommand(long menuResult);
void AdjustMenus( void );
void Initialize(void);

main()
{
	Boolean gotEvent;
	
	Initialize();
    InitSoundConfusion();
   
    while (gQuit == false) {
		gotEvent = WaitNextEvent(everyEvent, &gERecord, gMySleep, 0);
		if (gotEvent)
			{
			DoEvent(&gERecord);
			}
		if (!gQuit)
			{
			ConfuseSound();
			}
    }
	StopConfusion();
	ExitToShell();
}

void DoEvent(EventRecord *event)
{
	short		part;
	WindowPtr	window;
	Boolean		hit;
	char		key;

	switch ( event->what ) {
		case mouseDown:
			part = FindWindow(event->where, &window);
			switch ( part ) {
				case inMenuBar:				/* process a mouse menu command (if any) */
					AdjustMenus();
					DoMenuCommand(MenuSelect(event->where));
					break;
				case inSysWindow:			/* let the system handle the mouseDown */
					SystemClick(event, window);
					break;
			}
			break;
		case keyDown:
		case autoKey:						/* check for menukey equivalents */
			key = event->message & charCodeMask;
			if ( event->modifiers & cmdKey )			/* Command key down */
				if ( event->what == keyDown ) {
					AdjustMenus();						/* enable/disable/check menu items properly */
					DoMenuCommand(MenuKey(key));
				}
			break;
		case osEvt:
			switch ((event->message >> 24) & 0x0FF) {		/* high byte of message */
				case suspendResumeMessage:		/* suspend/resume is also an activate/deactivate */
					gInBackground = (event->message & resumeFlag	) == 0;
					break;
			}
			break;
		case kHighLevelEvent:
			if (gHasAppleEvents)
				DoHighLevel(&gERecord);
			break;
	}
} /*DoEvent*/


void DoMenuCommand(long menuResult)
{
	short		menuID;				/* the resource ID of the selected menu */
	short		menuItem;			/* the item number of the selected menu */
	short		itemHit;
	Str255		daName;
	short		daRefNum;
	Boolean		handledByDA;

	menuID = HiWord(menuResult);	/* use macros for efficiency to... */
	menuItem = LoWord(menuResult);	/* get menu item number and menu number */
	switch ( menuID ) {
		case mApple:
			switch ( menuItem ) {
				case iAbout:		/* bring up alert for About */
					DoAbout();
					break;
				default:			/* all non-About items in this menu are DAs */
					GetItem(GetMHandle(mApple), menuItem, daName);
					daRefNum = OpenDeskAcc(daName);
					break;
			}
			break;
		case mFile:
			switch ( menuItem ) {
				case iQuit:
					gQuit = true;
					break;
			}
			break;
		case mEdit:					/* call SystemEdit for DA editing & MultiFinder */
			handledByDA = SystemEdit(menuItem-1);	/* since we don’t do any Editing */
			break;
	}
	HiliteMenu(0);					/* unhighlight what MenuSelect (or MenuKey) hilited */
} /*DoMenuCommand*/


void AlertUser(OSErr err, short errID)
{
	short		itemHit;
	Str255		errStr;
	Str255		description;
	long		lErr = (long)err;
	DialogPtr	dlg;
	
	SetCursor(&qd.arrow);
	GetIndString(description, rErrStrings, errID);
	NumToString(lErr, errStr);
	
	ParamText(errStr, description, "\p", "\p");
	dlg = GetNewDialog(rErrAlert, NULL, (WindowPtr)-1L);
	ModalDialog(NULL, &itemHit);
	DisposDialog(dlg);
	
	gQuit = true;
} /* AlertUser */

void AlertUserStr(Str255 description)
{
	short		itemHit;
	DialogPtr	dlg;
	
	SetCursor(&qd.arrow);

	ParamText("\pnone", description, "\p", "\p");
	dlg = GetNewDialog(rErrAlert, NULL, (WindowPtr)-1L);
	ModalDialog(NULL, &itemHit);
	DisposDialog(dlg);
} /* AlertUser */

void AdjustMenus( void )
{

}


void Initialize()
{
	Handle		menuBar;
	EventRecord event;
	short		count;

	gInBackground = false;

	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();

	for (count = 1; count <= 3; count++)
		EventAvail(everyEvent, &event);

    MaxApplZone();
    
	menuBar = GetNewMBar(rMenuBar);			/* read menus into menu bar */
	SetMenuBar(menuBar);					/* install menus */
	DisposHandle(menuBar);
	AddResMenu(GetMHandle(mApple), 'DRVR');	/* add DA names to Apple menu */
	DrawMenuBar();
	
	InitAEStuff();
}


/* InitAEStuff checks for the availability of the AppleEvent Manager and */
/* installs our event handlers. */
/* if the AEM isn't around, we bail. */
void InitAEStuff(void)
{
    static AEinstalls HandlersToInstall[] =  {
         {
            kCoreEventClass, kAEOpenApplication, AEOpenHandler
        },  {
            kCoreEventClass, kAEOpenDocuments, AEOpenDocHandler
        },  {
            kCoreEventClass, kAEQuitApplication, AEQuitHandler
        },  {
            kCoreEventClass, kAEPrintDocuments, AEPrintHandler
        }, 
        /* The above are the four required AppleEvents. */
    };
    
    OSErr aevtErr = noErr;
    long aLong = 0;
    Boolean gHasAppleEvents = false;
    gHasAppleEvents = (Gestalt(gestaltAppleEventsAttr, &aLong) == noErr);
    if (gHasAppleEvents) {
        register qq;
        for (qq = 0; qq < ((sizeof(HandlersToInstall) / sizeof(AEinstalls))); qq++) {
            aevtErr = AEInstallEventHandler(HandlersToInstall[qq].theClass, HandlersToInstall[qq].theEvent,
                                            HandlersToInstall[qq].theProc, 0, false);
            if (aevtErr) {
            	AlertUser(aevtErr, iAEErr);
            }
        }
    }
}
/* end InitAEStuff */

void DoHighLevel(EventRecord *AERecord)
{
    AEProcessAppleEvent(AERecord);
}

pascal OSErr AEOpenHandler(AppleEvent *messagein, AppleEvent *reply, long refIn)
{
#pragma unused (messagein,reply,refIn)
    return(noErr);
}

/* end AEOpenHandler */

pascal OSErr AEOpenDocHandler(AppleEvent *messagein, AppleEvent *reply, long refIn)
{
#pragma unused (reply, refIn,messagein)
    return(errAEEventNotHandled);
}

pascal OSErr AEPrintHandler(AppleEvent *messagein, AppleEvent *reply, long refIn)
{
#pragma unused (reply,refIn,messagein)
    return(errAEEventNotHandled);
}

pascal OSErr AEQuitHandler(AppleEvent *messagein, AppleEvent *reply, long refIn)
{
#pragma unused (messagein,refIn,reply)
    extern Boolean Quit;
    gQuit = true;
    return(noErr);
}
