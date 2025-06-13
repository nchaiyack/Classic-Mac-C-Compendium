// Copyright © 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticle.cp

#include "UArticle.h"
#include "UArticleTextCache.h"
#include "NetAsciiTools.h"
#include "UOffsetLengthList.h"
#include "ISO2022Conversion.h"
#include "Tools.h"

#include <Script.h>

#undef Inherited
#define Inherited TObject
#pragma segment MyArticle
DefineClass(TArticle, TObject)


#define qDebugArticleRefCount qDebug

// The parsing in this code assumes that the article text
// is terminated with a null.
// The text is obtained from TArticleTextCache.
		
TArticle::TArticle()
{
	fTextIndex = nil;
	fGroupDotName = "";
	fArticleID = -1;
	fContainsJapaneseEncoding = false;
}

void TArticle::IArticle(const CStr255 &groupDotName, long articleID)
{
	Inherited::IObject();
	FailInfo fi;
	Try(fi)
	{
		TOffsetLengthList *olL = new TOffsetLengthList();
		olL->IOffsetLengthList();
		fTextIndex = olL;
		fTextIndex->fAllocationIncrement = 4 * 1024; 
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

void TArticle::Free()
{
	FreeIfObject(fTextIndex); fTextIndex = nil;
	Inherited::Free();
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
	Handle h = gArticleTextCache->GetArticleText(fGroupDotName, fArticleID);
	Boolean insSpace = false;
	if (ol.fOffset < 0)
	{
		insSpace = true;
		ol.fOffset = -ol.fOffset;
	}
	text.Length() = short(Min(250, ol.fLength));
	if (text.Length())
		BytesMove(*h + ol.fOffset, &text[1], text.Length());
	if (insSpace)
		text.Insert("    ", 1);
	if (fContainsJapaneseEncoding)
	{
		TStringTextCodeConverter cnv;
		cnv.IStringTextCodeConverter(text);
		cnv.ConvertNet2Mac();
		text = cnv.fOutputString;
	}
}
			
Boolean TArticle::GetHeader(const char *name, CStr255 &text)
{
	text.Length() = 0;
	Handle h = gArticleTextCache->GetArticleText(fGroupDotName, fArticleID);
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
			if (text.Length() < 255)
				text[++text.Length()] = *p++;
			else
				return true;
		}
		++p; // skip CR
		if (*p == 10)
			++p; // skip LF
		if (*p == 13)
			break; // start of body
		if (*p > 32) // start of next header
			break;
		if (text.Length() < 255)
			text[++text.Length()] = char(32); // soft break is space
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
	OffsetLength ol;
	fTextIndex->DeleteAll();
	fNoHeaderLines = 0;
	fContainsJapaneseEncoding = false;
	Handle h = gArticleTextCache->GetArticleText(fGroupDotName, fArticleID);
	if (!h)
		return;
	register unsigned char *p = (unsigned char*)*h;
	short noLines = 0;
	Boolean inHeader = true;
	Boolean indentLine = false;
	Boolean brokeLine = false;
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
		ol.fOffset = Ptr(lineStartP) - *h;
		if (indentLine)
			ol.fOffset = -ol.fOffset;
		ol.fLength = p - lineStartP + 1;
		// don't break line if it's a quote
		if (ol.fLength <= 90 || *lineStartP == '>' || fContainsJapaneseEncoding) 
		{
			p++; // skip past CR
			brokeLine = false;
		}
		else // break line
		{
			brokeLine = true;
			long len = 80;
			p = lineStartP + len - 1;
			const long minLen = 20;
			while (true)
			{
				if (len < minLen)
				{
					len = 80;
					break;
				}
				unsigned char ch = *p;
				if (ch == 32 || ch == 8)
					break;
				if (ch == '!')
				{
					--len;
					break;
				}
				--len;
				--p;
			}
			ol.fLength = len;
			p = lineStartP + len;
		}
		if (!ol.fLength || ol.fLength == 1 && *lineStartP == 13)
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
		indentLine = brokeLine && inHeader;
	}
	if (GetScriptManagerVariable(smEnabled) > 1)
		fContainsJapaneseEncoding = (numEscapes >= 2);
	// strip empty lines at bottom.
	while (fTextIndex->fSize)
	{	
		fTextIndex->AtGet(fTextIndex->fSize, ol);
		if (ol.fLength)
			break;
		fTextIndex->fSize--;
	}
}
