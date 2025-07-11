/**********************************************************************\

File:		dialectic scrap.c

Purpose:	This module handles clipboard conversion.


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
#include "dialectic scrap.h"
#include "dialectic.h"
#include "dialectic dispatch.h"
#include "error.h"
#include "progress.h"
#include "program globals.h"
#include "util.h"
#include "environment.h"
#include "menus.h"

void NewScrapConvert(void)
{
	int				err;
	Boolean			notDoneYet;
	Str255			titleStr;
	Handle			inputHandle, outputHandle;
	unsigned long	dummy;
	unsigned long	lastSavedOffset;
	
	LoadScrap();
	inputHandle=NewHandle(0L);
	if (GetScrap(inputHandle, 'TEXT', &dummy)==noTypeErr)
	{
		DisposeHandle(inputHandle);
		HandleError(kNoTextInScrap, FALSE);
		return;
	}
	
	gInputLength=GetHandleSize(inputHandle);
	if (gInputLength==0L)
	{
		DisposeHandle(inputHandle);
		HandleError(kScrapTooLarge, FALSE);
		return;
	}
	
	outputHandle=NewHandle(0L);
	
	lastSavedOffset=gInputOffset=gOutputOffset=gAbsoluteOffset=0L;
	
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
		SetProgressText("\pConverting clipboard...","\p","\p","\p");
	}
	
	gDoingRTF=gInWord=gSeenI=gSeenBackslash=FALSE;
	gCurlyLevel=0;
	
	notDoneYet=TRUE;
	while ((notDoneYet) && (gAbsoluteOffset<gInputLength))
	{
		if (gInputOffset-lastSavedOffset>INPUT_BUFFER_MAX)
		{
			lastSavedOffset=gInputOffset;
			if ((gShowProgress) && (gInputLength>INPUT_BUFFER_MAX) && (gInProgress))
			{
				UpdateProgressDialog(gAbsoluteOffset);
				notDoneYet=DealWithOtherPeople();
			}
		}
		
		if (notDoneYet)
		{
			if (GetHandleSize(outputHandle)-gOutputOffset<OUTPUT_BUFFER_MAX)
				SetHandleSize(outputHandle, GetHandleSize(outputHandle)+OUTPUT_BUFFER_MAX);
			
			HLock(inputHandle);
			HLock(outputHandle);
			gInputBuffer=*inputHandle;
			gOutputBuffer=*outputHandle;
			
			ConvertDispatch();
			
			HUnlock(inputHandle);
			HUnlock(outputHandle);
		}
	}
	
	if ((gShowProgress) && (gInputLength>INPUT_BUFFER_MAX) && (gInProgress))
	{
		UpdateProgressDialog(gAbsoluteOffset);
		DealWithOtherPeople();
	}
	
	if (notDoneYet)
	{
		ZeroScrap();
		HLock(outputHandle);
		PutScrap(gOutputOffset, 'TEXT', *outputHandle);
	}
	
	DisposeHandle(inputHandle);
	DisposeHandle(outputHandle);
	
	DismissProgressDialog();
	
	if (gTheWindow[kClipboard])
		OpenTheWindow(kClipboard);
}
