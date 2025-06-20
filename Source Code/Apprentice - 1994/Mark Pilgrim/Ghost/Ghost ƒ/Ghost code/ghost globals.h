/**********************************************************************\

File:		ghost globals.h

Purpose:	This is the header file for all the Ghost-specific global
			variables and constants


Ghost -=- a classic word-building challenge
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
	kNoIndex,
	kNoDictionaries,
	kCantFindSmallDict,
	kCantReadSmallDict,
	kSmallDictDamaged,
	kCantFindLargeDict,
	kCantReadLargeDict,
	kLargeDictDamaged,
	kNoMemory,
	kCantCreateGame,
	kCantOpenGameToSave,
	kCantWriteGame,
	kCantOpenGameToLoad,
	kCantLoadGame,
	kBadChecksum,
	kSaveVersionNotSupported
};

enum
{
	kPartial=0,
	kFull
};

#define CREATOR			'GH�T'
#define	SAVE_TYPE		'Ggme'
#define	SAVE_VERSION	1

extern	FSSpec			gameFile;
extern	Boolean			deleteTheThing;

extern	CIconHandle		gColorIcons[24];
extern	Handle			gBWIcons[24];

extern	Ptr				gTheDictionary[2];
extern	long			gIndex[2][27];

extern	int				gNumComputerPlayers;
extern	StringHandle	gIconNames[24];
extern	int				gComputerIconIndex[5];
extern	int				gNumHumanPlayers;
extern	StringHandle	gHumanName[5];
extern	int				gHumanIconIndex[5];

extern	int				gActualHumanPlayers, gActualComputerPlayers;
extern	int				gNumPlayers;
extern	int				gPlayOrderIndex[10];

extern	int				gComputerPlayerScore[5];
extern	int				gHumanPlayerScore[5];

extern	Str255			gTheWord;
extern	Str255			gTheMessage;
extern	int				gCurrentPlayer;

extern	unsigned char	gShowMessageBox;
extern	unsigned char	gGameSpeed;
extern	unsigned char	gComputerIntelligence;
extern	unsigned char	gUseFullDictionary;

extern	int				gStatus;

enum
{
	kNoStatus=0,
	kJustGotALetter,
	kJustLost,
	kIsChallenging,
	kLostChallenge,
	kWonChallenge,
	kNewPlayer
};
