
#include <Traps.h>

int NumToolboxTraps()
{
	if (NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(0xAA6E, ToolTrap)) 
		return 0x200;
	else
		return 0x400;
}

TrapType GetTrapType (theTrap)
int	theTrap;
{
	int	TrapMask = 0x0800;

	if( BAND(theTrap, TrapMask) >0 )
		return ToolTrap;
	else
		return OSTrap;
}



Boolean TrapAvailable (theTrap)
int theTrap;
{
	TrapType	tType;

	tType = GetTrapType(theTrap);
	if( tType == ToolTrap)
	{
		theTrap = BAND(theTrap, 0x07FF);
		if( theTrap >= NumToolBoxTraps())
			theTrap = _Unimplemented;
	}
	if( NGetTrapAddress(theTrap, tType) != NGetTrapAddress(_Unimplemented, ToolTrap))
		return 1;
	else
		return 0;
}