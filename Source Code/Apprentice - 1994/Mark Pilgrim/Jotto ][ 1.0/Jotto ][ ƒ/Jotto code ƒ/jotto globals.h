/**********************************************************************\

File:		jotto globals.h

Purpose:	This is the header file containing all Jotto-specific
			globals, enums, #defines, and structs.


Jotto ][ -=- a simple word game, revisited
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

enum
{
	allsWell=0,
	kNoDictionaries,
	kNoFiveLetterCustom,
	kNoSixLetterCustom,
	kNoCustomAtAll,
	kCantGetFiveLetterComputerWord,
	kCantGetSixLetterComputerWord,
	kCantGetFiveLetterHumanWord,
	kCantGetSixLetterHumanWord,
	kCantGetFiveLetterCustomWord,
	kCantGetSixLetterCustomWord,
	kCantSaveFiveLetterCustomWord,
	kCantSaveSixLetterCustomWord,
	kCantCreateGame,
	kCantOpenGameToSave,
	kCantWriteGame,
	kCantOpenGameToLoad,
	kCantLoadGame,
	kBadChecksum,
	kSaveVersionNotSupported,
	kNoMoreCustomWords
};

#define CREATOR			'Jot2'
#define CUSTOM_TYPE		'TEXT'
#define	SAVE_TYPE		'SvGm'
#define	SAVE_VERSION	1

extern	int				gComputerFile[2];
extern	int				gHumanFile[4];

extern	CIconHandle		gColorIcons[3];
extern	Handle			gBWIcons[3];
extern	PicHandle		gBackgroundPict;

extern	FSSpec			gameFile;
extern	Boolean			deleteTheThing;

#define MAX_TRIES 15

extern	char			gComputerWord[6];
extern	char			gHumanWord[MAX_TRIES+1][6];
extern	char			gNumRight[MAX_TRIES];
extern	int				gNumHumanWords[2];
extern	int				gNumComputerWords[2];
extern	unsigned char	gNumTries;
extern	unsigned char	gWhichChar;
extern	unsigned char	gNumLetters;

extern	char			gAllowDup;
extern	char			gNonWordsCount;
extern	char			gAnimation;
extern	Boolean			gFiveLetterOK;
extern	Boolean			gSixLetterOK;
extern	Boolean			gFiveLetterCustomOK;
extern	Boolean			gSixLetterCustomOK;
extern	Boolean			gFiveLetterCustomSaveOK;
extern	Boolean			gSixLetterCustomSaveOK;

extern	int				gWhichWipe;
extern	int				gLastWipe;

extern	Boolean			gIsEndGame;

#define NUMWIPES 12
