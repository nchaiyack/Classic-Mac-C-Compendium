/*______________________________________________________________________

	scan.c - Scanning Module
	
	Copyright � 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
	
	This module takes care of various tasks involving scanning.
_____________________________________________________________________*/


#pragma load "precompile"
#include "utl.h"
#include "rep.h"
#include "vol.h"
#include "rez.h"
#include "scn.h"
#include "gff.h"
#include "glob.h"
#include "mssg.h"
#include "misc.h"
#include "prog.h"
#include "main.h"
#include "scan.h"

#pragma segment scan

/*______________________________________________________________________

	Global Variables.
_____________________________________________________________________*/


static short		InsertedVRefNum;	/* vol ref num of inserted disk */
static Boolean		BadDisk;				/* true if uninitialized disk inserted */

/*______________________________________________________________________

	scan_CheckCancel - Check for Cancel of Scan in Progress.
	
	Exit:			function result = true if canceled, else false.
	
	This routine is called by the scanning routines to see if the operation 
	has been canceled (command-period or for a click on the cancel
	button).
_____________________________________________________________________*/


Boolean scan_CheckCancel(void)

{
	prog_Event();
	return Canceled;
}

/*______________________________________________________________________

	scan_CheckFull - Check to See if the Report is Full.
	
	Entry:		report = report handle.
					slop = number of bytes to guarantee free.
					
	Exit:			function result = true if cancel button selected or error
					saving report.
_____________________________________________________________________*/


Boolean scan_CheckFull (Handle report, short slop)

{
	short				item;					/* alert item number */
	Str255			line1;				/* unplugged message */
	Str255			line2;				/* plugged message */
	Str255			vName;				/* vol name */

	if (rep_Full(report, slop)) {
		if (!Prefs.scanningStation) {
			misc_Notify(requiresAttn, true);
			item = utl_StopAlert(repFullID, nil, 2);
			switch (item) {
				case 1:		/* Save */
					if (!main_Save()) return true;
					break;
				case 2:		/* Cancel */
					return true;
				case 3:		/* Clear */
					break;
			};
			misc_Update();
		};
		misc_ClearReport();
		misc_Update();
		rep_Append(report, "\p", false, false);
		rep_Append(report, "\p===========================================",
			true, true);
		rep_Append(report, "\p", true, true);
		GetIndString(line1, strListID, repContStr);
		vol_GetName(vName);
		utl_PlugParams(line1, line2, &vName, nil, nil, nil);
		rep_Append(report, line2, true, true);
		rep_Append(report, "\p", true, true);
	};
	return Canceled;
};

/*______________________________________________________________________

	DrawVolList - Draw Volume List in Dialog.
	
	Entry:	dlog = pointer to volume list dialog.
				itemNo = item number of volume list rectangle.
_____________________________________________________________________*/


static pascal void DrawVolList (WindowPtr dlog, short itemNo)

{
	short					itemType;	/* type of dialog item */
	Handle				itemHandle;	/* handle to dialog item */
	Rect					itemBox;		/* rectangle enclosing dialog item */
	ListHandle			volList;		/* volume list */	
	
	volList = (ListHandle)((WindowPeek)dlog)->refCon;
	LUpdate(dlog->visRgn , volList);
	GetDItem(dlog, itemNo, &itemType, &itemHandle, &itemBox);
	FrameRect(&itemBox);
}

/*______________________________________________________________________

	OutlineOK - Outline OK button in Dialog.
	
	Entry:	dlog = pointer to volume list dialog.
				itemNo = item number of outline OK button useritem.
_____________________________________________________________________*/


static pascal void OutlineOK (WindowPtr dlog, short itemNo)

{
	short					itemType;	/* type of OK button item */
	Handle				itemHandle;	/* handle to OK button item */
	Rect					itemBox;		/* rectangle enclosing OK button item */	
	
#pragma unused (itemNo)
	
	GetDItem(dlog, 1, &itemType, &itemHandle, &itemBox);
	PenSize(3, 3);
	InsetRect(&itemBox, -4, -4);
	FrameRoundRect(&itemBox, 16, 16);
	PenSize(1, 1);
}

/*______________________________________________________________________

	VolListFilter - Filter Proc for Volume List Dialog.
	
	Entry:	dlog = pointer to volume list dialog.
				theEvent = pointer to event record.
				
	Exit:		itemHit = 3 if hit in volume list rectangle.
						  = 1 if enter or return key pressed or double-click.
						  = 2 if command period or escape pressed.
				function result = true if hit in volume list rectangle
					or enter or return key or command period or escape pressed.
_____________________________________________________________________*/

static pascal Boolean VolListFilter (WindowPtr dlog, EventRecord *theEvent,
	short *itemHit)
	
{
	short			key;			/* ascii code of key pressed */
	Point			where;		/* location of mouse down */
	short			itemType;	/* type of dialog item */
	Handle		itemHandle;	/* handle to dialog item */
	Rect			itemBox;		/* rectangle enclosing dialog item */	
	ListHandle	volList;		/* volume list */
	Cell			dClickCell;	/* cell double-clicked on */

	SetPort(dlog);
	switch (theEvent->what) {
		case mouseDown:
			where = theEvent->where;
			GlobalToLocal(&where);
			GetDItem(dlog, 3, &itemType, &itemHandle, &itemBox);
			if (PtInRect(where, &itemBox)) {
				volList = (ListHandle)((WindowPeek)dlog)->refCon;
				if (LClick(where, theEvent->modifiers, volList)) {
					/* double click - select clicked cell and return 1 */
					dClickCell = LLastClick(volList);
					LSetSelect(true, dClickCell, volList);
					*itemHit = 1;
					return true;
				} else {
					/* single click = return 3 */
					*itemHit = 3;
					return true;
				};
			};
			break;
		case keyDown:
		case autoKey:
			key = theEvent->message & charCodeMask;
			if (key == returnKey || key == enterKey) {
				*itemHit = 1;
				utl_FlashButton(dlog, 1);
				return true;
			};
			if ((key == '.' && theEvent->modifiers & cmdKey) ||
				(key == escapeKey)) {
				*itemHit = 2;
				utl_FlashButton(dlog, 2);
				return true;
			};
			break;
	};
	return false;
}

/*______________________________________________________________________

	FolderFilter - Folder Filter Function for Modified Get File Dialog.
	
	Entry:	pBlock = pointer to GetFileInfo param block.
				
	Exit:		function result = true if file, false if folder.
	
	This function is used by the scan_DoScan function below to process the
	"Folder" menu item in the Scan and Disinfect menus.  It
	filters out all but the folder names in the modified get file
	dialog.
_____________________________________________________________________*/


static pascal Boolean FolderFilter (ParmBlkPtr pBlock)

{
	return (pBlock->fileParam.ioFlAttrib & 0x10) == 0;
}

/*______________________________________________________________________

	Enumerate - Enumerate file.
	
	
	Entry:		pBlock = pointer to PBGetCatInfo param block.
					folderList = handle to folder list.
					refCon = report handle.
					mfs = true if mfs volume.
					
	Exit:			function result = true if scan should be canceled.
_____________________________________________________________________*/


static Boolean Enumerate (CInfoPBRec *pBlock, scn_FListElHandle folderList, 
	long refCon, Boolean mfs)

{
	Str255			line;				/* report line */
	char				*p;				/* ptr to current position in report line */
	
#pragma unused (pBlock, refCon, mfs)	
	
	/* Check to see if the report is full. */
	
	if (scan_CheckFull(Report, 3000)) return true;
	
	/* Bump counters. */
	
	TotFiles++;
	mssg_BumpCounter(0);
	
	/* Put three spaces in the report line for each folder level. */
	
	p = line+1;
	folderList = (**folderList).next;
	while (folderList=(**folderList).next) {
		*p++ = ' ';
		*p++ = ' ';
		*p++ = ' ';
	};
	
	/* Copy file or folder name to report line */
	
	memcpy(p, pBlock->hfileInfo.ioNamePtr+1, *pBlock->hfileInfo.ioNamePtr);
	p += *pBlock->hfileInfo.ioNamePtr;
	
	/* Append (�) for folders. */
	
	if (((pBlock->hfileInfo.ioFlAttrib >> 4) & 1) == 1) {
		*p++ = ' ';
		*p++ = '(';
		*p++ = '�';
		*p++ = ')';
	};
	
	/* Store report line length. */
	
	*line = p - (line + 1);
	
	/* Write the line to the report. */
	
	rep_Append(Report, line, true, true);
	return false;
}

/*______________________________________________________________________

	scan_DoScan - Process a Scan or Disinfect Operation.
	
	Entry:		scanKind = type of scan (volScan, foldScan, fileScan, 
						etc.)
					scanOp = scanning operation (checkOp or disinfectOp).
_____________________________________________________________________*/


void scan_DoScan (ScanKind scanKind, ScanOp scanOp)

{
	unsigned char		saveHilite[numControls];
											/* saved control hilite states */
	short					i;				/* loop index */
	short					vRefNum;		/* vol ref num of vol containing object
												being scanned */
	long					dirID;		/* dir id of folder if folder scan */
	char					*fName;		/* ptr to file name if file scan */
	short					fVRefNum;	/* wd or vol ref num of folder or vol
												containing file if file scan */
	scn_DoFilePtr		doFile;		/* ptr to do file function for scan */												
	Boolean				infected;	/* true if infected file found */
	Boolean				sysInfected;  /* true if infected file found in
												  currently active system folder */
	Handle				h;				/* handle to DLOG resource */
	Point					where;		/* loctn of get file or folder dialog */
	SFReply				reply;		/* reply from gff_Get */
	Boolean				wait;			/* true to wait for disk insertion */
	short					volInx;		/* index in VCB queue */
	HParamBlockRec		vBlock;		/* vol info param block */
	ParamBlockRec		fBlock;		/* file info param block */
	DrvQEl				*curDrive;	/* ptr to drive queue element */
	unsigned char		flagByte;	/* drive queue el flag byte */
	short					driveNum;	/* drive number */
	Boolean				firstDisk;	/* true if asking for first disk */
	OSErr					rCode;		/* result code */
	WDPBRec				wdBlock;		/* working directory info param block */
	FCBPBRec				fcbBlock;	/* FCB info param block */
	char					sysFileName[32];	/* system file name */
	DialogPtr			dlog;			/* pointer to vol list dialog */
	Rect					dlogRect;	/* dialog rectangle */
	short					itemHit;		/* item number hit in vol list dialog */
	short					itemType;	/* type of dialog item */
	Handle				itemHandle;	/* handle to dialog item */
	Rect					itemBox;		/* rectangle enclosing dialog item */
	ListHandle			volList;		/* volume list */
	Rect					rView;		/* rectangle enclosing volume list */
	Rect					dataBounds;	/* vol list dimensions */
	Point					volPt;		/* vol list cell */
	char					volName[30];	/* vol name */
	Handle				vrn;			/* handle to list of vRefNums */
	short					vrnSize;		/* size of list of vRefNums */
	short					vrnInx;		/* cur index in list of vRefNums */
	short					lastVrn;		/* index in list of vRefNums following last
												entry */
	GrafPtr				thePort;		/* saved grafport */
	short					strInx;		/* index of STR# resource string */
	char					scanOpWord[20];	/* "scanned" or "disinfected" */
	Str255				prompt1;		/* unplugged prompt string for get
												file/folder dialog */
	Str255				prompt2;		/* plugged prompt string for get 
												file/folder dialog */
												
	static char			*desktopName = "\pDesktop";		/* Finder Desktop file name */
	
	/* Remember the scan kind in a global variable, and initialize 
		Canceled to false. */
	
	CurScanKind = scanKind;
	Canceled = false;
	
	/* Get currently selected volume. */

	(void) vol_GetSel(&vRefNum);
	
	/* Get pointer to do file function. */
	
	switch (scanOp) {
		case checkOp:
			doFile = Enumerate;
			break;
	};
	
	/* Initialize depending on type of scan.  Present open file, open folder,
		or open file/folder dialog if necessary.  Get the following info:
		
		vRefNum = vol ref num of vol containing object to be scanned.
		dirID = dir id of folder if folder scan.
		fName = pointer to file name if file scan.
		fVRefNum = vol or wd ref num of vol or folder containing file
			if file scan. */
		
	dirID = 0;
	fName = "\p";
	fVRefNum = 0;
	wait = false;
	
	if (scanKind == fileScan || scanKind == foldScan || 
		scanKind == fileFoldScan || scanKind == volSetScan) {
		switch (scanOp) {
			case checkOp:
				strInx = scannedWord;
				break;
		};
		GetIndString(scanOpWord, strListID, strInx);
	};
	
	switch (scanKind) {
	
		case allScan:
		
			volInx = 0;
			break;
			
		case volScan:
		
			break;
			
		case fileScan:
		case foldScan:
		case fileFoldScan:
		
			/* Set standard file dialog default vol to currently selected vol. */
			
			if (-vRefNum != *(short*)SFSaveDisk) {
				*(short*)SFSaveDisk = -vRefNum;
				if (!OldRom) *(long*)CurDirStore = fsRtDirID;
			};
			
			/* Load the proper get file dialog and center it. */
			
			h = GetResource('DLOG', 
				(scanKind == fileScan) ? getFileID : getFFID);
			HNoPurge(h);
			utl_CenterDlogRect(*(Rect**)h, MenuPick);
			where = **(Point**)h;
			
			/* Present the dialog. */
			
			switch (scanKind) {
				case fileScan:
					strInx = getFilePrompt;
					break;
				case foldScan:
					strInx = getFoldPrompt;
					break;
				case fileFoldScan:
					strInx = getFFPrompt;
					break;
			};
			GetIndString(prompt1, strListID, strInx);
			utl_PlugParams(prompt1, prompt2, scanOpWord, nil, nil, nil);
			switch (scanKind) {
				case fileScan:
					SFPGetFile(where, prompt2, nil, -1, nil, nil, &reply,
						getFileID, nil);
					break;
				case foldScan:
					gff_Get(&where, prompt2, FolderFilter, -1, nil, &reply, getFFID);
					break;
				case fileFoldScan:
					gff_Get(&where, prompt2, nil, -1, nil, &reply, getFFID);
					break;
			};
			misc_Update();
			
			/* Clean up and get out if the user clicked on the cancel button. */
			
			HPurge(h);
			if (!reply.good) return;
			
			/* Set CurScanKind = kind of scan (file or folder), and
				set other variables. */
				
			switch (scanKind) {
				case fileScan:
					/* call GetWDInfo to get vRefNum = ref num of vol containing 
						file */
					wdBlock.ioNamePtr = nil;
					wdBlock.ioVRefNum = reply.vRefNum;
					wdBlock.ioWDIndex = 0;
					wdBlock.ioWDProcID = 0;
					PBGetWDInfo(&wdBlock, false);
					vRefNum = wdBlock.ioWDVRefNum;
					fName = reply.fName;
					fVRefNum = reply.vRefNum;
					break;
				case foldScan:
					if (reply.fType == fsRtDirID) CurScanKind = volScan;
					vRefNum = reply.vRefNum;
					break;
				case fileFoldScan:
					CurScanKind = *reply.fName ? fileScan : foldScan;
					if (CurScanKind == fileScan) {
						vRefNum = reply.version;
						fName = reply.fName;
						fVRefNum = reply.vRefNum;
					} else {
						if (reply.fType == fsRtDirID) CurScanKind = volScan;
						vRefNum = reply.vRefNum;
					};
			};
			if (CurScanKind == foldScan) dirID = reply.fType;
			break;
			
		case autoScan:
		
			/* Check drives 1 and 2 (internal and external floppy drives).
				Eject them if they are not empty.  See IM IV-181.
				
				Set wait=true and firstDisk=true to wait for insertion
				of first disk. */
				
			firstDisk = wait = true;
			curDrive = (DrvQEl*)(GetDrvQHdr())->qHead;
			while (curDrive) {
				driveNum = curDrive->dQDrive;
				if (driveNum == 1 || driveNum == 2) {
					flagByte = *((Ptr)curDrive-3);
					if (flagByte && flagByte < 0xfc) {
						misc_CheckEject(driveNum);
						Eject(nil, driveNum);
					};
				};
				curDrive = (DrvQEl*)curDrive->qLink;
			};
			break;
			
		case sysFileScan:
		
			CurScanKind = fileScan;
			vRefNum = SysVol;
			fVRefNum = utl_GetSysWD();
			fcbBlock.ioVRefNum = 0;
			fcbBlock.ioFCBIndx = 0;
			fcbBlock.ioRefNum = SysRefNum;
			fcbBlock.ioNamePtr = sysFileName;
			PBGetFCBInfo(&fcbBlock, false);
			fName = sysFileName;
			break;
			
		case sysFoldScan:
		
			vRefNum = SysVol;
			if (utl_VolIsMFS(vRefNum) || SysDirID == fsRtDirID) {
				CurScanKind = volScan;
			} else {
				CurScanKind = foldScan;
				dirID = SysDirID;
			};
			break;
			
		case volSetScan:
		
			/* Initialize the dialog. */
			
			GetPort(&thePort);
			dlog = GetNewDialog(volListID, nil, (WindowPtr)-1);
			dlogRect = dlog->portRect;
			utl_CenterDlogRect(&dlogRect, MenuPick);
			MoveWindow(dlog, dlogRect.left, dlogRect.top, false);
			GetDItem(dlog, 3, &itemType, &itemHandle, &rView);
			SetDItem(dlog, 3, itemType, (Handle)DrawVolList, &rView);
			GetDItem(dlog, 4, &itemType, &itemHandle, &itemBox);
			SetDItem(dlog, 4, itemType, (Handle)OutlineOK, &itemBox);
			GetDItem(dlog, 5, &itemType, &itemHandle, &itemBox);
			GetIText(itemHandle, prompt1);
			utl_PlugParams(prompt1, prompt2, scanOpWord, nil, nil, nil);
			SetIText(itemHandle, prompt2);
			
			/* Initialize an empty volume list. */
			
			InsetRect(&rView, 1, 1);
			rView.right -= 15;
			SetRect(&dataBounds, 0, 0, 1, 0);
			SetPt(&volPt, 0, 0);
			volList = LNew (&rView, &dataBounds, volPt, 0, dlog, 
				false, false, false, true);
			(**volList).selFlags = lNoExtend | lUseSense;
			((WindowPeek)dlog)->refCon = volList;
			
			/* Fill the volume list with the names of all the online volumes.
				Also record their vRefNums in the vrn array. */
				
			vrn = NewHandle(20);
			vrnSize = 20;
			vrnInx = 0;
			volInx = 0;
			vBlock.volumeParam.ioNamePtr = volName;
			while (true) {
				vBlock.volumeParam.ioVolIndex = ++volInx;
				vBlock.volumeParam.ioVRefNum = 0;
				if (PBHGetVInfo(&vBlock, false)) break;
				if (vBlock.volumeParam.ioVDrvInfo) {
					LAddRow(1, volPt.v, volList);
					LSetCell(volName+1, *volName, volPt, volList);
					volPt.v++;
					if (vrnInx >= vrnSize) {
						vrnSize += 20;
						SetHandleSize(vrn, vrnSize);
					};
					*(short*)(*vrn + vrnInx) = vBlock.volumeParam.ioVRefNum;
					vrnInx += 2;
				};
			};
			
			/* Show the window and process events. */
			
			LDoDraw(true, volList);
			ShowWindow((WindowPtr)dlog);
			do {
				ModalDialog(VolListFilter, &itemHit);
			} while (itemHit != ok && itemHit != cancel);
			
			/* Collapse the list of vRefNums to just those that have
				been selected by the user. */
			
			vrnInx = 0;
			volPt.v = 0;
			while (LGetSelect(true, &volPt, volList)) {
				*(short*)(*vrn + vrnInx) = *(short*)(*vrn + (volPt.v<<1));
				vrnInx += 2;
				volPt.v++;
			};
			lastVrn = vrnInx;
			vrnInx = 0;
			
			/* Dispose of the volume list and the dialog, leaving just the
				vRefNum list. */
			
			LDispose(volList);
			DisposDialog(dlog);
			SetPort(thePort);
			misc_Update();
			
			/* If the cancel button was pressed, or if there are no selected
				volumes in the list, dispose of the vRefNum list and return. */
				
			if (itemHit == cancel || !lastVrn) {
				DisposHandle(vrn);
				return;
			};
			break;
	
		case desktopScan:
		
			volInx = 0;
			CurScanKind = fileScan;
			break;
			
	};
	
	HiliteMenu(0);
	
	/* Set selected volume and process resulting update event. */
	
	vol_SetSel(vRefNum, true);
	misc_Update();	
	Scanning = true;

	/* Save old hilite states for all buttons.  Unhilite all buttons
		except for Cancel and Quit.  Also unhilite the report scroll bar. */

	for (i=0; i<numControls; i++) {
		saveHilite[i] = (**Controls[i]).contrlHilite;
		if (i+firstControl == cancelID || i+firstControl == quitID) {
			HiliteControl(Controls[i], 0);
		} else {
			HiliteControl(Controls[i], 255);
		};
	};
	main_SetPort();
	rep_Activate(Report, false);
				
	/* Main loop - do one volume at a time. */
	
	while (true) {
	
		/* If wait=true wait for the user to insert the next floppy. */
		
		if (wait) {
			main_WaitInsert(firstDisk, &vRefNum, &Canceled);
			firstDisk = false;
			if (Canceled) {
				vol_Verify();
				break;
			};
			vol_SetSel(vRefNum, false);
			misc_Update();
		};
		
		/* If allScan advance to next volume.  If desktopScan advance to
			next volume which contains a desktop file. */
		
		if (scanKind == allScan || scanKind == desktopScan) {
			vBlock.volumeParam.ioNamePtr = nil;
			while (true) {
				vBlock.volumeParam.ioVolIndex = ++volInx;
				vBlock.volumeParam.ioVRefNum = 0;
				if (rCode = PBHGetVInfo(&vBlock, false)) break;
				if (!vBlock.volumeParam.ioVDrvInfo) continue;
				if (scanKind == allScan) break;
				/* check to see if Desktop file exists. */
				fBlock.fileParam.ioNamePtr = desktopName;
				fBlock.fileParam.ioVRefNum = vBlock.volumeParam.ioVRefNum;
				fBlock.fileParam.ioFVersNum = 0;
				fBlock.fileParam.ioFDirIndex = 0;
				if (PBGetFInfo(&fBlock, false)) continue;
				fName = desktopName;
				fVRefNum = vBlock.volumeParam.ioVRefNum;
				break;
			};
			if (rCode) break;
			vRefNum = vBlock.volumeParam.ioVRefNum;
			vol_SetSel(vRefNum, false);
			misc_Update();
		};
		
		/* If volSetScan advance to next selected volume. */
		
		if (scanKind == volSetScan) {
			if (vrnInx >= lastVrn) break;
			vRefNum = *(short*)(*vrn + vrnInx);
			vrnInx += 2;
			vol_SetSel(vRefNum, false);
			misc_Update();
		};
		
		/* Check for report full. */
		
		if (scan_CheckFull(Report, 6000)) break; 
			
		/* Output report header. */
		
		mssg_Begin(false, dirID, 
			fName, fVRefNum, vRefNum, RectList[counterRect].top,
			RectList[counterRect].right);
			
		/* Find out if volume is local (for Zig). */
		
		vBlock.volumeParam.ioNamePtr = nil;
		vBlock.volumeParam.ioVRefNum = vRefNum;
		vBlock.volumeParam.ioVolIndex = 0;
		PBHGetVInfo(&vBlock, false);
		VolIsLocal = vBlock.volumeParam.ioVFSID == 0;
		
		/* Scan the volume. */
		
		Canceled = scn_Vol(vRefNum, dirID, fName, fVRefNum, doFile,
			(long)Report, scan_CheckCancel, &RectList[foldNameRect], 
			&RectList[fileNameRect], &RectList[thermRect], systemFont, 0); 
		
		/* Redraw the thermometer frame. */
		
		FrameRect(&RectList[thermRect]);
		
		/* Output report summary. */
		
		mssg_End(false, Canceled, &infected, &sysInfected);
		
		/* Break out of loop if not allScan or autoScan or volSetScan or desktopScan, 
			or if canceled. */
		
		if (scanKind != allScan && scanKind != autoScan && 
			scanKind != volSetScan && scanKind != desktopScan) break;
		if (Canceled) break;
		
		/* If autoscan, eject floppy.  Also unmount it if it's not the
			current system volume or the volume containing the current 
			application.  Set wait=true to wait for the user to insert the
			next floppy. */
			
		if (scanKind == autoScan) {
			misc_CheckEject(vRefNum);
			Eject(nil, vRefNum);
			if (vRefNum != utl_GetSysVol() && vRefNum != utl_GetApplVol()) 
				UnmountVol(nil, vRefNum);
			wait = true;
		};
	
	};
	
	Scanning = false;
	
	/* Restore saved hilite states. 
		Disable the Disinfect button if requested by vrep_Repair.
		Also hilite the scroll bar. */
	
	for (i=0; i<numControls; i++) {
		HiliteControl(Controls[i], saveHilite[i]);
	};
	rep_Activate(Report, true);
	
	/* Verify the current volume. */
	
	vol_Verify();
	
	/* If volSetScan, dispose of vRefNum list. */

	if (scanKind == volSetScan) {
		DisposHandle(vrn);
	};
	misc_SetCursor();
}

/*______________________________________________________________________

	scan_DoButton - Process a Mouse Down on One of the Two Scanning Buttons
	
	Entry:		scanOp = scanning operation (checkOp or disinfectOp).
					modifiers = command key modifiers.
_____________________________________________________________________*/


void scan_DoButton (ScanOp scanOp, short modifiers)

{
	Boolean				optionDown;	/* true if option key held down */
	Boolean				cmdDown;		/* true if command key held down */
	short					scanKind;	/* kind of scan */

	/* Check for modifier keys. */
	
	optionDown = (modifiers & optionKey) != 0;
	cmdDown = (modifiers & cmdKey) != 0;
	
	/* Determine kind of scan. */
	
	if (optionDown) {
		if (cmdDown) {
			scanKind = allScan;				/* option and command */
		} else {
			scanKind = fileFoldScan;		/* option */
		};
	} else {
		if (cmdDown) {
			scanKind = autoScan;				/* command */
		} else {
			scanKind = volScan;				/* no modifer keys */
		};
	};
	
	/* Call scan_DoScan to do the scan. */
	
	scan_DoScan(scanKind, scanOp);
}
