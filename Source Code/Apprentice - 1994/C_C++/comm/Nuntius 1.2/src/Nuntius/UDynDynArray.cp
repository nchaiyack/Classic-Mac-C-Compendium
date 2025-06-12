// Copyright © 1993 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UDynDynArray.cp

#include "UDynDynArray.h"
#include "Tools.h"
#include "StreamTools.h"

#pragma segment MyArray

#define qDebugDynDyn qDebug & 0
#define qDebugDynDynSpecial qDebug & 0
#define qDebugDynDynIntense qDebug & 0
#define qDebugStream qDebug & 0

#define qDebugLogHandleResize qDebug & 0

const long kCurrentDynDynVersion = 1;
const long kMinDynDynVersion = 1;

const long kIndexEntrySize = 2 * sizeof(long);
//========================================================================

CChunkyHandle::CChunkyHandle()
{
	fHandleH = nil;
	fSize = 0;
	fAllocSize = 0;
	fHandleIsLocked = false;
	fChunk = 4;
}

void CChunkyHandle::IChunkyHandle(long chunk)
{
	if (qDebug && chunk <= 0)
		ProgramBreak("Bad chunk");
	fChunk = chunk;
	fHandleH = NewPermHandle(0);
}

CChunkyHandle::~CChunkyHandle()
{
	fHandleH = DisposeIfHandle(fHandleH);
}

void CChunkyHandle::SizeToFit()
{
	if (fAllocSize == fSize)
		return;
#if qDebugLogHandleResize
	fprintf(stderr, "ChunckSizeToFit: %ld -> %ld\n", fAllocSize, fSize);
#endif
	SetHandleSize(fHandleH, fSize);
	if (qDebug && MemError() != noErr) 
		ProgramBreak("MemError() after DownSizing");
	fAllocSize = fSize;
}

void CChunkyHandle::SetNeededSize(long needed)
{
	if (qDebug && needed < 0)
		ProgramBreak("needed < 0");
	if (needed <= fAllocSize && fAllocSize - needed <= 2 * fChunk)
	{
		fSize = needed;
		if (qDebugDynDynIntense)
			SanityCheck();
		return;
	}
	long newAllocSize = (needed + fChunk) - (needed + fChunk) % fChunk;
	if (newAllocSize == 0)
	{
		newAllocSize = fChunk;
		// else we'll do SetSize(0)/SetSize(1024) when adding/deleting
		// the first element from the list
	}
	if (qDebugDynDynIntense && newAllocSize < needed)
		ProgramBreak("newAllocSize < needed");
#if qDebugLogHandleResize
	fprintf(stderr, "Chunckresize: %ld -> %ld\n", fAllocSize, newAllocSize);
#endif
	if (newAllocSize > fAllocSize)
		SetPermHandleSize(fHandleH, newAllocSize);
	else
	{
		SetHandleSize(fHandleH, newAllocSize);
		if (qDebug && MemError() != noErr) 
			ProgramBreak("MemError() after DownSizing");
	}
	fAllocSize = newAllocSize;
	fSize = needed;
	if (qDebugDynDynIntense)
		SanityCheck();
}

void CChunkyHandle::DeltaSize(long delta)
{
	SetNeededSize(fSize + delta);
}

void CChunkyHandle::DeleteAll()
{
	SetNeededSize(0);
}

void CChunkyHandle::MakeGap(long offset, long gaplen)
{
#if qDebug
	if (offset < 0 || gaplen < 0 || offset > fSize)
		ProgramBreak("Bad gap");
#endif
	long oldSize = fSize;
	long movebytes = oldSize - offset;
	SetNeededSize(fSize + gaplen);
	if (movebytes)
	{
		Ptr fromP = PtrAtOffset(offset);
		Ptr toP = fromP + gaplen;
		if (qDebug && offset + gaplen + movebytes > fSize)
			ProgramBreak("Bad calc");
		MyBlockMove(fromP, toP, movebytes);
	}
#if qDebug
	BlockSet(PtrAtOffset(offset), gaplen, 0xF1);
#endif
}

void CChunkyHandle::DeleteGap(long offset, long gaplen)
{
#if qDebug
	if (offset < 0 || gaplen < 0 || offset + gaplen > fSize)
		ProgramBreak("Bad gap");
#endif
	long oldSize = fSize;
	long movebytes = oldSize - offset - gaplen;
	if (movebytes)
	{
		Ptr toP = PtrAtOffset(offset);
		Ptr fromP = toP + gaplen;
		if (qDebug && offset + gaplen + movebytes > fSize)
			ProgramBreak("Bad calc");
		MyBlockMove(fromP, toP, movebytes);
	}
	SetNeededSize(fSize - gaplen);
}

void CChunkyHandle::AppendChar(char ch)
{
	SetNeededSize(fSize + 1);
	*(*fHandleH + fSize - 1) = ch;
}

Boolean CChunkyHandle::LockHandle(Boolean newLock, Boolean moveHigh)
{
	if (newLock == fHandleIsLocked) 
		return newLock;
#if qDebugDynDyn
	fprintf(stderr, "CChunkyHandle::LockHandle() at $%lx, ", long(this));
	fprintf(stderr, "%slocked -> ", fHandleIsLocked ? "" : "un");
	fprintf(stderr, "%slocked", newLock ? "" : "un");
	fprintf(stderr, "%s\n", moveHigh ? ", moveHigh" : "");
#endif
	fHandleIsLocked = newLock;
	if (newLock)
		if (moveHigh)
			HLockHi(fHandleH);
		else
			HLock(fHandleH);
	else
		HUnlock(fHandleH);
}

void CChunkyHandle::DoRead(TStream *aStream)
{
	LockHandle(false, false);
	MyStreamReadHandle(aStream, fHandleH);
	fSize = GetHandleSize(fHandleH);
	fAllocSize = fSize;
}

void CChunkyHandle::DoWrite(TStream *aStream)
{
	SizeToFit();
#if qDebugStream
	long oldSize = aStream->GetPosition();
#endif	
	MyStreamWriteHandle(aStream, fHandleH);
#if qDebugStream
	long size = aStream->GetPosition() - oldSize;
	fprintf(stderr, "Chunck at $%lx: stream write: %ld -> %ld = %ld\n", long(this), oldSize, aStream->GetPosition(), size);
#endif
}

long CChunkyHandle::NeededDiskSpace()
{
	// Break the implementation of MyStreamSizeOfHandle as it requires me to resize the handle
	// and this will cause every bit of speed to disappear as this function is called each time
	// an element is saved
	long size = (fSize + 3) & ~3;
	size += sizeof(long) + sizeof(long);
//	long size = MyStreamSizeOfHandle(fHandleH);
#if qDebugStream
	fprintf(stderr, "Chunck at $%lx: stream get size = %ld\n", long(this), size);
#endif
	return size;
}

void CChunkyHandle::DebugDump(Boolean /* verbose */)
{
#if qDebug
	fprintf(stderr, "  CChunkyHandle: fSize = %ld, fAllocSize = %ld, fChunk = %ld, lock = %ld, ghs = %ld\n", fSize, fAllocSize, fChunk, fHandleIsLocked, (fHandleH?GetHandleSize(fHandleH):-1));
#endif
}

Boolean CChunkyHandle::SanityCheck()
{
#if qDebug
	if (!fHandleH)
	{
		fprintf(stderr, "CChunkyHandle::SanityCheck, Missing handle\n");
		return false;
	}
	if (!VerboseIsHandle(fHandleH))
		ProgramBreak("Ups, real bad handle");
	if (fSize < 0 || fAllocSize < 0 || fSize > fAllocSize)
	{
		fprintf(stderr, "CChunkyHandle::SanityCheck, bad sizes: fSize  = %ld, fAllocSize = %ld\n", fSize, fAllocSize);
		return false;
	}
	long ghs = GetHandleSize(fHandleH);
	if (ghs != fAllocSize)
	{
		fprintf(stderr, "CChunkyHandle::SanityCheck, GetHandleSize() = %ld  !=  fAllocSize == %ld\n", ghs, fAllocSize);
		return false;
	}
	if (fHandleIsLocked)
		fprintf(stderr, "CChunkyHandle::SanityCheck, WARNING: handle is locked\n");
#endif
	return true;
}

//========================================================================

PDynDynArray::PDynDynArray()
	: fIndex(), fData()
{
#if qDebugDynDyn
	fprintf(stderr, "PDynDynArray::PDynDynArray() at $%lx\n", long(this));
#endif
	if (qDebug) FailNonPtrObject(this);
}

void PDynDynArray::IDynDynArray(long allocChunk)
{
	if (qDebug) FailNonPtrObject(this);
	FailInfo fi;
	if (fi.Try())
	{
		fIndex.IChunkyHandle(128 * 4);
		fData.IChunkyHandle(allocChunk);
#if qDebugDynDyn
		fprintf(stderr, "PDynDynArray::IDynDynArray() at $%lx\n", long(this));
#endif
		fi.Success();
	}
	else // fail
	{
		delete this;
		fi.ReSignal();
	}
}

PDynDynArray::~PDynDynArray()
{
	if (qDebug) FailNonPtrObject(this);
#if qDebugDynDyn
	fprintf(stderr, "PDynDynArray::~PDynDynArray() at $%lx, had %ld items\n", long(this), MySize());
#endif
}

void PDynDynArray::DoRead(TStream *aStream)
{
	if (qDebug) FailNonPtrObject(this);
	long version = aStream->ReadLong();
	MyStreamCheckVersion(version, kMinDynDynVersion, kCurrentDynDynVersion, "PDynDynArray");
	fData.DoRead(aStream);
	fIndex.DoRead(aStream);
#if qDebugDynDyn
	fprintf(stderr, "PDynDynArray::DoRead() at $%lx, read %ld items\n", long(this), MySize());
#endif
#if qDebug | qDebugDynDynIntense
	PDynDynArray::SanityCheck();
#endif
}

void PDynDynArray::DoWrite(TStream *aStream)
{
	if (qDebug) FailNonPtrObject(this);
#if qDebugDynDynIntense
	PDynDynArray::SanityCheck();
#endif
	aStream->WriteLong(kCurrentDynDynVersion);
	fData.DoWrite(aStream);
	fIndex.DoWrite(aStream);
#if qDebugDynDyn
	fprintf(stderr, "PDynDynArray::DoWrite() at $%lx, wrote %ld items\n", long(this), MySize());
#endif
}

long PDynDynArray::NeededDiskSpace()
{
	if (qDebug) FailNonPtrObject(this);
	return	sizeof(long) +									// version number
					fData.NeededDiskSpace() +
					fIndex.NeededDiskSpace();
}

void PDynDynArray::DeleteAll()
{
	if (qDebug) FailNonPtrObject(this);
#if qDebugDynDynIntense
	PDynDynArray::SanityCheck();
#endif
#if qDebugDynDyn
	fprintf(stderr, "PDynDynArray::DeleteAll() at $%lx, deleted %ld items\n", long(this), MySize());
#endif
	fIndex.DeleteAll();
	fData.DeleteAll();
#if qDebugDynDynIntense
	PDynDynArray::SanityCheck();
#endif
}

ArrayIndex PDynDynArray::CreateNewElement(long size)
{
	if (qDebug) FailNonPtrObject(this);
#if qDebugDynDynIntense
	PDynDynArray::SanityCheck();
#endif
	if (qDebug && size < 0)
		ProgramBreak("size < 0");
// should I unlock fDataH if it's locked? I do that now.
	if (fData.LockHandle(false, false))
		if (qDebug) ProgramBreak("Data handle was locked ???");
#if qDebugDynDynSpecial
	fprintf(stderr, "PDynDynArray::CreateNewElement() at $%lx\n", long(this));
	fprintf(stderr, "-   no elem: %ld, new element size = %ld, ", MySize(), size);
#endif
	FailInfo fi;
	if (fi.Try()) 
	{
		long oldDataLen = fData.GetSize();
		fData.DeltaSize(size);
		fIndex.DeltaSize(kIndexEntrySize);
		long indexOffset = GetIndexOffset(MySize());
		long *indexEntry = (long*)fIndex.PtrAtOffset(indexOffset);
		*indexEntry++ = oldDataLen;
		*indexEntry = size;
#if qDebugDynDynIntense
		PDynDynArray::SanityCheck();
#endif
		fi.Success();
		return MySize();
	}
	else 
	{
		// no recovery is needed as the last failable statement is the 
		// resizing of fIndex which holds the size of the array
		fi.ReSignal();
	}
}

Boolean PDynDynArray::LockDataHandle(Boolean newLock, Boolean moveHigh)
{
	if (qDebug) FailNonPtrObject(this);
	return fData.LockHandle(newLock, moveHigh);
}

void PDynDynArray::DeleteElementAt(ArrayIndex index)
{
	if (qDebug) FailNonPtrObject(this);
#if qDebugDynDynIntense
	PDynDynArray::SanityCheck();
#endif
	if (index < 1 || index > MySize())
	{
#if qDebug
		fprintf(stderr, "index = %ld out of range: 1 - %ld", index, MySize());
		ProgramBreak(gEmptyString);
#endif
		return;
	}
	FailInfo fi;
	if (fi.Try()) 
	{
		long dataOffset = GetElementOffset(index);
		long dataLen = GetElementSize(index);
		long indexOffset = GetIndexOffset(index);
		long oldSize = MySize();
#if qDebugDynDynSpecial
		fprintf(stderr, "PDynDynArray::DeleteElementAt() at $%lx, index = %ld\n", long(this), index);
		fprintf(stderr, "-  element size: %ld, no array elements: %ld\n", dataLen, MySize());
#endif
		// kill the index entry
		fIndex.DeleteGap(indexOffset, kIndexEntrySize);
		if (index < oldSize)  // if not last element, update the offsets for the rest of the elements
		{
			long *iP = (long*)fIndex.PtrAtOffset(indexOffset);
			for (ArrayIndex i = index; i < oldSize; i++) // we have delete it now, so i < oldSize
			{
				*iP++ -= dataLen; // adjust offset
				++iP; // jump past length
			}
		}
		// kill the data
		fData.DeleteGap(dataOffset, dataLen);
#if qDebugDynDynSpecial
		fprintf(stderr, "After DeleteElementAt:\n");
		DebugDump(false);
#endif
#if qDebugDynDynIntense
		PDynDynArray::SanityCheck();
#endif
		fi.Success();
	}
	else // fail
	{
		DebugStr("This should never happend: failure in PDynDynArray::DeleteElementAt()");
		fi.ReSignal();
	}
}

void PDynDynArray::SetElementSize(ArrayIndex index, long newElementSize)
{
	if (qDebug) FailNonPtrObject(this);
#if qDebugDynDynIntense
	PDynDynArray::SanityCheck();
#endif
	long oldElementSize = GetElementSize(index);
	if (newElementSize == oldElementSize)
		return; // no change in size
#if qDebugDynDynSpecial
	fprintf(stderr, "PDynDynArray::SetElementSize() at $%lx, index = %ld, size %ld -> %ld\n", long(this), index, oldElementSize, newElementSize);
#endif
	if (index < 1 || index > MySize())
	{
#if qDebug
		fprintf(stderr, "index = %ld out of range: 1 - %ld", index, MySize());
		ProgramBreak(gEmptyString);
#endif
		return;
	}
#if qDebug
	if (newElementSize < 0 || newElementSize > 10 * 1024 * 1024)
	{
		fprintf(stderr, "Crazy element size: %ld\n", newElementSize);
		ProgramBreak(gEmptyString);
		return;
	}
#endif
	FailInfo fi;
	if (fi.Try()) 
	{
		long dataOffset = GetElementOffset(index);
		long deltaElementSize = newElementSize - oldElementSize;
		long oldDataLen = fData.GetSize();
		long newDataLen = oldDataLen + deltaElementSize;
#if qDebugDynDynSpecial
		fprintf(stderr, "-  ÆelmSize: %ld, oldDataLen = %ld, newDataLen = %ld\n", deltaElementSize, oldDataLen, newDataLen);
#endif
		if (deltaElementSize > 0)
			fData.MakeGap(dataOffset + oldElementSize, deltaElementSize);
		else
			fData.DeleteGap(dataOffset + newElementSize, -deltaElementSize);
		// adjust rest of offsets
		long indexOffset = GetIndexOffset(index);
		long *lP = (long*)fIndex.PtrAtOffset(indexOffset);
		lP++; // offset
		*lP++ = newElementSize; // new length
		if (index < MySize()) // if not last element, then adjust rest of offsets
		{
			for (ArrayIndex i = index + 1; i <= MySize(); i++)
			{
				*lP++ += deltaElementSize; // adjust offset
				lP++; // no change in length
			}
		}
#if qDebugDynDynSpecial
		fprintf(stderr, "-  adjusted %ld index entries (offset += %ld bytes)\n", MySize() - index + 1, deltaElementSize);
#endif
#if qDebugDynDynSpecial
		fprintf(stderr, "After SetElementSize:\n");
		DebugDump(false);
#endif
#if qDebugDynDynIntense
		PDynDynArray::SanityCheck();
#endif
		fi.Success();
	}
	else // fail
	{
		fi.ReSignal();
	}
}

void PDynDynArray::InsertElementBefore(ArrayIndex index) // buggy, I think
{
	if (qDebug) FailNonPtrObject(this);
#if qDebugDynDynSpecial
	fprintf(stderr, "PDynDynArray::InsertElementBefore() at $%lx, index = %ld, MySize() = %ld\n", long(this), index, MySize());
#endif
#if qDebugDynDynIntense
	PDynDynArray::SanityCheck();
#endif
	if (index > MySize())
	{
#if qDebugDynDynSpecial
		fprintf(stderr, "-  appending new item to list\n");
#endif
		CreateNewElement(0);
		return;
	}
	if (qDebug && index < 1)
		ProgramBreak("-  index < 1 *****");
	long dataOffset = GetElementOffset(index);
	long indexOffset = GetIndexOffset(index);
	fIndex.MakeGap(indexOffset, kIndexEntrySize);
	long *lP = (long*) fIndex.PtrAtOffset(indexOffset);
	*lP++ = dataOffset;
	*lP = 0;
#if qDebugDynDynIntense
	PDynDynArray::SanityCheck();
#endif
}

void PDynDynArray::InsertElementBefore(ArrayIndex index, const void *elementPtr, long size)
{
	if (qDebug) FailNonPtrObject(this);
	VOLATILE(index);
	InsertElementBefore(index);
	FailInfo fi;
	if (fi.Try())
	{
		SetElementSize(index, size);
		BytesMove(elementPtr, ComputeAddress(index), size);
		fi.Success();
	}
	else // fail
	{
		// remove the empty element
		DeleteElementAt(index);
		fi.ReSignal();
	}
}

void PDynDynArray::InsertLast(const void *elementPtr, long size)
{
	if (qDebug) FailNonPtrObject(this);
	ArrayIndex index = CreateNewElement(size);
	Ptr p = ComputeAddress(index);
	BytesMove(elementPtr, p, size);
}

ArrayIndex PDynDynArray::GetSize()
{
	if (qDebug) FailNonPtrObject(this);
	return MySize();
}

long PDynDynArray::GetDataAllocSize()
{
	return fData.GetAllocSize();
}

void PDynDynArray::SizeAllocToFit()
{
	fData.SizeToFit();
	fIndex.SizeToFit();
}

void PDynDynArray::GetElementsAt(ArrayIndex index, void* elementPtr, ArrayIndex count)
{
	if (qDebug) FailNonPtrObject(this);
#if qDebug
	if (index < 1 || index + count - 1 > MySize())
	{
		fprintf(stderr, "PDynDynArray::GetElementsAt, index = %ld, count = %ld, MySize() = %ld\n", index, count, MySize());
		ProgramBreak(gEmptyString);
	}
#endif
	long size = 0;
	for (ArrayIndex i = 0; i < count; i++)
		size += GetElementSize(index + i);
#if qDebugDynDynSpecial
	fprintf(stderr, "PDynDynArray::GetElementsAt() at $%lx, ", long(this));
	fprintf(stderr, "index = %ld, count = %ld, to $%lx\n", index, count, elementPtr);
	fprintf(stderr, "-  size of the elements: %ld bytes\n", size);
#endif
	BytesMove(ComputeAddress(index), elementPtr, size);
}

void PDynDynArray::ReplaceElementsAt(ArrayIndex index, const void *elementPtr, ArrayIndex count)
{
	if (qDebug) FailNonPtrObject(this);
#if qDebugDynDynIntense
	PDynDynArray::SanityCheck();
#endif
#if qDebug
	if (index < 1 || index + count - 1 > MySize())
	{
		fprintf(stderr, "PDynDynArray::ReplaceElementsAt, index = %ld, count = %ld, MySize() = %ld\n", index, count, MySize());
		ProgramBreak(gEmptyString);
	}
#endif
	long size = 0;
	for (ArrayIndex i = 0; i < count; i++)
		size += GetElementSize(index + i);
#if qDebugDynDynSpecial
	fprintf(stderr, "PDynDynArray::ReplaceElementsAt() at $%lx, ", long(this));
	fprintf(stderr, "index = %ld, count = %ld, from $%lx\n", index, count, elementPtr);
	fprintf(stderr, "-  size of the elements: %ld bytes\n", size);
#endif
	BytesMove(elementPtr, ComputeAddress(index), size);
#if qDebugDynDynIntense
	PDynDynArray::SanityCheck();
#endif
}

void PDynDynArray::Specify(PDynDynArray *arrayToCopy)
{
if (qDebug) ProgramBreak("Totally untested");
	if (qDebug) FailNonPtrObject(this);
	if (qDebug) FailNonPtrObject(arrayToCopy);
	FailInfo fi;
	if (fi.Try())
	{
		LockDataHandle(false);
		DeleteAll();
		long dataSize = arrayToCopy->fData.GetSize();
		long indexSize = arrayToCopy->fIndex.GetSize();
		fData.SetNeededSize(dataSize);
		fIndex.SetNeededSize(indexSize);
		BlockMove(arrayToCopy->fData.PtrAtOffset(0), fData.PtrAtOffset(0), dataSize);
		BlockMove(arrayToCopy->fIndex.PtrAtOffset(0), fIndex.PtrAtOffset(0), indexSize);
		fi.Success();
	}
	else // fail
	{
		DeleteAll();
		fi.ReSignal();
	}
}

long PDynDynArray::GetIndexOffset(ArrayIndex index)
{
	return (index - 1) * kIndexEntrySize;
}

long PDynDynArray::GetElementOffset(ArrayIndex index)
{
	return *( (long*)fIndex.PtrAtOffset(GetIndexOffset(index)) );
}

long PDynDynArray::GetElementSize(ArrayIndex index)
{
	if (qDebug) FailNonPtrObject(this);
	return *(long*)fIndex.PtrAtOffset(GetIndexOffset(index) + sizeof(long));
}

Ptr PDynDynArray::ComputeAddress(ArrayIndex index) // OBS: CPointer into an unlocked handle!
{
#if qDebug
	FailNonPtrObject(this);
	if (index < 1 || index > MySize()) 
	{
		fprintf(stderr, "Invalid index = %ld (%ld-%ld)\n", index, 1, MySize());
		ProgramBreak(gEmptyString);
		return nil;
	}
#endif
	Ptr p = fIndex.PtrAtOffset((index - 1) * kIndexEntrySize);
	return fData.PtrAtOffset(*(long*)p);
}

void PDynDynArray::DebugDump(Boolean verbose)
{
	if (qDebug) FailNonPtrObject(this);
#if qDebug
	fprintf(stderr, "DebugDump af PDynDynArray\n");
	fprintf(stderr, "Index: ");
	fIndex.DebugDump(verbose);
	fprintf(stderr, "Data:  ");
	fData.DebugDump(verbose);
	for (ArrayIndex index = 1; index <= MySize(); index++)
	{
		long offset = GetElementOffset(index);
		if (offset < 0 || offset > fData.GetSize())
		{
			fprintf(stderr, "%3ld seems too bad to dump (offset = %ld)\n", index, offset);
			continue;
		}
		long size = GetElementSize(index);
		fprintf(stderr, "%3ld: offset =%6ld, length = %5ld, ",
			index, offset, size);
		if (verbose)
		{
			fprintf(stderr, "data = ");
			Ptr p = ComputeAddress(index);
			long elNr = *p - 'A' + 1;
			long *lP = (long*) p;
			if (size >= 4)
				fprintf(stderr, "$%8lx", *lP);
			if (size >= 8)
				fprintf(stderr, "%8lx", *(lP+1));
			fprintf(stderr, " = ");
			for (long i = MinMax(0, size, 8); i; i--)
				putc(*p++, stderr);
			if (elNr >= 1 && elNr <= 50)
				fprintf(stderr, " = #%2ld", elNr);
		}
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "End of dump\n");
#else
	verbose = verbose;
#endif
}

Boolean PDynDynArray::SanityCheck()
{
	if (qDebug) FailNonPtrObject(this);
	Boolean isGood = true;
#if qDebug
	long checkOffset = 0;
	for (ArrayIndex index = 1; index <= MySize(); index++)
	{
		long offset = GetElementOffset(index);
		long size = GetElementSize(index);
		if (offset != checkOffset)
		{
			fprintf(stderr, "WRONG: PDynDynArray::SanityCheck, bad element-offset, index = %ld, offset = %ld, expected offset = %ld\n", index, offset, checkOffset);
			isGood = false;
			break;
		}
		checkOffset = offset + size;
	}
	if (checkOffset != fData.GetSize())
	{
		fprintf(stderr, "WRONG: PDynDynArray::SanityCheck, used size of datahandle  = %ld, fData.GetSize() = %ld\n", checkOffset, fData.GetSize());
		isGood = false;
	}
	if (fIndex.GetSize() & 3)
	{
		fprintf(stderr, "WRONG: PDynDynArray::SanityCheck, fIndex.GetSize() = %ld   (long-even odd)\n", fIndex.GetSize());
		isGood = false;
	}
	if (!isGood)
		PDynDynArray::DebugDump(true);
#endif
	return isGood;
}
//==========================================================================

PString255Array::PString255Array()
{
}

void PString255Array::IString255Array(long allocChunk = 512)
{
	IDynDynArray(allocChunk);
}

PString255Array::~PString255Array()
{
}

inline long PString255Array::StringSize(const CStr255 &s)
{
	return (s.Length() + 4) & ~3;
}

void PString255Array::Append(const CStr255 &s)
{
	const unsigned char *p = (const unsigned char*)&s;
	InsertLast(p, StringSize(s));
}

void PString255Array::GetStringAt(ArrayIndex index, CStr255 &s)
{
	Ptr p = ComputeAddress(index);
	BytesMove(p, &s, 1 + *(unsigned char*)p);
}

void PString255Array::ReplaceStringAt(ArrayIndex index, CStr255 &s)
{
	SetElementSize(index, StringSize(s));
	ReplaceElementsAt(index, &s, 1);
}

void PString255Array::InsertStringBefore(ArrayIndex index, CStr255 &s)
{
	InsertElementBefore(index, &s, StringSize(s));
}
//==========================================================================
PDynDynArray *NewDynDynArray(long allocChunk)
{
	PDynDynArray *dda = new PDynDynArray();
	dda->IDynDynArray(allocChunk);
	return dda;
}

PString255Array *NewString255Array(long allocChunk)
{
	PString255Array *dda = new PString255Array();
	dda->IString255Array(allocChunk);
	return dda;
}
