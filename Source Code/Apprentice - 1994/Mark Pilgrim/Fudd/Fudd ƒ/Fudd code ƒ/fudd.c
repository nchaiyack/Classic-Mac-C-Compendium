/**********************************************************************\

File:		fudd.c

Purpose:	This module handles Fudd-specific initialization and
			shutdown, memory setup for converting, and a dispatch
			for file conversion.


Fudd -=- convert text to Elmer Fudd talk
Copyright ©1994, Mark Pilgrim

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

#include "fudd.h"
#include "fudd meat.h"
#include "fudd files.h"
#include "fudd file management.h"
#include "fudd error.h"
#include "fudd progress.h"
#include "program globals.h"
#include "util.h"
#include "msg environment.h"
#include "msg menus.h"

unsigned char	gShowSaveDialog;
unsigned char	gAddSuffix;
unsigned char	gShowProgress;

void InitProgram(void)
{
	int				i,j,k;
	AppFile			myFile;
	
	CountAppFiles(&i, &j);
	if ((j>0) && (i==0))
	{
		for (k=1; k<=j; k++)
		{
			GetAppFiles(1, &myFile);
			MyMakeFSSpec(myFile.vRefNum, 0, myFile.fName, &inputFS);
			StartConvert();
			ClrAppFiles(k);
		}
	}
}

void NewConvert(void)
{
	inputFS.name[0]=outputFS.name[0]=tempFS.name[0]=0x00;
	if (GetSourceFile(&inputFS, FALSE))
		StartConvert();
}

void StartConvert(void)
{
	int				err;
	Boolean			notDoneYet;
	
	outputFS=inputFS;
	if (gAddSuffix)
	{
		if (outputFS.name[0]>28)
			outputFS.name[0]=28;
		outputFS.name[++(outputFS.name[0])]='.';
		outputFS.name[++(outputFS.name[0])]='f';
		outputFS.name[++(outputFS.name[0])]='u';
		outputFS.name[++(outputFS.name[0])]='d';
		outputFS.name[++(outputFS.name[0])]='d';
	}
	deleteTheThing=TRUE;
	
	if (gShowSaveDialog)
		if (!GetDestFile(&outputFS, &deleteTheThing))
			return;

	gInputNeedsUpdate=TRUE;
	gOutputNeedsUpdate=FALSE;
	gInputOffset=gOutputOffset=gAbsoluteOffset=0L;
	
	InitFiles();
	HandleError(err=OpenInputFile());
	if (err!=allsWell)
		return;
	HandleError(err=CreateTempFile());
	if (err!=allsWell)
	{
		FinalizeFiles(FALSE);
		return;
	}
	HandleError(err=SetupTempFile());
	if (err!=allsWell)
	{
		FinalizeFiles(FALSE);
		return;
	}
	
	gInputBuffer=NewPtrClear(INPUT_BUFFER_MAX+16);
	if (gInputBuffer==0L)
	{
		FinalizeFiles(FALSE);
		HandleError(kNoMemory);
		return;
	}
	
	gOutputBuffer=NewPtrClear(OUTPUT_BUFFER_MAX+16);
	if (gOutputBuffer==0L)
	{
		FinalizeFiles(FALSE);
		DisposePtr(gInputBuffer);
		HandleError(kNoMemory);
		return;
	}
	
	gInProgress=TRUE;
	AdjustMenus();
	DrawMenuBar();
	
	if (showThatTharProgress)
	{
		SetProgressText("\pConverting ",inputFS.name, "\p...","\p");
		OpenProgressDialog(gInputLength, "\pFudd");
	}
	
	notDoneYet=TRUE;
	err=allsWell;
	while ((notDoneYet) && (err==allsWell) && (gAbsoluteOffset<gInputLength))
	{
		if (gInputNeedsUpdate)
		{
			if (gInputOffset>0L)
				ShiftInputBuffer();
			HandleError(err=ReadInputFile(inputRefNum, gInputBuffer+
				gWhatsReallyInInputBuffer, INPUT_BUFFER_MAX+16-gWhatsReallyInInputBuffer));
			gInputNeedsUpdate=FALSE;
			if (showThatTharProgress)
			{
				UpdateProgressDialog(gAbsoluteOffset);
				notDoneYet=DealWithOtherPeople();
			}
		}
		
		if ((notDoneYet) && (err==allsWell) && (gOutputNeedsUpdate))
		{
			HandleError(err=WriteTempFile(outputRefNum, gOutputBuffer, gOutputOffset));
			gOutputOffset=0L;
			gOutputNeedsUpdate=FALSE;
		}
		
		if ((notDoneYet) && (err==allsWell))
			ConvertIt();
	}
	
	if (showThatTharProgress)
	{
		UpdateProgressDialog(gAbsoluteOffset);
		DealWithOtherPeople();
	}
	
	if (gOutputOffset>0L)
	{
		HandleError(err=WriteTempFile(outputRefNum, gOutputBuffer, gOutputOffset));
	}
	
	DisposePtr(gInputBuffer);
	DisposePtr(gOutputBuffer);
	
	FinalizeFiles((notDoneYet) && (err==allsWell));
	
	if (showThatTharProgress)
		DismissProgressDialog();
	gInProgress=FALSE;
	AdjustMenus();
	DrawMenuBar();
}

void ShiftInputBuffer(void)
{
	Mymemcpy(gInputBuffer, gInputBuffer+gInputOffset, INPUT_BUFFER_MAX+16-gInputOffset);
	gWhatsReallyInInputBuffer=INPUT_BUFFER_MAX+16-gInputOffset;
	gInputOffset=0L;
}

void ShutDownProgram(void)
{
}
