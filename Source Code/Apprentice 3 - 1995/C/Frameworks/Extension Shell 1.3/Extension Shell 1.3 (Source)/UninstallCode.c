/*	NAME:
		UninstallCode.c

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		Routines for uninstalling the various code resources Extension Shell supports.

	NOTES:
		¥	Not everything that can be installed can be uninstalled.

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
#include "ParamBlock.h"
#include "UninstallCode.h"
#include "ExtensionShell.h"
#include "AddrsTable.h"





//=============================================================================
//		Private function prototypes																	 
//-----------------------------------------------------------------------------
void	UninstallTrapPatch(short i);
void	UninstallGestaltSelector(short i);
void	UninstallShutdownTask(short i);
void	UninstallVBLTask(short i);
void	UninstallLowMemFilter(short i);
void	UninstallCodeBlock(short i);
void	UninstallTimeManagerTask(short i);





//=============================================================================
//		Global variables																 
//-----------------------------------------------------------------------------
extern ESParamBlock		gTheParamBlock;
extern AddressTable		*gTheAddressTable;










//=============================================================================
//		UninstallCode : Uninstall a code resource depending on its type.																 
//-----------------------------------------------------------------------------
//		Note :	Not all types can be removed successfully. Right now, we still
//				don't know of a clean way to remove a Gestalt Selector.
//-----------------------------------------------------------------------------
void UninstallCode(short i)
{




	// Case out on the type of the code, and call the uninstall routine
	switch(gTheParamBlock.theCodeResources[i].codeType) {
		case kTrapPatchType:
		     UninstallTrapPatch(i);
		     break;
		     
		case kGestaltSelectorType:
		     UninstallGestaltSelector(i);
		     break;
		     
		case kShutdownTaskType:
		     UninstallShutdownTask(i);
		     break;
		     
		case kVBLTaskType:
		     UninstallVBLTask(i);
		     break;
		
		case kLowMemFilterType:
		     UninstallLowMemFilter(i);
		     break;
		
		case kCodeBlockType:
		     UninstallCodeBlock(i);
		     break;
		
		case kTimeManagerTaskType:
		     UninstallTimeManagerTask(i);
		     break;
		     
		default:
		     ;
	}
} 










//=============================================================================
//		UninstallTrapPatch : Uninstall a trap patch.														 
//-----------------------------------------------------------------------------
//		Note :	We check the value currently held in the dispatch table. If
//				it's still the same as our address, then it's safe to remove
//				our routine. We do this just by replacing the dispatch table
//				entry with the old value we originally replaced.
//-----------------------------------------------------------------------------
void UninstallTrapPatch(short i)
{	int			trapNum;
	ProcPtr		currentCode;
	ProcPtr		oldCodeAddress;
	TrapType	tType;
	ProcPtr		currentDispatch;
	Handle		theHnd;
	

	
 
 	// Copy the relevent details into local variables for speed
 	trapNum			= gTheParamBlock.theCodeResources[i].theCodeThing.theTrapPatch.trapNum;
 	currentCode		= (ProcPtr) gTheParamBlock.theCodeResources[i].theAddress;
 	oldCodeAddress	= gTheAddressTable->theTable[i];
 	
 	
 	
 	// Get the type of the trap
	tType = GetTrapType(trapNum);
	
	
	
	// Compare the value in the dispatch table with our own code. If they match,
	// then it's safe to remove the patch. Otherwise we're stuck.
	currentDispatch = (ProcPtr) StripAddress((Ptr) NGetTrapAddress(trapNum, tType));
	if (currentDispatch == currentCode)
		{
		// Overwrite the dispatch table entry with the old address.
		NSetTrapAddress((long) StripAddress(oldCodeAddress), trapNum, tType);
   
   
		
		// Recover a handle to our code, and throw it away
		theHnd = RecoverHandle(currentCode);
		HUnlock(theHnd);
		DisposHandle(theHnd);
		}
}










//=============================================================================
//		UninstallGestaltSelector : Uninstall a Gestalt Selector.														 
//-----------------------------------------------------------------------------
//		Note :	I don't know a way of removing a Gestalt Selector at present.
//				We just have to leave them floating around for now. :-(
//-----------------------------------------------------------------------------
void UninstallGestaltSelector(short i)
{
}










//=============================================================================
//		UninstallShutdownTask : Uninstall a Shutdown Task.														 
//-----------------------------------------------------------------------------
void UninstallShutdownTask(short i)
{


	// Remove the shutdown task
	ShutDwnRemove((ShutDwnProcPtr) gTheParamBlock.theCodeResources[i].theAddress);
}










//=============================================================================
//		UninstallVBLTask : Uninstall a VBL task.														 
//-----------------------------------------------------------------------------
void UninstallVBLTask(short i)
{


	// Remove the VBL task
	VRemove((QElemPtr) gTheParamBlock.theCodeResources[i].theAddress);
}










//=============================================================================
//		UninstallLowMemFilter : Remove a low-memory filter.														 
//-----------------------------------------------------------------------------
void UninstallLowMemFilter(short i)
{	Handle		theHnd;
	ProcPtr		theEntryPoint, currentHead, oldHead, ourCode;


	
	
	// Get the address of the filter chain
	theEntryPoint = (ProcPtr) gTheParamBlock.theCodeResources[i].theCodeThing.theLowMemFilter.theEntryPoint;
	
	
	
	// Find out where everything else is
	currentHead	= (ProcPtr) *((ProcPtr *) theEntryPoint);
	oldHead		= gTheAddressTable->theTable[i];
	ourCode		= gTheParamBlock.theCodeResources[i].theAddress;
	
	
	
	// Compare the value at the head of the chain with our own code. If
	// they match, then we can remove our filter. Otherwise we're stuck.
	if (currentHead == ourCode)
		{
		// Overwrite the entry at the head of the chain with the original
		*((ProcPtr *) theEntryPoint) = oldHead;
		
		
		
		// Recover a handle to our code, and throw it away
		theHnd = RecoverHandle(ourCode);
		HUnlock(theHnd);
		DisposHandle(theHnd);
		}
}










//=============================================================================
//		UninstallCodeBlock : Remove a block of code.														 
//-----------------------------------------------------------------------------
void UninstallCodeBlock(short i)
{	ProcPtr		theCode;
	Handle		theHnd;
	

	
 
 	// Get the pointer to the code block
 	theCode = (ProcPtr) gTheParamBlock.theCodeResources[i].theAddress;
 
 	
 	
 	// Recover a handle and dispose of it
 	theHnd = RecoverHandle(theCode);
	HUnlock(theHnd);
	DisposHandle(theHnd);
}










//=============================================================================
//		UninstallTimeManagerTask : Remove a Time Manager task.														 
//-----------------------------------------------------------------------------
void UninstallTimeManagerTask(short i)
{



	// Remove the Time Manager task
	RmvTime((QElemPtr) gTheParamBlock.theCodeResources[i].theAddress);
}
