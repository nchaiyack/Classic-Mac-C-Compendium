// Copyright © 1994-1995 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UArticleEntry.cp

#include "UArticleEntry.h"
#include "Tools.h"
#include "UHeaderFetcher.h"
#include "UIDDictionary.h"
#include "NetAsciiTools.h"

#include <stdio.h>

#pragma segment MyGroup

long CArticleLink::NumArticles() const
{
	long count = 0;
	const CArticleLink *al = fPrev;
	while (al)
	{
		al = al->fPrev;
		count++;
	}
	al = fNext;
	while (al)
	{
		al = al->fNext;
		count++;
	}
	return count;
}

CArticleLink *CArticleLink::GetLastLink()
{
	CArticleLink *al = this;
	while (al->fNext)
		al = al->fNext;
	return al;
}

Boolean CArticleLink::SanityCheck() const
{
#if qDebug
	if (fPrev && fPrev->fNext != this)
		fprintf(stderr, "fPrev->fNext != this\n");
	else if (fNext && fNext->fPrev != this)
		fprintf(stderr, "fNext->fPrev != this\n");
	else
#endif
		return true;
#if qDebug
	ProgramBreak("ups");
	return false;
#endif
}

//========================================================================
void CDiscEntry::TakeArticles(CDiscEntry *discEntry)
{
	CArticleLink *lastP = GetLastLink();
	lastP->fNext = discEntry->fNext;
	discEntry->fNext = nil;
	lastP->fNext->fPrev = lastP;
	GetDateTime(&fModDate);
#if qDebug
	SanityCheck();
#endif
}

void CDiscEntry::InsertArticle(CArticleEntry *entryP)
{
	//@@ should be improved: insert before first article
	// refering the inserted article
#if 0
	CArticleLink *al = GetLastLink();
	al->fNext = entryP;
	entryP->fPrev = al;
#else
	long newMsgID = entryP->GetRefID(0);
	CArticleLink *prevLink = this;
	CArticleEntry *ae = FirstArticleEntry();
	while (ae)
	{
		for (long index = 0; index < ae->GetNumRefIDs(); index++)
		{
			if (ae->GetRefID(index) == newMsgID)
				goto found;
		}
		prevLink = ae;
		ae = ae->NextArticleEntry();
	}
found:
	prevLink->fNext = entryP;
	entryP->fPrev = prevLink;
	if (ae)
	{
		entryP->fNext = ae;
		ae->fPrev = entryP;
	}
#endif
	GetDateTime(&fModDate);
#if qDebug
	SanityCheck();
#endif
}
	
CArticleEntry *CDiscEntry::ArticleByID(long id)
{
	CArticleEntry *entryP = FirstArticleEntry();
	while (entryP)
	{
		if (entryP->GetID() == id)
			return entryP;
		entryP = entryP->NextArticleEntry();
	}
	return nil;
}

long CDiscEntry::GetBiggestArticleNumber()
{
	long maxID = 0;
	CArticleEntry *entryP = FirstArticleEntry();
	while (entryP)
	{
		long id = entryP->GetID();
		if (id > maxID)
			maxID = id;
		entryP = entryP->NextArticleEntry();
	}
	return maxID;	
}

const char *CDiscEntry::GetName()
{
	CArticleEntry *entryP = FirstArticleEntry();
	if (entryP == nil)
		return "";
	const unsigned char *p = (const unsigned char *)entryP->GetSubject();
	while (true)
	{
		while (*p == chSpace || *p == chTab)
			++p;
		if (gToLowerChar[*p] != 'r')
			break;
		if (gToLowerChar[p[1]] != 'e')
			break;
		if (p[2] != ':' && p[2] != '>')
			break;
		p += 3;
	}
	return (const char*)p;
}

void CDiscEntry::DebugDump() const
{
#if qDebug
	fprintf(stderr, "DiscEntry, index = %ld\n", fDiscListIndex);
	CArticleLink *al = fNext;
	while (al)
	{
		( (CArticleEntry*) al)->DebugDump();
		al = al->fNext;
	}
#endif
}

Boolean CDiscEntry::SanityCheck() const
{
	Boolean isOK = true;
#if qDebug
	isOK &= CArticleLink::SanityCheck();
	const CArticleEntry *aeP = FirstArticleEntry();
	while (aeP)
	{
		isOK &= aeP->SanityCheck();
		aeP = aeP->NextArticleEntry();
	}
#endif
	return isOK;
}
//========================================================================

CDiscEntry *CArticleEntry::GetDiscEntry()
{
	CArticleLink *al = fPrev;
	while (al->fPrev)
		al = al->fPrev;
	return (CDiscEntry *)al;
}

CArticleEntry *CreateArticleEntry(THeaderFetcher *hf, TIDDictionary *dict)
{
	if (hf->GetNextArticle() == false)
		return nil;
	CStr255 subject, from;
	hf->GetSubject(subject);
	TranslateCStr255(subject, gNetAscii2Mac);
	hf->GetFrom(from);
	TranslateCStr255(from, gNetAscii2Mac);
	const long kMaxNumIDs = 64;
	long idTable[kMaxNumIDs+2];
	short numIDs = 1;
	CStr255 s;
	hf->GetMessageID(s);
	idTable[0] = dict->AddWord(s);
	while (numIDs < kMaxNumIDs && hf->GetNextReferenceID(s))
		idTable[numIDs++] = dict->AddWord(s);
	long size = sizeof(CArticleEntry)
		+ subject.Length() + 1
		+ from.Length() + 1
		+ numIDs * 4;
	char *addr = new char[size];
	CArticleEntry *ae = (CArticleEntry*)addr;
	ae->fPrev = nil;
	ae->fNext = nil;
	ae->fID = hf->GetArticleID();
	ae->fNumRefIDs = numIDs;
	BytesMove(idTable, addr + sizeof(CArticleEntry), numIDs * 4);
	ae->fSubjectOffset = sizeof(CArticleEntry) + numIDs * 4;
	ae->fFromOffset = ae->fSubjectOffset + subject.Length() + 1;
	p2cstrcpy(addr + ae->fSubjectOffset, subject);
	p2cstrcpy(addr + ae->fFromOffset, from);
	return ae;
}

long CArticleEntry::MemSize() const
{
	const char *p = GetFrom();
	while (*p)
		++p;
	return p - ((const char *)this) + 1;
}

void CArticleEntry::DebugDump() const
{
#if qDebug
	fprintf(stderr, "  ArticleEntry at $%ld: id = %ld\n", this, fID);
	fprintf(stderr, "    subject = '%s'\n", GetSubject());
	fprintf(stderr, "    from    = '%s'\n", GetFrom());
	fprintf(stderr, "    refs    = ");
	for (long index = 0; index < GetNumRefIDs(); index++)
		fprintf(stderr, " %ld", GetRefID(index));
	fprintf(stderr, "\n");
#endif
}

#if qDebug
CArticleLink *CArticleLink::operator ->()
{
	if (this == nil)
		ProgramBreak("this is nil!!!");
	return this;
}
CDiscEntry *CDiscEntry::operator ->()
{
	if (this == nil)
		ProgramBreak("this is nil!!!");
	return this;
}
CArticleEntry *CArticleEntry::operator ->()
{
	if (this == nil)
		ProgramBreak("this is nil!!!");
	return this;
}
#endif