/*/
     Project Arashi: RamFiles.c
     Major release: Version 1.1, 7/22/92

     Last modification: Monday, April 5, 1993, 0:11
     Created: Friday, October 23, 1992, 16:39

     Copyright � 1992-1993, Juri Munkki
/*/

#include <RamFiles.h>

short	IncreaseRamFile(
	Handle	theFile,
	long	*realSize,
	long	*logicalSize,
	long	increase)
{	
	increase += increase & 1;	/*	Keep it even!	*/
	*logicalSize += increase;
	
	if( *logicalSize <= *realSize )
	{	return 0;
	}
	else
	{	SetHandleSize(theFile, *realSize+increase);
		*realSize = GetHandleSize(theFile);
		
		if(*logicalSize <= *realSize)
			return 0;
		else
		{	*logicalSize-=increase;
			return memFullErr;
		}
	}
}

short	IncreaseByClump(
	Handle	theFile,
	long	*realSize,
	long	*logicalSize,
	long	increase,
	long	clumpSize)
{	
	increase += increase & 1;
	*logicalSize += increase;	/*	Keep it even!	*/
	
	if( *logicalSize <= *realSize )
	{	return noErr;
	}
	else
	{	SetHandleSize(theFile, *realSize+increase+clumpSize);
		*realSize = GetHandleSize(theFile);
		
		if(*logicalSize <= *realSize)
		{	return noErr;
		}
		else
		{	SetHandleSize(theFile, *realSize+increase);
			*realSize = GetHandleSize(theFile);
			
			if(*logicalSize <= *realSize)
				return noErr;
			else
			{	*logicalSize-=increase;
				return memFullErr;
			}

		}
	}
}

void	PackRamFile(
	Handle	theFile,
	long	*realSize,
	long	*logicalSize)
{
	int	state;
	
	state = HGetState(theFile);
	HUnlock(theFile);
	
	SetHandleSize(theFile, *logicalSize);
	*realSize = GetHandleSize(theFile);
	
	HSetState(theFile,state);
}

short	OddIncreaseRamFile(
	Handle	theFile,
	long	*realSize,
	long	*logicalSize,
	long	increase)
{	
	*logicalSize += increase;
	
	if( *logicalSize <= *realSize )
	{	return 0;
	}
	else
	{	SetHandleSize(theFile, *realSize+increase);
		*realSize = GetHandleSize(theFile);
		
		if(*logicalSize <= *realSize)
			return 0;
		else
		{	*logicalSize-=increase;
			return memFullErr;
		}
	}
}

short	OddIncreaseByClump(
	Handle	theFile,
	long	*realSize,
	long	*logicalSize,
	long	increase,
	long	clumpSize)
{	
	*logicalSize += increase;
	
	if( *logicalSize <= *realSize )
	{	return noErr;
	}
	else
	{	SetHandleSize(theFile, *realSize+increase+clumpSize);
		*realSize = GetHandleSize(theFile);
		
		if(*logicalSize <= *realSize)
		{	return noErr;
		}
		else
		{	SetHandleSize(theFile, *realSize+increase);
			*realSize = GetHandleSize(theFile);
			
			if(*logicalSize <= *realSize)
				return noErr;
			else
			{	*logicalSize-=increase;
				return memFullErr;
			}

		}
	}
}
