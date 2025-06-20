// Copyright � 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// NetAsciiTools.h

#define __NETASCIITOOLS__

typedef unsigned char CharsetTranslateTableType[256];
typedef unsigned char *CharsetTranslateTablePtr;
extern CharsetTranslateTableType gUS2Finnish, gUS2swe, gUS2dk, gToLowerChar;
extern CharsetTranslateTableType gMac2NetAscii, gNetAscii2Mac, gROT13Table;

void InitUNetAsciiTools();

void TranslateCStr255(CStr255 &s, const CharsetTranslateTablePtr table);
void TranslateViaTable(const CharsetTranslateTablePtr table, char *textP, long len);
void LoadTranslateTable(ResType theType, const CStr255 &rsrcName, CharsetTranslateTablePtr table);
void LoadNetAsciiTables();

void StripSpaces(CStr255 &s);
void StripWhiteSpaceAtEnd(CStr255 &s);

void MakeLastCharCR(Handle h); // unlocks handle
void MakeLastCharNull(Handle h); // unlock handle

void GetAuthorName(const CStr255 &text, CStr255 &realName, CStr255 &email);
void GetNationalAuthorNames(const CStr255 &text, CStr255 &realName, CStr255 &email);
Boolean GetPrintableAuthorName(const CStr255 &text, CStr255 &name, CStr255 &email);
// GetPrintableAuthorName uses name part of email if no real name

void MakeStringPrintable(const CStr255 &textIn, CStr255 &textOut);