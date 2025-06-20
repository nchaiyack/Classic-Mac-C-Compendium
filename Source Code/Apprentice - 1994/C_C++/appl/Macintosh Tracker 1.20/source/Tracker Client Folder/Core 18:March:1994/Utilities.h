/* Utilities.h */

#pragma once

/* utility functions and stuff & Definitions */
/* this file contains useful type definitions and utility functions */

#define True (0 == 0)
#define False (0 != 0)
typedef short MyBoolean;

/* some useful typedefs */
#define NIL (0L)
typedef	unsigned long  ulong;
typedef unsigned short  ushort;
typedef unsigned char  uchar;
typedef	uchar  PString[256];
typedef struct {long Numerator; long Denominator;} Ratio;

typedef struct
	{
		long	x;
		long	y;
	} LongPoint;


/* memory utilities */

/* a better MEMCPY. */
void			MemCpy(char* Target, char* Source, signed long NumBytes);

/* memory compare */
MyBoolean	MemEqu(char* First, char* Second, long NumBytes);

/* the labs function */
long			LAbs(long value);

/* number of chars in a C string */
long			StrLen(char* It);


/* string utilities */

/* get a string from the resource file stored as a C string */
Handle		GetCString(ulong StringID);

/* make a handle string out of a C string constant */
Handle		CString2Handle(char* String);

/* make a handle string out of a Pascal string constant */
Handle		PString2Handle(PString String);

/* make a Pascal string out of a handle */
void			Handle2PString(Handle Source, PString PlaceToPut);

/* concatenate strings (the two original strings remain valid) */
Handle		HStrCat(Handle First, Handle Second);

/* compare two strings */
short			HStrCmp(Handle First, Handle Second);

/* duplicate a string */
Handle		HDuplicate(Handle Original);

/* make a handle out of data */
Handle		HandleOf(char* Data, long NumBytes);

/* convert a real number to a string handle (with no length delimitors) */
Handle		Real2String(long double Value, char Mode, short Digits);

/* convert a string to a real number */
long double		String2Real(Handle StringHandle);

/* convert a long integer to a string handle */
Handle		Int2String(long TheInt);

/* convert a string to an integer */
long			String2Int(Handle StringHand);


/* hex utilities */

/* convert a hexadecimal character into a byte value */
char			Hex2Byte(char Hex);

/* convert a byte to a hexadecimal character */
char			Byte2Hex(char Value);


#include "Debug.h"
#include "Audit.h"
