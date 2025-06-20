#include "generic open.h"
#include "program globals.h"
#if USE_SAVE
#include "save dialog.h"
#include "nim load-save.h"
#include "nim globals.h"
#include "nim endgame.h"
#include "environment.h"
#include "error.h"
#include "file utilities.h"
#include "window layer.h"
#endif

void OpenTheFile(FSSpec *myFSS)
{
#if !USE_SAVE
	#pragma unused(myFSS)
#else
	FSSpec			saveFile;
	WindowRef		theWindow;
	short			saveResult;
	Boolean			goon;
	
	switch (GetFileType(myFSS))
	{
		case SAVE_TYPE:
			if (!IndWindowExistsQQ(kMainWindow))
				OpenTheIndWindow(kMainWindow, kAlwaysOpenNew);
			theWindow=GetIndWindowRef(kMainWindow);
			if (theWindow==0L)
			{
				HandleError(kNoMemory, FALSE, TRUE);
			}
			else
			{
				if (gGameStatus==kGameInProgress)
				{
					saveResult=DisplaySaveAlert(theWindow, "\p", "\pstarting another");
					switch (saveResult)
					{
						case kUserSaved:
							goon=TRUE;
							break;
						case kUserDidntSave:
							EndGame(FALSE, FALSE);
							goon=TRUE;
							break;
						case kUserCanceled:
							goon=FALSE;
							break;
					}
				}
				else goon=TRUE;
				
 				if (goon)
 				{
 					saveFile=*myFSS;
					HandleError(GetTheFile(&saveFile), FALSE, FALSE);
					gNeedToOpenWindow=FALSE;	/* not technically needed */
				}
			}
			break;
	}
#endif
}

void PrintTheFile(FSSpec *myFSS)
{
	#pragma unused(myFSS)
}
