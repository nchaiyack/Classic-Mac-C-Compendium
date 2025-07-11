/* StandardFile.c */

#include "StandardFile.h"
#include "CWindow.h"
#include "File.h"
#include "Compatibility.h"


MyBoolean	FGetFile(FSSpec* FileInfo, pascal Boolean (*FileFilter)(CInfoPBRec* pb),
						OSType TypeList[4], short NumTypes)
	{
		StandardFileReply		MySFR;

		if (ActiveWindow != NIL)
			{
				ActiveWindow->DoSuspend();
				ActiveWindow = NIL;
			}
		StandardGetFile((void*)FileFilter,NumTypes,TypeList,&MySFR);
		*FileInfo = MySFR.sfFile;
		return MySFR.sfGood;
	}


/* if you want to select folders, this routine isn't going to help. */
MyBoolean	FPutFile(PString DefaultFileName, FSSpec* FileInfo, MyBoolean* Replacing)
	{
		StandardFileReply		MySFR;

		if (ActiveWindow != NIL)
			{
				ActiveWindow->DoSuspend();
				ActiveWindow = NIL;
			}
		StandardPutFile(NIL,DefaultFileName,&MySFR);
		*FileInfo = MySFR.sfFile;
		*Replacing = MySFR.sfReplacing;
		return MySFR.sfGood;
	}
