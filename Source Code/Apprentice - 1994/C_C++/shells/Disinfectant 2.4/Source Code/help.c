/*______________________________________________________________________

	help.c - Help Window Manager.
	
	Copyright � 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
_____________________________________________________________________*/


#pragma load "precompile"
#include "hlp.h"
#include "utl.h"
#include "rez.h"
#include "glob.h"
#include "wstm.h"
#include "misc.h"
#include "help.h"
#include "main.h"
#include "misc.h"
#include "unmount.h"
#include "vol.h"

#pragma segment help

/*______________________________________________________________________

	Global Variables.
_____________________________________________________________________*/


static WindowPtr			HelpWindow = nil;		/* ptr to help window */
static WindowRecord		HelpWRecord;			/* help window record */
static WindowObject		HelpWindObject;		/* help window object */

/*______________________________________________________________________

	Click - Process Mouse Down Event.
	
	Entry:		where = mouse down location, local coords.
					modifiers = modifier keys.
_____________________________________________________________________*/

static void Click (Point where, short modifiers)

{
#pragma unused (modifiers)

	hlp_Click(where);
}

/*______________________________________________________________________

	Help - Process Mouse Down Event in Help Mode.
	
	Entry:		where = mouse down location, local coords.
_____________________________________________________________________*/


static void Help (Point where)

{
	Rect		tcRect;			/* tcon rectangle */

	hlp_GetTconRect(&tcRect);
	tcRect.top -= 15;
	help_Open(PtInRect(where, &tcRect) ? tagHelpTcon : tagHelpWind);
}
	
/*______________________________________________________________________

	Close - Close Window.
_____________________________________________________________________*/


static void Close (void)

{
	Prefs.helpState.moved = HelpWindObject.moved;
	wstm_Save(HelpWindow, &Prefs.helpState);
	Prefs.helpScrollPos = hlp_GetScrollPos();
	hlp_Close();
	CloseWindow(HelpWindow);
	HelpWindow = nil;
}
	
/*______________________________________________________________________

	Save - Process Save Command.
	
	Exit:		function result = true (nobody cares if the user canceled!)
_____________________________________________________________________*/


static Boolean Save (void)

{
	Str255			prompt;			/* SFPutFile prompt string */
	Str255			defName;			/* default name for help save dlog */
	OSErr				rCode;			/* result code */
	Str255			rCodeStr;		/* result code as a string */
	long				savedTrapAddr;	/* saved UnmountVol trap adress */
	
	/* Get the prompt string and default file name. */
	
	GetIndString(prompt, strListID, abPromptStr);
	GetIndString(defName, strListID, abDefNameStr);
	
	/* If scanning, patch the UnmountVol trap to prevent volume unmounting. */
	
	if (Scanning) {
		savedTrapAddr = NGetTrapAddress(_UnmountVol & 0x3ff, ToolTrap);
		NSetTrapAddress((long)UNMOUNT, _UnmountVol & 0x3ff, ToolTrap);
	};
	
	/* Call hlp_Save to save the file. */
	
	rCode = hlp_Save(prompt, defName, Prefs.docCreator, MenuPick);
	
	/* Handle errors. */
	
	if (rCode) {
		if (rCode == dskFulErr) {
			utl_StopAlert(diskFullID, nil, 0);
		} else if (rCode == fLckdErr) {
			utl_StopAlert(fileLockedID, nil, 0);
		} else {
			NumToString(rCode, rCodeStr);
			ParamText(rCodeStr, nil, nil, nil);
			utl_StopAlert(unexpectedSaveID, nil, 0);
		};
	};
	
	/* If scanning, restore the UnmountVol trap. */
	
	if (Scanning) {
		NSetTrapAddress(savedTrapAddr, _UnmountVol & 0x3ff, ToolTrap);
	} else {
		main_SetPort();
		vol_Verify();
		misc_HiliteScan();
	};
	
	return true;
}
	
/*______________________________________________________________________

	PageSetup - Process Page Setup Command.
	
	Exit:		function result = error code.
_____________________________________________________________________*/


static OSErr PageSetup (void)

{
	Boolean			canceled;
	
	misc_ValPrint(&Prefs.helpPrint, false);
	Prefs.helpPrint.menuPick = MenuPick;
	return rpp_StlDlog(&Prefs.helpPrint, &canceled);
}
	
/*______________________________________________________________________

	Print - Process Print Command.
	
	Entry:	printOne = true if Print One command.
	
	Exit:		function result = error code.
_____________________________________________________________________*/


static OSErr Print (Boolean printOne)

{
	Str255			titleTmpl;	/* template for date, time, pnum in headers */
	Str255			docName;		/* document name */
	
	GetIndString(titleTmpl, strListID, hlpDocTmpl);
	GetIndString(docName, strListID, hlpDocTitle);
	Prefs.helpPrint.title = docName;
	Prefs.helpPrint.titleTmpl = titleTmpl;
	Prefs.helpPrint.docName = docName;
	misc_ValPrint(&Prefs.helpPrint, false);
	Prefs.helpPrint.menuPick = MenuPick;
	return hlp_Print(&Prefs.helpPrint, printOne);
}
	
/*______________________________________________________________________

	Adjust - Adjust Menus.
_____________________________________________________________________*/


static void Adjust (void)

{
	MenuHandle				fileM;					/* handle to file menu */
	MenuHandle				editM;					/* handle to edit menu */
	MenuHandle				scanM;					/* handle to scan menu */
	
	fileM = GetMHandle(fileMID);
	editM = GetMHandle(editMID);
	scanM = GetMHandle(scanMID);
	EnableItem(fileM, closeCommand);
	EnableItem(fileM, saveAsCommand);
	EnableItem(fileM, pageSetupCommand);
	EnableItem(fileM, printCommand);
	EnableItem(fileM, printOneCommand);
	if (Scanning) {
		DisableItem(scanM, 0);
	} else {
		EnableItem(scanM, 0);
	};
	DisableItem(editM, 0);
}

/*______________________________________________________________________

	DrawTconTitle - Draw Table of Contents Title.
_____________________________________________________________________*/


static void DrawTconTitle ()

{
	Str255			str;				/* title */
	short				h;					/* horizontal coord of title */

	GetIndString(str, strListID, tconTitle);
	TextFace(bold);
	h = (RectList[tconRect].left + RectList[tconRect].right
		- StringWidth(str)) >> 1;
	MoveTo(h, RectList[tconRect].top-5);
	DrawString(str);
	TextFace(normal);
}
	
/*______________________________________________________________________

	help_Open - Open Help Window.
	
	Entry:		tag = Tagged line to jump to, or 0 if none.
_____________________________________________________________________*/


void help_Open (short tag)

{
	hlp_PBlock		p;				/* hlp_Open param block. */
	short				fNum;			/* font number */
	
	HelpMode = false;
	InitCursor();
	
	/* If the window is already open, activate it and jump to the tag. */
	
	if (HelpWindow) {
		SelectWindow(HelpWindow);
		if (tag) hlp_Jump(tag);
		return;
	};
	
	/* Get the help window and restore its state. */
	
	HelpWindow = wstm_Restore(false, helpWindID, (Ptr)&HelpWRecord,
		&Prefs.helpState);
	SetPort(HelpWindow);
	
	/* Initialize the window object. */
	
	((WindowPeek)HelpWindow)->refCon = (long)&HelpWindObject;
	HelpWindObject.windKind = helpWind;
	HelpWindObject.moved = Prefs.helpState.moved;
	SetRect(&HelpWindObject.sizeRect, HelpWindow->portRect.right, 
		minHelpSize, HelpWindow->portRect.right, 0x7fff);
	HelpWindObject.update = hlp_Update;
	HelpWindObject.activate = hlp_Activate;
	HelpWindObject.deactivate = hlp_Deactivate;
	HelpWindObject.resume = nil;
	HelpWindObject.suspend = nil;
	HelpWindObject.click = Click;
	HelpWindObject.help = Help;
	HelpWindObject.grow = hlp_Grow;
	HelpWindObject.zoom = hlp_Zoom;
	HelpWindObject.key = hlp_Key;
	HelpWindObject.close = Close;
	HelpWindObject.disk = nil;
	HelpWindObject.save = Save;
	HelpWindObject.pageSetup = PageSetup;
	HelpWindObject.print = Print;
	HelpWindObject.edit = nil;
	HelpWindObject.adjust = Adjust;
	HelpWindObject.periodic = nil;
	HelpWindObject.dialogPre = nil;
	HelpWindObject.dialogPost = nil;
	
	/* Initialize hlp_Open param block. */
	
	if (!utl_GetFontNumber("\pGeneva", &fNum)) fNum = applFont;
	p.scrollLine = Prefs.helpScrollPos;
	p.firstStrID = firstDocID;
	p.listDefID = lDefID;
	p.textRect = RectList[docRect];
	p.fontNum = fNum;
	p.fontSize = 9;
	p.tabConID = tconID;
	p.tabConRect = RectList[tconRect];
	p.tabConFNum = fNum;
	p.tabConFSize = 9;
	p.tabConLSep = 12;
	p.tag = tag;
	p.tagRezID = tagID;
	p.cellRezID = cellID;
	p.cellOption = 2;
	p.extraUpdate = DrawTconTitle;
	
	/* Call hlp_Open to complete the open. */
	
	hlp_Open(HelpWindow, &p);
}
