/*
	GUtilities.c
	
	General-purpose utility routines
	
*/

#include "GUtilities.h"

//Globals
Str255		gAppName;
OSType		gSignature;
short		gAppResRef;
Boolean		gInBackground = false;

Handle	GetAppIndResource(ResType theType, short index, OSErr *err)
{
#pragma unused (err)

	short	savedResFile;
	Handle	returnHandle;
	
	savedResFile = CurResFile ();
	UseResFile (gAppResRef);
	returnHandle = Get1IndResource(theType, index);
	UseResFile (savedResFile);
	return (returnHandle);
}

void InitSystem( short mbCount )
{
	short counter;
	Handle	apParam;
	Handle	bndlResource;
	OSErr	err;
	
	for (counter = 0; counter < mbCount; counter++)
		MoreMasters();
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	TEInit();
	InitMenus();
	InitDialogs(0L);
	DrawMenuBar();
	FlushEvents(everyEvent,0L);
	InitCursor();
	InitAllPacks();
	GetAppParms(gAppName, &gAppResRef, &apParam);
	bndlResource = GetAppIndResource('BNDL', 1, &err);
	if (bndlResource)
		gSignature = *(OSType *) (*bndlResource);

}

Boolean LoadMenus( short mBarNum )
{
	Handle	menuBar;
	
	menuBar = GetNewMBar(mBarNum);	
	if (!menuBar)
		return false;
	SetMenuBar(menuBar);					/* install menus */
	DisposHandle(menuBar);
	AddResMenu(GetMHandle(mApple), 'DRVR');	/* add DA names to Apple menu */
	DrawMenuBar();
	return true;
}

void TellUser(Str255 what, short errNum)
{
	short		itemHit;
	unsigned char	errNumStr[256];
	
	SetCursor(&qd.arrow);
	if (errNum) 
		sprintf((char *) &errNumStr, "Significant number: %d", errNum);
	else
		sprintf((char *) &errNumStr, "No error number to report", 0);
	ParamText(what, (ConstStr255Param) c2pstr((char *) errNumStr),nil,nil);
	itemHit = NoteAlert(rUserAlert, nil);
}

char *C2PStrCpy(char *Cstr, Str255 Pstr)
{
	short i, len = strlen(Cstr);
	
	for(i=len; i>0; i--) {
		Pstr[i] = Cstr[i-1];
	}
	Pstr[i] = len;
	
	return( (char *) Pstr );
}

Boolean GetOpenFSSpec(SFTypeList *types, short nTypes, FSSpec *fileSpec)
{
	StandardFileReply	openReply;
	
	StandardGetFile(nil, nTypes, *types, &openReply);
	*fileSpec = openReply.sfFile;
	return(openReply.sfGood);
}

Boolean GetSaveFSSpec(SFTypeList *types, FSSpec *fileSpec)
{
	StandardFileReply	saveReply;
	
	StandardPutFile((ConstStr255Param) "\pSave as:",(ConstStr255Param) "", &saveReply);
	
	*fileSpec = saveReply.sfFile;
	return (saveReply.sfGood);
}

