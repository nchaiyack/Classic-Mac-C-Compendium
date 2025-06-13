/**********************************************************************\

File:		dialect dispatch.h

Purpose:	This is the header file for dialect dispatch.c


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

enum
{
	kChef=0,
	kFudd,
	kWAREZ,
	kUbby,
	kOlde,
	kPig,
	kMorse,
	kOp,
	kRot13
};

void ConvertDispatch(void);
void SetupSuffix(void);
void SetSuffixMenuItem(MenuHandle theMenu, int theItem);

void ConvertChef(void);
void ConvertFudd(void);
void ConvertUbby(void);
void ConvertDeUbby(void);
void ConvertWAREZ(void);
void ConvertOlde(void);
void ConvertPig(void);
void ConvertMorse(void);
void ConvertOp(void);
void ConvertRot13(void);
