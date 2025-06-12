/**********************************************************************\

File:		pent.c

Purpose:	This module handles game initialization/shutdown and
			mouse and key events during a game.


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

#include "pent.h"
#include "pent setup.h"
#include "pent meat.h"
#include "pent load-save.h"
#include "pent generic open.h"
#include "pent files.h"
#include "msg environment.h"
#include "msg sounds.h"
#include "msg graphics.h"
#include "msg menus.h"
#include "msg dialogs.h"

CIconHandle		gColorIcons[12];
Handle			gBWIcons[12];
PixPatHandle	gPlainColors[15];
PatHandle		gBWPatterns[15];
int				gShapes[12][5][5];

char			SolutionLetters[14];
int				gWhichBoard;
int				gNumHilited;
int				Board[8][20];
int				gNumRows;
int				gNumCols;
Boolean			gShowGrid;
Boolean			PieceUsed[12];
int				gNumPlayed;
int				PiecesPlayed[12];

long			oldTicks;
int				gCurrentColor;
Boolean			freshClick;

void InitGame(void)
{
	int			i,j,k;
	AppFile		myFile;
	
	SolutionLetters[0]=' ';
	SolutionLetters[1]='A';
	SolutionLetters[2]='H';
	SolutionLetters[3]='D';
	SolutionLetters[4]='B';
	SolutionLetters[5]='L';
	SolutionLetters[6]='F';
	SolutionLetters[7]='G';
	SolutionLetters[8]='C';
	SolutionLetters[9]='J';
	SolutionLetters[10]='K';
	SolutionLetters[11]='E';
	SolutionLetters[12]='I';
	SolutionLetters[13]='Z';
	
	gHelpHeight=200;
	gHelpWidth=300;
	
	for (i=0; i<12; i++)
	{
		if (gHasColorQD)
			gColorIcons[i]=GetCIcon(128+i);
		gBWIcons[i]=GetIcon(128+i);
	}
	for (i=0; i<15; i++)
	{
		if (gHasColorQD)
			gPlainColors[i]=GetPixPat(141+i);
		gBWPatterns[i]=GetPattern(126+i);
	}
	for (i=0; i<12; i++)
		for (j=0; j<5; j++)
			for (k=0; k<5; k++)
				gShapes[i][j][k]=0;
	gShapes[0][0][0]=gShapes[0][1][0]=gShapes[0][2][0]=gShapes[0][3][0]=gShapes[0][4][0]=1;
	gShapes[1][0][0]=gShapes[1][1][0]=gShapes[1][2][0]=gShapes[1][3][0]=gShapes[1][3][1]=1;
	gShapes[2][0][0]=gShapes[2][1][0]=gShapes[2][2][0]=gShapes[2][2][1]=gShapes[2][2][2]=1;
	gShapes[3][0][1]=gShapes[3][1][0]=gShapes[3][1][1]=gShapes[3][1][2]=gShapes[3][2][1]=1;
	gShapes[4][0][0]=gShapes[4][0][1]=gShapes[4][1][0]=gShapes[4][1][1]=gShapes[4][1][2]=1;
	gShapes[5][0][0]=gShapes[5][1][0]=gShapes[5][1][1]=gShapes[5][2][1]=gShapes[5][2][2]=1;
	gShapes[6][0][0]=gShapes[6][0][2]=gShapes[6][1][0]=gShapes[6][1][1]=gShapes[6][1][2]=1;
	gShapes[7][0][1]=gShapes[7][0][2]=gShapes[7][1][1]=gShapes[7][2][0]=gShapes[7][2][1]=1;
	gShapes[8][0][0]=gShapes[8][1][0]=gShapes[8][2][0]=gShapes[8][2][1]=gShapes[8][3][0]=1;
	gShapes[9][0][1]=gShapes[9][1][0]=gShapes[9][1][1]=gShapes[9][2][1]=gShapes[9][2][2]=1;
	gShapes[10][0][0]=gShapes[10][0][1]=gShapes[10][0][2]=gShapes[10][1][1]=gShapes[10][2][1]=1;
	gShapes[11][0][0]=gShapes[11][1][0]=gShapes[11][2][0]=gShapes[11][2][1]=gShapes[11][3][1]=1;

	gameFile.name[0]=0x00;
	deleteTheThing=FALSE;
	freshClick=TRUE;
	
	CountAppFiles(&i, &j);
	if ((j>0) && (i==0))
	{
		GetAppFiles(1, &myFile);
		MyMakeFSSpec(myFile.vRefNum, 0, myFile.fName, &gameFile);
		GenericOpen(&gameFile);
	}
}

void NewGame(void)
{
	CalculateRowsandCols();
	InitGameVariables();
	OpenNewGame();
}

void WinGame(void)
{
	DoSound(sound_wingame);
	PositionDialog('ALRT', congratsAlert);
	Alert(congratsAlert, 0L);
}

void GameUndo(void)
{
	int			i,j;
	
	if (gNumHilited)
	{
		gNumHilited = 0;
		for (i=0; i<gNumRows; i++)
			for (j=0; j<gNumCols; j++)
				if (Board[i][j]==-2)
					Board[i][j]=(gCurrentColor==-2) ? -1 : gCurrentColor;
		gCurrentColor=-2;
	}
	else if (gNumPlayed)
	{
		DoSound(sound_undo);
		gNumPlayed--;
		PieceUsed[PiecesPlayed[gNumPlayed]] = FALSE;
		for (i=0; i<gNumRows; i++)
			for (j=0; j<gNumCols; j++)
				if (Board[i][j]==PiecesPlayed[gNumPlayed])
					Board[i][j]=-1;
	}
	UpdateBoard();
}

void GameEvent(void)
{
	Point		mouseLoc;
	Rect		boardRect;
	int			i,j;
	int			iter,utter;
	int			whichWay;
	Rect		smallRect;
	Boolean		didSomething;
	Boolean		freshClick;
	Boolean		justRemovedOne;
	
	SetRect(&boardRect,5,gWindowHeight-(gNumRows*25)-5,gWindowWidth-5,gWindowHeight-5);
	GetMouse(&mouseLoc);
	
	if (PtInRect(mouseLoc,&boardRect))
	{
		whichWay=0;
		justRemovedOne=FALSE;
		freshClick=TRUE;
		
		while (StillDown())
		{
			GetMouse(&mouseLoc);
			if (PtInRect(mouseLoc,&boardRect))
			{
				j=(mouseLoc.h-5)/25;
				i=(mouseLoc.v-gWindowHeight+(gNumRows*25)+5)/25;
				smallRect.top=gWindowHeight-((gNumRows-i)*25)-4;
				smallRect.bottom=smallRect.top+24;
				smallRect.left=6+j*25;
				smallRect.right=smallRect.left+24;

				switch (Board[i][j])
				{
					case -2:
					{
						if (gCurrentColor!=-2)
						{
							if (freshClick)
							{
								if (Ticks-oldTicks<DoubleTime)
								{
									RemovePiece();
									justRemovedOne=TRUE;
								}
								else
									GameUndo();
								oldTicks=0;
								freshClick=FALSE;
							}
						}
						else if (whichWay>=0)
						{
							gNumHilited--;
							Board[i][j]=-1;
							if (GetWindowDepth() > 2)
								FillCRect(&smallRect,gPlainColors[1]);
							else
								FillRect(&smallRect,*gBWPatterns[1]);
							whichWay=1;
						}
						break;
					}
					case -1:
					{
						if ((whichWay<=0) && (gCurrentColor==-2) && (!justRemovedOne))
						{
							gNumHilited++;
							Board[i][j]=-2;
							if (GetWindowDepth() > 2)
								FillCRect(&smallRect,gPlainColors[0]);
							else
								FillRect(&smallRect,*gBWPatterns[0]);
							whichWay=-1;
							freshClick=FALSE;
						}
						break;
					}
					default:
					{
						if ((whichWay==0) && (gNumHilited==0) && (Board[i][j]!=12))
						{
							gCurrentColor=Board[i][j];
							for (iter=0; iter<gNumRows; iter++)
								for (utter=0; utter<gNumCols; utter++)
									if (Board[iter][utter]==gCurrentColor)
										Board[iter][utter]=-2;
							gNumHilited=5;
							oldTicks=Ticks;
							UpdateBoard();
							freshClick=FALSE;
						}
						break;
					}
				}
			}
		}
	}
	else
	{
		i=0;
		didSomething=FALSE;
		do
		{
			boardRect.top=10;
			boardRect.bottom=42;
			boardRect.left=(gWindowWidth/2)-((3-i)*34)+1;
			boardRect.right=boardRect.left+32;
			if ((!PieceUsed[i]) && (PtInRect(mouseLoc,&boardRect)))
			{
				if (gNumHilited==5)
					j=PlacePiece(i);
				didSomething=TRUE;
			}
			else
			{
				boardRect.top+=40;
				boardRect.bottom+=40;
				if ((!PieceUsed[i+6]) && (PtInRect(mouseLoc, &boardRect)))
				{
					if (gNumHilited==5)
						j=PlacePiece(i+6);
					didSomething=TRUE;
				}
			}
			i++;
		}
		while ((!didSomething) && (i<6));
		
		if (didSomething)
		{
			if (gNumHilited==0)
			{
				DoSound(sound_error);
				ParamText("\pYou must first highlight on the board where this piece \
should go.","","","");
				PositionDialog('ALRT', generalAlert);
				StopAlert(generalAlert,0L);
			}
			else if (gNumHilited!=5)
			{
				DoSound(sound_error);
				ParamText("\pThat piece does not match what you have highlighted \
on the board.","","","");
				PositionDialog('ALRT', generalAlert);
				StopAlert(generalAlert,0L);
			}
			else if (j>=0)
			{
				DoSound(sound_placepiece);
				PieceUsed[j]=TRUE;
				PiecesPlayed[gNumPlayed]=j;
				gNumPlayed++;
				gNumHilited=0;
				UpdateBoard();
				if (gNumPlayed==12)
					WinGame();
			}
			else if (j==-1)
			{
				DoSound(sound_error);
				ParamText("\pThat piece does not match what you have highlighted \
on the board.","","","");
				PositionDialog('ALRT', generalAlert);
				StopAlert(generalAlert,0L);
			}
		}
	}
}

void GameKeyEvent(char charPressed)
{
	int			pieceNum;
	
	if ((charPressed==0x0d) || (charPressed==0x03))
	{
		if (gNumHilited==5)
		{
			if (gCurrentColor==-2)
				PlaceUnknownPiece();
		}
		else if (gNumHilited>0)
		{
			DoSound(sound_error);
			ParamText("\pThat is not one of the 12 Pentominoes pieces.","","","");
			PositionDialog('ALRT', generalAlert);
			StopAlert(generalAlert,0L);
		}
	}
	if (charPressed==0x08)
	{
	 	if ((gNumHilited==5) && (gCurrentColor!=-2))
			RemovePiece();
		else if (gNumHilited>0)
			GameUndo();
	}
}

void ShutDownGame(void)
{
	int			i;
	
	for (i=0; i<12; i++)
	{
		if (gHasColorQD)
			DisposeCIcon(gColorIcons[i]);
		ReleaseResource(gBWIcons[i]);
	}
	for (i=0; i<15; i++)
	{
		if (gHasColorQD)
			DisposePixPat(gPlainColors[i]);
		ReleaseResource(gBWPatterns[i]);
	}
}
