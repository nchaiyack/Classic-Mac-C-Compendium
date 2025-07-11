/*
	Determining whether _WaitNextEvent is available.
	
	These routines were converted to THINK C directly from the examples
	show in Inside Macintosh Volume VI, page 3-8.
*/

#include <Traps.h>

int NumToolboxTraps(void);
TrapType GetTrapType(int theTrap);
Boolean TrapAvailable(int theTrap);
Boolean WNEAvailable(void);

int NumToolboxTraps(void)
{
	if (NGetTrapAddress(_InitGraf, ToolTrap) ==
		NGetTrapAddress(0xAA6E, ToolTrap))
	{
		return (0x200);
	}
	else
	{
		return (0x400);
	}
}

TrapType GetTrapType(int theTrap)
{
	#define TrapMask 0x800
	
	if ((theTrap & TrapMask) > 0)
	{
		return (ToolTrap);
	}
	else
	{
		return (OSTrap);
	}
}

Boolean TrapAvailable(int theTrap)
{
	TrapType tType;
	
	tType = GetTrapType(theTrap);
	if (tType == ToolTrap)
	{
		theTrap = theTrap & 0x07FF;
		if (theTrap >= NumToolboxTraps())
			theTrap = _Unimplemented;
	}
	return (NGetTrapAddress(theTrap, tType) !=
		NGetTrapAddress(_Unimplemented, ToolTrap));
}

Boolean WNEAvailable(void)
{
	return (TrapAvailable(_WaitNextEvent));
}