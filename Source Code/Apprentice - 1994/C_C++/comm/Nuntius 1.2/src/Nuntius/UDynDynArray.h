// Copyright � 1993 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UDynDynArray.h

#define __UDYNDYNARRAY__

#ifndef __UPTROBJECT__
#include "UPtrObject.h"
#endif

// PDynDynArray is an array, in which the elements can have different size

class CChunkyHandle
{
	private:
		Handle fHandleH;
		long fSize;
		long fAllocSize;
		long fChunk;
		Boolean fHandleIsLocked;
		
	public:
		CChunkyHandle();
		void IChunkyHandle(long chunk);
		~CChunkyHandle();

		void SizeToFit();
		void DeleteAll(); // does not SizeToFit
		void SetNeededSize(long needed);
		void DeltaSize(long delta);
		long GetSize() { return fSize; }
		long GetAllocSize() { return fAllocSize; }
		Ptr PtrAtOffset(long offset) { return  *fHandleH + offset; }
		void MakeGap(long offset, long gaplen);
		void DeleteGap(long offset, long gaplen);
		void AppendChar(char ch);
		
		Boolean LockHandle(Boolean newLock, Boolean moveHigh); // returns old lock state
		void DoRead(TStream *aStream);
		void DoWrite(TStream *aStream);
		long NeededDiskSpace();

		void DebugDump(Boolean verbose);
		Boolean SanityCheck();
};

class PDynDynArray : public PPtrObject
{
	public:
		virtual Boolean LockDataHandle(Boolean newLock, Boolean moveHigh = false); // returns old lock state

		virtual ArrayIndex CreateNewElement(ArrayIndex size);
		virtual void InsertLast(const void *elementPtr, long size);
		virtual void DeleteElementAt(ArrayIndex index);
		virtual void DeleteAll();
		virtual void Specify(PDynDynArray *arrayToCopy);

		virtual ArrayIndex GetSize();
		virtual long GetDataAllocSize();
		virtual void SizeAllocToFit();
		Ptr ComputeAddress(ArrayIndex index); // OBS: Pointer into an unlocked handle!
		virtual void GetElementsAt(ArrayIndex index, void* elementPtr, ArrayIndex count);
		virtual void SetElementSize(ArrayIndex index, long newElementSize);
		virtual long GetElementSize(ArrayIndex index);
		virtual void ReplaceElementsAt(ArrayIndex index,
										  const void *elementPtr, ArrayIndex count);
		virtual void InsertElementBefore(ArrayIndex index);
		virtual void InsertElementBefore(ArrayIndex index, const void *elementPtr, long size);

		virtual void DoRead(TStream *aStream);
		virtual void DoWrite(TStream *aStream);
		virtual long NeededDiskSpace();
		
		virtual void DebugDump(Boolean verbose);
		virtual Boolean SanityCheck();

		PDynDynArray();
		void IDynDynArray(long allocChunk);
		virtual ~PDynDynArray();
	private:
		CChunkyHandle fIndex;
		CChunkyHandle fData;
		
		long GetIndexOffset(ArrayIndex index);
		long GetElementOffset(ArrayIndex index);
		ArrayIndex MySize() { return fIndex.GetSize() >> 3; } // 2 longs for each item
};

class PString255Array : public PDynDynArray
{
	public:
		virtual void Append(const CStr255 &s);
		virtual void GetStringAt(ArrayIndex index, CStr255 &s);
		virtual void ReplaceStringAt(ArrayIndex index, CStr255 &s);
		virtual void InsertStringBefore(ArrayIndex index, CStr255 &s);
		
		PString255Array();
		void IString255Array(long allocChunk);
		~PString255Array();
	private:
		long StringSize(const CStr255 &s);
};

PDynDynArray *NewDynDynArray(long allocChunk);
PString255Array *NewString255Array(long allocChunk = 512);
