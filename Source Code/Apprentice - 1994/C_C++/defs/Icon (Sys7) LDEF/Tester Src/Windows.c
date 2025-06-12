// File "Windows.c"

#include "main.h"
#include "Windows.h"

extern WindowPtr gWindow;
extern ListHandle gList;

// * **************************************************************************** * //
// * **************************************************************************** * //

void DoUpdate() {
	Rect grayRect;
	GrafPtr savePort;
	
	GetPort(&savePort);
	SetPort(gWindow);
	BeginUpdate(gWindow);
	
	DrawGrowIcon(gWindow);
	grayRect = gWindow->portRect;
	grayRect.right -= 15;
	grayRect.bottom -= 15;
	FillRect(&grayRect, gray);
	
	LUpdate(gWindow->visRgn, gList);

	EndUpdate(gWindow);
	SetPort(savePort);
	}

// * **************************************************************************** * //
// * **************************************************************************** * //

void DoActivate(short active) {
	GrafPtr savePort;
	
	GetPort(&savePort);
	SetPort(gWindow);
	
	DrawGrowIcon(gWindow);
	LActivate(active, gList);
	
	SetPort(savePort);
	}
