/**********************************************************************\

File:		ubby.c

Purpose:	This module handles actually converting text into Ubby talk.


Dialectic -=- dialect text conversion extraordinare
Copyright �1994, Mark Pilgrim

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

#include "dialectic dispatch.h"
#include "dialectic utilities.h"
#include "program globals.h"

void ConvertUbby(void)
{
	char			oneChar;
	
	oneChar=(ThisChar())|0x20;
	
	if (oneChar=='a')
	{
		StoreString(IsUpperAlpha(ThisChar()) ? IsUpperAlpha(NextChar(1)) ?
			"\pUHBA" : "\pUhba" : "\puhba");
		InputPlus(1);
		return;
	}
	
	if (oneChar=='e')
	{
		StoreString(IsUpperAlpha(ThisChar()) ? IsUpperAlpha(NextChar(1)) ?
			"\pUHBE" : "\pUhbe" : "\puhbe");
		InputPlus(1);
		return;
	}
	
	if (oneChar=='i')
	{
		StoreString(IsUpperAlpha(ThisChar()) ? IsUpperAlpha(NextChar(1)) ?
			"\pUHBI" : "\pUhbi" : "\puhbi");
		InputPlus(1);
		return;
	}
	
	if (oneChar=='o')
	{
		StoreString(IsUpperAlpha(ThisChar()) ? IsUpperAlpha(NextChar(1)) ?
			"\pUHBO" : "\pUhbo" : "\puhbo");
		InputPlus(1);
		return;
	}
	
	if (oneChar=='u')
	{
		StoreString(IsUpperAlpha(ThisChar()) ? IsUpperAlpha(NextChar(1)) ?
			"\pUHBU" : "\pUhbu" : "\puhbu");
		InputPlus(1);
		return;
	}
	
	StoreChar(ThisChar());
	InputPlus(1);
}
