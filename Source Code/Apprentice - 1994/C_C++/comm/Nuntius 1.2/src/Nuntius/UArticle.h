// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticle.h

#define __UARTICLE__

#ifndef __XTYPES__
#include "XTypes.h"
#endif

class TOffsetLengthList;

class TArticle : public TObject
{
	public:
		void GetLineInfo(ArrayIndex &noLines, ArrayIndex &bodyStartLineNo);

		void GetLine(ArrayIndex lineNo, CStr255 &text); // without CR
			
		Boolean GetHeader(const char *name, CStr255 &text); 
		// returns false if non-existing.
		// *name == name of header without colon and space, eg "References"
	
		Boolean ContainsJapaneseEncoding();
		
		Boolean IsArticle(const CStr255 &groupDotName, long articleID);	
		void GetArticleID(CStr255 &groupDotName, long &articleID);	
		
		pascal void Initialize();
		void IArticle(const CStr255 &groupDotName, long articleID);
		pascal void Free();
	private:
		TOffsetLengthList *fTextIndex;
		CStr255 fGroupDotName;
		long fArticleID;
		long fNoHeaderLines;
		Boolean fContainsJapaneseEncoding;

		Boolean CompareStoreHeaderName(const char *name, unsigned char *lineStart, CStr255 &text);
		void MakeTextIndex();
};