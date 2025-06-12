#define FILE_NUM 51
/************************************************************************
 * ported from C++ code written by heksterb@cs.utwente.nl (Ben Hekster)
 ************************************************************************/
#pragma load EUDORA_LOAD
#pragma parameter __D0 CurA5
void *CurA5() = {0x200d};					/* MOVE.L A5,D0 */
#pragma parameter __D0 CurrentA7
void *CurrentA7() = {0x200f};					/* MOVE.L A7,D0 */

long UnloadUnneeded(long needed)
{
	long freed = 0;
	short i;
	void *mask = StripAddress((Ptr) 0xffffffff);
	Handle codeH;
	Size codeSize;

	/* find all purgeable but locked CODE resources in memory */
	SetResLoad(false);
	
	for (i = 0; i < NUM_CODE; i++)
	{
		CodeAddress[i].start = nil;
		codeH = GetResource('CODE', i);
		if (codeH && *codeH)
			/* is it a purgeable but locked resource? */
			if ((HGetState(codeH) & 0xe0) == 0xe0)
			{
				/* how big is it? */
				codeSize = GetHandleSize(codeH);
				if (codeSize > 0)
				{
					/* record the address range so it is eligible for purging */
					CodeAddress[i].start = (UPtr) ((long) *codeH & (long) mask);
					CodeAddress[i].end = CodeAddress[i].start + codeSize;
				}
			}
	}
	SetResLoad(true);

	/* examine the stack */
	{
		Zone *applZone = ApplicZone();
		void
			*stackTop = CurrentA7(),					/* stack to search */
			*stackBase = *((void**) CurStackBase),
			*heapStart = &applZone->heapData,	/* application zone data */
			*heapEnd = applZone->bkLim;				/* and the end... */
		char *a7;
		void *stackData;
		
		/* search the stack for references to loaded segments */
		for (a7 = (char*) stackTop; a7 < stackBase; a7 += 2)
		{
			/* is the address on the stack within the application heap? */
			stackData = (void*) (*(long*) a7 & (long) mask);
			if (stackData >= heapStart && stackData < heapEnd)
				/* is it in any of the code segments? */
				for (i = 0; i < NUM_CODE; i++)
					if (CodeAddress[i].start && stackData >= CodeAddress[i].start &&
							stackData < CodeAddress[i].end)
					{
						/* exclude it from unloading */
						CodeAddress[i].start = NULL;
						break;
					}
			}
		
		/* unload everything that wasn't referenced inside the stack */
		for (i = 2; i < NUM_CODE && needed>0; i++)
			/* may we unload it? */
			if (CodeAddress[i].start)
			{
				/* ready to get it on! */
				UnloadSeg((char*) CurA5() +
					(*(short*) CurJTOffset) + (*(short*) CodeAddress[i].start) + 2);
				freed += CodeAddress[i].end-CodeAddress[i].start;
				needed -= CodeAddress[i].end-CodeAddress[i].start;
			}
	}
	return(freed);
}
