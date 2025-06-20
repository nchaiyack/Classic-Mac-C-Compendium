////////////
//
//	CChessInfoPane.cp
//
//	The ChessInfoPane Class
//
//	SUPERCLASS = CPane
//	Copyright � 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include "CChessInfoPane.h"
#include "CChessOptionsDialog.h"

void	CChessInfoPane::Draw(Rect *area)
{
	Str255	easyGameInfoString = "\pOn this setting, the computer takes about 5 to 30 seconds per move.",
			hardGameInfoString = "\pOn this setting, the computer takes about 1 to 6 minutes per move.",
			wayHardGameInfoString = "\pOn this setting, the computer takes about 10 to 60 minutes per move.",
			soundOnBetterMovesString = "\pWith this box checked, the program will beep every time it finds a better move.",
			showThoughtsString = "\pWith this box checked, the program will highlight the piece it is analyzing.",
			doReallyCoolThingsString = "\pWith this box checked, a back-propagated analysis of all moves will be stored in an internal library.";
	Rect	aRect;
	
	aRect.top = aRect.left = 0;
	aRect.right = width;
	aRect.bottom = height;

	FrameRect(&aRect);
	InsetRect(&aRect,1,1);

	switch (infoString) {
		case kEasyGame:
			TextBox(&easyGameInfoString[1],easyGameInfoString[0],&aRect,1);
			break;
		case kHardGame:
			TextBox(&hardGameInfoString[1],hardGameInfoString[0],&aRect,1);
			break;
		case kWayHardGame:
			TextBox(&wayHardGameInfoString[1],wayHardGameInfoString[0],&aRect,1);
			break;
		case kSoundOnBetterMoves:
			TextBox(&soundOnBetterMovesString[1],soundOnBetterMovesString[0],&aRect,1);
			break;
		case kShowThoughts:
			TextBox(&showThoughtsString[1],showThoughtsString[0],&aRect,1);
			break;
		case kDoReallyCoolThings:
			TextBox(&doReallyCoolThingsString[1],doReallyCoolThingsString[0],&aRect,1);
			break;
		default:
			EraseRect(&aRect);
			break;
	}		
}