// File "List.c"

#include <Icons.h>
#include "main.h"
#include "List.h"

extern WindowPtr gWindow;
extern ListHandle gList;

// * **************************************************************************** * //
// * **************************************************************************** * //

void CreateList() {
	short iconID;
	Cell theCell;
	Rect destRect, cellRect;
	GrafPtr savePort;
	Str63 textBuff;
	
	GetPort(&savePort);
	SetPort(gWindow);
	
	destRect = gWindow->portRect;
	destRect.right -= 15;
	destRect.bottom -= 15;
	
	SetRect(&cellRect, 0, 0, 1, 0);
	theCell.h = 120; theCell.v = 54;
	
	TextFont(1); TextSize(9); TextFace(bold);
	gList = LNew(&destRect, &cellRect, theCell, 131, gWindow, 0, -1, -1, -1);
	
	LAddRow(6, 0, gList);
	for(theCell.h = theCell.v = 0; theCell.v < (*gList)->dataBounds.bottom; theCell.v++) {
		iconID = 1001 + theCell.v;
		LSetCell(&iconID, sizeof(iconID), theCell, gList);
		LAddToCell("Cell ", sizeof("Cell "), theCell, gList);
		NumToString(theCell.v, textBuff);
		LAddToCell(textBuff+1, textBuff[0], theCell, gList);
		}
	LDoDraw(-1, gList);
	
	SetPort(savePort);
	}

// * **************************************************************************** * //
// * **************************************************************************** * //

void DisposeList() {

	LDispose(gList);
	}
