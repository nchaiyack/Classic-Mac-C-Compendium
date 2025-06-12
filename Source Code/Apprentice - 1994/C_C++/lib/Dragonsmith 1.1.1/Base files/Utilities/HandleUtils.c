/*
	HandleUtils.c
	
	Created	28 Mar 1992	NeedHandle only
	
	Modified	14 Apr 1992	Added more functions
			19 Apr 1992	NeedHandle will now resize the handle Ñ duh!
			28 May 1992	Fixed idiocy in HandleToScrap (it was ignoring the type parameter)
			29 Aug 1992	Added a return result to ResizeHandle (what was I thinking?)
						Added CopyHandle
			11 Sep 1992	Removed GrowByAndPoint, NeedHandle, and BigAnyHandle

	Copyright © 1992 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (except as expressly permitted by the author)
		2.	This statement and the above copyright notice are left intact.

*/

#include	"HandleUtils.h"

OSErr CopyHandle (Handle srcHndl, Handle destHndl)
{
	long		srcSize;
	OSErr	err;
	
	if (srcHndl == NULL || destHndl == NULL || *srcHndl == NULL || *destHndl == NULL)
		return nilHandleErr;
	
	srcSize = GetHandleSize (srcHndl);
	err = ResizeHandle (destHndl, srcSize);
	if (err != noErr)
		return err;
		
	BlockMove (*srcHndl, *destHndl, srcSize);
	return noErr;
}

OSErr ResizeHandle (Handle h, long newSize)
{
	char		hState;
	OSErr	err;
	
	hState = HGetState (h);
	HUnlock (h);
	SetHandleSize (h, newSize);
	err = MemError ();
	HSetState (h, hState);
	
	return err;
}

Handle BigHandle (long *actualSize)
{
	// Get a handle to as big a block as you can without getting too fancy
	Handle	h = NULL;
	long		sz;
	
	// We AND sz to make sure it's a positive value; otherwise the >> may wreak havoc
	//	Ñ nobody should be asking for a 2 Gigabyte block, but you never knowÉ
	for (sz = *actualSize & 0x7FFFFFFF; h == NULL && sz > 0; sz = (sz >> 1) + 1) {
		h = NewHandle (sz);
		if (h != NULL)
			*actualSize = sz;
	}
	if (h == NULL) *actualSize = 0L;
	return h;
}

Handle AnyHandle (long size)
{
	Handle	h;
	OSErr	err;
	
	return ((h = NewHandle (size)) ? h : TempNewHandle (size, &err));
}

OSErr HandleToScrap (Handle h, ResType type)
{
	long		err;
	char		hState;
	
	err = ZeroScrap ();
	if (err == 0L) {
		hState = SmartHLock (h);
		err = PutScrap (GetHandleSize (h), type, *h);
		HSetState (h, hState);
		SystemEdit (4);		// Make the system think the user chose "Copy" from the Edit menu
	}
	return (short) err;
}

char SmartHLock (Handle h)
{
	char		hState = HGetState (h);
	
	HLock (h);
	return hState;
}