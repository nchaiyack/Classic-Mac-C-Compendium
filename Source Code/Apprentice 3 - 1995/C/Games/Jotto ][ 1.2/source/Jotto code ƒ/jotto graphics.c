#include "jotto graphics.h"
#include "program globals.h"
#include "util.h"
#include "window layer.h"

void DrawWordInList(WindowPtr theWindow)
{
	Str255			tempStr;

	TextFont(monaco);
	TextSize(9);
	TextMode(srcXor);
	
	MoveTo(20+(GetWindowWidth(theWindow))/2-4*CharWidth(' '),105+gNumTries*12);
	Mymemcpy((Ptr)((long)tempStr+1), gHumanWord[gNumTries], gNumLetters);
	tempStr[0]=gNumLetters;
	DrawString(tempStr);
	DrawString("\p    ");
	NumToString(gNumRight[gNumTries], tempStr);
	DrawChar(tempStr[1]);
}

void DrawOneChar(WindowPtr theWindow)
{
	Rect			theRect;
	char			thisChar;
	
	TextFont(144);
	TextSize(24);
	TextMode(srcXor);
	theRect.top=32;
	theRect.bottom=theRect.top+36;
	theRect.left=(GetWindowWidth(theWindow))/2-98-((gNumLetters==6) ? 20 : 0)+40*gWhichChar;
	theRect.right=theRect.left+36;
	FillRect(&theRect, &qd.black);
	thisChar=gHumanWord[gNumTries][gWhichChar];
	MoveTo((GetWindowWidth(theWindow))/2-80-((gNumLetters==6) ? 20 : 0)+
		gWhichChar*40-CharWidth(thisChar)/2, (thisChar=='Q') ? 59 : 61);
	DrawChar(thisChar);
}

void DrawCurrentWord(WindowPtr theWindow)
{
	short			j;
	Rect			theRect;
	char			thisChar;

	TextFont(144);
	TextSize(24);
	TextMode(srcXor);
	theRect.top=31;
	theRect.bottom=theRect.top+38;
	theRect.left=(GetWindowWidth(theWindow))/2-99-((gNumLetters==6) ? 20 : 0);
	theRect.right=theRect.left+38;
	for (j=0; j<gNumLetters; j++)
	{
		FillRect(&theRect, &qd.black);
		ForeColor(whiteColor);
		FrameRect(&theRect);
		ForeColor(blackColor);
		theRect.left+=40;
		theRect.right+=40;
	}
	
	for (j=0; j<gNumLetters; j++)
	{
		thisChar=gHumanWord[gNumTries][j];
		MoveTo((GetWindowWidth(theWindow))/2-80-((gNumLetters==6) ? 20 : 0)+j*40-CharWidth(thisChar)/2,
			(thisChar=='Q') ? 59 : 61);
		DrawChar(thisChar);
	}
	TextFont(monaco);
	TextSize(9);
}

void HighlightChar(WindowPtr theWindow)
{
	Rect			theRect;
	
	theRect.top=32;
	theRect.bottom=theRect.top+36;
	theRect.left=(GetWindowWidth(theWindow))/2-98-((gNumLetters==6) ? 20 : 0)+40*gWhichChar;
	theRect.right=theRect.left+36;
	InvertRect(&theRect);
}
