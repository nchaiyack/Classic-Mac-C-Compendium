// Copyright © 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleCache.cp

#include "UArticleCache.h"
#include "UArticle.h"
#include "Tools.h"
#include "UFatalError.h"

#include <stdio.h>

#pragma segment MyArticle

#define qDebugACBrief qDebug & 0
#define qDebugAC qDebugACBrief & 0

TArticleCache *gArticleCache = nil;

void InitUArticleCache()
{
#if qDebug
	if (gArticleCache)
		ProgramBreak("gArticleCache is not nil in InitUArticleCache");
#endif
	TArticleCache *ac = new TArticleCache();
	ac->IArticleCache();
	gArticleCache = ac;
}

void CloseDownUArticleCache()
{
	FreeIfObject(gArticleCache); gArticleCache = nil;
}

//-----------------------
#undef Inherited
#define Inherited TObject
DefineClass(TArticleCache, TObject)

TArticleCache::TArticleCache()
{
	fArticleList = nil;
	fRefCountList = nil;
}

void TArticleCache::IArticleCache()
{
	Inherited::IObject();
	FailInfo fi;
	Try(fi)
	{
		fArticleList = NewList();
		fRefCountList = NewLongintList();
		fi.Success();
	}
	else // fail
	{
//		Free();
		FreeIfObject(this);
		fi.ReSignal();
	}
}

void TArticleCache::Free()
{
	if (fArticleList)
	{
#if qDebug
		if (fArticleList->GetSize())
			fprintf(stderr, "TArticleCache, had %ld TArticles in cache when closed down\n", fArticleList->GetSize());
#endif
		fArticleList->FreeAll();
		fArticleList->Free();
	}
	FreeIfObject(fRefCountList);
 Inherited::Free();
}

TArticle *TArticleCache::GetArticle(const CStr255 &dotName, long articleID)
{
	ArrayIndex index = fArticleList->GetSize();
	while (index >= 1)
	{
		TArticle *article = (TArticle*) fArticleList->At(index);
		if (article->IsArticle(dotName, articleID))
		{
#if qDebugACBrief
			fprintf(stderr, "TAC::GetArticle: group = %s, article = %ld found with index = %ld\n", (char*)dotName, articleID, index);
#endif
			fRefCountList->AtPut(index, fRefCountList->At(index) + 1);
			return article;
		}
		index--;
	}

	// not found, create new
#if qDebugAC
	fprintf(stderr, "TAC::GetArticle: group = %s, article = %ld not found creating new\n", (char*)dotName, articleID);
#endif
	TArticle *article = new TArticle();
	article->IArticle(dotName, articleID);
	fArticleList->InsertLast(article);
	fRefCountList->InsertLast(1);
	return article;
}

void TArticleCache::ReturnArticle(TArticle *article)
{
	if (!article)
		return;
#if qDebug
	if (!VerboseIsObject(article))
		ProgramBreak("article is not object");
#endif
	ArrayIndex index = fArticleList->GetSize();
	while (index >= 1)
	{
		if (fArticleList->At(index) == article)
		{
			long refs = fRefCountList->At(index) - 1;
#if qDebugACBrief
			if (qDebugAC || refs != 0)
			{
				CStr255 ddnn;
				long articleID;
				article->GetArticleID(ddnn, articleID);
				fprintf(stderr, "TAC: retuned article, group = %s, article = %ld, new refs = %ld\n", (char*)ddnn, articleID, refs);
			}
#endif
			if (refs)
				fRefCountList->AtPut(index, refs);
			else
			{
				fRefCountList->DeleteElementsAt(index, 1);
				fArticleList->DeleteElementsAt(index, 1);
				article->Free();
			}
			return;
		}
		index--;
	}
#if qDebug
	CStr255 ddnn;
	long articleID;
	article->GetArticleID(ddnn, articleID);
	fprintf(stderr, "TAC: got (unknow) retuned article, group = %s, article = %ld\n", (char*)ddnn, articleID);
	ProgramBreak("Did not find returned article");
#endif
}

void TArticleCache::DiscardObject(TObject *obj)
{
#if qDebug
	ProgramBreak("TArticleCache::DiscardObject() called!");
#endif
	if (!obj)
		return;
	PanicExitToShell("TArticleCache::DiscardObject() called");
}

void TArticleCache::DebugDump()
{
#if qDebug
	fprintf(stderr, "Debug dump of TArticleCache:\n");
	ArrayIndex index = 1;
	while (index <= fArticleList->GetSize())
	{
		TArticle *article = (TArticle*) fArticleList->At(index);
		long refs = fRefCountList->At(index);
		CStr255 dotName;
		long articleID;
		article->GetArticleID(dotName, articleID);
		fprintf(stderr, "  group =%20s, article = %6ld, refs = %ld\n", (char*)dotName, articleID, refs);
	}
	fprintf(stderr, "End of dump\n");
#endif
}
