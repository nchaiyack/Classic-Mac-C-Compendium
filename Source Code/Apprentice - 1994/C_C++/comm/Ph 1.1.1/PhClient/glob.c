/*______________________________________________________________________

	glob.c - Global Variables and Functions.
_____________________________________________________________________*/

#pragma load "precompile"
#include "MacTCPCommonTypes.h"
#include "rez.h"
#include "utl.h"
#include "glob.h"
#include "serv.h"
#include "mtcp.h"
#include "oop.h"
#include "rslv.h"

/*______________________________________________________________________

	Variables.
_____________________________________________________________________*/

short				NumMenus;			/* number of menus in menu bar */
MenuHandle		AppleMenu;			/* handle to apple menu */
MenuHandle		FileMenu;			/* handle to file menu */
MenuHandle		EditMenu;			/* handle to edit menu */
MenuHandle		ProxyMenu;			/* handle to proxy menu */
MenuHandle		WindowsMenu;		/* handle to windows menu */
MenuHandle		HelpMenu;			/* handle to help menu */
CursHandle		IBeamHandle;		/* handle to ibeam cursor */
CursHandle		WatchHandle;		/* handle to watch cursor */
CursHandle		BBArray[numBB];	/* array of handles to beachball cursors */
RgnHandle		BigClipRgn;			/* infinite clip region */
Boolean			Done = false;		/* true when time to quit */
short				PhPort;				/* Ph server port number */
short				FontNum;				/* monaco font number */
short				Ascent;				/* font ascent */
short				Descent;				/* font descent */
short				Leading;				/* font leading */
short				LineHeight;			/* font line height */
MenuHandle		SiteMenu = nil;	/* handle to site popup menu */
short				NumSites;			/* number of sites in menu */
Handle			Servers;				/* handle to list of server domain names */
Handle			Domains;				/* handle to list of email domain names */
Boolean			HaveBalloons;		/* true if balloon help available */
Handle			HelpTopics;			/* handle to help topic list, or nil if
												help not available */
Boolean			HelpNative;			/* true if only native help text avail */
Str255			HelpServer;			/* current help server */
short				HelpMenuLength;	/* length of initial part of help menu */
unsigned long	LastSiteUpdate;	/* time of last site list update */
unsigned long	LastHelpUpdate;	/* time of last help menu update */
WindState		TranState;			/* saved transaction window state */
WindState		LoginState;			/* saved login window state */
WindState		NewState;			/* saved new window state */
WindState		OpenState;			/* saved open window state */
WindState		PswdState;			/* saved pswd window state */
WindState		SiteState;			/* saved site window state */
WindState		QueryStates[numPosSave];	/* saved query window states */
WindState		EditStates[numPosSave];		/* saved edit window states */
WindState		HelpStates[numPosSave];		/* saved help window states */
Str255			DefaultServer;		/* default server domain name */
Str255			DbOffMsg = "\p";	/* database turned off error message */

/*_____________________________________________________________________

	glob_Error - Issue Error Message Alert.
		
	Entry:	rezID = resource id of STR# resource.
				index = index in STR# resource of error message.
				p0 = param to plug into error message.
_____________________________________________________________________*/

void glob_Error (short rezID, short index, Str255 p0)

{
	glob_BringToFront();
	utl_ErrorAlertRez(rezID, index, errorMsgID, 
		oop_ModalUpdate, p0, nil, nil, nil);
}

/*_____________________________________________________________________

	glob_ErrorCode - Issue MacTCP Error Message Alert.
		
	Entry:	rCode = error code.
_____________________________________________________________________*/

void glob_ErrorCode (OSErr rCode)

{
	short			i;				/* index in STR# rsrc of error message */
	Str255		p0;			/* parameter to plug into error message */

	if (rCode == mtcpCancel) {
		return;
	} else if (rCode == ipBadCnfgErr) {
		i = msgBadCnfg;
	} else if (rCode == rslvNoServers) {
		i = msgNoServers;
	} else if (rCode == badUnitErr || rCode == unitEmptyErr) {
		i = msgBadUnit;
	} else if (rCode == ipDestDeadErr) {
		i = msgDestDead;
	} else if (rCode == authNameErr || rCode == rslvNoSuchDomain) {
		i = msgNoSuchName;
	} else if (rCode == noNameServer) {
		i = msgNoNameServer;
	} else if (rCode == openFailed || rCode == commandTimeout) {
		i = msgOpenFailed;
	} else if (rCode == connectionTerminated || rCode == connectionClosing) {
		if (*DbOffMsg) {
			glob_Error(servErrors, msgDbOff, DbOffMsg);
			return;
		}
		i = msgCTerminated;
	} else if (rCode == noAnsErr || rCode == rslvNoResponse) {
		i = msgNoAnswer;
	} else if (rCode == ipBadAddr) {
		i = msgBadAddr;
	} else if (rCode == cacheFault) {
		i = msgCacheFault;
	} else if (rCode == dnrErr) {
		i = msgDnrErr;
	} else if (rCode == rslvNameSyntaxErr) {
		i = msgDnrSynErr;
	} else {
		i = msgUnexpected;
		NumToString(rCode, p0);
	}
	glob_Error(macTCPErrors, i, p0);
}

/*_____________________________________________________________________

	glob_ServErrorCode - Issue Server Error Message Alert.
	
	Entry:	sCode = server response code.
				alias = alias.
_____________________________________________________________________*/

void glob_ServErrorCode (short sCode)

{
	short		i;				/* index in STR# rsrc of error message */
	Str255	p0;			/* parameter to plug into error message */
	
	if (sCode == phFailReadOnly) {
		i = msgReadOnly;
		serv_GetReadOnlyReason(p0);
	} else if (sCode == phNoMatches) {
		i = msgNoSuchRecord;
	} else if (sCode == phManyMatches) {
		i = msgNameNotUniq;
	} else if (sCode == phIllegalVal) {
		i = msgIllegal;
	} else if (sCode == phPermErr) {
		i = msgBadLogin;
	} else if (sCode == phLockTimeout) {
		i = msgLockTimeout;
	} else if (sCode == phFieldNotThere) {
		i = msgLoginNoAlias;
	} else {
		i = msgCodeErr;
		NumToString(sCode, p0);
	}
	glob_Error(servErrors, i, p0);
}

/*_____________________________________________________________________

	glob_CheckPswdSel - Check Password Selection
	
	Entry:	theDialog = pointer to movable modal dialog record.
	
	This function checks the current TextEdit field in the dialog, 
	which should be a password field. If the selection range is not
	either the entire password or an insertion point at the end of the
	password, the selection range is reset to be an insertion point at
	the end of the password.
_____________________________________________________________________*/

void glob_CheckPswdSel (DialogPtr theDialog)

{
	TEHandle			textH;			/* handle to active TextEdit record */
	short				selStart;		/* selection start pos */
	short				selEnd;			/* selection end pos */
	short				nSel;				/* number of chars in selection */
	short				teLength;		/* size of active TextEdit record */
	
	textH = ((DialogPeek)theDialog)->textH;
	selStart = (**textH).selStart;
	selEnd = (**textH).selEnd;
	nSel = selEnd - selStart;
	teLength = (**textH).teLength;
	if ((selStart < teLength || selEnd < teLength) && nSel < teLength) {
		SetPort(theDialog);
		TESetSelect(teLength, teLength, textH);
	}
}

/*_____________________________________________________________________

	glob_FilterPswdChar - Filter Password Character.
	
	Entry:	theDialog = pointer to movable modal dialog record.
				key = the character.
				modifiers = modifiers field from event record.
				pswd = pointer to password string.
				
	Exit:		function result = true if character should be processed,
					false if it should be discarded.
					
	This function processes keys pressed when a password field is
	active. The actual password string is kept separately, and the field
	displays only bullets.
_____________________________________________________________________*/

Boolean glob_FilterPswdChar (DialogPtr theDialog, char key, 
	short modifiers, Str255 pswd)

{
	TEHandle			textH;			/* handle to active TextEdit record */
	short				teLength;		/* size of active TextEdit record */
	short				nSel;				/* number of chars in selection */

	glob_CheckPswdSel(theDialog);
	textH = ((DialogPeek)theDialog)->textH;
	teLength = (**textH).teLength;
	nSel = (**textH).selEnd - (**textH).selStart;
	if (key == deleteKey) {
		if (nSel) {
			*pswd = 0;
		} else if (*pswd) {
			(*pswd)--;
		}
		return true;
	} else if (utl_StandardAsciiChar(key) && *pswd-nSel < 255) {
		Event.message = '¥';
		if (nSel) *pswd = 0;
		(*pswd)++;
		*(pswd + *pswd) = key;
		return true;
	} else if (key == tabKey || key == returnKey || 
		key == enterKey || key == escapeKey ||
		((modifiers & cmdKey) && key == '.')) {
		return true;
	} else {
		glob_Error(servErrors, msgBadFieldChar, nil);
		return false;
	}
}

/*_____________________________________________________________________

	glob_FilterPswdEditCmd - Filter Password Edit Command.
	
	Entry:	theDialog = pointer to movable modal dialog record.
				theItem = menu item.
				pswd = pointer to password string.
					
	This function processes the Edit menu commands Copy and Paste when a 
	password field is active.  The actual password string is kept separately, 
	and the field displays only bullets.
_____________________________________________________________________*/

void glob_FilterPswdEditCmd (DialogPtr theDialog, short theItem, Str255 pswd)

{
	TEHandle			textH;			/* handle to active TextEdit record */
	short				teLength;		/* size of active TextEdit record */
	short				nSel;				/* number of chars in selection */
	Handle			scrap;			/* handle to scrap to be pasted */
	short				scrapLen;		/* length of scrap to be pasted */
	char				*p;				/* pointer to scrap char */

	glob_CheckPswdSel(theDialog);
	textH = ((DialogPeek)theDialog)->textH;
	teLength = (**textH).teLength;
	nSel = (**textH).selEnd - (**textH).selStart;
	if (theItem == clearCmd) {
		if (nSel) {
			*pswd = 0;
			oop_DoCommand(theDialog, editID, clearCmd);
		}
	} else if (theItem == pasteCmd) {
		if (!glob_FilterPaste()) return;
		TEFromScrap();
		scrap = TEScrapHandle();
		scrapLen = GetHandleSize(scrap);
		if (scrapLen - nSel > 255) return;
		if (nSel) {
			*pswd = 0;
			TEDelete(textH);
		}
		p = *scrap;
		while (scrapLen--) {
			*(pswd + *pswd + 1) = *p;
			(*pswd)++;
			p++;
			TEKey('¥', textH);
		}
	}
}

/*_____________________________________________________________________

	glob_FilterAsciiChar - Filter Server or Alias/Name Character.
	
	Entry:	theDialog = pointer to movable modal dialog record.
				key = the character.
				modifiers = modifiers field from event record.
				
	Exit:		function result = true if character should be processed,
					false if it should be discarded.
					
	This function processes keys pressed when a dialog server or alias/name
	field is active. 
_____________________________________________________________________*/

Boolean glob_FilterAsciiChar (DialogPtr theDialog, 
	char key, short modifiers)

{
	TEHandle			textH;			/* handle to active TextEdit record */
	short				teLength;		/* size of active TextEdit record */
	short				nSel;				/* number of chars in selection */

	textH = ((DialogPeek)theDialog)->textH;
	teLength = (**textH).teLength;
	nSel = (**textH).selEnd - (**textH).selStart;
	if ((utl_StandardAsciiChar(key) && teLength-nSel < 255) ||
		key == tabKey || key == returnKey || key == deleteKey ||
		key == enterKey || key == escapeKey ||
		key == leftArrow || key == rightArrow ||
		((modifiers & cmdKey) && key == '.')) {
		return true;
	} else {
		glob_Error(servErrors, msgBadFieldChar, nil);
		return false;
	}
}

/*_____________________________________________________________________

	glob_FilterPaste - Filter Paste Command.
	
	Exit:		function result = true if the scrap contains only
					standard printable ascii characters.
_____________________________________________________________________*/

Boolean glob_FilterPaste (void)

{	
	Handle			scrap;		/* handle to scrap to be pasted */
	short				scrapLen;	/* length of scrap to be pasted */
	char				*p;			/* pointer to scrap char */

	TEFromScrap();
	scrap = TEScrapHandle();
	scrapLen = GetHandleSize(scrap);
	p = *scrap;
	while (scrapLen--) {
		if (!utl_StandardAsciiChar(*p)) {
			glob_Error(servErrors, msgBadFieldChar, nil);
			return false;
		};
		p++;
	}
	return true;
}

/*_____________________________________________________________________

	glob_DrawSitePopup - Draw Server Site Popup Menu.
	
	Entry:	rect = rectangle in which to draw the menu.
				curSel = index in menu of currently selected item.
_____________________________________________________________________*/

void glob_DrawSitePopup (Rect *rect, short curSel)

{
	Rect			frameRect;			/* framing rect */
	Str255		server;				/* currently selected server name */
	PolyHandle	triangle;			/* handle to triangle polygon */
	GrafPtr		curPort;				/* the grafport */
	short			oldFont;				/* saved font number */
	short			oldSize;				/* saved font size */
	
	GetPort(&curPort);
	oldFont = curPort->txFont;
	oldSize = curPort->txSize;
	TextFont(0);
	TextSize(12);
	frameRect = *rect;
	frameRect.left += 4;
	GetItem(SiteMenu, curSel, server);
	TextBox(server+1, *server, &frameRect, teJustLeft);
	frameRect.left -= 4;
	InsetRect(&frameRect, -1, -1);
	FrameRect(&frameRect);
	MoveTo(frameRect.right, frameRect.top+2);
	LineTo(frameRect.right, frameRect.bottom);
	LineTo(frameRect.left+2, frameRect.bottom);
	triangle = OpenPoly();
	MoveTo(frameRect.right-14, (frameRect.top + frameRect.bottom - 5)>>1);
	Line(10, 0);
	Line(-5, 5);
	Line(-5, -5);
	ClosePoly();
	PaintPoly(triangle);
	KillPoly(triangle);
	TextFont(oldFont);
	TextSize(oldSize);
}

/*_____________________________________________________________________

	glob_GetSiteIndex - Get Index of Site in Site Menu.
	
	Entry:	server = server domain name.
	
	Exit: 	function result = index in Site menu of corresponding site.
_____________________________________________________________________*/

short glob_GetSiteIndex (Str255 server)

{
	short					i;						/* loop index */
	char					*p;					/* pointer to server domain name */
	
	p = *Servers;
	for (i = 1; i <= NumSites; i++) {
		if (EqualString(p, server, true, true)) break;
		p += *p+1;
	}
	if (i > NumSites) i = 1;
	return i;
}

/*_____________________________________________________________________

	glob_PopupSiteMenu - Popup the Site Menu.
	
	Entry:	rect = popup rectangle.
				oldSel = old selected item.
	
	Exit: 	newSel = new selected item, or -1 if none selected.
				newDomain = new domain name, if newSel != -1.
_____________________________________________________________________*/

void glob_PopupSiteMenu (Rect *rect, short oldSel, short *newSel,
	Str255 newDomain)
	
{
	Point			popPoint;			/* location of popup menu, global coords */
	char			*p;					/* pointer to domain name */
	short			i;						/* loop index */
	long			result;				/* PopUpMenuSelect result */
	
	CheckItem(SiteMenu, oldSel, true);
	popPoint = *(Point*)rect;
	LocalToGlobal(&popPoint);
	InsertMenu(SiteMenu, -1);
	result = PopUpMenuSelect(SiteMenu, popPoint.v, popPoint.h, oldSel);
	DeleteMenu(popupID);
	CheckItem(SiteMenu, oldSel, false);
	if ((result >> 16) & 0xffff) {
		*newSel = result & 0xffff;
		InvalRect(rect);
		i = *newSel;
		p = *Servers;
		while (--i) p += *p+1;
		utl_CopyPString(newDomain, p);
	} else {
		*newSel = -1;
	}
}

/*_____________________________________________________________________

	glob_BringToFront - Bring program to front.
_____________________________________________________________________*/

void glob_BringToFront (void)

{
	NMRec			nRec;			/* Notification manger record */
	
	if (oop_InForeground()) return;
	nRec.qType = nmType;
	nRec.nmMark = 1;
	nRec.nmIcon = GetResource('SICN', phSmallIconID);
	nRec.nmSound = nil;
	nRec.nmStr = nil;
	nRec.nmResp = nil;
	NMInstall(&nRec);
	while (!oop_InForeground()) oop_DoEvent(nil, everyEvent, longSleep, nil);
	NMRemove(&nRec);
	oop_UpdateAll();
}
