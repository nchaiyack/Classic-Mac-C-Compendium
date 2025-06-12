/*
 * StrUtil.c
 *
 * String utility functions:
 *	pstrncpy		- Copy a Pascal string, up to a maximum length
 *	pstrncat		- Concatenate 2 Pascal strings, up to a maximum length
 *	ReplaceStr		- Replace the 1st occurence of a string with new text
 *	ParamString		- Like ParamText, but puts the result in a string
 *
 * Author:
 *	John Werner, werner@soe.berkeley.edu, 5/93
 */

#include "StrUtil.h"

#include <GestaltEqu.h>
#include <Packages.h>
#include <Script.h>			/* String truncation stuff */


/*-----------------------------------------------------------------------------
 * pstrncat
 * Copy one Pascal string to another.
 */
void pstrncpy(
	unsigned char		*to,		/* String to copy to */
	const unsigned char	*from,		/* What to copy into it */
	short				size)		/* Size of 'to', including length byte */
{
	short len = *from;
	if (len > size - 1)
		len = size - 1;
	BlockMove(from + 1, to + 1, len);
	*to = len;
}

/*-----------------------------------------------------------------------------
 * pstrncat
 * Concatenate two Pascal strings, just like 'strcat'
 */
void pstrncat(
	unsigned char		*to,		/* String to append to */
	const unsigned char	*from,		/* What to append to it */
	short				size)		/* Size of 'to', including length byte */
{
	short toLen = *to;
	short copyLen = *from;
	
	if (toLen + copyLen > size - 1) {
		copyLen = size - toLen - 1;
	}
	if (copyLen > 0) {
		BlockMove(from + 1, to + toLen + 1, copyLen);
		*to = toLen + copyLen;
	}
}

/*-----------------------------------------------------------------------------
 * ReplaceStr
 * Replace the first occurrance of a substring with another string
 */
OSErr ReplaceStr(
	unsigned char		*str,		/* String to search/replace in */
	const unsigned char *old, 		/* What to look for */
	const unsigned char *new,		/* What to replace it with */
	short				maxLen)		/* Max length of 'str' on output */
{
	OSErr err = noErr;
	Handle hdl = nil;
	short len;
	
	err = PtrToHand(str+1, &hdl, *str);
	if (!err && Munger(hdl, 0, old+1, *old, new+1, *new) < 0)
		err = paramErr;
	
	if (!err) {
		len = *str + *new - *old;
		if (len > maxLen-1)
			len = maxLen-1;
		*str = len;
		BlockMove(*hdl, str+1, len);
	}

	if (hdl != nil)
		DisposHandle(hdl);
	return err;
}

/*-----------------------------------------------------------------------------
 * ParamString
 * This is like the ParamText trap, it specifies string that replace '^0'..'^3'.
 * The difference is that the replacements are done immediately in the
 * string 'str', rather than later in a dialog box
 */
OSErr ParamString(
	unsigned char *		str,
	ConstStr255Param	r0, 
	ConstStr255Param	r1,
	ConstStr255Param	r2, 
	ConstStr255Param	r3,
	short				maxLen)
{
	OSErr err = noErr;
	Handle hdl = nil;
	short len;
	
	err = PtrToHand(str+1, &hdl, *str);
	if (!err) {
		if (r0)	Munger(hdl, 0, "^0", 2, r0+1, *r0);
		if (r1)	Munger(hdl, 0, "^1", 2, r1+1, *r1);
		if (r2)	Munger(hdl, 0, "^2", 2, r2+1, *r2);
		if (r3)	Munger(hdl, 0, "^3", 2, r3+1, *r3);
		len = GetHandleSize(hdl);
		if (len > maxLen - 1)
			len = maxLen - 1;
		*str = len;
		BlockMove(*hdl, str+1, len);
		DisposHandle(hdl);
	}
	return err;
}

