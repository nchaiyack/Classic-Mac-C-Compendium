/**********************************************************************\

File:		cube generic open.c

Purpose:	This module handles opening a saved game gotten through
			apple events.


DevilÕs Cubes -- a simple cubes puzzle
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

#include "cube generic open.h"
#include "cube load-save.h"
#include "msg graphics.h"
#include "msg dialogs.h"

void GenericOpen(FSSpec *myFSS)
{
	HParamBlockRec	paramBlock;
	
	if (gMainWindow==0L)
	{
		paramBlock.fileParam.ioCompletion=0L;
		paramBlock.fileParam.ioNamePtr=myFSS->name;
		paramBlock.fileParam.ioVRefNum=myFSS->vRefNum;
		paramBlock.fileParam.ioFDirIndex=0;
		paramBlock.fileParam.ioDirID=myFSS->parID;
		PBHGetFInfo(&paramBlock, FALSE);
		
		switch (paramBlock.fileParam.ioFlFndrInfo.fdType)
		{
			case 'DcSG':
				gameFile=*myFSS;
				GetSavedGame(&gameFile);
				break;
		}
	}
	else
	{
		ParamText("\pPlease close the current game before loading a new one.","\p","\p","\p");
		PositionDialog('ALRT', generalAlert);
		Alert(generalAlert, 0L);
	}
}
