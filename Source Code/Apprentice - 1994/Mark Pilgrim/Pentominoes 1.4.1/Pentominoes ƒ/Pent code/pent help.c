/**********************************************************************\

File:		pent help.c

Purpose:	This module handles the help windows.


Pentominoes - a 2-D geometry board game
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

#include "pent help.h"

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
			DrawHelpCopyright();
			break;
		case 2:
			DrawHelpContact();
			break;
		case 3:
			DrawHelpMath();
			break;
		case 4:
			DrawHelpFAQ();
			break;
	}
}

void DrawHelpPlay(void)
{
	DrawString("\pPentominoes is a game with a deceptively simple objective: fit");
	MoveTo(10,27);
	DrawString("\ptwelve different pieces on a game board with no spaces left");
	MoveTo(10,39);
	DrawString("\pover.  (The one exception to this rule is the 8 x 8 board minus");
	MoveTo(10,51);
	DrawString("\pany 4, where you may leave blank any four squares you wish.)");
	MoveTo(10,63);
	DrawString("\pPieces may be rotated and/or flipped when they are put on the");
	MoveTo(10,75);
	DrawString("\pboard, but they may not overlap each other.");
	MoveTo(10,99);
	DrawString("\pTo place a piece, highlight the squares on the board where the");
	MoveTo(10,111);
	DrawString("\ppiece should go, then either click on the picture of that piece,");
	MoveTo(10,123);
	DrawString("\ppress <RETURN>, press <ENTER>, or choose “Place Piece”");
	MoveTo(10,135);
	DrawString("\pfrom the “Fluff” menu.  You may use each piece only once.");
	MoveTo(10,159);
	DrawString("\pTo remove the pieces in the reverse order in which you placed");
	MoveTo(10,171);
	DrawString("\pthem, use “Undo”.  To remove a specific piece on the board,");
	MoveTo(10,183);
	DrawString("\pdouble-click on the piece on the board, or click on the piece");
	MoveTo(10,195);
	DrawString("\ponce and choose “Remove Piece” from the “Fluff” menu.");
}

void DrawHelpCopyright(void)
{
	DrawString("\pPentominoes is copyright ©1993 Mark Pilgrim.  This");
	MoveTo(10,27);
	DrawString("\pprogram is free; you can redistribute it and/or modify it");
	MoveTo(10,39);
	DrawString("\punder the terms of the GNU General Public License as published");
	MoveTo(10,51);
	DrawString("\pby the Free Software Foundation; either version 2 of the");
	MoveTo(10,63);
	DrawString("\pLicense, or (at your option) any later version.");
	MoveTo(10,87);
	DrawString("\pPentominoes is distributed in the hope that it will be useful,");
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
	DrawString("\pPentominoes or any other program I’ve written; discussing the");
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

void DrawHelpMath(void)
{
	DrawString("\pThe game of Pentominoes is based on an article by Martin");
	MoveTo(10,27);
	DrawString("\pGardner (see reference below), which in turn is based on");
	MoveTo(10,39);
	DrawString("\pearlier work by Solomon Golomb, Henry Dudeney, and others.");
	MoveTo(10,51);
	DrawString("\pGolomb explored not only pentominoes, but also other sizes");
	MoveTo(10,63);
	DrawString("\pof polyominoes, including tetrominoes, which make up the");
	MoveTo(10,75);
	DrawString("\pplaying pieces in the popular game, Tetris™.  Gardner’s book");
	MoveTo(10,87);
	DrawString("\pis good if you don’t care about (or can’t handle) the");
	MoveTo(10,99);
	DrawString("\pmathematical side of polyominoes.  Gardner discusses mainly");
	MoveTo(10,111);
	DrawString("\ppentominoes and hexominoes, and gives suggestions for other");
	MoveTo(10,123);
	DrawString("\ppentomino activities which are not part of this program.");
	MoveTo(10,147);
	DrawString("\pGardner, Martin, “Polyominoes,” Scientific American Book");
	MoveTo(10,159);
	DrawString("\p     of Mathematical Puzzles and Diversions, Simon and");
	MoveTo(10,171);
	DrawString("\p     Schuster, New York, 1959, chapter 13.");
	MoveTo(10,183);
	DrawString("\pGolomb, Solomon, Polyominoes, Charles Scribner’s Sons,");
	MoveTo(10,195);
	DrawString("\p     New York, 1965.");
	MoveTo(167,148);
	LineTo(278,148);
	MoveTo(25,160);
	LineTo(201,160);
	MoveTo(93,184);
	LineTo(147,184);
}

void DrawHelpFAQ(void)
{
	MoveTo(5,15);
	TextFace(bold);
	DrawString("\pWhy is my copy of Pentominoes registered to Bob?");
	MoveTo(10,27);
	TextFace(0);
	DrawString("\pThat’s the default name the program uses if you don’t enter");
	MoveTo(10,39);
	DrawString("\pyour name in the personalization dialog box.");
	MoveTo(5,63);
	TextFace(bold);
	DrawString("\pHow do I re-personalize my copy of Pentominoes?");
	MoveTo(10,75);
	TextFace(0);
	DrawString("\pDuplicate it or copy it to another disk, then run the copy.");
	MoveTo(10, 87);
	DrawString("\p(Or just trash “Pentominoes prefs” in your system folder.)");
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
