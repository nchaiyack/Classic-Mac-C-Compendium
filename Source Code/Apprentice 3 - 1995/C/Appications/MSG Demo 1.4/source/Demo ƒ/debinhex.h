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

#define BINHEX_BUFFER_LENGTH	5120

int InitDeBinHex(void);
void EndDeBinHex(void);
int GetNBytes(int inputRefNum, Ptr outBuf, unsigned long n);
int FlushBuffer(unsigned char *outputChar);
int GetByteFromBuffer(unsigned char *outputChar);
int ExtractByteFromBinHex(unsigned char *outputChar);
int BinHexChecksum(Ptr buffer, unsigned long len, Boolean StartOver);
void CalcCRC(unsigned int *crc, unsigned int v);
