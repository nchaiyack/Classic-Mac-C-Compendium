// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UHeaderList.cp

#include "UHeaderList.h"
#include "UOffsetLengthList.h"
#include "Tools.h"
#include "StreamTools.h"

#ifndef __XTYPES__
#include "XTypes.h"
#endif

#pragma segment MyGroup

#define qDebugNewHeaders qDebug & 0
#define qDebugNewHeadersVerbose qDebugNewHeaders & 0

const long kCurrentHeaderListVersion = 1;
const long kMinHeaderListVersion = 1;

THeaderList::THeaderList()
{
}

pascal void THeaderList::Initialize()
{
	inherited::Initialize();
	fDataH = nil;
	fIndexList = nil;
	fFirstID = fLastID = 0;
}

void THeaderList::IHeaderList()
{
	inherited::IObject();
	FailInfo fi;
	if (fi.Try())
	{
		fDataH = NewPermHandle(0);
		TOffsetLengthList *list = new TOffsetLengthList();
		list->IOffsetLengthList();
		fIndexList = list;
		fi.Success();
	}
	else // fail
	{
		Free();
		fi.ReSignal();
	}
}

pascal void THeaderList::Free()
{
	FreeIfObject(fIndexList); fIndexList = nil;
	fDataH = DisposeIfHandle(fDataH);
	fFirstID = fLastID = 0;
	inherited::Free();
}

void THeaderList::DoRead(TStream *aStream)
{
	long version = aStream->ReadLong();
	MyStreamCheckVersion(version, kMinHeaderListVersion, kCurrentHeaderListVersion, "THeaderList");
	MyStreamReadHandle(aStream, fDataH);
	fIndexList->DoRead(aStream);
	fFirstID = aStream->ReadLong();
	fLastID = fFirstID + fIndexList->GetSize() - 1;
#if qDebug
	THeaderList::SanityCheck();
#endif
}

void THeaderList::DoWrite(TStream *aStream)
{
	aStream->WriteLong(kCurrentHeaderListVersion);
	MyStreamWriteHandle(aStream, fDataH);
	fIndexList->DoWrite(aStream);
	aStream->WriteLong(fFirstID);
}

void THeaderList::DoNeedDiskSpace(long &dataForkBytes)
{
	dataForkBytes += sizeof(long); // version
	dataForkBytes += MyStreamSizeOfHandle(fDataH);
	fIndexList->DoNeedDiskSpace(dataForkBytes);
	dataForkBytes += sizeof(long); // fFirstID
}

void THeaderList::DeleteAll()
{
	fFirstID = fLastID = 0;
	fIndexList->DeleteAll();
	SetHandleSize(fDataH, 0);
}

void THeaderList::AtGet(long id, HandleOffsetLength &hol)
{
#if qDebug
	if (id < fFirstID || id > fLastID)
	{
		fprintf(stderr, "THeaderList::AtGet, id = %ld, fFirstID = %ld, fLastID = %ld\n", id, fFirstID, fLastID);
		ProgramBreak(gEmptyString);
	}
#endif
	OffsetLength ol;
	fIndexList->AtGet(id - fFirstID + 1, ol);
	hol.fH = fDataH;
	hol.fOffset = ol.fOffset;
	hol.fLength = ol.fLength;
}

void THeaderList::AddNewHeaders(Handle h, 
																long groupFirstID, 
																long newFirstID, long newLastID)
// The parsing in this proc assumes that the text in h
// is terminated with a null
{
#if qDebug
	SanityCheck();
#endif
#if qDebugNewHeadersVerbose
	DebugDump(true);
#endif
	if (fIndexList->GetSize() == 0 || newFirstID != fLastID + 1) 
	{
	// if we havn't got anything, or the stuff we have is too old
	// then reset the array
#if qDebugNewHeaders
		fprintf(stderr, "THL::ANH, deleting everything\n");
		fprintf(stderr, "(old: %ld -> %ld, new: %ld -> %ld)\n", fFirstID, fLastID, newFirstID, newLastID);
#endif
		fFirstID = newFirstID;
		fLastID = newLastID;
		fIndexList->DeleteAll();
		SetPermHandleSize(fDataH, 0);
	}
// if we have got some headers that's become too old, then delete them
	if (fIndexList->GetSize() > 0 && fFirstID < groupFirstID) 
	{
		long noToDel = groupFirstID - fFirstID;
		long offset;
		long oldDataSize = GetHandleSize(fDataH);
		if (fIndexList->GetSize() == noToDel) 
		{
			offset = oldDataSize;
		}
		else
		{
			offset = fIndexList->OffsetAt(noToDel + 1);
			BytesMove(*fDataH + offset, *fDataH, oldDataSize - offset); // moving down
			long i, maxi = fIndexList->GetSize();
			long *lP = (long*) fIndexList->ComputeAddress(noToDel + 1);
			for (i = noToDel + 1; i <= maxi; i++) // adjust offsets
			{
				*lP -= offset;
				lP += 2;
			}
		}
#if qDebugNewHeaders
		fprintf(stderr, "THL::ANH, updating (old: %ld -> %ld, new: %ld -> %ld)\n", fFirstID, fLastID, newFirstID, newLastID);
		fprintf(stderr, "-         noToDel = %ld, offset = %ld, oldDataSize = %ld\n", noToDel, offset, oldDataSize);
#endif
		fFirstID += noToDel;
		fIndexList->DeleteElementsAt(1, noToDel);
		SetPermHandleSize(fDataH, oldDataSize - offset);
	}
#if qDebug
	SanityCheck();
#endif
	fLastID = newLastID;
// add new entries, and set them to zero
	long noIDs = fLastID - fFirstID + 1;
	fIndexList->SetArraySize(noIDs);
	fIndexList->fSize = noIDs;
	long oldSize = GetHandleSize(fDataH);
	long newSize = oldSize + GetHandleSize(h);
#if qDebugNewHeaders
	fprintf(stderr, "-  oldSize = %ld, newSize = %ld, noIDs = %ld\n", oldSize, newSize, noIDs);
#endif
	SetPermHandleSize(fDataH, newSize);
	BytesMove(*h, *fDataH + oldSize, newSize - oldSize);

// no mem-move from now!
#if qDebug
	HLockHi(fDataH);
#endif
	register Ptr p = *fDataH + oldSize;
	register long id;
	long lastID = newFirstID - 1;
	register char ch;
#define qGetTime 0
#if qGetTime
	long startTick = TickCount();
#endif
	OffsetLength ol;
	while (*p)
	{
#if qDebug
		Ptr lineStart = p;
#endif
		id = 0;
		ch = *p;
		while (ch > 32) // get id (decimal)
		{
			id *= 10;
			id += ch - '0';
			ch = *++p;
		}
		p++; // step past space
		ol.fOffset = p - *fDataH;
		while (*p != 13)
			++p; // find text end
		if (id == 0) // ignore, 0 means line without number (eg. foldet line)
		{
#if qDebugNewHeaders
			fprintf(stderr, "THeaderList::ANH, article with id = 0 ignored\n");
#endif
		}
		else if (id > lastID && id <= newLastID) 
		{
			while (++lastID < id)
			{
#if qDebugNewHeadersVerbose
				fprintf(stderr, "THeaderList::ANH, missed article with id = %ld, put at %ld\n", lastID, lastID - fFirstID + 1);
#endif
				OffsetLength emptyOL;
				emptyOL.fOffset = ol.fOffset;
				emptyOL.fLength = 0;
				fIndexList->AtPut(lastID - fFirstID + 1, emptyOL);
			}
			ol.fLength = p - *fDataH - ol.fOffset;
			if (ol.fLength == 6 && strncmp(p - 6, "(none)", 6) == 0) 
			{
				// quick and dirty parsing: kill "(none)" comment
				// speciality by nntp daemon (vers 1.5.11) XHDR command
				ol.fOffset += 6;
				ol.fLength -= 6;
			}
#if qDebugNewHeadersVerbose
			fprintf(stderr, "THeaderList::ANH, article with id = %ld ", id);
			fprintf(stderr, "inserted at %ld: ", id - fFirstID + 1);
			fprintf(stderr, "ol = %ld, %ld, text = ", ol.fOffset, ol.fLength);
			fwrite(*fDataH + ol.fOffset, 1, int(MinMax(0, ol.fLength, 200)), stderr);
			fprintf(stderr, "\n");
#endif
			fIndexList->AtPut(id - fFirstID + 1, ol); // could be optimized
		}
		else
		{
#if qDebug
			fprintf(stderr, "article id %ld out of range, newFirstID = %ld newLastID = %ld, lastID = %ld\n", id, newFirstID, newLastID, lastID);
			fprintf(stderr, "Line: >");
			fwrite(lineStart, 1, p - lineStart, stderr);
			fprintf(stderr, "<\n");
#endif
		}
		ch = *p;
		while (ch == 13 || ch == 10) 
			ch = *++p; // step past CR and/or LF
	}
#if qDebugNewHeaders
	fprintf(stderr, "THeaderList::ANH, after loop: lastID = %ld\n", lastID);
#endif
	while (++lastID <= newLastID)
	{
#if qDebugNewHeadersVerbose
		fprintf(stderr, "THeaderList::ANH, missed article with id = %ld, atput %ld\n", lastID, lastID - fFirstID + 1);
#endif
		OffsetLength emptyOL;
		emptyOL.fOffset = GetHandleSize(fDataH);
		emptyOL.fLength = 0;
		fIndexList->AtPut(lastID - fFirstID + 1, emptyOL);
	}
#if qGetTime
	long ticks = TickCount() - startTick;
	char ss[1000];
	sprintf(ss, "Index ticks: %ld", ticks);
	DebugStr(CStr255(ss));
#endif
#if qDebug
	SanityCheck();
	HUnlock(fDataH);
#endif
}

void THeaderList::DebugDump(Boolean verbose)
{
#if qDebug
	verbose = verbose;
	HLock(fDataH);
	fprintf(stderr, "DebugDump of THeaderList at $%lx (%ld headers):\n", long(this), fIndexList->GetSize());
	long size = GetHandleSize(fDataH);
	for (short index = 1; index <= fIndexList->GetSize(); index++)
	{
		fprintf(stderr, "%8ld: ", index);
		OffsetLength ol;
		fIndexList->AtGet(index, ol);
		if (ol.fLength < 0 || ol.fLength > 1000)
		{
			fprintf(stderr, "(bad length: %ld, %ld)\n", ol.fOffset, ol.fLength);
			continue;
		}
		if (ol.fOffset < 0 || ol.fOffset + ol.fLength > size)
		{
			fprintf(stderr, "(bad offset: %ld, %ld)\n", ol.fOffset, ol.fLength);
			continue;
		}
		if (!ol.fLength)
			fprintf(stderr, "<ÉnoneÉ>");
		else
			fwrite(*fDataH + ol.fOffset, 1, int(MinMax(0, ol.fLength, 80)), stderr);
		fprintf(stderr, " (%ld, %ld)\n", ol.fOffset, ol.fLength);
	}
	HUnlock(fDataH);
#else
	verbose = verbose;
#endif
}

Boolean THeaderList::SanityCheck()
{
	Boolean isGood = true;
#if qDebug
	if (fIndexList->GetSize() > 0 && fLastID - fFirstID + 1 != fIndexList->GetSize())
	{
		long shouldBe = fIndexList->GetSize() + fFirstID - 1;
		fprintf(stderr, "WRONG: THeaderList::SanityCheck, fLastID is wrong (%ld), should be %ld\n", fLastID, shouldBe);
		fprintf(stderr, "-                                ListSize = %ld, fFirstID = %ld\n", fIndexList->GetSize(), fFirstID);
		isGood = false;
	}
	long size = GetHandleSize(fDataH);
	for (short index = 1; index <= fIndexList->GetSize(); index++)
	{
		OffsetLength ol;
		fIndexList->AtGet(index, ol);
/*
		if (ol.fLength == 0) // compability with old files
		{
			if (index == 1)
				ol.fOffset = 0;
			else
			{
				OffsetLength prevOL;
				fIndexList->AtGet(index - 1, prevOL);
				ol.fOffset = prevOL.fOffset + prevOL.fLength;
			}
			fIndexList->AtPut(index, ol);
		}					
*/			
		if (ol.fLength < 0 || ol.fLength > 1000)
		{
			fprintf(stderr, "WRONG: THeaderList::SanityCheck: bad header length %ld for index %ld\n", ol.fLength, index);
			isGood = false;
		}
		if (ol.fOffset < 0 || ol.fOffset + ol.fLength > size)
		{
			fprintf(stderr, "WRONG: THeaderList::SanityCheck: bad header offset %ld for index %ld\n", ol.fOffset, index);
			isGood = false;
		}
	}
	if (!isGood)
	{
		DebugDump(true);
		ProgramBreak("THeaderList was bad");
	}
#endif
	return isGood;
}

//------------------------------------------------
THeaderList *NewHeaderList()
{
	THeaderList *hl = new THeaderList();
	hl->IHeaderList();
	return hl;
}
