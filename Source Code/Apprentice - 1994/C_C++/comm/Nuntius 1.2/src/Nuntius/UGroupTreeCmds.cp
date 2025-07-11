// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupTreeCmds.cp

#include "UGroupTreeCmds.h"
#include "UGroupTreeView.h"
#include "UNewsAppl.h"
#include "UGroupTreeDoc.h"
#include "UGroupTree.h"
#include "UGroupList.h"
#include "UProgress.h"

#include <RsrcGlobals.h>

#pragma segment MyGroupList

#define qDebugTreeKey qDebug & 0
#define qDebugTreeKeyVerbose qDebugTreeKey

//========================================================================
TExpandGroupTracker::TExpandGroupTracker()
{
}


pascal void TExpandGroupTracker::Initialize()
{
	inherited::Initialize();
	fGroupTreeView = nil;
	fWayDown = false;
	fLastHighlighted = false;
}

pascal void TExpandGroupTracker::IExpandGroupTracker(
												TGroupTreeView *groupTreeView,
												const VPoint &itsMouse,
												GridCell cell,
												Boolean isExpanded,
												Boolean wayDown,
												const Rect &cellRect)
{
	this->ITracker(cExpandGroup, nil, kCantUndo, kDoesNotCauseChange, NULL, 
					groupTreeView, nil, itsMouse);
	fGroupTreeView = groupTreeView;
	fCell = cell;
	fWasExpanded = isExpanded;
	fCellRect = cellRect;
	fWayDown = wayDown;
	fLastHighlighted = false;
}

pascal void TExpandGroupTracker::DoIt()
{
	fGroupTreeView->ExpandCompactCell(fCell, !fWasExpanded, fWayDown);
}

pascal void TExpandGroupTracker::TrackFeedback(TrackPhase /* aTrackPhase */,
										const VPoint& /* anchorPoint */,
										const VPoint& /* previousPoint */,
										const VPoint& /* nextPoint */,
										Boolean /* mouseDidMove */,
										Boolean /* turnItOn */)
{
}

pascal TTracker *TExpandGroupTracker::TrackMouse(TrackPhase aTrackPhase,
											 VPoint& /* anchorPoint */,
											 VPoint& /* previousPoint */,
											 VPoint &nextPoint,
											 Boolean mouseDidMove) // override 
{
	if (!mouseDidMove && aTrackPhase != trackPress)
		return this;
	fGroupTreeView->Focus();

	Point mouse(fGroupTreeView->ViewToQDPt(nextPoint));
	Boolean inside = fCellRect.Contains(mouse);
	if (mouse.h < fCellRect.left || mouse.h > fCellRect.left + 14)
		inside = false;
	if (inside == fLastHighlighted && aTrackPhase != trackRelease)
		return this; // no change
	fLastHighlighted = inside;

	fGroupTreeView->DrawTrackedTriangle(fCell, fWasExpanded, false, inside);

	if (aTrackPhase != trackRelease)
		return this;

	if (!inside)
		return nil; // outside, don't do anything
	fGroupTreeView->DrawTrackedTriangle(fCell, false, true, true);
	long dl;
	Delay(2, dl); // from hacking Finder�
	fGroupTreeView->DrawTrackedTriangle(fCell, !fWasExpanded, false, true);
	return this;
}

//========================================================================
TSubscribeGroupTracker::TSubscribeGroupTracker()
{
}


pascal void TSubscribeGroupTracker::Initialize()
{
	inherited::Initialize();
	fGroupTreeView = nil;
}

pascal void TSubscribeGroupTracker::ISubscribeGroupTracker(
												TGroupTreeView *groupTreeView,
												const VPoint &localMouse,
												Boolean makingCopy)
{

#if qDebug
	if (!IsObject(groupTreeView))
		ProgramBreak("groupTreeView is not object");
#endif
	groupTreeView->Focus();
	CPoint globalMouse(localMouse.ToPoint());
	LocalToGlobal(globalMouse);

#if qDebugGroupDrag
	fprintf(stderr, "ISubscribeGroupTracker, localMouse = %s,", (char*)localMouse);
	fprintf(stderr, "globalMouse = %s\n", (char*)globalMouse);
#endif
	IGroupTracker(cGroupListChange, globalMouse, makingCopy);
	fGroupTreeView = groupTreeView;
	
	fMouseOffsetInGrayPict = localMouse.ToPoint();
}

pascal void TSubscribeGroupTracker::Free()
{
	inherited::Free();
}


void TSubscribeGroupTracker::MakeGrayPict()
{
	fGroupTreeView->Focus();	
	ClipRect(CRect(0, 0, 1000, 1000));
	fGrayPictH = OpenPicture(CRect(0, 0, 1000, 1000));
	PenNormal();
	PenPat(&qd.gray);
	PenMode(patXor);
	if (!fGroupTreeView->IsAnyCellSelected())
	{
#if qDebugGroupDrag
		fprintf(stderr, "No cell selected\n");
#endif
		ClosePicture();
		return;
	}

	fHasPrevCell = false;
	
	ArrayIndex firstLine = fGroupTreeView->FirstSelectedCell().v;
	ArrayIndex lastLine = fGroupTreeView->LastSelectedCell().v;
	CPoint offset;
	Boolean gotOffset = false;
#if qDebugGroupDrag
	fprintf(stderr, "firstLine = %ld, lastLine = %ld\n", firstLine, lastLine);
#endif
	ArrayIndex line = firstLine;
	while (line <= lastLine)
	{
		if (!fGroupTreeView->IsCellSelected(GridCell(1, short(line))))
		{
#if qDebugGroupDrag
			fprintf(stderr, "Skips line %ld as it's not selected\n", line);
#endif
			line++;
			continue;
		}
		ArrayIndex noSubLines = 1;
		Boolean atFolder = true;
		while (noSubLines)
		{
			CStr255 text;
			long level, noVisibleSubLines;
			Boolean isFolder;
			CRect triangleRect, iconRect, textRect;
			
			fGroupTreeView->GetCellInformation(GridCell(1, short(line)), level, noVisibleSubLines, isFolder, triangleRect, iconRect, textRect, text);
			iconRect.Inset(CPoint(1, 1));
			textRect.Inset(CPoint(0, 1));
			
			if (!gotOffset)
			{
				offset = CPoint(40, 40) - iconRect[topLeft];
				gotOffset = true;
				fMouseOffsetInGrayPict += offset;
			}
			iconRect += offset;
			textRect += offset;
			DrawGroupIcon(line, level, textRect, iconRect);
			noSubLines -= 1;
			if (atFolder)
			{
				noSubLines += noVisibleSubLines;
				atFolder = false;
			}
			line++;
		}
	}		
	if (fHasPrevCell)
		FrameRect(CRect(fPrevIconRect)); // flush it
	ClosePicture();
}

void TSubscribeGroupTracker::DrawGroupIcon(ArrayIndex line, long level, CRect textRect, CRect iconRect)
{
	if (fHasPrevCell)
	{
		if (fPrevIndex == line - 1 && fPrevLevel == level)
		{
			fPrevIconRect.bottom = iconRect.bottom;
			fPrevIndex++;
		}
		else
		{
			FrameRect(CRect(fPrevIconRect)); // flush it
			fHasPrevCell = false;
		}
	}
	if (!fHasPrevCell)
	{
		fPrevIconRect = iconRect;
		fHasPrevCell = true;
		fPrevLevel = level;
		fPrevIndex = line;
	}
	FrameRect(textRect);
}

void TSubscribeGroupTracker::CreateListOfDraggedGroups()
{
	DoFocus();
	FailInfo fi;
	if (fi.Try())
	{
		TGroupList *gl = new TGroupList();
		gl->IGroupList(nil);
		fDraggedGroups = gl;
		
		if (fGroupTreeView->IsAnyCellSelected())
		{
			ArrayIndex firstLine = fGroupTreeView->FirstSelectedCell().v;
			ArrayIndex lastLine = fGroupTreeView->LastSelectedCell().v;
			ArrayIndex line = firstLine;
			TGroupTree *tree = gNewsAppl->GetGroupTreeDoc()->GetGroupTree();
			while (line <= lastLine)
			{
				if (!fGroupTreeView->IsCellSelected(GridCell(1, short(line))))
				{
					line++;
					continue;
				}
				CStr255 text;
				long level, noVisibleSubLines;
				Boolean isFolder;
				CRect triangleRect, iconRect, textRect;
				
				fGroupTreeView->GetCellInformation(GridCell(1, short(line)), level, noVisibleSubLines, isFolder, triangleRect, iconRect, textRect, text);

				ArrayIndex treeIndex = tree->FindSubGroupIndexFromLine(fGroupTreeView->fExpandData, fGroupTreeView->fWindowFolderIndex, line);
				CRealGroupIterator iter(tree, treeIndex);
				for (ArrayIndex index = iter.FirstGroup(); iter.More(); index = iter.NextGroup())
				{
					if (index == kEmptyIndex)
						continue;
					CStr255 name;
					tree->GetDotNameFromNodeIndex(index, name);
					fDraggedGroups->InsertGroupBefore(fDraggedGroups->fSize + 1, name);
				}
				line += noVisibleSubLines;
				line++;
			}
		}
		fi.Success();
	}
	else // fail
	{
		fi.ReSignal();
	}
}


//========================================================================
TTreeTypeName::TTreeTypeName()
{
}


pascal void TTreeTypeName::Initialize()
{
	inherited::Initialize();
	fExpandData = nil;
	fWindowFolderIndex = 0;
	fGroupTree = nil;
	fGroupTreeView = nil;
}

void TTreeTypeName::ITreeTypeName(TGroupTreeView *gtv, TLongintList *expandData, ArrayIndex windowFolderIndex, TToolboxEvent *event)
{
	inherited::IGroupViewTypeNameCommand(gtv, event);
	fGroupTreeView = gtv;
	fGroupTree = fGroupTreeView->GetGroupTree();
	fExpandData = expandData;
	fWindowFolderIndex = windowFolderIndex;
#if qDebug
	if (!IsObject(fExpandData))
		ProgramBreak("fExpandData is not object");
	if (fWindowFolderIndex < 1 || fWindowFolderIndex > fGroupTree->GetSize())
		ProgramBreak("Invalid fWindowFolderIndex");
	if (!IsObject(fGroupTreeView))
		ProgramBreak("fGroupTreeView is not object");
	if (!IsObject(fGroupTree))
		ProgramBreak("fGroupTree is not object");
#endif
}

pascal void TTreeTypeName::Free()
{
	inherited::Free();
}

void TTreeTypeName::GetLineText(ArrayIndex line, CStr255 &text)
{
	ArrayIndex index = fGroupTree->FindSubGroupIndexFromLine(fExpandData, fWindowFolderIndex, line);
	long level;
	Boolean isFolder;
	fGroupTree->GetDrawInfo(index, level, text, isFolder);
#if qDebugTreeKeyVerbose & 0
	fprintf(stderr, "TTreeKeyCommand: line = %ld, index = %ld, text = '%s', level = %ld, isFolder = %hd\n",
		line, index, (char*)text, level, isFolder);
#endif
}

void TTreeTypeName::SetUp()
{
	fLastTick = fGroupTreeView->fLastTypeKeyTick;
	fTypeChars = fGroupTreeView->fKeyTypeChars;
	inherited::SetUp();
}

void TTreeTypeName::SetDown()
{
	inherited::SetDown();
	fGroupTreeView->fLastTypeKeyTick = fLastTick;
	fGroupTreeView->fKeyTypeChars = fTypeChars;
}

//========================================================================
TTreeTabKeyCommand::TTreeTabKeyCommand()
{
}


pascal void TTreeTabKeyCommand::Initialize()
{
	inherited::Initialize();
	fFirstLastLine = kEmptyIndex;
}

void TTreeTabKeyCommand::ITreeTabKeyCommand(TGroupTreeView *gtv, TLongintList *expandData, ArrayIndex windowFolderIndex, Boolean forward)
{
	inherited::IGroupViewKeyCommand(gtv);
	fGroupTreeView = gtv;
	fGroupTree = fGroupTreeView->GetGroupTree();
	fExpandData = expandData;
	fWindowFolderIndex = windowFolderIndex;
#if qDebug
	if (!IsObject(fExpandData))
		ProgramBreak("fExpandData is not object");
	if (fWindowFolderIndex < 1 || fWindowFolderIndex > fGroupTree->GetSize())
		ProgramBreak("Invalid fWindowFolderIndex");
	if (!IsObject(fGroupTreeView))
		ProgramBreak("fGroupTreeView is not object");
	if (!IsObject(fGroupTree))
		ProgramBreak("fGroupTree is not object");
#endif
	fForward = forward;
}

pascal void TTreeTabKeyCommand::Free()
{
	inherited::Free();
}

void TTreeTabKeyCommand::GetLineText(ArrayIndex line, CStr255 &text)
{
	ArrayIndex index = fGroupTree->FindSubGroupIndexFromLine(fExpandData, fWindowFolderIndex, line);
	long level;
	Boolean isFolder;
	fGroupTree->GetDrawInfo(index, level, text, isFolder);
#if qDebugTreeKeyVerbose & 0
	fprintf(stderr, "TTreeKeyCommand: line = %ld, index = %ld, text = '%s', level = %ld, isFolder = %hd\n",
		line, index, (char*)text, level, isFolder);
#endif
}

//========================================================================
pascal void TUpdateListOfAllGroupsCommand::Initialize()
{
	inherited::Initialize();
	fGroupTreeDoc = nil;
}

void TUpdateListOfAllGroupsCommand::IUpdateListOfAllGroupsCommand(TGroupTreeDoc *gtd, Boolean rebuild)
{
	inherited::ICommand(cUpdateGroupTree, nil, false, false, nil);
	fGroupTreeDoc = gtd;
	fRebuild = rebuild;
#if qDebug
	if (!IsObject(fGroupTreeDoc))
		ProgramBreak("fGroupTreeDoc is not object");
#endif
}

pascal void TUpdateListOfAllGroupsCommand::Free()
{
	fGroupTreeDoc = nil;
	inherited::Free();
}

pascal void TUpdateListOfAllGroupsCommand::DoIt()
{
	FailInfo fi;
	if (fi.Try())
	{
		fGroupTreeDoc->CloseAllWindows();
		gCurProgress->SetProgressType(true, false);
		gCurProgress->SetTitle(kUpdGTProgressTitle);
		if (fRebuild)
			fGroupTreeDoc->RebuildGroupTree();
		else
			fGroupTreeDoc->UpdateGroupTree();
		gCurProgress->WorkDone();
		fi.Success();
	}
	else // fail
	{
		gCurProgress->WorkDone();
		fi.ReSignal();
	}
}
