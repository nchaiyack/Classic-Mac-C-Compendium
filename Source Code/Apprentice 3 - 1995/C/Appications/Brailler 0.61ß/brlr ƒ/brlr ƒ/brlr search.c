#include "brlr search.h"

short SearchForwards(short startPosition, Str255 theStr, TEHandle hTE)
{
	short			i;
	short			max;
	short			strPos;
	short			strLen;
	unsigned char	*a;
	
	i=startPosition;
	max=(**hTE).teLength;
	HLock((**hTE).hText);
	a=(unsigned char*)(*((**hTE).hText));
	strPos=1;
	strLen=theStr[0];
	while ((i<max) && (strPos<=strLen))
	{
		if (a[i]==theStr[strPos])
			strPos++;
		else
			strPos=1;
		i++;
	}
	
	HUnlock((**hTE).hText);
	if (strPos>strLen)
		return i-strLen;
	else
		return -1;
}

short SearchBackwards(short startPosition, Str255 theStr, TEHandle hTE)
/* note that startPosition is the starting position for looking for the last character in theStr */
{
	short			i;
	short			strPos;
	short			strLen;
	unsigned char	*a;
	
	i=startPosition;
	HLock((**hTE).hText);
	a=(unsigned char*)(*((**hTE).hText));
	strLen=theStr[0];
	strPos=strLen;
	while ((i>=0) && (strPos>0))
	{
		if (a[i]==theStr[strPos])
			strPos--;
		else
			strPos=strLen;
		i--;
	}
	
	HUnlock((**hTE).hText);
	if (strPos==0)
		return i+1;
	else
		return -1;
}
