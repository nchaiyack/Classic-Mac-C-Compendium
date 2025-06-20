// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleTextCache.h

#define __UARTICLETEXTCACHE__

#ifndef __UPTROBJECT__
#include "UPtrObject.h"
#endif

const short kMaxNoCachedArticleTexts = 50;

class ArticleEntry
{
	public:
		long fID;
		Handle fH;
		long fSize;
		CStr255 fGroupDotName;
		
		ArticleEntry();
};
typedef ArticleEntry *ArticleEntryPtr;

class PArticleTextCache : public PPtrObject 
{
	public:
	
		Handle GetArticleText(const CStr255 &groupDotName, long id); // returns nil if article not avaible
		// The handles are null terminated (PNntp does the job)
		// OBS: PArticleTextCache _owns_ the handle!
		// The handle is only valid until
		//   1) GetArticle is called again
		//   2) MacAppYield is called

		void FlushCache();
		
		void DebugDump();
		
		PArticleTextCache();
		void IArticleTextCache();
		~PArticleTextCache();
	private:		
		ArticleEntry fArticleTable[kMaxNoCachedArticleTexts];

		short fOutIndex; // CPoints at entry that should be flushed when space is needed
		short fInIndex; // CPoints on last fetched entry
		short fLastUsedIndex; // CPoints on last used index
		short fNoUsedEntries;
		long fSumSize;
		
		void FlushOneArticle();
		void FlushToFit(const long maxSumSize);
		Handle GetArticleFromNntp(const CStr255 &groupDotName, long id);
};

extern PArticleTextCache *gArticleTextCache;
void InitUArticleTextCache();
void CloseDownUArticleTextCache();
