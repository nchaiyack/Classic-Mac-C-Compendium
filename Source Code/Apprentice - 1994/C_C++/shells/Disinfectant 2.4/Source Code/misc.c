/*______________________________________________________________________

	misc.c - Miscellany Module
	
	Copyright � 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
	
	This module takes care of various miscellaneous tasks.
_____________________________________________________________________*/


#pragma load "precompile"
#include "utl.h"
#include "scn.h"
#include "vol.h"
#include "rep.h"
#include "rez.h"
#include "glob.h"
#include "mssg.h"
#include "wstm.h"
#include "unmount.h"
#include "misc.h"
#include "prog.h"
#include "main.h"

#pragma segment misc

/*______________________________________________________________________

	misc_HiliteScan - Hilite Scan Button.
	
	Exit:	Scan button hilited iff there is a current
			volume.
_____________________________________________________________________*/


void misc_HiliteScan(void)

{
	short			vRefNum;				/* ref num of selected vol */
	short			hiliteVal;			/* 0 to hilite, 255 to unhilite */
	
	main_SetPort();
	hiliteVal = (vol_GetSel(&vRefNum) == nsvErr) ? 255 : 0;
	HiliteControl(Controls[scanID-firstControl], hiliteVal);
}

/*______________________________________________________________________

	misc_ClearReport - Clear the Report.
_____________________________________________________________________*/


void misc_ClearReport (void)

{
	Rect				inval;			/* rect to be invalidated */

	main_SetPort();
	rep_Clear(Report);
	rep_Fill(Report, repHeadID, false);
	rep_GetRect(Report, &inval);
	InsetRect(&inval, 1, 1);
	InvalRect(&inval);
	mssg_ClearTags();
};

/*______________________________________________________________________

	LoadAll - Load all Sample Resources of a Given Type
	
	Entry:		rType = resource type.
					firstID = first resource ID.
					lastID = last resource ID.
_____________________________________________________________________*/


static void LoadAll (OSType rType, short firstID, short lastID)

{
	short				i;				/* loop index */
	short				n;				/* number of resources of type */
	Handle			h;				/* handle to resource */
	short				theID;		/* resource id */
	ResType			theType;		/* resource type */
	Str255			name;			/* resource name */
	
	n = CountResources(rType);
	SetResLoad(false);
	for (i = 1; i <= n; i++) {
		h = GetIndResource(rType, i);
		if (!*h && HomeResFile(h) == DfectRefNum) {
			GetResInfo(h, &theID, &theType, name);
			if (firstID <= theID && theID <= lastID) LoadResource(h);
		};
	};
	SetResLoad(true);
}

/*______________________________________________________________________

	misc_CheckEject - Check for Eject of System or Sample Volume
	
	Entry:		refNum = vol ref num or drive num of volume about
						to be ejected.
						
	If the system volume or Sample's volume is about to be ejected, 
	this routine loads all the resources which might be needed from the system 
	file or from the Sample file.
_____________________________________________________________________*/


void misc_CheckEject (short refNum)

{
	Handle				h;				/* handle to resource */
	short					vRefNum;		/* vol ref num */
	Str255				volName;		/* vol name */
	long					freeBytes;	/* free bytes on vol */
	Handle				hdlog;		/* handle to please wait dialog template */
	DialogPtr			dlog;			/* pointer to please wait dialog */
	Boolean				memTight;	/* true if < 300K free memory */
	static Boolean		firstTimeD=true;	/* true if haven't preloaded Sample
														resources yet */
	static Boolean		firstTimeS=true;	/* true if haven't preloaded System
														resources yet */

	/* Convert a drive number to a vol ref number. */
	
	if (refNum > 0) {
		(void) GetVInfo(refNum, volName, &vRefNum, &freeBytes);
	} else {
		vRefNum = refNum;
	};

	/* Put up "Please wait" dialog if we need to load resources for 
		the first time. */

	dlog = nil;
	if ((vRefNum == SysVol && firstTimeS) || 
		(vRefNum == DfectVol && firstTimeD)) {
		SetCursor(*Watch);
		hdlog = GetResource('DLOG', plsWaitID);
		HNoPurge(hdlog);
		utl_CenterDlogRect(*(Rect**)hdlog, false);
		dlog = GetNewDialog(plsWaitID, nil, (WindowPtr)-1);
		ShowWindow((WindowPtr)dlog);
		DrawDialog(dlog);
		if (vRefNum == SysVol) {
			firstTimeS = false;
		} else {
			firstTimeD = false;
		};
	} else {
		return;
	};
		
	/* If ejecting Sample volume, load Sample resources that 
		we might need later.  If memory is tight (<300K free memory), then
		we don't preload the Help window resources. */
	
	if (vRefNum == DfectVol) {
		memTight = FreeMem() < 300000;
		if (memTight) {
			LoadAll('PICT', 128, 199);
			LoadAll('STR#', 128, 199);
		} else {
			LoadAll('PICT', 0, 9999);
			LoadAll('STR#', 0, 9999);
			LoadAll('TCON', 0, 9999);
			LoadAll('TAG ', 0, 9999);
			LoadAll('CELL', 0, 9999);
			LoadAll('LDEF', 0, 9999);
		};
		LoadAll('WIND', 0, 9999);
		LoadAll('CNTL', 0, 9999);
		LoadAll('ALRT', 0, 9999);
		LoadAll('DITL', 0, 9999);
		LoadAll('DLOG', 0, 9999);
		LoadAll('SICN', 0, 9999);
		LoadAll('CURS', 0, 9999);
		LoadAll('CODE', 0, 9999);
		LoadAll('CREA', 0, 9999);
		LoadAll('MENU', 0, 9999);
	};
	
	/* If ejecting System volume, load system resources that we
		might need later. */

	if (vRefNum == SysVol) {
		(void) utl_CheckPack(listMgr, true);
		(void) utl_CheckPack(stdFile, true);
		(void) utl_CheckPack(dskInit, true);
		(void) utl_CheckPack(intUtil, true);
		(void) utl_CheckPack(bdConv, true);
		CouldDialog(putDlgID);
		CouldAlert(sfNoPrivs);
		CouldAlert(sfBadChar);
		CouldAlert(sfSysErr);
		CouldAlert(sfReplaceExisting);
		CouldAlert(sfDiskLocked);
		utl_DILoad();
		h = GetResource('MBDF', 0);
		h = GetResource('MDEF', 0);
		h = GetResource('itl0', 0);
		h = GetResource('ICON', stopIcon);
		h = GetResource('ICON', noteIcon);
		h = GetResource('INTL', 0);
		/* call SysBeep to force snd and snth resources to be loaded. */
		SysBeep(beepDuration);
	};
	
	/* Dispose of please wait dialog. */
	
	if (dlog) {
		DisposDialog(dlog);
		HPurge(hdlog);
	};
	
	misc_SetCursor();
}

/*______________________________________________________________________

	misc_ValPrint - Validate Print Record.
	
	Entry:	p = pointer to rpp param block.
				p->hPrint = pointer to print record, or nil if none
					yet allocated.
				report = true if this is for a report printing operation.
				report = false if this is for a document printing operation.
	
	Exit:		p->hPrint = pointer to print record.  A new one is allocated
					if necessary, and any existing one is validated.
					
				If a new print record is allocated, or if the existing one
				was invalid, then the other fields in the rpp param block
				are also initialized.  The initial values depend on whether
				this is a report or document being printed, and whether
				the output device is a LaserWriter or ImageWriter.
_____________________________________________________________________*/


void misc_ValPrint (rpp_PrtBlock *p, Boolean report)

{
	Boolean				setDefaults;	/* true to set default values */
	Boolean				laser;			/* true if laserwriter */
	char					*defFont1;		/* pointer to first default font name */
	char					*defFont2;		/* pointer to second default font name */
	short					defSize1;		/* first default size */
	short					defSize2;		/* second default size */

	setDefaults = true;
	if (p->hPrint) {
		setDefaults = PrValidate(p->hPrint);
	} else {
		p->hPrint = (THPrint)NewHandle(sizeof(TPrint));
		PrintDefault(p->hPrint);
		defSize1 = (**(p->hPrint)).prInfo.iVRes;
	};
	if (setDefaults) {
		laser = utl_IsLaser(p->hPrint);
		if (report) {
			if (laser) {
				defFont1 = defFont2 = "\pHelvetica";
			} else {
				defFont1 = defFont2 = "\pGeneva";
			};
			defSize1 = defSize2 = 10;
		} else {
			if (laser) {
				defFont1 = "\pPalatino";
				defSize1 = 10;
				defFont2 = "\pTimes";
				defSize2 = 12;
			} else {
				defFont1 = defFont2 = "\pGeneva";
				defSize1 = defSize2 = 10;
			};
		};
		p->reverseOrder = laser ? true : false;
		if (utl_GetFontNumber(defFont1, &p->fontNum)) {
			p->fontSize = defSize1;
		} else if (utl_GetFontNumber(defFont2, &p->fontNum)) {
			p->fontSize = defSize2;
		} else {
			p->fontNum = applFont;
			p->fontSize = defSize1;
		};
		p->topMargin = p->botMargin = 50;
		p->leftMargin = p->rightMargin = 100;
	};
	p->header = true;
	p->titleSep = report ? 25 : 40;
	p->titleFont = p->fontNum;
	p->titleStyle = bold;
	p->titleSize = 90;
	p->dlogID = prDlogID;
	p->tabConID = tconID;
	p->emptyPageRangeID = noPagesID;
	p->ditlID = pageSetupID;
	p->sizeRangeID = illegalSizeID;
	p->marginsTooBigID = marTooBigID;
	p->truncateRightID = truncRightID;
	p->truncateBottomID = truncBottomID;
	p->minFontSize = 1;
	p->maxFontSize = 24;
	p->updateAll = misc_Update;
}

/*______________________________________________________________________

	misc_ReadPref - Read the Preferences File.
_____________________________________________________________________*/


#pragma segment init

void misc_ReadPref (void)

{
	ParamBlockRec		pBlockMFS;		/* param block for PBSetVol call */
	WDPBRec				pBlockHFS;		/* param block for PBHSetVol call */
	Str255				fName;			/* "Sample Prefs" file name */
	OSErr					rCode;			/* result code */
	short					refNum;			/* resource file refnum */
	Boolean				error;			/* true if error encountered */
	char					version[30];	/* version number */
	Handle				thePrefs;		/* handle to prefs resource */
	Boolean				sane;				/* true if prefs resource fork is sane */
	
	/* Set the default volume and directory to the system volume and the
		blessed folder. */
	
	if (utl_VolIsMFS(SysVol)) {
		pBlockMFS.volumeParam.ioNamePtr = nil;
		pBlockMFS.volumeParam.ioVRefNum = SysVol;
		rCode = PBSetVol(&pBlockMFS, false);
	} else {
		pBlockHFS.ioNamePtr = nil;
		pBlockHFS.ioVRefNum = SysVol;
		pBlockHFS.ioWDDirID = SysDirID;
		rCode = PBHSetVol((WDPBPtr)&pBlockHFS, false);
	};
	error = rCode;
	
	/* Open the "Sample Prefs" resource file with read-only permission. */
	
	refNum = -1;
	if (!error) {
		GetIndString(fName, strListID, prefsFileName);
		if (utl_RFSanity(fName, &sane) || !sane) {
			error = true;
			utl_StopAlert(prefsBadID, nil, 0);
			FSDelete(fName, 0);
		};
		if (OldRom) {
			refNum = OpenResFile(fName);
		} else {
			refNum = OpenRFPerm(fName, 0, fsRdPerm);
		};
		error = refNum == -1;
	};
	
	/* Load the prefs resource. */
	
	if (!error) {
		thePrefs = GetResource('DPRF', 0);
		error = !thePrefs;
	};
	
	/* Check for the proper size of the resource. */
	
	if (!error) error = GetHandleSize(thePrefs) != sizeof(Prefs);
	
	/* Copy the contents of the prefs resource to the global Prefs variable. */
	
	if (!error) memcpy(&Prefs, *thePrefs, sizeof(Prefs));
	
	/* Check the version number. */
	
	if (!error) {
		GetIndString(version, strListID, prefsVersStr);
		error = !EqualString(version, Prefs.version, false, false);
	};
	
	/* Copy the print records into relocatable blocks, and store handles
		to them in the print blocks. */
		
	if (!error && Prefs.mainPrint.hPrint)
		error = PtrToHand((Ptr)&Prefs.mainPrintRec, 
			(Handle*)&Prefs.mainPrint.hPrint,
			sizeof(TPrint));
	if (!error && Prefs.helpPrint.hPrint)
		error = PtrToHand((Ptr)&Prefs.helpPrintRec, 
			(Handle*)&Prefs.helpPrint.hPrint,
			sizeof(TPrint));
	
	/* If any error was encountered, initialize the Prefs global variable to
		default values. */
		
	if (error) {
		GetIndString(Prefs.version, strListID, prefsVersStr);
		wstm_Init(&Prefs.mainState);
		wstm_Init(&Prefs.helpState);
		wstm_Init(&Prefs.prefState);
		wstm_Init(&Prefs.abouState);
		Prefs.mainPrint.hPrint = nil;
		Prefs.helpPrint.hPrint = nil;
		Prefs.numOpenWind = 1;
		Prefs.openWind[0] = mainWind;
		Prefs.helpScrollPos = 0;
		Prefs.beepCount = 0;
		Prefs.scanningStation = false;
		Prefs.scanningStationOp = checkOp;
		Prefs.repCreator = 'ttxt';
		Prefs.docCreator = 'MSWD';
		Prefs.repOtherCre = '????';
		Prefs.docOtherCre = '????';
		Prefs.notifOption = notifIcon;
	};
	
	/* Close the resource file. */
	
	if (refNum != -1) CloseResFile(refNum);
}

#pragma segment misc

/*______________________________________________________________________

	misc_WritePref - Write the Preferences File.
_____________________________________________________________________*/


void misc_WritePref (void)

{
	ParamBlockRec		pBlockMFS;		/* param block for PBSetVol call */
	WDPBRec				pBlockHFS;		/* param block for PBHSetVol call */
	Str255				fName;			/* "Sample Prefs" file name */
	OSErr					rCode;			/* result code */
	short					refNum;			/* resource file refnum */
	Boolean				error;			/* true if error encountered */
	FInfo					fndrInfo;		/* Finder info */
	Handle				thePrefs;		/* handle to prefs resource */
	short					numOpenWind;	/* number of open windows */
	WindowPtr			curWind;			/* ptr to cur window */
	WindowObject		*curObj;			/* ptr to cur window object */
	WindowPtr			nextWind;		/* ptr to next window */
	WindKind				windKind;		/* window kind */
	
	/* Walk the window list.  Close all windows, and save the open window
		states and orders in the Prefs variable. */
		
	numOpenWind = 0;
	curWind = FrontWindow();
	while (curWind) {
		nextWind = (WindowPtr)((WindowPeek)curWind)->nextWindow;
		windKind = misc_GetWindKind(curWind);
		if (windKind != daWind) {
			Prefs.openWind[numOpenWind] = windKind;
			numOpenWind++;
			curObj = (WindowObject*)((WindowPeek)curWind)->refCon;
			(*curObj->close)();
		};
		curWind = nextWind;
	};
	Prefs.numOpenWind = numOpenWind;
	
	/* Copy the print records into the Prefs variable. */
	
	if (Prefs.mainPrint.hPrint) 
		memcpy(&Prefs.mainPrintRec, *Prefs.mainPrint.hPrint, sizeof(TPrint));
	if (Prefs.helpPrint.hPrint)
		memcpy(&Prefs.helpPrintRec, *Prefs.helpPrint.hPrint, sizeof(TPrint));
	
	/* Set the default volume and directory to the system volume and the
		blessed folder. */
	
	if (utl_VolIsMFS(SysVol)) {
		pBlockMFS.volumeParam.ioNamePtr = nil;
		pBlockMFS.volumeParam.ioVRefNum = SysVol;
		rCode = PBSetVol(&pBlockMFS, false);
	} else {
		pBlockHFS.ioNamePtr = nil;
		pBlockHFS.ioVRefNum = SysVol;
		pBlockHFS.ioWDDirID = SysDirID;
		rCode = PBHSetVol((WDPBPtr)&pBlockHFS, false);
	};
	error = rCode;
	
	/* Open the "Sample Prefs" resource file with read/write permission.
		If it doesn't exist, create it with type D2C1 and creator D2CT. */
	
	refNum = -1;
	if (!error) {
		GetIndString(fName, strListID, prefsFileName);
		if (OldRom) {
			refNum = OpenResFile(fName);
		} else {
			refNum = OpenRFPerm(fName, 0, fsRdWrPerm);
		};
		if (refNum == -1) {
			CreateResFile(fName);
			if (!(error = ResError())) {
				if (!(error = GetFInfo(fName, 0, &fndrInfo))) {
					fndrInfo.fdType = 'D2C1';
					fndrInfo.fdCreator = 'D2CT';
					if (!(error = SetFInfo(fName, 0, &fndrInfo))) {
						if (OldRom) {
							refNum = OpenResFile(fName);
						} else {
							refNum = OpenRFPerm(fName, 0, fsRdWrPerm);
						};
						error = refNum == -1;
					};
				};
			};
		};
	};
	
	/* Load the prefs resource.  If no such resource exists, create one. */
	
	if (!error) {
		thePrefs = GetResource('DPRF', 0);
		if (!thePrefs) {
			thePrefs = NewHandle(sizeof(Prefs));
			AddResource(thePrefs, 'DPRF', 0, "\p");
			error = ResError();
		};
	};
	
	/* Set the proper size of the resource and copy the Prefs global variable
		to the resource. */
	
	if (!error) error = PtrToXHand((Ptr)&Prefs, thePrefs, sizeof(Prefs));
	
	/* Mark the resource changed. */
	
	if (!error) ChangedResource(thePrefs);
	
	/* Close the resource file. */
	
	if (refNum != -1) CloseResFile(refNum);
}

/*______________________________________________________________________

	misc_GetWindKind - Get Window Kind.
	
	Entry:	theWindow = pointer to window.
	
	Exit:		function result = window kind.
_____________________________________________________________________*/


WindKind misc_GetWindKind (WindowPtr theWindow)

{
	return utl_IsDAWindow(theWindow) ?
		daWind :
		((WindowObject*)((WindowPeek)theWindow)->refCon)->windKind;
}

/*______________________________________________________________________

	misc_Notify - Notify User.
	
	Entry:	msg = index in STR# resource of message.
				wait = true to wait until brought to foreground.
_____________________________________________________________________*/


void misc_Notify (short msg, Boolean wait)

{
	if (InForeground) return;
	if (utl_SysHasNotMgr && !Notified) {
		Notified = true;
		NotifRec.qType = nmType;
		NotifRec.nmMark = 1;
		NotifRec.nmSIcon = Prefs.notifOption == notifDiamond ? nil : 
			GetResource('SICN', smallDfectIconID);
		if (Prefs.notifOption == notifAlert) {
			NotifRec.nmSound = (Handle)-1;
			NotifRec.nmStr = NotifString;
			GetIndString(NotifString, strListID, msg);
			NotifRec.nmResp = (ProcPtr)-1;
		} else {
			NotifRec.nmSound = nil;
			NotifRec.nmStr = nil;
			NotifRec.nmResp = nil;
		};
		NMInstall((QElemPtr)&NotifRec);
	};
	if (!wait) return;
	while (!InForeground) prog_Event();
}

/*______________________________________________________________________

	misc_Update - Process All Pending Update Events.
_____________________________________________________________________*/


void misc_Update (void)

{
	EventRecord			event;

	while (EventAvail(updateMask, &event)) prog_Event();
}

/*______________________________________________________________________

	misc_PrintError - Handle Printing Error.
_____________________________________________________________________*/


void misc_PrintError (OSErr rCode)

{
	Str255			rCodeStr;	/* result code as a string */
	
	if (rCode && rCode != iPrAbort) {
		if (rCode == fnfErr) {
			utl_StopAlert(noDriverID, nil, 0);
		} else if (rCode == resNotFound) {
			utl_StopAlert(noPrinterID, nil, 0);
		} else  if (rCode == dskFulErr) {
			utl_StopAlert(printDskFullID, nil, 0);
		} else if (rCode == wPrErr) {
			utl_StopAlert(printLockedID, nil, 0);
		} else {
			NumToString(rCode, rCodeStr);
			ParamText(rCodeStr, nil, nil, nil);
			utl_StopAlert(printErrID, nil, 0);
		};
	};
}
	
/*______________________________________________________________________

	misc_SetCursor - Set Cursor.
_____________________________________________________________________*/


void misc_SetCursor (void)

{
	if (HelpMode) {
		SetCursor(*HelpCurs);
	} else {
		InitCursor();
	};
}
