/**********************************************************************\

File:		program globals.c

Purpose:	This is the header file for all the program-specific
			global variables, #defines, enums, and structs.

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

#define CREATOR		'MSG0'
#define APPLICATION_NAME "\pMSG Demo"

enum
{
	allsWell=0,
	kNoMemory=-1,
	kNoMemoryAndQuitting=-2,
	kProgramIntegrityNotVerified=-3,
	kProgramIntegritySet=-4,
	kSystemTooOld=-5,
	kCantOpenInputFile=-6,
	kCantCreateTempFile=-7,
	kBinHexErr=-8,
	kDiskReadErr=-9,
	kDiskWriteErr=-10,
	kEofErr=-11,
	kNeedMoreBinHexErr=-12,
	userCancelErr=-13
};

enum
{
	kEffectsOnly=0,
	kEffectsPlusReverseEffects,
	kFadesPlusEffects,
	kFadesPlusReverseEffects,
	kFullScreen
};

#define BINHEX_VALUES_STRING	128
#define BINHEX_HEADER_STRING	129

extern	unsigned char	gWhichPict;
extern	unsigned char	gIsReversed;
extern	int				gWhichWipe;
extern	int				gLastWipe;
extern	int				gWipeStatus;
