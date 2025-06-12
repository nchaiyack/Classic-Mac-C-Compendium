/*------------------------------------------------------------------------
				Copyright (c) 1993-1994 by Melburn R. Park

This code is free. It may be freely distributed and incorporated into other
programs. This code may not be subsequently copyrighted, sold, or assigned 
in any way that restricts its use by any others, including its authors.

--------------------------------------------------------------------------*/

/*---------------------------- PTroot.h ----------------------------
	This is the header for the root file. It is only #included by the
	root. The PTglobs.h file is generated from it.
	
	The contents of this file are:
		1. Global definitions (these cannot be duplicated.)
		
------------------------------------------------------------------------*/

#include "PlainText.h"

/* 1. Globals for PlainText.c; rescue stuff */

/* 2. Globals are declared here. */
/*	2a. variables */


MenuHandle DeskMenu;
MenuHandle FileMenu;
MenuHandle EditMenu;
MenuHandle FindMenu;
MenuHandle MarkMenu;
MenuHandle WindowMenu;
MenuHandle ConvertMenu;
	
Str255 gFindBuffer = "\p";
Str255 gReplaceBuffer = "\p";
Str255 gCommandLine = "";

Handle AppScratch;

short gAppResRefNum;
short gPreferencesFile;
short gSearchMethod = 0;

Boolean gAppIsRunning = true;
Boolean gSearchBackwards = false;
Boolean gCaseSensitive = false;
Boolean gWrapSearch = false;
Boolean gUseWorksheet = true;

DocumentRecord Documents[MAXWINDS];
DocumentPeek CurrentScrollDocument=0;
DocumentPeek gWorkSheetDoc=0;
DocumentPeek gClipboardDoc=0;
WindowPtr gTextWindowPosition=-1;

DialogRecord DlogStor;
DialogRecord AuxDlogStor;
	
CTabHandle MyCMHandle=0;
PaletteHandle DefaultPalette;
	
Rect DragBoundsRect;
short NumWindows=0;
	
short NumFileTypes = 1;
SFTypeList MyFileTypes = {'TEXT',
						0 ,0, 0 };

Boolean gHasColorQD;
Boolean gMacPlusKBD;
Boolean gInBackground;
Boolean gDAonTop;
short gLastScrapCount;
/* OSType gClipType; */
FSSpec DefaultSpecs;
FSSpec HomeSpecs;

/* to pass an event record into ScrollAction */
EventRecord *PassEvent;

unsigned long gSleepTime=0;

