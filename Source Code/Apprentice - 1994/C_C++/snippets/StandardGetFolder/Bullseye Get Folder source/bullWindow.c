/*****
 * bullWindow.c
 *
 *		The window routines for the Bullseye demo
 *
 *****/

#include "bullWindow.h"

WindowPtr	bullseyeWindow;
Rect		dragRect;
Rect		windowBounds = { 40, 40, 150, 150 };
Rect		circleStart = {10, 10, 100, 100};
int			width = 5;

/****
 * SetUpWindow()
 *
 *	Create the Bullseye window, and open it.
 *
 ****/

void SetUpWindow(void)

{
	dragRect = screenBits.bounds;
	
	bullseyeWindow = NewWindow(0L, &windowBounds, "\pBullseye", true, noGrowDocProc, (WindowPtr) -1L, true, 0);
	SetPort(bullseyeWindow);
}
/* end SetUpWindow */


/*****
 * DrawBullseye()
 *
 *	Draws the bullseye.
 *
 *****/

void DrawBullseye(short active)

{
	Rect	myRect;
	int		color = true;
	
	SetPort(bullseyeWindow);
	EraseRect(&circleStart);
	myRect = circleStart;
	
	while(myRect.left < myRect.right)
	  {
	  FillOval(&myRect, color ? (active ? black : gray) : white);
	  InsetRect(&myRect, width, width);
	  color = !color;
	  } 
}
/* end DrawBullseye */