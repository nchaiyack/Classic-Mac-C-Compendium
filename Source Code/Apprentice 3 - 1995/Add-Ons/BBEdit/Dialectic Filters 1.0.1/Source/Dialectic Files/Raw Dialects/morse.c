/**********************************************************************\

File:		morse.c

Purpose:	This module handles actually converting text into Morse Code.


Dialectic -=- dialect text conversion extraordinare
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

#include "dialectic dispatch.h"
#include "dialectic utilities.h"
#include "program globals.h"

void ConvertMorse(void)
{
	unsigned char		oneChar;
	
	oneChar=ThisChar()|0x20;
	
	if (ThisChar()=='.')		StoreString("\p.-.-.-");
	else if (ThisChar()==',')	StoreString("\p--..--");
	else if ((ThisChar()==' ') || (ThisChar()==0x0d) || (ThisChar()==0x0a))
								StoreChar(ThisChar());
	else if (oneChar=='a')		StoreString("\p.-");
	else if (oneChar=='b')		StoreString("\p-...");
	else if (oneChar=='c')		StoreString("\p-.-.");
	else if (oneChar=='d')		StoreString("\p-..");
	else if (oneChar=='e')		StoreChar('.');
	else if (oneChar=='f')		StoreString("\p..-.");
	else if (oneChar=='g')		StoreString("\p--.");
	else if (oneChar=='h')		StoreString("\p....");
	else if (oneChar=='i')		StoreString("\p..");
	else if (oneChar=='j')		StoreString("\p.---");
	else if (oneChar=='k')		StoreString("\p-.-");
	else if (oneChar=='l')		StoreString("\p.-..");
	else if (oneChar=='m')		StoreString("\p--");
	else if (oneChar=='n')		StoreString("\p-.");
	else if (oneChar=='o')		StoreString("\p---");
	else if (oneChar=='p')		StoreString("\p.--.");
	else if (oneChar=='q')		StoreString("\p--.-");
	else if (oneChar=='r')		StoreString("\p.-.");
	else if (oneChar=='s')		StoreString("\p...");
	else if (oneChar=='t')		StoreChar('-');
	else if (oneChar=='u')		StoreString("\p..-");
	else if (oneChar=='v')		StoreString("\p...-");
	else if (oneChar=='w')		StoreString("\p.--");
	else if (oneChar=='x')		StoreString("\p-..-");
	else if (oneChar=='y')		StoreString("\p-.--");
	else if (oneChar=='z')		StoreString("\p--..");
	else if (oneChar=='0')		StoreString("\p-----");
	else if (oneChar=='1')		StoreString("\p.----");
	else if (oneChar=='2')		StoreString("\p..---");
	else if (oneChar=='3')		StoreString("\p...--");
	else if (oneChar=='4')		StoreString("\p....-");
	else if (oneChar=='5')		StoreString("\p.....");
	else if (oneChar=='6')		StoreString("\p-....");
	else if (oneChar=='7')		StoreString("\p--...");
	else if (oneChar=='8')		StoreString("\p---..");
	else if (oneChar=='9')		StoreString("\p----.");
	
	InputPlus(1);
}
