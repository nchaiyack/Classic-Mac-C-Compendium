/* driver.c  - version 2.2 - October 21, 1993 */

/*
 *	The following code is to install and remove RAM drivers in the system
 *	heap. Written by Pete Resnick with the help of J. Geagan, Joe Holt,
 *	Tom Johnson, Michael A. Libes, Charles Martin, John Norstad, Phil
 *	Shapiro, Eric Braun, David Brown and Matthias Urlichs. Feel free to
 *	use this in your code, though I do ask that you give credit. Please
 *	report any bugs to Pete Resnick - resnick@cogsci.uiuc.edu. Please read
 *	the README file and check defines in drvrincludes.h before you use
 *	this code!!
 *
 *	Change Log
 *	----------
 *	Date:		Change:												Who:
 *	-----		-------												----
 *	6/2/92		Changed ThinkCleanup so that it compiles and works	pr
 *	6/22/92		Corrected declaration of DisableInterrupts			eb
 *	7/1/92		Corrected declaration of DrvrInstall and DrvrRemove	eb/pr
 *	10/15/92	Changed Get1SysRsrc to Get1SysXRsrc					pr
 *	10/18/92	Got rid of thinkReOpen; just return 1 from close	pr
 *				Fixed up PtrInZone to make it a little quicker		pr
 *	11/6/92		Got rid of auto initialize for newCode and oldCode	pr
 *				Changed PBxxx calls to PBxxxSync					pr
 *	11/8/92		A little cleanup; moved a few things				pr
 *	12/17/92	Added HNoPurge to Get1SysXRsrc						db/pr
 *	1/24/93		Fixed double deletion of DATA handle and dispose	db/pr
 *				of code handle -- major changes to all ThinkXXX
 *				routines and THINKProc.c
 *	2/5/93		Made DriverAvail a little more efficent				pr
 *	2/6/93		Re-wrote all of the Think routines and THINKProc.c	pr
 *				so that the THINK proc is a pointer instead of a
 *				handle (needed for locked drivers).
 *	2/23/93		Passed drvrInstFlags to RemoveRAMDriver	from		pr
 *				InstallRAMDriver error
 *	10/21/93	Check for nil handles in RemoveRAMDriver			pr
 *				Zero out close block in RemoveRAMDriver
 *				Prettified GetDrvrRefNum
 *				Moved DisableInterrupts, ResetInterrupts,
 *				DrvrInstall, and DrvrRemove from driver.h to
 *				drvrincludes.h
 *					
 */

#include "drvrincludes.h"		/* Internal use and other include files	*/
#include "driver.h"				/* To be used in your application		*/

/*
 *	InstallRAMDriver will install the named driver into the system heap
 *	return the driver reference number in refNum. The drvrInstFlags
 *	parameter (defined in driver.h) specifies whether to call the
 *	driver's open routine, whether the driver is written in THINK C
 *	and therefore has a DATA resource, whether the driver has THINK C's
 *	multiple segment option turned on, and finally whether the driver
 *	is written in THINK C and can be closed and then opened again.
 *	THINK C makes some assumptions about owned resources being available
 *	which will have to be taken care of. See the ThinkInstall routine to
 *	see how it is handled.
 */
 
OSErr InstallRAMDriver(Str255 drvrName, short *refNum,
                       drvrFlagBits drvrInstFlags)
{
	register DriverPtr drvrPtr;
	register DCtlPtr ctlEntryPtr;
	ResType rsrcType;
	Handle drvrHandle;
	OSErr errCode;
	short rsrcID, unitNum;
	IOParam openBlock;
	Byte hndlState;
	
	/* Sanity check the flags */
	if(!(drvrInstFlags & thinkDATA) && (drvrInstFlags & thinkMulSeg))
		return(paramErr);

	/* Don't re-install a device driver */
	if(GetDrvrRefNum(drvrName) != 0)
		return(badUnitErr);

	/* Get the new unit number for the driver */
	errCode = DriverAvail(&unitNum);
	if(errCode != noErr)
		return(errCode);

	/*	Get the driver resource in the system heap and non-purgeable */
	errCode = Get1SysXRsrc(&drvrHandle, 'DRVR', 0, 0, drvrName);
	if(errCode != noErr)
		return(dInstErr);
	
	/* Save the resource ID for later use */
	GetResInfo(drvrHandle, &rsrcID, &rsrcType, drvrName);
	if((errCode = ResError()) == noErr) {
		/* Now detach it */
		DetachResource(drvrHandle);
		errCode = ResError();
	}
	
	/* Return on errors */
	if(errCode != noErr) {
		ReleaseResource(drvrHandle);
		return(errCode);
	}
	
	/* Patch the driver if we are using THINK C */
	if(drvrInstFlags & thinkDATA) {
		errCode = ThinkInit(drvrHandle, rsrcID, unitNum, drvrInstFlags);
		/* Return on errors */
		if(errCode != noErr) {
			DisposHandle(drvrHandle);
			return(errCode);
		}
	}
	
	/* Install DRVR with the refNum. The refNum is the -(unitNum + 1) */
	hndlState = HGetState(drvrHandle);
	HLock(drvrHandle);
	errCode = DrvrInstall(drvrHandle, ~unitNum);
	HSetState(drvrHandle, hndlState);

	/* Cleanup on errors */
	if(errCode != noErr) {
		if(drvrInstFlags & thinkDATA)
			ThinkCleanup(drvrHandle, true);
		DisposHandle(drvrHandle);
		return(errCode);
	}
	
	/* Move the important information to the driver entry */
	ctlEntryPtr = *(GetDCtlEntry(~unitNum));
	drvrPtr = *(DriverHandle)drvrHandle;
	ctlEntryPtr->dCtlDriver = (Ptr)drvrHandle;
	ctlEntryPtr->dCtlFlags = drvrPtr->drvrFlags | dRAMBased;
	ctlEntryPtr->dCtlDelay = drvrPtr->drvrDelay;
	ctlEntryPtr->dCtlEMask = drvrPtr->drvrEMask;
	ctlEntryPtr->dCtlMenu = drvrPtr->drvrMenu;
	
	/* Open the driver */
	if(drvrInstFlags & open) {
		openBlock.ioCompletion = nil;
		openBlock.ioNamePtr = drvrName;
		openBlock.ioPermssn = fsCurPerm;
		errCode = PBOpenSync((ParmBlkPtr)&openBlock);
	}
	
	/* If an error occurred during the open, remove the DRVR */
	if(errCode != noErr)
		RemoveRAMDriver(~unitNum, drvrInstFlags);
	else
		*refNum = ~unitNum;
	return(errCode);
}

/*
 *	RemoveRAMDriver removes the driver installed in the system heap by
 *	InstallRAMDriver. Pass the same flags to RemoveRAMDriver that you
 *	did to InstallRAMDriver. If THINK C resources were being used,
 *	they will be removed. (See ThinkInit and ThinkCleanup)
 */

OSErr RemoveRAMDriver(short refNum, drvrFlagBits drvrInstFlags)
{
	OSErr errCode = noErr;
	Handle drvrHandle, drvrStorage;
	DCtlHandle ctlEntryHndl;
	IOParam closeBlock;
	Byte hndlState;
	
	/* Get the driver control entry */
	if((ctlEntryHndl = GetDCtlEntry(refNum)) == nil)
		return(unitEmptyErr);
		
	/* Check for nil handle */
	if(*ctlEntryHndl == nil)
		return(nilHandleErr);
	
	/* Get the driver handle */
	drvrHandle = (Handle)(**ctlEntryHndl).dCtlDriver;

	/* If the driver is open, close it */
	if((**ctlEntryHndl).dCtlFlags & dOpened) {
		closeBlock.ioCompletion = nil;
		closeBlock.ioResult = 0;
		closeBlock.ioNamePtr = nil;
		closeBlock.ioVRefNum = 0;
		closeBlock.ioRefNum = refNum;
		closeBlock.ioPermssn = 0;
		if((errCode = PBCloseSync((ParmBlkPtr)&closeBlock)) != noErr)
			return(errCode);
	}
	
	/* Hold onto the driver storage handle */
	drvrStorage = (**ctlEntryHndl).dCtlStorage;
	
	/* Remove the driver */
	hndlState = HGetState(drvrHandle);
	HLock(drvrHandle);
	errCode = DrvrRemove(refNum);
	HSetState(drvrHandle, hndlState);
	
	/* Return on errors */
	if(errCode != noErr)
		return(errCode);
	
	/* Do THINK C stuff */
	if(drvrInstFlags & thinkDATA)
		ThinkCleanup(drvrHandle, drvrStorage != nil);

	/*
	 *	Since the driver has been detached, it will have to be disposed of
	 *	since DrvrRemove just does a ReleaseResource on the handle.
	 */
	if(drvrHandle != nil)
		DisposHandle(drvrHandle);

	return(noErr);
}

/*
 *	GetDrvrRefNum simply searches through each driver control entry
 *	for a driver with the same name as that specified in drvrName.
 *	If found, the reference number is returned. If no driver is found
 *	by that name, 0 is returned. Reads the low-memory global UnitNtryCnt.
 */

short GetDrvrRefNum(Str255 drvrName)
{
	short unitNum;
	DCtlPtr curDCtlPtr, *curDCtlHndl, **UTableEntry;
	DriverPtr curDrvrPtr;
	
	/* Start at UTableBase */
	UTableEntry = UTABLEBASE;
	
	/* Walk through the Unit Table */
	for(unitNum = 0; unitNum < UNITNTRYCNT; ++unitNum) {
	
		/* Get the entry at this point in the unit table and increment */
		curDCtlHndl = *UTableEntry++;
		
		if(curDCtlHndl != nil) {

			/* Dereference the control entry */
			curDCtlPtr = *curDCtlHndl;
			
			/* Get the pointer to the driver */
			curDrvrPtr = (DriverPtr)curDCtlPtr->dCtlDriver;

			/* If this is a RAM driver, it's a handle. ROM is a pointer */
			if((curDCtlPtr->dCtlFlags & dRAMBased) && (curDrvrPtr != nil))
				curDrvrPtr = *(DriverPtr *)curDrvrPtr;
			
			/* Does the driver name match? */
			if(curDrvrPtr != nil)
				if(EqualString(drvrName, curDrvrPtr->drvrName,
				               false, true))
					return(~unitNum);
		}
	}
	return(0);
}

/*
 *	GrowUTable increases the size of the driver unit table by newEntries.
 *	Interrupts must be disabled during this operation. Changes the
 *	low-memory globals UTableBase and UnitNtryCnt.
 */

OSErr GrowUTable(short newEntries)
{
	DCtlHandle *newUTableBase;
	short oldSR;
	
	/* Make room for the new Unit Table */
	newUTableBase =
		(DCtlHandle *)NewPtrSysClear((long)((UNITNTRYCNT + newEntries) *
		                                    sizeof(DCtlHandle)));
	if(MemError() != noErr)
		return(MemError());

	/* Any Device Manager action now would be bad! */
	oldSR = DisableInterrupts();
	
	/* Move the old Unit Table to the new Unit Table */
	BlockMove(UTABLEBASE, newUTableBase,
	          (long)(UNITNTRYCNT * sizeof(DCtlHandle)));
	DisposPtr((Ptr)UTABLEBASE);
	UTABLEBASE = newUTableBase;
	UNITNTRYCNT += newEntries;
	
	/* Renable interrupts */
	ResetStatusRegister(oldSR);

	return(noErr);
}

/*
 *	DriverAvail finds the first available slot in the unit table to
 *	install the new device driver. It will call GrowUTable if there is
 *	not enough room in the current unit table. It will return the first
 *	available slot between LOW_UNIT and UP_UNIT. Reads the low-memory
 *	global UTableBase and may change as well as read the low-memory global
 *	UnitNtryCnt.
 */

OSErr DriverAvail(short *unitNum)
{
	short unitIndex;
	Size UTableSize;
	OSErr errCode = noErr;

#define LOW_UNIT		48		/* First Unit Table Entry to use		*/
#define NEW_UNIT		64		/* Size of a "normal" Unit Table		*/
#define MAX_UNIT		128		/* Maximum size of a Unit Table			*/
#define UP_UNIT			4		/* Size to bounce up Unit Table			*/
	
	*unitNum = 0;
	UTableSize = GetPtrSize((Ptr)UTABLEBASE) / sizeof(DCtlHandle);
	
	/* See if we have less than the minimum number of unit entries */
	if(UNITNTRYCNT <= LOW_UNIT) {

		/* Try to make Unit Table the standard size */
		if(UTableSize < NEW_UNIT)
			errCode = GrowUTable(NEW_UNIT - UTableSize);

		/* If there is an error, see if there is enough room anyway */
		if(errCode != noErr) {
			if(UTableSize > LOW_UNIT) {
				UNITNTRYCNT = (UNITNTRYCNT + 4 < UTableSize ?
				               UNITNTRYCNT + 4 :
				               UTableSize);
				errCode = noErr;
			}
		}
		if(errCode == noErr)
			*unitNum = LOW_UNIT;
		return(errCode);
	}
	
	/* Look for an empty slot in what's already there */
	for(unitIndex = LOW_UNIT;
	    (unitIndex < UNITNTRYCNT) && (*unitNum == 0);
	    ++unitIndex)
		if(UTABLEBASE[unitIndex] == nil)
			*unitNum = unitIndex;
	
	/* Unit Table full up to UnitNtryCnt, so increase it */
	if(*unitNum == 0) {

		/* If there is space in the Unit Table, just up the count */
		if(UTableSize > UNITNTRYCNT) {
			*unitNum = UNITNTRYCNT;
			UNITNTRYCNT += (UTableSize - UNITNTRYCNT < UP_UNIT ?
			                UTableSize - UNITNTRYCNT :
			                UP_UNIT);
		
		/* If there isn't enough space, try to make it bigger */
		} else {
			if(MAX_UNIT - UNITNTRYCNT != 0) {
				unitIndex = UNITNTRYCNT;
				errCode = GrowUTable(MAX_UNIT - UNITNTRYCNT < UP_UNIT ?
				                     MAX_UNIT - UNITNTRYCNT :
				                     UP_UNIT);
				if(errCode != noErr)
					return(errCode);
				*unitNum = unitIndex;
			}
		}
	}
	if(*unitNum == 0)
		return(unitTblFullErr);
	else
		return(noErr);
}


/*
 *	Get1SysXRsrc gets a handle to the requested resource making sure that
 *	both the resource itself and the master pointer are in the system heap
 *	and non-purgeable. The resource is returned in rsrcHndl. The resource
 *	will be retrieved according to resource type and either resource name,
 *	or resource index, or resource ID, in that order, whichever is
 *	non-zero.
 */

OSErr Get1SysXRsrc(Handle *rsrcHndl, ResType rsrcType, short rsrcID,
                   short rsrcInd, StringPtr rsrcName)
{
	THz tempZone, tempSysZone;
	OSErr errCode, ptrCode = noErr;
	
	/* Make sure everything loads in the system heap */
	tempZone = GetZone();
	tempSysZone = SystemZone();
	SetZone(tempSysZone);
	SetResLoad(true);
	
	/* Get the resource and make sure it is in the system heap */
	do {
		errCode = ptrCode;
		if(rsrcName != nil)
			*rsrcHndl = Get1NamedResource(rsrcType, rsrcName);
		else if(rsrcInd != 0)
			*rsrcHndl = Get1IndResource(rsrcType, rsrcInd);
		else
			*rsrcHndl = Get1Resource(rsrcType, rsrcID);
		if(*rsrcHndl != nil) {
			if(PtrInZone(tempSysZone, (Ptr)*rsrcHndl) &&
			   PtrInZone(tempSysZone, (Ptr)**rsrcHndl)) {
				HNoPurge(*rsrcHndl);
				errCode = noErr;
			} else {
				ReleaseResource(*rsrcHndl);
				*rsrcHndl = nil;
				ptrCode = memAZErr;
			}
		} else {
			if((errCode = ResError()) == noErr)
				errCode = resNotFound;
		}
	} while(errCode == noErr && *rsrcHndl == nil);
	
	/* Restore the zone to what it was */
	SetZone(tempZone);
	return(errCode);
}

/*
 *	PtrInZone just checks to see whether the specified pointer is within
 *	the specified zone.
 */

Boolean PtrInZone(THz theZone, Ptr thePtr)
{
	register unsigned long stripMask, testPtr, dataStart, dataLim;
	
	stripMask = (unsigned long)StripAddress((void *)0xFFFFFFFF);
	dataStart = (unsigned long)&theZone->heapData & stripMask;
	dataLim = (unsigned long)theZone->bkLim & stripMask;
	testPtr = (unsigned long)thePtr & stripMask;
	return((testPtr < dataLim) && (testPtr >= dataStart));
}

/*
 *	THINK C uses a DATA resource for storage and DCOD resources for
 *	multiple segments in device drivers. THINK C makes two pretty bad
 *	assumptions about these resources: (a) that the resource file will
 *	be open when the resources are needed; and (b) that the resources
 *	will be "owned" by the driver. Resource ownership is described in
 *	Inside Macintosh I, but the important point is that the driver assumes
 *	that its unit table number will correspond to its own resource ID and
 *	to the resource ID's of resources it owns. Since the driver is going
 *	to be installed in the first free unit table entry and will be detached
 *	in the system heap, both of these assumptions are bad. To correct
 *	these problems, the THINK C driver glue is patched. A pointer to a
 *	small block of code is loaded into the system heap. Occurances of
 *	GetResource calls in the THINK C driver glue are replaced by calls to
 *	the new routine. The handles to the DATA (and DCOD) resource(s) are
 *	stored along with the routine. So, when THINK C goes looking for its
 *	owned resource(s), the appropriate handle(s) are returned instead.
 *
 *	ThinkInit gets all of the resources needed and installs the patch to
 *	the THINK C glue. It takes as parameters the handle to the driver that
 *	is to be patched, the original resource ID of the driver (to figure out
 *	the resource ID's of the DATA and DCOD resources), the current unit
 *	table number, and the drvrInstFlags.
 */

OSErr ThinkInit(Handle drvrHandle, short rsrcID, short unitNum,
                drvrFlagBits drvrInstFlags)
{
	Handle codeHandle = nil;
	Ptr codePtr;
	Size codeSize;
	OSErr errCode;
	
	/* Get the new procedure code */
	codeHandle = Get1Resource('PROC', THINK_PROC);
	if(codeHandle == nil) {
		if((errCode = ResError()) == noErr)
			errCode = resNotFound;
		return(errCode);
	}

	/* Add the DATA resource to the end of the code handle */
	errCode = ThinkAddRsrcs(codeHandle, 'DATA', rsrcID, unitNum);

	if(errCode == noErr) {
		/* Add the DCOD resources if this is a multi-segment driver */
		if(drvrInstFlags & thinkMulSeg)
			errCode = ThinkAddRsrcs(codeHandle, 'DCOD', rsrcID, unitNum);
		
		if(errCode == noErr) {
		
			/* Create a pointer in the system heap to hold the code */
			codeSize = GetHandleSize(codeHandle);
			codePtr = NewPtrSys(codeSize);
			if(codePtr == nil)
				errCode = memFullErr;
			else
				BlockMove(*codeHandle, codePtr, codeSize);
		}

		/* Cleanup on errors */
		if(errCode != noErr) {
			HLock(codeHandle);
			ThinkRmvRsrcs(*codeHandle, true, true);
		}
	}
	
	/* Get rid of the code resource */
	ReleaseResource(codeHandle);

	if(errCode != noErr)
		return(errCode);
	
	/* Patch the driver code for the DATA resource. */
	errCode = ThinkChangeCode(drvrHandle, codePtr, 'DATA');
	
	/* If there are multiple segments, take care of them */
	if((errCode == noErr) && (drvrInstFlags & thinkMulSeg))
		errCode = ThinkChangeCode(drvrHandle, codePtr, 'DCOD');

	/* Cleanup on errors */
	if(errCode != noErr)
		ThinkRmvRsrcs(codePtr, false, true);
	
	return(errCode);
}

/*
 *	ThinkChangeCode changes the piece of the THINK C device driver glue
 *	that has been compiled into the driver code. The call to _GetResource
 *	that retrieves the driver's owned resources is replaced by a procedure
 *	call (JSR) to some new code. The procedure that is to be called
 *	instead, which is compiled separately and must be in this resource
 *	file, is loaded into the system heap and its address is patched into
 *	the driver code.
 */

OSErr ThinkChangeCode(Handle drvrHandle, Ptr codePtr, ResType theType)
{
	OSErr errCode = noErr;
	CodeChunk oldCode, newCode;
	long *chunkPtr;

	/* Fill in oldCode with what the THINK C glue looks like now */
	chunkPtr = (long *)&oldCode;
	*chunkPtr++ = 0x50F80A5E;		/* ST ResLoad */
	*chunkPtr++ = 0x598F2F3C;		/* SUBQ.L #4,A7 ; MOVE.L Imm.L,-(A7) */
	*chunkPtr++ = theType;			/* ResType for _GetResource */
	*chunkPtr++ = 0x3F00A9A0;		/* MOVE.W D0,-(A7) ; _GetResource */
	*(short *)chunkPtr = 0x201F;	/* MOVE.L (A7)+,D0 */

	/* Fill in newCode with what we want the THINK C glue to look like */
	chunkPtr = (long *)&newCode;
	*chunkPtr++ = 0x598F2F3C;		/* SUBQ.L #4,A7 ; MOVE.L Imm.L,-(A7) */
	*chunkPtr++ = theType;			/* ResType for the handle */
	*chunkPtr++ = 0x3F004EB9;		/* MOVE.W D0,-(A7) ; JSR Imm.L */
	*chunkPtr++ = (long)codePtr;	/* Code address for JSR */
	*(short *)chunkPtr = 0x201F;	/* MOVE.L (A7)+,D0 */

	/* Search for the appropriate call to GetResource and fix it */
	if(Munger(drvrHandle, 0L,
	          &oldCode, sizeof(CodeChunk),
	          &newCode, sizeof(CodeChunk)) < 0)
		return(paramErr);
	else
		return(noErr);
}

/*
 *	ThinkAddRsrcs adds the appropriate resource handle(s), ID(s), and an
 *	indicator for type (0 for DATA and 1 for DCOD) to the end of the code
 *	that is passed in so that they can be found when the code is called.
 */

OSErr ThinkAddRsrcs(Handle codeHandle, ResType theType, short rsrcID,
                    short unitNum)
{
	OSErr errCode = noErr;
	short index;
	RsrcRec resRec;
	Str255 rsrcName;
	
	/* Find out how many resources of the type there are */
	index = Count1Resources(theType) - 1;
	if(index < 0)
		return(resNotFound);
	
	do {
		/* Get the indexed resource */
		errCode = Get1SysXRsrc(&resRec.rsrc, theType, 0, index + 1, nil);
		if(errCode == noErr) {
			GetResInfo(resRec.rsrc, &resRec.id, &theType, rsrcName);
			
			/*
			 *	If the GetResInfo failed or the ID says that this is not
			 *	owned by the driver, get rid of the resource. Otherwise
			 *	detach it.
			 */
			if(((errCode = ResError()) != noErr) ||
			   (((resRec.id & 0x0FE0) >> 5) != rsrcID)) {
					ReleaseResource(resRec.rsrc);
			} else {
				DetachResource(resRec.rsrc);
				if((errCode = ResError()) == noErr) {

					/* Set the saved resource ID to what it "should" be */
					resRec.id &= ~0x0FE0;
					resRec.id |= (unitNum << 5);
					resRec.typ = theType;
					
					/* Add the handle to the end of the code */
					errCode = PtrAndHand(&resRec, codeHandle,
					                     sizeof(resRec));
					if(errCode != noErr)
						DisposHandle(resRec.rsrc);
				} else {
					ReleaseResource(resRec.rsrc);
				}
			}
		}
	} while((errCode == noErr) && (--index != -1));
	
	return(errCode);
}

/*
 *	ThinkCleanup gets rid of the THINK C detached resources.
 */

void ThinkCleanup(Handle drvrHandle, Boolean rmvDATA)
{
	CodeChunk newCode, *drvrChunk;
	long codeOffset = 0L, codeSize, *chunkPtr;
	Ptr codePtr = nil;

	/* Fill in newCode with what the fixed THINK C glue looks like */
	chunkPtr = (long *)&newCode;
	*chunkPtr++ = 0x598F2F3C;		/* SUBQ.L #4,A7 ; MOVE.L Imm.L,-(A7) */
	*chunkPtr++ = 'DATA';			/* ResType for the handle */
	*chunkPtr++ = 0x3F004EB9;		/* MOVE.W D0,-(A7) ; JSR Imm.L */
	*chunkPtr++ = 0L;				/* Code address for JSR */
	*(short *)chunkPtr = 0x201F;	/* MOVE.L (A7)+,D0 */

	codeSize = sizeof(newCode) -
	           (sizeof(newCode.p3) + sizeof(newCode.inst7));

	/* Look for an occurance of "fixed" driver glue */
	do {
		codeOffset = Munger(drvrHandle, codeOffset, &newCode,
		                    codeSize, nil, 0);
		
		/* If we found something ... */
		if(codeOffset >= 0) {

			drvrChunk = (CodeChunk *)*drvrHandle + codeOffset;
			
			/* Make sure the last instruction matches */
			if(drvrChunk->inst7 == newCode.inst7)
				codePtr = drvrChunk->p3.codePtr;
				
		}
	} while((codePtr == nil) && (codeOffset >= 0));
	
	if(codePtr != nil) {		
		ThinkRmvRsrcs(codePtr, false, rmvDATA);
		DisposPtr(codePtr);
	}
}

/*
 *	ThinkRmvRsrcs disposes of the handles attached to the end of the code
 *	pointer (unless the handle is marked as the DATA resource has already
 *	been disposed of by the driver glue) and sets them to nil.
 */

void ThinkRmvRsrcs(Ptr codePtr, Boolean isHandle, Boolean rmvDATA)
{
	Size codeSize;
	RsrcRec *resPtr;
	
	if(isHandle)
		codeSize = GetHandleSize(RecoverHandle(codePtr));
	else
		codeSize = GetPtrSize(codePtr);

	/* Find the end of the code resource and work back until done */
	resPtr = (RsrcRec *)(codePtr + codeSize);
	while((--resPtr)->rsrc != nil) {
		if(rmvDATA || resPtr->typ != 'DATA')
			DisposHandle(resPtr->rsrc);
		resPtr->rsrc = nil;
	}
}