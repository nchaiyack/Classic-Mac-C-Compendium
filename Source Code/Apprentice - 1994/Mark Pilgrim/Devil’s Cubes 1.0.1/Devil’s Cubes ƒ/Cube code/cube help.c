/**********************************************************************\

File:		cube help.c

Purpose:	This module handles displaying the different help windows.


Devil’s Cubes -- a simple cubes puzzle
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

#include "cube help.h"

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
			DrawHelpUse();
			break;
		case 1:
			DrawHelpCopyright();
			break;
		case 2:
			DrawHelpContact();
			break;
		case 3:
			DrawHelpFAQ();
			break;
	}
}

void DrawHelpUse(void)
{
	DrawString("\pThe Devil’s Cubes are a set of four cubes, each painted with");
	MoveTo(10,27);
	DrawString("\pfour colors with some colors repeated.  All four cubes are");
	MoveTo(10,39);
	DrawString("\pdifferent.  The object of the game is to rotate the four cubes");
	MoveTo(10,51);
	DrawString("\puntil the top of each Devil’s Cube is a different color than");
	MoveTo(10,63);
	DrawString("\pthe top of the other three cubes, and likewise for the bottom,");
	MoveTo(10,75);
	DrawString("\pfront, and back of each Devil’s Cube.");
	MoveTo(10,99);
	DrawString("\pJust above the key map is a picture of each cube, showing the");
	MoveTo(10,111);
	DrawString("\ptop, front, and right sides.  Above that is a picture of each");
	MoveTo(10,123);
	DrawString("\pcube in a mirror, showing the left, back, and bottom sides.");
	MoveTo(10,135);
	DrawString("\pRemember that the left and right sides are irrelevant; only the");
	MoveTo(10,147);
	DrawString("\ptop, bottom, front, and back sides matter.  If you get confused,");
	MoveTo(10,159);
	DrawString("\pun-check “Show All Sides” under the Options menu to black");
	MoveTo(10,171);
	DrawString("\pout the left and right sides of each cube.");
	MoveTo(10,195);
	DrawString("\pUse the keys indicated under each cube to rotate that cube.");
}

void DrawHelpCopyright(void)
{
	DrawString("\pDevil’s Cubes is copyright ©1993 Mark Pilgrim.  This program");
	MoveTo(10,27);
	DrawString("\pis completely free; you can redistribute it and/or modify it");
	MoveTo(10,39);
	DrawString("\punder the terms of the GNU General Public License as published");
	MoveTo(10,51);
	DrawString("\pby the Free Software Foundation; either version 2 of the");
	MoveTo(10,63);
	DrawString("\pLicense, or (at your option) any later version.");
	MoveTo(10,87);
	DrawString("\pDevil’s Cubes is distributed in the hope that it will be useful,");
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
	DrawString("\pDevil’s Cubes or any other program I’ve written; discussing the");
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
	DrawString("\pWhy is my copy of Devil’s Cubes registered to Bob?");
	MoveTo(10,27);
	TextFace(0);
	DrawString("\pThat’s the default name the program uses if you don’t enter");
	MoveTo(10,39);
	DrawString("\pyour name in the personalization dialog box.");
	MoveTo(5,63);
	TextFace(bold);
	DrawString("\pHow do I re-personalize my copy of Devil’s Cubes?");
	MoveTo(10,75);
	TextFace(0);
	DrawString("\pDuplicate it or copy it to another disk, then run the copy.");
	MoveTo(10, 87);
	DrawString("\p(Or just trash “Devil’s Cubes prefs” in your system folder.)");
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
