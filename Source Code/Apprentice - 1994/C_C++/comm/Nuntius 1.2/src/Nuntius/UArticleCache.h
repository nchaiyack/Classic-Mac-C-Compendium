// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleCache.h

#define __UARTICLECACHE__

#ifndef __UPTROBJECT__
#include "UPtrObject.h"
#endif

class TArticle;

class PArticleCache : public PPtrObject
{
	public:
		TArticle *GetArticle(const CStr255 &dotName, long articleID);
		void ReturnArticle(TArticle *article);
		// obs: TArticles cannot be discarded as others may have refs to it
		
		void DebugDump();
		PArticleCache();
		void IArticleCache();
		~PArticleCache();
	private:
		TList *fArticleList;
		TLongintList *fRefCountList; // ref count for garbage collector

		void DiscardObject(TObject *obj); // may not be called
};

extern PArticleCache *gArticleCache;
void InitUArticleCache();
void CloseDownUArticleCache();
