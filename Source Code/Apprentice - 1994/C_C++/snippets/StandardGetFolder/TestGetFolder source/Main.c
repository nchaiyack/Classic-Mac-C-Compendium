#ifdef SystemSevenOrLater
#undef SystemSevenOrLater
#endif
#define SystemSevenOrLater 1

#define kHasFSSpecCalls 0x00000002

#include <GestaltEqu.h>
#include <Script.h>
#include <Traps.h>
#include "StandardGetFolder.h"

void main (void);

void main (void)
{	
	StandardFolderReply	theReply;
	OSErr				theError;
	Str255				testPrompt = {"\pSelect A Folder:"};
	Str255				fileName = {"\pTestFile"};
	FSSpec				newFile;
	Boolean				useFSSpecCalls = false;
	long				result;

	MaxApplZone();
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent,0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	InitCursor();
	
	/* Check to see if we can use FSSpec calls */
	
	if (GetOSTrapAddress(_Gestalt) != GetToolTrapAddress(_Unimplemented))
		if ((Gestalt(gestaltFSAttr,&result) == noErr)&&(result&kHasFSSpecCalls))
			useFSSpecCalls = true;
	
	theError = StandardGetFolder (testPrompt,&theReply,NULL);

	/* if a folder was selected and no error occurred, create the test file, using FSSpec */
	/* calls if available.                                                                */
	
	if ((theError == noErr)&&(theReply.sfGood))
		{
		if (useFSSpecCalls)
			{
			theError = FSMakeFSSpec(theReply.sfVRefNum,theReply.sfDirID,fileName,&newFile);
			theError = FSpCreate(&newFile,'MSWD','WDBN',smSystemScript);
			}
		else
			HCreate(theReply.sfVRefNum,theReply.sfDirID,fileName,'MSWD','WDBN');
		}
}

