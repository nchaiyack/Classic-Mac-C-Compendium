// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleTextCache.cp

#include "UArticleTextCache.h"
#include "UNntp.h"
#include "UGroupDoc.h"
#include "UPrefsDatabase.h"
#include "Tools.h"

#include <RsrcGlobals.h>
#include <Folders.h>

#pragma segment MyArticle

#define qDebugCache qDebug & 0
#define qDebugCacheIntense qDebug & 0

PArticleTextCache *gArticleTextCache = nil;

//------------------------------------------------------
ArticleEntry::ArticleEntry()
{
}

//------------------------------------------------------
PArticleTextCache::PArticleTextCache()
{
//	inherited::Initialize();
	fInIndex = 0;
	fOutIndex = fInIndex + 1;
	fLastUsedIndex = 0;
	fNoUsedEntries = 0;
	fSumSize = 0;
	for (short i = 0; i < kMaxNoCachedArticleTexts; i++)
	{
		fArticleTable[i].fID = -1;
		fArticleTable[i].fH = nil;
		fArticleTable[i].fSize = 0;
	}
}

void PArticleTextCache::IArticleTextCache()
{
//	inherited::IObject();
}

PArticleTextCache::~PArticleTextCache()
{
	for (short i = 0; i < kMaxNoCachedArticleTexts; i++)
	{
		DisposeIfHandle(fArticleTable[i].fH);
	}
//	inherited::Free();
}

inline void GoNextEntry(short &index)
{ 
	if (index == kMaxNoCachedArticleTexts - 1)
		index = 0;
	else
		index++;
}

inline void GoPrevEntry(short &index)
{
	if (index)
		index--;
	else
		index = kMaxNoCachedArticleTexts - 1;
}

void PArticleTextCache::FlushOneArticle()
{
	ArticleEntryPtr aeP = fArticleTable + fOutIndex;
#if qDebug
//	Boolean prevLock = Lock(true);
#if qDebugCache
	fprintf(stderr, "PArticleTextCache: Flushing, id = %ld, size = %ld, handle = %lx, fSumSize = %ld\n", aeP->fID, aeP->fSize, aeP->fH, fSumSize);
#endif
	if (!IsHandle(aeP->fH))
	{
		ProgramBreak("The article handle is not valid (flushing)\n");
		aeP->fH = nil;
	}
//	Lock(prevLock);
#endif
	Handle h = aeP->fH;  // wait with m-mgr until done with Pointer
	aeP->fH = nil;
	fSumSize -= aeP->fSize;
	aeP->fSize = 0;
	aeP->fID = -1;
	if (h)
		DisposeHandle(h);
	fNoUsedEntries--;
	GoNextEntry(fOutIndex);
	fLastUsedIndex = fInIndex;
}

void PArticleTextCache::FlushToFit(const long maxSumSize)
{
	while (fSumSize > maxSumSize || fNoUsedEntries >= kMaxNoCachedArticleTexts)
	{
		FlushOneArticle();
	} // while
}

Handle PArticleTextCache::GetArticleFromNntp(const CStr255 &groupDotName, long id)
{
	FlushToFit(gPrefs->GetLongPrefs('AMem'));
	PNntp *nntp = gNntpCache->GetNntp();
	Handle h = nil;
	FailInfo fi;
	if (fi.Try())
	{
		nntp->SetGroup(groupDotName);
		h = nntp->GetArticle(id);
		fi.Success();
	}
	else // fail
	{
		gNntpCache->DiscardNntp(nntp);
		fi.ReSignal();
	}
	gNntpCache->ReturnNntp(nntp); nntp = nil;
#if qDebug
	if (!h)
		ProgramBreak("PNntp returned _NIL_ article handle to PArticleTextCache");
#endif
	GoNextEntry(fInIndex);
	if (!fNoUsedEntries)
	{
#if qDebug
		if (fInIndex != fOutIndex)
			ProgramBreak("misalign between fInIndex and fOutIndex");
#endif
		fOutIndex = fInIndex;
	}
	fNoUsedEntries++;
#if qDebugCacheIntense
	fprintf(stderr, "PArticleTextCache, got article group = %s, ID = %ld from nntp\n", (char*)groupDotName, id);
	fprintf(stderr, "-   put in cache with index = %ld, size = %ld\n", fInIndex, GetHandleSize(h));
#endif
	fArticleTable[fInIndex].fID = id;
	fArticleTable[fInIndex].fGroupDotName = groupDotName;
	fArticleTable[fInIndex].fH = h;
	long size = GetHandleSize(h);
	fSumSize += size;
	fArticleTable[fInIndex].fSize = size;
	fLastUsedIndex = fInIndex;
#if qDebug
		ArticleEntryPtr aeP = fArticleTable + fInIndex;
//		Boolean prevLock = Lock(true);
#if qDebugCache
		fprintf(stderr, "PArticleTextCache: new article id = %ld, size = %ld, handle = 0x%lx\n", aeP->fID, aeP->fSize, aeP->fH);
#endif
		if (!IsHandle(aeP->fH))
			ProgramBreak("The article handle is not valid (obtained from nntp)\n");
//		Lock(prevLock);
#endif
	return h;
}

Handle PArticleTextCache::GetArticleText(const CStr255 &groupDotName, long id)
{
#if qDebugCacheIntense
	fprintf(stderr, "PArticleTextCache::GetArticle, group = %s, ID = %ld\n", (char*)groupDotName, id);
#endif
	// was it the last one?
	if (fNoUsedEntries && id == fArticleTable[fLastUsedIndex].fID && groupDotName == fArticleTable[fLastUsedIndex].fGroupDotName)
	{
#if qDebugCacheIntense
		fprintf(stderr, "-  found article as last used with index = %ld\n", fLastUsedIndex);
#endif
		return fArticleTable[fLastUsedIndex].fH;
	}
	// else try go seek for it
	short index = fInIndex;
	for (short i = 1; i <= kMaxNoCachedArticleTexts; i++)
	{
		if (id == fArticleTable[index].fID && groupDotName == fArticleTable[index].fGroupDotName)
		{
#if qDebugCacheIntense
			fprintf(stderr, "-  found article with index = %ld, fLastUsedIndex = %ld\n", i, fLastUsedIndex);
#endif
			fLastUsedIndex = index;
			return fArticleTable[index].fH; // found it
		}
		GoPrevEntry(index);
	}
#if qDebugCacheIntense
	fprintf(stderr, "-  did not find article, fetches it from nntp\n");
#endif
	return GetArticleFromNntp(groupDotName, id);
}

void PArticleTextCache::FlushCache()
{
	while (fNoUsedEntries)
		FlushOneArticle();
}

void InitUArticleTextCache()
{
#if qDebug
	if (gArticleTextCache)
		ProgramBreak("InitUArticleTextCache is called twice");
#endif
	PArticleTextCache *ac = new PArticleTextCache();
	ac->IArticleTextCache();
	gArticleTextCache = ac;
}

void CloseDownUArticleTextCache()
{
#if qDebug
	if (!gArticleTextCache)
		fprintf(stderr, "gArticleTextCache is nil in CloseDownUArticleTextCache\n");
#endif
	FreeIfPtrObject(gArticleTextCache); 	gArticleTextCache = nil;
}

void PArticleTextCache::DebugDump()
{
#if qDebug
	fprintf(stderr, "Debug dump of article text cache:\n");
	short index = fInIndex;
	for (short i = 1; i <= kMaxNoCachedArticleTexts; i++)
	{
		ArticleEntry entry;
		BytesMove(fArticleTable + index, &entry, sizeof(entry));
		if (entry.fID >= 0)
		{
			fprintf(stderr, "   index = %ld, group = %s, ID = %ld, size = %ld, handle = $%lx\n",
				i, (char*)entry.fGroupDotName, entry.fID, entry.fSize, entry.fH);
			if (!IsHandle(entry.fH))
				fprintf(stderr, "    *** This handle is not valid\n");
		}
		GoPrevEntry(index);
	}
#endif
}
