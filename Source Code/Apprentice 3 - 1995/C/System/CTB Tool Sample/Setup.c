// ===========================================================================
// "Connection Tool Skeleton in C" for the Communications Toolbox
// 
//	Copyright � 1994 Peter J. Creath
//	All Rights Reserved Worldwide
// ===========================================================================

#include "ConnToolCommon.h"

// ===========================================================================
// Function prototypes
// ===========================================================================
extern pascal long	main(CMSetupPtr pSetup, short msg, Ptr pParam1, short *pItem, PrivatePtr *pPrivate);
extern long	DoPreflight(CMSetupPtr pSetup, PrivatePtr *hPrivate);
extern void	DoSetup(CMSetupPtr pSetup);
extern void	DoItem(CMSetupPtr pSetup, PrivatePtr pPrivate, short *pItem);
extern long	DoFilter(CMSetupPtr pSetup, PrivatePtr pPrivate, EventRecord *pEvent, short *pItem);
extern void	DoCleanup(Ptr *thePtr);


// ===========================================================================
// main()
// 	This function is the entry point for the 'cset' resource.  It passes control to the appropriate
// 	subroutines, depending on the incoming message.  This can probably remain unchanged.
// ===========================================================================

pascal long main(CMSetupPtr pSetup, short msg, Ptr pParam1, short *pItem, PrivatePtr *pPrivate)
{
long	rtnValue;

	switch (msg)
		{
			case cmSpreflightMsg:															// Initialize private data storage
				rtnValue = DoPreflight(pSetup, pPrivate);
				break;
				
			case cmSsetupMsg:																// Set up the configuration dialog
				DoSetup(pSetup);
				break;
				
			case cmSitemMsg:																// Handle any clicked items in the dialog
				DoItem(pSetup, *pPrivate, (short *)pParam1);	
				break;
				
			case cmSfilterMsg:																// Filter events to the dialog
				rtnValue = DoFilter(pSetup, *pPrivate, (EventRecord *)pParam1, pItem);
				break;
				
			case cmScleanupMsg:															// Deallocate private data storage
				DoCleanup((Ptr *)pPrivate);
				break;
				
			default:																					// Say what?
				rtnValue = cmNotSupported;
				break;
		}

return (rtnValue);
}


// ===========================================================================
// DoPreflight()
// 	This function is called in response to a cmSpreflightMsg.  It should allocate and initialize the
// 	PrivateRecord structure (or just use long in place of the pointer, if no more data is required).
// 	It should then return a handle to a dialog item list, which the caller of this tool should dispose of.
// ===========================================================================

long DoPreflight(CMSetupPtr pSetup, PrivatePtr *hPrivate)
{
Handle			hDITL;
short			theID;
short			oldResRef;
long				rtnValue;
PrivatePtr	pPrivate;

	pPrivate = (PrivatePtr)NewPtr(sizeof(PrivateRecord));		// Allocate new private data storage
	pPrivate->foobar = 0;															// and initialize it
	*hPrivate = pPrivate;															// Pass its address back up the chain

	theID = CRMLocalToRealID(classCM, pSetup->procID, 'DITL', localDITLID);
	if (theID == -1)
		{
			rtnValue = FALSE;															// No DITL found
		}
	else
		{
			oldResRef = CurResFile();
			UseResFile(pSetup->procID);											// procID is the tool's resource refnum
			hDITL = Get1Resource('DITL', theID);								// Grab the DITL
			UseResFile(oldResRef);
			
			if (hDITL)
				{
					DetachResource(hDITL);										// Detach it from the resource file
				}
			
			rtnValue = (long)hDITL;													// Return the handle
		}

return (rtnValue);
}


// ===========================================================================
// DoSetup()
// 	This subroutine is called in response to a cmSetupMsg.  It should set up the setup dialog's controls
// 	based on the current data in the ConfigRecord.
// ===========================================================================

void	DoSetup(CMSetupPtr pSetup)
{
short			first;
ConfigPtr		pConfig;
Handle			itemHandle;
short			itemKind;
Rect				itemRect;

	first = pSetup->count - 1;													// count is 1-based
	pConfig = (ConfigPtr)(pSetup->theConfig);							// get the config ptr
	
	GetDItem(pSetup->theDialog, first + myFirstItem, &itemKind, &itemHandle, &itemRect);
	SetCtlValue((ControlHandle)itemHandle, pConfig->foobar);
	GetDItem(pSetup->theDialog, first + mySecondItem, &itemKind, &itemHandle, &itemRect);
	SetCtlValue((ControlHandle)itemHandle, 1-pConfig->foobar);
}


// ===========================================================================
// DoItem()
// 	This subroutine is called in response to a cmSitemMsg.  It should handle any item clicks on the
// 	setup dialog.
// ===========================================================================

void	DoItem(CMSetupPtr pSetup, PrivatePtr pPrivate, short *pItem)
{
short			value;
short			first;
ConfigPtr		pConfig;
Handle			itemHandle;
short			itemKind;
Rect				itemRect;

	first = pSetup->count - 1;													// count is 1-based
	pConfig = (ConfigPtr)(pSetup->theConfig);							// get the config ptr
	
	switch (*pItem - first)
		{
			case myFirstItem:
				GetDItem(pSetup->theDialog, first + myFirstItem, &itemKind, &itemHandle, &itemRect);
				value = 1 - GetCtlValue((ControlHandle)itemHandle);
				pConfig->foobar = value;										// stick value into config record
				SetCtlValue((ControlHandle)itemHandle, value);		// update control
				break;

			case mySecondItem:
				SysBeep(5);
				FlashMenuBar(0);
				break;
		}
}


// ===========================================================================
// DoFilter()
// 	This function is called in response to a cmSfilterMsg.  It acts as the filter routine would in a
// 	modal dialog.  It should return TRUE if the event was handled, or FALSE if not.
// ===========================================================================

long	DoFilter(CMSetupPtr pSetup, PrivatePtr pPrivate, EventRecord *pEvent, short *pItem)
{
long	rtnValue;

	rtnValue = FALSE;
	if (pEvent->what == keyDown)
		{
			SysBeep(5);
			rtnValue = TRUE;
		}
		
return (rtnValue);
}


// ===========================================================================
// DoCleanup()
// 	This subroutine is called in response to a cmScleanupMsg.  It should clean up any private data
// 	storage allocated in DoPreflight().
// ===========================================================================

void	DoCleanup(Ptr *thePtr)
{
	DisposPtr(*thePtr);
	*thePtr = (Ptr)0L;
}
