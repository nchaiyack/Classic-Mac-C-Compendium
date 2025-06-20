/**********************************************************************\

File:		v help.c

Purpose:	This module handles displaying the different help windows.


Voyeur -- a no-frills file viewer
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

#include "v help.h"

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
	DrawString("\pFirst, open a file (from the �File� menu, or, under system 7,");
	MoveTo(10,27);
	DrawString("\pdrag-and-drop a file onto Voyeur).  If this file has a data fork,");
	MoveTo(10,39);
	DrawString("\pthis fork will be opened; otherwise, the resource fork will be");
	MoveTo(10,51);
	DrawString("\popened.  Once a file is open, its type and creator can be");
	MoveTo(10,63);
	DrawString("\pmodified, or the current fork�s length can be altered.  Neither");
	MoveTo(10,75);
	DrawString("\pof these options is recommended unless you know what you�re");
	MoveTo(10,87);
	DrawString("\pdoing.  (Even then, only do it on a copy!)");
	MoveTo(10,111);
	DrawString("\pTo move around the file, use the options in the �Options�");
	MoveTo(10,123);
	DrawString("\pmenu, their keyboard equivalents, or the keyboard equivalents");
	MoveTo(10,135);
	DrawString("\pwithout the command key (i.e. �N� for the next block, �D�");
	MoveTo(10,147);
	DrawString("\pto view the data fork, etc).  The left and right arrows will");
	MoveTo(10,159);
	DrawString("\palso work to go to the previous and next blocks of file data.");
	MoveTo(10,171);
	DrawString("\p");
	MoveTo(10,183);
	DrawString("\pTo find text within the current fork, use �Find.�  To find the");
	MoveTo(10,195);
	DrawString("\psame text again, use �Find Again.�");
}

void DrawHelpCopyright(void)
{
	DrawString("\pVoyeur is copyright �1993 Mark Pilgrim.  This program is");
	MoveTo(10,27);
	DrawString("\pcompletely free; you can redistribute it and/or modify it");
	MoveTo(10,39);
	DrawString("\punder the terms of the GNU General Public License as published");
	MoveTo(10,51);
	DrawString("\pby the Free Software Foundation; either version 2 of the");
	MoveTo(10,63);
	DrawString("\pLicense, or (at your option) any later version.");
	MoveTo(10,87);
	DrawString("\pVoyeur is distributed in the hope that it will be useful,");
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
	DrawString("\pVoyeur or any other program I�ve written; discussing the");
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
	DrawString("\pWhy is my copy of Voyeur registered to Bob?");
	MoveTo(10,27);
	TextFace(0);
	DrawString("\pThat�s the default name the program uses if you don�t enter");
	MoveTo(10,39);
	DrawString("\pyour name in the personalization dialog box.");
	MoveTo(5,63);
	TextFace(bold);
	DrawString("\pHow do I re-personalize my copy of Voyeur?");
	MoveTo(10,75);
	TextFace(0);
	DrawString("\pDuplicate it or copy it to another disk, then run the copy.");
	MoveTo(10, 87);
	DrawString("\p(Or just trash �Voyeur prefs� in your system folder.)");
	MoveTo(5,111);
	TextFace(bold);
	DrawString("\pWhy all the fuss over the MBDF-A virus?");
	MoveTo(10,123);
	TextFace(0);
	DrawString("\pI�m trying to put back into the Macintosh community as much");
	MoveTo(10,135);
	DrawString("\pas I took out of it.  When I reach the break-even point, I�ll");
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
