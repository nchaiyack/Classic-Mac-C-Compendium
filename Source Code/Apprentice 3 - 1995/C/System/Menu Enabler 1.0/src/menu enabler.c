/**********************************************************************\

File:		menu enabler.c

Purpose:	This file installs a tail-patch (gasp!) to MenuSelect to
			let you select disabled menu items.
			

Menu Enabler -=- a small menu hack
Copyright ©1994, Mark Pilgrim

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "Traps.h"
#include "SetUpA4.h"

unsigned long DisabledMenuSelect : 0xb54;

typedef		pascal unsigned long (*SelectProc)(Point startPt);

pascal void main(void);
pascal unsigned long MyMenuSelect(Point startPt);

SelectProc		real_MenuSelect;

pascal void main (void)
{
	void			*me;
	THz				saveZone;
	
	asm	{	move.l	a0,me	}
	RememberA0();
	SetUpA4();
	saveZone=GetZone();
	SetZone(SysZone);
	real_MenuSelect = (SelectProc)GetToolTrapAddress(_MenuSelect);
	SetToolTrapAddress((long)MyMenuSelect, _MenuSelect);
	DetachResource (RecoverHandle (me));
	SetZone(saveZone);
	RestoreA4();
}

pascal unsigned long MyMenuSelect(Point startPt)
{
	unsigned long	real_returnValue;
	
	SetUpA4();
	real_returnValue=real_MenuSelect(startPt);
	if (((real_returnValue>>16)&0xffff)==0)
	{
		real_returnValue=DisabledMenuSelect;
		DisabledMenuSelect=0L;
	}
	RestoreA4();
	return real_returnValue;
}
