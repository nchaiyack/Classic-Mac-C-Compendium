/*
	StringUtils.c
	
	Created	24 May 1992	
	Modified	25 May 1992	Fixed idiocies in AppendPStr (sheesh!)
			09 Aug 1992	Added CopyPStrFromHandle
			16 Aug 1992	Added PStrToULong
			20 Aug 1992	Fixed CopyPStrFromHandle so it works with 'STR#' (and not 'STR ') resources
			12 Sep 1992	Removed CopyPStrFromHandle Ñ Dragonsmith doesn't use it
			23 Feb 1993	Added GetNumberedPascalString
			18 May 1993	Added ComparePStrings

	Copyright © 1992, 1993 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (except as expressly permitted by the author)
		2.	This statement and the above copyright notice are left intact.

*/

#include	"StringUtils.h"

StringPtr GetNumberedPascalString (ResType resType, short resID, short stringIndex)
{
	StringResHndl		h;
	register short		i, n;
	register StringPtr	p;
	
	if (stringIndex > 0) {
		h = (StringResHndl) GetResource (resType, resID);
		if (h && *h) {
			n = (*h)->numStrings;
			if (stringIndex <= n) {
				for (p = &((*h)->strings[0]); --stringIndex; )
					p += p[0] + 1;
				return p;
			}
		}
	}
	
	return NULL;
}

Boolean PStrToULong (Str255 str, long *num)
{
	// Return TRUE if it's a valid number, otherwise FALSE

	register unsigned char	i, c, *p;
	unsigned char			len = *str;
	unsigned char			max[] = "\p4294967295";
	
	// Check for too-long numbers
	if (len > 10)
		return FALSE;
		
	// Check for max-length, overly large numbers
	else if (len == 10)
		for (i = 1, p = str + 1; i <= 10; i++) {
			if ((c = *p++) > max[i])
				return FALSE;
			else if (c < max[i])
				break;
		}
				
	for (i = len, p = str + 1; i--; )
		if ((c = *p++) < '0' || c > '9')
			return FALSE;
	StringToNum (str, num);
	return TRUE;
}

void SmartCopyPStr (register unsigned char *p1, register unsigned char *p2)
{
	if (p1 == NULL)
		return;
	if (p2 == NULL)										// If the destination pointer is NULL,
		p2 = p1;										//	just copy the source POINTER
	else if (StripAddress (p1) != StripAddress (p2))			// Otherwise, if the two pointers aren't equal,
		CopyPStr (p1, p2);								//	go ahead and call CopyPStr
}

void CopyPStr (register unsigned char *p1, register unsigned char *p2)
{
	register short	len;
	
	len = (short) *p1;
	do
		*p2++ = *p1++;
	while (len--);
}

void AppendPStr (unsigned char *p1, unsigned char *p2)
{
	/* Append the string pointed to by p2 to the string pointed to by p1 */
	register unsigned short	loop;
	register unsigned char	*s1, *s2 = p2;
	unsigned short		lenChange;
	
	lenChange = loop = (unsigned short) *s2++;
	s1 = p1 + *p1 + 1;
	for ( ; loop--; )
		*s1++ = *s2++;
	p1 [0] += lenChange;
}

short ComparePStrings (StringPtr str1, StringPtr str2)
{
	register short			len1, len2, i, ch;
	short				minLen, lenDiff;
	register unsigned char	*p1, *p2;
	
	len1 = str1[0];
	len2 = str2[0];
	lenDiff = len1 - len2;
	minLen = (lenDiff < 0 ? len1 : len2);
	
	for (i = minLen, p1 = str1, p2 = str2; i-- ; )
		if (ch = *p1++ - *p2++)
			return ch;
	
	return lenDiff;
	
}

