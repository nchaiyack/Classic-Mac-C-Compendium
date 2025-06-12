/**********************************************************************\

File:		fix help.c

Purpose:	This module handles the help windows.


Menu Fixer -=- synchronize menu IDs and menu resource IDs
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

#include "fix help.h"

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
	}
}

void DrawHelpUse(void)
{
	DrawString("\pApple Computer, Inc., in their infinite wisdom, has set up the");
	MoveTo(10,27);
	DrawString("\pMENU resource to be as flexible as possible.  Each MENU");
	MoveTo(10,39);
	DrawString("\presource has a unique resource ID, like any resource, but");
	MoveTo(10,51);
	DrawString("\pthis resource ID actually has nothing to do with the menu ID");
	MoveTo(10,63);
	DrawString("\pby which you reference the menu.  The internal menu ID, within");
	MoveTo(10,75);
	DrawString("\pthe resource, is the ID the toolbox uses.  The problem is that");
	MoveTo(10,87);
	DrawString("\pthis is not automatically set to the resource ID!  If you are");
	MoveTo(10,99);
	DrawString("\pexpecting to reference your menus by the MENU resource IDs,");
	MoveTo(10,111);
	DrawString("\pyour menus may be scrambled, or they may not show up at all.");
	MoveTo(10,135);
	DrawString("\pMenu Fixer looks at each MENU resource in a file and checks");
	MoveTo(10,147);
	DrawString("\pthe resource ID against the internal menu ID for that MENU");
	MoveTo(10,159);
	DrawString("\presource.  If they don’t match, Menu Fixer will tell you this");
	MoveTo(10,171);
	DrawString("\pand offer to fix the problem.  NOTE: if you are not having a");
	MoveTo(10,183);
	DrawString("\pproblem with your menus, do not try Menu Fixer.  There is no");
	MoveTo(10,195);
	DrawString("\psense creating problems where there are none to begin with!");
}

void DrawHelpCopyright(void)
{
	DrawString("\pMenu Fixer is copyright ©1993 Mark Pilgrim.  This");
	MoveTo(10,27);
	DrawString("\pprogram is free; you can redistribute it and/or modify it");
	MoveTo(10,39);
	DrawString("\punder the terms of the GNU General Public License as published");
	MoveTo(10,51);
	DrawString("\pby the Free Software Foundation; either version 2 of the");
	MoveTo(10,63);
	DrawString("\pLicense, or (at your option) any later version.");
	MoveTo(10,87);
	DrawString("\pMenu Fixer is distributed in the hope that it will be useful,");
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
	DrawString("\pMenu Fixer or any other program I’ve written; discussing the");
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
