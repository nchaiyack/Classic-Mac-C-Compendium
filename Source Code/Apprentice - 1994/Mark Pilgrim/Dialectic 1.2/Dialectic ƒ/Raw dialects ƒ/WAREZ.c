/**********************************************************************\

File:		WAREZ.c

Purpose:	This module handles actually converting text into WAREZ talk.


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

void ConvertWAREZ(void)
{
	char			oneChar;
	
	if ((ThisChar()=='.') && (!IsAlpha(NextChar(1))) && (!IsNumeric(NextChar(1))))
	{
		StoreString("\p!!");
		InputPlus(1);
		return;
	}
	
	if (ThisChar()==0x27)
	{
		StoreChar(0x22);
		InputPlus(1);
		return;
	}
	
	if (ThisChar()=='Õ')
	{
		StoreChar('Ó');
		InputPlus(1);
		return;
	}
	
	if (!IsAlpha(ThisChar()))
	{
		gInWord=FALSE;
		StoreChar(ThisChar());
		InputPlus(1);
		return;
	}
	
	oneChar=(ThisChar())|0x20;
	
	if ((!gInWord) && (oneChar=='d') && ((NextChar(1)|0x20)=='u') && ((NextChar(2)|0x20)=='d') && ((NextChar(3)|0x20)=='e'))
	{
		gInWord=TRUE;
		StoreString("\pD00D");
		InputPlus(4);
		return;
	}
	
	if ((gInWord) && (oneChar=='o') && ((NextChar(1)|0x20)=='u') && ((NextChar(2)|0x20)=='l') && ((NextChar(3)|0x20)=='d'))
	{
		StoreString("\pUD");
		InputPlus(4);
		return;
	}
	
	if ((gInWord) && (oneChar=='o') && ((NextChar(1)|0x20)=='u') && ((NextChar(2)|0x20)=='g') && ((NextChar(3)|0x20)=='h'))
	{
		StoreString("\pUF");
		InputPlus(4);
		return;
	}
	
	if ((gInWord) && (oneChar=='a') && ((NextChar(1)|0x20)=='u') && ((NextChar(2)|0x20)=='g') && ((NextChar(3)|0x20)=='h'))
	{
		StoreString("\pAFF");
		InputPlus(4);
		return;
	}
	
	if ((oneChar=='i') && ((NextChar(1)|0x20)=='l') && ((NextChar(2)|0x20)=='e'))
	{
		gInWord=TRUE;
		StoreString("\pIEL");
		InputPlus(3);
		return;
	}
	
	if (oneChar=='o')
	{
		gInWord=TRUE;
		StoreChar('0');
		InputPlus(1);
		return;
	}
	
	if (oneChar=='i')
	{
		gInWord=TRUE;
		StoreChar('1');
		InputPlus(1);
		return;
	}
	
	if ((!gInWord) && (oneChar=='t') && ((NextChar(1)|0x20)=='o') && (!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar('2');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='f') && ((NextChar(1)|0x20)=='o') && ((NextChar(2)|0x20)=='r') && ((NextChar(3)|0x20)=='e'))
	{
		gInWord=TRUE;
		StoreChar('4');
		InputPlus(4);
		return;
	}
	
	if ((oneChar=='f') && ((NextChar(1)|0x20)=='o') && ((NextChar(2)|0x20)=='r'))
	{
		gInWord=TRUE;
		StoreChar('4');
		InputPlus(3);
		return;
	}
	
	if (oneChar=='s')
	{
		gInWord=TRUE;
		StoreChar('5');
		InputPlus(1);
		return;
	}
	
	if ((oneChar=='a') && ((NextChar(1)|0x20)=='t') && ((NextChar(2)|0x20)=='e'))
	{
		gInWord=TRUE;
		StoreChar('8');
		InputPlus(3);
		return;
	}
	
	if ((!gInWord) && (oneChar=='b') && ((NextChar(1)|0x20)=='e'))
	{
		gInWord=TRUE;
		StoreChar('B');
		InputPlus(2);
		return;
	}
	
	if ((!gInWord) && (oneChar=='s') && ((NextChar(1)|0x20)=='e') && ((NextChar(2)|0x20)=='e') && (!IsAlpha(NextChar(3))))
	{
		gInWord=TRUE;
		StoreChar('C');
		InputPlus(3);
		return;
	}
	
	if ((!gInWord) && (oneChar=='a') && ((NextChar(1)|0x20)=='m') && (!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar('M');
		InputPlus(2);
		return;
	}
	
	if ((!gInWord) && (oneChar=='a') && ((NextChar(1)|0x20)=='r') && ((NextChar(2)|0x20)=='e') && (!IsAlpha(NextChar(3))))
	{
		gInWord=TRUE;
		StoreChar('R');
		InputPlus(3);
		return;
	}
	
	if ((!gInWord) && (oneChar=='e') && ((NextChar(1)|0x20)=='x'))
	{
		gInWord=TRUE;
		StoreChar('X');
		InputPlus(2);
		return;
	}
	
	if ((!gInWord) && (oneChar=='w') && ((NextChar(1)|0x20)=='h') && ((NextChar(2)|0x20)=='y') && (!IsAlpha(NextChar(3))))
	{
		gInWord=TRUE;
		StoreChar('Y');
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='c') && ((NextChar(1)|0x20)=='h'))
	{
		gInWord=TRUE;
		StoreString("\pCH");
		InputPlus(2);
		return;
	}
	
	if ((!gInWord) && (oneChar=='c'))
	{
		gInWord=TRUE;
		StoreChar('K');
		InputPlus(1);
		return;
	}
	
	if ((gInWord) && (oneChar=='s') && ((NextChar(1)|0x20)=='e') && (!IsAlpha(NextChar(2))))
	{
		StoreChar('Z');
		InputPlus(2);
		return;
	}
	
	if ((gInWord) && (oneChar=='e') && ((NextChar(1)|0x20)=='s') && (!IsAlpha(NextChar(2))))
	{
		StoreString("\pEZ");
		InputPlus(2);
		return;
	}
	
	if ((gInWord) && (oneChar=='s') && (!IsAlpha(NextChar(1))))
	{
		StoreChar('Z');
		InputPlus(1);
		return;
	}
	
	if ((oneChar=='p') && ((NextChar(1)|0x20)=='h'))
	{
		gInWord=TRUE;
		StoreChar('F');
		InputPlus(2);
		return;
	}
	
	if ((!gInWord) && (oneChar=='y') && ((NextChar(1)|0x20)=='o') && ((NextChar(2)|0x20)=='u') && ((NextChar(3)|0x20)=='r'))
	{
		gInWord=TRUE;
		StoreString("\pUR");
		InputPlus(4);
		return;
	}
	
	if ((!gInWord) && (oneChar=='y') && ((NextChar(1)|0x20)=='o') && ((NextChar(2)|0x20)=='u') && (!IsAlpha(NextChar(3))))
	{
		gInWord=TRUE;
		StoreChar('U');
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='e') && ((NextChar(1)|0x20)=='a'))
	{
		gInWord=TRUE;
		StoreString("\pEE");
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='a') && ((NextChar(1)|0x20)=='n') && ((NextChar(2)|0x20)=='d'))
	{
		gInWord=TRUE;
		StoreChar('&');
		InputPlus(3);
		return;
	}
	
	if ((!gInWord) && (oneChar=='w') && ((NextChar(1)|0x20)=='a') && ((NextChar(2)|0x20)=='s'))
	{
		gInWord=TRUE;
		StoreString("\pWUZ");
		InputPlus(3);
		return;
	}
	
	if ((!gInWord) && (oneChar=='s') && ((NextChar(1)|0x20)=='a') && ((NextChar(2)|0x20)=='y') && ((NextChar(3)|0x20)=='s'))
	{
		gInWord=TRUE;
		StoreString("\pSEZ");
		InputPlus(4);
		return;
	}
	
	gInWord=TRUE;
	StoreChar(ThisChar()&0xdf);
	InputPlus(1);
}
