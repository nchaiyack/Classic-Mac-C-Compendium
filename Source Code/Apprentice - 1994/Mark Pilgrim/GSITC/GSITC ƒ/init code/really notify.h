/**********************************************************************\

File:		really notify.h

Purpose:	This is the header file for really notify.c


Go Sit In The Corner -=- not you, just the cursor
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

#define		STARTUP_ERROR_STR		128		/* Preload */
#define		RESPONSE_NMRP			128		/* System Heap, Preload */

void StartupError(int errorcode);
void StartupGood(PrefHandle cdevStorage);
int SetupNM(StringPtr str);
