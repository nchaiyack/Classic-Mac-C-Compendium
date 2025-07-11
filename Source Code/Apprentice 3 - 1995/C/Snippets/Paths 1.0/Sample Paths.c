/*******************************************************************************
 * Sample Paths.c
 *
 *******************************************************************************/

#include "Paths.h"
#include <stdio.h>


// local prototypes

void InitToolbox(void);
unsigned char * ptocstr(Str255 p, unsigned char *c);



/*******************************************************************************
 * InitToolbox
 *******************************************************************************/

void InitToolbox()
{
	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	FlushEvents(everyEvent,0);
	TEInit();
	InitDialogs(0L);
	InitCursor();
} // InitToolbox



/*******************************************************************************
 * ptocstr
 *******************************************************************************/

unsigned char * ptocstr(Str255 p, unsigned char *c)
{
	BlockMove((Ptr)(p + 1), (Ptr)c, p[0]);
	c[p[0]] = '\0';
	return c;
} // ptocstr



/*******************************************************************************
 * main
 *******************************************************************************/

main(void)
{
	short			vRefNum;
	long			dirId;
	short			index;
	Boolean			findDirs, findFiles;
	Str32			rootVolName;
	Str255			pathName;
	unsigned char	cstr[256];
	Point			where;
	OSErr			err;
	
	InitToolbox();
	
	
	// get root information
	
	err = Get_Root_Vol_Info(&vRefNum, rootVolName);
	printf("%-30s %s\n\n", "Root Volume:", ptocstr(rootVolName, cstr));
	

	// get system folder information
	
	err = Get_SystemFolder_Path(rootVolName, pathName);
	Prepend_Dir_To_Path(rootVolName, pathName, pathName);
	printf("%-30s %s\n\n", "System Folder Path:", ptocstr(pathName, cstr));

	// get preferences folder information
	
	err = Get_Preferences_Folder(&vRefNum, &dirId);
	DirId_To_Path(vRefNum, dirId, rootVolName, pathName);
	Prepend_Dir_To_Path(rootVolName, pathName, pathName);
	printf("%-30s %s\n\n", "Preferences Folder Path:", ptocstr(pathName, cstr));


	// find TeachText
	
	vRefNum = 0;
	dirId = fsRtDirID;
	index = 1;
	findDirs = FALSE;
	findFiles = TRUE;
	printf("Searching...\r");
	err = Find_File("\pTeachText", &vRefNum, &dirId, &index, -1, &findDirs, &findFiles);
	if (err == noErr)
	{
		DirId_To_Path(vRefNum, dirId, rootVolName, pathName);
		Prepend_Dir_To_Path(rootVolName, pathName, pathName);
		printf("%-30s %s\n\n", "TeachText found in:", ptocstr(pathName, cstr));
	}
	else
		printf("%-30s\n\n", "TeachText not found.");
	
	
	// get information about a user-selected directory
	
	where.h = where.v = 0;
	if (!SFGetDirectory(where, -4000, &vRefNum, &dirId))
		return 0;
	DirId_To_Path(vRefNum, dirId, rootVolName, pathName);
	Prepend_Dir_To_Path(rootVolName, pathName, pathName);
	printf("%-30s %s\n\n", "User-selected Folder:", ptocstr(pathName, cstr));
		
		
	printf("Click Mouse To Exit.\n");
	while (!Button());
	return 0;
} // main

