/**********************************************************************\

File:		pent meat.c

Purpose:	This module handles the pattern recognition that underlies
			the whole game, figuring out what piece you've highlighted.


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

#include "pent meat.h"
#include "pent.h"
#include "msg sounds.h"
#include "msg graphics.h"
#include "msg dialogs.h"

int			tempPiece[5][5];
int			maxi,maxj;

void RemovePiece(void)
{
	int			iter,utter;
	
	for (iter=0; iter<gNumRows; iter++)
		for (utter=0; utter<gNumCols; utter++)
			if (Board[iter][utter]==-2)
				Board[iter][utter]=-1;
	PieceUsed[gCurrentColor]=FALSE;
	for (iter=0; iter<gNumPlayed; iter++)
	{
		if (PiecesPlayed[iter]==gCurrentColor)
		{
			for (utter=iter; utter<gNumPlayed; utter++)
				PiecesPlayed[utter]=PiecesPlayed[utter+1];
			iter=gNumPlayed;
		}
	}
	gNumPlayed--;
	gNumHilited=0;
	gCurrentColor=-2;
	UpdateBoard();
	DoSound(sound_undo);
}

void PlaceUnknownPiece(void)
{
	int			pieceNum;
	
	pieceNum=PlacePiece(-1);
	if (pieceNum>=0)
	{
		DoSound(sound_placepiece);
		PieceUsed[pieceNum]=TRUE;
		PiecesPlayed[gNumPlayed]=pieceNum;
		gNumPlayed++;
		gNumHilited=0;
		UpdateBoard();
		if (gNumPlayed==12)
			WinGame();
	}
	else if (pieceNum==-1)
	{
		DoSound(sound_error);
		ParamText("\pThat is not one of the 12 Pentominoes pieces.","\p","\p","\p");
		PositionDialog('ALRT', generalAlert);
		StopAlert(generalAlert,0L);
	}
	else if (pieceNum==-2)
	{
		DoSound(sound_error);
		ParamText("\pThat Pentominoes piece has already been used.","\p","\p","\p");
		PositionDialog('ALRT', generalAlert);
		StopAlert(generalAlert,0L);
	}
}

int PlacePiece(int pieceNum)
{
	int			i,j;
	Boolean		found;
	int			shapeTop,shapeLeft;
	int			returnValue;
	
	i=0;
	do
	{
		j=0;
		do
		{
			found=(Board[i][j]==-2);
			j++;
		}
		while ((!found) && (j<gNumCols));
		i++;
	}
	while ((!found) && (i<gNumRows));
	shapeTop=i-1;
	j=0;
	do
	{
		i=0;
		do
		{
			found=(Board[i][j]==-2);
			i++;
		}
		while ((!found) && (i<gNumRows));
		j++;
	}
	while ((!found) && (j<gNumCols));
	shapeLeft=j-1;
		for (i=0; i<5; i++)
		for (j=0; j<5; j++)
			tempPiece[i][j]=0;
		i=0;
	do
	{
		j=0;
		do
		{
			if (Board[i+shapeTop][j+shapeLeft]==-2)
				tempPiece[i][j]=1;
			j++;
		}
		while ((j<5) && (j<gNumCols-shapeLeft));
		i++;
	}		
	while ((i<5) && (i<gNumRows-shapeTop));
		maxi=5;
	while (BlankRow(maxi-1))
		maxi--;
	maxj=5;
	while (BlankCol(maxj-1))
		maxj--;
		found=FALSE;
	
	if (pieceNum>=0)
		found=MatchPiece(pieceNum);
	else
	{
		pieceNum=0;
		do
		{
			found=MatchPiece(pieceNum);
			pieceNum++;
		}
		while ((pieceNum<12) && (!found));
		if (found)
			pieceNum--;
	}
	if (found)
	{
		if (!PieceUsed[pieceNum])
		{
			for (i=0; i<gNumRows; i++)
				for (j=0; j<gNumCols; j++)
					if (Board[i][j]==-2)
						Board[i][j]=pieceNum;

			returnValue=pieceNum;
		}
		else
		{
			returnValue=-2;
		}
	}
	else
	{
		returnValue=-1;
	}
	
	return returnValue;
}

Boolean BlankRow(int i)
{
	int			j;
	
	j=0;
	while ((!tempPiece[i][j]) && (j<5))
		j++;
	return (j==5);
}

Boolean BlankCol(int j)
{
	int			i;
	
	i=0;
	while ((!tempPiece[i][j]) && (i<5))
		i++;
	return (i==5);
}

Boolean MatchPiece(int pieceNum)
{
	int			i;
	Boolean		done;
	
	i=0;
	do
	{		
		RotatePiece();
		done=CheckPiece(pieceNum);
		i++;
	}
	while ((!done) && (i<4));
	if (!done)
	{
		FlipPiece();
		i=0;
		do
		{
			RotatePiece();
			done=CheckPiece(pieceNum);
			i++;
		}
		while ((!done) && (i<4));
	}
	return done;
}

Boolean CheckPiece(int pieceNum)
{
	int			i,j;
	Boolean		matched;
	
	i=0;
	do
	{
		j=0;
		do
		{
			matched=(tempPiece[i][j]==gShapes[pieceNum][i][j]);
			j++;
		}
		while ((matched) && (j<5));
		i++;
	}
	while ((matched) && (i<5));
	return matched;
}

void RotatePiece(void)
{
	int			otherPiece[5][5];
	int			i,j;
	int			temp;
	
	for (i=0; i<maxi; i++)
		for (j=0; j<maxj; j++)
			otherPiece[i][j]=tempPiece[i][j];

	for (i=0; i<5; i++)
		for (j=0; j<5; j++)
			tempPiece[i][j]=0;

	for (i=0; i<maxi; i++)
		for (j=0; j<maxj; j++)
			tempPiece[j][maxi-i-1]=otherPiece[i][j];
			
	temp=maxj;
	maxj=maxi;
	maxi=temp;
}

void FlipPiece(void)
{
	int			i,j;
	int			temp;
		
	for (i=0; i<maxi; i++)
		for (j=0; j<maxj/2; j++)
		{
			temp=tempPiece[i][j];
			tempPiece[i][j]=tempPiece[i][maxj-j-1];
			tempPiece[i][maxj-j-1]=temp;
		}
}
