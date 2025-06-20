/* main.c - main file of RandomDot
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
#include "RandomDotMain.h"
#include "RandomDotRes.h"

#include "RandomDotWin.h"
#include "Error.h"
#include "Help.h"
#include "Menu.h"
#include "Utils.h"
#include "ZoomCode.h"
#include <AppleEvents.h>
#include <Folders.h>
#include <GestaltEqu.h>


#define bDoOpenCommandOnce	(1L << 0)
#define bDoHelpCommandOnce	(1L << 1)
#define bDone				(1L << 2)

/* *** globals
 */
StringPtr	emptyS = "\p";
Integer		appResFile = -1;	/* our own resID */
Integer		prefResFile = -1;	/* preferences file resID */
FSSpec		prefSpec;			/* file spec for preferences file */
SysEnvRec	world;

/* forward declarations.
 */

/* *** local to this file.
 */
static LongInt localEventSelect = 0;


/* FSSpecFunc - call this with an FSSpecPtr and return an OSErr
 */
typedef OSErr (*FSSpecFunc)(FSSpecPtr);

/* BetaExpired - TRUE is later than August 1, 1994
>>>
 */
static Boolean BetaExpired(void){
	return FALSE;
}

/* InitPreferences - Initialize the preferences file and connect to it.
 */
static OSErr InitPreferences(void){
	OSErr	errCode;

	if(noErr == (errCode = FindFolder(kOnSystemDisk, kPreferencesFolderType, kDontCreateFolder, &prefSpec.vRefNum, &prefSpec.parID))){
		GetIndString(prefSpec.name, kMainStrs, kPrefNameS);
		if(-1 == (prefResFile = FSpOpenResFile(&prefSpec, fsRdWrPerm))){
			localEventSelect |= bDoHelpCommandOnce;
			FSpCreateResFile(&prefSpec, kCreator, kPrefType, NIL);
			if(noErr == (errCode = ResError())){
				prefResFile = FSpOpenResFile(&prefSpec, fsRdWrPerm);
			}
		}
	}
	return errCode;
}

/* MissedAEParameters - 
 */
static OSErr MissedAEParameters(AppleEvent *message){
	DescType typeCode;
	Size actualSize;
	OSErr err;

	if(errAEDescNotFound == (err = AEGetAttributePtr(message, keyMissedKeywordAttr, typeWildCard,
			&typeCode, NIL, 0L, &actualSize))){

		return noErr;
	}
	return (noErr == err ? errAEEventNotHandled : err);
}

/* FSOpenWD - convenience function for opening a wRef
 */
static OSErr FSOpenWD(Integer vRef, LongInt dirId, OSType signature, Integer *wRef){
	WDPBRec	io;
	OSErr	val;

	io.ioNamePtr	= NIL;
	io.ioVRefNum	= vRef;
	io.ioWDDirID	= dirId;
	io.ioWDProcID	= signature;
	if(noErr == (val = PBOpenWD(&io, FALSE))){
		*wRef = io.ioVRefNum;
	}
	return val;
}

/* DirIDVRefToWRef - given a dirId, vRef pair, return the working ref
 */
static OSErr DirIDVRefToWRef(Integer vRef, LongInt dirID, Integer *wRefp){
	return FSOpenWD(vRef, dirID, 'ERIK', wRefp);
}


static OSErr OpenPrint(AppleEvent *message, AppleEvent *reply, LongInt refCon, FSSpecFunc f){
	FSSpec		fss;
	AEDescList	docList;
	LongInt		index, itemsInList;
	Size		actualSize;
	AEKeyword	keywd;
	DescType	typeCode;
	OSErr		err;

	if((err = AEGetParamDesc(message, keyDirectObject, typeAEList, &docList)) != noErr ||
		(err = MissedAEParameters(message)) != noErr ||
		(err = AECountItems(&docList, &itemsInList)) != noErr){
		return err;
	}

	for(index = 1; index <= itemsInList; index++){
		if(noErr != (err = AEGetNthPtr(&docList, index, typeFSS, &keywd, &typeCode,
					(Ptr)&fss, sizeof(FSSpec), &actualSize))){
			break;
		}
		if(noErr != (err = (*f)(&fss))){
			break;
		}
	}
	return AEDisposeDesc(&docList);
}

/* DoOpenApp - We get this if we weren't given any documents.
	Nothing to do for now in RandomDot
 */
static pascal OSErr DoOpenApp(AppleEvent *message, AppleEvent *reply, LongInt refCon){
	OSErr err;

	if ((err = MissedAEParameters(message)) != noErr){
		return err;
	}
	return noErr;
}

/* RandomDotOpen1 - wrapper for RandomDotOpen when we don't know the script code.
 */
static OSErr RandomDotOpen1(FSSpecPtr fs){
	return TellError(RandomDotOpen(fs, smSystemScript));
}

/* DoOpenDoc
 */
static pascal OSErr DoOpenDoc(AppleEvent *message, AppleEvent *reply, LongInt refCon){
	return OpenPrint(message, reply, refCon, RandomDotOpen1);
}

/* DoPrintDoc
 */
static pascal OSErr DoPrintDoc(AppleEvent *message, AppleEvent *reply, LongInt refCon){
	return OpenPrint(message, reply, refCon, RandomDotOpen1);
}

/* DoQuitApp - 
 */
static pascal OSErr DoQuitApp(AppleEvent *message, AppleEvent *reply, LongInt refcon){
	OSErr err;

	if ((err = MissedAEParameters(message)) != noErr){
		return err;
	}
	localEventSelect |= bDone;
	return noErr;
}

static void InitAppleEventHandlers(void){
	LongInt	response;

	if(noErr == Gestalt(gestaltAppleEventsAttr, &response) && 
		(response & (1L << gestaltAppleEventsPresent))){

		AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, 
				NewAEEventHandlerProc(DoOpenApp), 0, FALSE);
		AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
				NewAEEventHandlerProc(DoOpenDoc), 0, FALSE);
		AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,
				NewAEEventHandlerProc(DoPrintDoc), 0, FALSE);
		AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
				NewAEEventHandlerProc(DoQuitApp), 0, FALSE);
	}
}

/* InitPopUpMenus - initialize menus used in popup
 */
static void InitPopUpMenus(Integer resId){
	Integer 	n;
	Integer		**h;
	SignedByte	state;

	if(NIL != (h = (Integer	**) Get1Resource('MBAR', resId))){
		LoadResource((Handle) h);
		state = HGetState((Handle) h);
		HNoPurge((Handle) h);
		HLock((Handle) h);
		n = (*h)[ **h ]; 
		while( NIL != GetMenu(++n) ){
			/* empty */
		}
		HSetState((Handle) h, state);
	}
}



/* Init 
 */
static void Init(void){
	InitGraf((Ptr) &qd.thePort);	/* initialize the toolbox */
	InitFonts();
	InitWindows();
	InitMenus();
	FlushEvents(everyEvent,0);
	TEInit();
	InitDialogs(0L);
	InitCursor();
	MaxApplZone();

	if(BetaExpired()){
		Alert(kBetaDie, NIL);
		ExitToShell();
	}
	SetMenuBar(GetNewMBar(kMBAR));
	DrawMenuBar();
	appResFile = CurResFile();
	InitPopUpMenus(kMBAR);
	AddResMenu(GetMHandle(kAppleMenu), 'DRVR');
	SysEnvirons(1, &world);
	if(NOT (world.hasColorQD && world.systemVersion >= 0x700)){
		TellError(eNeed7AndColor);
		ExitToShell();
	}
	if(50000L > FreeMem()){	/* check 50k free */
		TellError(memFullErr);
		ExitToShell();
	}
	InitHelpItem();
	TellError(InitPreferences());
	TellError(InitRandomDot());
	InitAppleEventHandlers();
}

/* GoHighLevelEvent - dispatch to apple event processor
 */
static void GoHighLevelEvent(EventRecord *theEvent){
	AEProcessAppleEvent(theEvent);
}


/* GoMenuBar - 
 */
static void GoMenuBar(EventRecord *event){
	SetCursor(&qd.arrow);
	GoMenu(MenuSelect(event->where));
}

/* GoKey - the keystroke handler
 */
static void GoKey(EventRecord *event){
	WindowPtr	win;

	if(cmdKey & event->modifiers){
		GoMenu(MenuKey((char) event->message));
	}else if(NIL != (win = FrontWindow())){
		SetPort(win);
		RandomDotKey(event);
	}
}

/* GoAway - handle click in goAway box.
 */
static void GoAway(EventRecord *e, WindowPtr win){
	if(TrackGoAway(win, e->where)){
		DoCloseWin(win);
	}
}

/* GoDrag - handle drag event on title bar
 */
static void GoDrag(EventRecord *e, WindowPtr win){
	Rect	limitR;

	limitR = (**LMGetGrayRgn()).rgnBBox;
	DragWindow(win, e->where, &limitR);
}

static void GoGrow(EventRecord *e, WindowPtr win){
	Rect		r;
	LongInt		amount;
	WindowPtr	savePort;

	GetPort(&savePort);
	SetPort(win);
	RandomDotGrowBounds(&r);
	if(0 != (amount = GrowWindow(win, e->where, &r))){
		SizeWindow(win, LoWord(amount), HiWord(amount), FALSE);
		InvalRect(&win->portRect);
		RandomDotGrow();
	}
	SetPort(savePort);
}

static void GoContent(EventRecord *e, WindowPtr win){
	SetPort(win);
	RandomDotClick(e);
}

/* GoZoom - user clicked in zoom area.
 */
static void GoZoom(EventRecord *e, WindowPtr win, Integer sel){
	WindowPtr	savePort;

	GetPort(&savePort);
	SetPort(win);
	if(TrackBox(win, e->where, sel)){
		SetPort(win);
		ZoomTheWindow((WindowPeek) win, sel, RandomDotIdealSize);
		InvalRect(&win->portRect);
		RandomDotGrow();
	}
	SetPort(savePort);
}


/* GoMouseDown - the mouse down tracker
 */
static void GoMouseDown(EventRecord *event, Boolean allOK){
	Integer		sel;
	WindowPtr	win;

	sel = FindWindow(event->where, &win);
	switch(sel){
	case inMenuBar:		if(allOK){ GoMenuBar(event); }			break;
	case inGoAway:		if(allOK){ GoAway(event, win); }		break;
	case inDrag:		GoDrag(event, win);						break;
	case inGrow:		if(allOK){ GoGrow(event, win); }		break;
	case inContent:		if(allOK){ GoContent(event, win); }		break;
	case inZoomIn:
	case inZoomOut:		if(allOK){ GoZoom(event, win, sel); }	break;
	case inSysWindow:	SystemClick(event, win);				break;
	}
}

/* GoIdle - 
 */
static void GoIdle(EventRecord *e){
	WindowPtr	win;

	if(NIL != (win = FrontWindow()) && userKind == ((WindowPeek) win)->windowKind){
		SetPort(win);
		RandomDotIdle(e);
	}
}

/* ModelessFilterProc - wrapper around StdFilterProc to get the window
	correct.
 */
static Boolean ModelessFilterProc(EventRecord *e, DialogPtr *dpp, Integer *itemp){
	DialogPtr	dp;
	static ModalFilterUPP	stdFilerProc = NIL;

	switch(e->what){
	case updateEvt:
	case activateEvt:	dp = (DialogPtr) e->message;	break;
	default:			dp = FrontWindow();	break;
	}
	if(NIL == stdFilerProc){
		GetStdFilterProc(&stdFilerProc);
	}
	if(CallModalFilterProc(stdFilerProc, dp, e, itemp)){
		*dpp = dp;
		return TRUE;
	}
	return FALSE;
}

/* DeviceLoopUpdate - interface between devicelop and our update procedure.
 */
static pascal void DeviceLoopUpdate(Integer depth, Integer flags, GDHandle gdev, LongInt refCon){
	RandomDotUpdate();
}

/* GoUpdate - stack the port, dispatch the update
 */
static void GoUpdate(EventRecord *e){
	WindowPtr	savePort;
	static DeviceLoopDrawingUPP deviceLoopUpdate = NIL;

	if(NIL == deviceLoopUpdate){
		deviceLoopUpdate = NewDeviceLoopDrawingProc(DeviceLoopUpdate);
	}
	GetPort(&savePort);
	SetPort((WindowPtr) e->message);
	BeginUpdate((WindowPtr) e->message);
	DeviceLoop(qd.thePort->visRgn, deviceLoopUpdate, 0, 0);
	EndUpdate((WindowPtr) e->message);
	if(NIL != savePort){
		SetPort(savePort);
	}
}

/* GoActivateEvent - activate sets the port. deactivate stacks theport.
 */
static void GoActivateEvent(EventRecord *e){
	WindowPtr	savePort;

	if(userKind == ((WindowPeek) e->message)->windowKind){
		GetPort(&savePort);
		SetPort((WindowPtr) e->message);
		if(activeFlag & e->modifiers){
			RandomDotActivate();
		}else{
			RandomDotDeactivate();
			if(NIL != savePort){
				SetPort(savePort);
			}
		}
	}
}

/* GoOSEvent - 
 */
static void GoOSEvent(EventRecord *e){
	WindowPtr	win;

	switch((e->message >> 12) & 0xFF){
	case suspendResumeMessage:
		if(resumeFlag & e->modifiers){
			if(NIL != (win = FrontWindow()) && userKind == ((WindowPeek) win)->windowKind){
				SetPort(win);
				RandomDotActivate();
			}
		}else{
			if(NIL != (win = FrontWindow()) && userKind == ((WindowPeek) win)->windowKind){
				SetPort(win);
				RandomDotDeactivate();
			}
		}
		break;
	}
}



/* GoEvent - main event dispatcher.
 */
static void GoEvent(EventRecord *e){
	DialogPtr	dp;
	Integer		item;

	if(IsDialogEvent(e) && 
		(ModelessFilterProc(e, &dp, &item) || 
		DialogSelect(e, &dp, &item)) &&
		NIL != dp){
#if 0
			if(gRandomDotConfig == dp){
				DoRandomDotConfigItem(item);
			}
#endif
			return;
	}
	switch(e->what){
	case nullEvent:			GoIdle(e);			break;
	case mouseDown:			GoMouseDown(e, TRUE);	break;
	case autoKey:			
	case keyDown:			GoKey(e);			break;
	case updateEvt:			GoUpdate(e);		break;
	case activateEvt:		GoActivateEvent(e);	break;
	case osEvt:				GoOSEvent(e);		break;
	case kHighLevelEvent:	GoHighLevelEvent(e);	break;
	default:				break;
	}
}

/* DialogOnTopGoEvent - call this when we can't handle keydowns or update events.
	and the only legal mouse events are window drags.
 */
void DialogOnTopGoEvent(EventRecord *e){
	switch(e->what){
	case nullEvent:			GoIdle(e);			break;
	case mouseDown:			GoMouseDown(e, FALSE);		break;
	case activateEvt:		GoActivateEvent(e);	break;
	case osEvt:				GoOSEvent(e);		break;
	case kHighLevelEvent:	GoHighLevelEvent(e);	break;
	default:				break;
	}
}


/* HandleEvents - 
 */
void HandleEvents(void){
	EventRecord e;

	if(0 != localEventSelect){
		if(localEventSelect & bDone){
			DoQuit();
		}
		if(localEventSelect & bDoOpenCommandOnce){
			localEventSelect &= ~bDoOpenCommandOnce;
			DoOpen();
		}
		if(localEventSelect & bDoHelpCommandOnce){
			localEventSelect &= ~bDoHelpCommandOnce;
			DoHelp();
		}
	}
	WaitNextEvent(everyEvent, &e, NIL, 0);
	GoEvent(&e);	/* allow null event processing */
}


/* main
 */
main(){
	Init();
	for(;;){
		HandleEvents();
	}
	return 0;
}
