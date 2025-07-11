/**********************************************************************\

File:		environment.h

Purpose:	This is the header file for environment.c

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

extern	Boolean			gHasAppleEvents;
extern	Boolean			gHasFSSpecs;
extern	Boolean			gStandardFile58;
extern	Boolean			gHasColorQD;
extern	Boolean			gHasPowerManager;
extern	Boolean			gHasNotificationManager;
extern	Boolean			gSystemSevenOrLater;
extern	Boolean			gWaitNextEventAvailable;
extern	Boolean			gHasGestalt;

extern	short			gForegroundWaitTime;
extern	short			gBackgroundWaitTime;
extern	Boolean			gIsInBackground;
extern	Boolean			gInProgress;
extern	Boolean			gDone;
extern	Boolean			gFrontWindowIsOurs;
extern	short			gFrontWindowIndex;
extern	Boolean			gIsVirgin;

Boolean InitTheEnvironment(void);
