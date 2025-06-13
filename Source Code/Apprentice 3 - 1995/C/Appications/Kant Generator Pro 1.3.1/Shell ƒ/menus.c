#include "menus.h"
#include "help.h"
#include "environment.h"
#include "error.h"
#include "file interface.h"
#include "resource layer.h"
#include "printing layer.h"
#include "text layer.h"
#include "key layer.h"
#include "memory layer.h"
#include "music layer.h"
#include "speech layer.h"
#include "graphics dispatch.h"
#include "windows menu.h"
#include "window layer.h"
#include "program globals.h"
#include "program init.h"
#include "kant load-save.h"
#include "kant parser dispatch.h"
#include "kant main window.h"
#include "kant build window.h"
#include "kant build files.h"
#include "kant build dialogs.h"
#include "kant build dispatch.h"
#include "kant build lists.h"
#include "kant build print.h"
#include "kant search.h"
#include "kant event class dispatch.h"
#include <Icons.h>

static	MenuHandle		gAppleMenu=0L;
static	MenuHandle		gFileMenu=0L;
static	MenuHandle		gEditMenu=0L;
static	MenuHandle		gOptionsMenu=0L;
static	MenuHandle		gSearchMenu=0L;
static	MenuHandle		gInsertMenu=0L;
static	MenuHandle		gBuildMenu=0L;
		MenuHandle		gWindowsMenu=0L;
static	MenuHandle		gSpeedMenu=0L;
static	MenuHandle		gMusicMenu=0L;
static	MenuHandle		gModulesMenu=0L;
		MenuHandle		gSpeechMenu=0L;

static	Handle			gFileIconHandle=0L;
static	Handle			gEditIconHandle=0L;
static	Handle			gOptionsIconHandle=0L;
static	Handle			gSearchIconHandle=0L;
static	Handle			gWindowsIconHandle=0L;
static	Handle			gInsertIconHandle=0L;
static	Handle			gBuildIconHandle=0L;

#define NUM_MODULES		(CountMItems(gModulesMenu)-modules_first+1)

enum
{
	speedMenu = 200, musicMenu, modulesMenu, speechMenu,
	
	appleMenu = 400, fileMenu, editMenu, optionsMenu, searchMenu, insertMenu, buildMenu, windowsMenu,
	
	aboutItem = 1, otherProductsItem, helpPointerItem,
	
	newItem = 1, openItem, file_unused1, closeItem, saveItem, saveAsItem, file_unused2,
		pageSetupItem, printItem, file_unused3, quitItem,
	
	undoItem = 1, edit_unused0, cutItem, copyItem, pasteItem, clearItem, edit_unused1, selectAllItem,
	
	resolveItem = 1, alwaysResolveItem, showStagesItem, showInterestingItem, option_unused1,
		modulesMenuPtr, speedMenuPtr, musicMenuPtr, speechMenuPtr, option_unused4,
		dynamicScrollItem, iconifyItem,
	
	findItem = 1, findAgainItem, search_unused1, replaceItem, replaceAndFindItem, replaceAllItem,
		search_unused2, enterSearchItem, enterReplaceItem,
	
	sectionItem = 1, paragraphItem, sentenceItem, mainClauseItem, introClauseItem,
		throwawayClauseItem, proofItem, throwawaySentenceItem,
	
	build_new = 1, build_open, build_edit_current, build_unused1, build_new_ref, build_new_instant,
		build_unused2, build_edit_ref, build_delete_ref, build_mark_interesting,
		build_unused3, build_show_message, build_show_toolbar,

	iZoom = 1, iSendToBack,
	
	modules_default = 1, modules_other, modules_unused, modules_first,
	
	delay0Item = 1, delay6Item, delay12Item, delay30Item, delay60Item,
	
	musicAlwaysItem = 1, musicOnlyItem, musicNeverItem,
	
	iSpeakAll = 1, iSpeakSelection
};

enum
{
	kFileIconID=500, kEditIconID, kOptionsIconID, kSearchIconID, kWindowsIconID,
		kInsertIconID, kBuildIconID
};

/*-----------------------------------------------------------------------------------*/
/* internal stuff for menus.c                                                        */

static	void HandleAppleMenu(short menuItem);
static	void HandleFileMenu(short menuItem);
static	void HandleEditMenu(short menuItem, Boolean alreadyPassedThrough);
static	void HandleOptionsMenu(short menuItem);
static	void HandleSearchMenu(short menuItem);
static	void HandleInsertMenu(short menuItem);
static	void HandleBuildMenu(short menuItem);
static	void HandleWindowsMenu(short menuItem);
static	void HandleSpeedMenu(short menuItem);
static	void HandleMusicMenu(short menuItem);
static	void HandleModulesMenu(short menuItem);
static	void HandleSpeechMenu(short menuItem);
static	void EDItem(MenuHandle theMenu, short theItem, Boolean theCondition);
static	void IconifyOneMenu(MenuHandle *theMenuHandlePtr, Handle iconHandle, short menuID,
	short beforeMenuID, Boolean useIcons);

Boolean InitTheMenus(void)
{
	Handle			MBARHandle;
	FSSpec			theFS;
	
	if ((MBARHandle=GetNewMBar(400))==0L)		/* sez which menus are in menu bar. */
		return FALSE;
	SetMenuBar(MBARHandle);						/* set this to be THE menu bar to use. */
	
	if ((gAppleMenu=GetMHandle(appleMenu))==0L)	/* GetNewMBar also got menu handles of */
		return FALSE;
	if ((gFileMenu=GetMHandle(fileMenu))==0L)	/* every menu it includes, so just */
		return FALSE;
	if ((gEditMenu=GetMHandle(editMenu))==0L)	/* grab these handles and assign them */
		return FALSE;
	if ((gOptionsMenu=GetMHandle(optionsMenu))==0L)
		return FALSE;
	if ((gSearchMenu=GetMHandle(searchMenu))==0L)
		return FALSE;
	if ((gInsertMenu=GetMHandle(insertMenu))==0L)
		return FALSE;
	if ((gBuildMenu=GetMHandle(buildMenu))==0L)
		return FALSE;
	if ((gWindowsMenu=GetMHandle(windowsMenu))==0L)
		return FALSE;
	
	if ((gSpeedMenu=GetMenu(speedMenu))==0L)
		return FALSE;
	if ((gMusicMenu=GetMenu(musicMenu))==0L)
		return FALSE;
	if ((gModulesMenu=GetMenu(modulesMenu))==0L)
		return FALSE;
	if ((gSpeechMenu=GetMenu(speechMenu))==0L)
		return FALSE;
	
	InsertMenu(gSpeedMenu, -1);
	InsertMenu(gMusicMenu, -1);
	InsertMenu(gModulesMenu, -1);
	InsertMenu(gSpeechMenu, -1);
	
	AddResMenu(gAppleMenu, 'DRVR');				/* adds control panels to apple menu */
	
	if (GetIconSuite(&gFileIconHandle, kFileIconID, svAllSmallData)!=noErr)
		return FALSE;
	if (GetIconSuite(&gEditIconHandle, kEditIconID, svAllSmallData)!=noErr)
		return FALSE;
	if (GetIconSuite(&gOptionsIconHandle, kOptionsIconID, svAllSmallData)!=noErr)
		return FALSE;
	if (GetIconSuite(&gSearchIconHandle, kSearchIconID, svAllSmallData)!=noErr)
		return FALSE;
	if (GetIconSuite(&gInsertIconHandle, kInsertIconID, svAllSmallData)!=noErr)
		return FALSE;
	if (GetIconSuite(&gBuildIconHandle, kBuildIconID, svAllSmallData)!=noErr)
		return FALSE;
	if (GetIconSuite(&gWindowsIconHandle, kWindowsIconID, svAllSmallData)!=noErr)
		return FALSE;
	
	if (BuildModulesList(gModulesMenu)!=noErr)
	{
		HandleError(kCantFindModulesFolder, FALSE, FALSE);
		theFS.name[0]=0x00;
		UseTheModule(&theFS, TRUE);	/* revert to built-in */
	}
	
	if (!gUseDefault)
		RebuildInsertMenu();
	
	if (gIconifyMenus)
		IconifyMenus(TRUE);
	
	RebuildSpeechMenu(gSpeechMenu);
	
	AdjustMenus();								/* dim/enable/check/mark menus/items */
	DrawMenuBar();								/* draws the actual menu bar */
	
	return TRUE;
}

void ShutDownTheMenus(void)
{
	gAppleMenu=SafeDisposeMenu(gAppleMenu);
	gFileMenu=SafeDisposeMenu(gFileMenu);
	gEditMenu=SafeDisposeMenu(gEditMenu);
	gOptionsMenu=SafeDisposeMenu(gOptionsMenu);
	gSearchMenu=SafeDisposeMenu(gSearchMenu);
	gInsertMenu=SafeDisposeMenu(gInsertMenu);
	gBuildMenu=SafeDisposeMenu(gBuildMenu);
	gWindowsMenu=SafeDisposeMenu(gWindowsMenu);
	gSpeedMenu=SafeDisposeMenu(gSpeedMenu);
	gMusicMenu=SafeDisposeMenu(gMusicMenu);
	gModulesMenu=SafeDisposeMenu(gModulesMenu);
	gSpeechMenu=SafeDisposeMenu(gSpeechMenu);
	
	gFileIconHandle=SafeDisposeIconSuite(gFileIconHandle);
	gEditIconHandle=SafeDisposeIconSuite(gEditIconHandle);
	gOptionsIconHandle=SafeDisposeIconSuite(gOptionsIconHandle);
	gSearchIconHandle=SafeDisposeIconSuite(gSearchIconHandle);
	gBuildIconHandle=SafeDisposeIconSuite(gBuildIconHandle);
	gWindowsIconHandle=SafeDisposeIconSuite(gWindowsIconHandle);
}

void AdjustMenus(void)
{
	short			kind;
	WindowRef		front, frontDoc, mainWindow, buildWindow;
	Boolean			refActive, instantActive, refHighlighted, instantHighlighted, refInteresting;
	Str255			theStr;
	Boolean			gotone;
	short			i, numOpenWindows;
	unsigned char	*otherStr="\pOther...";
	Boolean			a;
	
	front=FrontWindow();
	kind=front ? ((WindowPeek)front)->windowKind : 0;
	frontDoc=GetFrontDocumentWindow();
	mainWindow=GetIndWindowRef(kMainWindow);
	buildWindow=GetIndWindowRef(kBuildWindow);
	refActive=(buildWindow!=0L) && (ReferenceListActiveQQ());
	instantActive=(buildWindow!=0L) && (InstantListActiveQQ());
	refHighlighted=(buildWindow!=0L) && (GetHighlightedReference()>=0);
	instantHighlighted=(buildWindow!=0L) && (GetHighlightedInstant()>=0);
	refInteresting=(buildWindow!=0L) && (HighlightedReferenceInterestingQQ());
	numOpenWindows=GetNumberOfOpenWindows();
	
	CheckItem(gOptionsMenu, alwaysResolveItem, gAlwaysResolve ? TRUE : FALSE);
	CheckItem(gOptionsMenu, dynamicScrollItem, gDynamicScroll ? TRUE : FALSE);
	CheckItem(gOptionsMenu, showInterestingItem, gShowAllRefs ? FALSE : TRUE);
	CheckItem(gOptionsMenu, iconifyItem, gIconifyMenus ? TRUE : FALSE);
	CheckItem(gOptionsMenu, showStagesItem, gFastResolve ? FALSE : TRUE);
	CheckItem(gSpeedMenu, delay0Item, (gSpeedDelay==0));
	CheckItem(gSpeedMenu, delay6Item, (gSpeedDelay==6));
	CheckItem(gSpeedMenu, delay12Item, (gSpeedDelay==12));
	CheckItem(gSpeedMenu, delay30Item, (gSpeedDelay==30));
	CheckItem(gSpeedMenu, delay60Item, (gSpeedDelay==60));
	CheckItem(gMusicMenu, musicAlwaysItem, (gMusicStatus==kMusicAlways));
	CheckItem(gMusicMenu, musicOnlyItem, (gMusicStatus==kMusicGeneratingOnly));
	CheckItem(gMusicMenu, musicNeverItem, (gMusicStatus==kMusicNever));
	
	SetItem(gBuildMenu, build_edit_current, (buildWindow==0L) ? "\pOpen current module" : "\pEdit current module");
	SetItem(gBuildMenu, build_edit_ref, refActive ? "\pEdit reference" : "\pEdit instantiation");
	SetItem(gBuildMenu, build_delete_ref, refActive ? "\pDelete reference" : "\pDelete instantiation");
	SetItem(gBuildMenu, build_mark_interesting, refInteresting ?
		"\pMark reference “uninteresting”" : "\pMark reference “interesting”");
	SetItem(gBuildMenu, build_show_toolbar, gShowToolbar ? "\pHide tool bar" : "\pShow tool bar");
	SetItem(gBuildMenu, build_show_message, gShowMessageBox ? "\pHide message box" :
		"\pShow message box");
	
	CheckItem(gModulesMenu, modules_default, gUseDefault);
	CheckItem(gModulesMenu, modules_other, FALSE);
		
	gotone=gUseDefault;
	for (i=0; i<NUM_MODULES; i++)
	{
		if (!gotone)
		{
			GetItem(gModulesMenu, i+modules_first, theStr);
			if (Mymemcompare((Ptr)theStr, (Ptr)gModuleFS.name, theStr[0]+1))
				gotone=TRUE;
			CheckItem(gModulesMenu, i+modules_first, gotone);
		}
		else CheckItem(gModulesMenu, i+modules_first, FALSE);
	}
	
	if (!gotone)
	{
		CheckItem(gModulesMenu, modules_other, TRUE);
		Mymemcpy((Ptr)theStr, (Ptr)otherStr, otherStr[0]+1);
		AppendStr255(theStr, "\p (");
		AppendStr255(theStr, gModuleFS.name);
		AppendStr255(theStr, "\p)");
		SetItem(gModulesMenu, modules_other, theStr);
	}
	else SetItem(gModulesMenu, modules_other, "\pOther...");
	
	EDItem(gAppleMenu, aboutItem, !gInProgress);
	EDItem(gAppleMenu, otherProductsItem, !gInProgress);
	EDItem(gAppleMenu, helpPointerItem, !gInProgress);
	EDItem(gFileMenu, 0, !gInProgress);
	EDItem(gEditMenu, 0, !gInProgress);
	EDItem(gOptionsMenu, 0, !gInProgress);
	EDItem(gSearchMenu, 0, !gInProgress);
	EDItem(gInsertMenu, 0, !gInProgress);
	EDItem(gBuildMenu, 0, !gInProgress);
	EDItem(gWindowsMenu, 0, !gInProgress);
	
	if (gInProgress)
		return;
	
	EDItem(gFileMenu, newItem, TRUE);
	EDItem(gFileMenu, openItem, TRUE);
	EDItem(gFileMenu, closeItem, front!=0L);
	EDItem(gFileMenu, saveItem, (mainWindow!=0L) && (WindowHasLayer(mainWindow)) &&
		(WindowIsModifiedQQ(mainWindow)));
	EDItem(gFileMenu, saveAsItem, mainWindow!=0L);
	EDItem(gFileMenu, pageSetupItem, TRUE);
	EDItem(gFileMenu, printItem, (frontDoc!=0L) && (WindowHasLayer(frontDoc)) &&
		(WindowIsPrintableQQ(frontDoc)));
	EDItem(gFileMenu, quitItem, TRUE);
	
	EDItem(gEditMenu, undoItem, kind<0);
	EDItem(gEditMenu, cutItem, (frontDoc!=0L) && WindowIsEditableQQ(frontDoc) && (AnyHighlightedQQ(frontDoc)));
	EDItem(gEditMenu, copyItem, (frontDoc!=0L) && WindowIsEditableQQ(frontDoc) && (AnyHighlightedQQ(frontDoc)));
	EDItem(gEditMenu, pasteItem, (frontDoc!=0L) && WindowIsEditableQQ(frontDoc) && (AnyTextInScrapQQ()));
	EDItem(gEditMenu, clearItem, (frontDoc!=0L) && WindowIsEditableQQ(frontDoc) && (AnyHighlightedQQ(frontDoc)));
	EDItem(gEditMenu, selectAllItem, (frontDoc!=0L) && WindowIsEditableQQ(frontDoc) && (AnyTextQQ(frontDoc)));
	
	EDItem(gOptionsMenu, resolveItem, (mainWindow!=0L));
	EDItem(gOptionsMenu, alwaysResolveItem, TRUE);
	EDItem(gOptionsMenu, showInterestingItem, TRUE);
	EDItem(gOptionsMenu, modulesMenuPtr, TRUE);
	EDItem(gOptionsMenu, speedMenuPtr, TRUE);
	EDItem(gOptionsMenu, musicMenuPtr, gMusicAvailable);
	EDItem(gOptionsMenu, speechMenuPtr, SpeechIsAvailableQQ());
	
	a=(frontDoc==mainWindow);
	EDItem(gSearchMenu, findItem, (AnyTextQQ(frontDoc))&&a);
	EDItem(gSearchMenu, findAgainItem, (AnyTextQQ(frontDoc) && AnythingToFindQQ())&&a);
	EDItem(gSearchMenu, replaceItem, (AnyHighlightedQQ(frontDoc) && AnythingToReplaceQQ())&&a);
	EDItem(gSearchMenu, replaceAndFindItem, (AnyHighlightedQQ(frontDoc) && AnythingToReplaceQQ())&&a);
	EDItem(gSearchMenu, replaceAllItem, (AnyTextQQ(frontDoc) && AnythingToReplaceQQ())&&a);
	EDItem(gSearchMenu, enterSearchItem, (AnyHighlightedQQ(frontDoc))&&a);
	EDItem(gSearchMenu, enterReplaceItem, (AnyHighlightedQQ(frontDoc))&&a);
	
	EDItem(gBuildMenu, build_new, buildWindow==0L);
	EDItem(gBuildMenu, build_open, buildWindow==0L);
	EDItem(gBuildMenu, build_edit_current, !gUseDefault);
	EDItem(gBuildMenu, build_new_ref, buildWindow!=0L);
	EDItem(gBuildMenu, build_new_instant, refHighlighted);
	EDItem(gBuildMenu, build_edit_ref, (refActive && refHighlighted) || (instantActive && instantHighlighted));
	EDItem(gBuildMenu, build_delete_ref, (refActive && refHighlighted) || (instantActive && instantHighlighted));
	EDItem(gBuildMenu, build_mark_interesting, (refActive && refHighlighted));
	EDItem(gBuildMenu, build_show_toolbar, buildWindow!=0L);
	EDItem(gBuildMenu, build_show_message, (buildWindow!=0L) && (gShowToolbar));

	EDItem(gWindowsMenu, iZoom, (frontDoc!=0L) && (WindowIsZoomableQQ(frontDoc)));
	EDItem(gWindowsMenu, iSendToBack, numOpenWindows>1);
	AdjustWindowsMenu(frontDoc);
	
	EDItem(gSpeechMenu, iSpeakAll, (AnyTextQQ(frontDoc)) && a);
	EDItem(gSpeechMenu, iSpeakSelection, (AnyHighlightedQQ(frontDoc)) && a);
	AdjustSpeechMenu(gSpeechMenu);
}

void HandleMenu(long mSelect)
{
	short			menuID = HiWord(mSelect);
	short			menuItem = LoWord(mSelect);
	
	switch (menuID)
	{
		case appleMenu:
			HandleAppleMenu(menuItem);
			break;
		case fileMenu:
			HandleFileMenu(menuItem);
			break;	
		case editMenu:
			HandleEditMenu(menuItem, FALSE);
			break;
		case optionsMenu:
			HandleOptionsMenu(menuItem);
			break;
		case searchMenu:
			HandleSearchMenu(menuItem);
			break;
		case insertMenu:
			HandleInsertMenu(menuItem);
			break;
		case buildMenu:
			HandleBuildMenu(menuItem);
			break;
		case windowsMenu:
			HandleWindowsMenu(menuItem);
			break;
		case speedMenu:
			HandleSpeedMenu(menuItem);
			break;
		case musicMenu:
			HandleMusicMenu(menuItem);
			break;
		case modulesMenu:
			HandleModulesMenu(menuItem);
			break;
		case speechMenu:
			HandleSpeechMenu(menuItem);
			break;
	}
}

void DoTheCloseThing(WindowPeek theWindow)
/* a standard close procedure, called when "close" is chosen from File menu or when
   a window is closed through its close box */
{
	short			kind;
	
	if (theWindow==0L)
		return;
	
	kind = theWindow ? theWindow->windowKind : 0;
	if (kind<0)		/* DA window or other system window */
		CloseDeskAcc(kind);
	else
	{
		if (WindowHasLayer((WindowRef)theWindow))
		{
			if (OptionKeyWasDown())
			{
				while ((theWindow=(WindowPeek)GetFrontDocumentWindow())!=0L)
					CloseTheWindow((WindowRef)theWindow);
			}
			else
			{
				CloseTheWindow((WindowRef)theWindow);
			}
		}
		else
		{
			DisposeWindow((WindowRef)theWindow);
		}
		
		AdjustMenus();
	}
}

static	void HandleAppleMenu(short menuItem)
{
	GrafPtr		savePort;
	Str255		name;
	
	switch (menuItem)
	{
		case aboutItem:
			if (!IndWindowExistsQQ(kAboutWindow))
				OpenTheIndWindow(kAboutWindow, kOpenOldIfPossible);
			else
				MySelectWindow(GetIndWindowRef(kAboutWindow));
			break;
		case otherProductsItem:
			if (!IndWindowExistsQQ(kOtherProductsWindow))
				OpenTheIndWindow(kOtherProductsWindow, kOpenOldIfPossible);
			else
				MySelectWindow(GetIndWindowRef(kOtherProductsWindow));
			break;
		case helpPointerItem:
			if (!IndWindowExistsQQ(kHelpWindow))
				OpenTheIndWindow(kHelpWindow, kOpenOldIfPossible);
			else
				MySelectWindow(GetIndWindowRef(kHelpWindow));
			break;
		default:
			if (menuItem > helpPointerItem+1)
			{
				GetPort(&savePort);
				GetItem(gAppleMenu, menuItem, name);
				OpenDeskAcc(name);
				SetPort(savePort);
			}
			break;
	}
}

static	void HandleFileMenu(short menuItem)
{
	WindowRef		theWindow;
	TEHandle		hTE;
	FSSpec			fs;
	
	switch (menuItem)
	{
		case newItem:
			KGPNew();
			break;
		case openItem:
			LoadSaveDispatch(&fs, TRUE, FALSE);
			break;
		case closeItem:
			if ((theWindow=GetFrontDocumentWindow())!=0L)
				DoTheCloseThing((WindowPeek)theWindow);
			else
				DoTheCloseThing((WindowPeek)FrontWindow());
			break;
		case saveItem:
			theWindow=GetIndWindowRef(kMainWindow);
			fs=GetWindowFS(theWindow);
			LoadSaveDispatch(&fs, FALSE, TRUE);
			break;
		case saveAsItem:
			fs.name[0]=0x00;
			LoadSaveDispatch(&fs, FALSE, FALSE);
			break;
		case pageSetupItem:
			DoThePageSetup();
			break;
		case printItem:
			if ((theWindow=GetFrontDocumentWindow())==0L)
				break;
			if ((WindowHasLayer(theWindow)) && (WindowIsPrintableQQ(theWindow)))
			{
				if (theWindow==GetIndWindowRef(kBuildWindow))
					PrintTheModule();
				else
				{
					hTE=GetWindowTE(theWindow);
					if (hTE!=0L)
					{
						PrintText(hTE);
					}
				}
			}
			else SysBeep(7);
			break;
		case quitItem:
			gDone=ShutDownTheProgram();
			break;
	}
}

static	void HandleEditMenu(short menuItem, Boolean alreadyPassedThrough)
{
	WindowRef		theWindow;
	DispatchError	resultCode;
	
	if (!alreadyPassedThrough)
		theWindow=FrontWindow();
	else
	{
		theWindow=GetFrontDocumentWindow();
	}
	
	if (theWindow!=0L)
	{
		switch (menuItem)
		{
			case undoItem:		resultCode=UndoDispatch(theWindow);			break;
			case cutItem:		resultCode=CutDispatch(theWindow);			break;
			case copyItem:		resultCode=CopyDispatch(theWindow);			break;
			case pasteItem:		resultCode=PasteDispatch(theWindow);		break;
			case clearItem:		resultCode=ClearDispatch(theWindow);		break;
			case selectAllItem:	resultCode=SelectAllDispatch(theWindow);	break;
			default:			resultCode=kSuccess;						break;
		}
		
		if ((resultCode==kPassThrough) && (!alreadyPassedThrough))
			HandleEditMenu(menuItem, TRUE);
	}
	else SystemEdit(menuItem-1);
}

static	void HandleOptionsMenu(short menuItem)
{
	switch (menuItem)
	{
		case resolveItem:
			KGPResolve();
			break;
		case alwaysResolveItem:
			KGPSetOption(kOptionAlwaysResolve, gAlwaysResolve ? kSettingOff : kSettingOn);
			break;
		case showInterestingItem:
			KGPSetOption(kOptionListInteresting, gShowAllRefs ? kSettingOff : kSettingOn);
			break;
		case showStagesItem:
			KGPSetOption(kOptionShowStages, gFastResolve ? kSettingOn : kSettingOff);
			break;
		case dynamicScrollItem:
			KGPSetOption(kOptionDynamicScrolling, gDynamicScroll ? kSettingOff : kSettingOn);
			break;
		case iconifyItem:
			KGPSetOption(kOptionMenuBarIcons, gIconifyMenus ? kSettingOff : kSettingOn);
			break;
	}
}

static	void HandleSearchMenu(short menuItem)
{
	if (GetFrontDocumentWindow()!=GetIndWindowRef(kMainWindow))
		return;
	
	switch (menuItem)
	{
		case findItem:
			DoFindDialog();
			break;
		case replaceAndFindItem:
			DoReplace();
			/* no break intentionally */
		case findAgainItem:
			if (!DoFindAgain())
				SysBeep(7);
			break;
		case replaceItem:
			DoReplace();
			break;
		case replaceAllItem:
			DoReplaceAll();
			break;
		case enterSearchItem:
			DoEnterString(TRUE);
			break;
		case enterReplaceItem:
			DoEnterString(FALSE);
			break;
	}
}

static	void HandleInsertMenu(short menuItem)
{
	Str255			theName, fullName;
	
	if (gUseDefault)
	{
		switch (menuItem)
		{
			case sectionItem:
				KGPInsert("\p&section");
				break;
			case paragraphItem:
				KGPInsert("\p&paragraph");
				break;
			case sentenceItem:
				KGPInsert("\p&sentence");
				break;
			case mainClauseItem:
				KGPInsert("\p&main-clause");
				break;
			case introClauseItem:
				KGPInsert("\p&intro-clause");
				break;
			case throwawayClauseItem:
				KGPInsert("\p&throwaway-clause");
				break;
			case proofItem:
				KGPInsert("\p&proof");
				break;
			case throwawaySentenceItem:
				KGPInsert("\p&throwaway-sentence");
				break;
		}
	}
	else
	{
		GetItem(gInsertMenu, menuItem, theName);
		fullName[0]=0x01;
		fullName[1]='&';
		AppendStr255(fullName, theName);
		KGPInsert(fullName);
	}
}

static	void HandleBuildMenu(short menuItem)
{
	FSSpec			theFS;
	WindowRef		buildWindow;
	OSErr			oe;
	Boolean			needToUpdate;
	
	buildWindow=GetIndWindowRef(kBuildWindow);
	if ((buildWindow!=0L) && (GetFrontDocumentWindow()!=buildWindow))
	{
		MySelectWindow(buildWindow);
		SetCursor(&qd.arrow);
	}
	
	needToUpdate=TRUE;
	if (buildWindow!=0L)
		RememberBuildButtonState(buildWindow);
	
	switch (menuItem)
	{
		case build_new:
			oe=DoNewModule();
			if ((oe!=noErr) && (oe!=-1))	/* -1 = user cancel */
				HandleError(kCantCreateNewModule, FALSE, FALSE);
			needToUpdate=FALSE;
			break;
		case build_open:
			if (buildWindow==0L)
			{
				oe=OpenTheModule(&theFS, FALSE, FALSE);
				if ((oe!=noErr) && (oe!=-1))
				{
					HandleError(kCantOpenModule, FALSE, FALSE);
				}
			}
			needToUpdate=FALSE;
			break;
		case build_edit_current:
			if (buildWindow!=0L)
				CloseTheWindow(buildWindow);
			oe=OpenTheModule(&gModuleFS, FALSE, TRUE);
			if ((oe!=noErr) && (oe!=-1))
			{
				HandleError(kCantOpenModule, FALSE, FALSE);
			}
			needToUpdate=FALSE;
			break;
		case build_new_ref:
			DoNewDispatch(buildWindow, TRUE);
			break;
		case build_new_instant:
			DoNewDispatch(buildWindow, FALSE);
			break;
		case build_edit_ref:
			DoEditDispatch(buildWindow);
			break;
		case build_delete_ref:
			DoDeleteDispatch(buildWindow);
			break;
		case build_mark_interesting:
			DoMarkInterestingDispatch(buildWindow);
			break;
		case build_show_toolbar:
			DoShowToolbarDispatch(buildWindow);
			break;
		case build_show_message:
			DoShowMessageDispatch(buildWindow);
			break;
	}
	
	if ((buildWindow!=0L) && (needToUpdate))
		UpdateBuildButtons(buildWindow, TRUE);
}

static	void HandleWindowsMenu(short menuItem)
{
	switch (menuItem)
	{
		case iZoom:
			MyZoomWindow(GetFrontDocumentWindow(), -1);
			break;
		case iSendToBack:
			MySendBehind(GetFrontDocumentWindow(), 0L);
			break;
		default:
			SelectWindowsMenuItem(menuItem);
			break;
	}
}

static	void HandleSpeedMenu(short menuItem)
{
	switch (menuItem)
	{
		case delay0Item:
			KGPSpeed(kSpeedNoDelay);
			break;
		case delay6Item:
			KGPSpeed(kSpeedOneTenthSecondDelay);
			break;
		case delay12Item:
			KGPSpeed(kSpeedOneFifthSecondDelay);
			break;
		case delay30Item:
			KGPSpeed(kSpeedOneHalfSecondDelay);
			break;
		case delay60Item:
			KGPSpeed(kSpeedOneSecondDelay);
			break;
	}
}

static	void HandleMusicMenu(short menuItem)
{
	switch (menuItem)
	{
		case musicNeverItem:
			KGPMusic(kMusicOptionNever);
			break;
		case musicOnlyItem:
			KGPMusic(kMusicOptionOnly);
			break;
		case musicAlwaysItem:
			KGPMusic(kMusicOptionAlways);
			break;
	}
}

static	void HandleModulesMenu(short menuItem)
{
	FSSpec			theFS;
	Str255			theName;
	
	switch (menuItem)
	{
		case modules_default:
			if (!gUseDefault)
			{
				KGPUseModule("\p", FALSE);
			}
			break;
		case modules_other:
			if (GetSourceFile(&theFS, BUILD_TYPE))
			{
				UseTheModule(&theFS, FALSE);
			}
			break;
		case modules_unused:
			break;
		default:
			if (NUM_MODULES>0)
			{
				GetItem(gModulesMenu, menuItem, theName);
				KGPUseModule(theName, FALSE);
			}
			break;
	}
}

static	void HandleSpeechMenu(short menuItem)
{
	switch (menuItem)
	{
		case iSpeakAll:
			KGPSpeech(TRUE);
			break;
		case iSpeakSelection:
			KGPSpeech(FALSE);
			break;
		default:
			SetCurrentVoiceFromMenuItem(menuItem);
			break;
	}
}

static	void EDItem(MenuHandle theMenu, short theItem, Boolean theCondition)
{
	if (theCondition)
		EnableItem(theMenu, theItem);
	else
		DisableItem(theMenu, theItem);
}

void RebuildModulesMenu(void)
{
	FSSpec			theFS;
	
	DeleteMenu(modulesMenu);
	gModulesMenu=SafeDisposeMenu(gModulesMenu);
	gModulesMenu=GetMenu(modulesMenu);
	InsertMenu(gModulesMenu, -1);
	if (BuildModulesList(gModulesMenu)!=noErr)
	{
		HandleError(kCantFindModulesFolder, FALSE, FALSE);
		theFS.name[0]=0x00;
		UseTheModule(&theFS, TRUE);	/* revert to built-in */
	}
}

void RebuildInsertMenu(void)
{
	short			i;
	FSSpec			theFS;
	
	gInsertMenu=SafeDisposeMenu(gInsertMenu);
	gInsertMenu=GetMenu(insertMenu);
	InsertMenu(gInsertMenu, buildMenu);
	if (gIconifyMenus)
		IconifyOneMenu(&gInsertMenu, gInsertIconHandle, insertMenu, buildMenu, TRUE);
	if (!gUseDefault)
	{
		for (i=CountMItems(gInsertMenu); i>0; i--)
			DelMenuItem(gInsertMenu, i);
		if (BuildReferencesList(gInsertMenu, gModuleFS)!=noErr)
		{
			HandleError(kCantBuildReferenceList, FALSE, FALSE);
			theFS.name[0]=0x00;
			UseTheModule(&theFS, TRUE);	/* revert to built-in */
		}
	}
}

void IconifyMenus(Boolean useIcons)
{
	short			menuHeight;
	
	menuHeight=LMGetMBarHeight();
	LMSetMBarHeight(0);
	IconifyOneMenu(&gFileMenu, gFileIconHandle, fileMenu, editMenu, useIcons);
	IconifyOneMenu(&gEditMenu, gEditIconHandle, editMenu, optionsMenu, useIcons);
	IconifyOneMenu(&gOptionsMenu, gOptionsIconHandle, optionsMenu, searchMenu, useIcons);
	IconifyOneMenu(&gSearchMenu, gSearchIconHandle, searchMenu, insertMenu, useIcons);
	RebuildInsertMenu();
	IconifyOneMenu(&gBuildMenu, gBuildIconHandle, buildMenu, windowsMenu, useIcons);
	IconifyOneMenu(&gWindowsMenu, gWindowsIconHandle, windowsMenu, 0, useIcons);
	RebuildWindowsMenu(GetFrontDocumentWindow());
	RebuildModulesMenu();
	LMSetMBarHeight(menuHeight);
	AdjustMenus();
	InvalMenuBar();
}

static	void IconifyOneMenu(MenuHandle *theMenuHandlePtr, Handle iconHandle, short menuID,
	short beforeMenuID, Boolean useIcons)
{
	MenuHandle		tempMenu;
	Str255			menuName;
	Str255			menuItemStr;
	short			i, numItems;
	short			cmdChar, markChar, iconIndex;
	Style			theStyle;
	
	DeleteMenu(menuID);
	*theMenuHandlePtr=SafeDisposeMenu(*theMenuHandlePtr);
	
	if (useIcons)
	{
		Mymemcpy((Ptr)&menuName[2], (Ptr)&iconHandle, 4);
		menuName[0]=0x05;
		menuName[1]=0x01;
		*theMenuHandlePtr=NewMenu(menuID, menuName);
		tempMenu=GetMenu(menuID);
		numItems=CountMItems(tempMenu);
		for (i=1; i<=numItems; i++)
		{
			GetMenuItemText(tempMenu, i, menuItemStr);
			AppendMenu(*theMenuHandlePtr, "\p ");
			SetMenuItemText(*theMenuHandlePtr, i, menuItemStr);
			GetItemCmd(tempMenu, i, &cmdChar);
			SetItemCmd(*theMenuHandlePtr, i, cmdChar);
			GetItemMark(tempMenu, i, &markChar);
			SetItemMark(*theMenuHandlePtr, i, markChar);
			GetItemStyle(tempMenu, i, &theStyle);
			SetItemStyle(*theMenuHandlePtr, i, theStyle);
			GetItemIcon(tempMenu, i, &iconIndex);
			SetItemIcon(*theMenuHandlePtr, i, iconIndex);
		}
		ReleaseResource((Handle)tempMenu);
	}
	else
	{
		*theMenuHandlePtr=GetMenu(menuID);
	}
	
	InsertMenu(*theMenuHandlePtr, beforeMenuID);
	InvalMenuBar();
}
