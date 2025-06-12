// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UOffsetLengthList.cp

#include "UOffsetLengthList.h"
#include "StreamTools.h"

#pragma segment MyArray

const long kCurrentOLLVersion = 1;
const long kMinOLLVersion = 1;

TOffsetLengthList::TOffsetLengthList()
{
}

pascal void TOffsetLengthList::Initialize()
{
	inherited::Initialize();
}

void TOffsetLengthList::IOffsetLengthList()
{
	inherited::IDynamicArray(0, sizeof(long) + sizeof(long));
}

pascal void TOffsetLengthList::Free()
{
	inherited::Free();
}

void TOffsetLengthList::DoRead(TStream *aStream)
{
	long version = aStream->ReadLong();
	MyStreamCheckVersion(version, kMinOLLVersion, kCurrentOLLVersion, "TOffsetLengthList");
	ReadDynamicArray(aStream, this);
}

void TOffsetLengthList::DoWrite(TStream *aStream)
{
	aStream->WriteLong(kCurrentOLLVersion);
	WriteDynamicArray(aStream, this);
}

void TOffsetLengthList::DoNeedDiskSpace(long &dataForkBytes)
{
	dataForkBytes += sizeof(long); // version
	dataForkBytes += MyStreamSizeOfDynamicArray(this);
}

void TOffsetLengthList::InsertLast(const OffsetLength &ol)
{
	InsertElementsBefore(fSize + 1, &ol, 1);
}

OffsetLength TOffsetLengthList::At(long index)
{
#if qDebug
	if (index < 1 || index > fSize)
	{
		fprintf(stderr, "Index (%ld) out of range [1, %ld]\n", index, 1, fSize);
		ProgramBreak(gEmptyString);
	}
#endif
	return *OffsetLengthPtr(ComputeAddress(index));
}

long TOffsetLengthList::OffsetAt(long index)
{
#if qDebug
	if (index < 1 || index > fSize)
	{
		fprintf(stderr, "Index (%ld) out of range [1, %ld]\n", index, 1, fSize);
		ProgramBreak(gEmptyString);
	}
#endif
	return *LongIntPtr(ComputeAddress(index));
}

long TOffsetLengthList::LengthAt(long index)
{
#if qDebug
	if (index < 1 || index > fSize)
	{
		fprintf(stderr, "Index (%ld) out of range [1, %ld]\n", index, 1, fSize);
		ProgramBreak(gEmptyString);
	}
#endif
	return *LongIntPtr(ComputeAddress(index) + 4);
}

void TOffsetLengthList::AtPut(long index, const OffsetLength &ol)
{
#if qDebug
	if (index < 1 || index > fSize)
	{
		fprintf(stderr, "Index (%ld) out of range [1, %ld]\n", index, 1, fSize);
		ProgramBreak(gEmptyString);
	}
#endif
	*OffsetLengthPtr(ComputeAddress(index)) = ol;
	// somewhat faster than ReplaceElementsAt()
}

void TOffsetLengthList::AtGet(long index, OffsetLength &ol)
{
#if qDebug
	if (index < 1 || index > fSize)
	{
		fprintf(stderr, "Index (%ld) out of range [1, %ld]\n", index, fSize);
		ProgramBreak(gEmptyString); 
	}
#endif
	ol = *OffsetLengthPtr(ComputeAddress(index));
	// somewhat faster than GetElementsAt()
}
