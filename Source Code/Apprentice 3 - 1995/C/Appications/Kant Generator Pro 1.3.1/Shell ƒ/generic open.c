#include "generic open.h"
#include "kant load-save.h"
#include "kant build files.h"
#include "kant build print.h"
#include "environment.h"
#include "error.h"
#include "printing layer.h"
#include "file utilities.h"
#include "window layer.h"
#include "program globals.h"

void OpenTheFile(FSSpec *myFSS)
{
	OSErr			oe;
	
	switch (GetFileType(myFSS))
	{
		case SAVE_TYPE:
			if (GetTheFile(myFSS)==allsWell)
				gNeedToOpenWindow=FALSE;
			break;
		case BUILD_TYPE:
			if (IndWindowExistsQQ(kBuildWindow))
				CloseTheWindow(GetIndWindowRef(kBuildWindow));
			oe=OpenTheModule(myFSS, FALSE, TRUE);
			if ((oe!=noErr) && (oe!=-1))	/* -1 = user cancel */
			{
				HandleError(kCantOpenModule, FALSE, FALSE);
			}
			if (oe==noErr)
				gNeedToOpenWindow=FALSE;
			break;
	}
}

void PrintTheFile(FSSpec *myFSS)
{
	WindowRef		theWindow;
	OSErr			oe;
	
	switch (GetFileType(myFSS))
	{
		case SAVE_TYPE:
			if (GetTheFile(myFSS)==allsWell)
			{
				theWindow=GetIndWindowRef(kMainWindow);
				PrintText(GetWindowTE(theWindow));
				CloseTheWindow(theWindow);
			}
			break;
		case BUILD_TYPE:
			if (IndWindowExistsQQ(kBuildWindow))
				CloseTheWindow(GetIndWindowRef(kBuildWindow));
			oe=OpenTheModule(myFSS, FALSE, TRUE);
			if ((oe!=noErr) && (oe!=-1))	/* -1 = user cancel */
			{
				HandleError(kCantOpenModule, FALSE, FALSE);
			}
			
			if (oe==noErr)
			{
				theWindow=GetIndWindowRef(kBuildWindow);
				PrintTheModule();
				CloseTheWindow(theWindow);
			}
			break;
	}
}
