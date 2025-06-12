/**********************************************************************\

File:		jotto help.c

Purpose:	This module handles displaying the different help windows.


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

#include "jotto help.h"

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
	DrawString("\pJotto ][ is a simple word game based on logic and frustration.");
	MoveTo(10,27);
	DrawString("\pThe computer will pick a 5- or 6-letter word from its 2000+");
	MoveTo(10,39);
	DrawString("\pword database, and you have fifteen tries to guess it.  If");
	MoveTo(10,51);
	DrawString("\pyour guess is not right, the computer will tell you how many");
	MoveTo(10,63);
	DrawString("\pletters from your guess are in the actual word.  However, all");
	MoveTo(10,75);
	DrawString("\pyour guesses have to be valid words.");
	MoveTo(10,99);
	DrawString("\pThe dictionary is based on the Official Scrabble® Players");
	MoveTo(10,111);
	DrawString("\pDictionary.  You can guess any of the 8000+ words in the");
	MoveTo(10,123);
	DrawString("\pdictionary; however, the computer will only pick from a");
	MoveTo(10,135);
	DrawString("\psubset of the words that I’ve designated as “common” words.");
	MoveTo(10,147);
	DrawString("\pThis means that, while you are free to guess “aalii” or");
	MoveTo(10,159);
	DrawString("\p“aaliis,” the computer will never force you to do so.");
	MoveTo(10,171);
	MoveTo(10,183);
	DrawString("\pThe original version of Jotto was programmed from scratch in");
	MoveTo(10,195);
	DrawString("\pless than 24 hours straight, in the spirit of B&D Productions.");
}

void DrawHelpOptions(void)
{
	DrawString("\pChoose either “5-letter words” or “6-letter words” from");
	MoveTo(10,27);
	DrawString("\pthe “Options” menu.  Start a new game with “New game,” or");
	MoveTo(10,39);
	DrawString("\pselect “Open...” to load a saved game from disk.  Type your");
	MoveTo(10,51);
	DrawString("\pguess and press <RETURN> or <ENTER>.  Your guess will appear");
	MoveTo(10,63);
	DrawString("\pin the notebook with the number of correct letters it contains.");
	MoveTo(10,87);
	DrawString("\pIf you choose “Allow duplicate letters” under the “Options”");
	MoveTo(10,99);
	DrawString("\pmenu, the word the computer picks may contain more than one");
	MoveTo(10,111);
	DrawString("\pof the same letter.  If you choose “Non-words count against");
	MoveTo(10,123);
	DrawString("\pyou,” any words you guess that are not in the computer’s");
	MoveTo(10,135);
	DrawString("\pdatabase will count as one of your fifteen tries.  Both of");
	MoveTo(10,147);
	DrawString("\pthese options must be set before the game starts; if you try");
	MoveTo(10,159);
	DrawString("\pto set them during a game, you will be rebuffed.");
	MoveTo(10,183);
	DrawString("\pThe “Sound” and “Animation” options may be changed at any");
	MoveTo(10,195);
	DrawString("\ptime before, during, or after a game.");
}

void DrawHelpCopyright(void)
{
	DrawString("\pJotto ][ is copyright ©1993 Mark Pilgrim.  This program is");
	MoveTo(10,27);
	DrawString("\pcompletely free; you can redistribute it and/or modify it");
	MoveTo(10,39);
	DrawString("\punder the terms of the GNU General Public License as published");
	MoveTo(10,51);
	DrawString("\pby the Free Software Foundation; either version 2 of the");
	MoveTo(10,63);
	DrawString("\pLicense, or (at your option) any later version.");
	MoveTo(10,87);
	DrawString("\pJotto ][ is distributed in the hope that it will be useful,");
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
	DrawString("\pJotto ][ or any other program I’ve written; discussing the");
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
	DrawString("\pWhy is my copy of Jotto ][ registered to Bob?");
	MoveTo(10,27);
	TextFace(0);
	DrawString("\pThat’s the default name the program uses if you don’t enter");
	MoveTo(10,39);
	DrawString("\pyour name in the personalization dialog box.");
	MoveTo(5,63);
	TextFace(bold);
	DrawString("\pHow do I re-personalize my copy of Jotto ][?");
	MoveTo(10,75);
	TextFace(0);
	DrawString("\pDuplicate it or copy it to another disk, then run the copy.");
	MoveTo(10, 87);
	DrawString("\p(Or just trash “Jotto ][ prefs” in your system folder.)");
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
