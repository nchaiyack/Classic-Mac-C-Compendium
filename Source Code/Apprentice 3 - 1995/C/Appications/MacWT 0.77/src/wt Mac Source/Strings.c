#include "Strings.h"


#ifndef __PLSTRINGFUNCS__

// Grrr -- need to hack together my own versions of PLstrcpy and PLstrcat for THICK C and MWC/PPC!

pascal StringPtr PLstrcpy(StringPtr str1, StringPtr str2)
{
	unsigned char	*cp = str1;
	unsigned		len = *(unsigned char *)str2;

	// Need to copy length byte as well, remember...

	do {
		*cp++ = *str2++;
	} while (len--);

	return str1;
}


/**/


pascal StringPtr PLstrcat(StringPtr str1, StringPtr str2)
{
	unsigned char	*cp = str1;
	unsigned		len1 = *(unsigned char *)str1,
					len2 = *(unsigned char *)str2;

	if ((len1 + len2) >= 255) {
		DebugStr((unsigned char *)"\pString concatenation -- dest too big!");
		return 0;
	}

	cp += len1 + 1;						// skip to end of str1
	*(unsigned char *)str1 += len2;		// adjust length byte
	++str2;								// don't copy "str2" length byte

	while (len2--)
		*cp++ = *str2++;

	return str1;
}

#endif /* __PLSTRINGFUNCS__ */


/**/

#ifndef	p2c

char *p2c(StringPtr pStr)
{
	unsigned	len;
	char		*cp;

	len = *(unsigned char *)pStr;
	cp = (char *)pStr;

	while (len--) {
		cp[0] = cp[1];					// slide chars backwards (over length byte)
		++cp;
	}

	*cp = 0;							// terminate C-string

	return (char *)pStr;				// return now C-style string
}


StringPtr c2p(char *cStr)
{
	unsigned char *cp;
	unsigned len, cStrLen;

	cStrLen = len = strlen(cStr);

	if (len > 255)						// perhaps we should have a _DebugStr here...
		len = 255;						// (don't overrun the max length)

	cp = (unsigned char *)cStr + len - 1;

	while (len--) {
		cp[1] = cp[0];					// slide chars forward (make room for length byte)
		--cp;
	}

	*cStr = cStrLen;					// bung in length byte

	return (StringPtr)cStr;				// return now-pascal style string
}

#endif	/* __STRINGS__ */
