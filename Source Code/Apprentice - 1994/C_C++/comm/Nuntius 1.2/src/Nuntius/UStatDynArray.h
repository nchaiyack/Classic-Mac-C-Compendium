// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UStatDynArray.h

#define __USTATDYNARRAY__

#ifndef __UDYNDYNARRAY__
#include "UDynDynArray.h"
#endif

class PStatDynArray : public PDynDynArray
{
	public:
		virtual void DoRead(TStream *aStream);
		virtual void DoWrite(TStream *aStream);
		virtual long NeededDiskSpace();
		
		virtual ArrayIndex CreateNewElement(ArrayIndex size);
		virtual void DeleteElementAt(ArrayIndex index);
		virtual void DeleteAll();
		
		virtual Boolean EntryIsFree(ArrayIndex index);

		virtual void DebugDump(Boolean verbose);
		virtual Boolean SanityCheck();
		
		PStatDynArray();
		virtual void IStatDynArray(long allocChunk, long maxFreeElementDiff);
		~PStatDynArray();
	protected:
		TSortedLongintList *fFreeEntriesList;
		long fMaxFreeElementDiff;
};
