/**********************************************************************\

File:		dialectic.c

Purpose:	This module handles Dialectic-specific initialization and
			shutdown, memory setup for converting, and a dispatch
			for file conversion.


Dialectic -=- dialect text conversion extraordinare
Copyright �1994, Mark Pilgrim

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

#include "graphics.h"
#include "dialectic.h"
#include "dialectic dispatch.h"
#include "dialectic graphics.h"
#include "file interface.h"
#include "dialectic file management.h"
#include "error.h"
#include "progress.h"
#include "program globals.h"
#include "util.h"
#include "environment.h"
#include "menus.h"

unsigned char	gShowSaveDialog;
unsigned char	gAddSuffix;
unsigned char	gShowProgress;
unsigned char	gUseRTF;

void InitTheProgram(void)
{
	int				i,j,k;
	AppFile			myFile;
	
	(**(gTheWindowData[kClipboard])).dispatchProc=ClipboardWindowDispatch;
	((**(gTheWindowData[kClipboard])).dispatchProc)(gTheWindowData[kClipboard], kStartup, 0L);
	
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
	if (GetSourceFile(&inputFS))
		StartConvert();
}

void StartConvert(void)
{
	int				err;
	Boolean			notDoneYet;
	Str255			titleStr;
	
	outputFS=inputFS;
	if (gAddSuffix)
		SetupSuffix();
	deleteTheThing=TRUE;
	
	if (gShowSaveDialog)
		if (!GetDestFile(&outputFS, &deleteTheThing, "\pSave converted text as..."))
			return;

	gInputNeedsUpdate=TRUE;
	gOutputNeedsUpdate=FALSE;
	gInputOffset=gOutputOffset=gAbsoluteOffset=0L;
	
	InitFiles();
	HandleError(err=OpenInputFile(), FALSE);
	if (err!=allsWell)
		return;
	HandleError(err=CreateTempFile(), FALSE);
	if (err!=allsWell)
	{
		FinalizeFiles(FALSE);
		return;
	}
	HandleError(err=SetupTempFile(), FALSE);
	if (err!=allsWell)
	{
		FinalizeFiles(FALSE);
		return;
	}
	
	gInputBuffer=NewPtrClear(INPUT_BUFFER_MAX+256);
	if (gInputBuffer==0L)
	{
		FinalizeFiles(FALSE);
		HandleError(kNoMemory, FALSE);
		return;
	}
	
	gOutputBuffer=NewPtrClear(OUTPUT_BUFFER_MAX+256);
	if (gOutputBuffer==0L)
	{
		FinalizeFiles(FALSE);
		DisposePtr(gInputBuffer);
		HandleError(kNoMemory, FALSE);
		return;
	}
	
	if ((!gShowProgress) || (gInputLength<=INPUT_BUFFER_MAX))
	{
		gInProgress=TRUE;
		AdjustMenus();
		DrawMenuBar();
	}
	else
	{
		GetItem(gDialectMenu, gWhichDialect+1, titleStr);
		OpenProgressDialog(gInputLength, titleStr);
		SetProgressText("\pConverting ",inputFS.name, "\p...","\p");
	}
	
	gDoingRTF=gInWord=gSeenI=gSeenBackslash=FALSE;
	gCurlyLevel=0;
	
	notDoneYet=TRUE;
	err=allsWell;
	while ((notDoneYet) && (err==allsWell) && (gAbsoluteOffset<gInputLength))
	{
		if (gInputNeedsUpdate)
		{
			if (gInputOffset>0L)
				ShiftInputBuffer();
			err=ReadInputFile(inputRefNum, gInputBuffer+gWhatsReallyInInputBuffer,
					INPUT_BUFFER_MAX+256-gWhatsReallyInInputBuffer);
			if (err!=allsWell)
			{
				DismissProgressDialog();
				HandleError(err, FALSE);
			}
			
			gInputNeedsUpdate=FALSE;
			if ((gShowProgress) && (gInputLength>INPUT_BUFFER_MAX) && (gInProgress))
			{
				UpdateProgressDialog(gAbsoluteOffset);
				notDoneYet=DealWithOtherPeople();
			}
		}
		
		if ((notDoneYet) && (err==allsWell) && (gOutputNeedsUpdate))
		{
			err=WriteTempFile(outputRefNum, gOutputBuffer, gOutputOffset);
			if (err!=allsWell)
			{
				DismissProgressDialog();
				HandleError(err, FALSE);
			}
			gOutputOffset=0L;
			gOutputNeedsUpdate=FALSE;
		}
		
		if ((notDoneYet) && (err==allsWell))
			ConvertDispatch();
	}
	
	if ((gShowProgress) && (gInputLength>INPUT_BUFFER_MAX) && (gInProgress))
	{
		UpdateProgressDialog(gAbsoluteOffset);
		DealWithOtherPeople();
	}
	
	if ((gOutputOffset>0L) && (err==allsWell))
	{
		err=WriteTempFile(outputRefNum, gOutputBuffer, gOutputOffset);
		if (err!=allsWell)
		{
			DismissProgressDialog();
			HandleError(err, FALSE);
		}
	}
	
	DisposePtr(gInputBuffer);
	DisposePtr(gOutputBuffer);
	
	FinalizeFiles((notDoneYet) && (err==allsWell));
	
	DismissProgressDialog();
}

void ShiftInputBuffer(void)
{
	Mymemcpy(gInputBuffer, gInputBuffer+gInputOffset, INPUT_BUFFER_MAX+256-gInputOffset);
	gWhatsReallyInInputBuffer=INPUT_BUFFER_MAX+256-gInputOffset;
	gInputOffset=0L;
}

void ShutDownTheProgram(void)
{
	((**(gTheWindowData[kClipboard])).dispatchProc)(gTheWindowData[kClipboard], kShutdown, 0L);
}
