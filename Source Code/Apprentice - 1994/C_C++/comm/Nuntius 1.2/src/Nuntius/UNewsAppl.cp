// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UNewsAppl.cp

#include "UNewsAppl.h"
#include "UNewsApplCmds.h"
#include "UGroupDoc.h"
#include "UGroupListDoc.h"
#include "UGroupTreeDoc.h"
#include "UPrefsFileMgr.h"
#include "UPreferencesView.h"
#include "UGroupTree.h"
#include "UArticleStatus.h"
#include "TestCode.h"
#include "UThread.h"
#include "UProgressCache.h"
#include "FileTools.h"
#include "Tools.h"
#include "UFatalError.h"
#include "FontTools.h"
#include "UGroupTreeCmds.h"
#include "UPassword.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>

#include <Resources.h>
#include <Packages.h>
#include <ToolUtils.h>
#include <Fonts.h>
#include <Script.h>
#include <OSEvents.h>

#define qFlushVolumes 1
#define qDebugPrefOpen qDebug & 0
#define qDebugWindowState qDebug & 0

TNewsAppl *gNewsAppl;

const long kSaveWindowVersion = 1;

DialogPtr gSplashDiaP = nil;

#pragma segment Main
void ShowSplashScreen()
{
	if (gSplashDiaP)
		return;
	if (gApplication)
		gApplication->InvalidateFocus();
	gSplashDiaP = GetNewDialog(phAboutBoxDlog, nil, WindowPtr(-1));
	if (!gSplashDiaP)
	{
		if (qDebug)
			ProgramBreak("Couldn't load splash screen");
		return;
	}
	SetPort(gSplashDiaP);
	BeginUpdate(gSplashDiaP);
	DrawDialog(gSplashDiaP);
	SetPort(gSplashDiaP);
	short width = qd.thePort->portRect.right;
	CStr255 s("?");
	VersRecHndl vH = VersRecHndl(GetResource('vers', 1));
	if (vH && *vH)
		s = (*vH)->shortVersion;
	s += "  ";
	TextStyle ts;
	MAGetTextStyle(kAboutBoxVersionTS, ts);
	SetPortTextStyle(ts);
	FontInfo theFontInfo;
	MAGetFontInfo(theFontInfo);
	MoveTo(width - StringWidth(s), theFontInfo.ascent + theFontInfo.leading);
	DrawString(s);
	EndUpdate(gSplashDiaP);
}

void RemoveSplashScreen()
{
	if (gSplashDiaP)
	{
		FlushEvents(mDownMask | mUpMask | keyDownMask | keyUpMask | autoKeyMask, 0);
		DisposeDialog(gSplashDiaP);
		gSplashDiaP = nil;
	}
}
#pragma segment MyMisc
//==============================================================
TNewsAppl::TNewsAppl()
{
}

pascal void TNewsAppl::Initialize()
{
	inherited::Initialize();
	fWindowMenuH = nil;
	fGroupListMenuH = nil;
	fFontNameMenuH = nil;
	fFontSizeMenuH = nil;
	fLastFolderModDate = 0;
	fGroupTreeDoc = nil;
	fApplIsRunning = false;
	fFlushVolumes = (GetResource('CMNU', mTest) != nil); // !q_Final;
	fHadGroupList = false;
	fLaunchWithNewDocument = false; // we don't lauch with untitled document
	fIdentifier = kSignature;
	fNeedCheckFolderMenus = true;
}

void TNewsAppl::INewsAppl()
{
	inherited::IApplication('TEXT', kSignature);
	FailInfo fi;
	if (fi.Try())
	{
#if !qFlushVolumes & qDebug
		fprintf(stderr, "Does NOT flush volumes!!!\n");
#endif
		InitUProgressCache();
		InitUThread();
		
		CreateMenus();
		RemoveSplashScreen();

		fi.Success();
	}
	else // fail
	{
		RemoveSplashScreen();
		Free();
		fi.ReSignal();
	}
}

pascal void TNewsAppl::Free()
{
	delete gPrefs;
	inherited::Free();
}

pascal void TNewsAppl::Close()
{
#if qDebug
	fprintf(stderr, "> TNewsAppl::Close()\n");
#endif
	// Kill all threads except main thread
	gCurThread->SetYieldDisable(false);
	while (gAllThreads->GetSize() > 1)
	{
		TThread *thread = (TThread*)gAllThreads->Last();
		thread->Kill();
	}

	// save 'open windows' info 
	if (gPrefs && fGroupTreeDoc)
		SaveWindowState();

	WindowPtr aWMgrWindow;
	// Close group + groupLists documents.  Add a block for failure handling
	// start by closing the gg-document, the frontmost window belongs to
	// after that, close all gg-documents
	{
		while ((aWMgrWindow = MAFrontWindow()) != 0)
		{
			TWindow *window = WMgrToWindow(aWMgrWindow);
			if (!window)
				break;
			TDocument *doc = window->fDocument;
			if (!doc)
				break;
			if (doc == fGroupTreeDoc)
				break;
			doc->CloseAndFree();
		}
		CDocumentIterator iter(this, kIterateBackward);
		for (TDocument *doc = iter.FirstDocument(); iter.More(); doc = iter.NextDocument())
		{
			if (doc != fGroupTreeDoc)
			{
				if (doc->fWindowList && doc->fWindowList->GetSize() >= 1)
				{
					// mac centers the "save changes" dialog over frontmost window,
					// so make sure the documents main window is frontmost
					TWindow *window = (TWindow*)doc->fWindowList->First();
					window->Select();
					window->Update();
				}
				doc->CloseAndFree();
			}
		}
	}

	// Close all of the visible non-floater windows 
	while ((aWMgrWindow = MAFrontWindow()) != 0)
		this->CloseToolboxWindow(aWMgrWindow);

	// Close all of the windows 
	while ((aWMgrWindow = FrontWindow()) != 0)
		this->CloseToolboxWindow(aWMgrWindow);

	// some of the managers may
	//   have cohandlers,
	//   need TPrefsDoc
	// so close them before closing
	//   the cohandler chain
	//   the preferences
	CloseDownTheManagers();
	

	// Close any windowless documents.  Add a block for failure handling
	{
		CDocumentIterator iter(this);

		for (TDocument * aDocument = iter.FirstDocument(); iter.More(); aDocument = iter.NextDocument())
			aDocument->CloseAndFree();
	}

	// Close down the cohandler chain.  Add a block for failure handling
	{
		CHandlerIterator iter(fHeadCohandler);
		for (TEventHandler *aHandler = iter.FirstHandler(); iter.More(); aHandler = iter.NextHandler())
			FreeIfObject(aHandler);				// ??? also call Terminate ??? 
	}

	if (gPrefsFileMgr)
		gPrefsFileMgr->Save();
#if qDebug
	fprintf(stderr, "< TNewsAppl::Close()\n");
#endif
	inherited::Close(); // does not free
}

void TNewsAppl::SaveWindowState()
{
#if qDebugWindowState
	fprintf(stderr, "Saves Window State\n");
#endif
	Handle h = nil;
	VOLATILE(h);
	THandleStream *aStream = nil;
	VOLATILE(aStream);
	FailInfo fi;
	if (fi.Try())
	{
		h = NewPermHandle(0);
		THandleStream *hs = new THandleStream();
		hs->IHandleStream(h, 128);
		aStream = hs; hs = nil;
		
		aStream->WriteLong(kSaveWindowVersion);
		aStream->WriteLong('TInf');
		fGroupTreeDoc->WriteTreeInfo(aStream);
		if (gPrefs->GetBooleanPrefs('WSav'))
			DoSaveWindowStates(aStream);
		aStream->WriteLong('Slut');
		
		FreeIfObject(aStream); aStream = nil;
		gPrefs->SetHandlePrefs('Wind', h);
		h = DisposeIfHandle(h);
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(aStream); aStream = nil;
		h = DisposeIfHandle(h);
		// no resignal as we're quitting
	}
}

void TNewsAppl::DoSaveWindowStates(TStream *aStream)
{
	CWMgrIterator iter(kIterateBackward);
	for (WindowPtr windP = iter.FirstWMgrWindow(); iter.More(); windP = iter.NextWMgrWindow())
	{
		TWindow *window = WMgrToWindow(windP);
		if (!window || !window->fDocument)
			continue;
		OSType id = window->fDocument->fIdentifier;
#if qDebugWindowState
		fprintf(stderr, "Writes type id = $%lx, position = %ld\n", id, aStream->GetPosition() + 4);
		fprintf(stderr, "Got window with id = $%lx\n", id);
#endif
		if (id == kGroupTreeDocFileType)
		{
#if qDebugWindowState
			fprintf(stderr, "Writes TWin\n");
#endif
			aStream->WriteLong('TWin');
			fGroupTreeDoc->WriteWindowInfo(aStream, window);
		}
		else if (id == kGroupListDocFileType)
		{
#if qDebugWindowState
			fprintf(stderr, "Writes GLis\n");
#endif
			aStream->WriteLong('GLis');
			AliasHandle ah = ( (TGroupListDoc*) window->fDocument)->GetAlias();
			aStream->WriteHandle(Handle(ah));
			DisposeIfHandle(Handle(ah)); ah = nil;
		}
	}
}

void TNewsAppl::RestoreWindowState()
{
	if (!gPrefs->PrefExists('Wind') || gPrefs->GetPrefsSize('Wind') <= 4)
		return; // got no info
#if qDebugWindowState
	fprintf(stderr, "Restores Window State\n");
#endif
	Handle h = gPrefs->GetHandlePrefs('Wind');
	VOLATILE(h);
	THandleStream *aStream = nil;
	VOLATILE(aStream);
	FailInfo fi;
	if (fi.Try())
	{
		THandleStream *hs = new THandleStream();
		hs->IHandleStream(h, 128);
		aStream = hs; hs = nil;
		if (aStream->ReadLong() != kSaveWindowVersion)
			Failure(0 ,0);
		while (true)
		{
			long id = aStream->ReadLong();
#if qDebugWindowState
			fprintf(stderr, "Restore type id = $%lx, position = %ld\n", id, aStream->GetPosition());
#endif
			if (id == 'Slut')
				break;
			else if (id == 'TInf')
				fGroupTreeDoc->ReadTreeInfo(aStream);
			else if (id == 'TWin')
				fGroupTreeDoc->ReadWindowInfo(aStream);
			else if (id == 'GLis')
			{
				AliasHandle ah = AliasHandle(aStream->ReadHandle());
				TFile *file = DoMakeFile(cNewGroupListDoc);
				if (file->SpecifyWithAlias(ah) == noErr)
				{
					FailInfo fi2;
					if (fi2.Try())
					{
						OpenOneOld(file);
						fi2.Success();
					}
				}
				DisposeIfHandle(Handle(ah)); ah = nil;
			}
			else
				break; // unknown type
		} // while
		FreeIfObject(aStream); aStream = nil;
		h = DisposeIfHandle(h);
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(aStream); aStream = nil;
		h = DisposeIfHandle(h);
		// no resignal, just ignore error
	}
}

void TNewsAppl::SetGroupTreeDoc(TGroupTreeDoc *doc)
{
#if qDebug
	if (fGroupTreeDoc)
		ProgramBreak("has fGroupTreeDoc");
#endif
	fGroupTreeDoc = doc;
}

TGroupTreeDoc *TNewsAppl::GetGroupTreeDoc()
{
#if qDebug
	if (!fGroupTreeDoc)
		ProgramBreak("Called before fGroupTreeDoc is set");
#endif
	return fGroupTreeDoc;
}

TGroupTree *TNewsAppl::GetGroupTree()
{
#if qDebug
	if (!fGroupTreeDoc)
		ProgramBreak("Called before fGroupTreeDoc is set");
#endif
	return fGroupTreeDoc->GetGroupTree();
}

void TNewsAppl::QuickUpdateGroupHasNoNewArticles(const CStr255 &dotName)
{
	CDocumentIterator iter(this);
	for (TDocument *doc = iter.FirstDocument(); iter.More(); doc = iter.NextDocument())
	{
		if (doc->fIdentifier != kGroupListDocFileType)
			continue;
		TGroupListDoc *gld = (TGroupListDoc*)doc;
		gld->UpdateGroupStatus(dotName, false);
	}
}

pascal void TNewsAppl::DoAboutBox()
{
	FailSpaceIsLow();
	FlushEvents(mDownMask | mUpMask | keyDownMask | keyUpMask | autoKeyMask, 0);
	ShowSplashScreen();
	EventRecord event;
	while (!Button() && !EventAvail(mDownMask | keyDownMask, event))
		UpdateAllWindows();
	RemoveSplashScreen();
}


pascal void TNewsAppl::GetHelpParameters(ResNumber helpResource,
														short helpIndex, short helpState,
														HMMessageRecord &helpMessage,
														CPoint &localQDTip, CRect &localQDRect, short &balloonVariant)
{
	// all of this is needed as the HelpMgr calls Get1Resource (and I have multiple rsrc files)
	short oldRefNum = CurResFile();
	VOLATILE(oldRefNum);
	FailInfo fi;
	if (fi.Try())
	{
		Handle h = GetResource(kHMDialogResType, helpResource);
		if (h)
			UseResFile(HomeResFile(h));
		inherited::GetHelpParameters(helpResource, helpIndex, helpState, helpMessage, localQDTip, localQDRect, balloonVariant);
		UseResFile(oldRefNum);
		fi.Success();
	}
	else // fail
	{
		UseResFile(oldRefNum);
		fi.ReSignal();
	}
}

pascal void TNewsAppl::DoAppleCommand(CommandNumber aCommandNumber, const AppleEvent& message, const AppleEvent& reply)
{
	if (!gPrefs && aCommandNumber == cFinderNew)
	{
#if qDebugPrefOpen
	fprintf(stderr, "Got cFinderOpen and !gPrefs in DoAppleCommand, opens default prefs\n");
#endif
		// open prefs in preferences folder
		TOpenPrefsCommand *cmd = new TOpenPrefsCommand();
		cmd->IOpenPrefsCommand();
		gApplWideThreads->ExecuteCommand(cmd, "TOpenPrefsCommand without files (cFinderNew)");
	}
	inherited::DoAppleCommand(aCommandNumber, message, reply);
}

pascal void TNewsAppl::OpenOld(CommandNumber itsOpenCommand, TList* aFileList)
{
	if (gPrefs)
	{
		inherited::OpenOld(itsOpenCommand, aFileList);
		return;
	}
#if qDebugPrefOpen
	fprintf(stderr, "Got OpenOld with !gPrefs\n");
#endif
	TList *docList = nil;
	VOLATILE(docList);
	FailInfo fi;
	if (fi.Try())
	{
		docList = NewList();
		CopyDynamicArray(aFileList, docList);
		aFileList->DeleteAll();
#if qDebugPrefOpen
		fprintf(stderr, "Executes open prefs command with %ld doc files\n", docList->GetSize());
#endif
		TOpenPrefsCommand *cmd = new TOpenPrefsCommand();
		cmd->IOpenPrefsCommand(docList);
		docList = nil;
		gApplWideThreads->ExecuteCommand(cmd, "TOpenPrefsCommand with files (OpenOld)");
		fi.Success();
	}
	else // fail: could not open prefs file
	{
#if qDebugPrefOpen
		fprintf(stderr, "Could not make open_prefs_file command, quits\n");
#endif
		DoMenuCommand(cQuit);
		FreeIfObject(docList); docList = nil;
		fi.ReSignal();
	}
}

pascal CommandNumber TNewsAppl::KindOfDocument(CommandNumber itsCommandNumber, TFile* itsFile)
{
	if (itsFile)
	{
		switch (itsFile->fFileType)
		{
			case kGroupTreeDocFileType:
				return cNewGroupTreeDoc;

			case kGroupDBFileType:
			case kArticleStatusFileType:
				FailOSErr(errNotImplemented);

			case kGroupListDocFileType:
				return cNewGroupListDoc;
			
			default:
				break;
		}
	}
	switch (itsCommandNumber)
	{
		case cNewGroupListDoc:
		case cOpenGroupListDoc:
			return cNewGroupListDoc;

		case cNewGroupTreeDoc:
		case cOpenGroupTreeDoc:
			return cNewGroupTreeDoc;
		
		default:
			return inherited::KindOfDocument(itsCommandNumber, itsFile);
	}
}

pascal TDocument *TNewsAppl::DoMakeDocument(CommandNumber itsCommandNumber, TFile *itsFile)
{
	switch (itsCommandNumber)
	{
		case cNewGroupTreeDoc:
			if (!fGroupTreeDoc)
			{
				TGroupTreeDoc *gtd = new TGroupTreeDoc();
				gtd->IGroupTreeDoc(itsFile);
				return gtd;
			}
			else 
				FailOSErr(errNotImplemented);
			
		case cNewGroupListDoc:
			gBusyCursor->ForceBusy();
			{
				TGroupListDoc *gld = new TGroupListDoc();
				gld->IGroupListDoc(itsFile);
				return gld;
			}

		default:
			FailOSErr(errNotMyType);
			return nil;
	}
}

pascal TFile* TNewsAppl::DoMakeFile(CommandNumber itsCommandNumber)
{
	TFile *file = nil;
	VOLATILE(file);
	FailInfo fi;
	if (fi.Try())
	{
		switch (itsCommandNumber)
		{
			case cOpen:
			case cFinderOpen:
			case cSave:
			case cSaveAs:
				//@@ not sure here, does the caller always change fileType etc..
				file = NewFile('????', fCreator, !kUsesDataFork, preferResourceFork, !kDataOpen, !kRsrcOpen);
				break;

			case cNewGroupListDoc:
				file = NewFile(kGroupListDocFileType, kSignature, kUsesDataFork, preferResourceFork, kDataOpen, kRsrcOpen);
				GoPrefsLocation(file, kUntitledFileName);
				break;
				
			case cNewGroupTreeDoc:
				file = NewFile(kGroupTreeDocFileType, kSignature, kUsesDataFork, preferResourceFork, kDataOpen, kRsrcOpen);
				GoPrefsLocation(file, kGroupTreeFileName);
				break;
				
			default:
				FailOSErr(errNotMyType);
		}
		fi.Success();
		return file;
	}
	else // fail
	{
		FreeIfObject(file); file = nil;
		fi.ReSignal();
	}
}

pascal void TNewsAppl::GetFileTypeList(CommandNumber, TypeListHandle& typeList)
{
	typeList = TypeListHandle(NewPermHandle(4));
	if (!gPrefs)
	{
		// if we are not up and running, only prefs file can be opened
		(*typeList)[0] = kPrefsFileType;
	}
	else if (!fGroupTreeDoc)
	{
		(*typeList)[0] = kGroupTreeDocFileType;
	}
	else
	{
		(*typeList)[0] = kGroupListDocFileType;
	}
}

pascal Boolean TNewsAppl::CanOpenDocument(CommandNumber itsCommandNumber, TFile* aFile)
{
	if (!gPrefs)
		return (aFile->fFileType == kPrefsFileType);
	if (!fGroupTreeDoc)
		return (aFile->fFileType == kGroupTreeDocFileType);
	switch (aFile->fFileType)
	{
		case kGroupDBFileType:
		case kArticleStatusFileType:
			FailOSErr(errNotImplemented);

		case kPrefsFileType:
		case kGroupTreeDocFileType:
			FailOSErr(errNotImplemented);

		case kGroupListDocFileType:
			return true;
			
		default:
			return inherited::CanOpenDocument(itsCommandNumber, aFile);
	}
}

void TNewsAppl::OpenOneOld(TFile *file)
{
	VOLATILE(file);
	TList *list = nil;
	VOLATILE(list);
	FailInfo fi;
	if (fi.Try())
	{
		list = NewList();
		list->InsertLast(file); file = nil;
		OpenOld(cOpen, list);
		FreeIfObject(list); list = nil;
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(file); file = nil;
		if (list)
		{
			list->DeleteAll();
			FreeIfObject(list); list = nil;
		}
		fi.ReSignal();
	}
}

pascal void TNewsAppl::DoToolboxEvent(TToolboxEvent* event)
{
	// avoid to yield while updating windows as CSubViewIterator has
	// links in the stack
	Boolean wasDisabled = gCurThread->SetYieldDisable(true);
	VOLATILE(wasDisabled);
	FailInfo fi;
	if (fi.Try())
	{
		inherited::DoToolboxEvent(event);
		gCurThread->SetYieldDisable(wasDisabled);
		fi.Success();
	}
	else // fail
	{
		gCurThread->SetYieldDisable(wasDisabled);
		fi.ReSignal();
	}
}

pascal void TNewsAppl::SpaceIsLowAlert()
{
	if (GetNumThreads() <= 1)
	{
		inherited::SpaceIsLowAlert();
		return;
	}
	// Show 'space is low' alert only after ever fLowSpaceInterval ticks. 
	if ((fLowSpaceInterval > 0) && this->IsFrontProcess())
	{
		long now = TickCount();
		if (now > fNextSpaceMessage)
		{
			gInhibitNestedHandling = TRUE;		// Don't tell em again from the alert 
			StdAlert(phMySpaceIsLow);
			fNextSpaceMessage = now + fLowSpaceInterval;
		}
	}
}

pascal void TNewsAppl::UpdateAllWindows()
{
	TToolboxEvent * event;

	++fEventLevel;
	short iters = 0;
	while (event = this->GetEvent(updateMask + activMask, 0, NULL))
	{
		event->Process();
		if (++iters > 2)
			break;
	}
	--fEventLevel;
}

pascal TToolboxEvent *TNewsAppl::GetEvent(short eventMask, long sleep, RgnHandle sleepRegion)
{
	if (GetNumThreads() > 1)
	{
		Boolean wasDisabled = gCurThread->SetYieldDisable(false);
		gCurThread->YieldTime();
		gCurThread->SetYieldDisable(wasDisabled);
		if (IsFrontProcess())
			sleep = 0;
		else
			sleep = Min(6, sleep); // be a bit gently when in background
	}
	if (fFlushVolumes)
		FlushVols();
	return inherited::GetEvent(eventMask, sleep, sleepRegion);
}

pascal void TNewsAppl::DoKeyEvent(TToolboxEvent *event)
{
	switch (event->fCharacter)
	{
		case 19: // ctrl-s
			{
				CDocumentIterator iter(this);
				for (TDocument *doc = iter.FirstDocument(); iter.More(); doc = iter.NextDocument())
				{
					if (doc->GetChangeCount() > 0)
						doc->SaveDocument(cSave);
				}
				FlushVols();
			}
			break;
				
		default:
			inherited::DoKeyEvent(event);
			break;
	}
}

//========================== M E N U S ====================================
Boolean TNewsAppl::HandleFontMenu(CommandNumber cmd, TextStyle &ts)
{
	if (cmd >= 0)
		return false;
	short menu, item;
	CommandToMenuItem(cmd, menu, item);
	CStr255 s;
	if (menu == mFontSize)
	{
		GetItem(fFontSizeMenuH, item, s);
		long size;
		StringToNum(s, size);
		if (ts.tsSize == size)
			return false;
		ts.tsSize = short(size);
		return true;
	}
	else if (menu == mFontName)
	{
		GetItem(fFontNameMenuH, item, s);
		short num;
		GetFNum(s, num);
		if (num == ts.tsFont)
			return false;
		ts.tsFont = num;
		return true;
	}
	else
		return false;
}					

void TNewsAppl::CreateMenus()
{
// Group lists
	fGroupListMenuH = MAGetMenu(mGroupLists); // GetMHandle
	FailNIL(fGroupListMenuH);

// Window
	fWindowMenuH = MAGetMenu(mWindows);
	FailNIL(fWindowMenuH);

// Font size
	fFontSizeMenuH = MAGetMenu(mFontSize);
	FailNIL(fFontSizeMenuH);

// Font names
	fFontNameMenuH = MAGetMenu(mFontName);
	FailNIL(fFontNameMenuH);
	AddResMenu(fFontNameMenuH, 'FONT');
	NeedCalcMenuSize(fFontNameMenuH);
}

void TNewsAppl::EnableFontMenu(TextStyle ts)
{
// size
	short item = CountMItems(fFontSizeMenuH);
	CStr255 s, currSel;
	NumToString(ts.tsSize, currSel);
	while (item >= 1)
	{
		GetItem(fFontSizeMenuH, item, s);
		CheckItem(fFontSizeMenuH, item, s == currSel);
		item--;
	}
	(*fFontSizeMenuH)->enableFlags = 0xFFFFFFFF;
// names
	GetFontName(ts.tsFont, currSel);
	item = CountMItems(fFontNameMenuH);
	while (item >= 1)
	{
		GetItem(fFontNameMenuH, item, s);
		CheckItem(fFontNameMenuH, item, s == currSel);
		item--;
	}
	(*fFontNameMenuH)->enableFlags = 0xFFFFFFFF;
}

void TNewsAppl::DoWindowsMenu(short itemToSelect)
{
	(*fWindowMenuH)->enableFlags = 0;
	short itemNo = CountMItems(fWindowMenuH);
	for (;itemNo; itemNo--)
		DelMenuItem(fWindowMenuH, itemNo);
	GrafPtr savePort;
	GetPort(savePort);
	SetPort(gWorkPort);
	SetPortTextStyle(gSystemStyle);
	Boolean isModal = true;
	WindowPtr fw = MAFrontWindow();
	if (fw)
	{
		TWindow *wind = WMgrToWindow(fw);
		if (wind)
			isModal = wind->IsInModalState();
	}

	Boolean gotADocument = false;
	Boolean multipleDocuments = (fDocumentList->fSize > 1);
	CDocumentIterator docIter(this);
	for (TDocument *doc = docIter.FirstDocument(); docIter.More(); doc = docIter.NextDocument())
	{
#if qDebug
		if (!IsObject(doc))
		{
			fprintf(stderr, "doc = $%lx is not object\n", long(doc));
			ProgramBreak(gEmptyString);
			continue;
		}
#endif
		Boolean firstDocWindow = true;
		CWindowIterator windIter(doc);
		for (TWindow *wind = windIter.FirstWindow(); windIter.More(); wind = windIter.NextWindow())
		{
#if qDebug
			if (!IsObject(wind))
				ProgramBreak("wind is not object");
#endif
//			if (wind->IsShown() && !wind->fFloats)
			if (wind->IsShown())
			{
				CStr255 title;
				if (firstDocWindow)
				{
					if (gotADocument)
					{
						AppendMenu(fWindowMenuH, "-");
						itemNo++;
					}
#if 0
					if (multipleDocuments)
					{
						AppendMenu(fWindowMenuH, "z"); 
						itemNo++;
						title = doc->fTitle;
						if (title == "")
							title = "?";
						SetItem(fWindowMenuH, itemNo, title);
					}
#endif
				}
				WindowPtr windP = wind->GetGrafPort();
				GetWTitle(windP, title);
				TruncSystemFontString(300, title, smTruncMiddle);
#if 1
				if (firstDocWindow == false)
					title.Insert("   ", 1);
#else
				if (multipleDocuments)
					title.Insert("   ", 1);
#endif
				if (wind->fFloats)
					title.Insert(" ", 1);
				if (title.Length() == 0)
					continue;
				AppendMenu(fWindowMenuH, "z"); 
				itemNo++;
				SetItem(fWindowMenuH, itemNo, title);
				if (itemNo == itemToSelect)
				{
					InvalidateFocus();
					wind->Select();
				}
				if (!isModal)
					EnableItem(fWindowMenuH, itemNo);
				if (windP == fw)
					CheckItem(fWindowMenuH, itemNo, true);
				firstDocWindow = false;
				gotADocument = true;
			}
		}
	}
	Boolean firstWindow = true;
	CObjectIterator objIter(fFreeWindowList);
	for (TObject *obj = objIter.FirstObject(); objIter.More(); obj = objIter.NextObject())
	{
		TWindow *wind = (TWindow*)obj;
#if qDebug
			if (!IsObject(wind))
				ProgramBreak("wind is not object");
#endif
//		if (wind->IsShown() && !wind->fFloats)
		if (wind->IsShown())
		{
			if (gotADocument && firstWindow)
			{
				AppendMenu(fWindowMenuH, "-");
				itemNo++;
			}
			WindowPtr windP = wind->GetGrafPort();
			CStr255 title;
			GetWTitle(windP, title);
			TruncSystemFontString(300, title, smTruncMiddle);
			AppendMenu(fWindowMenuH, "z"); 
			itemNo++;
			if (itemNo == itemToSelect)
			{
				InvalidateFocus();
				wind->Select();
				return;
			}
			SetItem(fWindowMenuH, itemNo, title);
			if (!isModal)
				EnableItem(fWindowMenuH, itemNo);
			if (windP == fw)
				CheckItem(fWindowMenuH, itemNo, true);
			firstWindow = false;
		}
	}
	SetPort(savePort);
	NeedCalcMenuSize(fWindowMenuH);
}

void TNewsAppl::UpdateGroupListMenu()
{
	if (fHadGroupList)
		(*fGroupListMenuH)->enableFlags = 0x7FFFFFFF;
	if (fNeedCheckFolderMenus == false)
		return;
	CStr255 name;
	FSSpec spec;
	GetPrefsDocLocation(spec);
	CInfoPBRec pbc;
	pbc.dirInfo.ioFDirIndex = -1;
	pbc.dirInfo.ioNamePtr = (StringPtr)name;
	pbc.dirInfo.ioVRefNum = spec.vRefNum;
	pbc.dirInfo.ioDrDirID = spec.parID;
	if (PBGetCatInfoSync(&pbc) != noErr)
		return;
	if (pbc.dirInfo.ioDrMdDat == fLastFolderModDate)
		return;
#if qDebug
	fprintf(stderr, "Updates group list menu\n");
#endif
	fLastFolderModDate = pbc.dirInfo.ioDrMdDat;
	short noItems = CountMItems(fGroupListMenuH);
	while (noItems)
		DelMenuItem(fGroupListMenuH, noItems--);
	HParamBlockRec pbf;
	pbf.fileParam.ioFDirIndex = 1;
	while (true)
	{
		pbf.fileParam.ioVRefNum = spec.vRefNum;
		pbf.fileParam.ioDirID = spec.parID;
		pbf.fileParam.ioNamePtr = (StringPtr)name;
		pbf.fileParam.ioFVersNum = 0;
		if (PBHGetFInfoSync(&pbf) != noErr)
			break;
		pbf.fileParam.ioFDirIndex++;
		if (pbf.fileParam.ioFlFndrInfo.fdType != kGroupListDocFileType)
			continue;

		noItems++;
		AppendMenu(fGroupListMenuH, "hi there"); // may not be empty
		SetItem(fGroupListMenuH, noItems, name);
		if (noItems <= 9)
			SetItemCmd(fGroupListMenuH, noItems, '0' + noItems);
		fHadGroupList = true;
	}
	if (noItems)
		(*fGroupListMenuH)->enableFlags = 0x7FFFFFFF;
	else
	{
		MyGetIndString(name, kEmptyGroupListMenuText);
		AppendMenu(fGroupListMenuH, "hi there"); // may not be empty
		SetItem(fGroupListMenuH, 1, name);
		(*fGroupListMenuH)->enableFlags = 0;
	}
	NeedCalcMenuSize(fGroupListMenuH);
	fNeedCheckFolderMenus = false;
}

pascal void TNewsAppl::RegainControl(Boolean checkClipboard)
{
	SetNeedCheckFolderMenus();
	inherited::RegainControl(checkClipboard);
}

void TNewsAppl::SetNeedCheckFolderMenus()
{
	fNeedCheckFolderMenus = true;
}

void TNewsAppl::HandleGroupListMenu(short item)
{
	FailInfo fi;
	if (fi.Try())
	{
		CStr255 name;
		GetItem(fGroupListMenuH, item, name);
	
		TFile *file = gNewsAppl->DoMakeFile(cNewGroupListDoc);
		file->SetName(name);
		ResolveAliasFile(file);
		gNewsAppl->OpenOneOld(file);
		fi.Success();
	}
	else // fail
		FailNewMessage(fi.error, fi.message, messageOpenGroupFailed);
}
//---------------------
pascal void TNewsAppl::DoMenuCommand(CommandNumber aCommandNumber)
{
	TWindow *aWindow;			
	if (aCommandNumber < 0)
	{
		short menu, itemNo;
		CommandToMenuItem(aCommandNumber, menu, itemNo);
		if (menu == mWindows)
		{
			DoWindowsMenu(itemNo);
			return;
		}
		else if (menu == mGroupLists)
		{
			HandleGroupListMenu(itemNo);
			return;
		}
	}
	switch (aCommandNumber)
	{
		case cQuit:
			SetupTheMenus();
			if (CommandEnabled(cQuit))
				inherited::DoMenuCommand(cQuit);
			break;
			
		case cOpenListOfAllGroups:
			fGroupTreeDoc->CreateGroupTreeWindow();
			break;
		
//-----

		case cPrefShowAllArticles:
			gPrefs->SetShortPrefs('artS', kShowAllArticles);
			break;

		case cPrefShowOnlyFirstArticle:
			gPrefs->SetShortPrefs('artS', kShowFirstArticleOnly);
			break;

		case cPrefShowOnlyUnreadArticles:
			gPrefs->SetShortPrefs('artS', kShowUnreadArticles);
			break;

		case cPrefShowOnlyNewArticles:
			gPrefs->SetShortPrefs('artS', kShowNewArticles);
			break;

		case cPrefShowNoneArticles:
			gPrefs->SetShortPrefs('artS', kShowNoArticles);
			break;

//-----

		case cPrefShowAllDiscussions:
			gPrefs->SetShortPrefs('disS', kShowAllDiscs);
			break;

		case cPrefShowOnlyTodayDiscussions:
			gPrefs->SetShortPrefs('disS', kShowTodaysDiscs);
			break;

		case cPrefShowDiscsWithUnreadArticles:
			gPrefs->SetShortPrefs('disS', kShowDiscsWithUnreadArticles);
			break;

		case cPrefShowDiscsWithNewArticles:
			gPrefs->SetShortPrefs('disS', kShowDiscsWithNewArticles);
			break;

//-----

		case cUpdateGroupTree:
			{
				TUpdateListOfAllGroupsCommand *cmd = new TUpdateListOfAllGroupsCommand();
				cmd->IUpdateListOfAllGroupsCommand(fGroupTreeDoc, false);
				gApplWideThreads->ExecuteCommand(cmd, "TUpdateListOfAllGroupsCommand (update)");
			}
			break;
		
		case cRebuildGroupTree:
			{
				TUpdateListOfAllGroupsCommand *cmd = new TUpdateListOfAllGroupsCommand();
				cmd->IUpdateListOfAllGroupsCommand(fGroupTreeDoc, true);
				gApplWideThreads->ExecuteCommand(cmd, "TUpdateListOfAllGroupsCommand (rebuild)");
			}
			break;
		
		case cOpenYourNamePrefs:
			DoYourNamePreferencesDialog();
			break;
			
		case cOpenNewsServerPrefs:
			DoNewsServerPreferencesDialog();
			break;

		case cOpenBinariesPrefs:
			DoBinariesPreferencesDialog();
			break;

		case cOpenEditorPrefs:
			DoEditorPreferencesDialog();
			break;
		
		case cOpenMailerPrefs:
			DoMailerPreferencesDialog();
			break;
		
		case cOpenMiscPrefs:
			DoMiscPreferencesDialog();
			break;

		case cForgetPassword:
			InvalidateCurrentPassword();
			break;

		case cSysBug:
			ProgramBreak("SysBug");
			break;
			
		case cMacsBug:
			DebugStr("MenuMacsBug");
			break;
			
		case cTest1: Test1(); break;
		case cTest2: Test2(); break;
		case cTest3: Test3(); break;
		case cTest4: Test4(); break;
		
		case cDumpPrefs:
			gPrefs->DumpPrefs();
			break;
			
		case cDebugDumpThreads:
			DumpDebugThreadDescription();
			break;

		case cShowViewInspector:
			{
				FailNIL(aWindow = gViewServer->NewTemplateWindow(kViewInspector, NULL));
				aWindow->SetResizeLimits(CPoint(210,180),CPoint(210,1000));
				aWindow->Open();
			}
			break;
		
		case cShowTargetInspector:
			{
				FailNIL(aWindow = gViewServer->NewTemplateWindow(kTargetInspector, NULL));
				aWindow->SetResizeLimits(CPoint(210,180),CPoint(210,1000));
				aWindow->Open();
			}
			break;

		case cToggleFlushVolumes:
			fFlushVolumes = !fFlushVolumes;
			break;
		
		case cMenuForcedFatalError:
			PanicExitToShell("  Forced from menu");
			break;
			
		default:
			inherited::DoMenuCommand(aCommandNumber);
	}
}

pascal void TNewsAppl::DoSetupMenus()
{
// Test menu
	Enable(cTest1, true);
	Enable(cTest2, true);
	Enable(cTest3, true);
	Enable(cTest4, true);
	if (gPrefs)
		Enable(cDumpPrefs, true);
	Enable(cDebugDumpThreads, true);
	Enable(cShowViewInspector, true);
	Enable(cShowTargetInspector, true);
	EnableCheck(cToggleFlushVolumes, true, fFlushVolumes);
	Enable(cMacsBug, true);
	Enable(cSysBug, true);
	Enable(cMenuForcedFatalError, true);
	if (!gPrefs || !fApplIsRunning || !fGroupTreeDoc)
	{
		inherited::DoSetupMenus();
		Enable(cNew, false);
		if (gPrefs)
		{
			Enable(cOpen, false);
			// may not disable cOpen before we open the prefs,
			// as we need to be able to open it!
		}
		return;
	}
	Boolean lowSpace = MemSpaceIsLow();

// File menu
	Enable(cGroupListsMenu, !lowSpace);
	UpdateGroupListMenu();

	Enable(cNewGroupListDoc, !lowSpace);
	Enable(cOpenListOfAllGroups, !lowSpace);

// Discussions
	Enable(cDiscussionsPrefs, true);
	DiscussionShowType discShowType = DiscussionShowType(gPrefs->GetShortPrefs('disS'));
	EnableCheck(cPrefShowAllDiscussions, true, discShowType == kShowAllDiscs);
	EnableCheck(cPrefShowOnlyTodayDiscussions, true, discShowType == kShowTodaysDiscs);
	EnableCheck(cPrefShowDiscsWithUnreadArticles, true, discShowType == kShowDiscsWithUnreadArticles);
	EnableCheck(cPrefShowDiscsWithNewArticles, true, discShowType == kShowDiscsWithNewArticles);

// Articles
	Enable(cArticlesPrefs, true);
	ArticleShowType showWhat = ArticleShowType(gPrefs->GetShortPrefs('artS'));
	EnableCheck(cPrefShowAllArticles, true, showWhat == kShowAllArticles);
	EnableCheck(cPrefShowOnlyFirstArticle, true, showWhat == kShowFirstArticleOnly);
	EnableCheck(cPrefShowOnlyUnreadArticles, true, showWhat == kShowUnreadArticles);
	EnableCheck(cPrefShowOnlyNewArticles, true, showWhat == kShowNewArticles);
	EnableCheck(cPrefShowNoneArticles, true, showWhat == kShowNoArticles);

// Preferences
	if (!lowSpace)
	{
		Enable(cFontNameMenu, true);
		Enable(cFontSizeMenu, true);
		Enable(cOpenYourNamePrefs, true);
		Enable(cOpenBinariesPrefs, true);
		Enable(cOpenEditorPrefs, true);
		Enable(cOpenMailerPrefs, true);
		Enable(cOpenNewsServerPrefs, true);
		Enable(cOpenMiscPrefs, true);
		Enable(cUpdateGroupTree, true);
		Enable(cRebuildGroupTree, true);
		Enable(cForgetPassword, HasPassword());
	}
	
// Windows
	DoWindowsMenu(-1);

// inherited
	inherited::DoSetupMenus();
	
	if (IsControlKeyDown())
		Enable(cQuit, true);
}
