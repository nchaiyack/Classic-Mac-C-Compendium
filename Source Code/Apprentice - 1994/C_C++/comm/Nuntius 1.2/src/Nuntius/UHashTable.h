// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UHashTable.h

#define __UHASHTABLE__

#ifndef __STDIO__
#include <stdio.h>
#endif

const short kHashBitShift = 5;
const short kHashTableEntries = 1 << (2 * kHashBitShift);
const short kHashTableSize = kHashTableEntries * sizeof(ArrayIndex);

class THashTable : public TObject
{
	public:
		short HashMessageID(char *p, long &len); // p must CPoint on '<' when called
		// does no mem-move
		
		void  SetValue(short index, ArrayIndex value);
		short GetValue(short index);
		void  FillHashTable(ArrayIndex value);

		void DoRead(TStream *aStream);
		void DoWrite(TStream *aStream);
		void DoNeedDiskSpace(long &dataForkBytes);
		
		void DebugDump(FILE *file);
		Boolean SanityCheck();
		
		THashTable();
		pascal void Initialize();
		void IHashTable();
		pascal void Free();
	private:
		ArrayIndex fHashTable[kHashTableEntries];
};

inline void THashTable::SetValue(short index, ArrayIndex value)
{ 
	fHashTable[index] = value;
}

inline short THashTable::GetValue(short index)
{
	return fHashTable[index];
}
