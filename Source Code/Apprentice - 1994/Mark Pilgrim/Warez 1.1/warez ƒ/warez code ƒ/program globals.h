/**********************************************************************\

File:		program globals.h

Purpose:	This is the header file containing all WAREZ-specific
			globals, enums, #defines, and structs.


WAREZ -=- nostalgia isn't what it used to be
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
	allsWell=0,
	kNoMemory,
	kCantOpenInputFile,
	kCantCreateTempFile,
	kCantOpenTempFile,
	kCantReadInputFile,
	kCantWriteTempFile
};

#define CREATOR			'K00L'
#define APPLICATION_NAME "\pWAREZ"
#define	SAVE_TYPE		'TEXT'

#define INPUT_BUFFER_MAX 10240
#define OUTPUT_BUFFER_MAX 10240

#define showThatTharProgress ((gShowProgress) && (gInputLength>INPUT_BUFFER_MAX) && (gInProgress))

extern	FSSpec			inputFS;
extern	FSSpec			outputFS;
extern	FSSpec			tempFS;
extern	Boolean			deleteTheThing;
extern	int				inputRefNum;
extern	int				outputRefNum;
extern	Boolean			gInWord;
extern	unsigned long	gInputOffset;
extern	unsigned long	gOutputOffset;
extern	Ptr				gInputBuffer;
extern	Ptr				gOutputBuffer;
extern	Boolean			gInputNeedsUpdate;
extern	Boolean			gOutputNeedsUpdate;
extern	unsigned long	gAbsoluteOffset;
extern	unsigned long	gInputLength;
extern	unsigned long	gWhatsReallyInInputBuffer;
extern	Boolean			gSeenBackslash;
extern	int				gCurlyLevel;

extern	unsigned char	gUseRTF;
extern	unsigned char	gShowSaveDialog;
extern	unsigned char	gAddSuffix;
extern	unsigned char	gShowProgress;
