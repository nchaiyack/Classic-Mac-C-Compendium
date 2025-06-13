#include "util.h"

/* Mymemcpy is now #defined in util.h */

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
