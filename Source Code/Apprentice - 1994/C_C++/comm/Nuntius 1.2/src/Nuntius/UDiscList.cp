// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UDiscList.cp

#include "UDiscList.h"
#include "UDiscussion.h"
#include "UHashTable.h"
#include "Tools.h"
#include "StreamTools.h"
#include "UThread.h"

#include <ErrorGlobals.h>

#include <Packages.h>

#pragma segment MyGroup

#define qDebugDBUpdate qDebug & 0
#define qDebugDeleteOldStuff qDebug & 0

const Boolean gHeapCheckDiscList = false;

const short kNone = -1;

const long kCurrentDLVersion = 1;
const long kMinDLVersion = 1;

void DebugMsg(const char *p)
{
#if qDebug
	ProgramBreak(p);
#endif
	Failure(errDatabaseScrambled, 0);
	p = p;
}

PDiscList::PDiscList()
	: PStatDynArray()
{
	fHashTable = nil;
}

void PDiscList::IDiscList()
{
	if (gHeapCheckDiscList)
		DebugStr("Entering PDiscList::IDiscList();hc;g");
	IStatDynArray(4 * 1024, 30);
	FailInfo fi;
	if (fi.Try())
	{
		THashTable *ht = new THashTable();
		ht->IHashTable();
		fHashTable = ht;
		fHashTable->FillHashTable(kNone);
		fKillDiscussionTime = 3 * 24 * 60 * 60;
#if qDebugDBUpdate & qDebug
		fprintf(stderr, "*** Testing update of the database of discussions\n");
#endif
		fi.Success();
	}
	else // fail
	{
		delete this;
		fi.ReSignal();
	}
	if (gHeapCheckDiscList)
		DebugStr("Leaving PDiscList::IDiscList();hc;g");
}

PDiscList::~PDiscList()
{
	if (gHeapCheckDiscList)
		DebugStr("Entering PDiscList::Free();hc;g");
	FreeIfObject(fHashTable); fHashTable = nil;
	if (gHeapCheckDiscList)
		DebugStr("Leaving PDiscList::Free();hc;g");
}

void PDiscList::DoRead(TStream *aStream)
{
	if (gHeapCheckDiscList)
		DebugStr("Entering PDiscList::DoRead();hc;g");
	long version = aStream->ReadLong();
	MyStreamCheckVersion(version, kMinDLVersion, kCurrentDLVersion, "PDiscList");
	PStatDynArray::DoRead(aStream);
	fHashTable->DoRead(aStream);
#if qDebug & 0
	if (!PDiscList::SanityCheck())
		Failure(errDatabaseScrambled, 0);
#endif
#if qDebugCDiscussion & 0
	fprintf(stderr, "DiscList dump after read from disc:\n");
	DebugDump(true);
#endif
	if (gHeapCheckDiscList)
		DebugStr("Leaving PDiscList::DoRead();hc;g");
}

void PDiscList::DoWrite(TStream *aStream)
{
	if (gHeapCheckDiscList)
		DebugStr("Entering PDiscList::DoWrite();hc;g");
	aStream->WriteLong(kCurrentDLVersion);
	PStatDynArray::DoWrite(aStream);
	fHashTable->DoWrite(aStream);
	if (gHeapCheckDiscList)
		DebugStr("Leaving PDiscList::DoWrite();hc;g");
}

long PDiscList::NeededDiskSpace()
{
	if (gHeapCheckDiscList)
		DebugStr("Entering PDiscList::DoNeedDiskSpace();hc;g");
	long needed = 
						sizeof(long) + // version
						PStatDynArray::NeededDiskSpace();
	fHashTable->DoNeedDiskSpace(needed);
	if (gHeapCheckDiscList)
		DebugStr("Leaving PDiscList::DoNeedDiskSpace();hc;g");
	return needed;
}

void PDiscList::DeleteAll()
{
	if (gHeapCheckDiscList)
		DebugStr("Entering PDiscList::DeleteAll();hc;g");
	fHashTable->FillHashTable(kNone);
	PStatDynArray::DeleteAll();
	if (gHeapCheckDiscList)
		DebugStr("Leaving PDiscList::DeleteAll();hc;g");
}

ArrayIndex PDiscList::CreateDiscussion(long id, 
										HandleOffsetLength msgIDHol, 
										HandleOffsetLength nameHol)
{
	if (gHeapCheckDiscList)
		DebugStr("Entering PDiscList::CreateDiscussion();hc;g");
	long oldSize = GetSize();
	VOLATILE(oldSize);
	FailInfo fi;
	if (fi.Try())
	{
		const short extraSize = 24 + 16; // articleID's + extra space
		short entrySize = sizeof(CDiscussion) + extraSize + short(msgIDHol.fLength + nameHol.fLength);
		entrySize = (entrySize + 3) & ~3; // round it
		ArrayIndex index = CreateNewElement(entrySize);
#if qDebugCDiscussion
		fprintf(stderr, "Created new entry for CDiscussion: %ld bytes\n", entrySize);
		Boolean prevLock = LockArray(true);
#endif
		CDiscussion *discP = (CDiscussion*) ComputeAddress(index);
		discP->Initialize(short(GetElementSize(index)) - sizeof(CDiscussion), kNone);
		discP->SetLink(kNone);
#if qDebugCDiscussion
		LockArray(prevLock);
#endif
		AddArticleAsOriginator(index, id, msgIDHol, nameHol);
#if qDebugCDiscussion
		SanityCheck();
#endif
		fi.Success();
		return GetSize();
	}
	else // fail
	{
		// fSize = oldSize;
		while (GetSize() > oldSize)
			DeleteElementAt(GetSize());
		fi.ReSignal();
	}
	if (gHeapCheckDiscList)
		DebugStr("Leaving PDiscList::CreateDiscussion();hc;g");
}

//-----------------

void PDiscList::ExpandDiscussion(ArrayIndex discIndex, short spaceNeeded)
{
	if (gHeapCheckDiscList)
		DebugStr("Entering PDiscList::ExpandDiscussion();hc;g");
#if qDebug
//	SanityCheck();
	if (spaceNeeded < 0)
	{
		fprintf(stderr, "***** PDiscList::ExpandDiscussion(), index = %ld, spaceNeeded = %ld\n", discIndex, spaceNeeded);
		ProgramBreak(gEmptyString);
		return;
	}
#endif
	short newSize = short(GetElementSize(discIndex)) + spaceNeeded;
#if qDebugCDiscussion
	fprintf(stderr, "Expanding CDiscussion/PDynDynArray entry %ld -> %ld (� = %ld)\n", GetElementSize(discIndex), long(newSize), long(spaceNeeded));
#endif
	SetElementSize(discIndex, newSize);
#if qDebug
//	SanityCheck();
#endif
	ComputeAddress(discIndex)->UpdateAlloc(newSize - sizeof(CDiscussion));
#if qDebug
//	SanityCheck();
#endif
	if (gHeapCheckDiscList)
		DebugStr("Leaving PDiscList::ExpandDiscussion();hc;g");
}

void PDiscList::AddArticle(ArrayIndex discIndex, long id)
{
	if (gHeapCheckDiscList)
		DebugStr("Entering PDiscList::AddArticle();hc;g");
#if qDebugCDiscussion
	SanityCheck();
#endif
	short spaceNeeded;
	do
	{
#if qDebugCDiscussion
		Boolean prevLock = LockArray(true);
#endif
		CDiscussion *discP = ComputeAddress(discIndex);
		spaceNeeded = discP->AddArticle(id);
#if qDebugCDiscussion
		LockArray(prevLock);
#endif
		if (spaceNeeded) 
			ExpandDiscussion(discIndex, spaceNeeded);
	} 
	while (spaceNeeded);
#if qDebugCDiscussion
	SanityCheck();
#endif
	if (gHeapCheckDiscList)
		DebugStr("Leaving PDiscList::AddArticle();hc;g");
}

void PDiscList::AddArticleAsOriginator(ArrayIndex discIndex, long id, 
								HandleOffsetLength msgIDHol, 
								HandleOffsetLength nameHol)
{
	if (gHeapCheckDiscList)
		DebugStr("Entering PDiscList::AddArticleAsOriginator();hc;g");
	UnchainDiscussion(discIndex);
	short hash = fHashTable->HashMessageID(*msgIDHol.fH + msgIDHol.fOffset, msgIDHol.fLength);
	ArrayIndex link = fHashTable->GetValue(hash);
	short spaceNeeded;
	do
	{
#if qDebugCDiscussion
		Boolean prevLock = LockArray(true);
#endif
		CDiscussion *discP = ComputeAddress(discIndex);
		spaceNeeded = discP->AddArticleAsOriginator(id, msgIDHol, hash, link, nameHol);
#if qDebugCDiscussion
		LockArray(prevLock);
#endif
		if (spaceNeeded) 
			ExpandDiscussion(discIndex, spaceNeeded);
	} 
	while (spaceNeeded);	
	fHashTable->SetValue(hash, discIndex);
#if qDebugCDiscussion
	SanityCheck();
#endif
	if (gHeapCheckDiscList)
		DebugStr("Leaving PDiscList::AddArticleAsOriginator();hc;g");
}

void PDiscList::GetName(ArrayIndex discIndex, CStr255 &name)
{
#if qDebug
	if (discIndex < 1 || discIndex > GetSize())
		ProgramBreak("invalid index");
#endif
	ComputeAddress(discIndex)->GetName(name);
}

unsigned long PDiscList::GetLastActiveDateTime(ArrayIndex discIndex)
{
	return ComputeAddress(discIndex)->GetLastActiveDateTime();
}

//----------------- executing with CDiscussion locked!
TLongintList *PDiscList::GetArticleIDList(ArrayIndex discIndex)
{
	Boolean prevLock = LockArray(true);
	CDiscussion *discP = ComputeAddress(discIndex);
	TLongintList *list = discP->GetArticleIDList();
	LockArray(prevLock);
	return list;
}

//----------------- 
long PDiscList::GetArticleID(ArrayIndex discIndex, long articleIndex)
{
	return ComputeAddress(discIndex)->GetArticleID(articleIndex);
}

short PDiscList::GetNoArticles(ArrayIndex discIndex)
{
	return ComputeAddress(discIndex)->GetNoArticles();
}

//-----------------
CDiscussion *PDiscList::FindDiscussionFromRef(char*& refP, long refLen, 
																							short hash, ArrayIndex &discIndex)
{
	discIndex = fHashTable->GetValue(hash);
	if (discIndex == kNone)
		return nil;
	CDiscussion *discP = ComputeAddress(discIndex);
	while (true) 
	{
		if (discP->CompareMsgID(refP, refLen))
			return discP;
		discIndex = discP->GetLink();
		if (discIndex == kNone)
			return nil;
		discP = ComputeAddress(discIndex);
	}
}

Boolean PDiscList::FindNextReference(char *&refP, long &refLen, short &hash)
// no mem-move allowed
{
	while (*refP != '<')  // quick and dirty parsing
	{ 
		if (*refP == 13)
			return false;
		refP++;
	}
	hash = fHashTable->HashMessageID(refP, refLen);
	return true;
}

Boolean PDiscList::TryFindDiscussion(HandleOffsetLength idHol,
																		ArrayIndex &discIndex)
{
	char *refP = *idHol.fH + idHol.fOffset;
	long refLen;
	while (true) 
	{
		short hash;
		if(!FindNextReference(refP, refLen, hash))
			return false;
		if (FindDiscussionFromRef(refP, refLen, hash, discIndex))
			return true;
		refP += refLen;
	}
}

//-------------------
void PDiscList::UnchainDiscussion(ArrayIndex index)
{
	if (gHeapCheckDiscList)
		DebugStr("Entering PDiscList::UnchainDiscussion();hc;g");
	CDiscussion *theDiscP = ComputeAddress(index);
	short hash = theDiscP->GetHash();
	if (hash == kNone)
	{
		if (gHeapCheckDiscList)
			DebugStr("Leaving PDiscList::UnchainDiscussion();hc;g");
		return;
	}
	ArrayIndex hashValue = fHashTable->GetValue(hash);
	if (hashValue == kNone)
	{
		DebugMsg("HashTable/DiscLink damaged (Hashtable value is kNone)");
		SanityCheck();
		Failure(errDatabaseScrambled, 0); // should be "nice" dialog to the user
		return;
	}
	if (hashValue == index) 
	{
		fHashTable->SetValue(hash, theDiscP->GetLink());
		theDiscP->SetLink(kNone);
		if (gHeapCheckDiscList)
			DebugStr("Leaving PDiscList::UnchainDiscussion();hc;g");
		return;
	}
	CDiscussion *aDiscP = ComputeAddress(hashValue);
	while (true)
	{
		if (aDiscP->GetLink() == kNone) 
		{
			DebugMsg("DiscLink damaged (not found in linked list)"); // should be "nice" dialog to the user //?
			SanityCheck();
			Failure(errDatabaseScrambled, 0);
			return;
		}
		if (aDiscP->GetLink() == index) 
		{
			aDiscP->SetLink(theDiscP->GetLink());
			theDiscP->SetLink(kNone);
			if (gHeapCheckDiscList)
				DebugStr("Leaving PDiscList::UnchainDiscussion();hc;g");
			return;
		}
		aDiscP = ComputeAddress(aDiscP->GetLink());
	}
#if qDebugCDiscussion
	SanityCheck();
#endif
	if (gHeapCheckDiscList)
		DebugStr("Leaving PDiscList::UnchainDiscussion();hc;g");
}

TLongintList *PDiscList::FindDiscussionsToDelete(long firstAvailableArticleID, long /* lastAvailableArticleID */)
{
	if (gHeapCheckDiscList)
		DebugStr("Entering PDiscList::FindDiscussionsToDelete();hc;g");
	FailInfo fi;
	TLongintList *itemsToDelete = nil; // no need for TSortedLongintlist, looping in increasing order
	VOLATILE(itemsToDelete);
	if (fi.Try()) 
	{
		unsigned long dateTime;
		GetDateTime(dateTime);
		TLongintList *list = new TLongintList();
		list->ILongintList();
		itemsToDelete = list;
		itemsToDelete->fAllocationIncrement = 50;
		ArrayIndex size = GetSize();
		for (ArrayIndex index = 1; index <= size; index++) 
		{
			if (EntryIsFree(index))
				continue;
			CDiscussion *discP = ComputeAddress(index);
#if 1
			Boolean deleteIt =  (discP->GetLastArticleID() < firstAvailableArticleID);
#else
			Boolean deleteIt = ((discP->GetArticleID(1) < firstAvailableArticleID) &&
													(dateTime - discP->GetLastActiveDateTime() > fKillDiscussionTime)
												 ) || (discP->GetLastArticleID() < firstAvailableArticleID);
#endif
#if qDebugDBUpdate
			if (Random() > 25000)
				deleteIt = true;
#endif
			if (deleteIt)
				itemsToDelete->InsertLast(index);
			if ((index & 15) == 0)
				gCurThread->CheckYield();
		} // iter
		fi.Success();
		return itemsToDelete;
	}
	else // fail
	{
		FreeIfObject(itemsToDelete); itemsToDelete = nil;
		fi.ReSignal();
	}
	if (gHeapCheckDiscList)
		DebugStr("Leaving PDiscList::FindDiscussionsToDelete();hc;g");
}

void PDiscList::DeleteOldStuff(long firstAvailableArticleID, long lastAvailableArticleID)
{
	if (gHeapCheckDiscList)
		DebugStr("Entering PDiscList::DeleteOldStuff();hc;g");
#if qDebugDeleteOldStuff
	fprintf(stderr, "\n");
	fprintf(stderr, ">>> DeleteOldStuff()\n");
	SanityCheck();
#endif
	TLongintList *itemsToDelete;
	VOLATILE(itemsToDelete);
	FailInfo fi;
	if (fi.Try()) 
	{
		itemsToDelete = FindDiscussionsToDelete(firstAvailableArticleID, lastAvailableArticleID);
		if (itemsToDelete->GetSize() == 0) 
		{
#if qDebugDeleteOldStuff
			fprintf(stderr, "No discussions to be deleted for now.\n");
#endif
			itemsToDelete->Free();
			fi.Success();
			return;
		}
		{
			CLongintIterator iter(itemsToDelete);
			long index;
#if qDebugDeleteOldStuff		
			fprintf(stderr, "Discussions to delete (%ld entries):\n", itemsToDelete->GetSize());
			long zzz = 0;
			for (index = iter.FirstLong(); iter.More(); index = iter.NextLong())
			{
				fprintf(stderr, "%ld, ", index);
				if ((zzz++ % 10) == 0)
					fprintf(stderr, "\n");
			}
			fprintf(stderr, "\nStarting to delete them...\n");
			zzz = 0;
#endif
			long nr = 0;
			for (index = iter.FirstLong(); iter.More(); index = iter.NextLong())
			{
#if qDebugDeleteOldStuff
				fprintf(stderr, "%ld, ", index);
				if ((zzz++ % 10) == 0)
					fprintf(stderr, "\n");
#endif
				UnchainDiscussion(index);
				DeleteElementAt(index);
				if ((nr++ & 15) == 0)
					gCurThread->CheckYield();
			}
			itemsToDelete->Free(); itemsToDelete = nil;
		}

#if qDebugDeleteOldStuff
		fprintf(stderr, "\n");
		SanityCheck();
		fprintf(stderr, "<<< DeleteOldStuff()\n");
		fprintf(stderr, "\n");
#endif
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(itemsToDelete); itemsToDelete = nil;
		fi.ReSignal();
	}
	if (gHeapCheckDiscList)
		DebugStr("Leaving PDiscList::DeleteOldStuff();hc;g");
}	


// ---------------------------
void PDiscList::DebugDump(Boolean verbose)
{
	if (gHeapCheckDiscList)
		DebugStr("Entering PDiscList::DebugDump();hc;g");
	PStatDynArray::DebugDump(verbose || true);
#if qDebug
	if (1)
	{
		Boolean prevLock = LockArray(true);
		CStr255 s, num;
		unsigned long uli;
		GetDateTime(uli);
		NumToString(long(uli) & 0xFFFFF, num);
		s = "HD2:Desktop�folder:disc_";
		s += num;
		fprintf(stderr, "Dumping discussions to %s\n", (char*)s);
		FILE *file = fopen(s, "w");
		if (!file)
			ProgramBreak("Could not creat file to dump discussions into");
		else
		{
			fsetfileinfo(s, 'MPS ', 'TEXT');
			for (ArrayIndex i = 1; i <= GetSize(); i++)
			{
				fprintf(file, "%ld: ", i);
				if (EntryIsFree(i))
					fprintf(file, "is free entry (size: %ld bytes)\n\n", GetElementSize(i));
				else
					ComputeAddress(i)->Dump(file, i);
			}
			fHashTable->DebugDump(file);
			fclose(file);
		}
		LockArray(prevLock);
	}
	long freeSpace = 0;
	for (ArrayIndex i = 1; i <= GetSize(); i++)
	{
		if (!EntryIsFree(i))
			freeSpace += ComputeAddress(i)->GetFreeSpace();
	}
	fprintf(stderr, "Free space in CDiscussions: %ld bytes\n", freeSpace);	
	DumpHashTableUsage();
#endif
	if (gHeapCheckDiscList)
		DebugStr("Leaving PDiscList::DebugDump();hc;g");
}


void PDiscList::DumpHashTableUsage()
{
	fprintf(stderr, "Hash table entry count:\n");
	for (ArrayIndex index = 0; index < kHashTableEntries; index++) 
	{
		if (index % 20 == 0)
			fprintf(stderr, "%4ld: ", index);
		long discIndex = fHashTable->GetValue(short(index));
		long num = 0;
		while (discIndex != kNone)
		{
			num++;
			CDiscussion *discP = ComputeAddress(discIndex);
			discIndex = discP->GetLink();
		}
		if (!num)
			fprintf(stderr, "  -");
		else
			fprintf(stderr, "%3ld", num);
		if ((index + 1) % 20 == 0)
			fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
}

Boolean PDiscList::SanityCheck()
{
	Boolean isGood = PStatDynArray::SanityCheck();
#if qDebug
	if (isGood)
	{
		for (long index = 1; index <= GetSize(); index++)
		{
			if (!ComputeAddress(index)->SanityCheck(index))
				isGood = false;
		}
		if (!SanityCheckLinks())
			isGood = false;
		if (!isGood)
		{
			ProgramBreak("DiscList is BAD!");
			DebugDump(true);
		}
	}
#endif
	return isGood;
}

Boolean PDiscList::SanityCheckLinks()
{
	Boolean isGood = true;
#if qDebug
	Boolean prevLock = LockDataHandle(true, false);
	// this code should check for:
	//   a) all discussions are linked in the proper links
	//   b) all discussions are in the linked lists
	// part a:
	TSortedLongintList *foundDiscs = new TLongintList();
	foundDiscs->ISortedLongintList();
	for (ArrayIndex hash = 0; hash < kHashTableEntries; hash++) 
	{
		long discIndex = fHashTable->GetValue(short(hash));
		if (discIndex == kNone)
			continue;
		if (discIndex < 1 || discIndex > GetSize() || EntryIsFree(discIndex))
		{
			fprintf(stderr, "WRONG:: PDiscList::SanityCheckLinks, got bad value from hashtable: %ld, hash = %ld\n", discIndex, hash);
			isGood = false;
			continue;
		}
		while (true)
		{
			foundDiscs->Insert(discIndex);
			CDiscussion *discP = ComputeAddress(discIndex);
			long discHash = discP->GetHash();
			long nextDisc = discP->GetLink();
			if (discHash != hash)
			{
				fprintf(stderr, "WRONG:: PDiscList::SanityCheckLinks, discussion is in wrong list.\n");
				fprintf(stderr, "-       linkHash = %ld, discHash = %ld, discIndex = %ld\n", hash, discHash, discIndex);
				isGood = false;
			}
			if (nextDisc == kNone)
				break;
			if (nextDisc < 1 || nextDisc > GetSize())
			{
				fprintf(stderr, "WRONG:: PDiscList::SanityCheckLinks, got bad link from disc: %ld, index = %ld, GetSize() = %ld\n", nextDisc, discIndex, GetSize());
				isGood = false;
				break;
			}
			if (EntryIsFree(nextDisc))
			{
				fprintf(stderr, "WRONG:: PDiscList::SanityCheckLinks, got bad link from disc: %ld (which is a free entry!), discIndex = %ld\n", nextDisc, discIndex);
				isGood = false;
				// here we try to check the free empty, otherwise put a break here
			}
			discIndex = nextDisc;
		} // while
	} // hash table loop
	// part b:
	for (ArrayIndex index = 1; index < GetSize(); index++) 
	{
		if (foundDiscs->GetIdentityItemNo(index) != kEmptyIndex)
		{
			if (EntryIsFree(index))
			{
				fprintf(stderr, "WRONG:: PDiscList::SanityCheckLinks, an empty entry (index = %ld) is linked into the list\n", index);
				isGood = false;
			}
		}
		else
		{
			if (!EntryIsFree(index))
			{
				fprintf(stderr, "WRONG:: PDiscList::SanityCheckLinks, a discussion is not included in the linked lists\n");
				fprintf(stderr, "-       discIndex = %ld, here it comes:\n", index);
				isGood = false;
				ComputeAddress(index)->Dump(stderr, index);
			}
		}
	}
	FreeIfObject(foundDiscs); foundDiscs = nil;
	LockDataHandle(prevLock, false);
	if (!isGood)
		DebugDump(true);
#endif
	return isGood;
}

