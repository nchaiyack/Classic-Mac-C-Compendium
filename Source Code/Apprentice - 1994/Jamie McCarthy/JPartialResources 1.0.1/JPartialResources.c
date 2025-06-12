/*
 * JPartialResources.c
 *
 * Jamie's partial resource calls that work under any system.
 * © Copyright 1992 by Jamie R. McCarthy.  All rights reserved.
 * This code can be both distributed and used freely.
 * Internet: k044477@kzoo.edu			AppleLink: j.mccarthy
 * Telephone:  800-421-4157 or US 616-665-7075 (9:00-5:00 Eastern time)
 * I'm releasing this code with the hope that someone will get something
 * out of it.  Feedback of any sort, even just letting me know that you're
 * using it, is greatly appreciated!
 *
 *
 * "Be aware that having a copy of a resource in memory when you are using
 *  the partial resource routines may cause problems.  If you have modified
 *  the copy in memory and then access the resource on disk using either
 *  the ReadPartialResource or WritePartialResource procedure, you will
 *  lose changes made to the copy in memory."  - IM VI, 13-21
 *
 * By "copy in memory," this refers to having the entire resource loaded in.
 * What it's saying is that ReadPartialResource and WritePartialResource
 * consider themselves free to always purge (and maybe reload) the resource
 * handle.  Incidentally, this code doesn't do that--it read directly from
 * disk--so if you're under system 6, your copy of the resource is safe.
 * (Memory may be moved, however, so if the resource is purgeable, it _may_
 * be purged.)
 *
 *
 * "...there's really no reason to access resources directly."  - IM IV-16.
 *
 * Yeah right.
 *
 */



/******************************/

#include "JPartialResources.h"

/******************************/

#include <GestaltEqu.h>

/******************************/

#define kUninitializedFlag (42)
Boolean partialResourceCallsAreAvailable = kUninitializedFlag;

void determineWhetherPRCallsAreAvailable(void);

/******************************/

typedef struct {
	unsigned long		offsetToRsrcData;
	unsigned long		offsetToRsrcMap;
	unsigned long		lengthOfRsrcData;
	unsigned long		lengthOfRsrcMap;
} rsrcForkHeader;

typedef struct {
	short					rsrcID;
	short					offsetToRsrcName;
	unsigned long		rsrcAttrs : 8;
	unsigned long		offsetToRsrcData : 24;
	Handle				rsrcHndl;
} rsrcReference;

struct rsrcMap;
typedef struct rsrcMap rsrcMap, *rsrcMapPtr, **rsrcMapHndl;
struct rsrcMap {
	rsrcForkHeader		copyOfRsrcForkHeader;
	rsrcMapHndl			nextRsrcMapHndl;
	short					fileRefNum;
	short					fileAttrs;
	unsigned short		offsetToTypeList;
	unsigned short		offsetToNameList;
} ;

/******************************/

		/*
		 * Lower-level internal functions.
		 */
		
	/*
	 * Just as _HomeResFile returns the refNum of a resource's file,
	 * so does getHomeRsrcMap() return a pointer to the resource
	 * file's map.  Warning--any calls that may move memory,
	 * including system memory, invalidate the pointer!
	 */
rsrcMapPtr getHomeRsrcMap(Handle theResource);

	/*
	 * This uses _RsrcMapEntry and getHomeRsrcMap() to find the
	 * offset from the beginning of a resource fork to the start of
	 * a resource's reference.
	 */
long getRsrcRefOffsetIntoFile(Handle theResource);

/******************************/

		/*
		 * Higher-level internal functions.
		 */
		
	/*
	 * This uses getRsrcRefOffsetIntoFile() and FSRead() to find
	 * the offset from the beginning of a resource fork to the start
	 * of a resource's data.  The offset points to the data itself,
	 * which is just past the longword that tells its size.  If you
	 * happen to want the size too, use _SizeResource.
	 */
long getRsrcDataOffsetIntoFile(Handle theResource);

long getRsrcFileMark(Handle theResource);
void setRsrcFileMark(Handle theResource, long newMark);

/******************************/



void jReadPartialResource(Handle theResource, long offset, void *buffer, long count)
{
	if (partialResourceCallsAreAvailable == kUninitializedFlag) {
		determineWhetherPRCallsAreAvailable();
	}
	
	if (partialResourceCallsAreAvailable) {
		ReadPartialResource(theResource, offset, buffer, count);
	} else {
		
		long theRsrcDataOffset;
		long theRsrcDataLength;
		OSErr theErr;
		long oldMark;
		
		oldMark = getRsrcFileMark(theResource);
		theRsrcDataOffset = getRsrcDataOffsetIntoFile(theResource);
		if (theRsrcDataOffset < 0) return ;
		setRsrcFileMark(theResource, theRsrcDataOffset + offset);
		theErr = FSRead( HomeResFile(theResource), &count, buffer );
		setRsrcFileMark(theResource, oldMark);
		
	}
}



void jWritePartialResource(Handle theResource, long offset, void *buffer, long count)
{
	if (partialResourceCallsAreAvailable == kUninitializedFlag) {
		determineWhetherPRCallsAreAvailable();
	}
	
		/*
		 * Disallow writing past the end of the resource;
		 * try to increase its size if possible.
		 */
	if (SizeResource(theResource) < offset+count) {
		jSetResourceSize(theResource, offset+count);
	}
	
		/*
		 * Check to be sure the size was successfully increased
		 * before continuing.
		 */
	if (SizeResource(theResource) < offset+count) {
		return ;
	}
	
	if (partialResourceCallsAreAvailable) {
		WritePartialResource(theResource, offset, buffer, count);
	} else {
		
		long theRsrcDataOffset;
		OSErr theErr;
		long oldMark;
		
		oldMark = getRsrcFileMark(theResource);
		theRsrcDataOffset = getRsrcDataOffsetIntoFile(theResource);
		if (theRsrcDataOffset < 0) return;
		setRsrcFileMark(theResource, theRsrcDataOffset + offset);
		theErr = FSWrite( HomeResFile(theResource), &count, buffer );
		setRsrcFileMark(theResource, oldMark);
		
	}
}



void jSetResourceSize(Handle theResource, long size)
{
	if (partialResourceCallsAreAvailable == kUninitializedFlag) {
		determineWhetherPRCallsAreAvailable();
	}
	
	if (partialResourceCallsAreAvailable) {
		SetResourceSize(theResource, size);
	} else {
		
		Debugger();
		
	}
}



void determineWhetherPRCallsAreAvailable(void)
{
	if (partialResourceCallsAreAvailable == kUninitializedFlag) {
		OSErr theOSErr;
		long theResponse;
		partialResourceCallsAreAvailable = FALSE;
#define kAppleActuallyImplementedTheResourceMgrAttrSelectorInSystem7_0_x (0)
#if kAppleActuallyImplementedTheResourceMgrAttrSelectorInSystem7_0_x
		theOSErr = Gestalt(gestaltResourceMgrAttr, &theResponse);
		if (theOSErr == noErr) {
			if ( (theResponse & 1L<<gestaltPartialRsrcs) != 0 ) {
				partialResourceCallsAreAvailable = TRUE;
			}
		}
#else
		theOSErr = Gestalt(gestaltSystemVersion, &theResponse);
		if (theOSErr == noErr) {
			if ( (theResponse & 0x0000FFFF) >= 0x0700 ) {
				partialResourceCallsAreAvailable = TRUE;
			}
		}
#endif
	}
}



/******************************/



rsrcMapPtr getHomeRsrcMap(Handle theResource)
{
	rsrcMapHndl cMapHndl;
	short theRefNum;
	
	theRefNum = HomeResFile(theResource);
	cMapHndl = (rsrcMapHndl) TopMapHndl;
	
	while ( (**cMapHndl).fileRefNum != theRefNum ) {
		cMapHndl = (**cMapHndl).nextRsrcMapHndl;
		if (cMapHndl == NULL) return NULL;
	}
	
	return *cMapHndl;
}



long getRsrcRefOffsetIntoFile(Handle theResource)
{
	long offsetToRsrcRefFromRsrcMap;
	long offsetToRsrcMapFromStartOfFile;
	
	rsrcMapPtr theRsrcMapPtr;
	
	ASSERT(theResource != NULL);
	
	offsetToRsrcRefFromRsrcMap = RsrcMapEntry(theResource);
	if (offsetToRsrcRefFromRsrcMap == 0) {
			/* ResError contains the error. */
		return -1;
	}
	
	theRsrcMapPtr = getHomeRsrcMap(theResource);
	offsetToRsrcMapFromStartOfFile = theRsrcMapPtr->copyOfRsrcForkHeader.offsetToRsrcMap;
	
	return offsetToRsrcMapFromStartOfFile + offsetToRsrcRefFromRsrcMap;
}



long getRsrcDataOffsetIntoFile(Handle theResource)
{
	rsrcReference theRsrcRef;
	long theOffsetToRsrcRef;
	long theOffsetToRsrcData;
	long oldMark;
	OSErr theErr;
	long nBytesToRead;
	
	theOffsetToRsrcRef = getRsrcRefOffsetIntoFile(theResource);
	
	oldMark = getRsrcFileMark(theResource);
	setRsrcFileMark(theResource, theOffsetToRsrcRef);
	nBytesToRead = sizeof(rsrcReference);
	theErr = FSRead( HomeResFile(theResource), &nBytesToRead, &theRsrcRef);
	setRsrcFileMark(theResource, oldMark);
	
	if (theErr != noErr || nBytesToRead < sizeof(rsrcReference)) {
		return -1;
	}
	
	theOffsetToRsrcData = theRsrcRef.offsetToRsrcData
		+ 0x0100			// add in the length of the resource header
		+ 4;				// skip over the longword that tells the resource's length
	return theOffsetToRsrcData;
}



long getRsrcFileMark(Handle theResource)
{
	OSErr theErr;
	long theMark;
	ASSERT(theResource != NULL);
	theErr = GetFPos( HomeResFile(theResource), &theMark );
	if (theErr != noErr) theMark = -1;
	return theMark;
}



void setRsrcFileMark(Handle theResource, long newMark)
{
	ASSERT(theResource != NULL);
	/* theOSErr = */ SetFPos( HomeResFile(theResource), fsFromStart, newMark );
}
