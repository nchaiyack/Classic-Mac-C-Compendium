/**********************************************************************\

File:		program globals.h

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

#ifndef _PROGRAM_GLOBALS_H_
#define _PROGRAM_GLOBALS_H_

#define CREATOR			'Hlma'
#define APPLICATION_NAME "\pHalma"
#define	SAVE_TYPE		'SvGm'
#define	SAVE_VERSION	1

enum					/* window indices in gTheWindowData[] lists */
{
	kAbout=0,			/* about box */
	kAboutMSG,			/* "About MSG" splash screen */
	kHelp,				/* help window */
	kBoardSize,			/* choose board size window */
	kMainWindow			/* main graphics window */
};

#define	NUM_WINDOWS		5		/* total number of windows (see above enum) */

enum ErrorTypes
{
	allsWell=0,
	
	/* shell errors */
	kNoMemory,
	kNoMemoryAndQuitting,
	kProgramIntegrityNotVerified,
	kProgramIntegritySet,
	kSystemTooOld,
	userCancelErr,
	
	/* deBinHex errors */
	kCantOpenInputFile,
	kCantCreateTempFile,
	kBinHexErr,
	kDiskReadErr,
	kDiskWriteErr,
	kNeedMoreBinHexErr,

	/* program-specific errors */
	kCantCreateGame,
	kCantOpenGameToSave,
	kCantWriteGame,
	kCantOpenGameToLoad,
	kCantLoadGame,
	kBadChecksum,
	kSaveVersionNotSupported,
	kJumpOverNonPieceError,
	kNonPieceError,
	kCantGetThereFromHereError,
	kTooManyJumpsError
};

enum
{
	sliderID=128,
	pieceColorID=200,
	pieceBWID=201
};

enum
{
	kNoPiece=0,
	kPiece,
	kHighlightedPiece
};

#define ROW_LEFT	40
#define ROW_TOP		29
#define COL_LEFT	110
#define COL_TOP		29

extern	PicHandle		gSliderPict;
extern	CIconHandle		gPieceIconColor;
extern	Handle			gPieceIconBW;

extern	short			gNumRows;
extern	short			gNumColumns;
extern	short			Board[9][9];
extern	short			gNumMoves;
extern	short			gNumJumps;
extern	short			gThisStartRow, gThisStartColumn;
extern	short			gStickyButtonRow, gStickyButtonColumn;
extern	Str255			gThisJumpString;
extern	unsigned char	**gTheFullJumpHandle;

extern	unsigned char	gSnowSpeed;
extern	unsigned char	gSnowColor;
extern	unsigned char	gSnowMutates;

extern	FSSpec			gameFile;
extern	Boolean			deleteTheThing;

#endif
