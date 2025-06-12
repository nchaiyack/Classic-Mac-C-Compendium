/**********************************************************************\

File:		msg sounds.h

Purpose:	This is the header file for msg sounds.c


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

#include <Sound.h>

#define NUM_SOUNDS		10

enum
{
	sound_keyclick = 1000,
	sound_playword_good,
	sound_badword,
	sound_wingame,
	sound_losegame,
	sound_virgin,
	sound_on,
	sound_aboutMSG,
	sound_fluff,
	sound_bluffing
};

extern	SndChannelPtr		myChannel;
extern	Handle				MySounds[NUM_SOUNDS];
extern	unsigned char		gSoundToggle;
extern	Boolean				gSoundAvailable;

OSErr OpenTheSoundDevice(void);
void CloseTheSoundDevice(void);
pascal Boolean ProcOFilter(DialogPtr dialog, EventRecord *event, short *item);
void InitSounds(void);
void DoSound(int);
void CloseSounds(void);
