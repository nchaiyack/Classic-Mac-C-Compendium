/*	NAME:
		InstallCode.c

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		Routines for installing the various code resources Extension Shell supports.

	___________________________________________________________________________

	VERSION HISTORY:
		(Jan 1994, dg)
			¥	Added low-memory filter support.

		(Jan 1994, dg)
			¥	First publicly distributed version.

		(Mar 1994, dg)
			¥	Added support for code blocks.
			
			
	___________________________________________________________________________
*/
//=============================================================================
//		Include files																	 
//-----------------------------------------------------------------------------
#include <GestaltEqu.h>
#include <Shutdown.h>
#include "ParamBlock.h"
#include "InstallCode.h"
#include "ExtensionShell.h"
#include "AddrsTable.h"





//=============================================================================
//		Private function prototypes																	 
//-----------------------------------------------------------------------------
OSErr	InstallTrapPatch(short i, Ptr theCode);
OSErr	InstallGestaltSelector(short i, Ptr theCode);
OSErr	InstallShutdownTask(short i, Ptr theCode);
OSErr	InstallVBLTask(short i, Ptr theCode);
OSErr	InstallLowMemFilter(short i, Ptr theCode);
OSErr	InstallCodeBlock(short i, Ptr theCode);
OSErr	InstallTimeManagerTask(short i, Ptr theCode);





//=============================================================================
//		Global variables																 
//-----------------------------------------------------------------------------
extern ESParamBlock		gTheParamBlock;
extern AddressTable		*gTheAddressTable;










//=============================================================================
//		InstallCode : Install a code resource depending on its type.																 
//-----------------------------------------------------------------------------
//		Note :	We get the index of something in gTheParamBlock.TheCodeResources
//				and case out on its type to do the installation. Both ourselves
//				and the actual install routines are only expected to return any
//				error code that arises. If an error does occur, our caller
//				looks after filling in the other error details in gTheParamBlock.
//-----------------------------------------------------------------------------
OSErr InstallCode(short i)
{	Handle		theHnd;
	OSErr		theErr;
	Ptr			theCode;
	int			theType;
	
	


	// Get the code resource in question, and abort if it wasn't there.
	theHnd = GetResource(gTheParamBlock.theCodeResources[i].resType,
	                     gTheParamBlock.theCodeResources[i].resID);
	if (theHnd == nil)
		return(resNotFound);


	
	// Unlock, move high, and lock the resource. Then detach it so
	// it stays around after Extension Shell is gone.
	HUnlock(theHnd);
	HLockHi(theHnd);
	DetachResource(theHnd);



	// Get a clean address from the handle
	theCode = (Ptr) StripAddress(*theHnd);



	// Now case out on the type of the code, and call the install routine
	switch(gTheParamBlock.theCodeResources[i].codeType) {
		case kTrapPatchType:
		     theErr = InstallTrapPatch(i, theCode);
		     break;
		     
		case kGestaltSelectorType:
		     theErr = InstallGestaltSelector(i, theCode);
		     break;
		     
		case kShutdownTaskType:
		     theErr = InstallShutdownTask(i, theCode);
		     break;
		     
		case kVBLTaskType:
		     theErr = InstallVBLTask(i, theCode);
		     break;
		
		case kLowMemFilterType:
		     theErr = InstallLowMemFilter(i, theCode);
		     break;
		
		case kCodeBlockType:
		     theErr = InstallCodeBlock(i, theCode);
		     break;
		
		case kTimeManagerTaskType:
		     theErr = InstallTimeManagerTask(i, theCode);
		     break;
		
		default:
		     theErr = noErr;
	}

	
	
	return(theErr);
} 










//=============================================================================
//		InstallAddressTable : Install an address table for the INIT.
//-----------------------------------------------------------------------------
//		Note :	We install the Gestalt selector, get a hold of the address
//				table and leave the address of the table in the global
//				gTheAddressTable.
//-----------------------------------------------------------------------------
void InstallAddressTable(void)
{	Handle			theHnd;
	OSErr			theErr;
	



	// Load the AddressTable Gestalt selector, and install it with NewGestalt.
	// If we can't get it, we die.
	theHnd = GetResource(kAddrsTableCodeType, kAddrsTableCodeID);
	if (theHnd == nil)
		DebugStr("\pExtension Shell - couldn't find AddressTable CODE resource");
	
		
	
	// We have it, so prepare the code by detaching it and locking it high
	HUnlock(theHnd);
	HLockHi(theHnd);
	DetachResource(theHnd);
	
	
	
	// Install it as a Gestalt selector
	theErr = NewGestalt(gTheParamBlock.addressTableSelector, (ProcPtr) *theHnd);
	if (theErr != noErr)
		DebugStr("\pExtension Shell - couldn't install AddressTable");
		


	// Call the new routine to get the address of the address table
	theErr = Gestalt(gTheParamBlock.addressTableSelector, &gTheAddressTable);
	if (theErr != noErr)
		DebugStr("\pExtension Shell - couldn't call AddressTable");
}










//=============================================================================
//		InstallTrapPatch : Install a trap patch.														 
//-----------------------------------------------------------------------------
//		Note :	Given a trap number and a pointer to the new routine, we save
//				the original entry in the dispatch table and insert ours.
//
//				We assume that the trap *is* implemented.
//-----------------------------------------------------------------------------
OSErr InstallTrapPatch(short i, Ptr theCode)
{	OSErr		theErr;
	int			trapNum;
	TrapType	tType;
	ProcPtr		oldCodeAddress;
	
	
	
	
 	// Copy the relevent details into local variables for speed
 	trapNum			= gTheParamBlock.theCodeResources[i].theCodeThing.theTrapPatch.trapNum;

 
 
 	// Get the type of the trap
	tType = GetTrapType(trapNum);
	
	
	
	// Replace the old value in the dispatch table with the new value.
	oldCodeAddress = (ProcPtr) StripAddress((Ptr) NGetTrapAddress(trapNum, tType));
	NSetTrapAddress((long) theCode, trapNum, tType);
	theErr = noErr;
	
	
	
	// If there wasn't a problem, fill in the entry in the address table
	// and save the address of the new code.
	if (theErr == noErr)
		{
		gTheAddressTable->theTable[i]					= oldCodeAddress;
		gTheParamBlock.theCodeResources[i].theAddress	= theCode;
		}
	
	
	
	// If there was a problem, return it
	return(theErr);	
}










//=============================================================================
//		InstallGestaltSelector : Install a Gestalt Selector.														 
//-----------------------------------------------------------------------------
OSErr InstallGestaltSelector(short i, Ptr theCode)
{	OSErr				theErr;
	AGestaltSelector	*theGestaltInfo;




	// Get a pointer to the information to save on dereferences
	theGestaltInfo = &gTheParamBlock.theCodeResources[i].theCodeThing.theGestaltSelector;
	
	
	
	// Try and install the routine
	theErr = NewGestalt(theGestaltInfo->theSelector, (ProcPtr) theCode);
	
	
	
	// If the selector exists, replace it if we're allowed to, and save
	// the address of the old routine in the address table
	if (theErr == gestaltDupSelectorErr && theGestaltInfo->overwriteExistingSelector)
		theErr = ReplaceGestalt(theGestaltInfo->theSelector,
								theCode,
								&gTheAddressTable->theTable[i]);
	
	
	
	// If there wasn't a problem, save the address of the new code. The address
	// table is only used if we need to get access to the old Gestalt routine.
	if (theErr == noErr)
		gTheParamBlock.theCodeResources[i].theAddress = theCode;
		
		
	
	// If there was a problem, return it
	return(theErr);
}










//=============================================================================
//		InstallShutdownTask : Install a Shutdown Task.														 
//-----------------------------------------------------------------------------
OSErr InstallShutdownTask(short i, Ptr theCode)
{	OSErr	theErr;




	// Install the task with its flags. We assume no errors can occur.
	ShutDwnInstall((ShutDwnProcPtr) theCode,
				   gTheParamBlock.theCodeResources[i].theCodeThing.theShutdownTask.theFlags);
	theErr = noErr;	
	
	
	
	// Save the address in gTheParamBlock. We don't use the address table at all.
	if (theErr == noErr)
		gTheParamBlock.theCodeResources[i].theAddress = theCode;



	// If there was a problem, return it
	return(theErr);
}










//=============================================================================
//		InstallVBLTask : Install a VBL task.														 
//-----------------------------------------------------------------------------
OSErr InstallVBLTask(short i, Ptr theCode)
{	VBLTask 		*theVblPtr;
	OSErr			theErr;
	
	
	
	
	// Allocate a little block of memory in the System Heap for the task information
	theVblPtr = (VBLTask*) NewPtrSys(sizeof(VBLTask));
	
	
	
	// Fill in the fields, and install it as a VBL task
	theVblPtr->qType    = vType;
	theVblPtr->vblAddr  = (VBLProcPtr) theCode;
	theVblPtr->vblCount = gTheParamBlock.theCodeResources[i].theCodeThing.theVBLTask.vblCount;
	theVblPtr->vblPhase = gTheParamBlock.theCodeResources[i].theCodeThing.theVBLTask.vblPhase;
	theErr = VInstall(theVblPtr);
	
	
	
	// Save the address of the VBLTask structure in the address table and the param block
	if (theErr == noErr)
		{
		gTheAddressTable->theTable[i]					= (ProcPtr) theVblPtr;
		gTheParamBlock.theCodeResources[i].theAddress	= (Ptr) theVblPtr;
		}
	
	
	// If there was a problem, return it
	return(theErr);
}










//=============================================================================
//		InstallLowMemFilter : Install a low-memory filter.														 
//-----------------------------------------------------------------------------
OSErr InstallLowMemFilter(short i, Ptr theCode)
{	ProcPtr		theEntryPoint;
	


	
	// Get the address of the filter chain.
	theEntryPoint = (ProcPtr) gTheParamBlock.theCodeResources[i].theCodeThing.theLowMemFilter.theEntryPoint;
	
	
	
	// Save the address currently at the front of the chain in the address table
	// and save ourselves in the param block.
	gTheAddressTable->theTable[i]					= (ProcPtr) *((void **) theEntryPoint);
	gTheParamBlock.theCodeResources[i].theAddress	= theCode;
	
	
	
	// Insert ourselves into the front of the chain
	*((long *) theEntryPoint) = (long) theCode;



	// If there was a problem, return it
	return(noErr);
}










//=============================================================================
//		InstallCodeBlock : Install a block of code.														 
//-----------------------------------------------------------------------------
OSErr InstallCodeBlock(short i, Ptr theCode)
{	ProcPtr		theEntryPoint;
	
	

	
	// Set the reserved field to some value
	gTheParamBlock.theCodeResources[i].theCodeThing.theCodeBlock.reserved = 0x0000;
	
	
	
	// Save the address of the code block in the address table and the param block
	gTheAddressTable->theTable[i]					= (ProcPtr) theCode;
	gTheParamBlock.theCodeResources[i].theAddress	= theCode;



	// If there was a problem, return it
	return(noErr);
}










//=============================================================================
//		InstallTimeManagerTask : Install a Time Manager task.														 
//-----------------------------------------------------------------------------
OSErr InstallTimeManagerTask(short i, Ptr theCode)
{	TMTask	 		*theTaskPtr;




	// Allocate a little block of memory in the System Heap for the task information
	theTaskPtr = (TMTask*) NewPtrSys(sizeof(TMTask));
	
	
	
	// Fill in the fields, and install it as an Time Manager task. We don't install
	// it as with InsXTime because we probably won't need that accurate timing.
	theTaskPtr->tmAddr	= (ProcPtr) theCode;
	theTaskPtr->tmCount	= 0;
	InsTime(theTaskPtr);
	PrimeTime(theTaskPtr,
				gTheParamBlock.theCodeResources[i].theCodeThing.theTimeManagerTask.theDelay);
	
	
	
	// Save the address of the TMTask structure in the address table and the param block
	gTheAddressTable->theTable[i]					= (ProcPtr) theTaskPtr;
	gTheParamBlock.theCodeResources[i].theAddress	= (Ptr) theTaskPtr;
	
	
	
	// Return no errors
	return(noErr);
}
