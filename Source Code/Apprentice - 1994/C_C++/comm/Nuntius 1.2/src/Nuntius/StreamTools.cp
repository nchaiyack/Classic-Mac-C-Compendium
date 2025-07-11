// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// StreamTools.cp

#include "StreamTools.h"
#include <ErrorGlobals.h>

#pragma segment MyTools

//----------------------------------------------------------------
#if qDebug
void MyStreamCheckVersion(long theVersion, long minVersion, long maxVersion, const char *debugName)
#else
void MyStreamCheckVersion(long theVersion, long minVersion, long maxVersion, const char *)
#endif
{
	if (theVersion < minVersion || theVersion > maxVersion)
	{
#if qDebug
		fprintf(stderr, "**** Incompatible %s disk version = %ld, min = %ld, cur = %ld. Aborts read of file.\n", debugName, theVersion, minVersion, maxVersion);
#endif
		if (theVersion < minVersion)
			FailOSErr(errTooOldFileFormat);
		else
			FailOSErr(errTooNewFileFormat);
	}
#if qDebug
	if (theVersion < maxVersion)
		fprintf(stderr, "Reads old version %s: version = %ld, min = %ld, cur = %ld.\n", debugName, theVersion, minVersion, maxVersion);
#endif
}

#if qDebug
Boolean MyCheckVersion(long theVersion, long minVersion, long maxVersion, const char *debugName)
#else
Boolean MyCheckVersion(long theVersion, long minVersion, long maxVersion, const char *)
#endif
{
	if (theVersion < minVersion)
	{
#if qDebug
		fprintf(stderr, "**** Incompatible %s disk version = %ld, min = %ld, cur = %ld. Doesn't read file - but no abort.\n", debugName, theVersion, minVersion, maxVersion);
#endif
		return false;
	}
	if (theVersion > maxVersion)
	{
#if qDebug
		fprintf(stderr, "**** Incompatible %s disk version = %ld, min = %ld, cur = %ld. Aborts read of file.\n", debugName, theVersion, minVersion, maxVersion);
#endif
		FailOSErr(errTooNewFileFormat);
	}
#if qDebug
	if (theVersion < maxVersion)
		fprintf(stderr, "Reads old version %s: version = %ld, min = %ld, cur = %ld.\n", debugName, theVersion, minVersion, maxVersion);
#endif
}

//----------------------------------------------------------------
const long kCurrentHandleVersion = 1;
const long kMinHandleVersion = 1;

void MyStreamReadHandle(TStream *aStream, Handle h)
{
	long version = aStream->ReadLong();
	MyStreamCheckVersion(version, kMinHandleVersion, kCurrentHandleVersion, "Handle");
	long handleSize = aStream->ReadLong();
	SetPermHandleSize(h, handleSize);
	if (handleSize)
	{
		HLock(h);
		aStream->ReadBytes(*h, handleSize);
		HUnlock(h);
		if (handleSize & 3) 
		{
			long pad = 0;
			aStream->ReadBytes(&pad, 4 - (handleSize & 3));
		}
	}
}

void MyStreamWriteHandle(TStream *aStream, Handle h)
{
	aStream->WriteLong(kCurrentHandleVersion);
	long handleSize = GetHandleSize(h);
	aStream->WriteLong(handleSize);
	if (handleSize)
	{
		HLock(h);
		aStream->WriteBytes(*h, handleSize);
		HUnlock(h);
		if (handleSize & 3) 
		{
			long pad = 0;
			aStream->WriteBytes(&pad, 4 - (handleSize & 3));
		}
	}
}

long MyStreamSizeOfHandle(Handle h)
{
	return sizeof(long) + sizeof(long) + GetHandleSize(h) + 3 & ~3;
}

void DoIronAgeFormatReadHandle(TStream *aStream, Handle h)
{
	long handleSize = aStream->ReadLong();
	SetPermHandleSize(h, handleSize);
	if (handleSize)
	{
		HLock(h);
		aStream->ReadBytes(*h, handleSize);
		HUnlock(h);
		if (handleSize & 3) 
		{
			long pad = 0;
			aStream->ReadBytes(&pad, 4 - (handleSize & 3));
		}
	}
}


//----------------------------------------------------------------
const long kCurrentArrayVersion = 1;
const long kMinArrayVersion = 1;

void ReadDynamicArray(TStream *aStream, TDynamicArray *array)
{
	long version = aStream->ReadLong();
	MyStreamCheckVersion(version, kMinArrayVersion, kCurrentArrayVersion, "TDynamicArray");
	long size = aStream->ReadLong();
	array->SetArraySize(size);
	array->fSize = size;
	if (size)
	{
		Boolean prevLock = array->Lock(true);
		aStream->ReadBytes(array->ComputeAddress(1), array->fSize * array->fElementSize);
		array->Lock(prevLock);
	}
}

void WriteDynamicArray(TStream *aStream, TDynamicArray *array)
{
	aStream->WriteLong(kCurrentArrayVersion);
	aStream->WriteLong(array->fSize);
	if (array->fSize) 
	{
		Boolean prevLock = array->Lock(true);
		aStream->WriteBytes(array->ComputeAddress(1), array->fSize * array->fElementSize);
		array->Lock(prevLock);
	}
}

long MyStreamSizeOfDynamicArray(TDynamicArray *array)
{
	return 
		sizeof(long) + // version number
		sizeof(long) + // no elements
		array->fSize * array->fElementSize; // actual data
}
//----------------------------------------------------------------
