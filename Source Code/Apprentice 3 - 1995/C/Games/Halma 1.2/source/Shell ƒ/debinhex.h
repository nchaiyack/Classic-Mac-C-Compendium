/**********************************************************************\

File:		debinhex.h

Purpose:	This is the header file for debinhex.c.

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

#include "program globals.h"

#define BINHEX_BUFFER_LENGTH	5120
#define BINHEX_VALUES_STRING	128
#define BINHEX_HEADER_STRING	129

void InitDeBinHex(void);
void EndDeBinHex(void);
enum ErrorTypes GetNBytes(short inputRefNum, Ptr outBuf, unsigned long n);
enum ErrorTypes FlushBuffer(unsigned char *outputChar);
enum ErrorTypes GetByteFromBuffer(unsigned char *outputChar);
enum ErrorTypes ExtractByteFromBinHex(unsigned char *outputChar);
short BinHexChecksum(Ptr buffer, unsigned long len, Boolean StartOver);
void CalcCRC(unsigned short *crc, unsigned short v);
