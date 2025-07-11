/**********************************************************************\

File:		ghost graphics.c

Purpose:	This module handles drawing the Ghost playing window.



Ghost -=- a classic word-building challenge
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

#include "ghost globals.h"
#include "ghost graphics.h"
#include "msg environment.h"

void DrawBoardColor(void)
{
	RGBColor	oldForeColor, oldBackColor;
	GrafPtr		curPort;
	Rect		space;
	int			i,j;
	Boolean		ghost;
	
	GetForeColor(&oldForeColor);
	GetBackColor(&oldBackColor);
	
	GetPort(&curPort);
	EraseRect(&(curPort->portRect));
	
	if (gShowMessageBox)
		DrawMessage();
	
	TextFont(geneva);
	TextSize(9);
	
	SetRect(&space, 40, 10, 72, 42);
	for (i=0; i<((gNumPlayers<5) ? gNumPlayers : 5); i++)
	{
		if (gPlayOrderIndex[i]>=gNumHumanPlayers)
		{
			PlotCIcon(&space, gColorIcons[gComputerIconIndex[gPlayOrderIndex[i]-gNumHumanPlayers]]);
			ghost=(gComputerPlayerScore[gPlayOrderIndex[i]-gNumHumanPlayers]==5);
		}
		else
		{
			PlotCIcon(&space, gColorIcons[gHumanIconIndex[gPlayOrderIndex[i]]]);
			ghost=(gHumanPlayerScore[gPlayOrderIndex[i]]==5);
		}

		DrawNameAndScore(i, gPlayOrderIndex[i], ghost);
				
		space.left+=72;
		space.right+=72;
	}

	if (gNumPlayers>5)
	{
		SetRect(&space, 40, 76, 72, 108);
		for (i=5; i<gNumPlayers; i++)
		{
			if (gPlayOrderIndex[i]>=gNumHumanPlayers)
			{
				PlotCIcon(&space, gColorIcons[gComputerIconIndex[gPlayOrderIndex[i]-gNumHumanPlayers]]);
				ghost=(gComputerPlayerScore[gPlayOrderIndex[i]-gNumHumanPlayers]==5);
			}
			else
			{
				PlotCIcon(&space, gColorIcons[gHumanIconIndex[gPlayOrderIndex[i]]]);
				ghost=(gHumanPlayerScore[gPlayOrderIndex[i]]==5);
			}
			DrawNameAndScore(i, gPlayOrderIndex[i], ghost);
			
			space.left+=72;
			space.right+=72;
		}
	}
	
	if (gTheWord[0]!=0x00)
	{
		TextFont(10119);
		TextFace(0);
		TextSize(48);
		MoveTo(200-(StringWidth(gTheWord)/2), 230);
		DrawString(gTheWord);
	}
	
	RGBForeColor(&oldForeColor);
	RGBBackColor(&oldBackColor);
}

void DrawBoardBW(void)
{
	GrafPtr		curPort;
	Rect		space;
	int			i,j;
	Boolean		ghost;
	
	GetPort(&curPort);
	
	EraseRect(&(curPort->portRect));

	if (gShowMessageBox)
		DrawMessage();
	
	TextFont(geneva);
	TextSize(9);
	
	SetRect(&space, 40, 10, 72, 42);
	for (i=0; i<((gNumPlayers<5) ? gNumPlayers : 5); i++)
	{
		if (gPlayOrderIndex[i]>=gNumHumanPlayers)
		{
			PlotIcon(&space, gBWIcons[gComputerIconIndex[gPlayOrderIndex[i]-gNumHumanPlayers]]);
			ghost=(gComputerPlayerScore[gPlayOrderIndex[i]-gNumHumanPlayers]==5);
		}
		else
		{
			PlotIcon(&space, gBWIcons[gHumanIconIndex[gPlayOrderIndex[i]]]);
			ghost=(gHumanPlayerScore[gPlayOrderIndex[i]]==5);
		}

		DrawNameAndScore(i, gPlayOrderIndex[i], ghost);
				
		space.left+=72;
		space.right+=72;
	}

	if (gNumPlayers>5)
	{
		SetRect(&space, 40, 76, 72, 108);
		for (i=5; i<gNumPlayers; i++)
		{
			if (gPlayOrderIndex[i]>=gNumHumanPlayers)
			{
				PlotIcon(&space, gBWIcons[gComputerIconIndex[gPlayOrderIndex[i]-gNumHumanPlayers]]);
				ghost=(gComputerPlayerScore[gPlayOrderIndex[i]-gNumHumanPlayers]==5);
			}
			else
			{
				PlotIcon(&space, gBWIcons[gHumanIconIndex[gPlayOrderIndex[i]]]);
				ghost=(gHumanPlayerScore[gPlayOrderIndex[i]]==5);
			}
			DrawNameAndScore(i, gPlayOrderIndex[i], ghost);
			
			space.left+=72;
			space.right+=72;
		}
	}
	
	if (gTheWord[0]!=0x00)
	{
		TextFont(10119);
		TextFace(0);
		TextSize(48);
		MoveTo(200-(StringWidth(gTheWord)/2), 230);
		DrawString(gTheWord);
	}
}

void DrawNameAndScore(int i, int index, Boolean ghost)
{
	int				cx,y;
	Str255			tempStr;
	int				j;
	
	tempStr[0]=0x00;
	
	cx=56+(i%5)*72;
	y=52+((i>=5) ? 66 : 0);
	
	if (index>=gNumHumanPlayers)
		for (j=((char*)(*(gIconNames[gComputerIconIndex[index-gNumHumanPlayers]])))[0]; j>=0; j--)
			tempStr[j]=((char*)(*(gIconNames[gComputerIconIndex[index-gNumHumanPlayers]])))[j];
	else
		for (j=((char*)(*(gHumanName[index])))[0]; j>=0; j--)
			tempStr[j]=((char*)(*(gHumanName[index])))[j];

	if (ghost)
	{
		tempStr[0]+=2;
		for (j=tempStr[0]-1; j>1; j--)
			tempStr[j]=tempStr[j-1];
		tempStr[1]='(';
		tempStr[tempStr[0]]=')';
	}
	
	MoveTo(cx-(StringWidth(tempStr)/2), y);
	DrawString(tempStr);

	y+=12;
	TextFace(bold);

	tempStr[1]='G';	tempStr[2]='H';	tempStr[3]='O';	tempStr[4]='S';	tempStr[5]='T';
	if (index>=gNumHumanPlayers)
		tempStr[0]=gComputerPlayerScore[index-gNumHumanPlayers];
	else
		tempStr[0]=gHumanPlayerScore[index];
	
	MoveTo(cx-(StringWidth(tempStr)/2), y);
	DrawString(tempStr);
	TextFace(0);
}

void DrawMessage(void)
{
	Rect			theRect;
	int				j;
	int				index;
	int				iconIndex;
	
	SetRect(&theRect, 19, 142, 381, 161);
	EraseRect(&theRect);
	FrameRect(&theRect);
	SetRect(&theRect, 22, 161, 383, 163);
	FillRect(&theRect, black);
	SetRect(&theRect, 381, 144, 383, 161);
	FillRect(&theRect, black);

	if (gStatus!=kNewPlayer)
	{
		gTheMessage[0]=0x00;
		if (gStatus==kWonChallenge)
			AddToMessage("\pThe dictionary search came up empty!");
		else if (gStatus==kLostChallenge)
			AddToMessage("\pThe dictionary search came up with this word:");
		else
		{
			index=gPlayOrderIndex[gCurrentPlayer];
			if (index>=gNumHumanPlayers)
			{
				for (j=((char*)(*(gIconNames[gComputerIconIndex[index-gNumHumanPlayers]])))[0]; j>=0; j--)
					gTheMessage[j]=((char*)(*(gIconNames[gComputerIconIndex[index-gNumHumanPlayers]])))[j];
				if (!gInProgress)
					AddToMessage("\p is the winner!");
				else if (gStatus==kIsChallenging)
					AddToMessage("\p challenges -- consulting dictionary...");
				else if (gStatus==kJustLost)
					AddToMessage("\p is out of the game!");
				else if (gStatus==kJustGotALetter)
				{
					switch (gComputerPlayerScore[index-gNumHumanPlayers])
					{
						case 0x01:	AddToMessage("\p gets a �G� !"); break;
						case 0x02:	AddToMessage("\p gets an �H� !"); break;
						case 0x03:	AddToMessage("\p gets an �O� !"); break;
						case 0x04:	AddToMessage("\p gets an �S� !"); break;
						case 0x05:	AddToMessage("\p is out of the game!"); break;
					}
				}
				else
				{
					if (gTheWord[0]==0x00)
					{
						AddToMessage("\p is plotting ");
						iconIndex=gComputerIconIndex[index-gNumHumanPlayers];
						if ((iconIndex==0) || (iconIndex==2) || (iconIndex==22))
							AddToMessage("\pher");
						else
							AddToMessage("\phis");
						AddToMessage("\p first move...");
					}
					else
						AddToMessage("\p is thinking...");
				}
			}
			else
			{
				for (j=((char*)(*(gHumanName[index])))[0]; j>=0; j--)
					gTheMessage[j]=((char*)(*(gHumanName[index])))[j];
				if (!gInProgress)
					AddToMessage("\p is the winner!");
				else if (gStatus==kIsChallenging)
					AddToMessage("\p challenges -- consulting dictionary...");
				else if (gStatus==kJustLost)
					AddToMessage("\p is out of the game!");
				else if (gStatus==kJustGotALetter)
				{
					switch (gHumanPlayerScore[index])
					{
						case 0x01:	AddToMessage("\p gets a �G� !"); break;
						case 0x02:	AddToMessage("\p gets an �H� !"); break;
						case 0x03:	AddToMessage("\p gets an �O� !"); break;
						case 0x04:	AddToMessage("\p gets an �S� !"); break;
						case 0x05:	AddToMessage("\p is out of the game!"); break;
					}
				}
				else
				{
					if (gTheWord[0]==0x00)
						AddToMessage("\p, please type a letter to start a new word.");
					else
						AddToMessage("\p, please add a letter to this word.");
				}
			}
		}
	}
	
	TextFont(geneva);
	TextSize(9);
	MoveTo(200-StringWidth(gTheMessage)/2, 155);
	DrawString(gTheMessage);
}

void AddToMessage(Str255 theString)
{
	int				i;
	
	for (i=1; i<=theString[0]; i++)
		gTheMessage[++gTheMessage[0]]=theString[i];
}
