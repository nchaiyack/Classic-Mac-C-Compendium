// Copyright © 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleCache.h

#define __UARTICLECACHE__

class TArticle;

class TArticleCache : public TObject
{
	DeclareClass(TArticleCache);
	public:
		TArticle *GetArticle(const CStr255 &dotName, long articleID);
		void ReturnArticle(TArticle *article);
		// obs: TArticles cannot be discarded as others may have refs to it
		
		void DebugDump();
		TArticleCache();
		void IArticleCache();
		void Free();
	private:
		TList *fArticleList;
		TLongintList *fRefCountList; // ref count for garbage collector

		void DiscardObject(TObject *obj); // may not be called
};

extern TArticleCache *gArticleCache;
void InitUArticleCache();
void CloseDownUArticleCache();
