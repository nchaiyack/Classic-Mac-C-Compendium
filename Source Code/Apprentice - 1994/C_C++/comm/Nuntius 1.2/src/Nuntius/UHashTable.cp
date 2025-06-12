// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UHashTable.cp

#include "UHashTable.h"
#include "StreamTools.h"

#pragma segment MyGroup

const short kHashByteMask = (1 << (    kHashBitShift)) - 1;
const short kHashWordMask = (1 << (2 * kHashBitShift)) - 1;

const long kCurrentHashTableVersion = 1;
const long kMinHashTableVersion = 1;

THashTable::THashTable()
{
}

pascal void THashTable::Initialize()
{
	inherited::Initialize();
}

void THashTable::IHashTable()
{
	inherited::IObject();
}

pascal void THashTable::Free()
{
	inherited::Free();
}

void THashTable::DoRead(TStream *aStream)
{
	long version = aStream->ReadLong();
	MyStreamCheckVersion(version, kMinHashTableVersion, kCurrentHashTableVersion, "THashTable");
	Lock(true);
	aStream->ReadBytes(fHashTable, kHashTableSize);
	Lock(false);
#if qDebug
	SanityCheck();
#endif
}

void THashTable::DoWrite(TStream *aStream)
{
	aStream->WriteLong(kCurrentHashTableVersion);
	Lock(true);
	aStream->WriteBytes(fHashTable, kHashTableSize);
	Lock(false);
}

void THashTable::DoNeedDiskSpace(long &dataForkBytes)
{
	dataForkBytes += sizeof(long); // version
	dataForkBytes += kHashTableSize;
}

short THashTable::HashMessageID(char *p, long &len) // I don't know how effective this stuff is
// p must point on '<' when called
{
	register unsigned long hash = 0;
//	register unsigned char ch;
	register unsigned long ch; // saves a lot of char -> long conversions
	register long theLen = 0;
	register unsigned char *up = (unsigned char*)p;
	while (true) 
	{
#if 0
// 1
		ch = *up++;
		if (ch <= 32)
			break;
		hash ^= ch << kHashBitShift;
		theLen++;
		if (ch == '>')
			break;
// 2
		ch = *up++;
		if (ch <= 32)
			break;
		hash ^= (ch & kHashByteMask);
		theLen++;
		if (ch == '>')
			break;
// 3
		ch = *up++;
		if (ch <= 32)
			break;
		hash ^= ch << (kHashBitShift / 2);
		theLen++;
		if (ch == '>')
			break;
#endif
#define macroHashByte(x)		\
		ch = *up++;							\
		if (ch <= 32)						\
			break;								\
		hash ^= ch << x;				\
		theLen++;								\
		if (ch == '>')					\
			break;
// end macro
		macroHashByte(8);
		macroHashByte(3);
		macroHashByte(0);
		macroHashByte(6);
		macroHashByte(2);
		macroHashByte(5);
		macroHashByte(1);
		macroHashByte(4);
	}
	len = theLen;
	return short(kHashWordMask & hash);
}

void THashTable::FillHashTable(ArrayIndex value)
{
	ArrayIndex *p = fHashTable;
	for (short i = 0; i < kHashTableEntries; i++) 
		*p++ = value; // faster than indexing all the time!
#if qDebug
	SanityCheck();
#endif
}

void THashTable::DebugDump(FILE *file)
{
#if qDebug
	fprintf(stderr, "DebugDump of hashtable:\n");
	for (ArrayIndex index = 0; index < kHashTableEntries; index++)
	{
		if (index % 20 == 0)
			fprintf(file, "%4ld: ", index);
		ArrayIndex hash = fHashTable[index];
		if (hash == -1)
			fprintf(file, "    -");
		else
			fprintf(file, "%5ld", hash);
		if ((index + 1) % 20 == 0)
			fprintf(file, "\n");
	}
	fprintf(file, "\n");
#else
	file = file;
#endif
}

Boolean THashTable::SanityCheck()
{
	Boolean isGood = true;
#if qDebug
	for (ArrayIndex index = 0; index < kHashTableEntries; index++)
	{
		long hash = fHashTable[index];
		if (hash == -1)
			continue;
		if (hash < 0 || hash > 3000)
		{
			fprintf(stderr, "Wrong: THashTable::SanityCheck, bad value %ld with index %ld\n", hash, index);
			isGood = false;
		}
	}
	if (!isGood)
		DebugDump(stderr);
#endif
	return isGood;
}

