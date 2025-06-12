// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupListCmds.cp

#include "UGroupListCmds.h"
#include "UGroupListView.h"
#include "UGroupList.h"
#include "UGroupListDoc.h"
#include "UGroupTree.h"
#include "UNewsAppl.h"
#include "UGroupDocCmds.h"
#include "UNntp.h"
#include "UProgress.h"
#include "UPrefsDatabase.h"
#include "UFatalError.h"
#include "FileTools.h"
#include "UThread.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>

#include <Errors.h>

#pragma segment MyGroupList

#define qDebugListTypeName qDebug & 0
#define qDebugNewArticlesCheck qDebug

//=========================================================================
TUnsubscribeGroupTracker::TUnsubscribeGroupTracker()
{
}


pascal void TUnsubscribeGroupTracker::Initialize()
{
	inherited::Initialize();
	fGroupListView = nil;
}

pascal void TUnsubscribeGroupTracker::IUnsubscribeGroupTracker(
												TGroupListView *groupListView,
												const VPoint &localMouse, Boolean makingCopy)
{
#if qDebug
	if (!IsObject(groupListView))
		ProgramBreak("groupListView is not object");
#endif
	groupListView->Focus();
	CPoint globalMouse(localMouse.ToPoint());
	LocalToGlobal(globalMouse);

#if qDebugGroupDrag
	fprintf(stderr, "IUnsubscribeGroupTracker, localMouse = %s,", (char*)localMouse);
	fprintf(stderr, "globalMouse = %s\n", (char*)globalMouse);
#endif
	IGroupTracker(cGroupListChange, globalMouse, makingCopy);
	fGroupListView = groupListView;
	
	fMouseOffsetInGrayPict = localMouse.ToPoint();
}

pascal void TUnsubscribeGroupTracker::Free()
{
	inherited::Free();
}

void TUnsubscribeGroupTracker::GetBeforeGroupName(TGroupListView *groupListView, short beforeRow, CStr255 &name)
{
	if (groupListView == fGroupListView)
	{
		while (beforeRow <= fGroupListView->fNumOfRows && fGroupListView->IsCellSelected(GridCell(1, beforeRow)))
			beforeRow++; // skip moved groups
	}
	inherited::GetBeforeGroupName(groupListView, beforeRow, name);
}

void TUnsubscribeGroupTracker::MakeGrayPict()
{
	fGroupListView->Focus();	
	ClipRect(CRect(0, 0, 1000, 1000));
	fGrayPictH = OpenPicture(CRect(0, 0, 1000, 1000));
	PenNormal();
	PenPat(&qd.gray);
	PenMode(patXor);

	CSelectedCellIterator iter(fGroupListView);
	for (GridCell cell = iter.FirstCell(); iter.More(); cell = iter.NextCell())
	{
		CRect r;
		fGroupListView->GetCellTextRect(cell, r);
		FrameRect(r);
	}
	ClosePicture();
}

void TUnsubscribeGroupTracker::CreateListOfDraggedGroups()
{
	fDraggedGroups = fGroupListView->GetSelectionAsList();
}

void TUnsubscribeGroupTracker::CheckForExistingGroups(TGroupListView *hitGroupListView, Boolean makingCopy)
{
	if (hitGroupListView != fGroupListView)
		inherited::CheckForExistingGroups(hitGroupListView, makingCopy);
	if (fGroupListView && !makingCopy)
		fGroupListView->DeleteSelection();
}
//=========================================================================
TListTypeName::TListTypeName()
{
}


pascal void TListTypeName::Initialize()
{
	inherited::Initialize();
	fGroupList = nil;
	fGroupListView = nil;
}

void TListTypeName::IListTypeName(TGroupListView *glv, TToolboxEvent *event)
{
	inherited::IGroupViewTypeNameCommand(glv, event);
	fGroupListView = glv;
	fGroupList = fGroupListView->GetGroupList();
#if qDebug
	if (!IsObject(fGroupListView))
		ProgramBreak("fGroupListView is not object");
	if (!IsObject(fGroupList))
		ProgramBreak("fGroupList is not object");
#endif
}

pascal void TListTypeName::Free()
{
	inherited::Free();
}

void TListTypeName::GetLineText(ArrayIndex line, CStr255 &text)
{
	fGroupList->GetGroupAt(line, text);
}

void TListTypeName::SetUp()
{
	fLastTick = fGroupListView->fLastTypeKeyTick;
	fTypeChars = fGroupListView->fKeyTypeChars;
	inherited::SetUp();
}

void TListTypeName::SetDown()
{
	inherited::SetDown();
	fGroupListView->fLastTypeKeyTick = fLastTick;
	fGroupListView->fKeyTypeChars = fTypeChars;
}

void TListTypeName::DoOneLine(ArrayIndex line)
{
	// split dot name out in parts, and call DoLine on each part
	CStr255 text;
	GetLineText(line, text);
#if qDebugListTypeName
	fprintf(stderr, "Got dot line: '%s'\n", (char*)text);
#endif
	while (true)
	{
		short pos = text.Pos(".");
		if (!pos || pos > text.Length())
			break;
		CStr255 s(text.Copy(1, pos - 1));
		if (s.Length())
			DoLine(line, s);
#if qDebugListTypeName
		fprintf(stderr, "Examines '%s'\n", (char*)s);
#endif
		text.Delete(1, pos);
	}
	if (text.Length())
		DoLine(line, text);
#if qDebugListTypeName
	fprintf(stderr, "Examines '%s'\n", (char*)text);
#endif
}
//------------------------------------------------------
TListTabKeyCommand::TListTabKeyCommand()
{
}

pascal void TListTabKeyCommand::Initialize()
{
	inherited::Initialize();
	fGroupList = nil;
	fGroupListView = nil;
}

void TListTabKeyCommand::IListTabKeyCommand(TGroupListView *glv, Boolean forward)
{
	inherited::IGroupViewTabKeyCommand(glv, forward);
	fGroupListView = glv;
	fGroupList = fGroupListView->GetGroupList();
#if qDebug
	if (!IsObject(fGroupListView))
		ProgramBreak("fGroupListView is not object");
	if (!IsObject(fGroupList))
		ProgramBreak("fGroupList is not object");
#endif
}

pascal void TListTabKeyCommand::Free()
{
	inherited::Free();
}

void TListTabKeyCommand::GetLineText(ArrayIndex line, CStr255 &text)
{
	fGroupList->GetGroupAt(line, text);
}

//=============================================================================
TPeriodicCheckNewArticles::TPeriodicCheckNewArticles()
{
}

pascal void TPeriodicCheckNewArticles::Initialize()
{
	inherited::Initialize();
	fGroupListDoc = nil;
	fGroupList = nil;
	fNntp = nil;
}

void TPeriodicCheckNewArticles::IPeriodicCheckNewArticles(TGroupListDoc *doc)
{
	inherited::IPeriodicAction(true);
	fGroupListDoc = doc;
	fGroupList = fGroupListDoc->GetGroupList();
}

pascal void TPeriodicCheckNewArticles::Free()
{
	GoSleep();
	gNntpCache->ReturnNntp(fNntp); fNntp = nil;
	fGroupListDoc = nil;
	fGroupList = nil;
	inherited::Free();
}

void TPeriodicCheckNewArticles::DoPeriodic()
{
	InvalidateMenus();
	FailSpaceIsLow();
	if (!fGroupListDoc)
	{
#if qDebug
		ProgramBreak("fGroupListDoc is nil");
#endif
		return;
	}
	FailInfo fi;
	if (fi.Try())
	{
		DoTheCheck();
		InvalidateMenus();
		fi.Success();
	}
	else // fail
	{
		gNntpCache->DiscardNntp(fNntp); fNntp = nil;
		gCurProgress->WorkDone();
		InvalidateMenus();
		fi.ReSignal();
	}
}

const char *TPeriodicCheckNewArticles::GetDebugDescription()
{
	return "TPeriodicCheckNewArticles";
}
	
void TPeriodicCheckNewArticles::DoTheCheck()
{
// may not cache TGroupTree as it can be replaced when rebuilding/updating it
	fNntp = gNntpCache->GetNntp();
	for (ArrayIndex index = 1;index <= fGroupList->GetSize(); index++)
	{
		gCurThread->CheckYield();
		CStr255 dotName;
		fGroupList->GetGroupAt(index, dotName);
		if (!gNewsAppl->GetGroupTree()->HasGroup(dotName))
			continue;
		FailSpaceIsLow();
		long lastID;
		if (!GetCurrentLastID(dotName, lastID))
			continue;
		if (lastID <= gNewsAppl->GetGroupTree()->GetLastReadArticleID(dotName))
			fGroupListDoc->UpdateGroupStatus(dotName, false);
		else
			fGroupListDoc->UpdateGroupStatus(dotName, true);
	}	
	if (gPrefs->GetBooleanPrefs('AuUp'))
		UpdateGroupDatabases();
	gCurProgress->WorkDone();
	gNntpCache->ReturnNntp(fNntp); fNntp = nil;
}

void TPeriodicCheckNewArticles::UpdateGroupDatabases()
{
	for (ArrayIndex index = 1;index <= fGroupList->GetSize(); index++)
	{
		gCurThread->CheckYield();
		CStr255 dotName;
		fGroupList->GetGroupAt(index, dotName);
		if (!gNewsAppl->GetGroupTree()->HasGroup(dotName))
			continue;
		long lastID;
		if (!GetCurrentLastID(dotName, lastID))
			continue;
		FailSpaceIsLow();
		FSSpec spec;
		GoPublicFile(dotName, spec);
		if (FileExist(spec) && lastID <= gNewsAppl->GetGroupTree()->GetLastUpdatedArticleID(dotName))
			continue;
		if (!UpdateGroup(dotName))
			return;
	}	
}

Boolean TPeriodicCheckNewArticles::GetCurrentLastID(const CStr255 &dotName, long &lastID)
{
	FailInfo fi;
	if (fi.Try())
	{
		fNntp->SetGroup(dotName, true);
		gCurThread->CheckYield();
		long firstID;
		fNntp->GetGroupInfo(firstID, lastID);
		if (lastID < firstID) // got no articles == no new articles
			lastID = 0;
		fi.Success();
		return true;
	}
	else // fail
	{
		// don't blow this thread up, if we meet an unexisting group,
		// just return "no new articles"
		if (fi.error == errNntpBadGroup)
			return false;
		fi.ReSignal();
	}
}

Boolean TPeriodicCheckNewArticles::UpdateGroup(const CStr255 &dotName)
{
	TOpenGroupCommand *cmd = nil;
	VOLATILE(cmd);
	FailInfo fi;
	if (fi.Try())
	{
		gCurProgress->SetStandardProgressType();
		cmd = new TOpenGroupCommand();
		cmd->IOpenGroupCommand(dotName, true, true, true);
		cmd->DoIt();
		cmd->Free(); cmd = nil;
		fi.Success();
		return true;
	}
	else // fail
	{
		FreeIfObject(cmd); cmd = nil;
		switch (fi.error)
		{
			case noErr:
				return false;  // don't do anything, just try checking the next groups
			
			case memFullErr:
				return true; // don't do anything, just try checking the next groups
			
			default:
				fi.ReSignal();
		}
	}
}
