/**********************************************************************\

File:		sounds.h

Purpose:	This is the header file for sounds.c

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

#define NUM_SOUNDS		8

enum
{
	sound_aboutMSG = 1000,
	sound_fluff,
	sound_on,
	sound_virgin,
	sound_click,
	sound_endgame,
	sound_buzz,
	sound_startgame
};

extern	SndChannelPtr		myChannel;
extern	Handle				MySounds[NUM_SOUNDS];
extern	Boolean				gSoundToggle;
extern	Boolean				gSoundAvailable;
extern	Boolean				gSoundIsFinishedPlaying;

void InitTheSounds(void);
void DoSound(short whichSound, Boolean async);
void CloseTheSoundChannel(void);
