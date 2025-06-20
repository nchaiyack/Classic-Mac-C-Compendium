#include "program globals.h"
#include "generic open.h"
#include "brlr load-save.h"
#include "environment.h"
#include "dialog layer.h"
#include "error.h"
#include "print meat.h"
#include "file utilities.h"
#include "graphics.h"
#include "window layer.h"

void OpenTheFile(FSSpec *myFSS)
{
	FSSpec				saveFile;
	
	switch (GetFileType(myFSS))
	{
		case SAVE_TYPE:
			if (!IndWindowExistsQQ(kMainWindow))
			{
				saveFile=*myFSS;
				GetTheFile(&saveFile);
				gNeedToOpenWindow=FALSE;
			}
			else
			{
				DisplayTheAlert(kStopAlert, kSmallAlertID,
					"\pPlease close the current document before opening another.", "\p", "\p", "\p", 0L);
			}
			break;
	}
}

void PrintTheFile(FSSpec *myFSS)
{
	WindowPtr		theWindow;
	
	switch (GetFileType(myFSS))
	{
		case SAVE_TYPE:
			if (!IndWindowExistsQQ(kMainWindow))
			{
				GetTheFile(myFSS);
				theWindow=GetIndWindowPtr(kMainWindow);
				PrintText(GetWindowTE(theWindow));
				CloseTheWindow(theWindow);
			}
			else
			{
				DisplayTheAlert(kStopAlert, kSmallAlertID,
					"\pPlease close the current document before printing.", "\p", "\p", "\p", 0L);
			}
			break;
	}
}
