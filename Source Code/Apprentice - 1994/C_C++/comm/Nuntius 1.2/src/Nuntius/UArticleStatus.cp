// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleStatus.cp

#include "UArticleStatus.h"
#include "Tools.h"
#include "UPrefsDatabase.h"
#include "StreamTools.h"

#include <RsrcGlobals.h>

#pragma segment MyArticle

#define qDebugSpecify qDebug & 0
#define qDebugStatusChanges qDebug & 0

const long kCurrentASVersion = 2;
const long kMinASVersion = 2;

TArticleStatus::TArticleStatus()
{
}

pascal void TArticleStatus::Initialize()
{
	inherited::Initialize();
	fFirstID = 0;
	fChanged = false;
	fDoc = nil;
}

void TArticleStatus::IArticleStatus(TDocument *doc)
{
	inherited::ILongintList();
	fDoc = doc;
}

pascal void TArticleStatus::Free()
{
#if qDebugSpecify
	fprintf(stderr, "Before TArticleStatus::Free():\n");
	DebugDump();
#endif
	inherited::Free();
}

void TArticleStatus::SpecifyWithArticleStatus(TArticleStatus *articleStatus)
{
	long size = articleStatus->GetSize();
	SetArraySize(size);
	fSize = size;
	fFirstID = articleStatus->fFirstID;
	Ptr fromP = articleStatus->ComputeAddress(1);
	Ptr toP = ComputeAddress(1);
	BytesMove(fromP, toP, size * sizeof(long));
	fChanged = articleStatus->fChanged;
}

void TArticleStatus::DoIronAgeFormatRead(TStream *aStream)
{
	aStream->ReadLong(); // size of info
	fFirstID = aStream->ReadLong();
	long size = aStream->ReadLong();

	SetArraySize(size);
	fSize = size;
	if (fSize) 
	{
		Boolean prevLock = Lock(true);
		aStream->ReadBytes(ComputeAddress(1), fSize * sizeof(long));
		Lock(prevLock);
	}
	fChanged = false;
#if qDebugSpecify
	fprintf(stderr, "After TArticleStatus::DoIronAgeFormatRead():\n");
	DebugDump();
#endif
}

void TArticleStatus::DoRead(TStream *aStream)
{
	long version = aStream->ReadLong();
	MyStreamCheckVersion(version, kMinASVersion, kCurrentASVersion, "TArticleStatus");
	fFirstID = aStream->ReadLong();
	ReadDynamicArray(aStream, this);
	fChanged = false;
#if qDebugSpecify
	fprintf(stderr, "After TArticleStatus::DoRead():\n");
	DebugDump();
#endif
}

void TArticleStatus::DoWrite(TStream *aStream)
{
	aStream->WriteLong(kCurrentASVersion);
	aStream->WriteLong(fFirstID);
	WriteDynamicArray(aStream, this);
	fChanged = false;
}

void TArticleStatus::DoNeedDiskSpace(long &dataForkBytes)
{
	dataForkBytes += sizeof(long); // version
	dataForkBytes += sizeof(long); // fFirstID
	dataForkBytes += MyStreamSizeOfDynamicArray(this);
}

void TArticleStatus::IsChanged()
{
	if (fChanged)
		return;
	fChanged = true;
#if qDebug
	if (!fDoc)
		return; // TestCode
#endif
	fDoc->Changed(cArticleStatusChange, this);
#if qDebugStatusChanges
	fprintf(stderr, "TArticleStatus::IsChanged():  Changed() document\n");
#endif
}

ArticleStatus TArticleStatus::GetStatus(long id)
{
	long index = id - fFirstID + 1;
	if (index < 1)
		return kArticleSeen; // I think
	if (index > fSize)
		return kArticleNew;
#if qDebugStatusChanges
	fprintf(stderr, "TArticleStatus::GetStatus(%ld) == %ld\n", id, At(index));
#endif
	return ArticleStatus(At(index));
}

void TArticleStatus::SetStatus(long id, ArticleStatus status)
{
	long index = id - fFirstID + 1;
#if qDebugStatusChanges
	fprintf(stderr, "TArticleStatus::SetStatus(id = %ld, stat = %ld), index = %ld, fSize = %ld\n",
	  id, long(status), index, fSize);
#endif
	if (index < 1 || index > fSize)
	{
#if qDebug
		fprintf(stderr, "TArticleStatus::SetStatus, id = %ld, fFirstID = %ld lastID = %ld\n", id, fFirstID, fFirstID+fSize-1);
#endif
		return;
	}
	if (At(index) == status)
		return;
#if qDebugStatusChanges
	fprintf(stderr, "-   Changed from %ld\n", At(index));
#endif
	AtPut(index, status);
	IsChanged();
}

void TArticleStatus::SetMinStatus(long id, ArticleStatus status)
{
	long index = id - fFirstID + 1;
#if qDebugStatusChanges
	fprintf(stderr, "TArticleStatus::SetMinStatus(id = %ld, stat = %ld), index = %ld, fSize = %ld\n",
	  id, long(status), index, fSize);
#endif
	if (index < 1 || index > fSize)
	{
#if qDebug
		fprintf(stderr, "TArticleStatus::SetMinStatus, id = %ld, fFirstID = %ld lastID = %ld\n", id, fFirstID, fFirstID+fSize-1);
#endif
		return;
	}
	if (At(index) >= status)
		return;
#if qDebugStatusChanges
	fprintf(stderr, "-   Changed from %ld\n", At(index));
#endif
	AtPut(index, status);
	IsChanged();
}

void TArticleStatus::SetNewRange(long newFirstID, long newLastID)
{
	if (newLastID < newFirstID)
	{
		// empty groups, delete everything
		if (fSize)
			IsChanged();
		fFirstID = 0;
		DeleteAll();
		return;
	}
#if qDebug
	if (newLastID > newFirstID + 10000)
		ProgramBreak("newLastID > newFirstID + 10000");
#endif
	long lastID = fFirstID + fSize - 1;
	if (newFirstID == fFirstID && newLastID == lastID)
		return; // no change in range
	long newSize = newLastID - newFirstID + 1;
#if qDebugSpecify
	fprintf(stderr, "TArticleStatus::SetNewRangle, newFirstID = %ld, newLastID = %ld, newSize = %ld\n", newFirstID, newLastID, newSize);
	fprintf(stderr, "-                fFirstID = %ld, fSize = %ld, fLastID = %ld\n", fFirstID, fSize, fFirstID + fSize - 1);
#endif
	IsChanged();
	if (newLastID < fFirstID || newFirstID > lastID)
	{
#if qDebugSpecify
		fprintf(stderr, "-      Deletes all old stuff, initializes whole array with kArticleNew\n");
#endif
		SetArraySize(newSize);
		fSize = newSize;
		long *lP = (long*)ComputeAddress(1);
		for (long i = 1; i <= newSize; i++)
			*lP++ = kArticleNew;
		fFirstID = newFirstID;
		return;
	}
	if (newSize > fSize)
	{
#if qDebugSpecify
		fprintf(stderr, "-    Extending allocated array size to %ld\n", newSize);
#endif
		SetArraySize(newSize);
		fSize = newSize;
	}
	long commonFirstID = Max(fFirstID, newFirstID);
	long commonLastID = Min(lastID, newLastID);
#if qDebugSpecify
	fprintf(stderr, "-    commonFirstID = %ld, commonLastID = %ld\n", commonFirstID, commonLastID);
#endif
	if (newFirstID != fFirstID)
	{
		Ptr fromP = ComputeAddress(commonFirstID - fFirstID + 1);
		Ptr toP = ComputeAddress(commonFirstID - newFirstID + 1);
		long size = commonLastID - commonFirstID + 1;
		MyBlockMove(fromP, toP, size * sizeof(long));
		if (newFirstID < commonFirstID)
		{
			long no = commonFirstID - newFirstID;
			long *lP = (long*)ComputeAddress(1);
			for (long i = 1; i <= no; i++)
				*lP++ = kArticleNew;
		}
#if qDebugSpecify
	fprintf(stderr, "-    moved %ld articles from $%lx to $%lx\n", size, fromP, toP);
#endif
	}
	if (newSize < fSize)
	{
#if qDebugSpecify
		fprintf(stderr, "-    Decreasing allocated array size to %ld\n", newSize);
#endif
		SetArraySize(newSize);
		fSize = newSize;
	}
	fFirstID = newFirstID;
	if (newLastID > commonLastID)
	{
		long no = newLastID - commonLastID;
		long firstNewID = commonLastID + 1;
		long *lP = (long*)ComputeAddress(firstNewID - newFirstID + 1);
		for (long i = 1; i <= no; i++)
			*lP++ = kArticleNew;
#if qDebugSpecify
		fprintf(stderr, "-    added %ld new articles with status = kArticleNew\n", no);
#endif
	}
#if qDebugSpecify
	fprintf(stderr, "After TArticleStatus::SetNewRange():\n");
	DebugDump();
#endif
}

void TArticleStatus::DebugDump()
{
#if qDebug
	fprintf(stderr, "DebugDump of TArticleStatus:\n");
	for (ArrayIndex index = 1; index <= fSize; index++)
	{
		long id = fFirstID + index - 1;
		if (id % 10 == 0 || index == 1)
			fprintf(stderr, "%4ld: ", id);
		long value = At(index);
		fprintf(stderr, "%3ld", value);
		if ((id + 1) % 10 == 0)
			fprintf(stderr, "\n");
	}
	fprintf(stderr, "\nEnd of dump\n");
#endif
}

Boolean TArticleStatus::SanityCheck()
{
	Boolean isGood = true;
#if qDebug
	for (ArrayIndex index = 1; index <= fSize; index++)
	{
		long stat = At(index);
		if (stat < 0 || stat > kArticleRead)
		{
			fprintf(stderr, "Wrong: TArticleStatus::SanityCheck, bad value %ld with index %ld\n", stat, index);
			isGood = false;
		}
	}
	if (!isGood)
	{
		DebugDump();
		ProgramBreak("TArticleStatus was BAD!");
	}
#endif
	return isGood;
}


#if 0
void Test1()
{
#if qDebug
	TArticleStatus *stat = new TArticleStatus();
	stat->IArticleStatus(nil);
	
	fprintf(stderr, "After init:\n");
	stat->SanityCheck();
	stat->DebugDump();
	
	fprintf(stderr, "After NewRange 10-20\n");
	stat->SetNewRange(10, 20);
	stat->SanityCheck();
	stat->DebugDump();
	
	ArrayIndex id;
	for (id = 10; id <= 20; id++)
		stat->SetStatus(id, ArticleStatus(id - 10));
	fprintf(stderr, "After Fill of 10-20\n");
	stat->SanityCheck();
	stat->DebugDump();

	stat->SetNewRange(13, 24);
	fprintf(stderr, "After NewRange 13-24:\n");
	stat->SanityCheck();
	stat->DebugDump();
	
	for (id = 21; id <= 24; id++)
		stat->SetStatus(id, ArticleStatus(id - 10));
	fprintf(stderr, "After fill range 21-25\n");
	stat->SanityCheck();
	stat->DebugDump();

	stat->SetNewRange(17, 29);
	fprintf(stderr, "After new range 17-29\n");
	stat->SanityCheck();
	stat->DebugDump();
	
	for (id = 25; id <= 29; id++)
		stat->SetStatus(id, ArticleStatus(id - 10));
	fprintf(stderr, "After fill range 26-29:\n");
	stat->SanityCheck();
	stat->DebugDump();
	
	stat->Free(); stat = nil;
#endif
}
#endif
