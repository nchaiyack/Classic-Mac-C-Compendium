#include	"Conversions.h"

//#pragma smart_code

char *NumToHex2(unsigned long n, short digits);

/*
 * NumToHex - fixed length, returns a C string
 */
Ptr ShortToHexText(short n, Ptr text)
{
const char *conv = "0123456789ABCDEF";
register short	i;

text += 4;
for (i = 4; i != 0; n >>= 4, --i)
	*--text = conv[n & 0xf];
text += 4;
return text;
}

/*
 * NumToHex2 -- allows variable length, returns a Pascal string.
 */

void ShortToHexString(short n, Str255 out)
{
const char *conv = "0123456789ABCDEF";
register short	i;

*out++ = 4;
out += 4;
for (i = 4; i != 0; n >>= 4, --i)
	*--out = conv[n & 0xf];
return;
}

void ShortToOctString(short n, Str255 out)
{
const char *conv = "01234567";
register short	i;

*out++ = 6;
out += 6;
for (i = 5; i != 0; n >>= 3, --i)
	*--out = conv[n & 0x7];
*--out = conv[n & 0x1];
return;
}

void ShortToBinString(short n, Str255 out)
{
const char *conv = "01";
register short	i;

*out++ = 16;
out += 16;
for (i = 16; i != 0; n >>= 1, --i)
	*--out = conv[n & 0x1];
return;
}

void HexStringToShort(ConstStr255Param in, short *p)
{
register short tmp, i, ris = 0;

i = *in++;
i--;
i <<= 2;
for ( ; i >= 0; i -= 4 ) {
	tmp = *in++;
	tmp -= 48;
	if (tmp > 9)
		tmp -= 7;
	tmp <<= i;
	ris += tmp;
	}
*p = ris;
return;
}

void OctStringToShort(ConstStr255Param in, short *p)
{
register short tmp, i, ris = 0;

i = *in++;
i--;
tmp = i;
i <<= 1;
i += tmp;
for ( ; i >= 0; i -= 3 ) {
	tmp = *in++;
	tmp -= 48;
	tmp <<= i;
	ris += tmp;
	}
*p = ris;
return;
}

void BinStringToShort(ConstStr255Param in, short *p)
{
register short tmp, i, ris = 0;

i = *in++;
i--;
for ( ; i >= 0; i-- ) {
	tmp = *in++;
	tmp -= 48;
	tmp <<= i;
	ris |= tmp;
	}
*p = ris;
return;
}

