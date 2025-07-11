/**********************************************************************\

File:		file management.h

Purpose:	This is the header file for file management.c.

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

extern	unsigned long	theFileType, theFileCreator;
extern	short			theFileFlags;
extern	FSSpec			inputFS, outputFS, tempFS;
extern	Boolean			deleteBinHexFile;
extern	short			inputRefNum, outputDFRefNum, outputRFRefNum;
extern	unsigned long	outputDFeof, outputRFeof;

void InitFiles(void);
enum ErrorTypes OpenInputFile(void);
enum ErrorTypes CreateTempFile(void);
enum ErrorTypes SetupTempFile(void);
void FinalizeFiles(Boolean good);
enum ErrorTypes ReadInputFile(short thisFile, Ptr buffer, unsigned long count,
	unsigned long *actualCount);
enum ErrorTypes WriteTempFile(short thatFile, Ptr buffer, unsigned long theLength);
