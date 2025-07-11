/**********************
** trapavail.c
**
** Checks for the existence of a trap.
**
**********************/

#include <Traps.h>

Boolean TrapAvailable(int);
static int NumToolboxTraps(void);
static TrapType GetTrapType(int);


static int NumToolboxTraps(void)
{
	return ( (NGetTrapAddress(_InitGraf, ToolTrap)==
			  NGetTrapAddress(0xAA6E, ToolTrap)) ? 0x0200 : 0x0400);
} /* NumToolboxTraps() */


static TrapType GetTrapType(int theTrap)
{
	unsigned TrapMask = 0x0800;
	return ((theTrap & TrapMask)>0 ? ToolTrap : OSTrap);
} /* GetTrapType() */


Boolean TrapAvailable(int theTrap)
{
	TrapType tType = GetTrapType( theTrap);
	if (tType == ToolTrap) {
		theTrap = (int)theTrap & (int)0x07FF;
		if (theTrap >= NumToolboxTraps())
			theTrap = _Unimplemented;
	}
	return (Boolean)(NGetTrapAddress(theTrap, tType) !=
			NGetTrapAddress(_Unimplemented, ToolTrap));
} /* TrapAvailable() */
