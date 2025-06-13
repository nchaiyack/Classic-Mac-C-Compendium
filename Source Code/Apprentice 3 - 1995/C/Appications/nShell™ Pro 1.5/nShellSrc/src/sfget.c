/* ==========================================

	sfget.c
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

#include <GestaltEqu.h>
#include <Folders.h>
#include <Traps.h>
#include <Script.h>
#include <Aliases.h>

#include "multi.h"
#include "resource.h"

#include "sfget.proto.h"
#include "path.proto.h"
#include "str_utl.proto.h"

/* typedefs */

typedef struct {
	StandardFileReply	*replyPtr;
	FSSpec				oldSelection;
	} SFData, *SFDataPtr;

/* globals */

extern Boolean	gHasFindFolder;		// true if the findfolder call is there
extern Boolean	gHasNewStdFile;		// true if the new SF calls are there

FSSpec	gDeskFolderSpec;

/* do getfile */

Boolean sfget(Str255 path)
{
	Point where = {-1,-1};
	StandardFileReply sfReply;
	SFData sfData;
	Boolean	success;
	Str255 dir_path;
	
	success = false;
	
	if ( !gHasNewStdFile ) {
		path[0] = 0;
		return( success );
		}
	
	/* initialize user data area */
	
	sfData.replyPtr = &sfReply;
	sfData.oldSelection.vRefNum = -9999;	/* init to ridiculous value */
	
	CustomGetFile(nil,-1,nil,&sfReply,SFGetDialog,where,sfget_hook,nil,nil,nil,&sfData);
	
	if (sfReply.sfGood)
		if (!path_from_dir( sfReply.sfFile.vRefNum, sfReply.sfFile.parID, dir_path ) ) {
			pStrCopy( path, "\p\"" );
			pStrAppend( path, dir_path );
			pStrAppend( path, sfReply.sfFile.name );
			if ( sfReply.sfFile.parID == 1 )
				pStrAppend( path, "\p:\"" );
			else
				pStrAppend( path, "\p\"" );
			success = true;
			}
		
	return( success );
}

pascal short sfget_hook(short item,DialogPtr theDlg,Ptr userData)
{
	SFDataPtr sfData;
	Boolean hiliteButton;
	FSSpec curSpec;
	OSType refCon;
	
	refCon = GetWRefCon(theDlg);
	if (refCon!=sfMainDialogRefCon)
		return item;
		
	sfData = (SFDataPtr) userData;
	
	if (item==sfHookFirstCall || item==sfHookLastCall)
		return item;
	
	if (item==sfItemVolumeUser) {
		sfData->replyPtr->sfFile.name[0] = '\0';
		sfData->replyPtr->sfFile.parID = 2;
		sfData->replyPtr->sfIsFolder = false;
		sfData->replyPtr->sfIsVolume = false;
		sfData->replyPtr->sfFlags = 0;
		item = sfHookChangeSelection;
		}
		
	if (!sfget_same_file(&sfData->replyPtr->sfFile,&sfData->oldSelection)) {
	
		BlockMove(&sfData->replyPtr->sfFile,&curSpec,sizeof(FSSpec));
		sfget_ensure_name(&curSpec);
		
		if (curSpec.vRefNum!=sfData->oldSelection.vRefNum)
			sfget_get_desktop_folder(curSpec.vRefNum);
			
		sfget_button(curSpec.name,theDlg);
		
		BlockMove(&sfData->replyPtr->sfFile,&sfData->oldSelection,sizeof(FSSpec));
		}
	
	if (item==SFGetButton)
		item = sfItemOpenButton;
		
	return item;
}

void sfget_button( StringPtr selName ,DialogPtr theDlg )
{
	short iType;
	Handle iHndl;
	Rect iRect;
	Str255 botton_string;
		
	pStrCopy( botton_string, "\pSelect" );
	pStrAppend( botton_string, "\p \"");
	pStrAppend( botton_string, selName );
	botton_string[ ++botton_string[0] ] = '"';

	GetDItem(theDlg,SFGetButton,&iType,&iHndl,&iRect);
	SetCTitle(iHndl,botton_string);
	SetDItem(theDlg,SFGetButton,iType,iHndl,&iRect);
}

Boolean sfget_same_file(FSSpec *file1,FSSpec *file2)
{
	if (file1->vRefNum != file2->vRefNum)
		return false;
	if (file1->parID != file2->parID)
		return false;
	if (!EqualString(file1->name,file2->name,false,true))
		return false;
	
	return true;
}

OSErr sfget_get_desktop_folder(short vRefNum)
{
	DirInfo infoPB;
	OSErr err;
	
	if (!gHasFindFolder) {
		gDeskFolderSpec.vRefNum = -9999;
		return( -1 );
		}
	
	gDeskFolderSpec.name[0] = '\0';
	err = FindFolder(vRefNum,kDesktopFolderType,kDontCreateFolder,&gDeskFolderSpec.vRefNum,&gDeskFolderSpec.parID);
	if (err!=noErr)
		return( err );
	
	return( sfget_ensure_name(&gDeskFolderSpec) );
}

OSErr sfget_ensure_name(FSSpec *fss)
{
	DirInfo infoPB;
	OSErr err;

	if (fss->name[0] != '\0')
		return( 0 );
		
	infoPB.ioNamePtr = fss->name;
	infoPB.ioVRefNum = fss->vRefNum;
	infoPB.ioDrDirID = fss->parID;
	infoPB.ioFDirIndex = -1;
	err = PBGetCatInfo(&infoPB,false);
	fss->parID = infoPB.ioDrParID;
	
	return( err );
}

