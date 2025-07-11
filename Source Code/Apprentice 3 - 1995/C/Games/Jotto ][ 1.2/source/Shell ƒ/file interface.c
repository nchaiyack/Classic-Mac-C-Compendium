#include "file interface.h"
#include "program globals.h"
#include "window layer.h"

Boolean GetSourceFile(FSSpec *sourceFS)
/* a standard procedure which shows an open dialog box and returns the FSSpec of
   the file you selected (or returns FALSE if you cancelled) */
{
	StandardFileReply	reply;
	SFTypeList			theTypes;
	
	RemoveHilitePatch();
	
	theTypes[0]=SAVE_TYPE;
	StandardGetFile(0L, 1, theTypes, &reply);	/* reply's got an FSSpec in it afterwards */

	if (reply.sfGood)
		FSMakeFSSpec(reply.sfFile.vRefNum, reply.sfFile.parID, reply.sfFile.name,
							sourceFS);
		
	InstallHilitePatch();
	
	return reply.sfGood;	/* TRUE if we have a valid file selected */
}

Boolean GetDestFile(FSSpec *destFS, Boolean *deleteTheThing, Str255 theTitle)
/* a standard save dialog box -- given a title (for the prompt), it returns the
   file's FSSpec in destFS and whether a file of that name already exists in
   deleteTheThing (TRUE if file already exists) */
/* details are pretty much the same as GetSourceFile(), see above */
{
	StandardFileReply	reply;
	
	RemoveHilitePatch();
	
	StandardPutFile(theTitle, destFS->name, &reply);

	if (reply.sfGood)
	{
		*deleteTheThing=reply.sfReplacing;	
		FSMakeFSSpec(reply.sfFile.vRefNum, reply.sfFile.parID, reply.sfFile.name,
						destFS);
	}
	
	InstallHilitePatch();
	
	return reply.sfGood;
}
