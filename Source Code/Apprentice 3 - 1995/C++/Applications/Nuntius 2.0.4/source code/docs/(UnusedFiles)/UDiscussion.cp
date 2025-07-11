// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UDiscussion.cp

#include "UDiscussion.h"
#include "Tools.h"

#include <OSUtils.h>
#include <stdio.h>

#pragma segment MyGroup

#define qIntenseDebugDisc 0
#define qDebugExpandTableEntry qDebug & 0

/*

The format of the entries in TDiscBase's list is as follows:

	+==================+
	| CDiscussion      |
	+------------------+
	| msg-ID           |
	+------------------+
	| name             |
	+------------------+
	| article id's     |
	+==================+

CDiscussion manages all this!
*/

void CDiscussion::Setup(short initialTableSize, short hash)
{
	fLink = 0xCFC;
	GetDateTime(&fCreateDateTime);
	fLastActiveDateTime = fCreateDateTime;
	fHash = hash;
	fNoArticles = 0;
	fTableBytesAlloc = initialTableSize;
	fTable[kMsgID].fOffset = sizeof(CDiscussion);
	fTable[kMsgID].fLength = 0;
	fTable[kMsgID].fUsedLength = 0;
	fTable[kName].fOffset = sizeof(CDiscussion);
	fTable[kName].fLength = 0;
	fTable[kName].fUsedLength = 0;
	fTable[kIDTable].fOffset = sizeof(CDiscussion);
	fTable[kIDTable].fLength = 0;
	fTable[kIDTable].fUsedLength = 0;
#if qDebug
	fTable[kMsgID].fFiller = 0;
	fTable[kName].fFiller = 0;
	fTable[kIDTable].fFiller = 0;
#endif
#if qDebugCDiscussion
	fprintf(stderr, "CDiscussion initialized with %hd bytes tablespace\n", initialTableSize);
#endif
#if qIntenseDebugDisc
	fprintf(stderr, "After CDiscussion::Initialize():\n");
	Dump(stderr);
#endif
#if qDebug
	SanityCheck(-1);
#endif
}

inline short MakeEven(short x)
{
	return (x + 1) & ~1;
}

short CDiscussion::ExpandTableEntry(TableType table, short newUsedSize)
{
#if qDebug
#if qDebugExpandTableEntry
	SanityCheck(-1);
#endif
	if (newUsedSize < 0 || newUsedSize > 300)
	{
		fprintf(stderr, "Bad newUsedSize = %ld\n", newUsedSize);
		ProgramBreak(gEmptyString);
	}
#endif
	short newSize = MakeEven(newUsedSize);
	if (newSize <= fTable[table].fLength)
	{
		fTable[table].fUsedLength = newUsedSize;
		return 0;
	}
#if qDebugCDiscussion
	if (fTable[table].fUsedLength)
		fprintf(stderr, "CDiscussion table needs to grow from %hd to %hd bytes\n", fTable[table].fUsedLength, newUsedSize);
#endif
	short curSizeOfTables = fTable[kMsgID].fLength + fTable[kName].fLength + fTable[kIDTable].fLength;
	short deltaSize = newSize - fTable[table].fLength;
/*
char sss[200];
sprintf(sss, "fTableBytesAlloc = %hd, curSize = %hd, delta = %hd", fTableBytesAlloc, curSizeOfTables, deltaSize);
DebugStr(sss);
*/
	if (deltaSize <= 0)
	{
#if qDebug
		ProgramBreak("deltaSize <= 0 (already checked for that!)");
#endif
		return 0; // don't ever want to compact!
	}
	if (curSizeOfTables + deltaSize > fTableBytesAlloc)
	{
		short needed = curSizeOfTables + deltaSize - fTableBytesAlloc;
#if qIntenseDebugDisc
		fprintf(stderr, "DID NOT have enough space for expansion, needed %hd bytes more (have %ld, tablesize: %ld)\n", needed, fTableBytesAlloc, curSizeOfTables);
		if (needed < 0)
		{
			ProgramBreak("********* Needed < 0");
			return 0;
		}
#endif
		return needed;
	}
	if (table < kLastTable)
	{
// DebugStr("Moving tables to give space for this one");
		short fromOffset = fTable[table + 1].fOffset;
		short toOffset = fromOffset + deltaSize;
		short moveSize = 0;
		for (TableType index = TableType(table + 1); index <= kLastTable; index = TableType(index + 1))
		{
			moveSize += fTable[index].fLength;
			fTable[index].fOffset += deltaSize;
		}
		Ptr p = Ptr(this);
		MyBlockMove(p + fromOffset, p + toOffset, moveSize);
#if qDebug
		fTable[table].fLength = newSize; // must too here, otherwise SanityCheck says shit
		SanityCheck(-1);
#endif
	}
	fTable[table].fLength = newSize;
#if qIntenseDebugDisc
	fprintf(stderr, "After CDiscussion::ExpandTableEntry() that changed the tables:\n");
	Dump(stderr);
#endif
#if qDebugExpandTableEntry
	SanityCheck(-1);
#endif
	fTable[table].fUsedLength = newUsedSize;
	return 0;
}

short CDiscussion::SetNameFromHOL(HandleOffsetLength hol)
{
	short nameLen = short(hol.fLength);
#if qDebug
	if (nameLen < 0 || nameLen > 250)
		ProgramBreak("Bad nameLen");
#endif
	short spaceNeeded = ExpandTableEntry(kName, nameLen);
	if (spaceNeeded) 
		return spaceNeeded;
	Ptr p = Ptr(this) + fTable[kName].fOffset;
	BytesMove(*hol.fH + hol.fOffset, p, nameLen);
#if qIntenseDebugDisc
	fprintf(stderr, "After CDiscussion::SetNameFromHOL():\n");
	Dump(stderr);
#endif
	return 0;
}

short CDiscussion::SetMsgIDFromHOL(HandleOffsetLength hol, short hash)
{
#if qDebug
	if (hol.fLength < 0 || hol.fLength > 250)
		ProgramBreak("Bad MsgID length");
#endif
	short spaceNeeded = ExpandTableEntry(kMsgID, short(hol.fLength));
	if (spaceNeeded) 
		return spaceNeeded;
	Ptr p = Ptr(this) + fTable[kMsgID].fOffset;
	BytesMove(*hol.fH + hol.fOffset, p, hol.fLength);
	fHash = hash;
#if qIntenseDebugDisc
	fprintf(stderr, "After CDiscussion::SetMsgIDFromHOL():\n");
	Dump(stderr);
#endif
	return 0;
}

short CDiscussion::AddArticle(long id)
{
	short chunk = (fNoArticles + 8) & ~7; // chunk-size is 8 articles
	short spaceNeeded = ExpandTableEntry(kIDTable, chunk * sizeof(long));
	if (spaceNeeded) 
		return spaceNeeded;

	Ptr p = Ptr(this) + fTable[kIDTable].fOffset;
	long *lP = ( (long*) p ) + fNoArticles;
	*lP = id;
	fNoArticles++;
	GetDateTime(&fLastActiveDateTime);
#if qIntenseDebugDisc
	fprintf(stderr, "After CDiscussion::AddArticle():\n");
	Dump(stderr);
#endif
	return 0;
}

short CDiscussion::AddArticleAsOriginator(long id, 
					HandleOffsetLength msgIDHol, short hash, ArrayIndex link, HandleOffsetLength nameHol)
{
	short spaceNeeded = SetMsgIDFromHOL(msgIDHol, hash);
	if (spaceNeeded) 
		return spaceNeeded;

	spaceNeeded = SetNameFromHOL(nameHol);
	if (spaceNeeded) 
		return spaceNeeded;
	
	short chunk = (fNoArticles + 4) & ~3; // chunk-size is 4 articles
	spaceNeeded = ExpandTableEntry(kIDTable, chunk * sizeof(long));
	if (spaceNeeded) 
		return spaceNeeded;

	Ptr p = Ptr(this) + fTable[kIDTable].fOffset;
	MyBlockMove(p, p + sizeof(long), fNoArticles * sizeof(long));
	// insert article as the first one
	long *lP = (long*) p;
	*lP = id;
	fNoArticles++;
	fLink = link;
	GetDateTime(&fLastActiveDateTime);
#if qIntenseDebugDisc
	fprintf(stderr, "After CDiscussion::AddArticleAsOriginator():\n");
	Dump(stderr);
#endif
	return 0;
}

void CDiscussion::GetName(CStr255 &name)
{
#if qDebug
	if (fTable[kName].fUsedLength < 0 || fTable[kName].fUsedLength > 250)
	{
		Dump(stderr, -1);
		ProgramBreak("Invalid name-length");
	}
#endif
	name.Length() = fTable[kName].fUsedLength;
	Ptr p = Ptr(this) + fTable[kName].fOffset;
	BytesMove(p, &name[1], name.Length() + 1);
}

short CDiscussion::GetNumArticles()
{
	return fNoArticles;
}

TLongintList *CDiscussion::GetArticleIDList()
{
	TLongintList *list = nil;
	VOLATILE(list);
	FailInfo fi;
	Try(fi)
	{
		TLongintList *aList = new TLongintList();
		aList->ILongintList();
		list = aList;
		Ptr p = Ptr(this) + fTable[kIDTable].fOffset;
		MyInsertElementsBefore(list, 1, p, fNoArticles);
		fi.Success();
		return list;
	}
	else // fail
	{
		if (list) list->Free();
		fi.ReSignal();
	}
	return nil; // <PCB> needed.
}

long CDiscussion::GetArticleID(long articleIndex)
{
	Ptr p = Ptr(this) + fTable[kIDTable].fOffset;
	long *lP = (long*) p;
	return lP[articleIndex - 1];
}

long CDiscussion::GetLastArticleID()
{
	Ptr p = Ptr(this) + fTable[kIDTable].fOffset;
	long *lP = (long*) p;
	return lP[fNoArticles - 1];
}

Boolean CDiscussion::CompareMsgID(Ptr p, long len)
{
	if (len != fTable[kMsgID].fUsedLength)
		return false;
	register Ptr p1 = Ptr(this) + fTable[kMsgID].fOffset;
	register Ptr p2 = p;
	register long l = len;
	while (l && *p1++ == *p2++) 
		l--;
	return l == 0;
}

short CDiscussion::GetFreeSpace()
{
	return fTableBytesAlloc - (fTable[kMsgID].fLength + fTable[kName].fLength + fTable[kIDTable].fLength);
}

void CDiscussion::Dump(FILE *file, long index)
{
#if qDebug
	short freeSpace = GetFreeSpace();
	DateTimeRec creat, active;
	SecondsToDate(fCreateDateTime, &creat);
	SecondsToDate(fLastActiveDateTime, &active);

	fprintf(file, "CDiscussion at $%lx with index = %ld\n", long(this), index);
	fprintf(file, "  fLink = %ld, fHash = %hd\n", long(fLink), fHash);
	fprintf(file, "  fCreateDateTime     = %hd/%hd-%hd, %hd:%hd:%hd\n", creat.day, creat.month, creat.year, creat.hour, creat.minute, creat.second);
	fprintf(file, "  fLastActiveDateTime = %hd/%hd-%hd, %hd:%hd:%hd\n", active.day, active.month, active.year, active.hour, active.minute, active.second);
	fprintf(file, "  fNoArticle = %hd\n", fNoArticles);
	fprintf(file, "  fTableBytesAlloc = %hd, freeSpace = %hd\n", fTableBytesAlloc, freeSpace);
	fprintf(file, "  kMsgID:   %hd, %hd, %hd  (offset, length, usedLength)\n", 
									fTable[kMsgID].fOffset, fTable[kMsgID].fLength, fTable[kMsgID].fUsedLength);
	fprintf(file, "  kName:    %hd, %hd, %hd\n", fTable[kName].fOffset, fTable[kName].fLength, fTable[kName].fUsedLength);
	fprintf(file, "  kIDTable: %hd, %hd, %hd\n", fTable[kIDTable].fOffset, fTable[kIDTable].fLength, fTable[kIDTable].fUsedLength);
	fprintf(file, "  msg-ID: ");
	Ptr p;
	p = Ptr(this) + fTable[kMsgID].fOffset;
	fwrite(p, 1, int(MinMax(0, fTable[kMsgID].fUsedLength, 200)), file);
	fprintf(file, "\n");
	fprintf(file, "  name: ");
	p = Ptr(this) + fTable[kName].fOffset;
	fwrite(p, 1, int(MinMax(0, fTable[kName].fUsedLength, 200)), file);
	fprintf(file, "\n");
	fprintf(file, "  ID's: ");
	p = Ptr(this) + fTable[kIDTable].fOffset;
	long *lP = (long*) p;
	for (short i = 1; i <= fNoArticles; i++)
		fprintf(file, "%6hd", *lP++);
	fprintf(file, "\n\n");
#else
	file = file;
	index = index;
#endif
}

Boolean macroValidChar(char ch)
{
	return ch >= 32 || ch == 9 || ch < 0;
}

Boolean CDiscussion::SanityCheck(long index)
{
	Boolean isGood = true;
#if qDebug
	if (fTable[kMsgID].fOffset != sizeof(CDiscussion))
	{
		fprintf(stderr, "WRONG: CDiscussion::SanityCheck, found invalid message-ID offset in CDiscussion\n");
		isGood = false;
	}
	if (fTable[kName].fOffset != fTable[kMsgID].fOffset + fTable[kMsgID].fLength)
	{
		fprintf(stderr, "WRONG: CDiscussion::SanityCheck, found invalid disc-name offset in CDiscussion\n");
		isGood = false;
	}
	if (fTable[kIDTable].fOffset != fTable[kName].fOffset + fTable[kName].fLength)
	{
		fprintf(stderr, "WRONG: CDiscussion::SanityCheck, found invalid ID-list offset in CDiscussion\n");
		isGood = false;
	}
	Ptr p;
	short i;
	p = Ptr(this) + fTable[kName].fOffset;
	if (long(p) & 1)
		fprintf(stderr, "WRONG: CDiscussion::SanityCheck, article name at odd address\n");
	for (i = fTable[kName].fUsedLength; i; i--, p++)
	{
		if (!macroValidChar(*p))
		{ 
			fprintf(stderr, "WRONG: CDiscussion::SanityCheck, found invalid char %c = %ld in name\n", *p, long(*p)); 
			isGood = false;
		}
	}
	p = Ptr(this) + fTable[kMsgID].fOffset;
	if (long(p) & 1)
		fprintf(stderr, "WRONG: CDiscussion::SanityCheck, article message-ID at odd address\n");
	for (i = fTable[kMsgID].fUsedLength; i; i--, p++)
	{
		if (!macroValidChar(*p))
		{
			fprintf(stderr, "WRONG: CDiscussion::SanityCheck, found invalid char  %c = %ld in MsgID\n", *p, long(*p)); 
			isGood = false;
		}
	}
	p = Ptr(this) + fTable[kIDTable].fOffset;
	if (long(p) & 1)
		fprintf(stderr, "WRONG: CDiscussion::SanityCheck, article ID at odd address\n");
	long *lP = (long*) p;
	for (i = 1; i <= fNoArticles; i++, lP++)
	{
		if (*lP <= 0 || *lP >= 90000)
		{ 
			fprintf(stderr, "WRONG: CDiscussion::SanityCheck, found invalid article id\n"); 
			isGood = false;
		}
	}
	if (!isGood)
	{
		Dump(stderr, index);
		ProgramBreak("CDiscussion was BAD");
	}
#else
	index = index;
#endif
	return isGood;
}
