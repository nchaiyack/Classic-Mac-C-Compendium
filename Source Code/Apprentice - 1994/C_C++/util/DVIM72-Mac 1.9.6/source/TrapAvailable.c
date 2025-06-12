#define _InitGraf		0xA86E
#define _Unimplemented	0xA89F

Boolean TrapAvailable( short trapnum );

#define GetNumToolBoxTraps (NGetTrapAddress(_InitGraf,ToolTrap) == \
	NGetTrapAddress(0xAA6E,ToolTrap)? 0x200 : 0x400)

#define TRAP_TYPE(n)	(n & 0x800 ? ToolTrap : OSTrap)

Boolean TrapAvailable( short trapnum )
{
	register short	ttype;
	
	ttype = TRAP_TYPE( trapnum );
	if (ttype == ToolTrap)
	{
		trapnum &= 0x7FF;
		if (trapnum >= GetNumToolBoxTraps)
			trapnum = _Unimplemented;
	}
	return (NGetTrapAddress( trapnum, ttype ) !=
		NGetTrapAddress( _Unimplemented, ToolTrap ));
}