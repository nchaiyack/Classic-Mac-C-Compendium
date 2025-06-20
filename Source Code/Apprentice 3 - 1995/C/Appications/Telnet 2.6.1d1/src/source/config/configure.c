/*	Configure.c
*	New Configuration scheme (the death of config.tel)
*
*****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1993,									*
*	Board of Trustees of the University of Illinois				*
*****************************************************************
*  Revisions:
*  8/92		Telnet 2.6:	Initial version.  Jim Browne
*/

#ifdef MPW
#pragma segment Configure
#endif

#include "TelnetHeader.h"

#include <stdio.h>
#include <ctype.h>
#include <Picker.h>

#include "newresrc.h"

#include "DlogUtils.proto.h"
#include "popup.h"
#include "popup.proto.h"
#include "configure.proto.h"
#include "netevent.proto.h"		/* For Stask proto */
#include "prefs.proto.h"
#include "bkgr.proto.h"			// For StartUpFTP proto
#include "menuseg.proto.h"
#include "macutil.proto.h"		

static	pascal short TerminalModalProc( DialogPtr dptr, EventRecord *evt, short *item);
PROTO_UPP(TerminalModalProc, ModalFilter);
static	pascal short SessionModalProc(DialogPtr dptr, EventRecord *evt, short *item);
PROTO_UPP(SessionModalProc, ModalFilter);
static	pascal short MyDlogWListFilter( DialogPtr dptr, EventRecord *evt, short *item);
PROTO_UPP(MyDlogWListFilter, ModalFilter);

static	pascal listDitemproc(DialogPtr theDlg, short itemNo);
PROTO_UPP(listDitemproc, UserItem);

extern SysEnvRec theWorld;		/* BYU 2.4.18 - System Environment record */
extern	FTPServerPrefs*		gFTPServerPrefs;
extern	ApplicationPrefs*	gApplicationPrefs;

static	void ZeroNumOnly(void);
static	Boolean InNumOnly(short item);

#define NUMONLYSIZE 8
short NumOnly[NUMONLYSIZE];		/* Items for which non-number keys are ignored */
								// NumOnly[0] is number of "safe" item to return

void	CONFIGUREunload(void) {}
						
void Cenviron( void)
{
	DialogPtr	dptr;
	short 		ditem;
	Str255		scratchPstring;
	long 		scratchlong;
	OSType		newtype;
	
	dptr = GetNewMySmallStrangeDialog( PrefDLOG, NULL, kInFront, (void *)ThirdCenterDialog);	
	
	SetCntrl(dptr, PrefDClose, gApplicationPrefs->WindowsDontGoAway);
	SetCntrl(dptr, PrefStag, gApplicationPrefs->StaggerWindows);
	SetCntrl(dptr, PrefCMDkey, gApplicationPrefs->CommandKeys);
	SetCntrl(dptr, PrefTMap, gApplicationPrefs->RemapTilde);
	SetCntrl(dptr, PrefBlink, gApplicationPrefs->BlinkCursor);
	
	SetCntrl(dptr, PrefBlockCursor, (gApplicationPrefs->CursorType == 0));
	SetCntrl(dptr, PrefUnderscoreCursor, (gApplicationPrefs->CursorType == 1));
	SetCntrl(dptr, PrefVerticalCursor, (gApplicationPrefs->CursorType == 2));

	scratchPstring[0] = 4;
	BlockMove(&(gApplicationPrefs->CaptureFileCreator), &scratchPstring[1], sizeof(OSType));
	SetTEText(dptr, PrefCaptTE, scratchPstring);
	newtype = gApplicationPrefs->CaptureFileCreator;

	scratchlong = gApplicationPrefs->CopyTableThresh;
	NumToString(scratchlong, scratchPstring);
	SetTEText(dptr, PrefCTt, scratchPstring);
	SelIText(dptr, PrefCTt, 0, 32767);

	NumToString(gApplicationPrefs->TimeSlice, scratchPstring);
	SetTEText(dptr, PrefTimeSlice, scratchPstring);

	ShowWindow(dptr);
	ditem=0;								/* initially no hits */
	while((ditem>2) || (ditem==0)) {	
		ModalDialog(DLOGwOK_CancelUPP,&ditem);
		switch(ditem) {
			case PrefDClose:
			case PrefStag:
			case PrefCMDkey:
			case PrefTMap:
			case PrefBlink:
				FlipCheckBox( dptr, ditem);
				break;

			case PrefBlockCursor:
				SetCntrl(dptr, PrefBlockCursor, 1);
				SetCntrl(dptr, PrefUnderscoreCursor, 0);
				SetCntrl(dptr, PrefVerticalCursor, 0);
				break;
			case PrefUnderscoreCursor:
				SetCntrl(dptr, PrefBlockCursor, 0);
				SetCntrl(dptr, PrefUnderscoreCursor, 1);
				SetCntrl(dptr, PrefVerticalCursor, 0);
				break;
			case PrefVerticalCursor:
				SetCntrl(dptr, PrefBlockCursor, 0);
				SetCntrl(dptr, PrefUnderscoreCursor, 0);
				SetCntrl(dptr, PrefVerticalCursor, 1);
				break;

			case PrefCaptCreat:
				GetApplicationType(&newtype);
				scratchPstring[0] = 4;
				BlockMove(&newtype, &scratchPstring[1], sizeof(OSType));
				SetTEText(dptr, PrefCaptTE, scratchPstring);
			default:
				break;
			} /* switch */
		}
	
	if (ditem==DLOGCancel) {
			DisposDialog( dptr);
			return;
			}

	GetTEText(dptr, PrefCTt, scratchPstring);
	StringToNum(scratchPstring, &scratchlong);
	gApplicationPrefs->CopyTableThresh = (short) scratchlong;

	GetTEText(dptr, PrefTimeSlice, scratchPstring);
	StringToNum(scratchPstring, &scratchlong);
	BoundsCheck(&scratchlong, 100, 2);
	gApplicationPrefs->TimeSlice = scratchlong;

	GetTEText(dptr, PrefCaptTE, scratchPstring);
	BlockMove(&scratchPstring[1], &(gApplicationPrefs->CaptureFileCreator), sizeof(OSType));

	gApplicationPrefs->CommandKeys = GetCntlVal(dptr, PrefCMDkey);					
	gApplicationPrefs->WindowsDontGoAway = GetCntlVal(dptr, PrefDClose);
	gApplicationPrefs->RemapTilde =	GetCntlVal(dptr, PrefTMap);
	gApplicationPrefs->StaggerWindows = GetCntlVal(dptr, PrefStag);
	gApplicationPrefs->BlinkCursor = GetCntlVal(dptr, PrefBlink);
	gApplicationPrefs->CursorType = (GetCntlVal(dptr, PrefUnderscoreCursor) == 1) + (2 *
									 (GetCntlVal(dptr, PrefVerticalCursor) == 1));
									 
	switchMenus(gApplicationPrefs->CommandKeys);
	SavePreferences();
	
	DisposDialog( dptr);
}

void Cftp(void)
{
	DialogPtr			dptr;
	short				ditem;
	Str255				scratchPstring;
	OSType				scratchOSType;
	SFReply				sfr;
	FInfo				fi;			
	Point				where = {100,100};
	
	dptr = GetNewMySmallStrangeDialog( FTPDLOG, NULL, kInFront, (void *)ThirdCenterDialog);	

	SetCntrl(dptr, FTPServerOff, (gFTPServerPrefs->ServerState == 0));
	SetCntrl(dptr, FTPServerUnsecure, (gFTPServerPrefs->ServerState == 1));
	SetCntrl(dptr, FTPServerPswdPlease, (gFTPServerPrefs->ServerState == 2));
	SetCntrl(dptr, FTPShowFTPlog, gFTPServerPrefs->ShowFTPlog);
	SetCntrl(dptr, FTPrevDNS, gFTPServerPrefs->DNSlookupconnections);
	SetCntrl(dptr, FTPUseMacBinaryII, gFTPServerPrefs->UseMacBinaryII);
	SetCntrl(dptr, FTPResetMacBinary, gFTPServerPrefs->ResetMacBinary);
	SetCntrl(dptr, FTPISO, gFTPServerPrefs->DoISOtranslation);

	HideDItem(dptr, FTPrevDNS);		// Sometime later, perhaps.
	
	scratchPstring[0] = 4;

	BlockMove(&(gFTPServerPrefs->BinaryCreator), &scratchPstring[1], sizeof(OSType));
	SetTEText(dptr, FTPbincreatTE, scratchPstring);
	
	BlockMove(&(gFTPServerPrefs->BinaryType), &scratchPstring[1], sizeof(OSType));
	SetTEText(dptr, FTPbintypeTE, scratchPstring);
	SelIText(dptr, FTPbintypeTE, 0, 32767);

	BlockMove(&(gFTPServerPrefs->TextCreator), &scratchPstring[1], sizeof(OSType));
	SetTEText(dptr, FTPtextcreatTE, scratchPstring);

	ShowWindow(dptr);
	ditem=0;									/* initially no hits */
	while((ditem>2) || (ditem==0)) {	
		ModalDialog(DLOGwOK_CancelUPP,&ditem);
		switch(ditem) {
			case	FTPShowFTPlog:
			case	FTPrevDNS:
			case	FTPUseMacBinaryII:
			case	FTPResetMacBinary:
			case	FTPISO:
				FlipCheckBox( dptr, ditem);
				break;

			case	FTPServerOff:
				SetCntrl(dptr, FTPServerOff, 1);
				SetCntrl(dptr, FTPServerUnsecure, 0);
				SetCntrl(dptr, FTPServerPswdPlease, 0);
				break;
			case	FTPServerUnsecure:
				SetCntrl(dptr, FTPServerOff, 0);
				SetCntrl(dptr, FTPServerUnsecure, 1);
				SetCntrl(dptr, FTPServerPswdPlease, 0);
				break;
			case	FTPServerPswdPlease:
				SetCntrl(dptr, FTPServerOff, 0);
				SetCntrl(dptr, FTPServerUnsecure, 0);
				SetCntrl(dptr, FTPServerPswdPlease, 1);
				break;

			case	FTPtextcreatorbutton:
				if (GetApplicationType(&scratchOSType)) {
					BlockMove(&scratchOSType, &scratchPstring[1], sizeof(OSType));
					SetTEText(dptr, FTPtextcreatTE, scratchPstring);
					}
				break;
			
			case	FTPbinexamplebutton:
				SFGetFile(where, NULL, NULL, -1, NULL, NULL, &sfr);
				if (sfr.good) {
					GetFInfo(sfr.fName, sfr.vRefNum, &fi);
					scratchPstring[0] = sizeof(OSType);
					BlockMove(&fi.fdCreator, &scratchPstring[1], sizeof(OSType));
					SetTEText(dptr, FTPbincreatTE, scratchPstring);
					BlockMove(&fi.fdType, &scratchPstring[1], sizeof(OSType));
					SetTEText(dptr, FTPbintypeTE, scratchPstring);
					}
				break;	
			default:
				break;
			} /* switch */
		} /* while */
	
	if (ditem==DLOGCancel) {
			DisposDialog( dptr);
			return;
			}

	gFTPServerPrefs->ServerState = GetCntlVal(dptr, FTPServerUnsecure) + 
		GetCntlVal(dptr, FTPServerPswdPlease) * 2;
	StartUpFTP();			// Make sure FTP server recognizes new mode.
	
	gFTPServerPrefs->ShowFTPlog = GetCntlVal(dptr, FTPShowFTPlog);
	gFTPServerPrefs->DNSlookupconnections = GetCntlVal(dptr, FTPrevDNS);
	gFTPServerPrefs->UseMacBinaryII = GetCntlVal(dptr, FTPUseMacBinaryII);
	gFTPServerPrefs->ResetMacBinary = GetCntlVal(dptr, FTPResetMacBinary);
	gFTPServerPrefs->DoISOtranslation = GetCntlVal(dptr, FTPISO);

	GetTEText(dptr, FTPbincreatTE, scratchPstring);
	BlockMove(&scratchPstring[1], &(gFTPServerPrefs->BinaryCreator), sizeof(OSType));
	
	GetTEText(dptr, FTPbintypeTE, scratchPstring);
	BlockMove(&scratchPstring[1], &(gFTPServerPrefs->BinaryType), sizeof(OSType));

	GetTEText(dptr, FTPtextcreatTE, scratchPstring);
	BlockMove(&scratchPstring[1], &(gFTPServerPrefs->TextCreator), sizeof(OSType));

	SavePreferences();
	
	DisposDialog( dptr);
}

Boolean GetApplicationType(OSType *type)
{
	SFTypeList types;
	SFReply sfr;
	FInfo fi;
	Point where;
	
	SetPt(&where, 100, 100);
	types[0] = 'APPL';
	SFGetFile(where, NULL, NULL, 1, types, NULL, &sfr);
	if (sfr.good) {
		GetFInfo(sfr.fName, sfr.vRefNum, &fi);
		BlockMove(&fi.fdCreator, type, sizeof(OSType));		// Copy the application creator type
		}
		
	return(sfr.good);
}

//	Our standard modal dialog filter with code for handling user items containing lists.
SIMPLE_UPP(MyDlogWListFilter, ModalFilter);
pascal short MyDlogWListFilter( DialogPtr dptr, EventRecord *evt, short *item)
{
	short keyCode, key, iType;
	Handle iHndl;
	Rect iRect;
	Point scratchPoint;

	SetPort(dptr);
	if (evt->what == keyDown) {
		key = evt->message & charCodeMask;
		keyCode = (evt->message >>8) & 0xff ;
		if ((key == 0x0d) || (key == 0x03)) { // 0x0d == return, 0x03 == enter
			*item = 1;				// ok
			FlashButton(dptr, 1);
			return(-1);
			}
		}
	else if (evt->what == updateEvt) {
		if ((WindowPtr)evt->message == dptr) {
			GetDItem(dptr,1,&iType,&iHndl,&iRect);	// ok
			PenSize(3,3);
			InsetRect(&iRect,-4,-4);
			FrameRoundRect(&iRect,16,16);
			}
	}
	
	if (evt->what == mouseDown) {
		GetDItem(dptr, kItemList, &iType, &iHndl, &iRect);
		scratchPoint = evt->where;
		GlobalToLocal(&scratchPoint);
		if (PtInRect(scratchPoint, &iRect)) {
			*item = kItemList;
			if (LClick(scratchPoint, evt->modifiers, (ListHandle)GetWRefCon(dptr))) {
				*item = kChange;
				FlashButton(dptr, kChange);
				}	
			return(-1);
		}
	}
	
	return(FALSE);
}

//	User Dialog Item Procedure for a list.  Assumes the Dialog's refcon contains the list
//	handle.
SIMPLE_UPP(listDitemproc, UserItem);
pascal listDitemproc(DialogPtr theDlg, short itemNo)
{
	short		itemType;
	Handle		ControlHndl;
	Rect		scratchRect;
	
	GetDItem(theDlg, itemNo, &itemType, &ControlHndl, &scratchRect);
	
	PenNormal();
	InsetRect(&scratchRect, -1, -1);
	FrameRect(&scratchRect);
	LUpdate(theDlg->visRgn, (ListHandle)GetWRefCon(theDlg));
}

void	BoundsCheck(long *value, long high, long low)
{
	if (*value > high) *value = high;
	if (*value < low) *value = low;
}

void	EditConfigType(ResType ConfigResourceType, Boolean (*EditFunction)(StringPtr))
{
	DialogPtr	dptr;
	Handle 		iHndl;
	short 		iType, length, ditem, numberofitems, index, resID;
	Rect 		ListBox, ListBounds;
	Point 		cellSize, theCell;
	ListHandle	thelist;
	Handle		ItemResource;
	Str255		ItemName;
	ResType		restype;

	dptr = GetNewMySmallDialog(kItemDLOG, NULL, kInFront, (void *)ThirdCenterDialog);
	ditem = 3;

	GetDItem(dptr, kItemList, &iType, &iHndl, &ListBox);
	SetDItem(dptr, kItemList, iType, (Handle)listDitemprocUPP, &ListBox);
	

	ListBox.right -= 15;		// Make room for scrollbar
	SetRect(&ListBounds, 0,0,1,0);
	SetPt(&cellSize,(ListBox.right-ListBox.left),16);
	thelist = LNew(&ListBox, &ListBounds, cellSize, 0, (WindowPtr)dptr,0,0,0,1);
#define kSCListMods lNoNilHilite+lOnlyOne
	(*(thelist))->listFlags = kSCListMods;
	
	SetWRefCon(dptr, (long)thelist);		// So the Ditem proc can find the list
	
	UseResFile(TelInfo->SettingsFile);
	numberofitems = Count1Resources(ConfigResourceType);
	
	for(index = 1, theCell.v = 0, theCell.h = 0; index <= numberofitems;
				 index++, theCell.v++) {
		ItemResource = Get1IndResource(ConfigResourceType, index);
		GetResInfo(ItemResource, &resID, &restype, (StringPtr)&ItemName);
		LAddRow(1,-1, thelist);
		LSetCell(ItemName+1, Length(ItemName), theCell, thelist);
		ReleaseResource(ItemResource);
		}
		
	theCell.v = 0;
	theCell.h = 0;
	if (numberofitems) LSetSelect(1, theCell, thelist);
	LDoDraw(1, thelist);
//	LUpdate(dptr->visRgn, thelist); Is this needed?
	
	while (ditem > 1) {
		ModalDialog(MyDlogWListFilterUPP, &ditem);
		
		switch(ditem) {
			case kRemove:
				SetPt(&theCell, 0, 0);
				if (LGetSelect(TRUE, &theCell, thelist)) {
					length = 254;
					LGetCell(ItemName+1, &length, theCell, thelist);
					ItemName[0] = (char)length;
					if (!(EqualString(ItemName, "\p<Default>", TRUE, FALSE))) {
						LDelRow(1,theCell.v,thelist);
						UseResFile(TelInfo->SettingsFile);
						ItemResource = Get1NamedResource(ConfigResourceType, ItemName);
						RmveResource(ItemResource);
						ReleaseResource(ItemResource);
						UpdateResFile(TelInfo->SettingsFile);
						}
					}
			break;
			
			case kChange:
				SetPt(&theCell, 0, 0);
				if (LGetSelect(TRUE, &theCell, thelist)) {
					length = 254;
					LGetCell(ItemName+1, &length, theCell, thelist);
					ItemName[0] = (char)length;
					if ((*EditFunction)((StringPtr)&ItemName)) {
						LSetCell(ItemName+1, Length(ItemName), theCell, thelist);
					}
				}
			break;
			
			case kNew:		
				ItemName[0] = 0;					// Signal new shortcut
				if ((*EditFunction)((StringPtr)&ItemName)) {
					theCell.v = LAddRow(1, -1, thelist);
					LSetCell(ItemName+1, Length(ItemName), theCell, thelist);
				}
			break;
		} // switch
		
		SetPt(&theCell, 0, 0);
		if (LGetSelect(TRUE, &theCell, thelist)) {
			Hilite(dptr, kRemove, 0);
			Hilite(dptr, kChange, 0);
			}
		else {
			Hilite(dptr, kRemove, 255);
			Hilite(dptr, kChange, 255);
			}
		
	} // while
	
	LDispose(thelist);
	DisposDialog(dptr);
}

void ZeroNumOnly(void)
{
	short i;
	
	for(i=0; i<NUMONLYSIZE; i++) NumOnly[i]=0;
}

Boolean InNumOnly(short item)
{
	short index = 1;
	
	while ((NumOnly[index] != item) && (NumOnly[index] != 0)) index++;
	
	return(NumOnly[index] == item);
}

short		NumberOfColorBoxes;
short		BoxColorItems[8];
RGBColor	BoxColorData[8];

SIMPLE_UPP(ColorBoxItemProc, UserItem);
pascal ColorBoxItemProc(DialogPtr theDlg, short itemNo)
{
	short		itemType, index = 0;
	Handle		ControlHndl;
	Rect		scratchRect;
	RGBColor	rgb;

	GetDItem(theDlg, itemNo, &itemType, &ControlHndl, &scratchRect);
	PenNormal();
	FrameRect(&scratchRect);
	InsetRect(&scratchRect, 1, 1);			// Why isn't this framing the boxes?
	
	GetForeColor(&rgb);

	while ((index < NumberOfColorBoxes) && (BoxColorItems[index] != itemNo)) index++;
	
	RGBForeColor(&BoxColorData[index]);		/* color corresponding to item */
	PaintRect(&scratchRect);

	RGBForeColor(&rgb);			/* restore previous foreground color */
}

SIMPLE_UPP(ColorBoxModalProc, ModalFilter);
pascal short ColorBoxModalProc( DialogPtr dptr, EventRecord *evt, short *item)
{
	short keyCode, key, iType, index;
	Handle	iHndl;
	Rect	iRect;
	
	if ((evt->what == keyDown) || (evt->what == autoKey)) {
		key = evt->message & charCodeMask;
		keyCode = (evt->message >>8) & 0xff;
		if ((key == 0x0d) || (key == 0x03)) { // 0x0d == return, 0x03 == enter
			*item = DLOGOk;
			return(-1);				/* BYU LSC - pascal doesn't like "1" as true */
			}
		if (((key == '.') && (evt->modifiers & cmdKey)) ||
			((key == 0x1b) && (keyCode == 0x35)) ) {	// 0x35 == escKey
			*item = DLOGCancel;
			return(-1);				/* BYU LSC - pascal doesn't like "1" as true */
			}

        if ((key < '0' || key > '9') && 
        	!(key == 0x08 || key == 0x09 || (key > 0x1B && key < 0x20))
        	&& InNumOnly(((DialogPeek)dptr)->editField + 1)) {
            	/* Reject non-numbers for this TE */
            	*item = NumOnly[0];	/* Kludge -- Return "safe" item */
				return(-1);
				}   
       	}
       	
	if (evt->what == updateEvt) {
		if ((WindowPtr)evt->message == dptr) {
			GetDItem(dptr,DLOGOk,&iType,&iHndl,&iRect);	// ok
			PenSize(3,3);
			InsetRect(&iRect,-4,-4);
			FrameRoundRect(&iRect,16,16);
			for (index = 0; index < NumberOfColorBoxes; index++)
				ColorBoxItemProc(dptr, BoxColorItems[index]);		// update color boxes
			}
	}
	
	Stask();
	return(FALSE);
}

SIMPLE_UPP(TerminalModalProc, ModalFilter);
pascal short TerminalModalProc( DialogPtr dptr, EventRecord *evt, short *item)
{
	if (evt->what == mouseDown) return(PopupMousedown(dptr, evt, item));

	return(ColorBoxModalProc(dptr, evt, item));
}

SIMPLE_UPP(SessionModalProc, ModalFilter);
pascal short SessionModalProc(DialogPtr dptr, EventRecord *evt, short *item)
{
	if ((evt->what == keyDown) || (evt->what == autoKey))
		if ((evt->message & charCodeMask) == ' ') {
			*item = NumOnly[0];	/* Kludge -- Return "safe" item */
			return(-1);
			}
	
	return(TerminalModalProc(dptr, evt, item));
}

Boolean EditTerminal(StringPtr PrefRecordNamePtr)
{
	DialogPtr		dptr;
	short			ditem, scratchshort, resourceID;
	long			scratchlong;
	ResType			scratchResType;
	Boolean			IsNewPrefRecord, UserLikesNewColor;
	TerminalPrefs**	TermPrefsHdl;
	TerminalPrefs*	TermPrefsPtr;
	Str255			scratchPstring;
	RGBColor		scratchRGBcolor;
	Point			ColorBoxPoint;
	MenuHandle		WeNeedAFontMenuHandle;
	popup TPopup[] = {{TermFontPopup, (MenuHandle) 0, 1},
						{0, (MenuHandle) 0, 0}};

	dptr = GetNewMySmallStrangeDialog(TermDLOG, NULL, kInFront, (void *)ThirdCenterDialog);
	ditem = 3;
	
	WeNeedAFontMenuHandle = NewMenu(666, "\p");
	AddResMenu(WeNeedAFontMenuHandle, 'FONT');
	TPopup[0].h = WeNeedAFontMenuHandle;
	PopupInit(dptr, TPopup);
	
	if (PrefRecordNamePtr[0] != 0) {
		IsNewPrefRecord = FALSE;
		UseResFile(TelInfo->SettingsFile);
		TermPrefsHdl = (TerminalPrefs **)Get1NamedResource(TERMINALPREFS_RESTYPE, PrefRecordNamePtr);
		if (EqualString(PrefRecordNamePtr, "\p<Default>", FALSE, FALSE)) {
			HideDItem(dptr, TermNameStatText);
			HideDItem(dptr, TermName);
			}
		}
	else {
		TermPrefsHdl = GetDefaultTerminal();
		IsNewPrefRecord = TRUE;
		GetIndString(PrefRecordNamePtr, MISC_STRINGS, MISC_NEWTERM);
		}

	HLock((Handle) TermPrefsHdl);
	TermPrefsPtr = *TermPrefsHdl;
	SetTEText(dptr, TermName, PrefRecordNamePtr);
	SetCntrl(dptr, TermANSI, TermPrefsPtr->ANSIgraphics);
	HideDItem(dptr, TermANSI);				// Perhaps later
	SetCntrl(dptr, TermXterm, TermPrefsPtr->Xtermsequences);
	SetCntrl(dptr, Termvtwrap, TermPrefsPtr->vtwrap);
	SetCntrl(dptr, Termmeta, TermPrefsPtr->emacsmetakey);
	SetCntrl(dptr, Termarrow, TermPrefsPtr->emacsarrows);
	SetCntrl(dptr, TermMAT, TermPrefsPtr->MATmappings);
	SetCntrl(dptr, Termeightbit, TermPrefsPtr->eightbit);
	SetCntrl(dptr, Termclearsave, TermPrefsPtr->clearsave);
	SetCntrl(dptr, TermVT100, (TermPrefsPtr->vtemulation == 0));
	SetCntrl(dptr, TermVT220, (TermPrefsPtr->vtemulation == 1));
	scratchlong = (long)(TermPrefsPtr->vtwidth);
	NumToString(scratchlong, scratchPstring);
	SetTEText(dptr, TermWidth, scratchPstring);
	scratchlong = (short)(TermPrefsPtr->vtheight);
	NumToString(scratchlong, scratchPstring);
	SetTEText(dptr, TermHeight, scratchPstring);
	scratchlong = TermPrefsPtr->fontsize;
	NumToString(scratchlong, scratchPstring);
	SetTEText(dptr, TermFontSize, scratchPstring);
	scratchlong = TermPrefsPtr->numbkscroll;
	NumToString(scratchlong, scratchPstring);
	SetTEText(dptr, TermScrollback, scratchPstring);
	SetTEText(dptr, TermAnswerback, TermPrefsPtr->AnswerBackMessage);
	
	for(scratchshort = CountMItems(TPopup[0].h); scratchshort; scratchshort--) {
		GetItem(TPopup[0].h, scratchshort, scratchPstring);
		if (EqualString(scratchPstring, (TermPrefsPtr->DisplayFont), TRUE, FALSE))
			TPopup[0].choice = scratchshort;
		}
	
	ZeroNumOnly();
	NumOnly[0] = TermSafeItem;
	NumOnly[1] = TermWidth; NumOnly[2] = TermHeight; NumOnly[3] = TermFontSize;
	NumOnly[4] = TermScrollback;  NumOnly[5] = 0;
	
	if (theWorld.hasColorQD) {
		for (scratchshort = 0, NumberOfColorBoxes = 4; scratchshort < NumberOfColorBoxes; scratchshort++) {
			BoxColorItems[scratchshort] = TermNFcolor + scratchshort;
			BlockMove(&(TermPrefsPtr->nfcolor) + scratchshort,
				&BoxColorData[scratchshort], sizeof(RGBColor));
			UItemAssign( dptr, TermNFcolor + scratchshort, ColorBoxItemProcUPP);
			}
		}
	else
		NumberOfColorBoxes = 0;		// B&W machine
		
	ColorBoxPoint.h = 0;			// Have the color picker center the box on the main
	ColorBoxPoint.v = 0;			// screen
		
	SelIText(dptr, TermName, 0, 32767);
	ShowWindow(dptr);
	
	while (ditem > 2) {
		ModalDialog(TerminalModalProcUPP, &ditem);
		switch (ditem) {
			case	TermANSI:
			case	TermXterm:
			case	Termvtwrap:
			case	Termmeta:
			case	Termarrow:
			case	TermMAT:
			case	Termeightbit:
			case	Termclearsave:
				FlipCheckBox(dptr, ditem);
				break;
			
			case	TermVT100:
			case	TermVT220:
				FlipRadioButton(dptr, TermVT100);
				FlipRadioButton(dptr, TermVT220);
				if (ditem == TermVT100)
					SetTEText(dptr, TermAnswerback, "\pVT100");
				else
					SetTEText(dptr, TermAnswerback, "\pVT220");
				break;
			
			case	TermNFcolor:	
			case	TermNBcolor:	
			case	TermBFcolor:	
			case	TermBBcolor:	
				if (theWorld.hasColorQD) {
					UserLikesNewColor = GetColor(ColorBoxPoint, "\pPlease Select New Color",
						 &BoxColorData[ditem-TermNFcolor], &scratchRGBcolor);
					if (UserLikesNewColor)
						BoxColorData[ditem-TermNFcolor] = scratchRGBcolor;
					}
				break;
				
			default:
				break;
			
			} // switch
		} // while
			
	GetItem(TPopup[0].h, TPopup[0].choice, scratchPstring);
	PopupCleanup();
	
	if (ditem == 2) {
		if (IsNewPrefRecord) DisposeHandle((Handle) TermPrefsHdl);
		else ReleaseResource((Handle) TermPrefsHdl);
		
		DisposeDialog(dptr);
		return(FALSE);			// No changes should be made.
		}
	
		
	if (theWorld.hasColorQD) {
		for (scratchshort = 0; scratchshort < NumberOfColorBoxes; scratchshort++) {
				BlockMove(&BoxColorData[scratchshort], 
					&(TermPrefsPtr->nfcolor) + scratchshort, sizeof(RGBColor));
			}
		}
		
	if (Length(scratchPstring) > 63) scratchPstring[0] = 63;
	BlockMove(scratchPstring, (TermPrefsPtr->DisplayFont), scratchPstring[0]+1);
	
	TermPrefsPtr->ANSIgraphics = GetCntlVal(dptr, TermANSI);
	TermPrefsPtr->Xtermsequences = GetCntlVal(dptr, TermXterm);
	TermPrefsPtr->vtwrap = GetCntlVal(dptr, Termvtwrap);
	TermPrefsPtr->emacsmetakey = GetCntlVal(dptr, Termmeta);
	TermPrefsPtr->emacsarrows = GetCntlVal(dptr, Termarrow);
	TermPrefsPtr->MATmappings = GetCntlVal(dptr, TermMAT);
	TermPrefsPtr->eightbit = GetCntlVal(dptr, Termeightbit);
	TermPrefsPtr->clearsave = GetCntlVal(dptr, Termclearsave);
	
	TermPrefsPtr->vtemulation = (GetCntlVal(dptr, TermVT220) != 0);

	GetTEText(dptr, TermWidth, scratchPstring);
	StringToNum(scratchPstring, &scratchlong);
	BoundsCheck(&scratchlong, 133, 10);
	TermPrefsPtr->vtwidth = (short) scratchlong;
	
	GetTEText(dptr, TermHeight, scratchPstring);
	StringToNum(scratchPstring, &scratchlong);
	BoundsCheck(&scratchlong, 80, 10);
	TermPrefsPtr->vtheight = (short) scratchlong;

	GetTEText(dptr, TermFontSize, scratchPstring);
	StringToNum(scratchPstring, &scratchlong);
	BoundsCheck(&scratchlong, 24, 4);
	TermPrefsPtr->fontsize = (short) scratchlong;

	GetTEText(dptr, TermScrollback, scratchPstring);
	StringToNum(scratchPstring, &scratchlong);
	BoundsCheck(&scratchlong, 50000, 24);
	TermPrefsPtr->numbkscroll = (short) scratchlong;

	GetTEText(dptr, TermAnswerback, scratchPstring);
	if (Length(scratchPstring) > 63) scratchPstring[0] = 63;
	BlockMove(scratchPstring, TermPrefsPtr->AnswerBackMessage, scratchPstring[0]+1);
	
	GetTEText(dptr, TermName, PrefRecordNamePtr);
	
	if (IsNewPrefRecord) {
		UseResFile(TelInfo->SettingsFile);
		resourceID = UniqueID(TERMINALPREFS_RESTYPE);
		AddResource((Handle)TermPrefsHdl, TERMINALPREFS_RESTYPE, resourceID, PrefRecordNamePtr);
		UpdateResFile(TelInfo->SettingsFile);
		ReleaseResource((Handle)TermPrefsHdl);
		}
	else {
		UseResFile(TelInfo->SettingsFile);
		GetResInfo((Handle)TermPrefsHdl, &resourceID, &scratchResType,(StringPtr) &scratchPstring);
		SetResInfo((Handle)TermPrefsHdl, resourceID, PrefRecordNamePtr);
		ChangedResource((Handle)TermPrefsHdl);
		UpdateResFile(TelInfo->SettingsFile);
		ReleaseResource((Handle)TermPrefsHdl);
		}
	
	DisposeDialog(dptr);
	return(TRUE);			// A resource has changed or been added.	
}

Boolean EditSession(StringPtr PrefRecordNamePtr)
{
	DialogPtr		dptr;
	short			ditem, scratchshort, resourceID;
	long			scratchlong;
	ResType			scratchResType;
	Boolean			IsNewPrefRecord;
	SessionPrefs**	SessPrefsHdl;
	SessionPrefs*	SessPrefsPtr;
	Str255			scratchPstring, scratchPstring2;
	popup SPopup[] = {{SessTermPopup, (MenuHandle) 0, 1},
						{SessTransTablePopup, (MenuHandle) 0, 1},
						{0, (MenuHandle) 0, 0}};

	dptr = GetNewMySmallStrangeDialog(SessionConfigDLOG, NULL, kInFront, (void *)ThirdCenterDialog);
	ditem = 3;
		
	SPopup[0].h = NewMenu(666, "\p");
	UseResFile(TelInfo->SettingsFile);
	AddResMenu(SPopup[0].h, TERMINALPREFS_RESTYPE);
	EnableItem(SPopup[0].h, 0);		// Make sure the entire menu is enabled
	
	SPopup[1].h = NewMenu(667, "\p");
	AppendMenu(SPopup[1].h, "\pNone");
	AddResMenu(SPopup[1].h, USER_TRSL);
	EnableItem(SPopup[1].h, 0);		// Make sure the entire menu is enabled

	PopupInit(dptr, SPopup);
	
	if (PrefRecordNamePtr[0] != 0) {
		IsNewPrefRecord = FALSE;
		UseResFile(TelInfo->SettingsFile);
		SessPrefsHdl = (SessionPrefs **)Get1NamedResource(SESSIONPREFS_RESTYPE, PrefRecordNamePtr);
		if (EqualString(PrefRecordNamePtr, "\p<Default>", FALSE, FALSE)) {
			HideDItem(dptr, SessAlias);
			HideDItem(dptr, SessAliasStatText);
			}
		}
	else {
		SessPrefsHdl = GetDefaultSession();
		IsNewPrefRecord = TRUE;
		GetIndString(PrefRecordNamePtr, MISC_STRINGS, MISC_NEWSESSION);
		}
		
	HLock((Handle) SessPrefsHdl);
	SessPrefsPtr = *SessPrefsHdl;
	SetCntrl(dptr, SessTEKinhib, (SessPrefsPtr->tektype == -1));
	SetCntrl(dptr, SessTEK4014, (SessPrefsPtr->tektype == 0));
	SetCntrl(dptr, SessTEK4105, (SessPrefsPtr->tektype == 1));
	SetCntrl(dptr, SessPasteQuick, (SessPrefsPtr->pastemethod == 0));
	SetCntrl(dptr, SessPasteBlock, (SessPrefsPtr->pastemethod == 1));
	SetCntrl(dptr, SessDeleteDel, (SessPrefsPtr->bksp == 1));
	SetCntrl(dptr, SessDeleteBS, (SessPrefsPtr->bksp == 0));
	SetCntrl(dptr, SessForceSave, SessPrefsPtr->forcesave);
	SetCntrl(dptr, SessBezerkeley, SessPrefsPtr->crmap);
	SetCntrl(dptr, SessLinemode, SessPrefsPtr->linemode);
	SetCntrl(dptr, SessTEKclear, SessPrefsPtr->tekclear);
	SetCntrl(dptr, SessHalfDuplex, SessPrefsPtr->halfdup);
	SetCntrl(dptr, SessLowLevelErrs, SessPrefsPtr->showlowlevelerrors);
	SetCntrl(dptr, SessAuthenticate, SessPrefsPtr->authenticate);
	SetCntrl(dptr, SessEncrypt, SessPrefsPtr->encrypt);
	SetCntrl(dptr, SessLocalEcho, SessPrefsPtr->localecho);
	SetTEText(dptr, SessHostName, SessPrefsPtr->hostname);
	SetTEText(dptr, SessAlias, PrefRecordNamePtr);
	NumToString(SessPrefsPtr->port, scratchPstring);
	SetTEText(dptr, SessPort, scratchPstring);
	NumToString(SessPrefsPtr->pasteblocksize, scratchPstring);
	SetTEText(dptr, SessBlockSize, scratchPstring);
	scratchPstring[0] = 2;
	scratchPstring[1] = '^';
	if (SessPrefsPtr->ckey != -1) {
		scratchPstring[2] = SessPrefsPtr->ckey ^ 64;
		SetTEText(dptr, SessInterrupt, scratchPstring);
		}
	if (SessPrefsPtr->skey != -1) {
		scratchPstring[2] = SessPrefsPtr->skey ^ 64;
		SetTEText(dptr, SessSuspend, scratchPstring);
		}
	if (SessPrefsPtr->qkey != -1) {
		scratchPstring[2] = SessPrefsPtr->qkey ^ 64;
		SetTEText(dptr, SessResume, scratchPstring);
		}
	for(scratchshort = CountMItems(SPopup[0].h); scratchshort; scratchshort--) {
		GetItem(SPopup[0].h, scratchshort, scratchPstring);
		if (EqualString(scratchPstring, (SessPrefsPtr->TerminalEmulation), TRUE, FALSE))
			SPopup[0].choice = scratchshort;
		}
	for(scratchshort = CountMItems(SPopup[1].h); scratchshort; scratchshort--) {
		GetItem(SPopup[1].h, scratchshort, scratchPstring);
		if (EqualString(scratchPstring, (SessPrefsPtr->TranslationTable), TRUE, FALSE))
			SPopup[1].choice = scratchshort;
		}
	
	NumberOfColorBoxes = 0;
	ZeroNumOnly();
	NumOnly[0] = SessSafeItem;
	NumOnly[1] = SessPort;  NumOnly[2] = SessBlockSize;
	
	SelIText(dptr, SessAlias, 0, 32767);
	ShowWindow(dptr);
	
	while (ditem > 2) {
		ModalDialog(SessionModalProcUPP, &ditem);
		switch (ditem) {
			case	SessForceSave:
			case	SessBezerkeley:
			case	SessLinemode:
			case	SessTEKclear:
			case	SessHalfDuplex:
			case	SessLowLevelErrs:
			case	SessAuthenticate:
			case	SessEncrypt:
			case	SessLocalEcho:
				FlipCheckBox(dptr, ditem);
				break;
			
			case	SessTEKinhib:
				SetCntrl(dptr, SessTEKinhib, 1);
				SetCntrl(dptr, SessTEK4014, 0);
				SetCntrl(dptr, SessTEK4105, 0);
				break;
			case	SessTEK4014:
				SetCntrl(dptr, SessTEKinhib, 0);
				SetCntrl(dptr, SessTEK4014, 1);
				SetCntrl(dptr, SessTEK4105, 0);
				break;
			case	SessTEK4105:
				SetCntrl(dptr, SessTEKinhib, 0);
				SetCntrl(dptr, SessTEK4014, 0);
				SetCntrl(dptr, SessTEK4105, 1);
				break;
				
			case	SessPasteQuick:
			case	SessPasteBlock:
				FlipRadioButton(dptr, SessPasteQuick);
				FlipRadioButton(dptr, SessPasteBlock);
				break;

			case	SessDeleteDel:
			case	SessDeleteBS:
				FlipRadioButton(dptr, SessDeleteDel);
				FlipRadioButton(dptr, SessDeleteBS);
				break;

			case	SessInterrupt:
			case	SessSuspend:
			case	SessResume:
				GetTEText(dptr, ditem, scratchPstring);
				if ((scratchPstring[1] < 32) && (scratchPstring[1] > 0)) {
					scratchPstring[0] = 2;
					scratchPstring[2] = scratchPstring[1] ^ 64;
					scratchPstring[1] = '^';
					SetTEText(dptr, ditem, scratchPstring);
					}
				break;
				
			default:
				break;
			
			} // switch
		} // while
		
	GetItem(SPopup[0].h, SPopup[0].choice, scratchPstring);
	GetItem(SPopup[1].h, SPopup[1].choice, scratchPstring2);
	PopupCleanup();

	if (ditem == 2) {
		if (IsNewPrefRecord) DisposeHandle((Handle) SessPrefsHdl);
		else ReleaseResource((Handle) SessPrefsHdl);
		
		DisposeDialog(dptr);
		return(FALSE);			// No changes should be made.
		}
		
	if (Length(scratchPstring) > 63) scratchPstring[0] = 63;
	BlockMove(scratchPstring, (SessPrefsPtr->TerminalEmulation), scratchPstring[0]+1);

	if (Length(scratchPstring2) > 63) scratchPstring[0] = 63;
	BlockMove(scratchPstring2, (SessPrefsPtr->TranslationTable), scratchPstring2[0]+1);

	SessPrefsPtr->tektype = (-1 * GetCntlVal(dptr, SessTEKinhib)) + GetCntlVal(dptr, SessTEK4105);
	SessPrefsPtr->pastemethod = !GetCntlVal(dptr, SessPasteQuick);
	SessPrefsPtr->bksp = GetCntlVal(dptr, SessDeleteDel);
	SessPrefsPtr->forcesave = GetCntlVal(dptr, SessForceSave);
	SessPrefsPtr->crmap = GetCntlVal(dptr, SessBezerkeley);
	SessPrefsPtr->linemode = GetCntlVal(dptr, SessLinemode);
	SessPrefsPtr->tekclear = GetCntlVal(dptr, SessTEKclear);
	SessPrefsPtr->halfdup = GetCntlVal(dptr, SessHalfDuplex);
	SessPrefsPtr->showlowlevelerrors = GetCntlVal(dptr, SessLowLevelErrs);
	SessPrefsPtr->authenticate = GetCntlVal(dptr, SessAuthenticate);
	SessPrefsPtr->encrypt = GetCntlVal(dptr, SessEncrypt);
	SessPrefsPtr->localecho = GetCntlVal(dptr, SessLocalEcho);

	GetTEText(dptr, SessPort, scratchPstring);
	StringToNum(scratchPstring, &scratchlong);
	BoundsCheck(&scratchlong, 65530, 1);
	SessPrefsPtr->port = (short) scratchlong;

	GetTEText(dptr, SessBlockSize, scratchPstring);
	StringToNum(scratchPstring, &scratchlong);
	BoundsCheck(&scratchlong, 4097, 100);
	SessPrefsPtr->pasteblocksize = (short) scratchlong;

	GetTEText(dptr, SessHostName, scratchPstring);
	if (Length(scratchPstring) > 63) scratchPstring[0] = 63;
	BlockMove(scratchPstring, SessPrefsPtr->hostname, scratchPstring[0]+1);
	
	GetTEText(dptr, SessHostName, scratchPstring);
	if (Length(scratchPstring) > 63) scratchPstring[0] = 63;
	BlockMove(scratchPstring, SessPrefsPtr->hostname, scratchPstring[0]+1);

	GetTEText(dptr, SessAlias, PrefRecordNamePtr);

	GetTEText(dptr, SessInterrupt, scratchPstring);
	if (scratchPstring[0]) SessPrefsPtr->ckey = toupper(scratchPstring[2]) ^ 64;
	else SessPrefsPtr->ckey = -1;
	
	GetTEText(dptr, SessSuspend, scratchPstring);
	if (scratchPstring[0]) SessPrefsPtr->skey = toupper(scratchPstring[2]) ^ 64;
	else SessPrefsPtr->skey = -1;
	
	GetTEText(dptr, SessResume, scratchPstring);
	if (scratchPstring[0]) SessPrefsPtr->qkey = toupper(scratchPstring[2]) ^ 64;
	else SessPrefsPtr->qkey = -1;
	
	if (IsNewPrefRecord) {
		UseResFile(TelInfo->SettingsFile);
		resourceID = UniqueID(SESSIONPREFS_RESTYPE);
		AddResource((Handle)SessPrefsHdl, SESSIONPREFS_RESTYPE, resourceID, PrefRecordNamePtr);
		UpdateResFile(TelInfo->SettingsFile);
		ReleaseResource((Handle)SessPrefsHdl);
		}
	else {
		UseResFile(TelInfo->SettingsFile);
		GetResInfo((Handle)SessPrefsHdl, &resourceID, &scratchResType,(StringPtr) &scratchPstring);
		SetResInfo((Handle)SessPrefsHdl, resourceID, PrefRecordNamePtr);
		ChangedResource((Handle)SessPrefsHdl);
		UpdateResFile(TelInfo->SettingsFile);
		ReleaseResource((Handle)SessPrefsHdl);
		}
	
	DisposeDialog(dptr);
	return(TRUE);			// A resource has changed or been added.	
}

Boolean EditFTPUser(StringPtr PrefRecordNamePtr)
{
	DialogPtr	dptr;
	short		ditem, scratchshort, resourceID, vRefNum;
	ResType		scratchResType;
	Boolean		IsNewPrefRecord;
	FTPUser**	FTPUHdl;
	FTPUser*	FTPUptr;
	Str255		scratchPstring, scratchPstring2;

	dptr = GetNewMySmallStrangeDialog(FTPUserDLOG, NULL, kInFront, (void *)ThirdCenterDialog);
	ditem = 3;

	HideDItem(dptr, FTPUcanchangeCWD);		// Sometime later, perhaps
		
	if (PrefRecordNamePtr[0] != 0) {
		IsNewPrefRecord = FALSE;
		UseResFile(TelInfo->SettingsFile);
		FTPUHdl = (FTPUser **)Get1NamedResource(FTPUSER, PrefRecordNamePtr);
		HLock((Handle) FTPUHdl);
		FTPUptr = *FTPUHdl;
		SetCntrl(dptr, FTPUcanchangeCWD, FTPUptr->UserCanCWD);
		SetTEText(dptr, FTPUusername, PrefRecordNamePtr);
		for (scratchshort = 8, scratchPstring[0] = 8; scratchshort > 0; scratchshort--)
			scratchPstring[scratchshort] = '�';
 		SetTEText(dptr, FTPUpassword, scratchPstring);
		vRefNum = VolumeNameToRefNum(FTPUptr->DefaultDirVolName);
		}
	else {
		IsNewPrefRecord = TRUE;
		FTPUHdl = (FTPUser **)NewHandleClear(sizeof(FTPUser));
		HLock((Handle) FTPUHdl);
		FTPUptr = *FTPUHdl;
		vRefNum = -1;						// Default Volume
		FTPUptr->DefaultDirDirID = 2;		// Root directory
		SetCntrl(dptr, FTPUcanchangeCWD, 0);
		}

	PathNameFromDirID(FTPUptr->DefaultDirDirID, vRefNum, scratchPstring);
	SetTEText(dptr, FTPUDfltDirDsply, scratchPstring);
	
	SelIText(dptr, FTPUusername, 0, 32767);
	ShowWindow(dptr);
	
	while (ditem > 2) {
		ModalDialog(DLOGwOK_CancelUPP, &ditem);
		switch (ditem) {
			case	FTPUcanchangeCWD:
				FlipCheckBox(dptr, ditem);
				break;
			
			case	FTPUDfltDirButton:
				SelectDirectory(&vRefNum, &(FTPUptr->DefaultDirDirID));
				PathNameFromDirID(FTPUptr->DefaultDirDirID, vRefNum, scratchPstring);
				SetTEText(dptr, FTPUDfltDirDsply, scratchPstring);
				break;
				
			default:
				break;
			
			} // switch
		} // while
		
	if (ditem == 2) {
		if (IsNewPrefRecord) DisposeHandle((Handle) FTPUHdl);
		else ReleaseResource((Handle) FTPUHdl);
		
		DisposeDialog(dptr);
		return(FALSE);			// No changes should be made.
		}
		
	FTPUptr->UserCanCWD = GetCntlVal(dptr, FTPUcanchangeCWD);
	GetDirectoryName(vRefNum, 2, FTPUptr->DefaultDirVolName);
	
	GetTEText(dptr, FTPUusername, PrefRecordNamePtr);
	GetTEText(dptr, FTPUpassword, scratchPstring);
	
	if (scratchPstring[0] != '�') {
		// Encrypt the new (or possibly modified) password.
		PtoCstr(scratchPstring);
		Sencompass((char *)scratchPstring, (char *)scratchPstring2);
		CtoPstr((char *)scratchPstring2);
		BlockMove(scratchPstring2, FTPUptr->EncryptedPassword, Length(scratchPstring2)+1);
		}
			
	if (IsNewPrefRecord) {
		UseResFile(TelInfo->SettingsFile);
		resourceID = UniqueID(FTPUSER);
		AddResource((Handle)FTPUHdl, FTPUSER, resourceID, PrefRecordNamePtr);
		UpdateResFile(TelInfo->SettingsFile);
		ReleaseResource((Handle)FTPUHdl);
		}
	else {
		UseResFile(TelInfo->SettingsFile);
		GetResInfo((Handle)FTPUHdl, &resourceID, &scratchResType,(StringPtr) &scratchPstring);
		SetResInfo((Handle)FTPUHdl, resourceID, PrefRecordNamePtr);
		ChangedResource((Handle)FTPUHdl);
		UpdateResFile(TelInfo->SettingsFile);
		ReleaseResource((Handle)FTPUHdl);
		}
	
	DisposeDialog(dptr);
	return(TRUE);			// A resource has changed or been added.	
}
