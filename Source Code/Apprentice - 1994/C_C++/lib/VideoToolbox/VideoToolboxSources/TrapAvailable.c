/*
TrapAvailable.c
From Apple. Simply include <Traps.h> in your program and call this routine to find
out whether any particular trap is available.
*/
#include "VideoToolbox.h"
#include <Traps.h>

// these are for internal use only
short NumToolboxTraps(void);		
TrapType GetTrapType(short theTrap);
	
Boolean	TrapAvailable(short theTrap)
{
	TrapType tType;
	
	tType=GetTrapType(theTrap);
	if(tType==ToolTrap){
		theTrap &= 0x07FF;
		if(theTrap>=NumToolboxTraps())theTrap=_Unimplemented;
	}
	return NGetTrapAddress(theTrap,tType) != NGetTrapAddress(_Unimplemented,ToolTrap);
}

short NumToolboxTraps(void)
{
	if (NGetTrapAddress(_InitGraf,ToolTrap) == NGetTrapAddress(0xAA6E,ToolTrap))
		return 0x0200;
	else return 0x0400;
}


TrapType GetTrapType(short theTrap)
{
	if((theTrap & 0x0800)>0) return ToolTrap;
	else return OSTrap;
}


