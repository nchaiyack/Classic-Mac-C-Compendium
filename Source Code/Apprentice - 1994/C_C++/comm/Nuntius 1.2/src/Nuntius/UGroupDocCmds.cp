// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupDocCmds.cp

#include "UGroupDocCmds.h"
#include "UNewsAppl.h"
#include "UNntp.h"
#include "UProgress.h"
#include "UGroupDoc.h"
#include "UDiscList.h"
#include "UDiscListView.h"
#include "UGroupTree.h"
#include "UHeaderList.h"
#include "FileTools.h"
#include "Tools.h"
#include "NetAsciiTools.h"
#include "UThread.h"
#include "UPrefsFileMgr.h"

#include <ErrorGlobals.h>
#include <RsrcGlobals.h>
#include <Script.h>
#include <Folders.h>
#include <ToolUtils.h>
#include <Errors.h>


#pragma segment MyGroup

#define qDebugShowDiscListWindow qDebug & 0
#define qDebugHeaderProgress qDebug & 0
#define qDebugDiscListCreateNewDiscussion qDebug & 0

void GoPublicFile(const CStr255 &groupDotName, FSSpec &spec)
{
	FailOSErr(FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, spec.vRefNum, spec.parID));
	gCurThread->CheckYield();
	CStr255 folderName;
	MyGetIndString(folderName, kPublicFilesFolderName);
	if (UsesFakeNntp())
		folderName += " (DB)";
	CopyCString2String(folderName, spec.name);
	gCurThread->CheckYield();
	GoInsideFolder(spec);
	gCurThread->CheckYield();
	GoGroupFile(groupDotName, spec);
	gCurThread->CheckYield();
}

//--------------------------------------------------------------------------------

TOpenGroupCommand::TOpenGroupCommand()
{
}

pascal void TOpenGroupCommand::Initialize()
{
	inherited::Initialize();
	fDoc = nil;
	fNntp = nil;
}

void TOpenGroupCommand::IOpenGroupCommand(const CStr255 &name, 
			Boolean updateDatabase, Boolean updateOnly, Boolean saveIfNonexisting)
{
	inherited::ICommand(cOpenGroupDoc, nil, false, false, nil);
	fGroupDotName = name;
	fUpdateDatabase = updateDatabase;
	fUpdateOnly = updateOnly;
	fSaveIfNonexisting = saveIfNonexisting;
}

pascal void TOpenGroupCommand::Free()
{
	gNntpCache->ReturnNntp(fNntp); fNntp = nil;
	fDoc = nil;
	inherited::Free();
}

pascal void TOpenGroupCommand::DoIt()
{
	FailInfo fi;
	if (fi.Try())
	{
		if (!fUpdateOnly)
			gCurProgress->SetStandardProgressType();
		CStr255 dotName(fGroupDotName);
		gCurProgress->SetTitle(dotName);
		gCurProgress->SetText(kOpenGroupReadsFile);
		gCurProgress->SetWorkToDo(kCandyStribes);
		Boolean progressWindowToFront = !fUpdateOnly;
		gCurProgress->StartProgress(progressWindowToFront);
		OpenDocument();
		gCurThread->CheckYield();
		UpdateDiscussions();
		fDoc->DoPostUpdate(fUpdateOnly);
		gCurThread->CheckYield();
		if (fUpdateOnly)
			fDoc->CloseAndFree();
		else
		{
			ShowDiscListWindow();
			CStr255 dotName(fGroupDotName);
			gNewsAppl->QuickUpdateGroupHasNoNewArticles(dotName);
			gNewsAppl->GetGroupTree()->SetLastReadArticleID(dotName, fLastID);
		}
		fi.Success();
	}
	else // fail
	{
		gCurProgress->WorkDone();
		if (fDoc)
		{
			fDoc->SetChangeCount(0); // abort changes -> avoid save
			fDoc->CloseAndFree(); fDoc = nil;
		}
		gErrorParm3 = fGroupDotName;
		FailNewMessage(fi.error, fi.message, messageOpenGroupFailed); 
	}
}

#if qDebugShowDiscListWindow
void GetWindowName(WindowPtr window, CStr255 &name)
{
	StringPtr p = (StringPtr) *WindowPeek(window)->titleHandle;
	name = p;
}
#endif

void TOpenGroupCommand::ShowDiscListWindow()
{
	EventRecord event; // we may not do this stuff when any update events is pending
//	CEventListIterator iter(fEventList);
	while (true)
	{
		Boolean hasEvent = false;
/*
		for (TEvent *itsEvent = iter.FirstEvent(); iter.More(); itsEvent = iter.NextEvent())
		{
			if (itsEvent->IsReadyToExecute())
			{
#if qDebugShowDiscListWindow
		fprintf(stderr, "ShowDiscListWindow (cmd = $%lx) waits for TEvent to be processed\n", long(this));
#endif
				hasEvent = true;
				break;
			}
*/
		if (!hasEvent)
		{
			if (!EventAvail(activMask, event))
				break;
//#if qDebugShowDiscListWindow
#if qDebug
		fprintf(stderr, "ShowDiscListWindow (cmd = $%lx) waits for activate event to be processed\n", long(this));
#endif
		}
		gCurThread->CheckYield();
	}
#if qDebugShowDiscListWindow
	CStr255 wname;
#endif
	fDoc->fDiscListView->UpdateList();
	TWindow *pw = gCurProgress->GetWindow();
	Boolean gotIt = false;
	if (pw)
	{
		GrafPtr progressGrafP = pw->GetGrafPort();
		if (progressGrafP)
		{
			WindowPtr discListWindowP = fDoc->fDiscListWindow->GetGrafPort();
#if qDebugShowDiscListWindow
			GetWindowName(progressGrafP, wname);
			fprintf(stderr, "Progress window $%lx, name = %s\n", long(progressGrafP), (char*)wname);
			GetWindowName(discListWindowP, wname);
			fprintf(stderr, "DiscListWindow  $%lx, name = '%s'\n", long(discListWindowP), (char*)wname);
#endif
			WindowPtr prevWindow = nil;
			WindowPtr sendBehindWindow = nil; // nil == not found
			CWMgrIterator iter;
			for (WindowPtr aWindowPtr = iter.FirstWMgrWindow(); iter.More(); aWindowPtr = iter.NextWMgrWindow())
			{
#if qDebugShowDiscListWindow
				GetWindowName(aWindowPtr, wname);
				fprintf(stderr, "Compare window $%lx,  name = '%s' - ", long(aWindowPtr), (char*)wname);
#endif
				if (aWindowPtr == progressGrafP)
				{
#if qDebugShowDiscListWindow
					fprintf(stderr, "found as progress window\n");
#endif
					sendBehindWindow = prevWindow;
					break;
				}
				if (aWindowPtr == discListWindowP)
				{
#if qDebugShowDiscListWindow
					fprintf(stderr, "is disclist, so it's fully ignored\n");
#endif
				}
				else if (!WindowPeek(aWindowPtr)->visible)
				{
#if qDebugShowDiscListWindow
					fprintf(stderr, "is invisible, so it's ignored\n");
#endif
				}
				else
				{
#if qDebugShowDiscListWindow
					fprintf(stderr, "skipped and stored\n");
#endif
					prevWindow = aWindowPtr;
				}
			}
#if qDebugShowDiscListWindow
			fprintf(stderr, "Done with progress window (hides it)\n");
#endif
			gCurProgress->WorkDone();
			if (sendBehindWindow)
			{
				if (aWindowPtr != progressGrafP)
				{
#if qDebug
					ProgramBreak("FrontWindow was not progress window");
#endif
					Failure(minErr, 0);
				}
//				if (MAFrontWindow() == aWindowPtr)
//					pw->Activate(false); //hack, as it otherwise didn't gets deactivated
				if (discListWindowP == sendBehindWindow)
				{
#if qDebug
					ProgramBreak("fDiscListwindow was sendBehindWindow!");
#endif
					Failure(minErr, 0);
				}
#if qDebugShowDiscListWindow
				GetWindowName(discListWindowP, wname);
				fprintf(stderr, "Does SendBehind:\n");
				fprintf(stderr, "-  DiscListWindow = $%lx, name = '%s'\n", long(discListWindowP), (char*)wname);
				GetWindowName(sendBehindWindow, wname);
				fprintf(stderr, "-  BeHindWindow   = $%lx, name = %s\n", long(sendBehindWindow), (char*)wname);
#endif
				SendBehind(fDoc->fDiscListWindow->GetGrafPort(), sendBehindWindow);
			}
			else
			{
				fDoc->fDiscListWindow->Select();
			}
			fDoc->fDiscListWindow->Open();
			gotIt = true;
		}
	}
	if (!gotIt)
	{
		fDoc->fDiscListWindow->Select();
		fDoc->fDiscListWindow->Open();
	}
}

//..........................................................................................
void TOpenGroupCommand::OpenDocument()
{
	TGroupDoc *doc = nil;
	VOLATILE(doc);
	TFile *statFile = nil;
	VOLATILE(statFile);
	TFile *dbFile = nil;
	VOLATILE(dbFile);
	FailInfo fi;
	if (fi.Try())
	{
		FSSpec spec;
		CStr255 dotName(fGroupDotName);
	
		gCurThread->CheckYield();
		GetPrefsDocLocation(spec);
		gCurThread->CheckYield();
		GoGroupFile(dotName, spec);
		gCurThread->CheckYield();
		statFile = NewFile(kArticleStatusFileType, kSignature, kUsesDataFork, noResourceFork, kDataOpen, !kRsrcOpen);
		statFile->Specify(spec);
		statFile->SetPermissions(fsRdWrPerm, fsRdWrPerm);
		TDocument *otherDoc = gApplication->FindDocument(statFile);
		if (otherDoc)
		{
			if (!fUpdateOnly)
				otherDoc->OpenAgain(cOpenGroupDoc, nil);
			Failure(0, 0); // silent like cancel
		}
		ResolveAliasFile(statFile);
		fGotStatFile = FileExist(statFile);
		gCurThread->CheckYield();
		
		GoPublicFile(dotName, spec);
		dbFile = NewFile(kGroupDBFileType, kSignature, kUsesDataFork, noResourceFork, !kDataOpen, !kRsrcOpen);
		dbFile->Specify(spec);
		dbFile->SetPermissions(fsRdPerm, fsRdPerm);
		ResolveAliasFile(dbFile);
		fGotDBFile = FileExist(dbFile);
		gCurThread->CheckYield();

		TFile *f1 = statFile; statFile = nil; // we don't own that one anymore!
		TFile *f2 = dbFile; dbFile = nil;     // nor that one
		TGroupDoc *aGroupDoc = new TGroupDoc();
		aGroupDoc->IGroupDoc(f1, f2, dotName);
		doc = aGroupDoc;
		gCurThread->CheckYield();
		
		if (fGotStatFile)
			doc->fFileHandler->ReadFile(kForDisplay);
		gCurThread->CheckYield();
		if (fGotDBFile)
			doc->fDBFileHandler->ReadFile(kForDisplay);
		doc->SetChangeCount(0);

		gCurThread->CheckYield();
		doc->DoMakeViews(kForDisplay);
		FailSpaceIsLow();				// Fail if the document leaves us with no memory
		doc->DoPostMakeViews(kForDisplay);
		fDoc = doc;
		fi.Success();
	}
	else // fail
	{
		if (doc)
		{
			doc->SetChangeCount(0); // abort changes -> avoid save
			doc->CloseAndFree(); doc = nil;
		}
		else
		{
			FreeIfObject(statFile); statFile = nil;
			FreeIfObject(dbFile); dbFile = nil;
		}
		gErrorParm3 = fGroupDotName;
		FailNewMessage(fi.error, fi.message, messageOpenGroupFailed); 
	}
}
//..........................................................................................

Boolean TOpenGroupCommand::StartUpdate()
{
	gCurProgress->SetText(kOpenGroupChecksForNew);
	gCurThread->CheckYield();
	CStr255 dotName(fGroupDotName);
	fNntp->SetGroup(dotName, true);
	long firstID, lastID;
	fNntp->GetGroupInfo(firstID, lastID);
	long numArticles = Min(lastID - firstID + 1, gPrefs->GetLongPrefs('Mart'));
	firstID = lastID - numArticles + 1;
	fFirstID = firstID;
	fLastID = lastID;
	fDoc->fArticleStatus->SetNewRange(fFirstID, fLastID); // always do this as files can be deleted
	fDoc->fOldArticleStatus->SetNewRange(fFirstID, fLastID);
	if (fFirstID > fLastID || fFirstID == 0)
	{
#if qDebug
		fprintf(stderr, "UPDATE: Empty group: fFirstID = %ld > fLastID = %ld\n", fFirstID, fLastID);
#endif
		fDoc->fFirstArticleID = fFirstID;
		fDoc->fLastArticleID = fLastID;
		fDoc->fSubjectHdr->DeleteAll();
		fDoc->fFromHdr->DeleteAll();
		fDoc->fMsgIDHdr->DeleteAll();
		fDoc->fRefsHdr->DeleteAll();
		fDoc->fDiscList->DeleteAll();
		return false; // empty group
	}
	if (fFirstID >= fDoc->fFirstArticleID && fFirstID <= fDoc->fLastArticleID + 1)
	{
		// could I do more sanity checking here??
		if (fLastID <= fDoc->fLastArticleID)
		{
#if qDebug
			fprintf(stderr, "UPDATE: no new stuff\n");
#endif
			return false; // no new stuff (funny, we don't delete old stuff, when there aren't new stuff)
		}
#if qDebug
		fprintf(stderr, "UPDATE: Old: %ld -> %ld,   new: %ld -> %ld", fDoc->fFirstArticleID, fDoc->fLastArticleID, fFirstID, fLastID);
#endif
		fFirstNewID = fDoc->fLastArticleID + 1;
		fDoc->fFirstArticleID = fFirstID;
		fDoc->fLastArticleID = fLastID;
#if qDebug
		fprintf(stderr, ",  update: %ld -> %ld\n\n", fFirstNewID, fLastID);
#endif
	}
	else // otherwise the stuff we've got is out of synch
	{
#if qDebug
		fprintf(stderr, "UPDATE: Delete all old headers (old: %ld -> %ld, new: %ld -> %ld)\n", fDoc->fFirstArticleID, fDoc->fLastArticleID, fFirstID, fLastID);
#endif
		fFirstNewID = fFirstID;
		fDoc->fFirstArticleID = fFirstID;
		fDoc->fLastArticleID = fLastID;
		fDoc->fSubjectHdr->DeleteAll();
		fDoc->fFromHdr->DeleteAll();
		fDoc->fMsgIDHdr->DeleteAll();
		fDoc->fRefsHdr->DeleteAll();
		fDoc->fDiscList->DeleteAll();
	}
	long work = (fLastID - fFirstNewID + 1) * (4 + 1); // headers 4, distribute 1
	gCurProgress->SetWorkToDo(work);
#if qDebug
	fprintf(stderr, "Work to do: %ld:  %ld %ld\n", work, fFirstNewID, fLastID);
#endif
	return true;
}

void TOpenGroupCommand::UpdateHeaderLists()
{
	gCurProgress->SetText(kOpenGroupFetchesHeaders);
	gCurThread->CheckYield();
	Handle h = nil;
	long firstNewID = fFirstNewID;
	long lastID = fLastID;
	fDoc->fMsgIDHdr->DeleteAll();
	fDoc->fRefsHdr->DeleteAll();
#if qDebugHeaderProgress
	fprintf(stderr, "Has to fetch %ld headers  %ld %ld\n", fLastID - firstNewID, firstNewID, fLastID);
#endif
	while (true)
	{
		lastID = Min(fLastID, firstNewID + gPrefs->GetLongPrefs('XHSz') - 1);
#if qDebugHeaderProgress
		fprintf(stderr, "Fetches %ld headers  %ld - %ld\n", lastID - firstNewID + 1, firstNewID, lastID);
		fprintf(stderr, "Before work: %ld\n", gCurProgress->GetWorkDone());
#endif

		 // header names are hardcoded, but they are hardcoded in RFC 977 too
		h = fNntp->GetHeaderList("Subject", firstNewID, lastID);
		TranslateViaTable(gNetAscii2Mac, *h, GetHandleSize(h));
		fDoc->fSubjectHdr->AddNewHeaders(h, fFirstID, firstNewID, lastID);
		h = DisposeIfHandle(h);
		
		h = fNntp->GetHeaderList("From", firstNewID, lastID);
		TranslateViaTable(gNetAscii2Mac, *h, GetHandleSize(h));
		fDoc->fFromHdr->AddNewHeaders(h, fFirstID, firstNewID, lastID);
		h = DisposeIfHandle(h);
	
		h = fNntp->GetHeaderList("Message-ID", firstNewID, lastID);
		fDoc->fMsgIDHdr->AddNewHeaders(h, fFirstID, firstNewID, lastID);
		h = DisposeIfHandle(h);

		h = fNntp->GetHeaderList("References", firstNewID, lastID);
		fDoc->fRefsHdr->AddNewHeaders(h, fFirstID, firstNewID, lastID);
		h = DisposeIfHandle(h);

#if qDebugHeaderProgress
		fprintf(stderr, "After work: %ld\n", gCurProgress->GetWorkDone());
#endif
		if (lastID >= fLastID)
			break;
		firstNewID = lastID + 1;
		lastID = fLastID;
	}
}

void TOpenGroupCommand::CreateNewDiscussion(long id)
{
	HandleOffsetLength discIDhol;
	fDoc->fRefsHdr->AtGet(id, discIDhol);
	if (discIDhol.fLength)
	{
		Ptr p = *discIDhol.fH + discIDhol.fOffset;
		while (discIDhol.fLength && *p != '<')
		{
			++p;
			++discIDhol.fOffset;
			--discIDhol.fLength;
		}
#if qDebug
		if (!discIDhol.fLength)
		{
			fprintf(stderr, "Avoids bogous References header:");
			HandleOffsetLength hol;
			fDoc->fRefsHdr->AtGet(id, hol);
			CStr255 s;
			CopyHolToCStr255(hol, s);
			fprintf(stderr, "   '%s'\n", (char*)s);
		}
#endif
	}
	if (discIDhol.fLength == 0)
	{
		fDoc->fMsgIDHdr->AtGet(id, discIDhol);
		if (discIDhol.fLength == 0)
			return; // non-existent article
		Ptr p = *discIDhol.fH + discIDhol.fOffset;
		while (discIDhol.fLength && *p != '<')
		{
			++p;
			++discIDhol.fOffset;
			--discIDhol.fLength;
		}
		if (!discIDhol.fLength)
		{
#if qDebug
			fprintf(stderr, "Avoids bogous Message-id header:");
			HandleOffsetLength hol;
			fDoc->fRefsHdr->AtGet(id, hol);
			CStr255 s;
			CopyHolToCStr255(hol, s);
			fprintf(stderr, "   '%s'\n", (char*)s);
#endif
			return;
		}
	}

	HandleOffsetLength nameHol;
	fDoc->fSubjectHdr->AtGet(id, nameHol);
	long discIndex = fDoc->fDiscList->CreateDiscussion(id, discIDhol, nameHol);
}
	
void TOpenGroupCommand::DistributeArticle(long id)
{
#if qDebug & 0
	fprintf(stderr, "id = %ld\n", id);
#endif
	HandleOffsetLength msgIDHol;
	fDoc->fMsgIDHdr->AtGet(id, msgIDHol);
	if (msgIDHol.fLength == 0)
		return; // non-existent article!

	HandleOffsetLength refsHol;
	fDoc->fRefsHdr->AtGet(id, refsHol);

	ArrayIndex discIndex;
	
	if (refsHol.fLength && fDoc->fDiscList->TryFindDiscussion(refsHol, discIndex))
	{
		fDoc->fDiscList->AddArticle(discIndex, id);
		return;
	}
	if (fDoc->fDiscList->TryFindDiscussion(msgIDHol, discIndex))
	{
		HandleOffsetLength nameHol;
		fDoc->fSubjectHdr->AtGet(id, nameHol);
		fDoc->fDiscList->AddArticleAsOriginator(discIndex, id, msgIDHol, nameHol);
		return;
	}
	CreateNewDiscussion(id);
#if qDebugDiscListCreateNewDiscussion
	if (!fDoc->fDiscList->SanityCheck())
		Failure(errDatabaseScrambled, 0);
#endif
}

void TOpenGroupCommand::DistributeNewArticles()
{
	gCurProgress->SetText(kOpenGroupUpdatesDatabase);
	gCurThread->CheckYield();
	short delta = 0;
	for (long id = fFirstNewID; id <= fLastID; id++) 
	{
		DistributeArticle(id);
		++delta;
		if (delta == 5)
		{
			gCurProgress->Worked(delta);
			delta = 0;
		}
		gCurThread->CheckYield();
	}
	if (delta)
		gCurProgress->Worked(delta);
#if qDebug
	gCurProgress->SetText("Sanity check after update");
	if (!fDoc->fDiscList->SanityCheck())
		Failure(errDatabaseScrambled, 0);
#endif
}

void TOpenGroupCommand::UpdateDiscussions()
{
	CStr255 dotName(fGroupDotName);
#if qDebug
	fprintf(stderr, "TOpenGroupCommand: Updates group '%s'\n", (char*)dotName);
	if (!IsObject(fDoc))
		ProgramBreak("fDoc is not object");
#endif
	FailInfo fi;
	if (fi.Try())
	{
		fNntp = gNntpCache->GetNntp();
		if (StartUpdate())
		{
			gCurProgress->SetText(kOpenGroupDeletesOld);
			fDoc->fDiscList->DeleteOldStuff(fFirstID, fLastID); //?? maybe even if no new stuff??
			UpdateHeaderLists();
			DistributeNewArticles();
			fDoc->Changed(cDatabaseUpdated, this);
			gCurProgress->SetText(kOpenGroupSavesDatabase);
			if (!fSaveIfNonexisting)
			{
				fDoc->fSaveDatabase = fGotDBFile;
				fDoc->fSaveArticleStatus = fGotStatFile;
			}
			fDoc->SaveDocument(cSave);
			gNewsAppl->GetGroupTree()->SetLastUpdatedArticleID(dotName, fLastID);
		}
		fDoc->fMsgIDHdr->DeleteAll(); // not needed anymore
		fDoc->fRefsHdr->DeleteAll();
		gNntpCache->ReturnNntp(fNntp); fNntp = nil;
		fi.Success();
	}
	else // fail
	{
		gNntpCache->DiscardNntp(fNntp);
		fNntp = nil;
		gErrorParm3 = fGroupDotName;
		FailNewMessage(fi.error, fi.message, messageUpdateGroupFailed);
	}
}
