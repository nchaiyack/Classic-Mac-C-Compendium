/*______________________________________________________________________

	scn.c - Volume Scanning Module.
	
	Copyright � 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
	
	This reusable module implements volume scanning.
	
	The caller supplies a pointer to a function to be called for each file
	and folder on the volume.  In addition, the caller may supply three 
	optional rectangles which are updated as the scan progresses, for the 
	folder name currently being scanned, the file name currently being scanned, 
	and a thermometer.
	
	The module handles both HFS and MFS volumes.
	
	The caller may also specify that only a single folder or file be 
	scanned.
	
	See TN 68 for details on the scan algorithm.  The algorithm has been
	modified to improve its behaviour when other users or tasks are creating
	or deleting files or folders while the scan is in progress.  This is
	especially important on server volume scans.
	
	All of the code is placed in its own segment named "scn".
_____________________________________________________________________*/


#pragma load "precompile"
#include "scn.h"
#include "utl.h"

#pragma segment scn

/*______________________________________________________________________

	Global Variables.
_____________________________________________________________________*/


/* Global variables are used instead of local variables to minimize the size
	of the stack frames for recursive calls to the ScanCat routine.
	See TN 68. */


static scn_DoFilePtr	DoFile;			/* ptr to func to be called to process
														each file */
static scn_CheckCancelPtr		CheckCancel;
													/* ptr to func to be called to check 
														for cancel of scan */
static long				RefCon;				/* value passed through to DoFile */
static Rect				*FoldNameRect;		/* folder name rectangle, or nil */
static Rect				*FileNameRect;		/* file name rectangle, or nil */
static Rect				*ThermRect;			/* thermometer rectangle, or nil */
static Boolean			MFS;					/* true if mfs volume */
static CInfoPBRec		PBlock;				/* PBGetCatInfo param block for hfs
														scanning. */
static ParamBlockRec	MFSPBlock;			/* PBGetFInfo param block for mfs
														scanning */
static Str255			FName;				/* current folder or file name */
static Str255			OldFName;			/* old folder or file name */
static long				TotFiles;			/* total num files on vol */
static long				NumFiles;			/* num files scanned so far */
static Rect				GrayRect;			/* therm subrect to be filled */
static Boolean			Canceled;			/* true if scan canceled */
static short			NameFont;			/* font num for fold and file names */
static short			NameSize;			/* font size for fold and file names */
static short			FontNum;				/* window's font number */
static short			FontSize;			/* window's font size */
static scn_FListElHandle	FolderList;	/* handle to head of folder list */
static scn_FListElHandle	NextFolder;	/* handle to next el of folder list */
static Boolean			DoingScan = false;	/* true while scan in progress */
static Str255			FFName;				/* file name if file scan */
static short			OldIndex;			/* index in directory of file */

/*______________________________________________________________________

	DrawName - Draw File or Folder Name.
	
	Entry:	drawRect = pointer to rectangle in which the name should
					be drawn, or nil if none.
				fName = pointer to file or folder name.
_____________________________________________________________________*/


static void DrawName (Rect *drawRect, Str255 fName)

{
	Rect			rect;			/* rectangle to be erased */
	short			nameWidth;	/* width of name */
	
	if (drawRect) {
		TextFont(NameFont); TextSize(NameSize);
		rect = *drawRect;
		nameWidth = StringWidth(fName);
		TextBox(fName+1, *fName, drawRect, teJustLeft);
		if (nameWidth < (rect.right-rect.left)) {
			rect.left += nameWidth;
			EraseRect(&rect);
		};
		TextFont(FontNum); TextSize(FontSize);
	};
}

/*______________________________________________________________________

	DrawTherm - Update Thermometer.
_____________________________________________________________________*/


static void DrawTherm (void)

{
	short			newRight;			/* new right coord of therm rect */
	
	if (ThermRect) {
		NumFiles++;
		newRight = ThermRect->left + NumFiles * 
			(ThermRect->right - ThermRect->left - 1) / TotFiles;
		if (newRight > ThermRect->right) newRight = ThermRect->right;
		if (newRight > GrayRect.right) {
			GrayRect.left = GrayRect.right-1;
			GrayRect.right = newRight;
			FillRect(&GrayRect, qd.gray);
		};
	};
}

/*______________________________________________________________________

	ScanMFSVol - Scan MFS Volume.
_____________________________________________________________________*/


static void ScanMFSVol (void)

{
	short			index;				/* cur index in volume */
	
	index = 1;
	
	while (true) {
	
		MFSPBlock.fileParam.ioFDirIndex = index;
		if (PBGetFInfo(&MFSPBlock, false)) break;
			
		/*  Update the file name rectangle. */
	
		DrawName(FileNameRect, FName);
		
		/* Update the thermometer. */
		
		DrawTherm();
		
		/* Copy fields from MFSPBlock to PBlock. */
		
		PBlock.hfileInfo.ioFRefNum = MFSPBlock.fileParam.ioFRefNum;
		PBlock.hfileInfo.ioFlAttrib = MFSPBlock.fileParam.ioFlAttrib;
		PBlock.hfileInfo.ioFlFndrInfo = MFSPBlock.fileParam.ioFlFndrInfo;
		PBlock.hfileInfo.ioFlCrDat = MFSPBlock.fileParam.ioFlCrDat;
		PBlock.hfileInfo.ioFlMdDat = MFSPBlock.fileParam.ioFlMdDat;
		/* PBlock.hfileInfo.ioACUser = 0; */
		*(&PBlock.hfileInfo.ioFlAttrib+1) = 0;
					
		/* Call the user-specified function. */
		
		if (DoFile) {
			if (Canceled = 
				(*DoFile)(&PBlock, FolderList, RefCon, true)) return;
		};
		
		/* Check for user cancel. */
		
		if (CheckCancel)
			if (Canceled = (*CheckCancel)()) return;
		
		/* The following code deals with the problem of other users or
			tasks creating or deleting files while we are scanning, and also
			the case where the DoFile routine deleted the file. */
			
		utl_CopyPString(OldFName, FName);
		MFSPBlock.fileParam.ioFDirIndex = index;
		if (!PBGetFInfo(&MFSPBlock, false)) {
			if (*OldFName == *FName && !strncmp(OldFName+1, FName+1,
				*FName)) {
				index++;
				continue;
			};
		};
		OldIndex = index;
		index = 1;
		while (true) {
			MFSPBlock.fileParam.ioFDirIndex = index;
			if (PBGetFInfo(&MFSPBlock, false)) {
				index = OldIndex;
				break;
			};
			if (*OldFName == *FName && !strncmp(OldFName+1, FName+1,
				*FName)) {
					index++;
					break;
			};
			index++;
		};
				
	};
			
}

/*______________________________________________________________________

	ScanCat - Scan Catalog.
	
	Entry:	dirID = directory id of catalog to scan.
_____________________________________________________________________*/


static void ScanCat (long dirID)

{
	short			index;			/* cur index in directory */
	
	index = 1;
	
	while (true) {
	
		PBlock.hfileInfo.ioFDirIndex = index;
		PBlock.hfileInfo.ioDirID = dirID;
		/* PBlock.hfileInfo.ioACUser = 0; */
		*(&PBlock.hfileInfo.ioFlAttrib+1) = 0;
		if (PBGetCatInfo(&PBlock, false)) break;
		if (((PBlock.hfileInfo.ioFlAttrib >> 4) & 1) == 1) {
		
			/* Folder.  Update the file and folder name rectangles. */
		
			if (FileNameRect) EraseRect(FileNameRect);
			DrawName(FoldNameRect, FName);
			
			/* Call the user-specified function. */
			
			if (DoFile) {
				if (Canceled = 
					(*DoFile)(&PBlock, FolderList, RefCon, false)) return;
			};
	
			/* Check for user cancel. */
			
			if (CheckCancel)
				if (Canceled = (*CheckCancel)()) return;
				
			/* Push the folder name onto the folder list. */
			
			NextFolder =  (scn_FListElHandle) NewHandle(sizeof(scn_FListEl));
			(**NextFolder).next = FolderList;
			utl_CopyPString((**NextFolder).name, FName);
			/* (**NextFolder).accessRights = PBlock.hfileInfo.ioACUser; */
			(**NextFolder).accessRights = *(&PBlock.hfileInfo.ioFlAttrib+1);
			FolderList = NextFolder;
			
			/* Call ourselves recursively to scan the new folder. */
			
			ScanCat(PBlock.hfileInfo.ioDirID);
			if (Canceled) return;
			
			/* Pop the folder name from the folder list. */
			
			utl_CopyPString(FName, (**FolderList).name);
			NextFolder = (**FolderList).next;
			DisposHandle((Handle)FolderList);
			FolderList = NextFolder;
			
			/* Update the file and folder name rectangles. */
			
			if (FileNameRect) EraseRect(FileNameRect);
			HLock((Handle)FolderList);
			DrawName(FoldNameRect, (**FolderList).name);
			HUnlock((Handle)FolderList);
			
		} else {
		
			/* File. Update the file name rectangle. */
		
			DrawName(FileNameRect, FName);
			
			/* Update the thermometer. */
			
			DrawTherm();
			
			/* Call the user-specified function. */
			
			if (DoFile) {
				if (Canceled = 
					(*DoFile)(&PBlock, FolderList, RefCon, false)) return;
			};
	
			/* Check for user cancel. */
			
			if (CheckCancel)
				if (Canceled = (*CheckCancel)()) return;
				
		};
		
		/* The following code deals with the problem of other users or
			tasks creating or deleting files or folders while we are scanning.
			This activity can invalidate our index in the current directory.
			This problem is especially serious when scanning servers. The code
			also covers the case where the DoFile routine deleted the file or
			folder.
			
			The basic idea is to recall PBGetCatInfo to see if our position
			in the directory has changed.  If it has changed we search the
			directory to attempt to locate the new location. */
		
		utl_CopyPString(OldFName, FName);		/* save name of file or folder */
		PBlock.hfileInfo.ioDirID = dirID;
		PBlock.hfileInfo.ioFDirIndex = index;
		if (!PBGetCatInfo(&PBlock, false)) {
			if (*OldFName == *FName && !strncmp(OldFName+1, FName+1, 
				*FName)) {
				index++;
				continue;
			};
		};
		OldIndex = index;
		index = 1;
		while (true) {
			PBlock.hfileInfo.ioDirID = dirID;
			PBlock.hfileInfo.ioFDirIndex = index;
			if (PBGetCatInfo(&PBlock, false)) {
				index = OldIndex;
				break;
			};
			if (*OldFName == *FName && !strncmp(OldFName+1, FName+1, *FName)) {
				index++;
				break;
			};
			index++;
		};
	};
}

/*______________________________________________________________________

	scn_Vol - Scan Volume.
	
	Entry:	volRefNum = vol ref number of vol to be scanned.
				dirID = directory id of directory to scan, or 0 if not a 
					directory scan.
				fName = name of file to scan.  Pass empty string if not a file 
					scan.
				fVRefNum = volume or working directory ref num of directory
					containing file, if file scan.
				doFile = pointer to function to be called for each file and
					folder, or nil if none.
				refCon = a longword passed through to doFile on each call.
				checkCancel = pointer to function to be called to check for
					a cancel of the scan in progress, or nil if none.
				foldNameRect = pointer to folder name rectangle, or nil if none.
				fileNameRect = pointer to file name rectangle, or nil if none.
				thermRect = pointer to thermometer rectangle, or nil if none.
				nameFont = font number for folder and file names.
				nameSize = font size for folder and file names.
				
	Exit:		function result = true if scan was canceled.
	
	Don't forget to do a SetPort to the window before calling this routine.
	
	The folder and file names are drawn in font fontNum and size fontSize.
_____________________________________________________________________*/

				
/*______________________________________________________________________

	The caller should supply a CheckCancel routine declared as follows:
	
	Boolean CheckCancel(void)
	
	The function should return true if the scan should be canceled, 
	or false to continue the scan.  It's called once for every directory and 
	file encountered in the scan.  
	
	The function should at least call SystemTask, then call GetNextEvent
	to see if the user has typed command/period.  It can also check for a
	click on a Cancel button or whatever else you desire.  It should also
	check for update events, and call your window updating routine if one
	occurs.  Do this even in a modal environment, since screen savers like
	Moire generate update events when they deactivate, and you want your 
	window to be redrawn.
	
	If your DoFile routine does alot of processing you should
	also call CheckCancel periodically in DoFile.  If CheckCancel returns
	true, DoFile should stop doing whatever its doing and also return true
	as its function result.  This tells scn_Vol to terminate the scan.
	
	For example, in a modal environment (e.g., a modal dialog), the following
	code would suffice.  It checks for command/period, and handles update
	events:
	
	Boolean CheckCancel()
	
	{
		EventRecord		myEvent;
		
		SystemTask();
		
		if (GetNextEvent(everyEvent ^ diskMask, &myEvent) {
			if (myEvent.what == keyDown) 
				return (myEvent.modifiers & cmdKey) &&
					(myEvent.message & charCodeMask) == '.');
			else if (myEvent.what == updateEvt) 
				DoUpdate();
		};
		return false;
	}
	
	The CheckCancel routine in a non-modal environment must be more
	complicated, since it must handle a larger range of possible events.
_____________________________________________________________________*/

				
/*______________________________________________________________________

	The caller should supply a DoFile routine declared as follows:
	
	Boolean DoFile (CInfoPBRec *pBlock, scn_FListElHandle folderList, 
		long refCon, Boolean mfs)
	
	This function, like CheckCancel, is called once for every directory 
	and file encountered in the scan.	
	
	pBlock points to a PBGetCatInfo parameter block containing all sorts
	of useful information about the file or folder.  See IM IV-125 and 155
	for details.
	
	folderList is a handle to a linked list of all the folder names in 
	the current path, in reverse order (lowest-level folder name to highest-level 
	folder name).  The list is maintained as a push-down stack:  When a new folder 
	is encountered it is added to the head of the list, and it is removed when the 
	scan of that folder is complete.
	
	The first (lowest-level) folder name in the list is the name of the folder
	that contains the current file or folder.
	
	The last (highest-level) folder name in the list depends on the type of scan:
	a) Volume scans:  The root folder (volume name).
	b) Folder scans:  The folder being scanned.
	c) File scans: The folder containing the file being scanned.
	
	The folder list is terminated by a dummy element containing an empty name.
	
	Example: Suppose you are doing a volume scan of a volume named "MyVol",
	and the current file being scanned is named "MyFile".  "MyFile" is located
	inside folder "Folder1", which in turn is located inside folder "Folder2",
	which is not located inside any other folders.  When your DoFile routine
	is called for file "MyFile" the folder list contains four elements in the 
	following order:  "Folder1", "Folder2", "MyVol", "".
		
	refCon is the longword parameter passed by the caller to the ScanVol
	function.
	
	mfs is true if the volume is in mfs format.  In this case only the
	following fields in the parameter block are filled in:
	
		ioNamePtr		pointer to file name.
		ioVRefNum		volume reference number.
		ioFRefNum		path reference number.
		ioFlAttrib		file attributes.
		ioFlFndrInfo	finder info.
		ioACUser			access rights = 0 (all rights).
		ioFlCrDat		creation date/time.
		ioFlMdDat		last mod date/time.
		
	For HFS volumes the parameter block fields are left as set by the call
	to PBGetCatInfo.
	
	The function should return true if the scan should be canceled, 
	or false to continue the scan.
	
	The function may delete the file or folder.
_____________________________________________________________________*/


Boolean scn_Vol (short volRefNum, long dirID, 
	Str255 fName, short fVRefNum, scn_DoFilePtr doFile, 
	long refCon, scn_CheckCancelPtr checkCancel, 
	Rect *fldNameRect, Rect *fleNameRect, Rect *thrmRect, 
	short nameFont, short nameSize)

{
	CInfoPBRec		dBlock;				/* dir info param block */
	HParamBlockRec	vBlock;				/* vol info param block */
	WDPBRec			wdBlock;				/* wd info param block */
	scn_FListElHandle	newEl;			/* handle to new folder list el */
	Str255			dirName;				/* directory name */
	char				accessRights;		/* directory access rights */
	
	/* Copy params to global variables. */
	
	DoFile = doFile;
	RefCon = refCon;
	CheckCancel = checkCancel;
	FoldNameRect = fldNameRect;
	FileNameRect = fleNameRect;
	ThermRect = thrmRect;
	NameFont = nameFont;
	NameSize = nameSize;
	*dirName = 0;
	utl_CopyPString(FFName, fName);
	
	/* Save the current window's font number and size in global variables. */
	
	FontNum = qd.thePort->txFont;
	FontSize = qd.thePort->txSize;
	
	/* Initialize thermometer.  Disable the thermometer if the number
		of files on the volume is 0, to prevent divide by 0 errors.  Also
		disable it on folder and file scans. */
	
	if (ThermRect) {
		if (dirID || *fName) {
			EraseRect(ThermRect);
			ThermRect = nil;
		} else {
			if (TotFiles = utl_GetVolFilCnt(volRefNum)) {
				NumFiles = 0;
				GrayRect = *ThermRect;
				InsetRect(&GrayRect, 1, 1);
				GrayRect.right = GrayRect.left + 1;
			} else {
				EraseRect(ThermRect);
				ThermRect = nil;
			};
		};
	};
	
	/* Determine whether the volume is HFS or MFS. */
	
	MFS = utl_VolIsMFS(volRefNum);
	
	/* Initialize folder list */
	
	FolderList = (scn_FListElHandle) NewHandle(sizeof(scn_FListEl));
	(**FolderList).next = nil;
	*(**FolderList).name = 0;
	newEl = (scn_FListElHandle)NewHandle(sizeof(scn_FListEl));
	(**newEl).next = FolderList;
	FolderList = newEl;
	if (MFS) {
		vBlock.volumeParam.ioNamePtr = dirName;
		vBlock.volumeParam.ioVolIndex = 0;
		vBlock.volumeParam.ioVRefNum = volRefNum;
		(void) PBHGetVInfo(&vBlock, false);
		accessRights = 0;
	} else {
		dBlock.dirInfo.ioNamePtr = dirName;
		dBlock.dirInfo.ioVRefNum = volRefNum;
		dBlock.dirInfo.ioFDirIndex = -1;
		/* dBlock.dirInfo.ioACUser = 0; */
		*(&dBlock.dirInfo.ioFlAttrib+1) = 0;
		if (dirID) {
			dBlock.dirInfo.ioDrDirID = dirID;
		} else if (*fName) {
			wdBlock.ioNamePtr = dirName;
			wdBlock.ioVRefNum = fVRefNum;
			wdBlock.ioWDIndex = 0;
			wdBlock.ioWDProcID = 0;
			wdBlock.ioWDVRefNum = 0;
			(void) PBGetWDInfo(&wdBlock, false);
			dBlock.dirInfo.ioDrDirID = wdBlock.ioWDDirID;
		} else {
			dBlock.dirInfo.ioDrDirID = fsRtDirID;
		};
		(void) PBGetCatInfo(&dBlock, false);
		/* accessRights = dBlock.dirInfo.ioACUser; */
		accessRights = *(&dBlock.dirInfo.ioFlAttrib+1);
	};
	utl_CopyPString((**FolderList).name, dirName);
	(**FolderList).accessRights = accessRights;
	
	/* Draw the folder name. */
	
	if (FoldNameRect) {
		DrawName(FoldNameRect, dirName);
	};
	
	/* Draw the file name. */
	
	if (FileNameRect) EraseRect(FileNameRect);
	if (*fName) DrawName(FileNameRect, fName);
	
	/* Do the scan. */
	
	Canceled = false;
	DoingScan = true;
	
	if (*fName) {
		
		/* File scan. */
		
		if (MFS) {
			MFSPBlock.fileParam.ioNamePtr = fName;
			MFSPBlock.fileParam.ioVRefNum = volRefNum;
			MFSPBlock.fileParam.ioFVersNum = 0;
			MFSPBlock.fileParam.ioFDirIndex = 0;
			PBlock.hfileInfo.ioNamePtr = fName;
			PBlock.hfileInfo.ioVRefNum = volRefNum;
			(void) PBGetFInfo(&MFSPBlock, false);
			PBlock.hfileInfo.ioFRefNum = MFSPBlock.fileParam.ioFRefNum;
			PBlock.hfileInfo.ioFlAttrib = MFSPBlock.fileParam.ioFlAttrib;
			PBlock.hfileInfo.ioFlFndrInfo = MFSPBlock.fileParam.ioFlFndrInfo;
			PBlock.hfileInfo.ioFlCrDat = MFSPBlock.fileParam.ioFlCrDat;
			PBlock.hfileInfo.ioFlMdDat = MFSPBlock.fileParam.ioFlMdDat;
			if (DoFile) Canceled = 
				(*DoFile)(&PBlock, FolderList, RefCon, true);
		} else {
			PBlock.hfileInfo.ioNamePtr = fName;
			PBlock.hfileInfo.ioVRefNum = volRefNum;
			PBlock.hfileInfo.ioFDirIndex = 0;
			PBlock.hfileInfo.ioDirID = wdBlock.ioWDDirID;
			(void) PBGetCatInfo(&PBlock, false);
			if (DoFile) Canceled =
				(*DoFile)(&PBlock, FolderList, RefCon, false);
		};
		
	} else {
	
		/* Folder or volume scan. 
			Initialize param block. */
	
		if (MFS) {
			MFSPBlock.fileParam.ioNamePtr = FName;
			MFSPBlock.fileParam.ioVRefNum = volRefNum;
			MFSPBlock.fileParam.ioFVersNum = 0;
		};
		PBlock.hfileInfo.ioNamePtr = FName;
		PBlock.hfileInfo.ioVRefNum = volRefNum;
		
		/*	Scan the directory. */
		
		if (MFS) {
			ScanMFSVol();
		} else {
			ScanCat(dirID ? dirID : fsRtDirID);
		};
	};
	
	DoingScan = false;
	
	/* Clear the thermometer and folder and file names. */
	
	if (ThermRect) {
		GrayRect = *ThermRect;
		InsetRect(&GrayRect, 1, 1);
		EraseRect(&GrayRect);
	};
	if (FoldNameRect) EraseRect(FoldNameRect);
	if (FileNameRect) EraseRect(FileNameRect);
	
	/* Dispose of the folder list. */
	
	while (FolderList) {
		NextFolder = (**FolderList).next;
		DisposHandle((Handle)FolderList);
		FolderList = NextFolder;
	};
	
	return Canceled;
}

/*______________________________________________________________________

	scn_Update - Process an Update Event.
	
	Entry:		thermRect = pointer to thermometer rectangle, or nil if none.
	
	This routine should be called whenever an update event occurs.
	It redraws the volume, folder and file names and the thermometer if a 
	scan is in progress.
_____________________________________________________________________*/


void scn_Update (Rect *thrmRect)

{
	Rect			grayRect;		/* part of therm rect to be filled with
											gray */

	if (!DoingScan) {
		if (thrmRect) FrameRect(thrmRect);
	} else {
		if (ThermRect) {
			FrameRect(ThermRect);
			grayRect = GrayRect;
			grayRect.left = ThermRect->left + 1;
			FillRect(&grayRect, qd.gray);
		};
		if (FoldNameRect) {
			EraseRect(FoldNameRect);
			if (((PBlock.hfileInfo.ioFlAttrib >> 4) & 1) == 1) {
				DrawName(FoldNameRect, FName);
			} else {
				MoveHHi((Handle)FolderList);
				HLock((Handle)FolderList);
				DrawName(FoldNameRect, (**FolderList).name);
				HUnlock((Handle)FolderList);
			};
		};
		if (FileNameRect) EraseRect(FileNameRect);
		DrawName(FileNameRect, (*FFName) ? FFName : FName);
	};
}