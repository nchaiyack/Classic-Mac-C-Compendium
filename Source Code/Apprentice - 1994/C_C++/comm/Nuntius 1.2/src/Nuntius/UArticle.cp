// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticle.cp

#include "UArticle.h"
#include "UArticleTextCache.h"
#include "NetAsciiTools.h"
#include "UOffsetLengthList.h"
#include "ISO2022Conversion.h"

#include <Script.h>

#pragma segment MyArticle

#define qDebugArticleRefCount qDebug

// The parsing in this code assumes that the article text
// is terminated with a null.
// The text is obtained from PArticleTextCache.
		
pascal void TArticle::Initialize()
{
	inherited::Initialize();
	fTextIndex = nil;
	fGroupDotName = "";
	fArticleID = -1;
	fContainsJapaneseEncoding = false;
}

void TArticle::IArticle(const CStr255 &groupDotName, long articleID)
{
	inherited::IObject();
	FailInfo fi;
	if (fi.Try())
	{
		TOffsetLengthList *olL = new TOffsetLengthList();
		olL->IOffsetLengthList();
		fTextIndex = olL;
		fTextIndex->fAllocationIncrement = 50; 
		// I think the default (6) is too low for this
		
		fGroupDotName = groupDotName;
		fArticleID = articleID;
		MakeTextIndex();
		fi.Success();
	}
	else // fail
	{
		Free();
		fi.ReSignal();
	}
}

pascal void TArticle::Free()
{
	FreeIfObject(fTextIndex); fTextIndex = nil;
	inherited::Free();
}

		
void TArticle::GetLineInfo(ArrayIndex &noLines, ArrayIndex &bodyStartLineNo)
{
#if qDebug
	if (fArticleID < 0)
		ProgramBreak("TArticle::GetLineInfo called when fArticleID < 0");
#endif
	noLines = fTextIndex->GetSize();
	bodyStartLineNo = fNoHeaderLines + 2;
}

void TArticle::GetLine(ArrayIndex lineNo, CStr255 &text)
{
#if qDebug
	if (lineNo < 1 || lineNo > fTextIndex->GetSize())
		ProgramBreak("lineNo out of range");
#endif
	OffsetLength ol;
	fTextIndex->AtGet(lineNo, ol);
	HandleOffsetLength hol;
	hol.fOffset = ol.fOffset;
	hol.fLength = ol.fLength;
	CStr255 dotName(fGroupDotName);
	hol.fH = gArticleTextCache->GetArticleText(dotName, fArticleID);
	if (!hol.fH)
		text = "";
	else
		CopyHolToCStr255(hol, text);
	if (fContainsJapaneseEncoding)
	{
		PStringTextCodeConverter cnv(text);
		cnv.ConvertNet2Mac();
		text = cnv.fOutputString;
	}
}
			
Boolean TArticle::GetHeader(const char *name, CStr255 &text)
{
	text.Length() = 0;
	CStr255 dotName(fGroupDotName);
	Handle h = gArticleTextCache->GetArticleText(dotName, fArticleID);
	if (!h)
		return false;
	unsigned char *p = (unsigned char*)*h;
	while (*p)
	{
		if (*p == 13)
			return false; // start of body
		if (*p <= 32)
		{
			 // header was breaked, skip next line
		}
		else if (CompareStoreHeaderName(name, p, text))
			return true;
		while (*p != 13)
			++p;
		++p; // skip CR
		if (*p == 10)
			++p; // skip LF
	}
	return false;
}

Boolean TArticle::CompareStoreHeaderName(const char *name, unsigned char *lineStart, CStr255 &text)
{
	const unsigned char *theName = (const unsigned char*)name;
	const unsigned char *p = lineStart;
	while (*theName)
	{
		if (gToLowerChar[*p] != gToLowerChar[*theName])
			return false;
		++p;
		++theName;
	}
	if (*p != ':')
		return false;
	++p;
// found it!
	while (*p == 32) // skip leading spaces
		++p;
	while (*p)
	{
		while (*p != 13)
		{
			text += *p++;
		}
		++p; // skip CR
		if (*p == 10)
			++p; // skip LF
		if (*p == 13)
			break; // start of body
		if (*p > 32) // start of next header
			break;
		text += char(32); // soft break is space
		while (*p == 9 || *p == 32) 
			++p;// skip all soft break spaces as they only count for one space
	}
	while (text.Length() && text[text.Length()] == 32) // strip spaces at end
		--text.Length();
	return true;
}

Boolean TArticle::ContainsJapaneseEncoding()
{
	return fContainsJapaneseEncoding;
}

Boolean TArticle::IsArticle(const CStr255 &groupDotName, long articleID)
{
	return groupDotName == fGroupDotName && articleID == fArticleID;
}

void TArticle::GetArticleID(CStr255 &groupDotName, long &articleID)
{
	groupDotName = fGroupDotName;
	articleID = fArticleID;
}

void TArticle::MakeTextIndex()
{
	fTextIndex->DeleteAll();
	fNoHeaderLines = 0;
	fContainsJapaneseEncoding = false;
	CStr255 dotName(fGroupDotName);
	Handle h = gArticleTextCache->GetArticleText(dotName, fArticleID);
	if (!h)
		return;
	register unsigned char *p = (unsigned char*)*h;
	short noLines = 0;
	Boolean inHeader = true;
	long numEscapes = 0;
	while (*p) 
	{
		unsigned char *lineStartP = p;
		while (true)
		{
			switch (*p)
			{
				case 0x1B:
					++p;
				  if      (*p == 0x28 && (p[1] == 0x42 || p[1] == 0x49 || p[1] == 0x4a))
						; // ok
					else if (*p == 0x24 && (p[1] == 0x40 || p[1] == 0x42))
						; // ok
					else
						continue; // bad
					++numEscapes;
					p += 2;
					continue;

				default:
					++p;
					continue;

				case 0:
				case 13:
					break;
			}
			break;
		}
		OffsetLength ol;
		ol.fOffset = Ptr(lineStartP) - *h;
		ol.fLength = p - lineStartP;
		if (ol.fLength <= 90)
			p++; // skip past CR
		else // break line
		{
			long len = 80;
			p = lineStartP + len - 1;
			const long minLen = 20;
			while (len > minLen && *p > 32)
			{
				--len;
				--p;
			}
			if (len > minLen)
			{
				ol.fLength = len;
				++p;
			}
			else
			{
				ol.fLength = 80;
				p = lineStartP + ol.fLength;
			}
		}
		if (!ol.fLength)
			inHeader = false;
		if (*p == 10) 
			++p;
		++noLines;
		if (inHeader)
			++fNoHeaderLines;

		// save pos
		long offset = Ptr(p) - *h; // save pos
		fTextIndex->InsertLast(ol);

		// restore pos
		p = (unsigned char*)(*h + offset);
	}
	if (GetEnvirons(smEnabled) > 1)
		fContainsJapaneseEncoding = (numEscapes >= 2);
}
