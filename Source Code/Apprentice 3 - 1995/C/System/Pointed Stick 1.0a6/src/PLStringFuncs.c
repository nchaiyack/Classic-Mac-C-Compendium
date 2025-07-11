#include "PLStringFuncs.h"

#define Min(x, y)		((x) < (y) ? (x) : (y))
#define kString1Lesser	-1
#define kEqual			0
#define kString1Greater	1

/*
PLSTRLEN                $0000001C  Code  Main            ext   
PLSTRSPN                $0000004E  Code  Main            ext   
PLSTRPBRK               $00000054  Code  Main            ext   
PLPOS                   $0000004C  Code  Main            ext   
//PLSTRCAT                $00000042  Code  Main            ext   
PLSTRCHR                $00000030  Code  Main            ext   
PLSTRRCHR               $0000003C  Code  Main            ext   
PLSTRNCAT               $00000056  Code  Main            ext   
//PLSTRCMP                $0000004C  Code  Main            ext   
//PLSTRCPY                $00000026  Code  Main            ext   
PLSTRNCMP               $0000005E  Code  Main            ext   
PLSTRSTR                $0000004A  Code  Main            ext   
PLSTRNCPY               $00000052  Code  Main            ext   

help dumpobj
dumpobj "{CLibraries}"StdClib.o -m PLSTRCPY
*/

//--------------------------------------------------------------------------------

pascal StringPtr	PLstrcat(StringPtr str1, StringPtr str2)
{
	StringPtr		dest = str1;
	StringPtr		src = str2;
	short			destLength = *dest;
	short			srcLength = *src++;
	short			spaceLeft;
	short			amtToCopy;

	spaceLeft = 255 - destLength;
	amtToCopy = Min(spaceLeft, srcLength);
	*dest++ += amtToCopy;
	SmallBlockMove((Ptr) src, (Ptr) dest + destLength, amtToCopy);

	return str1;
}


//--------------------------------------------------------------------------------

pascal short		PLstrcmp(StringPtr str1, StringPtr str2)
{
#if 0
	//
	// This stuff works (does a case-sensitive string compare, returning
	// -1, 0, or 1), but it�s not as short as calling the Toolbox.
	//
	short			result;
	StringPtr		string1 = str1;
	StringPtr		string2 = str2;
	short			lenToCompare = *string1++;
	short			lenStr2 = *string2++;

	result = lenToCompare - lenStr2;
	/*
		result < 0 if string1 shorter than string2
		result = 0 if lengths are equal
		result > 0 if string1 longer than string2
	*/

	if (lenToCompare > lenStr2) {
		lenToCompare = lenStr2;
	}
	
	if (lenToCompare > 0) {
		do {
			result = *string1++ - *string2++;
		} while (!result && --lenToCompare);
		/*
			result < 0 if string1 less than string2
			result = 0 if strings are equal
			result > 0 if string1 greater than string2
		*/
	} else {
		/*
			One of the strings was empty. In this case, the
			string that was not empty is returned as the greater
			one. If both are empty, they are equal. Since "result"
			reflects these conditions from above, there is nothing
			to do here.
		*/
	}

	if (result > 0) {
		result = kString1Greater;
	} else if (result < 0) {
		result = kString1Lesser;
	}
	return result;
#else
#if 1
	return RelString(str1, str2, TRUE, FALSE);
#else
	asm
	{
		move.l	str1,A0
		move.l	str2,A1
		moveq	#0,D0
		move.b	(A0)+,D0
		swap	D0
		move.b	(A1)+,D0
		RelString	CASE		; RelString(�, �, TRUE, FALSE) {case sensitive}
		move.w	D0,16(A6)
	}
#endif
#endif
}


pascal short		PLstrcompare(StringPtr str1, StringPtr str2)
{
#if 1
	return RelString(str1, str2, FALSE, FALSE);
#else
	asm
	{
		move.l	str1,A0
		move.l	str2,A1
		moveq	#0,D0
		move.b	(A0)+,D0
		swap	D0
		move.b	(A1)+,D0
		RelString				; RelString(�, �, FALSE, FALSE) {all insensitive}
		move.w	D0,16(A6)
	}
#endif
}

//--------------------------------------------------------------------------------

pascal StringPtr 	PLstrcpy(StringPtr dest, StringPtr source)
{
	asm
	{
		move.l	source,A0
		move.l	dest,A1
		moveq	#00,D0
		move.b	(A0),D0
	@1	move.b	(A0)+,(A1)+
		dbf		D0,@1
	}
	
	return dest;
}
