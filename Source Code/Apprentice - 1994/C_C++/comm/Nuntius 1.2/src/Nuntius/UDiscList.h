// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UDiscList.h

#define __UDISCLIST__

#ifndef __XTYPES__
#include "XTypes.h"
#endif

#ifndef __USTATDYNARRAY__
#include "UStatDynArray.h"
#endif

class CDiscussion;
class THashTable;

class PDiscList : public PStatDynArray 
{
	public:		
		void AddArticle(ArrayIndex discIndex, long id);
		void AddArticleAsOriginator(ArrayIndex discIndex, long id, 
										HandleOffsetLength msgIDHol, 
										HandleOffsetLength nameHol);

		ArrayIndex CreateDiscussion(long id, 
										HandleOffsetLength msgIDHol, 
										HandleOffsetLength nameHol);

		CDiscussion *ComputeAddress(ArrayIndex discIndex);

		void GetName(ArrayIndex discIndex, CStr255 &name); // needs lock!
		TLongintList *GetArticleIDList(ArrayIndex discIndex); // caller _owns_ returned list
		long GetArticleID(ArrayIndex discIndex, long articleIndex);

		short GetNoArticles(ArrayIndex discIndex);
		unsigned long GetLastActiveDateTime(ArrayIndex discIndex);

		void DeleteOldStuff(long firstAvailableArticleID, long lastAvailableArticleID);
		void DeleteAll();
		Boolean TryFindDiscussion(HandleOffsetLength idHol, ArrayIndex &discIndex);
		
		void DoRead(TStream *aStream);
		void DoWrite(TStream *aStream);
		long NeededDiskSpace();

		void DebugDump(Boolean verbose);		
		Boolean SanityCheck();
		
		PDiscList();
		void IDiscList();
		~PDiscList();
	private:
		THashTable *fHashTable;
		long fKillDiscussionTime;


		CDiscussion *FindDiscussionFromRef(char *&refP, long refLen, 
																			short hash, ArrayIndex &discIndex);
		Boolean FindNextReference(char *&refP, long &refLen, short &hash);
		void UnchainDiscussion(ArrayIndex index);
		TLongintList *FindDiscussionsToDelete(long firstAvailableArticleID, long lastAvailableArticleID);

		void ExpandDiscussion(ArrayIndex discIndex, short spaceNeeded);
		Boolean LockArray(Boolean lock);
		
		void DumpHashTableUsage();  // only for qDebug
		Boolean SanityCheckLinks(); // only for qDebug
};

inline CDiscussion *PDiscList::ComputeAddress(ArrayIndex discIndex)
{ return (CDiscussion*) PStatDynArray::ComputeAddress(discIndex); }

inline Boolean PDiscList::LockArray(Boolean lock)
{ return LockDataHandle(lock); }
