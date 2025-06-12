/* CardSlot.c
Finds the first NuBus slot that contains a card of the given name.
Returns -1 if there's no such card.

HISTORY:
1990 dgp wrote it.
8/24/91	dgp	Made compatible with THINK C 5.0.
11/23/92 dgp dropped obsolete support for THINK C 4.
*/
#include "VideoToolbox.h"

OSErr CardSlot(char *cardName)
{
	SpBlock mySpBlock;
	OSErr error;
	int slot;
	unsigned char name[128];
	
	for(slot=9;slot<16;slot++){
		mySpBlock.spSlot=slot;
		error=SCkCardStat(&mySpBlock);
		if(!error){
			mySpBlock.spID=1;
			mySpBlock.spResult=(long) &name;
			error=SReadDrvrName(&mySpBlock);
			if(!error && !strcmp(cardName,PtoCstr(name)))return slot;
		}
	}
	return -1;	/* no such card */
}
