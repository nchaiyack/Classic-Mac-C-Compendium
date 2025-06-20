// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// NetAsciiTools.h

#define __NETASCIITOOLS__

#ifndef __XTYPES__
#include "XTypes.h"
#endif

typedef unsigned char CharsetTranslateTableType[256];
typedef unsigned char *CharsetTranslateTablePtr;
extern CharsetTranslateTableType gUS2Finnish, gUS2swe, gUS2dk, gToLowerChar, gMac2NetAsctii;
extern CharsetTranslateTableType gMac2NetAscii, gNetAscii2Mac, gROT13Table;

void InitUNetAsciiTools();

void TranslateViaTable(const CharsetTranslateTablePtr table, char *textP, long len);
void LoadTranslateTable(const CStr255 &rsrcName, CharsetTranslateTablePtr table1, CharsetTranslateTablePtr table2);
void TranslateCStr255(CStr255 &s, const CharsetTranslateTablePtr table);

void StripSpaces(CStr255 &s);

void MakeLastCharCR(Handle h); // unlocks handle
void MakeLastCharNull(Handle h); // unlock handle

void GetAuthorName(const CStr255 &text, CStr255 &realName, CStr255 &email);
void GetNationalAuthorNames(const CStr255 &text, CStr255 &realName, CStr255 &email);
Boolean GetPrintableAuthorName(const CStr255 &text, CStr255 &name, CStr255 &email);
// GetPrintableAuthorName uses name part of email if no real name

