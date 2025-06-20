//----------------------------------------------------------------------------------------
// UMemory.cp
// Copyright � 1985-1994 by Apple Computer, Inc.  All rights reserved.
//----------------------------------------------------------------------------------------

#ifndef __STDIO__
#include <stdio.h>
#endif


#ifndef __LOWMEM__
#include <LowMem.h>
#endif

//----------------------------------------------------------------------------------------
// Globals to this module and externally available.

ObjectHeap* gObjectHeap;

Size gMaxLockedRsrc;

short gCodeRefNum;

// Static, globals to this module. Many of these are static by convention, but we don't
// declare them as static because people may want to examine them.

#if !qPowerPC
Boolean gUnloadAllSegs;
#endif

UniversalProcPtr gGZPurgeNotify;

Handle pMemReserve = nil;
Boolean pPermAllocation = false;

Size pSzMemReserve;

// local private globals
static Boolean pDuringGrowZone = false;

// Function prototypes for some routines in this module.

pascal long GrowZoneProc(Size needed);

// Function prototypes for the routines in UMemory.a

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------

// These "MAFoo" functions were originally for THINK Pascal compatibility (but useful in the
// larger problem of multiple open resource maps in general); when running under the THINK
// environment, CODE resources are not found in the same resource file as other
// application resources, so a UseResFile call needs to be made to bring the project
// resource file into the search path. "gCodeRefNum" is set up at initialization time.

//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// MAGet1Resource: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

Handle MAGet1Resource(ResType rType, short rID)
{
	short oldResFile;
	Handle resource;

	oldResFile = MAUseResFile(gCodeRefNum);
	resource = Get1Resource(rType, rID);
	MAUseResFile(oldResFile);

	return resource;
} // MAGet1Resource 

//----------------------------------------------------------------------------------------
// MAGet1NamedResource: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

Handle MAGet1NamedResource(ResType rType, const CStr255 &name)
{
	short oldResFile;
	Handle namedResource;

	oldResFile = MAUseResFile(gCodeRefNum);
	namedResource = Get1NamedResource(rType, name);
	MAUseResFile(oldResFile);

	return namedResource;
} // MAGet1NamedResource 

//----------------------------------------------------------------------------------------
// MAGet1IndResource: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

Handle MAGet1IndResource(ResType rType, short index)
{
	short oldResFile;
	Handle indResource;

	oldResFile = MAUseResFile(gCodeRefNum);
	indResource = Get1IndResource(rType, index);
	MAUseResFile(oldResFile);

	return indResource;
} // MAGet1IndResource 

//----------------------------------------------------------------------------------------
// MACount1Resources: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

short MACount1Resources(ResType rType)
{
	short oldResFile;
	short resourceCnt;

	oldResFile = MAUseResFile(gCodeRefNum);
	resourceCnt = Count1Resources(rType);
	MAUseResFile(oldResFile);

	return resourceCnt;
} // MACount1Resources 

//----------------------------------------------------------------------------------------
// MAGetResource: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

Handle MAGetResource(ResType rType, short rID)
{
	Handle h;
	short oldResFile;

	oldResFile = MAUseResFile(gCodeRefNum);
	h = GetResource(rType, rID);
	MAUseResFile(oldResFile);

	return h;
} // MAGetResource 

//----------------------------------------------------------------------------------------
// MAGetNamedResource: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

Handle MAGetNamedResource(ResType rType, const CStr255 &name)
{
	Handle h;
	short oldResFile;

	oldResFile = MAUseResFile(gCodeRefNum);
	h = GetNamedResource(rType, name);
	MAUseResFile(oldResFile);

	return h;
} // MAGetNamedResource 

//----------------------------------------------------------------------------------------
// MAGetIndResource: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

Handle MAGetIndResource(ResType rType, short index)
{
	Handle h;
	short oldResFile;

	oldResFile = MAUseResFile(gCodeRefNum);
	h = GetIndResource(rType, index);
	MAUseResFile(oldResFile);

	return h;
} // MAGetIndResource 

//----------------------------------------------------------------------------------------
// MACountResources: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

short MACountResources(ResType rType)
{
	short oldResFile;
	short resourceCnt;

	oldResFile = MAUseResFile(gCodeRefNum);
	resourceCnt = CountResources(rType);
	MAUseResFile(oldResFile);

	return resourceCnt;
} // MACountResources 

//----------------------------------------------------------------------------------------
// IsHandleEligible: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

Boolean IsHandleEligible(Handle h)
{
	if (h == NULL)
		return false;							// Thanks Pillar! 
	else if (IsHandlePurged(h))
		return false;
	else
		return (h != LMGetGZMoveHnd()) && (h != LMGetGZRootHnd());
} // IsHandleEligible 

//----------------------------------------------------------------------------------------
// BuildAllReserves: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

void BuildAllReserves()
{
	static const short initVal = 0xF7;

	Boolean oldPerm;
#if qDebug
	Size theSize;
#endif

	// set the permanent flag to ensure that the code reserve is actually allocated and
    // not given up to the low space reserve

	oldPerm = pPermAllocation;
	pPermAllocation = true;

	// reallocate the low space handle, if necessary

	if (IsHandlePurged(pMemReserve))
	{
		ReallocateHandle(pMemReserve, pSzMemReserve);
#if qDebug
		theSize = GetHandleSize(pMemReserve);
		if (theSize != 0)
			BlockSet(*pMemReserve, theSize, initVal);
#endif

	}

	pPermAllocation = oldPerm;					// reset the permanent flag 
} // BuildAllReserves 

//----------------------------------------------------------------------------------------
// CheckReserve: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

Boolean CheckReserve()
{
	BuildAllReserves();
	return pMemReserve && *pMemReserve;
} // CheckReserve 

//----------------------------------------------------------------------------------------
// CheckRsrcUsage: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

void CheckRsrcUsage()
{
	// Always OK!
}
//----------------------------------------------------------------------------------------
// DoInitUMemory: Called from InitUMemory so that InitUMemory can be in the main segment
// and this code can be in another (unloadable) segment.
//----------------------------------------------------------------------------------------
#pragma segment MAMiniInit

void DoInitUMemory(Size &sizeObjectHeap,
				   Size &sizeHeapIncrement,
				   Size &sizeTempReserve,
				   Size &sizeLowSpaceReserve)
{
#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
	struct Mem
	{
		long objectHeap, heapIncrement, codeVal, lowSpaceVal, stackVal;
	};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

	typedef Mem *MemPtr, 
    **MemHandle;

	short i, 
	rsrcCnt;
	Handle h;
	long stackTot;

	// Initialize memory management globals

	pPermAllocation = false;
	pMemReserve = NewHandle(0);
	FailNIL(pMemReserve);

	pSzMemReserve = 0;

	gGZPurgeNotify = NULL;

#if !qPowerPC
	gUnloadAllSegs = true;
#endif

	// Compute memory slop needed

	sizeObjectHeap = 0;
	sizeHeapIncrement = 0;
	sizeTempReserve = 0;
	sizeLowSpaceReserve = 0;
	stackTot = 0;

	// Sum up the standard memory requirements
	
	rsrcCnt = CountResources('mem!');
	for (i = 1; i <= rsrcCnt; ++i)
	{
		h = GetIndResource('mem!', i);
		{
			const Mem &memH = **((MemHandle)h);

			sizeObjectHeap += memH.objectHeap;
			sizeHeapIncrement += memH.heapIncrement;
			sizeTempReserve += memH.codeVal;
			sizeLowSpaceReserve += memH.lowSpaceVal;
			stackTot += memH.stackVal;
		}
		ReleaseResource(h);
	}
	
	// Add on the processor dependent memory requirements

#if qPowerPC
	const ResType kCPUmemResType = 'ppc!';
#else
	const ResType kCPUmemResType = '68k!';
#endif

	rsrcCnt = CountResources(kCPUmemResType);
	for (i = 1; i <= rsrcCnt; ++i)
	{
		h = GetIndResource(kCPUmemResType, i);
		{
			const Mem &memH = **((MemHandle)h);

			sizeObjectHeap += memH.objectHeap;
			sizeHeapIncrement += memH.heapIncrement;
			sizeTempReserve += memH.codeVal;
			sizeLowSpaceReserve += memH.lowSpaceVal;
			stackTot += memH.stackVal;
		}
		ReleaseResource(h);
	}
#if qDebug & 0
	fprintf(stderr, "sizeObjectHeap = %ld, sizeHeapIncrement = %ld\n", sizeObjectHeap, sizeHeapIncrement);
	fprintf(stderr, "sizeTempReserve = %ld, sizeLowSpaceReserve = %ld, stackTot = %ld\n", sizeTempReserve, sizeLowSpaceReserve, stackTot);
#endif
	SetStackSpace(stackTot);
	SetReserveSize(0, sizeLowSpaceReserve);

	MaxApplZone();

} // DoInitUMemory 

//----------------------------------------------------------------------------------------
// FailNoReserve: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

void FailNoReserve()
{
	if (!CheckReserve())
		Failure(memFullErr, 0);
} // FailNoReserve 

//----------------------------------------------------------------------------------------
// FailSpaceIsLow: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

void FailSpaceIsLow()
{
	if (MemSpaceIsLow())
		Failure(memFullErr, 0);
} // FailSpaceIsLow 

//----------------------------------------------------------------------------------------
// GetReserveSize: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

void GetReserveSize(Size &szCodeReserve, Size &szMemReserve)
{
	szCodeReserve = 0;
	szMemReserve = pSzMemReserve;
} // GetReserveSize 

//----------------------------------------------------------------------------------------
// GetSegNumber: Gets seg number from a Jump table address.
//
// Must be in Main segment because we call this in order to make the resident segment
// resident.
//----------------------------------------------------------------------------------------
#ifndef powerc
#pragma segment MAMemoryRes

short GetSegNumber(ProcPtr /* aProc */) // Dummy for SetResidentSegment in UMacAppGlobals
{
	return -1;
}
#endif
//----------------------------------------------------------------------------------------
// GrowZoneProc: 
//----------------------------------------------------------------------------------------

#pragma segment MAMemoryRes

pascal long GrowZoneProc(Size)
{
	long result; 
	long OldA5 = SetCurrentA5();						// Can be called from other worlds 

	result = 0;									// default is to fail 

	if (!pDuringGrowZone)						// prevent re-entrancy 
	{
		pDuringGrowZone = true;
		// last ditch attempt-free emergency reserve
		if (IsHandleEligible(pMemReserve))
		{
			EmptyHandle(pMemReserve);
			result = 1;
		}
		pDuringGrowZone = false;
	}
	OldA5 = SetA5(OldA5);
	return result;
}

//----------------------------------------------------------------------------------------
// InstallGrowZoneProc: Once called the grow zone proc's segment CANNOT be moved since
// we're passing a NON-JT address to SetGrowZone (so we can be called from "other worlds"
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes
// Must be in same segment as grow zone proc

void InstallGrowZoneProc()
{
	pDuringGrowZone = false;

	GrowZoneUPP gzUPP = NewGrowZoneProc(&GrowZoneProc);
	FailNIL(gzUPP);
	SetGrowZone(gzUPP);
} // InstallGrowZoneProc 


//----------------------------------------------------------------------------------------
// InitUMemory: 
//----------------------------------------------------------------------------------------
#pragma segment Main
// Must be in main segment and called from main segment

void InitUMemory(short callsToMoreMasters)
{
	Size heapSize, heapIncrement, codeRes, lowSpaceRes = 0;
	THz applZone;
	short oldMoreMast;

	DoInitUMemory(heapSize, heapIncrement, codeRes, lowSpaceRes);
	InstallGrowZoneProc();
	// Here is a trick suggested by Jerome C.--it allocates one large block of master
	// pointers
	applZone = ApplicationZone();
	oldMoreMast = applZone->moreMast;
	long numMasterPointersDesired = oldMoreMast * callsToMoreMasters;
	do {
		applZone->moreMast = (short) Min(numMasterPointersDesired, kMaxShort);
		MoreMasters();
		numMasterPointersDesired -= applZone->moreMast;
	} while (numMasterPointersDesired > 0);
	applZone->moreMast = oldMoreMast;
	gUMemoryInitialized = true;

	// Now fully initialized, so create our pointer based heap for object allocation.
    // Now it will come out of permanent memory.
	
	if (gObjectHeap == NULL)
	{
		gObjectHeap = new ObjectHeap((size_t)heapSize, (size_t)heapIncrement);
		gObjectHeap->IObjectHeap();
	}
	else
	{
		gObjectHeap->ExpandHeap((size_t)heapSize, (size_t)heapIncrement);
	}
} // InitUMemory 


//----------------------------------------------------------------------------------------
// MemSpaceIsLow: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

Boolean MemSpaceIsLow()
{
	BuildAllReserves();
	return IsHandlePurged(pMemReserve);
} // MemSpaceIsLow 

//----------------------------------------------------------------------------------------
// NewPermHandle: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

Handle NewPermHandle(Size logicalSize)
{
	static const short initVal = 0xF3;			// odd at all byte boundaries			

	Boolean priorPerm;
	Handle aHandle;

	priorPerm = PermAllocation(true);
	aHandle = NewHandle(logicalSize);
	pPermAllocation = priorPerm;
	FailNIL(aHandle);
#if qDebug
	BlockSet(*aHandle, logicalSize, initVal);
#endif
	return aHandle;
} // NewPermHandle 

//----------------------------------------------------------------------------------------
// NewPermPtr: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

Ptr NewPermPtr(Size logicalSize)
{
	static const short initVal = 0xF3;			// odd at all byte boundaries			

	Boolean priorPerm;
	Ptr aPtr;


	priorPerm = PermAllocation(true);
	aPtr = NewPtr(logicalSize);
	pPermAllocation = priorPerm;
	FailNIL(aPtr);
#if qDebug
	BlockSet(aPtr, logicalSize, initVal);
#endif
	return aPtr;
} // NewPermPtr 

//----------------------------------------------------------------------------------------
// PermAllocation: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

Boolean PermAllocation(Boolean permanent)
{
	Boolean permAllocation = pPermAllocation;

	if (permanent != pPermAllocation)
	{
		pPermAllocation = permanent;
		if (permanent)
			BuildAllReserves();
	}

	return permAllocation;
} // PermAllocation 

//----------------------------------------------------------------------------------------
// PermHandToHand: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

void PermHandToHand(Handle& theHndl) 
{
	Boolean priorPerm;
	OSErr err;
	
	priorPerm = PermAllocation(true);
	err = HandToHand(&theHndl);
	pPermAllocation = priorPerm;
	
	FailOSErr(err);	
	
} // PermHandToHand 


//========================================================================================
// GLOBAL Procedures
//========================================================================================
#undef Inherited

//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

void SetPermHandleSize(Handle h, Size newSize)
{
	static const short initVal = 0xF3;

	Boolean priorPerm;
#if qDebug
	Size oldSize;
#endif

	priorPerm = PermAllocation(true);
#if qDebug
	oldSize = GetHandleSize(h);
#endif

	SetHandleSize(h, newSize);
	pPermAllocation = priorPerm;		// Since we are in the memory unit we can break
                                        // the encapsulation of the PermAllocation Call to
                                        // just set the pPermAllocation flag back
                                        // directly. This lets us be assured that no
                                        // operations have occurred that would invalidate
                                        // the MemErr flag thus the following call will
                                        // give a true result
	FailMemError();
#if qDebug
	if (oldSize < newSize)
		BlockSet((Ptr) * h + oldSize, newSize - oldSize, initVal);
#endif

} // SetPermHandleSize 

//----------------------------------------------------------------------------------------
// SetPermPtrSize: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

void SetPermPtrSize(Ptr p, Size newSize)
{
	static const short initVal = 0xF5;

	Boolean priorPerm;
#if qDebug
	Size oldSize;
#endif

	priorPerm = PermAllocation(true);
#if qDebug
	oldSize = GetPtrSize(p);
#endif

	SetPtrSize(p, newSize);
	pPermAllocation = priorPerm;		// Since we are in the memory unit we can break
                                        // the encapsulation of the PermAllocation Call to
                                        // just set the pPermAllocation flag back
                                        // directly. This lets us be assured that no
                                        // operations have occurred that would invalidate
                                        // the MemErr flag thus the following call will
                                        // give a true result
	FailMemError();
#if qDebug
	if (oldSize < newSize)
		BlockSet((Ptr)p + oldSize, newSize - oldSize, initVal);
#endif

} // SetPermPtrSize 

//----------------------------------------------------------------------------------------
// SetReserveSize: 
//----------------------------------------------------------------------------------------
#pragma segment MAMemoryRes

void SetReserveSize(Size /* forCode */, Size forOther)
{
#if qDebug & 0
	fprintf(stderr, "SetReserveSize: %ld -> %ld\n", pSzMemReserve, forOther);
#endif
	pSzMemReserve = forOther;

	// Since the numbers have changed, make sure we start from scratch. 
	EmptyHandle(pMemReserve);
	BuildAllReserves();
}
//----------------------------------------------------------------------------------------
// SetResidentSegment: 
//----------------------------------------------------------------------------------------
void SetResidentSegment(short /* segnum */, Boolean /* makeResident */)
{
	// Everything is resident for Nuntius
}

//----------------------------------------------------------------------------------------
// SetStackSpace: 
//----------------------------------------------------------------------------------------
#pragma segment MAMiniInit

void SetStackSpace(long numBytes)
{
	long newLimit;

	// Make sure numBytes is even
	if (odd(numBytes)) numBytes++;

	newLimit = (long)LMGetCurStackBase() - numBytes;
	if ((long)GetApplLimit() > newLimit)
		SetApplLimit((Ptr)newLimit);
} // SetStackSpace 

//----------------------------------------------------------------------------------------
// UnloadAllSegments: 
//----------------------------------------------------------------------------------------
#if !qPowerPC
#pragma segment MAMemoryRes
// must be in Main segment 

void UnloadAllSegments()
{
	// Never unload in Nuntius
}
#endif
