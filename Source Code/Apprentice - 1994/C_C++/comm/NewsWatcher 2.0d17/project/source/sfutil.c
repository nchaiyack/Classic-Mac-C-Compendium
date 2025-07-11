/*----------------------------------------------------------------------------

	sfutil.c

	This module contains Standard File utility routines.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <Packages.h>
#include <stdio.h>
#include <Finder.h>
#include <Folders.h>

#include "glob.h"
#include "sfutil.h"
#include "dlgutil.h"
#include "util.h"




#define kDirSelectDlg			200		/* Directory selection standard file dialog */
#define kSelectDirButtonTop 	11
#define kSelectDirButtonBottom	10

static FSSpec gDirTop;			/* file selected in top button */
static FSSpec gDirBottom;		/* file selected in bottom button */
static short gSelect;			/* 0 = cancel, 1 = top select button, 2 = bottom select button */
static Str255 gPrevNameTop;		/* previous name in top button */
static Str255 gPrevNameBottom;	/* previous name in bottom button */



/*----------------------------------------------------------------------------
	MyStandardGetFile 
	
	Presents the StandardGetFile dialog.
	
	The parameters are the same as in StandardGetFile.
----------------------------------------------------------------------------*/

void MyStandardGetFile(FileFilterProcPtr fileFilter, short numTypes,
	SFTypeList typeList, StandardFileReply *reply)
{
	PrepDialog();
	StandardGetFile(fileFilter, numTypes, typeList, reply);
}



/*----------------------------------------------------------------------------
	MyStandardPutFile 
	
	Presents the StandardPutFile dialog.
	
	The parameters are the same as in StandardPutFile.
----------------------------------------------------------------------------*/

void MyStandardPutFile (const Str255 prompt, const Str255 defaultName,
	StandardFileReply *reply)
{
	PrepDialog();
	StandardPutFile(prompt, defaultName, reply);
}



/*----------------------------------------------------------------------------
	GetDirectoryFilter 
	
	Filter function for MyGetStandardDirectory.
----------------------------------------------------------------------------*/

static pascal Boolean GetDirectoryFilter (ParmBlkPtr pb, void *myDataPtr)
{
	return (pb->fileParam.ioFlAttrib & 0x10) == 0;
}



/*----------------------------------------------------------------------------
	GetDirectoryHook
	
	Dialog hook function for MyGetStandardDirectory.
----------------------------------------------------------------------------*/

static pascal short GetDirectoryHook (short item, DialogPtr dlg, void *myDataPtr)
{
	Handle itemHandleTop, itemHandleBottom;
	short itemType;
	Rect boxTop, boxBottom;
	CInfoPBRec pb;
	StandardFileReply *reply;
	short width;
	CStr255 buttonTitle;
	Str255 name;
	short foundVRefNum;
	long foundDirID;

	if (GetWRefCon(dlg) != sfMainDialogRefCon) return item;
	GetDItem(dlg, kSelectDirButtonTop, &itemType, &itemHandleTop, &boxTop);
	GetDItem(dlg, kSelectDirButtonBottom, &itemType, &itemHandleBottom, &boxBottom);
	
	if (item == sfHookFirstCall) {
		pb.dirInfo.ioNamePtr = gDirBottom.name;
		pb.dirInfo.ioVRefNum = -SFSaveDisk;
		pb.dirInfo.ioFDirIndex = -1;
		pb.dirInfo.ioDrDirID = CurDirStore;
		PBGetCatInfo(&pb, false);
		gDirBottom.vRefNum = pb.dirInfo.ioVRefNum;
		gDirBottom.parID = pb.dirInfo.ioDrParID;
		gDirTop = gDirBottom;
	} else {
		reply = (StandardFileReply*)myDataPtr;
		if (reply->sfIsFolder || reply->sfIsVolume) {
			gDirBottom = reply->sfFile;
			DlgEnableItem(dlg, kSelectDirButtonBottom, true);
		} else {
			DlgEnableItem(dlg, kSelectDirButtonBottom, false);
			*gDirBottom.name = 0;
		}
		if (reply->sfFile.parID == fsRtParID) {
			FindFolder(kOnSystemDisk, kDesktopFolderType, kCreateFolder,
				&foundVRefNum, &foundDirID);
			pb.dirInfo.ioNamePtr = gDirTop.name;
			pb.dirInfo.ioVRefNum = foundVRefNum;
			pb.dirInfo.ioFDirIndex = -1;
			pb.dirInfo.ioDrDirID = foundDirID;
			PBGetCatInfo(&pb, false);
			gDirTop.vRefNum = pb.dirInfo.ioVRefNum;
			gDirTop.parID = pb.dirInfo.ioDrParID;
		} else {
			pb.dirInfo.ioNamePtr = gDirTop.name;
			pb.dirInfo.ioVRefNum = reply->sfFile.vRefNum;
			pb.dirInfo.ioFDirIndex = -1;
			pb.dirInfo.ioDrDirID = reply->sfFile.parID;
			PBGetCatInfo(&pb, false);
			gDirTop.vRefNum = pb.dirInfo.ioVRefNum;
			gDirTop.parID = pb.dirInfo.ioDrParID;
		}
	}
	
	if (!EqualString(gPrevNameBottom, gDirBottom.name, false, true)) {
		pstrcpy(gPrevNameBottom, gDirBottom.name);
		width = boxBottom.right - boxBottom.left - StringWidth("\pSelect ��") - 2*CharWidth(' ');
		pstrcpy(name, gDirBottom.name);
		TruncString(width, name, smTruncMiddle);
		sprintf(buttonTitle, "Select �%#s�", name);
		c2pstr(buttonTitle);
		SetCTitle((ControlHandle)itemHandleBottom, (StringPtr)buttonTitle);
		ValidRect(&boxBottom);
	}
	
	if (!EqualString(gPrevNameTop, gDirTop.name, false, true)) {
		pstrcpy(gPrevNameTop, gDirTop.name);
		width = boxTop.right - boxTop.left - StringWidth("\pSelect ��") - 2*CharWidth(' ');
		pstrcpy(name, gDirTop.name);
		TruncString(width, name, smTruncMiddle);
		sprintf(buttonTitle, "Select �%#s�", name);
		c2pstr(buttonTitle);
		SetCTitle((ControlHandle)itemHandleTop, (StringPtr)buttonTitle);
		ValidRect(&boxTop);
	}
	
	/* Handle clicks on Select and Cancel buttons. */
	
	if (item == kSelectDirButtonTop) {
		gSelect = 1;
		return sfItemCancelButton;
	} else if (item == kSelectDirButtonBottom) {
		gSelect = 2;
		return sfItemCancelButton;
	} else if (item == sfItemCancelButton) {
		gSelect = 0;
		return sfItemCancelButton;
	}
	
	return item;
}



/*----------------------------------------------------------------------------
	MyStandardGetDirectory 
	
	Presents a custom standard get file dialog to select a directory.
		
	Exit:	reply->sfGood = true if directory selected, false if dialog canceled.	
			reply->sfFile.vRefNum = vol ref num of selected directory.
			reply->sfFile.parID = directory id of *parent* of selected directory.
			reply->sfFile.name = name of selected directory.
			*dirID = directory id of selected directory.
----------------------------------------------------------------------------*/

void MyStandardGetDirectory (StandardFileReply *reply, long *dirID)
{
	Point point = {-1,-1};
	CInfoPBRec pb;
	
	PrepDialog();

	*gPrevNameTop = *gPrevNameBottom = 0;
	
	CustomGetFile(GetDirectoryFilter, -1, nil, reply, kDirSelectDlg, point,
		GetDirectoryHook, nil, nil, nil, reply);
		
	reply->sfGood = gSelect != 0;
	if (!reply->sfGood) return;
	
	if (gSelect == 1) {
		reply->sfFile = gDirTop;
	} else {
		reply->sfFile = gDirBottom;
	}

	pb.dirInfo.ioNamePtr = reply->sfFile.name;
	pb.dirInfo.ioVRefNum = reply->sfFile.vRefNum;
	pb.dirInfo.ioFDirIndex = 0;
	pb.dirInfo.ioDrDirID = reply->sfFile.parID;
	PBGetCatInfo(&pb, false);
	*dirID = pb.dirInfo.ioDrDirID;	   
}
