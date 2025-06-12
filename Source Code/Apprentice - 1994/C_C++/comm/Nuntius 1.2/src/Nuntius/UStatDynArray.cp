// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UStatDynArray.cp

#include "UStatDynArray.h"
#include "StreamTools.h"

#pragma segment MyArray

#define qDebugStatDyn qDebug & 0

const long kCurrentStatDynVersion = 1;
const long kMinStatDynVersion = 1;

PStatDynArray::PStatDynArray()
	: PDynDynArray()
{
	fFreeEntriesList = nil;
	fMaxFreeElementDiff = 0;
}

void PStatDynArray::IStatDynArray(long allocChunk, long maxFreeElementDiff)
{
	IDynDynArray(allocChunk);
	FailInfo fi;
	if (fi.Try())
	{
		TSortedLongintList *sl = new TSortedLongintList();
		sl->ISortedLongintList();
		fFreeEntriesList = sl;
		fMaxFreeElementDiff = maxFreeElementDiff;
		fi.Success();
	}
	else // fail
	{
		delete this;
		fi.ReSignal();
	}
}

PStatDynArray::~PStatDynArray()
{
	FreeIfObject(fFreeEntriesList); fFreeEntriesList = nil;
}

void PStatDynArray::DoRead(TStream *aStream)
{
	long version = aStream->ReadLong();
	MyStreamCheckVersion(version, kMinStatDynVersion, kCurrentStatDynVersion, "PStatDynArray");
	PDynDynArray::DoRead(aStream);
	ReadDynamicArray(aStream, fFreeEntriesList);
#if qDebug
	PStatDynArray::SanityCheck();
#endif
}

void PStatDynArray::DoWrite(TStream *aStream)
{
	aStream->WriteLong(kCurrentStatDynVersion);
	PDynDynArray::DoWrite(aStream);
	WriteDynamicArray(aStream, fFreeEntriesList);
}

long PStatDynArray::NeededDiskSpace()
{
	return	sizeof(long) + // version number
					PDynDynArray::NeededDiskSpace() +
					MyStreamSizeOfDynamicArray(fFreeEntriesList);
}

void PStatDynArray::DeleteAll()
{
	fFreeEntriesList->DeleteAll();
	PDynDynArray::DeleteAll();
}

void PStatDynArray::DeleteElementAt(ArrayIndex index)
{
#if qDebugStatDyn
	fprintf(stderr, "PStatDynArray::DeleteElementAt(), index = %ld, fSize = %ld\n", index, fSize);
#endif
#if qDebug
	if (fFreeEntriesList->GetIdentityItemNo(index) != kEmptyIndex)
	{
		fprintf(stderr, "PStatDynArray::DeleteElementAt, index = %ld, element is already deleted!\n", index);
		ProgramBreak(gEmptyString);
		return;
	}
	if (index <= 0 || index > GetSize())
	{
		fprintf(stderr, "PStatDynArray::DeleteElementAt(), index = %ld: Bad index [1, %ld]\n", index, GetSize());
		ProgramBreak(gEmptyString);
	}
#endif
	if (index != GetSize())
	{
		fFreeEntriesList->Insert(index);
		return;
	}
	PDynDynArray::DeleteElementAt(index);
	while (true)
	{
		index--;
		if (!index)
			break;
		if (fFreeEntriesList->Last() != index)
			break;
		fFreeEntriesList->Delete(index);
		PDynDynArray::DeleteElementAt(index);
	}
}

ArrayIndex PStatDynArray::CreateNewElement(ArrayIndex size)
{
	// if no in list of free entries, create a new
	if (fFreeEntriesList->GetSize() == 0)
	{
#if qDebugStatDyn
	fprintf(stderr, "PStatDynArray::CreateNewElement(), size = %ld, creates new element\n", size);
#endif
		return PDynDynArray::CreateNewElement(size);
	}
	// seek for entry with at least same size
	CLongintIterator iter(fFreeEntriesList);
	long index;
	for (index = iter.FirstLong(); iter.More(); index = iter.NextLong())
	{
		long elmSize = GetElementSize(index);
		if (elmSize >= size && size - elmSize <= fMaxFreeElementDiff)
		{
			fFreeEntriesList->Delete(index);
#if qDebugStatDyn
	fprintf(stderr, "PStatDynArray::CreateNewElement(), size = %ld, found one at %ld with size = %ld\n", size, index, GetElementSize(index));
#endif
			return index;
		}
	}
	// we did not find a free entry that meets our specifications, so create a new entry
	return PDynDynArray::CreateNewElement(size);


/*	// don't make existing entries bigger as the are never downsized
	// make the first entry the right size
	index = fFreeEntriesList->First();
#if qDebugStatDyn
	fprintf(stderr, "PStatDynArray::CreateNewElement(), size = %ld, resized one at %ld with old size = %ld\n", size, index, GetElementSize(index));
#endif
	SetElementSize(index, size);
	return index;
*/
}

Boolean PStatDynArray::EntryIsFree(ArrayIndex index)
{
	return fFreeEntriesList->GetIdentityItemNo(index) != kEmptyIndex;
}

void PStatDynArray::DebugDump(Boolean verbose)
{
	PDynDynArray::DebugDump(verbose && false);
#if qDebug
	
	long noFreeEntries = fFreeEntriesList->GetSize();
	
	if (!noFreeEntries)
	{
		fprintf(stderr, "No free entries in list.\n");
		return;
	}
	fprintf(stderr, "List of free items:\n");
	long freeSpace = 0;
	CLongintIterator iter(fFreeEntriesList);
	long index;
	for (index = iter.FirstLong(); iter.More(); index = iter.NextLong())
	{
		long size = GetElementSize(index);
		fprintf(stderr, "%6ld:%6ld bytes\n", index, size);
		freeSpace += size;
	}
	long dataSize = GetDataAllocSize();
	fprintf(stderr, "Total:%6ld bytes free in%4ld entries\n", freeSpace, noFreeEntries);
	fprintf(stderr, "Total:%6ld bytes used in%4ld entries\n", dataSize - freeSpace, GetSize() - noFreeEntries);
	if (dataSize > 0)
	{
		long perc = long((100.0 * freeSpace) / dataSize);
		fprintf(stderr, "Total:%6ld%% is free space in array\n", perc);
	}
#endif
}

Boolean PStatDynArray::SanityCheck()
{
	Boolean isGood = PDynDynArray::SanityCheck();
#if qDebug
	if (fFreeEntriesList->GetSize() > 0)
	{
		long prev = fFreeEntriesList->At(1);
		long maxIndex = fFreeEntriesList->GetSize();
		for (long index = 2; index <= maxIndex; index++)
		{
			long curr = fFreeEntriesList->At(index);
			if (curr <= prev || curr <= 0 || curr > GetSize())
			{
				fprintf(stderr, "WRONG: PStatDynArray::SanityCheck, deleted element: %ld (prev = %ld, freeList.fSize = %ld)\n", curr, prev, maxIndex);
				isGood = false;
			}
		}
	}
	if (!isGood)
		DebugDump(true);
#endif
	return isGood;
}
