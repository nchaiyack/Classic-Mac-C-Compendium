/*---------------------------- CVroot.c ----------------------------
	This is the header for the root file. The CVglobs.h file is generated 
	from it.
	
	The contents of this file are:
		1. Global definitions (these cannot be duplicated.)
		
------------------------------------------------------------------------*/

#include "CVectors.h"

/* 1. Globals for CVectors.c; rescue stuff */

/* 2. Globals are declared here. */
/*	2a. variables */

MenuHandle DeskMenu;
MenuHandle FileMenu;
MenuHandle EditMenu;
MenuHandle FindMenu;
MenuHandle MarkMenu;
MenuHandle WindowMenu;
MenuHandle ViewMenu;
MenuHandle ReportsMenu;
MenuHandle DataMenu;
MenuHandle RotateMenu;
MenuHandle ScaleMenu;
MenuHandle DataFieldsMenu;
MenuHandle SelectTemplateMenu;

Handle AppScratch;
	
short gAppResRefNum;
short gPrefsResRefNum;
short gSearchMethod = 0;

Boolean gAppIsRunning = true;
Boolean gSearchBackwards = false;
Boolean gCaseSensitive = false;
Boolean gWrapSearch = false;

short gSelectedBooks = 0;
unsigned short gProcessingFlagBits = 0x1E32;
unsigned short gTextOutputFlagBits = 0x8000;
short gNumRoseBins = 36;
short gCurrentTemplate = 0;
Handle gMapHandles = 0;

Str255 gCommandLine = "CML ";
Str255 gFindBuffer = "\p";
Str255 gReplaceBuffer = "\p";

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
short Scaling=false;
short NumWindows=0;
short ToolWindowVisible;
short SurveySelected=0;
short SelectedFSB;
char SelectedDesig[4];
	
short NumFileTypes = 2;
SFTypeList MyFileTypes = {'CMLB',
						'TEXT',0 ,0 };
Boolean gHasColorQD;
Boolean gMacPlusKBD;
Boolean gInBackground;
Boolean gDAonTop;
short gLastScrapCount;
OSType gClipType;
FSSpec DefaultSpecs;
FSSpec HomeSpecs;

/* to pass an event record into ScrollAction */
EventRecord *PassEvent;

unsigned long gSleepTime=0;
