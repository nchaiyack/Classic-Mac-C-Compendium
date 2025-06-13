#include "memory layer.h"

/* Mymemcpy is now #defined in memory layer.h */

void Mymemset(Ptr output, unsigned char value, unsigned long len)
{
	unsigned long	i;
	
	for (i=0; i<len; i++)
		*((unsigned char*)((long)output+i))=value;
}

Boolean Mymemcompare(Ptr thisThing, Ptr thatThing, unsigned long len)
{
	unsigned long	i;
	Boolean			goon;
	
	goon=TRUE;
	for (i=0; (i<len) && (goon); i++)
		goon=(*((unsigned char*)((long)thisThing+i))==*((unsigned char*)((long)thatThing+i)));
	return goon;
}

void AppendStr255(Str255 theStr, Str255 appendStr)
{
	short			i;
	
	for (i=1; i<=appendStr[0]; i++)
		theStr[++theStr[0]]=appendStr[i];
}

void AppendStr255ToHandle(Handle theHandle, Str255 theStr)
{
	Size			oldSize;
	Size			newSize;
	
	oldSize=GetHandleSize(theHandle);
	newSize=theStr[0];
	newSize+=oldSize;
	SetHandleSize(theHandle, newSize);
	Mymemcpy((Ptr)(*theHandle+oldSize), (Ptr)&theStr[1], theStr[0]);
}
