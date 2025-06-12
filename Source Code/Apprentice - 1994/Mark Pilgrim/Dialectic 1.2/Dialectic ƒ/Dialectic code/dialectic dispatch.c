/**********************************************************************\

File:		dialectic dispatch.c

Purpose:	This module handles dispatching to the different conversion
			routines.


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

unsigned char	gWhichDialect;
unsigned long	gInputOffset;
unsigned long	gOutputOffset;
Ptr				gInputBuffer;
Ptr				gOutputBuffer;
Boolean			gInputNeedsUpdate;
Boolean			gOutputNeedsUpdate;
unsigned long	gAbsoluteOffset;
unsigned long	gInputLength;
Boolean			gInWord;
Boolean			gSeenI;
Boolean			gSeenBackslash;
int				gCurlyLevel;
Boolean			gDoingRTF;

void ConvertDispatch(void)
{
	unsigned char	oneChar;
	
	if (gUseRTF)
	{
		oneChar=ThisChar();
		if ((oneChar=='}') && (gCurlyLevel>0))
		{
			gCurlyLevel--;
			if (gCurlyLevel==0)
				gDoingRTF=FALSE;
		}
		else if (oneChar=='{')
		{
			gCurlyLevel++;
			if ((NextChar(1)==0x5c) && (NextChar(2)=='r') && (NextChar(3)=='t') &&
				(NextChar(4)=='f'))
				gDoingRTF=TRUE;
		}
		
		if (gDoingRTF)
		{
			if (oneChar==0x5c)
				gSeenBackslash=TRUE;
			
			if ((gCurlyLevel>2) || (gSeenBackslash))
			{
				StoreChar(oneChar);
				InputPlus(1);
				if ((oneChar==' ') || (IsNumeric(oneChar)))
					gSeenBackslash=FALSE;
				return;
			}
		}
	}
	
	switch (gWhichDialect)
	{
		case kChef:		ConvertChef();		break;
		case kFudd:		ConvertFudd();		break;
		case kWAREZ:	ConvertWAREZ();		break;
		case kUbby:		ConvertUbby();		break;
		case kOlde:		ConvertOlde();		break;
		case kPig:		ConvertPig();		break;
		case kMorse:	ConvertMorse();		break;
		case kOp:		ConvertOp();		break;
		case kRot13:	ConvertRot13();		break;
	}
}

void SetupSuffix(void)
{
	switch (gWhichDialect)
	{
		case kChef:
			if (outputFS.name[0]>26)
				outputFS.name[0]=26;
			outputFS.name[++(outputFS.name[0])]='.';
			outputFS.name[++(outputFS.name[0])]='b';
			outputFS.name[++(outputFS.name[0])]='o';
			outputFS.name[++(outputFS.name[0])]='r';
			outputFS.name[++(outputFS.name[0])]='k';
			break;
		case kFudd:
			if (outputFS.name[0]>26)
				outputFS.name[0]=26;
			outputFS.name[++(outputFS.name[0])]='.';
			outputFS.name[++(outputFS.name[0])]='f';
			outputFS.name[++(outputFS.name[0])]='u';
			outputFS.name[++(outputFS.name[0])]='d';
			outputFS.name[++(outputFS.name[0])]='d';
			break;
		case kWAREZ:
			if (outputFS.name[0]>26)
				outputFS.name[0]=26;
			outputFS.name[++(outputFS.name[0])]='.';
			outputFS.name[++(outputFS.name[0])]='K';
			outputFS.name[++(outputFS.name[0])]='0';
			outputFS.name[++(outputFS.name[0])]='0';
			outputFS.name[++(outputFS.name[0])]='L';
			break;
		case kUbby:
			if (outputFS.name[0]>26)
				outputFS.name[0]=26;
			outputFS.name[++(outputFS.name[0])]='.';
			outputFS.name[++(outputFS.name[0])]='u';
			outputFS.name[++(outputFS.name[0])]='b';
			outputFS.name[++(outputFS.name[0])]='b';
			outputFS.name[++(outputFS.name[0])]='y';
			break;
		case kOlde:
			if (outputFS.name[0]>26)
				outputFS.name[0]=26;
			outputFS.name[++(outputFS.name[0])]='.';
			outputFS.name[++(outputFS.name[0])]='O';
			outputFS.name[++(outputFS.name[0])]='l';
			outputFS.name[++(outputFS.name[0])]='d';
			outputFS.name[++(outputFS.name[0])]='e';
			break;
		case kPig:
			if (outputFS.name[0]>25)
				outputFS.name[0]=25;
			outputFS.name[++(outputFS.name[0])]='.';
			outputFS.name[++(outputFS.name[0])]='i';
			outputFS.name[++(outputFS.name[0])]='g';
			outputFS.name[++(outputFS.name[0])]='p';
			outputFS.name[++(outputFS.name[0])]='a';
			outputFS.name[++(outputFS.name[0])]='y';
			break;
		case kMorse:
			if (outputFS.name[0]>25)
				outputFS.name[0]=25;
			outputFS.name[++(outputFS.name[0])]='.';
			outputFS.name[++(outputFS.name[0])]='m';
			outputFS.name[++(outputFS.name[0])]='o';
			outputFS.name[++(outputFS.name[0])]='r';
			outputFS.name[++(outputFS.name[0])]='s';
			outputFS.name[++(outputFS.name[0])]='e';
			break;
		case kOp:
			if (outputFS.name[0]>28)
				outputFS.name[0]=28;
			outputFS.name[++(outputFS.name[0])]='.';
			outputFS.name[++(outputFS.name[0])]='o';
			outputFS.name[++(outputFS.name[0])]='p';
			break;
		case kRot13:
			if (outputFS.name[0]>25)
				outputFS.name[0]=25;
			outputFS.name[++(outputFS.name[0])]='.';
			outputFS.name[++(outputFS.name[0])]='r';
			outputFS.name[++(outputFS.name[0])]='o';
			outputFS.name[++(outputFS.name[0])]='t';
			outputFS.name[++(outputFS.name[0])]='1';
			outputFS.name[++(outputFS.name[0])]='3';
			break;
	}
}

void SetSuffixMenuItem(MenuHandle theMenu, int theItem)
{
	switch (gWhichDialect)
	{
		case kChef:
			SetItem(theMenu, theItem, "\pAdd “.bork” suffix");
			break;
		case kFudd:
			SetItem(theMenu, theItem, "\pAdd “.fudd” suffix");
			break;
		case kWAREZ:
			SetItem(theMenu, theItem, "\pAdd “.K00L” suffix");
			break;
		case kUbby:
			SetItem(theMenu, theItem, "\pAdd “.ubby” suffix");
			break;
		case kOlde:
			SetItem(theMenu, theItem, "\pAdd “.Olde” suffix");
			break;
		case kPig:
			SetItem(theMenu, theItem, "\pAdd “.igpay” suffix");
			break;
		case kMorse:
			SetItem(theMenu, theItem, "\pAdd “.morse” suffix");
			break;
		case kOp:
			SetItem(theMenu, theItem, "\pAdd “.op” suffix");
			break;
		case kRot13:
			SetItem(theMenu, theItem, "\pAdd “.rot13” suffix");
			break;
	}
}
