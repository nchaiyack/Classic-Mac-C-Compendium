// File "notify.c" - 

#include "main.h"
#include "notify.h"

// * ****************************************************************************** *
// Global Declarations

extern GlobalsRec glob;
Boolean gDoneFlag=FALSE;

// * ****************************************************************************** *
// * ****************************************************************************** *

void UserNotify(short notifyList, short notifyIndex, Boolean exit) {
	long saveA5;
	Ptr buffer;
	NMRecPtr notify=0;
	StringPtr nmStr=0;
	EventRecord tempEvent;
	
	// Allocate one buffer for all the notification info
	buffer = NewPtrSysClear(sizeof(NMRec) + sizeof(Str255));
	if (! buffer) goto HANDLE_ERROR;
	notify = (NMRecPtr) buffer;
	nmStr = (StringPtr) buffer + sizeof(NMRec);
	
	SetA5(saveA5 = SetA5(0));
	GetIndString(nmStr, notifyList, notifyIndex);
	if (! nmStr[0]) goto HANDLE_ERROR;
	
	notify->qLink = 0;
	notify->qType = nmType;
	notify->nmMark = 0;
	notify->nmIcon = 0;
	notify->nmSound = (Handle) -1;
	notify->nmStr = nmStr;
	notify->nmResp = CompleteNotify; 
	notify->nmRefCon = saveA5;
	NMInstall(notify);
	
	glob.modalFloats = TRUE; 	// Block the floaters from displaying
	gDoneFlag = FALSE;			// Prime the cleanup semaphore 
	
	if (! exit) return;
	
	// We need to surrender time to let the notification complete
	//   before it is safe to release the notification and quit.
	while(! gDoneFlag) WaitNextEvent(everyEvent, &tempEvent, 60, 0);
	ExitToShell();	// Note: patched or not, E2S() should succeed


HANDLE_ERROR:
	if (notify) DisposePtr((Ptr) notify);

	SysBeep(7);
	if (exit) ExitToShell();
	  else return;
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

pascal void CompleteNotify(NMRecPtr notify) {
	long saveA5 = SetA5(notify->nmRefCon);
	
	glob.modalFloats = FALSE;	// Now we can show the floaters!
	gDoneFlag = TRUE;			// And, if necessary, reset the done flag
	
	NMRemove(notify);
	DisposePtr((Ptr) notify); 
	
	SetA5(saveA5);
	}
		
