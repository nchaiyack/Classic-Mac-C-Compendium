//
// TrapAvailable.c
//
//	From Symantec's THINK Reference, which in turn got the code from an Apple Tech note.
//

#include<Traps.h>
#include<OSUtils.h>

#define TrapMask 0x0800

short NumToolboxTraps( void ) ;
TrapType GetTrapType(short theTrap) ;
Boolean TrapAvailable(short theTrap) ;


short NumToolboxTraps( void )
{
	if (NGetTrapAddress(_InitGraf, ToolTrap) ==
			NGetTrapAddress(0xAA6E, ToolTrap))
		return(0x0200) ;
	else
		return(0x0400) ;
}

TrapType GetTrapType(short theTrap)
{

	if ((theTrap & TrapMask) > 0)
		return(ToolTrap) ;
	else
		return(OSTrap) ;

}

Boolean TrapAvailable(short theTrap)
{

	TrapType	tType ;

	tType = GetTrapType(theTrap) ;
	if (tType == ToolTrap)
	theTrap = theTrap & 0x07FF ;
	if (theTrap >= NumToolboxTraps())
		theTrap = _Unimplemented ;

	return (NGetTrapAddress(theTrap, tType) !=
			NGetTrapAddress(_Unimplemented, ToolTrap)) ;
}

