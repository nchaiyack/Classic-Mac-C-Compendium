/*----------------------------------------------------------------------------

	prefsdlog.c

	This module handles the preferences dialog.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <Packages.h>

#include "glob.h"
#include "prefsdlog.h"
#include "dlgutil.h"
#include "util.h"
#include "popup.h"
#include "font.h"
#include "sfutil.h"
#include "log.h"



/* Items common to all topics. The kInit and kTerm items are pseudo-items
   used to intialize and terminate the topics. The OK and Cancel buttons
   are also common to all topics (item numbers 1 and 2). */

#define kTopicPopup			3
#define kInit				100
#define kTerm				101

#define kPrefsDlg			300
#define kGeneralOptions		1
#define kServerAddresses	2
#define kServerOptions		3
#define kPersonalInfo		4
#define kSignature			5
#define kRemoteHost			6
#define kFontSize			7
#define kSavedText			8
#define kAuthorization		9

/* General options topic. */

#define kShowAuthors		4
#define kShowHeaders		5
#define kCollapseThreads	6
#define kKeypadShortcuts	7
#define kZoomWindows		8
#define kShowKilled			9
#define kCopyReplies		10
#define kCheckNewGroups		11
#define kAreYouSure			12
#define kLogActionsToFile	13
#define kAutoSaveOnQuit		14
#define kExpandHilited		15
#define kMaxArticles		17

/* Server info topic. */

#define kNewsServer			6
#define kMailServer			8

/* Server options topic. */

#define kBatchedGroupCmds	5
#define kUseXPAT			6
#define kNoNewConnection	7
#define kNoModeReader		8

/* Personal info topic. */

#define kFullName			6
#define kOrganization		8
#define kMailAddress		10

/* Signature topic. */

#define kSigStaticText		4
#define kSignature			5
#define kAddSigSeparator	6

/* Remote host info topic. */

#define	kRemoteHost			6
#define	kRemoteLogin		8
#define	kRemotePassword		10
#define kRemotePath			12
#define kAutoGetPut			13
#define	kSavePassword		14

/* Font/size topic. */

#define kListFontPopup 		5
#define kListSizeBox		7
#define kListSizePopup		8
#define kListSample			9
#define kListVertLine		10
#define kTextFontPopup		12
#define kTextSizeBox		14
#define kTextSizePopup		15
#define kTextSample			16

/* Saved text file topic. */

#define kAppBorder			5
#define kAppName			6
#define kAppButton			7
#define kDirCheckBox		8
#define kDirBorder			9
#define kDirName			10
#define kDirButton			11

/* Authoriation topic. */




static Str255 gStaticText;		/* static text for signature dialog */

static short gListFontID;		/* current list font id */
static short gListFontSize;		/* current list font size */
static short gTextFontID;		/* current text font id */
static short gTextFontSize;		/* current text font size */



/*----------------------------------------------------------------------------
	GeneralOptions

	Handles the general options preferences dialog.
	
	Entry:	prefs = pointer to new prefs.
			dlg = pointer to dialog.
			item = item number hit.
----------------------------------------------------------------------------*/

static void GeneralOptions (TPrefRec *prefs, DialogPtr dlg, short item)
{
	switch (item) {
	
		case kInit:
		
			DlgSetCheck(dlg, kShowAuthors, prefs->showAuthors);
			DlgSetCheck(dlg, kShowHeaders, prefs->showHeaders);
			DlgSetCheck(dlg, kCollapseThreads, prefs->showThreadsCollapsed);
			DlgSetCheck(dlg, kKeypadShortcuts, prefs->keypadShortcuts);
			DlgSetCheck(dlg, kZoomWindows, prefs->zoomWindows);
			DlgSetCheck(dlg, kShowKilled, prefs->showKilledArticles);
			DlgSetCheck(dlg, kCopyReplies, prefs->replyCC);
			DlgSetCheck(dlg, kCheckNewGroups, prefs->checkForNewGroups);
			DlgSetCheck(dlg, kAreYouSure, prefs->areYouSureAlert);
			DlgSetCheck(dlg, kLogActionsToFile, prefs->logActionsToFile);
			DlgSetCheck(dlg, kAutoSaveOnQuit, prefs->autoSaveOnQuit);
			DlgSetCheck(dlg, kExpandHilited, prefs->expandHilited);
			DlgSetNumber(dlg, kMaxArticles, prefs->maxFetch);
			SetItemNumeric(dlg, kMaxArticles, true);
			SetItemMaxLength(dlg, kMaxArticles, 6);
			SelIText(dlg, kMaxArticles, 0, 0x7fff);
			#ifndef FILTERS
				DlgSetCheck(dlg, kShowKilled, false);
				DlgSetCheck(dlg, kExpandHilited, false);
				DlgEnableItem (dlg, kShowKilled, false);
				DlgEnableItem (dlg, kExpandHilited, false);
			#endif
			break;
			
		case kTerm:
			
			prefs->showAuthors = DlgGetCheck(dlg, kShowAuthors);
			prefs->showHeaders = DlgGetCheck(dlg, kShowHeaders);
			prefs->showThreadsCollapsed = DlgGetCheck(dlg, kCollapseThreads);
			prefs->keypadShortcuts = DlgGetCheck(dlg, kKeypadShortcuts);
			prefs->zoomWindows = DlgGetCheck(dlg, kZoomWindows);
			prefs->showKilledArticles = DlgGetCheck(dlg, kShowKilled);
			prefs->replyCC = DlgGetCheck(dlg, kCopyReplies);
			prefs->checkForNewGroups = DlgGetCheck(dlg, kCheckNewGroups);
			prefs->areYouSureAlert = DlgGetCheck(dlg, kAreYouSure);
			prefs->logActionsToFile = DlgGetCheck(dlg, kLogActionsToFile);
			prefs->autoSaveOnQuit = DlgGetCheck(dlg, kAutoSaveOnQuit);
			prefs->expandHilited = DlgGetCheck(dlg, kExpandHilited);
			prefs->maxFetch = DlgGetNumber(dlg, kMaxArticles);
			break;
			
		case kShowAuthors:
		case kShowHeaders:
		case kCollapseThreads:
		case kKeypadShortcuts:
		case kZoomWindows:
		case kShowKilled:
		case kCopyReplies:
		case kCheckNewGroups:
		case kAreYouSure:
		case kLogActionsToFile:
		case kAutoSaveOnQuit:
		case kExpandHilited:
		
			DlgToggleCheck(dlg, item);
			break;
			
	}
}



/*----------------------------------------------------------------------------
	Server

	Handles the server info preferences dialog.
	
	Entry:	prefs = pointer to new prefs.
			dlg = pointer to dialog.
			item = item number hit.
----------------------------------------------------------------------------*/

static void Server (TPrefRec *prefs, DialogPtr dlg, short item)
{
	Boolean enable;

	switch (item) {
	
		case kInit:
		
			DlgSetPString(dlg, kNewsServer, prefs->newsServerName);
			SetItemMaxLength(dlg, kNewsServer, 255);
			DlgSetPString(dlg, kMailServer, prefs->mailServerName);
			SetItemMaxLength(dlg, kMailServer, 255);
			if (*prefs->newsServerName == 0) {
				SelIText(dlg, kNewsServer, 0, 0);
			} else if (*prefs->mailServerName == 0) {
				SelIText(dlg, kMailServer, 0, 0);
			} else {
				SelIText(dlg, kNewsServer, 0, 0x7fff);
			}
			break;
			
		case kTerm:
			
			break;
			
		case kNewsServer:
		case kMailServer:
		
			DlgGetPString(dlg, kNewsServer, prefs->newsServerName);
			DlgGetPString(dlg, kMailServer, prefs->mailServerName);
			enable = *prefs->newsServerName != 0 && *prefs->mailServerName != 0;
			DlgEnableItem(dlg, ok, enable);
			DlgEnableItem(dlg, kTopicPopup, enable);
			
	}
}



/*----------------------------------------------------------------------------
	ServerOptions

	Handles the server options preferences dialog.
	
	Entry:	prefs = pointer to new prefs.
			dlg = pointer to dialog.
			item = item number hit.
----------------------------------------------------------------------------*/

static void ServerOptions (TPrefRec *prefs, DialogPtr dlg, short item)
{
	switch (item) {
	
		case kInit:
		
			DlgSetCheck(dlg, kBatchedGroupCmds, prefs->batchedGroupCmds);
			DlgSetCheck(dlg, kUseXPAT, prefs->useXPAT);
			DlgSetCheck(dlg, kNoNewConnection, prefs->noNewConnection);
			DlgSetCheck(dlg, kNoModeReader, prefs->noModeReader);
			break;
			
		case kTerm:
			
			prefs->batchedGroupCmds = DlgGetCheck(dlg, kBatchedGroupCmds);
			prefs->useXPAT = DlgGetCheck(dlg, kUseXPAT);
			prefs->noNewConnection = DlgGetCheck(dlg, kNoNewConnection);
			prefs->noModeReader = DlgGetCheck(dlg, kNoModeReader);
			break;
			
		case kBatchedGroupCmds:
		case kUseXPAT:
		case kNoNewConnection:
		case kNoModeReader:
		
			DlgToggleCheck(dlg, item);
			break;
			
	}
}



/*----------------------------------------------------------------------------
	Personal

	Handles the personal info preferences dialog.
	
	Entry:	prefs = pointer to new prefs.
			dlg = pointer to dialog.
			item = item number hit.
----------------------------------------------------------------------------*/

static void Personal (TPrefRec *prefs, DialogPtr dlg, short item)
{
	switch (item) {
	
		case kInit:
		
			DlgSetCString(dlg, kFullName, prefs->fullName);
			SetItemMaxLength(dlg, kFullName, 255);
			DlgSetCString(dlg, kOrganization, prefs->organization);
			SetItemMaxLength(dlg, kOrganization, 255);
			DlgSetCString(dlg, kMailAddress, prefs->address);
			SetItemMaxLength(dlg, kMailAddress, 255);
			if (*prefs->fullName == 0) {
				SelIText(dlg, kFullName, 0, 0);
			} else if (*prefs->organization == 0) {
				SelIText(dlg, kOrganization, 0, 0);
			} else if (*prefs->address == 0) {
				SelIText(dlg, kMailAddress, 0, 0);
			} else {
				SelIText(dlg, kFullName, 0, 0x7fff);
			}
			break;
			
		case kTerm:
			
			DlgGetCString(dlg, kFullName, prefs->fullName);
			DlgGetCString(dlg, kOrganization, prefs->organization);
			break;
			
		case kMailAddress:
		
			DlgGetCString(dlg, kMailAddress, prefs->address);
			DlgEnableItem(dlg, ok, *prefs->address != 0);
			DlgEnableItem(dlg, kTopicPopup, *prefs->address != 0);
			
	}
}



/*----------------------------------------------------------------------------
	DrawSigStaticText

	This user item procedure draws the static text item in the signature dialog. 
	We need this because the dialog's font has been changed for editing the
	signature.
	
	Entry:	dlg = pointer to dialog.
			item = item number.
----------------------------------------------------------------------------*/

static pascal void DrawSigStaticText (DialogPtr dlg, short item)
{
	short itemType;
	Handle itemHandle;
	Rect box;
	short oldFontNum, oldFontSize;
	
	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	oldFontNum = dlg->txFont;
	oldFontSize = dlg->txSize;
	TextFont(0);
	TextSize(12);
	TextBox(gStaticText+1, *gStaticText, &box, teJustLeft);
	TextFont(oldFontNum);
	TextSize(oldFontSize);
}



/*----------------------------------------------------------------------------
	Signature

	Handles the signature preferences dialog.
	
	Entry:	prefs = pointer to new prefs.
			dlg = pointer to dialog.
			item = item number hit.
----------------------------------------------------------------------------*/

static void Signature (TPrefRec *prefs, DialogPtr dlg, short item)
{
	short fontNum, fontSize;
	short itemType;
	Rect box;
	Handle itemHandle;
	TEHandle textH;
	FontInfo fontInfo;
	short itemSize;

	switch (item) {
	
		case kInit:
		
			DlgSetCheck(dlg, kAddSigSeparator, prefs->addSigSeparatorLine);
		
			/* Stuff the signature (maybe longer than 255) into the item */
			
			GetDItem(dlg, kSignature, &itemType, &itemHandle, &box);
			PtrToXHand(prefs->signature, itemHandle, strlen(prefs->signature));
			SelIText(dlg, kSignature, 0, 0x7fff);
			SetItemReturnIsLegal(dlg, kSignature, true);
			SetItemMaxLength(dlg, kSignature, 4095);
	
			/* Set the dialog's text edit area to use a different font */
			
			SetPort(dlg);
			GetFNum(prefs->textFont, &fontNum);
			fontSize = prefs->textSize;
			textH = ((DialogPeek)dlg)->textH;
			(**textH).txFont = fontNum;		/* Set it for TextEdit */
			(**textH).txSize = fontSize;
			TextFont(fontNum);				/* And for QuickDraw */
			TextSize(fontSize);
			GetFontInfo(&fontInfo);			/* Adjust the TE line heights */
			(**textH).lineHeight = fontInfo.ascent+fontInfo.descent+fontInfo.leading;
			(**textH).fontAscent = fontInfo.ascent;
			TECalText(textH);
	
			/* Change the static text into a userItem so it can use the system font */
			
			GetDItem(dlg, kSigStaticText, &itemType, &itemHandle, &box);
			GetIText(itemHandle, gStaticText);
			SetDItem(dlg, kSigStaticText, userItem, (Handle)DrawSigStaticText, &box);
			break;
			
		case kTerm:
			
			prefs->addSigSeparatorLine = DlgGetCheck(dlg, kAddSigSeparator);
			GetDItem(dlg, kSignature, &itemType, &itemHandle, &box);
			itemSize = GetHandleSize(itemHandle);
			BlockMove(*itemHandle, prefs->signature, itemSize);
			prefs->signature[itemSize] = 0;
	
			/* Reset the dialog's text edit area to Chicago 12 */
			
			SetPort(dlg);
			fontNum = 0;
			fontSize = 12;
			textH = ((DialogPeek)dlg)->textH;
			(**textH).txFont = fontNum;		/* Set it for TextEdit */
			(**textH).txSize = fontSize;
			TextFont(fontNum);				/* And for QuickDraw */
			TextSize(fontSize);
			GetFontInfo(&fontInfo);			/* Adjust the TE line heights */
			(**textH).lineHeight = fontInfo.ascent+fontInfo.descent+fontInfo.leading;
			(**textH).fontAscent = fontInfo.ascent;
			TECalText(textH);
			break;
			
		case kAddSigSeparator:
		
			DlgToggleCheck(dlg, kAddSigSeparator);
			break;
			
	}
}



/*----------------------------------------------------------------------------
	Remote

	Handles the remote host info preferences dialog.
	
	Entry:	prefs = pointer to new prefs.
			dlg = pointer to dialog.
			item = item number hit.
----------------------------------------------------------------------------*/

static void Remote (TPrefRec *prefs, DialogPtr dlg, short item)
{
	CStr255 tempStr;
	short len;

	switch (item) {
	
		case kInit:
		
			DlgSetCString(dlg, kRemoteHost, prefs->host);
			SetItemMaxLength(dlg, kRemoteHost, 255);
			DlgSetCString(dlg, kRemoteLogin, prefs->name);
			SetItemMaxLength(dlg, kRemoteLogin, 255);
			len = strlen(prefs->remotePassword);
			memset(tempStr, '¥', len);
			tempStr[len] = 0;
			DlgSetCString(dlg, kRemotePassword, tempStr);
			SetItemPassword(dlg, kRemotePassword, prefs->remotePassword);
			SetItemMaxLength(dlg, kRemotePassword, 31);
			DlgSetCString(dlg, kRemotePath, prefs->remotePath);
			SetItemMaxLength(dlg, kRemotePath, 31);
			DlgSetCheck(dlg, kAutoGetPut, prefs->autoFetchnewsrc);
			DlgSetCheck(dlg, kSavePassword, prefs->savePassword);
			if (*prefs->host == 0) {
				SelIText(dlg, kRemoteHost, 0, 0);
			} else if (*prefs->name == 0) {
				SelIText(dlg, kRemoteLogin, 0, 0);
			} else if (*prefs->remotePassword == 0) {
				SelIText(dlg, kRemotePassword, 0, 0);
			} else if (*prefs->remotePath == 0) {
				SelIText(dlg, kRemotePath, 0, 0);
			} else {
				SelIText(dlg, kRemoteHost, 0, 0x7fff);
			}
			break;
			
		case kTerm:
			
			DlgGetCString(dlg, kRemoteHost, prefs->host);
			DlgGetCString(dlg, kRemoteLogin, prefs->name);
			DlgGetCString(dlg, kRemotePath, prefs->remotePath);
			prefs->autoFetchnewsrc = DlgGetCheck(dlg, kAutoGetPut);
			prefs->savePassword = DlgGetCheck(dlg, kSavePassword);
			break;
		
		case kSavePassword:
		case kAutoGetPut:
		
			DlgToggleCheck(dlg, item);
			break;
			
	}
}



/*----------------------------------------------------------------------------
	DrawFontSample
	
	A user item procedure to draw the font samples.
	
	Entry:	dlg = pointer to dialog.
			item = item number.
----------------------------------------------------------------------------*/

static pascal void DrawFontSample (DialogPtr dlg, short item)
{
	short itemType;
	Rect box;
	Handle itemHandle;
	StringPtr text;
	short fontID, fontSize, saveFont, saveSize;
	GrafPtr curPort;

	DlgGrayBorderItem(dlg, item);
	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	if (item == kListSample) {
		text = "\pThis is the font and size in which NewsWatcher group and subject lists are displayed.";
		fontID = gListFontID;
		fontSize = gListFontSize;
	} else {
		text = "\pThis is the font and size in which NewsWatcher article and message windows are displayed.";
		fontID = gTextFontID;
		fontSize = gTextFontSize;
	}
	GetPort(&curPort);
	saveFont = curPort->txFont;
	saveSize = curPort->txSize;
	TextFont(fontID);
	TextSize(fontSize);
	InsetRect(&box, 2, 2);
	TextBox(text + 1, *text, &box, teFlushDefault);
	TextFont(saveFont);
	TextSize(saveSize);
}



/*----------------------------------------------------------------------------
	RedrawSample

	Force one of the sample text items to be redrawn.
	
	Entry:	dlg = pointer to dialog.
			item = item number.
----------------------------------------------------------------------------*/

static void RedrawSample (DialogPtr dlg, short item)
{
	short itemType;
	Handle itemHandle;
	Rect box;
	
	SetPort(dlg);
	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	InsetRect(&box, 2, 2);
	EraseRect(&box);
	InvalRect(&box);
}



/*----------------------------------------------------------------------------
	SetNewFont

	Set a new font in the font/size dialog.
	
	Entry:	dlg = pointer to dialog.
			listFont = true if list font, false if text font.
			fontNum = new font number.
----------------------------------------------------------------------------*/

static void SetNewFont (DialogPtr dlg, Boolean listFont, short fontNum)
{
	short numItems, itemType, i;
	Rect box;
	Handle itemHandle;
	Str255 itemStr, fontName;
	ControlHandle ctl;
	long size;

	/* Select the right font in the font popup menu. */
	
	ctl = DlgGetControl(dlg, listFont ? kListFontPopup : kTextFontPopup);
	GetFontName(fontNum, fontName);
	SetPopupValue(ctl, fontName, false);
	
	/* Adjust the size popup so the nice sizes are outlined. */

	ctl = DlgGetControl(dlg, listFont ? kListSizePopup : kTextSizePopup);
	numItems = GetCtlMax(ctl);
	for (i = 1; i <= numItems; i++) {
		GetPopupPString(ctl, i, itemStr);
		StringToNum(itemStr, &size);
		if (RealFont(fontNum, (short)size)) {
			SetPopupItemStyle(ctl, i, outline);
		} else {
			SetPopupItemStyle(ctl, i, 0);
		}
	}

	/* Force the sample text to be redrawn */
	
	RedrawSample(dlg, listFont ? kListSample : kTextSample);
}



/*----------------------------------------------------------------------------
	FontIDFromPopup

	Get the font ID corresponding the currently selected font in a popup menu.
	
	Entry:	dlg = pointer to dialog.
			item = item number of popup menu.
----------------------------------------------------------------------------*/

static short FontIDFromPopup (DialogPtr dlg, short item)
{
	ControlHandle ctl;
	short fontNum;
	Str255 fontName;

	ctl = DlgGetControl(dlg, item);
	GetPopupPString(ctl, kCurrentPopupItem, fontName);
	GetFNum(fontName, &fontNum);
	return fontNum;
}



/*----------------------------------------------------------------------------
	Font

	Handles the font and size preferences dialog.
	
	Entry:	prefs = pointer to new prefs.
			dlg = pointer to dialog.
			item = item number hit.
----------------------------------------------------------------------------*/

static void Font (TPrefRec *prefs, DialogPtr dlg, short item)
{
	Str255 tempStr;
	long num;

	switch (item) {
	
		case kInit:
		
			GetFNum(prefs->listFont, &gListFontID);
			gListFontSize = prefs->listSize;
			SetNewFont(dlg, true, gListFontID);
			DlgSetNumber(dlg, kListSizeBox, gListFontSize);
			SetItemNumeric(dlg, kListSizeBox, true);
			SetItemMaxLength(dlg, kListSizeBox, 2);
			SelIText(dlg, kListSizeBox, 0, 0x7fff);
			SetItemPopupTypeinItem(dlg, kListSizePopup, kListSizeBox);
			
			GetFNum(prefs->textFont, &gTextFontID);
			gTextFontSize = prefs->textSize;
			SetNewFont(dlg, false, gTextFontID);
			DlgSetNumber(dlg, kTextSizeBox, gTextFontSize);
			SetItemNumeric(dlg, kTextSizeBox, true);
			SetItemMaxLength(dlg, kTextSizeBox, 2);
			SetItemPopupTypeinItem(dlg, kTextSizePopup, kTextSizeBox);
			
	  		DlgSetUserItem(dlg, kListSample, DrawFontSample);
	  		DlgSetUserItem(dlg, kTextSample, DrawFontSample);
	  		DlgSetUserItem(dlg, kListVertLine, DlgGrayBorderItem);
			break;
			
		case kTerm:
			
		  	GetFontName(gListFontID, prefs->listFont);
		  	GetFontName(gTextFontID, prefs->textFont);
		  	prefs->listSize = gListFontSize;
		  	prefs->textSize = gTextFontSize;
			break;
			
		case kListFontPopup:
		
	  		gListFontID = FontIDFromPopup(dlg, item);
	  		SetNewFont(dlg, true, gListFontID);
			break;
			
		case kTextFontPopup:
		
	  		gTextFontID = FontIDFromPopup(dlg, item);
	  		SetNewFont(dlg, false, gTextFontID);
			break;			
	  	
		case kListSizePopup:
		
		  	GetPopupPString(DlgGetControl(dlg, item), kCurrentPopupItem, tempStr);
		  	DlgSetPString(dlg, kListSizeBox, tempStr);
		  	SelIText(dlg, kListSizeBox, 0, 0x7fff);
			/* Fall through! */
			
		case kListSizeBox:
		
		  	DlgGetPString(dlg, kListSizeBox, tempStr);
		  	StringToNum(tempStr, &num);
		  	if (num != 0 && num != gListFontSize) {
		  		gListFontSize = num;
		  		RedrawSample(dlg, kListSample);
		  	}
			break;
	  	
		case kTextSizePopup:
		
		  	GetPopupPString(DlgGetControl(dlg, item), kCurrentPopupItem, tempStr);
		  	DlgSetPString(dlg, kTextSizeBox, tempStr);
		  	SelIText(dlg, kListSizeBox, 0, 0x7fff);
			/* Fall through! */
			
		case kTextSizeBox:
		
		  	DlgGetPString(dlg, kTextSizeBox, tempStr);
		  	StringToNum(tempStr, &num);
		  	if (num != 0 && num != gTextFontSize) {
		  		gTextFontSize = num;
		  		RedrawSample(dlg, kTextSample);
		  	}
			break;
			
	}
}



/*----------------------------------------------------------------------------
	Saved

	Handles the saved text file preferences dialog.
	
	Entry:	prefs = pointer to new prefs.
			dlg = pointer to dialog.
			item = item number hit.
----------------------------------------------------------------------------*/

static void Saved (TPrefRec *prefs, DialogPtr dlg, short item)
{
	StandardFileReply reply;
	FInfo fInfo;
	long dirID;
	Str31 name;
	OSErr err;
	CInfoPBRec cpb;
	HParamBlockRec pb;

	switch (item) {
	
		case kInit:

	  		DlgSetUserItem(dlg, kAppBorder, DlgGrayBorderItem);
			DlgSetPString(dlg, kAppName, prefs->textCreatorName);
			DlgSetCheck(dlg, kDirCheckBox, prefs->textDefaultDir);
	  		DlgSetUserItem(dlg, kDirBorder, DlgGrayBorderItem);			
			err = VolNameToVRefNum (prefs->textVolName, &cpb.dirInfo.ioVRefNum);
	  		if (err == noErr) {
				cpb.dirInfo.ioNamePtr = name;
				cpb.dirInfo.ioFDirIndex = -1;
				cpb.dirInfo.ioDrDirID = prefs->textDirID;
				err = PBGetCatInfo(&cpb, false);
			}
			if (err != noErr) *name = 0;
			DlgSetPString(dlg, kDirName, name);
			DlgEnableItem(dlg, kDirButton, prefs->textDefaultDir);
				
			break;
			
		case kTerm:
		
			break;
			
		case kAppButton:
		
			MyStandardGetFile(nil, 1, (OSType*)"APPL", &reply);
			if (!reply.sfGood) break;
			FSpGetFInfo(&reply.sfFile, &fInfo);
			prefs->textCreator = fInfo.fdCreator;
			pstrcpy(prefs->textCreatorName, reply.sfFile.name);
			DlgSetPString(dlg, kAppName, prefs->textCreatorName);
			break;
			
		case kDirCheckBox:
		
			DlgToggleCheck(dlg, item);
			prefs->textDefaultDir = DlgGetCheck(dlg, item);
			DlgEnableItem(dlg, kDirButton, prefs->textDefaultDir);
			break;
			
		case kDirButton:
		
			MyStandardGetDirectory(&reply, &dirID);
			if (!reply.sfGood) break;
			prefs->textDirID = dirID;
			pb.volumeParam.ioNamePtr = prefs->textVolName;
			pb.volumeParam.ioVRefNum = reply.sfFile.vRefNum;
			pb.volumeParam.ioVolIndex = 0;
			PBHGetVInfo(&pb, false);
			DlgSetPString(dlg, kDirName, reply.sfFile.name);
			break;
			
	}
}



/*----------------------------------------------------------------------------
	Auth

	Handles the authorization preferences dialog.
	
	Entry:	prefs = pointer to new prefs.
			dlg = pointer to dialog.
			item = item number hit.
----------------------------------------------------------------------------*/

static void Auth (TPrefRec *prefs, DialogPtr dlg, short item)
{
	switch (item) {
	
		case kInit:
		
			break;
			
		case kTerm:
			
			break;
			
	}
}



/*----------------------------------------------------------------------------
	DoPrefsDialog

	Presents the Preferences dialog.
----------------------------------------------------------------------------*/


void DoPrefsDialog (void)
{
	typedef void (*prefsFuncPtr) (TPrefRec *prefs, DialogPtr dlg, short item);
	
	static prefsFuncPtr prefsFuncPtrs[] = {
		GeneralOptions,
		Server,
		ServerOptions,
		Personal,
		Signature,
		Remote,
		Font,
		Saved,
		Auth,
	};

	static short curTopic = kGeneralOptions;
	
	short newTopic;
	prefsFuncPtr theFunc;
	TPrefRec *prefs;
	DialogPtr dlg;
	short item;
	CStr255 msg;
	Boolean listFontChanged, textFontChanged;
	Handle theDITL;
	
	prefs = (TPrefRec*)MyNewPtr(sizeof(TPrefRec));
	*prefs = gPrefs;
	
	dlg = MyGetNewDialog(kPrefsDlg);
	theDITL = GetResource('DITL', kPrefsDlg + curTopic);
	MyAppendDITL(dlg, theDITL, overlayDITL);
	ReleaseResource(theDITL);
	DlgSetCtlValue(dlg, kTopicPopup, curTopic);
	theFunc = prefsFuncPtrs[curTopic - 1];
	(*theFunc)(prefs, dlg, kInit);
	ShowWindow(dlg);
	
	while (true) {
		MyModalDialog(DialogFilter, &item, true, curTopic != kSignature);
		if (item == ok || item == cancel) {
			(*theFunc)(prefs, dlg, kTerm);
			break;
		} else if (item == kTopicPopup) {
			newTopic = DlgGetCtlValue(dlg, kTopicPopup);
			if (newTopic != curTopic) {
				(*theFunc)(prefs, dlg, kTerm);
				curTopic = newTopic;
				MyShortenDITL(dlg, CountDITL(dlg) - 3);
				theDITL = GetResource('DITL', kPrefsDlg + curTopic);
				MyAppendDITL(dlg, theDITL, overlayDITL);
				ReleaseResource(theDITL);
				theFunc = prefsFuncPtrs[curTopic - 1];
				(*theFunc)(prefs, dlg, kInit);
			}
		} else {
			(*theFunc)(prefs, dlg, item);
		}
	}
	
	MyDisposDialog(dlg);
	
	if (item == ok) {
		if (prefs->logActionsToFile && !gPrefs.logActionsToFile) {
			OpenLogFile();
		} else if (!prefs->logActionsToFile && gPrefs.logActionsToFile) {
			CloseLogFile();
		}
		if (!EqualString(prefs->newsServerName, gPrefs.newsServerName, false, true)) {
			strcpy(msg, "You must quit and restart the program ");
			strcat(msg, "for the news server change to take effect.");
			ErrorMessage(msg);
		}
		listFontChanged = prefs->listSize != gPrefs.listSize ||
			!EqualString(prefs->listFont, gPrefs.listFont, false, true);
		textFontChanged = prefs->textSize != gPrefs.textSize ||
			!EqualString(prefs->textFont, gPrefs.textFont, false, true);
		gPrefs = *prefs;
		if (listFontChanged) {
			gPrefs.maxGroupNameWidth = 0;
			FontWasChanged(true);
		}
		if (textFontChanged) {
			FontWasChanged(false);
		}
	}
	MyDisposPtr((Ptr)prefs);
}
