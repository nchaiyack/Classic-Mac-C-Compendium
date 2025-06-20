#include	"TrapAvailable.h"

/* Code from Apple� converted by Stefan Kurth in super-optimized (!) C */

/* InitGraf is always implemented (trap $A86E). If the trap table is big
  enough, trap $AA6E will always point to either Unimplemented or some other
  trap, but will never be the same as InitGraf. Thus, you can check the size
  of the trap table by asking if the address of trap $A86E is the same as
  $AA6E. */

#define NumToolboxTraps	((GetToolTrapAddress(_InitGraf) == GetToolTrapAddress(0xAA6E)) ? 0x0200 : 0x0400)

/* Determines the type of a trap based on its trap number. If bit 11 is clear,
  then it is an OS trap. Otherwise, it is a Toolbox trap. */
/* OS traps start with A0, Tool with A8 or AA. */

#define GetTrapType(which)	(((which) & 0x0800) ? ToolTrap : OSTrap)
#define IsToolTrap(trap)	((trap) & 0x0800)

/* Check to see if a given trap is implemented. */

pascal Boolean TrapAvailable(short theTrap)
{
register UniversalProcPtr	theTrapAddress;
register short	temp;

temp = theTrap;
if(IsToolTrap(temp)) {
	temp &= 0x07FF;
	if( temp >= NumToolboxTraps)
		return false;
	theTrapAddress = GetToolTrapAddress(temp);
	}
else
	theTrapAddress = GetOSTrapAddress(temp);
return( GetToolTrapAddress(_Unimplemented) != theTrapAddress);
}

