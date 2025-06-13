// File "aevents.c" -

#include "aevents.h"
#include "main.h"
#include "sample win.h"

// ***********************************************************************************
// Global Declarations 

extern GlobalsRec glob;

// ***********************************************************************************
// ***********************************************************************************

pascal short AEHandlerOAPP(AppleEvent *event, AppleEvent *reply, long refCon) {

	return(0);
	}
	
// ***********************************************************************************
// ***********************************************************************************

pascal short AEHandlerODOC(AppleEvent *event, AppleEvent *reply, long refCon) {
	
	return(0);
	}
	
// ***********************************************************************************
// ***********************************************************************************

pascal short AEHandlerPDOC(AppleEvent *event, AppleEvent *reply, long refCon) {

	return(0);	
	}
	
// ***********************************************************************************
// ***********************************************************************************

pascal short AEHandlerQUIT(AppleEvent *event, AppleEvent *reply, long refCon) {

	glob.quitting = TRUE;
	return(0);
	}
	
// ***********************************************************************************
// ***********************************************************************************

void InitHLEvents() {
	short i, err=0;
	
	err=AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, (AEEventHandlerUPP)AEHandlerOAPP, 0, 0);
	err=AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, (AEEventHandlerUPP)AEHandlerODOC, 0, 0);
	err=AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, (AEEventHandlerUPP)AEHandlerPDOC, 0, 0);
	err=AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, (AEEventHandlerUPP)AEHandlerQUIT, 0, 0);
	}


