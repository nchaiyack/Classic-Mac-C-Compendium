/**********************************************************************\

File:		fudd generic open.c

Purpose:	This module handles opening documents via apple events.


Fudd -=- convert text to Elmer Fudd talk
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

#include "program globals.h"
#include "fudd generic open.h"
#include "fudd error.h"
#include "fudd.h"
#include "msg environment.h"
#include "msg dialogs.h"

void GenericOpen(FSSpec *myFSS)
{
	int			resultCode;
	
	if (!gInProgress)
	{
		switch (GetFileType(myFSS))
		{
			case SAVE_TYPE:
				inputFS=*myFSS;
				StartConvert();
				break;
		}
	}
	else
	{
		PositionDialog('ALRT', generalAlert);
		ParamText("\pPlease wait until the current file is done before starting another.","\p","\p","\p");
		StopAlert(generalAlert, 0L);
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
