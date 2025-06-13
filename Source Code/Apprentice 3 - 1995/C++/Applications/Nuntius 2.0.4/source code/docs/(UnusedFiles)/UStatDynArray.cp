// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UStatDynArray.cp

#include "UStatDynArray.h"
#include "StreamTools.h"

#include <stdio.h>

#pragma segment MyArray

#define qDebugStatDyn qDebug & 0

const long kCurrentStatDynVersion = 1;
const long kMinStatDynVersion = 1;

#undef Inherited
#define Inherited TObject
DefineClass(TStatDynArray, TObject);

TStatDynArray::TStatDynArray()
{
	fFreeEntriesList = nil;
	fMaxFreeElementDiff = 0;
}

void TStatDynArray::IStatDynArray(long allocChunk, long maxFreeElementDiff)
{
	IDynDynArray(allocChunk);
	FailInfo fi;
	Try(fi)
	{
		TSortedLongintList *sl = new TSortedLongintList();
		sl->ISortedLongintList();
		fFreeEntriesList = sl;
		fMaxFreeElementDiff = maxFreeElementDiff;
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(this);
		fi.ReSignal();
	}
}

void TStatDynArray::Free()
{
	FreeIfObject(fFreeEntriesList); fFreeEntriesList = nil;
	Inherited::Free();
}

void TStatDynArray::DoRead(TStream *aStream)
{
	long version = aStream->ReadLong();
	MyStreamCheckVersion(version, kMinStatDynVersion, kCurrentStatDynVersion, "TStatDynArray");
	TDynDynArray::DoRead(aStream);
	ReadDynamicArray(aStream, fFreeEntriesList);
#if qDebug
	TStatDynArray::SanityCheck();
#endif
}

void TStatDynArray::DoWrite(TStream *aStream)
{
	aStream->WriteLong(kCurrentStatDynVersion);
	TDynDynArray::DoWrite(aStream);
	WriteDynamicArray(aStream, fFreeEntriesList);
}

long TStatDynArray::NeededDiskSpace()
{
	return	sizeof(long) + // version number
					TDynDynArray::NeededDiskSpace() +
					MyStreamSizeOfDynamicArray(fFreeEntriesList);
}

void TStatDynArray::DeleteAll()
{
	fFreeEntriesList->DeleteAll();
	TDynDynArray::DeleteAll();
}

void TStatDynArray::DeleteElementAt(ArrayIndex index)
{
#if qDebugStatDyn
	fprintf(stderr, "TStatDynArray::DeleteElementAt(), index = %ld, fSize = %ld\n", index, fSize);
#endif
#if qDebug
	if (fFreeEntriesList->GetIdentityItemNo(index) != kEmptyIndex)
	{
		fprintf(stderr, "TStatDynArray::DeleteElementAt, index = %ld, element is already deleted!\n", index);
		ProgramBreak(gEmptyString);
		return;
	}
	if (index <= 0 || index > GetSize())
	{
		fprintf(stderr, "TStatDynArray::DeleteElementAt(), index = %ld: Bad index [1, %ld]\n", index, GetSize());
		ProgramBreak(gEmptyString);
	}
#endif
	if (index != GetSize())
	{
		fFreeEntriesList->Insert(index);
		return;
	}
	TDynDynArray::DeleteElementAt(index);
	while (true)
	{
		index--;
		if (!index)
			break;
		if (fFreeEntriesList->Last() != index)
			break;
		fFreeEntriesList->Delete(index);
		TDynDynArray::DeleteElementAt(index);
	}
}

ArrayIndex TStatDynArray::CreateNewElement(ArrayIndex size)
{
	// if no in list of free entries, create a new
	if (fFreeEntriesList->GetSize() == 0)
	{
#if qDebugStatDyn
	fprintf(stderr, "TStatDynArray::CreateNewElement(), size = %ld, creates new element\n", size);
#endif
		return TDynDynArray::CreateNewElement(size);
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
	fprintf(stderr, "TStatDynArray::CreateNewElement(), size = %ld, found one at %ld with size = %ld\n", size, index, GetElementSize(index));
#endif
			return index;
		}
	}
	// we did not find a free entry that meets our specifications, so create a new entry
	return TDynDynArray::CreateNewElement(size);


/*	// don't make existing entries bigger as the are never downsized
	// make the first entry the right size
	index = fFreeEntriesList->First();
#if qDebugStatDyn
	fprintf(stderr, "TStatDynArray::CreateNewElement(), size = %ld, resized one at %ld with old size = %ld\n", size, index, GetElementSize(index));
#endif
	SetElementSize(index, size);
	return index;
*/
}

Boolean TStatDynArray::EntryIsFree(ArrayIndex index)
{
	return fFreeEntriesList->GetIdentityItemNo(index) != kEmptyIndex;
}

void TStatDynArray::DebugDump(Boolean verbose)
{
	TDynDynArray::DebugDump(verbose && false);
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

Boolean TStatDynArray::SanityCheck()
{
	Boolean isGood = TDynDynArray::SanityCheck();
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
				fprintf(stderr, "WRONG: TStatDynArray::SanityCheck, deleted element: %ld (prev = %ld, freeList.fSize = %ld)\n", curr, prev, maxIndex);
				isGood = false;
			}
		}
	}
	if (!isGood)
		DebugDump(true);
#endif
	return isGood;
}
