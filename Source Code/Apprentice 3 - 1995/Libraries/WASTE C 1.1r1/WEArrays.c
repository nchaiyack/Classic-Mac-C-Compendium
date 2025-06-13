/*
 *	Arrays.c
 *
 *	WASTE PROJECT
 *	Utilities for handling handle-based dynamic arrays
 *
 *	Copyright (c) 1993-1994 Marco Piovanelli
 *	All Rights Reserved
 *
 */

#include <Types.h>
#include <Errors.h>
#include <Memory.h>
#include <ToolUtils.h>
#include "WASTEIntf.h"

/*
 *	NOTE: Replace BlockMove with BlockMoveData as soon as the universal headers
 *	become available (BlockMove flushes the processor cache too often for my taste)
 */

pascal OSErr _WEInsertSlot(Handle h, Ptr element, long insertAt, long slotSize)
{
	long oldSize, offset;
	OSErr err;

/*	get handle size */

	oldSize = GetHandleSize(h);

/*	lengthen handle by one "slot" */

	SetHandleSize(h, oldSize + slotSize);
	if ((err = MemError()) != noErr)
		return err;

/*	calculate insertion offset */

	offset = insertAt * slotSize;

/*	make sure offset is within allowed bounds */

	if ((offset < 0) || (offset > oldSize))
		return paramErr;

/*	make room for new element */

	BlockMoveData( *h + offset, *h + offset + slotSize, oldSize - offset );

/*	insert new element */

	BlockMoveData( element, *h + offset, slotSize );

	return noErr;
}

pascal OSErr _WERemoveSlot(Handle h, long removeAt, long slotSize)
{
	long newSize, offset;
	OSErr err;

/*	get handle size minus a "slot" */

	newSize = GetHandleSize(h) - slotSize;

/*	calculate removal offset */

	offset = removeAt * slotSize;

/*	make sure offset is within allowed bounds */
	
	if ((offset < 0) || (offset > newSize))
		return paramErr;

/*	compact the array */
	
	BlockMoveData( *h + offset + slotSize, *h + offset, newSize - offset );

/*	shorten the handle */

	SetHandleSize(h, newSize);
	if ((err = MemError()) != noErr)
		return err;

	return noErr;
}