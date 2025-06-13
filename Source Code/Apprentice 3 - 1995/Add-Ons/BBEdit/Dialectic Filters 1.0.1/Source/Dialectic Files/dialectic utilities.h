/**********************************************************************\

File:		dialect utilities.h

Purpose:	This is the header file for dialect utilities.c


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

Boolean IsAllCaps(Str255 theWord);
unsigned char GetRestOfWord(Str255 theWord);
Boolean IsAlpha(char thisChar);
Boolean IsUpperAlpha(char thisChar);
Boolean IsNumeric(char thisChar);
Boolean IsVowel(char thisChar);
Boolean IsConsonant(char thisChar);
char ThisChar(void);					/* current char */
char NextChar(int howmany);				/* howmany-th char after current char */
void StoreChar(char thisChar);			/* appends thisChar to output */
void StoreString(Str255 thisString);	/* appends thisString to output */
void InputPlus(int howmany);			/* increments input offset by howmany */
