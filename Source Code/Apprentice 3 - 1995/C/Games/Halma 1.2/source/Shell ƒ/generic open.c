/**********************************************************************\

File:		generic open.c

Purpose:	This module handles opening saved games via apple events.

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

#include "program globals.h"
#include "graphics.h"
#include "generic open.h"
#include "halma load-save.h"
#include "debinhex dispatch.h"
#include "file management.h"
#include "environment.h"
#include "dialogs.h"
#include "error.h"

void GenericOpen(FSSpec *myFSS)
{
	enum ErrorTypes		resultCode;
	
	if (!gInProgress)
	{
		switch (GetFileType(myFSS))
		{
			case SAVE_TYPE:
				if (!GetIndWindowGrafPtr(kMainWindow))
				{
					gameFile=*myFSS;
					GetSavedGame(&gameFile);
				}
				else
				{
					PositionDialog('ALRT', smallAlert);
					ParamText("\pPlease close the current game before starting another.","\p","\p","\p");
					StopAlert(smallAlert, 0L);
				}
				break;
			case 'TEXT':
			case '????':
				inputFS=*myFSS;
				HandleError(DeBinHexDispatch(), FALSE);
				break;
		}
	}
	else
	{
		PositionDialog('ALRT', smallAlert);
		ParamText("\pPlease wait until the current file is done before starting another.","\p","\p","\p");
		StopAlert(smallAlert, 0L);
	}
}

OSType GetFileType(FSSpec *myFSS)
{
	HParamBlockRec	paramBlock;
	
	paramBlock.fileParam.ioCompletion=0L;
	paramBlock.fileParam.ioNamePtr=myFSS->name;
	paramBlock.fileParam.ioVRefNum=myFSS->vRefNum;
	paramBlock.fileParam.ioFDirIndex=0;
	paramBlock.fileParam.ioDirID=myFSS->parID;
	PBHGetFInfo(&paramBlock, FALSE);

	return paramBlock.fileParam.ioFlFndrInfo.fdType;
}
