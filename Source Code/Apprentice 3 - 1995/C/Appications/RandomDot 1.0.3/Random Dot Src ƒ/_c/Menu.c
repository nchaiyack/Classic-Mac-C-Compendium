/* Menu.c - code for handling the menubar in RandomDot
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
#ifndef __IMAGECOMPRESSION__
#include <ImageCompression.h>
#endif	/* __IMAGECOMPRESSION__ */

#ifndef __GESTALTEQU__
#include <GestaltEqu.h>
#endif	/* __GESTALTEQU__ */

#ifndef __BALLOONS__
#include <Balloons.h>
#endif	/* __BALLOONS__ */

#include "RandomDotMain.h"
#include "RandomDotRes.h"

#include "RandomDotWin.h"
#include "RandomDotWinCommands.h"
#include "RandomDotPrint.h"
#include "Error.h"
#include "Menu.h"
#include "Help.h"
#include "Progress.h"
#include "Utils.h"
#include "pgm.h"

static Integer myHelpItem = -2;


/* GetAboutWin - 
 */
static WindowPtr GetAboutWin(void){
	WindowPtr	win;

	win = GetNewDialog(rAbout, NIL, (WindowPtr) -1L);
	ShowWindow(win);
	SelectWindow(win);
	return win;
}

/* DoAppleMenu - 
 */
static void DoAppleMenu(Integer item){
	Str255		s;
	WindowPtr	win;

	if(1 == item){
		win = GetAboutWin();
		SetDialogDefaultItem(win, kOK);
		ModalDialog(NIL, &item);
		DisposeWindow(win);
		if(kOK != item){
			DoHelp();
		}
	}else{
		GetItem(GetMHandle(kAppleMenu), item, s);
		OpenDeskAcc(s);
	}
}

/* OpenFileFilterProc - return FALSE if we should show the file.
	i.e., we show PICT files and PGM files
 */
static pascal Boolean OpenFileFilterProc(ParmBlkPtr io){
	return NOT ('PICT' == io->fileParam.ioFlFndrInfo.fdType ||
		IsPGMFile(io));
}


/* DoOpen - 
 */
void DoOpen(void){
	LongInt				attr;
	StandardFileReply	reply;
	static	OSType	applType[] = { 'PICT' };
	static	FileFilterUPP	fileFilter = NIL;

	if(NIL == fileFilter){
		fileFilter = NewFileFilterProc(OpenFileFilterProc);
	}
	if(noErr == Gestalt(gestaltCompressionMgr, &attr) && NIL != StandardGetFilePreview){
		StandardGetFilePreview(fileFilter, 0, applType, &reply);
	}else{
		StandardGetFile(fileFilter, 0, applType, &reply);
	}
	if(reply.sfGood){
		TellError(RandomDotOpen(&reply.sfFile, reply.sfScript));
	}
}

void DoCloseWin(WindowPtr win){
	if(NIL == win){
		/* empty */
	}else if(userKind == ((WindowPeek) win)->windowKind){
		RandomDotDisposeWindow(win);
	}else{
		HideWindow(win);
	}
}

void DoClose(void){
	WindowPtr	win;

	if(NIL != (win = FrontWindow())){
		DoCloseWin(win);
	}
}


/* DoQuit - do a quit command
 */
void DoQuit(void){
	RecordProgress();
	ExitToShell();
}

/* DoFileMenu - 
 */
static void DoFileMenu(Integer item){
	switch(item){
	case kOpenI:		DoOpen();		break;
	case kCloseI:		DoClose();		break;
	case kSaveI:		DoRandomDotSave();		break;
	case kSaveAsI:		DoRandomDotSaveAs();	break;
	case kPageSetupI:	DoRandomDotPageSetup();	break;
	case kPrintI:		DoRandomDotPrint();		break;
	case kQuitI:		DoQuit();		break;
	}
}

/* DoEditMenu - 
 */
static void DoEditMenu(Integer item){
	if(NOT SystemEdit(item - 1)){
		switch(item){
		case kUndoI:	break;
		case kCutI:		DoRandomDotCut();	break;
		case kCopyI:	DoRandomDotCopy();	break;
		case kPasteI:	DoRandomDotPaste();	break;
		case kClearI:	break;
			break;
		}
	}
}

/* DoRandomDotMenu - 
 */
static void DoRandomDotMenu(Integer item){
	switch(item){
	case kGrayI:		DoRandomDotGrayState();			break;
	case kBWStereoI:	DoRandomDotBWStereoState();		break;
	case kGrayStereoI:	DoRandomDotGrayStereoState();	break;
	case kShimmerI:		DoRandomDotShimmer();			break;
	}
}

/* InitHelpItem - augment the help menu with our item, for system 7 only.
 */
void InitHelpItem(void){
	MenuHandle	mh;
	Str255		sName;

	if(noErr == HMGetHelpMenuHandle(&mh)){
		GetIndString(sName, kMainStrs, kOurHelpS);
		AppendMenu(mh, sName);
		myHelpItem = CountMItems(mh);
	}
}

/* DoHelpMenu - do items added to help menu.
 */
static void DoHelpMenu(Integer item){
	if(item == myHelpItem){
		DoHelp();
	}
}

/* GoMenu - do one menu command
 */
void GoMenu(LongInt selector){
	Integer	theLow;

	theLow = LoWord(selector);
	switch(HiWord(selector)){
	case kAppleMenu:	DoAppleMenu(theLow);		break;
	case kFileMenu:		DoFileMenu(theLow);			break;
	case kEditMenu:		DoEditMenu(theLow);			break;
	case kRandomDotMenu:DoRandomDotMenu(theLow);	break;
	case kHMHelpMenuID:	DoHelpMenu(theLow);			break;
	}
	HiliteMenu(0);
}
