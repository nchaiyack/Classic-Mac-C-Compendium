/**********************************************************************\

File:		main.c

Purpose:	This module handles all the file moving to and from the
			"Startup Screens" folder.
			

Startup Screen Picker -=- pick a random startup screen
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

#include "main.h"
#include "init.h"
#include "prefs.h"
#include "Folders.h"

unsigned char		gLastName[32];

int DoSSThing(void)
{
	OSErr			isHuman;
	CInfoPBRec		pb_file, pb_dir;
	HParamBlockRec	pb_basic, pb_new;
	CMovePBRec		pb_move;
	int				vRefNum;
	long			parID;
	int				i;
	long			temp;
	unsigned char	newName[32];
	
	isHuman=FindFolder(kOnSystemDisk, kSystemFolderType, kDontCreateFolder, &vRefNum,
				&parID);					/* find system folder */
	if (isHuman!=noErr)
		return cantFindSystemFolderErr;
	
	pb_dir.dirInfo.ioCompletion=0L;
	pb_dir.dirInfo.ioNamePtr="\pStartup Screens";
	pb_dir.dirInfo.ioVRefNum=vRefNum;
	pb_dir.dirInfo.ioFDirIndex=0; /* very important */
	pb_dir.dirInfo.ioDrDirID=parID;
	isHuman=PBGetCatInfo(&pb_dir, FALSE);	/* get info on "Startup Screens" dir */
	if (isHuman!=noErr)
		return cantGetDirInfoErr;
	
	if (pb_dir.dirInfo.ioDrNmFls==0)		/* any files in "Startup Screens" dir? */
		return noFilesErr;
	
	pb_new.fileParam.ioCompletion=0L;
	pb_new.fileParam.ioNamePtr=newName;
	pb_new.fileParam.ioVRefNum=vRefNum;
	GetDateTime(&temp);
	pb_new.fileParam.ioFDirIndex=(temp&0x7fffffff)%pb_dir.dirInfo.ioDrNmFls+1;
	pb_new.fileParam.ioDirID=pb_dir.dirInfo.ioDrDirID;
	isHuman=PBGetCatInfo(&pb_new, FALSE);	/* get info on random file in */
	if (isHuman!=noErr)						/* "Startup Screens dir */
		return cantGetNewFileInfoErr;
	
	pb_basic.fileParam.ioCompletion=0L;
	pb_basic.fileParam.ioNamePtr="\pStartupScreen";
	pb_basic.fileParam.ioVRefNum=vRefNum;
	pb_basic.ioParam.ioMisc=gLastName;
	pb_basic.fileParam.ioDirID=parID;
	isHuman=PBHRename(&pb_basic, FALSE);	/* rename "StartupScreen" to saved name */
	if ((isHuman!=noErr) && (isHuman!=fnfErr))
		return cantRenameStartupScreenErr;
	
	if (isHuman!=fnfErr)
	{
		pb_move.ioCompletion=0L;
		pb_move.ioNamePtr=gLastName;
		pb_move.ioVRefNum=vRefNum;
		pb_move.ioNewName=0L;
		pb_move.ioNewDirID=pb_dir.dirInfo.ioDrDirID;
		pb_move.ioDirID=pb_basic.fileParam.ioDirID;
		isHuman=PBCatMove(&pb_move, FALSE);	/* move old one into "Startup Screens" dir */
		if (isHuman!=noErr)
			return cantMoveStartupScreenErr;
	}
	
	pb_move.ioCompletion=0L;
	pb_move.ioNamePtr=newName;
	pb_move.ioVRefNum=vRefNum;
	pb_move.ioNewName=0L;
	pb_move.ioNewDirID=parID;
	pb_move.ioDirID=pb_dir.dirInfo.ioDrDirID;
	isHuman=PBCatMove(&pb_move, FALSE);		/* move new one into system folder */
	if (isHuman!=noErr)
		return cantMoveNewFileErr;
	
	pb_basic.fileParam.ioCompletion=0L;
	pb_basic.fileParam.ioNamePtr=newName;
	pb_basic.fileParam.ioVRefNum=vRefNum;
	pb_basic.ioParam.ioMisc="\pStartupScreen";
	pb_basic.fileParam.ioDirID=parID;
	isHuman=PBHRename(&pb_basic, FALSE);	/* rename new one to "StartupScreen" */
	if (isHuman!=noErr)
		return cantRenameNewFileErr;
	
	for (i=newName[0]; i>=0; i--)
		gLastName[i]=newName[i];
	
	SaveThePrefs();
	
	return allsWell;
}
