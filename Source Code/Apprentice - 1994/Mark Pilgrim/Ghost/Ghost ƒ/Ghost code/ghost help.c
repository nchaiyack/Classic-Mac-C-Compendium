/**********************************************************************\

File:		ghost help.c

Purpose:	This module handles displaying the different help windows.


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

#include "ghost help.h"

void DrawHelp(int whichHelp)
{
	GrafPtr		curPort;
	
	GetPort(&curPort);
	EraseRect(&(curPort->portRect));

	MoveTo(10,15);
	TextFont(3);
	TextSize(9);
	switch (whichHelp)
	{
		case 0:
			DrawHelpPlay();
			break;
		case 1:
			DrawHelpOptions();
			break;
		case 2:
			DrawHelpCopyright();
			break;
		case 3:
			DrawHelpContact();
			break;
		case 4:
			DrawHelpFAQ();
			break;
	}
}

void DrawHelpPlay(void)
{
	DrawString("\pGhost is a classic word-building game for 2 or more players.");
	MoveTo(10,27);
	DrawString("\pPlayers take turns adding a letter to a word.  Whoever adds");
	MoveTo(10,39);
	DrawString("\pthe letter that finishes the word loses the round and gets a");
	MoveTo(10,51);
	DrawString("\pletter of the word “ghost” added to their score.  Once a player");
	MoveTo(10,63);
	DrawString("\ploses five rounds, they are a “ghost” and are out of the game!");
	MoveTo(10,87);
	DrawString("\pBluffing: if you don’t know what letter to add, try bluffing and");
	MoveTo(10,99);
	DrawString("\ppicking a letter that seems reasonable.  The current player");
	MoveTo(10,111);
	DrawString("\palways has the option to challenge (“Challenge word” under");
	MoveTo(10,123);
	DrawString("\pthe “Options” menu), and the dictionary will be consulted to");
	MoveTo(10,135);
	DrawString("\psee if any words exist that start with that series of letters.");
	MoveTo(10,147);
	DrawString("\pIf so, the player who placed the challenge loses the round;");
	MoveTo(10,159);
	DrawString("\pif not, the player who bluffed loses the round!");
	MoveTo(10,171);
	MoveTo(10,183);
	DrawString("\pHint: the computer players do occasionally bluff; don’t be");
	MoveTo(10,195);
	DrawString("\pafraid to challenge them!");
}

void DrawHelpOptions(void)
{
	DrawString("\pYou can play Ghost against as many as five computer players,");
	MoveTo(10,27);
	DrawString("\pwith as many as five human players as well.  Set the number");
	MoveTo(10,39);
	DrawString("\pof computer players under “Computer players” before you");
	MoveTo(10,51);
	DrawString("\pstart a new game.  (The human players each select their names");
	MoveTo(10,63);
	DrawString("\pand icons individually once you select “New game”.)");
	MoveTo(10,87);
	DrawString("\pThe computer players are all equally intelligent; you can");
	MoveTo(10,99);
	DrawString("\pcontrol this intelligence level under “Level of play” before");
	MoveTo(10,111);
	DrawString("\pthe game starts.  “Easy” means the computer players are");
	MoveTo(10,123);
	DrawString("\pfairly stupid; “difficult” means they’re, well, difficult.");
	MoveTo(10,147);
	DrawString("\pThe “Dictionary” menu controls both the dictionary which the");
	MoveTo(10,159);
	DrawString("\pcomputer players use to strategically select letters, and the");
	MoveTo(10,171);
	DrawString("\preference used to resolve challenges.  [Heed the names!  -MP]");
	MoveTo(10,195);
	DrawString("\p“Game speed” controls how fast the game progresses.");
}

void DrawHelpCopyright(void)
{
	DrawString("\pGhost is copyright ©1993 Mark Pilgrim.  This program is");
	MoveTo(10,27);
	DrawString("\pcompletely free; you can redistribute it and/or modify it");
	MoveTo(10,39);
	DrawString("\punder the terms of the GNU General Public License as published");
	MoveTo(10,51);
	DrawString("\pby the Free Software Foundation; either version 2 of the");
	MoveTo(10,63);
	DrawString("\pLicense, or (at your option) any later version.");
	MoveTo(10,87);
	DrawString("\pGhost is distributed in the hope that it will be useful,");
	MoveTo(10,99);
	DrawString("\pbut WITHOUT ANY WARRANTY; without even the implied");
	MoveTo(10,111);
	DrawString("\pwarranty of MERCHANTABILITY or FITNESS FOR A");
	MoveTo(10,123);
	DrawString("\pPARTICULAR PURPOSE.  See the GNU General Public License");
	MoveTo(10,135);
	DrawString("\pfor more details.");
	MoveTo(10,159);
	DrawString("\pYou should have received a copy of the GNU General Public");
	MoveTo(10,171);
	DrawString("\pLicense along with this program; if not, write to the Free");
	MoveTo(10,183);
	DrawString("\pSoftware Foundation, Inc., 675 Mass Ave, Cambridge, MA,");
	MoveTo(10,195);
	DrawString("\p02139, USA.");
}

void DrawHelpContact(void)
{
	DrawString("\pAll Macintosh users should feel free to contact me for any of");
	MoveTo(10,27);
	DrawString("\pthe following reasons: reporting bugs or suggestions about");
	MoveTo(10,39);
	DrawString("\pGhost or any other program I’ve written; discussing the");
	MoveTo(10,51);
	DrawString("\ppossibility of collaborating on future programming projects;");
	MoveTo(10,63);
	DrawString("\pdiscussing the virus I wrote and released (MBDF-A); discussing");
	MoveTo(10,75);
	DrawString("\pways to keep potential virus-writers from becoming actual");
	MoveTo(10,87);
	DrawString("\pvirus-writers; discussing techniques for really good backrubs.");
	MoveTo(10,111);
	DrawString("\pMy e-mail address is f8dy@netaxs.com.  To send me e-mail");
	MoveTo(10,123);
	DrawString("\panonymously, mail to an48382@anon.penet.fi.  (Due to the");
	MoveTo(10,135);
	DrawString("\pdouble-blind system, you will be allocated an anonymous I.D.");
	MoveTo(10,147);
	DrawString("\pwhich will keep your identity hidden.)  To send me encrypted");
	MoveTo(10,159);
	DrawString("\pmail, e-mail me requesting my PGP 2.2 public key.  If you do");
	MoveTo(10,171);
	DrawString("\pnot have e-mail access, my home address is 1130 Radnor Hill");
	MoveTo(10,183);
	DrawString("\pRoad, Wayne PA, 19087-2203, USA.  Obviously, it will take");
	MoveTo(10,195);
	DrawString("\pme much longer to reply to snail-mail; use it as a last resort.");
}

void DrawHelpFAQ(void)
{
	MoveTo(5,15);
	TextFace(bold);
	DrawString("\pWhy is my copy of Ghost registered to Bob?");
	MoveTo(10,27);
	TextFace(0);
	DrawString("\pThat’s the default name the program uses if you don’t enter");
	MoveTo(10,39);
	DrawString("\pyour name in the personalization dialog box.");
	MoveTo(5,63);
	TextFace(bold);
	DrawString("\pHow do I re-personalize my copy of Ghost?");
	MoveTo(10,75);
	TextFace(0);
	DrawString("\pDuplicate it or copy it to another disk, then run the copy.");
	MoveTo(10, 87);
	DrawString("\p(Or just trash “Ghost prefs” in your system folder.)");
	MoveTo(5,111);
	TextFace(bold);
	DrawString("\pWhy all the fuss over the MBDF-A virus?");
	MoveTo(10,123);
	TextFace(0);
	DrawString("\pI’m trying to put back into the Macintosh community as much");
	MoveTo(10,135);
	DrawString("\pas I took out of it.  When I reach the break-even point, I’ll");
	MoveTo(10,147);
	DrawString("\plet you know.");
	MoveTo(5,171);
	TextFace(bold);
	DrawString("\pHow does MSG enhance the flavor of my Macintosh?");
	MoveTo(10,183);
	TextFace(0);
	DrawString("\pWe try to think of things that no other Macintosh program");
	MoveTo(10,195);
	DrawString("\pdoes, and then we do them.  Any suggestions?");
}
