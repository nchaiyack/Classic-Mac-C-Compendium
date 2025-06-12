/*********************************************************************
 * HexSearch.c
 *
 * HexEdit, a simple hex editor
 * copyright 1993, Jim Bumgardner
 *********************************************************************/
// Todo:
// 		Disable search buttons if text field is empty

#include "HexEdit.h"

#define SearchForwardItem	1
#define SearchBackwardItem	2
#define HexModeItem			3
#define AsciiModeItem		4
#define SearchTextItem		5

short			gSearchMode = EM_Ascii;
short			gSearchDir = 0;	// 1 = backward
unsigned char	gSearchBuffer[256],gSearchText[256],gGotoText[256];
DialogPtr		gSearchWin;
Boolean StringToSearchBuffer(void);




void OpenSearchDialog(EditWindowPtr dWin)
{
	Str255	sStr;
	// If Dialog Window isn't open
	if (gSearchWin == NULL) {
		// Open Dialog Window
		MySetCursor(C_Arrow);
		gSearchWin = GetNewDialog(SearchDLOG, NULL, (WindowPtr) -1L);
		if (gSearchWin) {
			// Convert Existing Search Scrap, if it exists to text
			SetText(gSearchWin, SearchTextItem, gSearchText);

			// Set Radio Buttons
			SetControl(gSearchWin, HexModeItem, gSearchMode == EM_Hex);
			SetControl(gSearchWin, AsciiModeItem, gSearchMode == EM_Ascii);
			SelIText(gSearchWin, SearchTextItem, 0, 32767);
			ShowWindow(gSearchWin);
		}
	}
	else
		SelectWindow(gSearchWin);
}

void PerformTextSearch(EditWindowPtr dWin)
{
	WindowPtr	wp;
	short		ch,matchIdx;
	long		addr,matchAddr;

	if (dWin == NULL) {
		// Find and Select Top Window
		wp = FrontWindow();
		while (wp && ((WindowPeek) wp)->refCon != MyWindowID)
			wp = (WindowPtr) ((WindowPeek) wp)->nextWindow;
		if (wp == NULL || ((WindowPeek) wp)->refCon != MyWindowID)
			return;
		dWin = (EditWindowPtr) wp;
	}

	MySetCursor(C_Watch);

	// Search in Direction gSearchDir
	// for text gSearchBuffer

	if (gSearchDir == 0)
		addr = dWin->endSel;
	else {
		addr = dWin->startSel - 1;
		if (addr < 0)
			return;
	}
	matchIdx = 0;
	while (1) {
		ch = GetByte(dWin, addr);
		if (ch == gSearchBuffer[matchIdx+1]) {
			if (matchIdx == 0)
				matchAddr = addr;
			++matchIdx;
			if (matchIdx >= gSearchBuffer[0])
				goto Success;
			++addr;
			if (addr == dWin->fileSize) {
				matchIdx = 0;
				addr = matchAddr;
			}
			else
				continue;
		}
		else {
			if (matchIdx) {
				matchIdx = 0;
				addr = matchAddr;
			}
		}
		if (gSearchDir == 0) {
			++addr;
			if (addr == dWin->fileSize)
				goto Failure;
		}
		else {
			--addr;
			if (addr < 0)
				goto Failure;
		}
	}

Failure:
	SysBeep(1);
	MySetCursor(C_Arrow);
	return;

Success:
	if (dWin != (EditWindowPtr) FrontWindow())
		SelectWindow((WindowPtr) dWin);
	dWin->startSel = matchAddr;
	dWin->endSel = dWin->startSel + gSearchBuffer[0];

	ScrollToSelection(dWin, dWin->startSel, true, true);
	MySetCursor(C_Arrow);
}


#define GAddrItem			3
#define GHexItem			4
#define GAsciiItem			5
#define GUserItem			6

pascal void GotoUserItem(DialogPtr dp, short itemNbr)
{
	switch (itemNbr) {
	case GUserItem:
		MyOutlineButton(dp, OK, black);
		break;
	}
	
}

void GotoAddress(EditWindowPtr dWin)
{
	GrafPtr		savePort;
	DialogPtr	dp;
	short		itemHit;
	short		t;
	Handle		h;
	Rect		r;

	GetPort(&savePort);
	MySetCursor(C_Arrow);
	dp = GetNewDialog(GotoDLOG, NULL, (WindowPtr) -1L);
	if (dp == NULL)
		return;
	GetDItem(dp, GUserItem, &t, &h, &r);
	SetDItem(dp, GUserItem, t, (Handle) GotoUserItem, &r);

	SetText(dp, GAddrItem, gGotoText);
	// Set Radio Buttons
	SetControl(dp, GHexItem, gPrefs.decimalAddr == EM_Hex);
	SetControl(dp, GAsciiItem, gPrefs.decimalAddr == EM_Ascii);
	SelIText(dp, GAddrItem, 0, 32767);
	ShowWindow(dp);
	do {
		ModalDialog(NULL, &itemHit);
		switch (itemHit) {
		case OK:
			GetText(dp, GAddrItem, gGotoText);
		case Cancel:
			break;
		case GHexItem:
			gPrefs.decimalAddr = EM_Hex;
			SetControl(dp, GHexItem, gPrefs.decimalAddr == EM_Hex);
			SetControl(dp, GAsciiItem, gPrefs.decimalAddr == EM_Ascii);
			break;
		case GAsciiItem:
			gPrefs.decimalAddr = EM_Ascii;
			SetControl(dp, GHexItem, gPrefs.decimalAddr == EM_Hex);
			SetControl(dp, GAsciiItem, gPrefs.decimalAddr == EM_Ascii);
			break;
		}
	} while (itemHit != OK && itemHit != Cancel);
	DisposDialog(dp);
	SetPort(savePort);
	if (itemHit == OK) {
		long		addr = -1;
		short		r;

		PtoCstr(gGotoText);
		if (gPrefs.decimalAddr == EM_Hex)
			r = sscanf((char *) gGotoText,"%lx",&addr);
		else
			r = sscanf((char *) gGotoText,"%ld",&addr);
		CtoPstr((char *) gGotoText);
		if (r == 1 && addr >= 0 && addr < dWin->fileSize) {
			dWin->startSel = dWin->endSel = addr;
			ScrollToSelection(dWin, addr, true, true);
		}
	}
}


void DoModelessDialogEvent(EventRecord *theEvent)
{
	DialogPtr	whichDlog;
	short		itemHit;
	// Do Event Filtering
	if (theEvent->what == keyDown) {
		// Process Edit Keys
		if ((theEvent->message & charCodeMask) == '\r') {
			whichDlog = FrontWindow();
			itemHit = OK;
			MySimulateButtonPress(whichDlog, OK);
			goto ButtonHit;
		}
	}
	if(DialogSelect(theEvent, &whichDlog, &itemHit)) {
ButtonHit:
		if (whichDlog == gSearchWin) {
			switch (itemHit) {
			case SearchForwardItem:
			case SearchBackwardItem:
				gSearchDir = (itemHit == SearchBackwardItem);
				GetText(gSearchWin, SearchTextItem, gSearchText);
				if (StringToSearchBuffer()) {
					PerformTextSearch(NULL);
				}
				break;
			case HexModeItem:
				gSearchMode = EM_Hex;
				SetControl(gSearchWin, HexModeItem, gSearchMode == EM_Hex);
				SetControl(gSearchWin, AsciiModeItem, gSearchMode == EM_Ascii);
				break;
			case AsciiModeItem:
				gSearchMode = EM_Ascii;
				SetControl(gSearchWin, HexModeItem, gSearchMode == EM_Hex);
				SetControl(gSearchWin, AsciiModeItem, gSearchMode == EM_Ascii);
				break;
			case SearchTextItem:
				break;
			}
		}
	}
	else {
		if (theEvent->what == updateEvt &&
			whichDlog == gSearchWin) {
			MyOutlineButton(gSearchWin, SearchForwardItem, black);
		}
	}
}

Boolean StringToSearchBuffer(void)
{
	Ptr				sp, dp;
	short			i;
	short			val;
	Boolean			loFlag;

	// Convert String to gSearchBuffer
	if (gSearchMode == EM_Hex) {
		sp = (Ptr) &gSearchText[1];
		dp = (Ptr) &gSearchBuffer[1];
		loFlag = false;
		for (i = 0; i < gSearchText[0]; ++i,++sp) {
			if (*sp == '0' && *(sp+1) == 'x') {
				loFlag = 0;
				++sp;
				++i;
				continue;
			}
			if (isspace(*sp) || ispunct(*sp)) {
				loFlag = 0;
				continue;
			}
			if (*sp >= '0' && *sp <= '9')
				val = *sp - '0';
			else if (*sp >= 'A' && *sp <= 'F')
				val = 0x0A + (*sp - 'A');
			else if (*sp >= 'a' && *sp <= 'f')
				val = 0x0A + (*sp - 'a');
			else
				goto HexError;
			if (loFlag) {
				*(dp-1) = (*(dp-1) << 4) | val;
				loFlag = 0;
			}			
			else {
				*dp = val;
				++dp;
				loFlag = 1;
			}
		}
		gSearchBuffer[0] = (long) dp - (long) &gSearchBuffer[1];
		if (gSearchBuffer[0] == 0)
			goto HexError;
	}
	else {
		BlockMove(gSearchText, gSearchBuffer, gSearchText[0]+1);
	}
	return true;
HexError:
	ErrorAlert(ES_Caution, "Only valid Hex values may be used");
	return false;
}
