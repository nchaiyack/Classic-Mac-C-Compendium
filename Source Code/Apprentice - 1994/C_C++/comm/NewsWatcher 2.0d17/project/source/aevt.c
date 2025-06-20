/*----------------------------------------------------------------------------

	aevt.c

	This module handles Apple events.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <stdio.h>
#include <appleevents.h>

#include "glob.h"
#include "aevt.h"
#include "dlgutil.h"
#include "newsrc.h"
#include "util.h"



static FSSpec **gDocList = nil;		/* initial ODOC list */
static short gNumDocs = 0;			/* number of document in initial ODOC list */
static Boolean gStartingUp=true;	/* true during startup */



/*----------------------------------------------------------------------------
	OpenDocList 
	
	Opens a list of documents.
	
	Entry:	docList = handle to array of FSSpec records.
			numDocs = number of FSSpec records in docList.
			
	Exit:	Documents opened.
			docList disposed.
----------------------------------------------------------------------------*/

void OpenDocList (FSSpec **docList, short numDocs)
{
	short i;
	FSSpec theFile;
	
	for (i = 0; i < numDocs; i++) {
		theFile = (*docList)[i];
		OpenFile(&theFile);
	}
	MyDisposHandle((Handle)docList);
}



/*----------------------------------------------------------------------------
	HandleOAPP 
	
	Handles the open application event. Does nothing.
----------------------------------------------------------------------------*/

static pascal OSErr HandleOAPP (AppleEvent *event, AppleEvent *reply, long refcon)
{
	return noErr;
}



/*----------------------------------------------------------------------------
	HandleODOC 
	
	Handles the open document event.
	
	The initial ODOC event during startup, if any, is treated specially.
	For this event, the FSSpec list is parsed and saved in the gDocList
	array, but the documents are not opened. The initialization code in
	init.c takes care of opening these initial documents.
----------------------------------------------------------------------------*/

static pascal OSErr HandleODOC (AppleEvent *event, AppleEvent *reply, long refcon)
{
	OSErr err;
	AEDescList docList;
	long numItems, i;
	AEKeyword keywd;
	DescType returnedType;
	Size actualSize;
	FSSpec theFile;
	
	if (!gStartupOK && !gStartingUp) return noErr;
	err = AEGetParamDesc(event, keyDirectObject, typeAEList, &docList);
	if (err != noErr) goto exit;
	err = AECountItems(&docList, &numItems);
	if (err != noErr) goto exit;
	gDocList = (FSSpec**)MyNewHandle(numItems * sizeof(FSSpec));
	for (i = 1; i <= numItems; i++) {
		err = AEGetNthPtr(&docList, i, typeFSS, &keywd, &returnedType,
			(Ptr)&theFile, sizeof(theFile), &actualSize);
		if (err != noErr) goto exit;
		(*gDocList)[i-1] = theFile;
	}
	err = AEDisposeDesc(&docList);
	if (err != noErr) goto exit;
	gNumDocs = numItems;
	if (gStartingUp) return noErr;
	OpenDocList(gDocList, gNumDocs);
	return noErr;

exit:
	UnexpectedErrorMessage(err);
	return err;
}



/*----------------------------------------------------------------------------
	HandlePDOC 
	
	Handles the print document event. Does nothing.
----------------------------------------------------------------------------*/

static pascal OSErr HandlePDOC (AppleEvent *event, AppleEvent *reply, long refcon)
{
	return noErr;
}



/*----------------------------------------------------------------------------
	HandleQUIT 
	
	Handles the quit application event.
----------------------------------------------------------------------------*/

static pascal OSErr HandleQUIT (AppleEvent *event, AppleEvent *reply, long refcon)
{
	gDone = true;
	return noErr;
}



/*----------------------------------------------------------------------------
	InitializeAppleEvents 
	
	Initializes Apple events. Installs the handlers.
----------------------------------------------------------------------------*/

void InitializeAppleEvents (void)
{
	OSErr err;

	err = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,
		HandleOAPP, 0, false);
	if (err) goto exit;
	err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
		HandleODOC, 0, false);
	if (err) goto exit;
	err = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,
		HandlePDOC, 0, false);
	if (err) goto exit;
	err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
		HandleQUIT, 0, false);
	if (err) goto exit;
	return;
	
exit:
	ErrorMessage("Internal error - could not install Apple event handlers.");
	ExitToShell();
}



/*----------------------------------------------------------------------------
	GetInitialDocList 
	
	Gets the initial document list from the first ODOC or OAPP event.
	
	Exit:	*docList = handle to array of FSSpec records.
			*numDocs = number of FSSpec records in docList.
----------------------------------------------------------------------------*/

void GetInitialDocList (FSSpec ***docList, short *numDocs)
{
	EventRecord ev;
	Boolean gotEvt;

	while (gStartingUp) {
		gotEvt = WaitNextEvent(highLevelEventMask, &ev, GetCaretTime(), nil);
		if (gotEvt && ev.what == kHighLevelEvent) {
			AEProcessAppleEvent(&ev);
			gStartingUp = false;
		}
	}
	*docList = gDocList;
	*numDocs = gNumDocs;
}