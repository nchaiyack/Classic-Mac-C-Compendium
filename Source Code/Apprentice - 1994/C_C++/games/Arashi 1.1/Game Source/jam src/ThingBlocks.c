/*/
     Project Arashi: ThingBlocks.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:29
     Created: Monday, March 9, 1992, 23:01

     Copyright � 1992, Juri Munkki
/*/

#include "ThingBlocks.h"

ThingBlock	NewThingBlock(long thingSize)
{
	ThingBlockHeader	tbh;
	ThingBlock			theBlock;
	
	tbh.thingSize = thingSize;
	tbh.clumpSize = thingSize * 8;
	tbh.numItems = 0;
	tbh.logicalSize = sizeof(ThingBlockHeader)-sizeof(char)*2;
	tbh.physicalSize = tbh.logicalSize;
	
	theBlock = (ThingBlock)NewHandle(tbh.physicalSize);
	BlockMove(&tbh,*theBlock,tbh.logicalSize);
	
	return	theBlock;
}

int		EnlargeThing(ThingBlock tb, long sizeRequested)
{
	ThingBlockHeader	*tbp;
	long				newsize;
	
	tbp = *tb;
	if(sizeRequested+tbp->logicalSize > tbp->physicalSize)
	{	SetHandleSize(tb, tbp->logicalSize+tbp->clumpSize+sizeRequested);
		tbp = *tb;
		newsize = GetHandleSize(tb);
		if(tbp->physicalSize == newsize)
			return -1;
		else
		{	tbp->physicalSize = newsize;
		}
	}
	return noErr;
}

void	AddThing(ThingBlock tb, void *theThing)
{
	if(EnlargeThing(tb,(*tb)->thingSize) == noErr)
	{	BlockMove(theThing,((char *)*tb)+(*tb)->logicalSize,(*tb)->thingSize);
		(*tb)->logicalSize += (*tb)->thingSize;
		(*tb)->numItems++;
	}
}

void	DeleteWithSwap(ThingBlock tb, int index)
{
	(*tb)->logicalSize -= (*tb)->thingSize;
	(*tb)->numItems--;

	if((*tb)->numItems != index)
		BlockMove(	((Ptr)(*tb))+(*tb)->thingSize*index,
					((Ptr)(*tb))+(*tb)->logicalSize,
					(*tb)->thingSize);
}

