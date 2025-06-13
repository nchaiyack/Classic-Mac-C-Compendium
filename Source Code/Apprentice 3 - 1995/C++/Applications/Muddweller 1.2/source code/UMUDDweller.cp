/* UMUDDweller - Implementation of TMUDDwellerApp                             */

#include "UMUDDweller.h"


		// ¥ Toolbox
#ifndef __ERRORS__
#include "Errors.h"
#endif

#ifndef __TOOLUTILS__
#include "ToolUtils.h"
#endif


		// ¥ MacApp
#ifndef __UFailure__
#include "UFailure.h"
#endif


		// ¥ Implementation use
#ifndef __GLOBALS__
#include "Globals.h"
#endif

#ifndef __MUDDOC__
#include "MUDDoc.h"
#endif

#ifndef __NOTRACE__
#include "NoTrace.h"
#endif

//------------------------------------------------------------------------------

const int kBusyTime = 10;
const short kBusyCurID = 1000;
const short kBusyCurMax = 7;

//------------------------------------------------------------------------------

#pragma segment MAApplicationRes

pascal void TMUDDwellerApp::BuildReserves (TDocument *preserve)
{
	while (gMemIsLow) {
		if (gMemReserve && !gMemReserve2) {
			gMemReserve2 = gMemReserve;
			gMemReserve = NULL;
		}
		if (!gMemReserve2) gMemReserve2 = NewPermHandle (kRsrvSize);
		if (gMemReserve2 && !gMemReserve)
			gMemReserve = NewPermHandle (kRsrvSize);
		if (IsHandlePurged (pMemReserve)) BuildAllReserves ();
		gMemIsLow = (!gMemReserve || !gMemReserve2);
		if (!DropSome (preserve)) break;
	}
}

//------------------------------------------------------------------------------

#pragma segment MATerminate

static Boolean canQuit;

pascal void QuitDoc (struct TDocument *aDocument, void * )
{
	TLogoutMacro *logout;
	TMacro *mac;
	
	if (((TMUDDoc *) aDocument)->fClosing) {
		mac = ((TMUDDoc *) aDocument)->fRunList;
		while (mac->fNext) mac = mac->fNext;
		((TLogoutMacro *) mac)->fDoneCmd = cQuit;
		canQuit = FALSE;
	} else if (((TMUDDoc *) aDocument)->fConnected) {
		logout = new TLogoutMacro;
		FailNIL (logout);
		logout->IMacro (aDocument, ((TMUDDoc *) aDocument)->fLogout);
		logout->fDoneCmd = cQuit;
		canQuit = FALSE;
	}
}

pascal void TMUDDwellerApp::Close (void)
{
	canQuit = TRUE;
	ForAllDocumentsDo (QuitDoc, kDummyLink);
	if (canQuit)
		inherited::Close ();
	else
		gAppDone = FALSE;
}

//------------------------------------------------------------------------------

#pragma segment MAApplicationRes

static Boolean gCMEvtHandled, gSwitchIn;
static EventInfo *gCMEvtInfo;
static ConnHandle gCMEvtConn;

pascal void CallCMEvent (struct TDocument *aDocument, void * )
{
	if (((TMUDDoc *) aDocument)->DoCMEvent (gCMEvtConn, gCMEvtInfo))
		gCMEvtHandled = TRUE;
}

pascal void CallCMResume (struct TDocument *aDocument, void * )
{
	((TMUDDoc *) aDocument)->DoCMResume (gSwitchIn);
}

pascal void TMUDDwellerApp::DispatchEvent (EventInfo *theEventInfo,
		struct TCommand **commandToPerform)
{
	WindowPtr theWindow;
	
	theWindow = NULL;
	switch (theEventInfo->thePEvent->what) {
	case keyDown:
	case autoKey:
		theWindow = FrontWindow ();
		break;
	case mouseDown:
		FindWindow (theEventInfo->thePEvent->where, &theWindow);
		break;
	case updateEvt:
	case activateEvt:
		theWindow = (WindowPtr) theEventInfo->thePEvent->message;
		break;
	}
	gCMEvtHandled = FALSE;
	if (theWindow != NULL) {
		gCMEvtInfo = theEventInfo;
		gCMEvtConn = (ConnHandle) GetWRefCon (theWindow);
		ForAllDocumentsDo (CallCMEvent, kDummyLink);
	}
	if (gCMEvtHandled)
		*commandToPerform = NULL;
	else {
		inherited::DispatchEvent (theEventInfo, commandToPerform);
		if (theEventInfo->thePEvent->what == app4Evt) {
			if (((theEventInfo->thePEvent->message >> 24) & 0xFF) ==
					suspendResumeMessage) {
				gSwitchIn = (Boolean) (theEventInfo->thePEvent->message & 1);
				ForAllDocumentsDo (CallCMResume, kDummyLink);
			}
		}
	}
}

//------------------------------------------------------------------------------

#pragma segment MAApplicationRes

pascal TCommand *TMUDDwellerApp::DoCommandKey (short ch, EventInfo *info)
{
	if (ch == '.') {
		gStop = TRUE;
		return NULL;
	} else
		return inherited::DoCommandKey (ch, info);
}

//------------------------------------------------------------------------------

#pragma segment AOpen

pascal TDocument *TMUDDwellerApp::DoMakeDocument (CmdNumber )
{
	TMUDDoc *aMUDDoc;
	Handle canPurge;

	if (TotalTempSize (FALSE, &canPurge) + 8 + GetHandleSize (pCodeReserve) >
			pSzCodeReserve)
		SetReserveSize (pSzCodeReserve, pSzMemReserve); /* forces rebuild */
	BuildReserves (NULL);
	if (gMemIsLow) Failure (memFullErr, 0);
	aMUDDoc = new TMUDDoc;
	FailNIL (aMUDDoc);
	aMUDDoc->IMUDDoc ();
	BuildReserves (NULL);
	if (gMemIsLow) Failure (memFullErr, 0);
	return aMUDDoc;
}

//------------------------------------------------------------------------------

#pragma segment MAApplicationRes

pascal void TMUDDwellerApp::DoSetupMenus (void)
{
	if (gDisableMenus) {
		TEvtHandler::DoSetupMenus ();
		Enable (cAboutApp, TRUE);
		Enable (cShowClipboard, TRUE);
		SetMenuState (cShowClipboard, kIDBuzzString, bzShowClip, bzHideClip,
			gClipWindow == GetActiveWindow ());
		Enable (cClose, FALSE);
	} else
		inherited::DoSetupMenus ();
}

//------------------------------------------------------------------------------

#pragma segment MAApplicationRes

pascal Boolean TMUDDwellerApp::DropSome (TDocument *preserve)
{
	long count;
	Boolean dropped;
	TMUDDoc *aMUDDoc;
	TLogWindow *aLogWindow;
	TLogView *aLogView;
	TBigText *aBigText;
	
	dropped = FALSE;
	count = GetHandleSize ((Handle) gMyDocList) / sizeof (Handle);
	while (count-- > 0) {
		aMUDDoc = (TMUDDoc *) ((**gMyDocList) [count]);
		if (aMUDDoc != preserve) {
			aLogWindow = aMUDDoc->fLogWindow;
			if (!aLogWindow) break;
			aLogView = aLogWindow->fLogView;
			if (!aLogView) break;
			aBigText = aLogView->fBT;
			if (!aBigText) break;
			dropped |= aBigText->DropSome ();
		}
	}
	return dropped;
}

//------------------------------------------------------------------------------

#pragma segment MAApplicationRes

static Boolean gAlertInhibit = FALSE;

pascal void TMUDDwellerApp::Idle (IdlePhase phase)
{
	if (IsHandlePurged (pMemReserve)) BuildAllReserves ();
	if (gMemReserve && !gMemReserve2) {
		gMemReserve2 = gMemReserve;
		gMemReserve = NULL;
	}
	if (IsHandlePurged (pMemReserve) || !gMemReserve2 ||
			(!gMemReserve && (phase == idleBegin))) {
		if (IsHandlePurged (pMemReserve) || !gMemReserve2) {
			DropSome (NULL);
			gMemReserve2 = NewPermHandle (kRsrvSize);
			BuildAllReserves (); /* might dellocate gMemReserve2 */
		}
		if (!gMemReserve && gMemReserve2)
			gMemReserve = NewPermHandle (kRsrvSize);
		if (gMemReserve && gMemReserve2) gMemIsLow = FALSE;
	}
	inherited::Idle (phase);
	if (gPurgedHist && !gAlertInhibit) {
		gAlertInhibit = TRUE;
		StdAlert (phPurgedID);
		gAlertInhibit = FALSE;
		gPurgedHist = FALSE;
	}
	if ((gEventLevel <= 1) && (phase == idleContinue) && gDocList->IsEmpty ())
		gStop = FALSE;
}

//------------------------------------------------------------------------------

#pragma segment AInit

pascal void TMUDDwellerApp::IMUDDwellerApp (void)
{
	Str255 aName;
	short aRef;
	Handle aHandle;
	FCBPBRec pb;
	
	gDefVRefNum = 0;
	gDefDirID = 2;
	gMacAppAlertFilter = (Ptr) (long) MacAppAlertFilter;
	IApplication (kFileType);
	GetAppParms (aName, &aRef, &aHandle);
	pb.ioCompletion = NULL;
	pb.ioNamePtr = aName;
	pb.ioVRefNum = 0;
	pb.ioRefNum = aRef;
	pb.ioFCBIndx = 0;
	FailOSErr (PBGetFCBInfo (&pb, FALSE));
	gDefVRefNum = pb.ioFCBVRefNum;
	gDefDirID = pb.ioFCBParID;
	gDisableMenus = FALSE;
	gStop = FALSE;
}

//------------------------------------------------------------------------------

#pragma segment MASelCommand

static Boolean gCMMenuHandled;
static short gCMMenu, gCMItem;

pascal void CallCMMenu (struct TDocument *aDocument, void * )
{
	if (((TMUDDoc *) aDocument)->DoCMMenu (gCMMenu, gCMItem))
		gCMMenuHandled = TRUE;
}

pascal struct TCommand *TMUDDwellerApp::MenuEvent (long menuItem)
{
	gCMMenu = (short) ((menuItem >> 16) & 0xFFFF);
	gCMItem = (short) (menuItem & 0xFFFF);
	gCMMenuHandled = FALSE;
	ForAllDocumentsDo (CallCMMenu, kDummyLink);
	if (gCMMenuHandled)
		return NULL;
	else
		return inherited::MenuEvent (menuItem);
}

//------------------------------------------------------------------------------

#pragma segment MAOpen

pascal void TMUDDwellerApp::SFGetParms (CmdNumber itsCmdNumber, short *dlgID,
		Point *where, Ptr *fileFilter, Ptr *dlgHook, Ptr *filterProc,
		TypeListHandle typeList)
{
	inherited::SFGetParms (itsCmdNumber, dlgID, where, fileFilter,dlgHook,
		filterProc, typeList);
	if (itsCmdNumber == cSendFile || itsCmdNumber == cUpload)
		**typeList[0] = 'TEXT';
	else if (itsCmdNumber == cPrefs)
		**typeList[0] = 'APPL';
}

//------------------------------------------------------------------------------

#pragma segment MAApplicationRes

static TWindow *actWindow;
static TMUDDoc *actDoc;

pascal void FindFront (struct TDocument *aDocument, void * )
{
	if (((TMUDDoc *) aDocument)->fLogWindow == actWindow)
		actDoc = (TMUDDoc *) aDocument;
}

pascal Boolean TMUDDwellerApp::TrackCursor (void)
{
	long ticks;
	Boolean result;
	
	if (gInBackground) return FALSE;
	actWindow = WMgrToWindow (FrontWindow ());
	actDoc = NULL;
	ForAllDocumentsDo (FindFront, kDummyLink);
	if (actDoc && actDoc->fRunning) {
		gAlwaysTrackCursor = TRUE;
		ticks = TickCount ();
		if ((gLastBusyTick == 0) || (ticks - gLastBusyTick > kBusyTime)) {
			gLastBusyTick = ticks;
			if (qNeedsColorQD || gConfiguration.hasColorQD)
				RectRgn (gTempRgn, &(**GetMainDevice ()).gdRect);
			else
				RectRgn (gTempRgn, &qd.screenBits.bounds);
			UnionRgn (GetGrayRgn (), gTempRgn, gCursorRgn);
			gBusyState = (gBusyState >= kBusyCurMax) ? 0 : gBusyState + 1;
			SetCursor (*GetCursor (kBusyCurID + gBusyState));
			ShowCursor ();
			return TRUE;
		} else
			return FALSE;
	} else {
		result = inherited::TrackCursor ();
		gAlwaysTrackCursor = FALSE;
		gLastBusyTick = 0;
		return result;
	}
}

//------------------------------------------------------------------------------
