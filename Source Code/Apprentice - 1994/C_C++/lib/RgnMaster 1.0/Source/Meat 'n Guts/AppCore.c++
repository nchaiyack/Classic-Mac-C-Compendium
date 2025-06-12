/*

		AppCore.c++
		by Hiep Dam, 3G Software.
		March 1994.
		Last Update April 1994
		Contact: America Online: Starlabs
		         Delphi        : StarLabs
		         Internet      : starlabs@aol.com
		                      or starlabs@delphi.com

		Handles the internal workings of the application; herein lies the material
		that is NOT pertinent to Macintosh interface or region upkeep in particular.
*/


#include <QDOffscreen.h>
#include "Compat.h"
#include "QDUtils.h"
#include "DialogUtils.h"
#include "Regions.h"
#include "RgnWork.h"
#include "Help Window.h"
#include "About.h"
#include "StringUtils.h"
#include "RgnMaster.h"
#include "AppCore.h"

// ---------------------------------------------------------------------------

// Globals from RgnMaster.c++ (the main() file)...
extern Boolean gDone;
extern DialogPtr gMainDialog;
extern MenuHandle gFileMenu, gEditMenu, gAppleMenu;
extern short gCurrentItem;
extern Boolean gShowHelp;

// ---------------------------------------------------------------------------

void UpdateHelpStatus() {
	Point theLoc;
	Str255 helpStr;
	short i, saveItem;
	Rect itemRect, helpRect;

	if (FrontWindow() != gMainDialog)
		return;

	SetPort(gMainDialog);

	FrameGrayRect(gMainDialog, helpItem);
	GetDItemRect(gMainDialog, helpItem, &helpRect);

	if (!gShowHelp)
		return;

	InsetRect(&helpRect, 5, 5);

	saveItem = gCurrentItem;
	gCurrentItem = 0;

	for (i = 1; i <= helpCheckbx; i++) {
		GetDItemRect(gMainDialog, i, &itemRect);
		GetMouse(&theLoc);
		if (PtInRect(theLoc, &itemRect)) {
			gCurrentItem = i;
		}
	}

	if (gCurrentItem != saveItem) {
		GetIndString(helpStr, kHelpStrsID, gCurrentItem + 1);
		DrawMsg(helpStr);
	}
} // ENd UpdateHelpStatus

// ---------------------------------------------------------------------------

void DrawMsg(Str255 theMsg) {
	Rect helpRect;

	GetDItemRect(gMainDialog, helpItem, &helpRect);
	InsetRect(&helpRect, 5, 5);
	TextBox(&theMsg[1], theMsg[0], &helpRect, teFlushLeft);
} // END DrawMsg

// ---------------------------------------------------------------------------

void DoError(Str255 theMsg) {
	EventRecord theEvent;
	Str31 errMsg = "\pClick to continue...";
	Rect msgRect, errMsgRect;

	// Since there can be another window open at the same time the main dialog
	// is open, we have to make sure that the main dialog is the topmost window
	// before drawing any error messages. If it isn't, we just sysbeep.
	// An alternative is to select the main dialog and then show the error
	// message. A matter of preference....
	if (FrontWindow() != gMainDialog) {
		SysBeep(1);
		return;
	}

	GetDItemRect(gMainDialog, helpItem, &msgRect);
	InsetRect(&msgRect, 5, 5);
	TextBox(&theMsg[1], theMsg[0], &msgRect, teFlushLeft);

	TextFace(bold);
	errMsgRect = msgRect;
	errMsgRect.top = errMsgRect.bottom - 14;
	TextBox(&errMsg[1], errMsg[0], &errMsgRect, teFlushLeft);
	TextFace(0);	// Plain.

	do {
		if (GetNextEvent(everyEvent, &theEvent))
			break;
	} while (!Button());
	FlushEvents(everyEvent, 0);

	InsetRect(&msgRect, -4, -4);
	EraseRect(&msgRect);
} // END DoError

// ---------------------------------------------------------------------------

pascal void DrawHelpStatus(DialogPtr theDialog, short theItem) {
	UpdateHelpStatus();
} // END DrawHelpStatus

// ---------------------------------------------------------------------------

void ShowRegionInfo() {
	if (GetCurrentRegion() == nil)
		DoError("\pThere is no region to get information on. Copy a picture to the clipboard first, convert it, and then view information on it.");
	else {
		RgnHandle theRgn = GetCurrentRegion();
		Rect msgRect;
		Str255 regionInfo = "\p";
		Str15 leftStr, rightStr, topStr, botStr, sizeStr;
		Str15 cret;	// A carrage return, decimal 13 in ASCII
		cret[0] = 1; cret[1] = 13;

		GetDItemRect(gMainDialog, helpItem, &msgRect);
		InsetRect(&msgRect, 5, 5);

		NumToString((**theRgn).rgnBBox.left, leftStr);
		NumToString((**theRgn).rgnBBox.top, topStr);
		NumToString((**theRgn).rgnBBox.right, rightStr);
		NumToString((**theRgn).rgnBBox.bottom, botStr);
		NumToString(GetHandleSize((Handle)theRgn), sizeStr);

		PStrCat("\pSize of region, in bytes: ", regionInfo);
		PStrCat(sizeStr, regionInfo);  PStrCat(cret, regionInfo); PStrCat(cret, regionInfo);
		PStrCat("\pLeft bounds:  ", regionInfo);
		PStrCat(leftStr, regionInfo);  PStrCat(cret, regionInfo);
		PStrCat("\pTop bounds:   ", regionInfo);
		PStrCat(topStr, regionInfo);   PStrCat(cret, regionInfo);
		PStrCat("\pRight bounds: ", regionInfo);
		PStrCat(rightStr, regionInfo); PStrCat(cret, regionInfo);
		PStrCat("\pBot bounds:   ", regionInfo);
		PStrCat(botStr, regionInfo);   PStrCat(cret, regionInfo);

		TextFont(monaco);
		TextBox(&regionInfo[1], regionInfo[0], &msgRect, teFlushLeft);
		TextFont(geneva);
	}
} // END ShowRegionInfo

// ---------------------------------------------------------------------------

void SaveOptions() {
	Handle rgnTypeHdl;
	short saveFileRef = CurResFile();

	UseResFile(gAppFileRef);

	rgnTypeHdl = Get1NamedResource('OPTN', "\pOptions");
	**(ResType**)rgnTypeHdl = GetRegionType();
	ChangedResource(rgnTypeHdl);		// Tell Resource Manager the rsrc was changed!
	WriteResource(rgnTypeHdl);			// Tell R. M. to write out changes right now!
	ReleaseResource(rgnTypeHdl);

	UseResFile(saveFileRef);
} // END SaveOptions

// ---------------------------------------------------------------------------

void ShowClipboard() {
	PicHandle thePic;
	GrafPtr savePort;
	WindowPtr clipboardWindow;
	Rect picRect;
	
	thePic = PictureFromScrap();

	if (thePic == nil) {
		DoError("\pNo picture on the clipboard to view.");
		return;
	}

	GetPort(&savePort);

	picRect = (**thePic).picFrame;
	
	if (picRect.left != 0) {
		picRect.right += picRect.left;
		picRect.left = 0;
	}
	if (picRect.top != 0) {
		picRect.bottom += picRect.top;
		picRect.top = 0;
	}

	if ((picRect.right - picRect.left < 20) || (picRect.bottom - picRect.top < 20))
		SetRect(&picRect, 0, 0, 24, 24);

	CenterRect(&picRect, &screenBits.bounds);
	clipboardWindow = NewWindow(nil, &picRect, "\pClipboard", false, noGrowDocProc,
		(WindowPtr)-1, false, 0);

	SetPort(clipboardWindow);
	ShowWindow(clipboardWindow);

	HLockHi((Handle)thePic);
	DrawPicture(thePic, &(**thePic).picFrame);
	HUnlock((Handle)thePic);
	HPurge((Handle)thePic);

	do {
		EventRecord dummyEvent;
		if (GetNextEvent(everyEvent, &dummyEvent))
			if (dummyEvent.what == mouseDown || dummyEvent.what == keyDown)
				break;
	} while (!Button());
	FlushEvents(everyEvent, 0);

	DisposeWindow(clipboardWindow);
	SetPort(savePort);
} // END ShowClipboard


// ---------------------------------------------------------------------------

enum {
	regionTypeItem = 3
};

void DoOptionsDialog() {
	GrafPtr savePort;
	DialogPtr optionsDialog;
	short itemHit;
	Boolean done = false;
	ResType rgnType;
	Str31 rgnTypeStr;
	
	GetPort(&savePort);
	
	optionsDialog = GetNewDialog(kOptionsDialogID, nil, (WindowPtr)-1);
	SetPort(optionsDialog);

	rgnType = GetRegionType();
	rgnTypeStr[0] = 4;
	for (short i = 1; i <= 4; i++)
		rgnTypeStr[i] = ((unsigned char*)&rgnType)[i-1];
	SetDItemText(optionsDialog, regionTypeItem, rgnTypeStr);

	ShowWindow(optionsDialog);
	OutlineDefaultButton(optionsDialog, ok);
	SelIText(optionsDialog, regionTypeItem, 0, 32767);
	
	do {
		ModalDialog(nil, &itemHit);
		switch(itemHit) {
			case cancel:
				done = true;
			break;
			case ok:
				GetIText(GetDItemHdl(optionsDialog, regionTypeItem), rgnTypeStr);
				if (rgnTypeStr[0] != 4) {
					SysBeep(0);
					SelIText(optionsDialog, regionTypeItem, 0, 32767);
				}
				else {
					for (i = 1; i <= 4; i++)
						((unsigned char*)&rgnType)[i-1] = rgnTypeStr[i];
					SetRegionType(rgnType);
					done = true;
				}
			break;
		} // END switch
	} while (!done);
	
	DisposeDialog(optionsDialog);
	SetPort(savePort);
} // END DoOptionsDialog

// ---------------------------------------------------------------------------