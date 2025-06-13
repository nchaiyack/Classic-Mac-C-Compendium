// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UDiscussion.h

#define __UDISCUSSION__

#ifndef __XTYPES__
#include "XTypes.h"
#endif

#ifndef __STDIO__
#include <stdio.h>
#endif

enum TableType {kMsgID, kName, kIDTable};
const short kLastTable = kIDTable;

#define qDebugCDiscussion qDebug & 0

class CDiscussion 
{
	public:
		// the following methods:
		// AddArticle, AddArticleAsOriginator, GetArticleIDList
		// which adds something to a CDiscussion
		// returns the amount of space needed for that operation.
		// if they returns non-zero, the CDiscussion is not touched,
		// if zero: operation has completed!

		// be warned:
		// all methods except GetArticleIDList executes with CDiscussion 
		// in unlocked memory

		short AddArticle(long id);
		short AddArticleAsOriginator(long id, 
										HandleOffsetLength msgIDHol, short hash, ArrayIndex link, 
										HandleOffsetLength nameHol);
		
		void UpdateAlloc(short newSize);

		short GetNumArticles();
		short GetHash();
		unsigned long GetLastActiveDateTime();
		
		Boolean CompareMsgID(Ptr p, long len);
		
		void GetName(CStr255 &name);
		
		void SetLink(ArrayIndex newLink);
		ArrayIndex GetLink();

		TLongintList *GetArticleIDList(); // caller _owns_ returned list
		long GetArticleID(long articleIndex);
		long GetLastArticleID();

		void Dump(FILE *file, long index);
		Boolean SanityCheck(long index);
		short GetFreeSpace(); // for debug

		void Setup(short initialTableSize, short hash); // set create time/date
	private:
		ArrayIndex fLink; // kNone when last in chain (only used by TDiscBase)

		unsigned long fCreateDateTime;
		unsigned long fLastActiveDateTime; // only updated for discussions
		short fHash;

		short fNoArticles; // number of articles in this discussion
		short fTableBytesAlloc; // size of space allocated for tables

		struct
		{
			short fOffset, fLength, fUsedLength, fFiller;
		} fTable[kLastTable + 1];
		
		short ExpandTableEntry(TableType table, short newUsedSize);
		// if !0: nothing touched (returns amount of extra space needed)
		// if 0:  tables resized


		short SetNameFromHOL(HandleOffsetLength hol);
		short SetMsgIDFromHOL(HandleOffsetLength hol, short hash);
};

inline void CDiscussion::SetLink(ArrayIndex newLink) { fLink = newLink; }
inline ArrayIndex CDiscussion::GetLink() { return fLink; }
inline short CDiscussion::GetHash() { return fHash; }
inline unsigned long CDiscussion::GetLastActiveDateTime() { return fLastActiveDateTime; }
inline void CDiscussion::UpdateAlloc(short newSize) { fTableBytesAlloc = newSize; }
