/*
	StringUtils.h
*/

#pragma once

typedef struct {
	short		numStrings;
	unsigned char	strings[];
} **StringResHndl;

StringPtr GetNumberedPascalString (ResType resType, short resID, short stringIndex);

Boolean PStrToULong (Str255 str, long *num);
void SmartCopyPStr (register unsigned char *p1, register unsigned char *p2);
void CopyPStr (unsigned char *p1, unsigned char *p2);
void AppendPStr (unsigned char *p1, unsigned char *p2);
short ComparePStrings (StringPtr str1, StringPtr str2);