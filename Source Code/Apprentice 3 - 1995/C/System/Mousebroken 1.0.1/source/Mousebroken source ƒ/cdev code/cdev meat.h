/**********************************************************************\

File:		cdev meat.h

Purpose:	This is the header file for cdev meat.c


Mousebroken -=- your computer isn't truly broken until it's mousebroken
Copyright (C) 1993 Mark Pilgrim

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

void RemoveTheMouseVBL(PrefHandle cdevStorage);
int InstallTheMouseVBL(PrefHandle cdevStorage);
int GetModuleInfo(PrefHandle cdevStorage);
int GetModuleInfoFromIndex(PrefHandle cdevStorage, int direction);
Boolean AtLeastOneModule(int vRefNum, long dirID);
int FindCurrentModule(PrefHandle cdevStorage, int vRefNum, long dirID, int numFiles);
int OpenTheModule(PrefHandle cdevStorage);
int CloseTheModule(PrefHandle cdevStorage);
