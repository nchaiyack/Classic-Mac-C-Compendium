/*	NAME:
		Gamma.c

	WRITTEN BY:
		Matt Slot

	MODIFIED BY:
		Dair Grant
								
	DESCRIPTION:
		Cut down version of Matt Slot's Gamma Utils Library source code. The
		routines to deal with a given graphics device have been removed, and
		the only calls now supported are those that work on all graphics
		devices at once.

	___________________________________________________________________________
*/
// File "gamma.c" - Source for Altering the Gamma Tables of GDevices
//   Last updated 3/13/93, MJS
//
// * ****************************************************************************** *
//	Stripped down version for Shutdown Fade
// * ****************************************************************************** *
//
//
// * ****************************************************************************** *
//
//	This is the Source Code for the Gamma Utils Library file. Use this to build
//		new functionality into the library or make an A4-based library. 
//	See the header file "gamma.h" for much more information. -- MJS
//
// * ****************************************************************************** *

#include <GestaltEqu.h>
#include <Quickdraw.h>
#include <Video.h>
#include <Traps.h>
#include "gamma.h"


globalGammasHdl	gammaTables=0;


// * ****************************************************************************** *
// * ****************************************************************************** *

Boolean IsGammaAvailable() {
	GDHandle theGDevice;

	if (NGetTrapAddress(kGetDeviceListTrapNum, ToolTrap) ==
			NGetTrapAddress(_Unimplemented, ToolTrap)) return(0);
	
	for(theGDevice = GetDeviceList(); theGDevice; theGDevice = GetNextDevice(theGDevice))
		if (TestDeviceAttribute(theGDevice, screenDevice) && 
				TestDeviceAttribute(theGDevice, noDriver)) return(0);

	return(-1);
	}


// * ****************************************************************************** *
// * ****************************************************************************** *

OSErr SetupGammaTools() {
	short errorCold=0;
	globalGammasHdl tempHdl;
	GammaTblPtr	masterGTable;
	GDHandle theGDevice;

	
	for(theGDevice = GetDeviceList(); theGDevice; theGDevice = GetNextDevice(theGDevice)) {
		if (errorCold = GetDevGammaTable(theGDevice, &masterGTable)) return(errorCold);
		
		tempHdl = (globalGammasHdl) NewHandle(sizeof(globalGammas));
		if (tempHdl == 0) return(errorCold = MemError());
		
		(*tempHdl)->size = sizeof(GammaTbl) + masterGTable->gFormulaSize +
				(masterGTable->gChanCnt * masterGTable->gDataCnt * masterGTable->gDataWidth / 8);
		(*tempHdl)->dataOffset = masterGTable->gFormulaSize;
		(*tempHdl)->theGDevice = theGDevice;
		
		(*tempHdl)->saved = (GammaTblHandle) NewHandle((*tempHdl)->size);
		if ((*tempHdl)->saved == 0) return(errorCold = MemError());
		(*tempHdl)->hacked = (GammaTblHandle) NewHandle((*tempHdl)->size);
		if ((*tempHdl)->hacked == 0) return(errorCold = MemError());
	
		BlockMove((Ptr) masterGTable, (Ptr) *(*tempHdl)->saved, (*tempHdl)->size);
		
		(*tempHdl)->next = gammaTables;
		gammaTables = tempHdl;
		}

	return(0);
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

OSErr DoGammaFade(short percent) {
	short errorCold=0;
	register long size, i, theNum;
	globalGammasHdl tempHdl;
	unsigned char *dataPtr;



	for(tempHdl = gammaTables; tempHdl; tempHdl = (*tempHdl)->next) {
	
		BlockMove((Ptr) *(*tempHdl)->saved, (Ptr) *(*tempHdl)->hacked, (*tempHdl)->size);
		dataPtr = (unsigned char *) (*(*tempHdl)->hacked)->gFormulaData + (*tempHdl)->dataOffset;
		size = (*(*tempHdl)->hacked)->gChanCnt * (*(*tempHdl)->hacked)->gDataCnt;
		
		for(i=0; i < size; i++) {
			theNum = dataPtr[i];
			theNum = (theNum * percent) / 100;
			dataPtr[i] = theNum;
			}
		
		if (errorCold = SetDevGammaTable((*tempHdl)->theGDevice, (*tempHdl)->hacked))
			return(errorCold);
		}
		
	return(0);
	}


// * ****************************************************************************** *
// * ****************************************************************************** *

OSErr DisposeGammaTools() {
	globalGammasHdl tempHdl, nextHdl;

	for(tempHdl = gammaTables; tempHdl; tempHdl = nextHdl) {
		nextHdl = (*tempHdl)->next;
		DisposeHandle((Handle) (*tempHdl)->saved);
		DisposeHandle((Handle) (*tempHdl)->hacked);
		DisposeHandle((Handle) tempHdl);
		}
		
	return(0);
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

OSErr GetDevGammaTable(GDHandle theGDevice, GammaTblPtr *theTable) {
	short errorCold=0;
	CntrlParam  *myCPB;
 
	((long *) theTable)[0] = 0;
 
			
	if ((myCPB = (CntrlParam *) NewPtrClear(sizeof(CntrlParam))) == 0) return(MemError());
	myCPB->csCode = cscGetGamma;
	myCPB->ioCRefNum = (*theGDevice)->gdRefNum;
	*(GammaTblPtr **) myCPB->csParam = theTable;
	errorCold = PBStatus((ParmBlkPtr) myCPB, 0);

	DisposePtr((Ptr) myCPB);
	return(errorCold);
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

OSErr SetDevGammaTable(GDHandle theGDevice, GammaTblPtr *theTable) {
	CntrlParam *myCPB;
	short errorCold=0;
	CTabHandle cTab;
	GDHandle saveGDevice;
 


	if ((myCPB = (CntrlParam *) NewPtrClear(sizeof(CntrlParam))) == 0) return(MemError());
	myCPB->csCode = cscSetGamma;
	myCPB->ioCRefNum = (*theGDevice)->gdRefNum;
	**(GammaTblPtr **) myCPB->csParam = *theTable;

	errorCold = PBControl((ParmBlkPtr) myCPB, 0);
 
	if (errorCold == 0) {
		saveGDevice = GetGDevice();
		SetGDevice(theGDevice);
 		cTab = (*(*theGDevice)->gdPMap)->pmTable;
		SetEntries (0, (*cTab)->ctSize, (*cTab)->ctTable);
		SetGDevice(saveGDevice);
		}

	DisposePtr((Ptr) myCPB);
	return (errorCold);
	}
