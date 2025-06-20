// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupTreeView.cp

#include "UGroupTreeView.h"
#include "UGroupViewCmds.h"
#include "UGroupTreeCmds.h"
#include "UGroupDocCmds.h"
#include "UDiscListView.h"
#include "UDynDynArray.h"
#include "UGroupList.h"
#include "UPrefsDatabase.h"
#include "UNewsAppl.h"
#include "UGroupTreeDoc.h"
#include "UGroupTree.h"
#include "Tools.h"
#include "StreamTools.h"
#include "UThread.h"
#include "ViewTools.h"

#include <RsrcGlobals.h>

#include <Resources.h>
#include <Packages.h>

#pragma segment MyGroupList
// as netnews folders can't be empty, fExpandData has zero items for compacted folders

#define qDebugExpand qDebug & 0
#define qDebugBalloons qDebug & 0
#define qDebugGroupTreeView qDebug & 0

//************************************************************************************************
const kIconSize = 16;

TreeSicn gNonExpandedDiamondSicn, gHalfExpandedDiamondSicn, gExpandedDiamondSicn;
TreeSicn gDocumentSicn, gFolderSicn;
Boolean gHasLoadedSicn = false;

void InitTreeSicn()
{
	InitializeSicnFastBits(gNonExpandedDiamondSicn[0]);
	InitializeSicnFastBits(gNonExpandedDiamondSicn[1]);
	InitializeSicnFastBits(gHalfExpandedDiamondSicn[0]);
	InitializeSicnFastBits(gHalfExpandedDiamondSicn[1]);
	InitializeSicnFastBits(gExpandedDiamondSicn[0]);
	InitializeSicnFastBits(gExpandedDiamondSicn[1]);
	InitializeSicnFastBits(gDocumentSicn[0]);
	InitializeSicnFastBits(gDocumentSicn[1]);
	InitializeSicnFastBits(gFolderSicn[0]);
	InitializeSicnFastBits(gFolderSicn[1]);
	GetNewSicnFastBits(gNonExpandedDiamondSicn[0], kNonExpandedDiamondSicn);
	GetNewSicnFastBits(gNonExpandedDiamondSicn[1], kNonExpandedDiamondSicn + 1);
	GetNewSicnFastBits(gHalfExpandedDiamondSicn[0], kHalfExpandedDiamondSicn);
	GetNewSicnFastBits(gHalfExpandedDiamondSicn[1], kHalfExpandedDiamondSicn + 1);
	GetNewSicnFastBits(gExpandedDiamondSicn[0], kExpandedDiamondSicn);
	GetNewSicnFastBits(gExpandedDiamondSicn[1], kExpandedDiamondSicn + 1);
	GetNewSicnFastBits(gDocumentSicn[0], kDocumentSicn);
	GetNewSicnFastBits(gDocumentSicn[1], kDocumentSicn + 1);
	GetNewSicnFastBits(gFolderSicn[0], kFolderSicn);
	GetNewSicnFastBits(gFolderSicn[1], kFolderSicn + 1);
	gHasLoadedSicn = true;
}
//-----------------------------------------------------------------------
TGroupTreeView::TGroupTreeView()
{
}

pascal void TGroupTreeView::Initialize()
{
	inherited::Initialize();
	fWindow = nil;
	fGroupTreeDoc = nil;
	fGroupTree = nil;
	fScreenDepth = 1;
	fPortBitsP = nil;
	fExpandData = nil;
	fWindowFolderIndex = kFirstIndex;
	fLevelOffset = 1;
	fLastTypeKeyTick = 0;
	fKeyTypeChars = "";
}

pascal void TGroupTreeView::Free()
{
	inherited::Free();
}

pascal void TGroupTreeView::ReadFields(TStream *aStream)
{
	inherited::ReadFields(aStream);	
}

pascal void TGroupTreeView::DoPostCreate(TDocument *itsDocument)
{
	inherited::DoPostCreate(itsDocument);
	
	if (!gHasLoadedSicn)
		InitTreeSicn();
	
	fGroupTreeDoc = (TGroupTreeDoc*) itsDocument;
	fGroupTree = fGroupTreeDoc->GetGroupTree();
	TLongintList *lList = new TLongintList();
	lList->ILongintList();
	fExpandData = lList;
	long size = fGroupTree->GetSize();
	fExpandData->SetArraySize(size);
	fExpandData->fSize = size;
	BlockSet(fExpandData->ComputeAddress(1), size * sizeof(long), 0);
	
	fWindow = GetWindow();
	
	fPortBitsP = &(fWindow->GetGrafPort()->portBits);

	StandardGridViewTextStyle gvts;
	gPrefs->GetTextStylePrefs('TSgt', gvts.fTextStyle);
	CalcStandardGridViewFont(gvts);
	fGridViewTextStyle = gvts;
	fCellHeight = short(Max(kIconSize, fGridViewTextStyle.fRowHeight));
}

pascal void TGroupTreeView::Close()
{
	VRect frame;
	fWindow->GetFrame(frame);
	fGroupTree->SetWindowFrame(fWindowFolderIndex, frame.ToRect());
	fGroupTreeDoc->ForgetGroupTreeView(this);
	inherited::Close();
}
//------------------------------------------------------------------
pascal void TGroupTreeView::ShowReverted()
{
	SetWindowFolder(fWindowFolderIndex); // maybe need a UpdateList in future
}

void TGroupTreeView::SetWindowFolder(ArrayIndex windowFolderIndex, TGroupTreeView *openFromView)
{
	fWindowFolderIndex = windowFolderIndex;
	CStr255 title;
	long level;
	Boolean isFolder;
	fGroupTree->GetDrawInfo(fWindowFolderIndex, level, title, isFolder);
#if qDebug
	if (!isFolder)
		ProgramBreak("fWindowFolderIndex is not folder");
#endif
	fGroupTree->GetDotNameFromNodeIndex(fWindowFolderIndex, title);
	fWindow->SetTitle(title);
	CRect frame = fGroupTree->GetWindowFrame(fWindowFolderIndex);
	fLevelOffset = -level;
	if (fNumOfRows)
		DelRowFirst(fNumOfRows);
	if (frame != CRect(0, 0, 0, 0))
	{
#if qDebug
		fprintf(stderr, "TGroupTreeView::SetWindowFolder, WindowFrame from TGroupTree: %s\n", (char*)frame);
#endif
		fWindow->SetFrame(frame, kRedraw);
	}
	else if (openFromView)
	{
		BytesMove(openFromView->fExpandData->ComputeAddress(1), fExpandData->ComputeAddress(1), fExpandData->GetSize() * sizeof(long));
		VRect frame;
		openFromView->GetWindow()->GetFrame(frame);
		frame += VPoint(kStdStaggerAmount, kStdStaggerAmount);
		fWindow->SetFrame(frame, kRedraw);
	}
	long noLines = fGroupTree->GetNoItems(fExpandData, fWindowFolderIndex);
	InsRowLast(short(noLines), fCellHeight);
#if qDebugGroupTreeView
	fprintf(stderr, "TGroupTreeView::SetWindowFolder, windowFolderIndex = %hd, noLines = %ld, fLevelOffset = %ld\n", 
			windowFolderIndex, noLines, fLevelOffset);
#endif
}

void TGroupTreeView::WriteWindowInfo(TStream *aStream)
{
	WriteDynamicArray(aStream, fExpandData);
	aStream->WriteLong(fWindowFolderIndex);
	VRect frame;
	fWindow->GetFrame(frame);
	aStream->WriteVRect(frame);
}

void TGroupTreeView::ReadWindowInfo(TStream *aStream)
{
	ReadDynamicArray(aStream, fExpandData);
	ArrayIndex windowFolderIndex = aStream->ReadLong();
	SetWindowFolder(windowFolderIndex);
	VRect frame;
	aStream->ReadVRect(frame);
	fWindow->SetFrame(frame, kRedraw);
}


void TGroupTreeView::ChangedFont()
{
	StandardGridViewTextStyle gvts = fGridViewTextStyle;
	gPrefs->SetTextStylePrefs('TSgt', gvts.fTextStyle);
	CalcStandardGridViewFont(gvts);
	fGridViewTextStyle = gvts;
	fCellHeight = short(Max(kIconSize, fGridViewTextStyle.fRowHeight));
	if (fNumOfRows)
		SetRowHeight(1, fNumOfRows, fCellHeight);
	Focus();
	ForceRedraw();
}

void TGroupTreeView::UpdateDrawInfo()
{
	Focus();
	TextStyle itsTextStyle = fGridViewTextStyle.fTextStyle;
	SetPortTextStyle(itsTextStyle);
	fScreenDepth = 1;
	if (gConfiguration.hasColorQD)
	{
		VRect vr(VPoint(0, 0), fWindow->fSize);
		vr += fWindow->fLocation;
		GDHandle dev = GetMaxDevice(vr.ToRect());
		if (dev)
		{
			PixMapHandle pixMap = (**dev).gdPMap;
			fScreenDepth = (**pixMap).pixelSize;
		}
	}
}

pascal void TGroupTreeView::DrawRangeOfCells(GridCell startCell,
										 GridCell stopCell,
										 const VRect & /* aRect */)
{
	UpdateDrawInfo();
	GridCell cell(startCell);
	while (cell.v <= stopCell.v)
	{
		DrawTheCell(cell);
		cell.v++;
	}
}

pascal void TGroupTreeView::DrawCell(GridCell aCell, const VRect & /* aRect */)
{
	UpdateDrawInfo();
	DrawTheCell(aCell);
}

void TGroupTreeView::DrawSmallIcon(const SicnFastBits *sicn, CRect iconRect, Boolean drawSelected)
{
	CRect theRect(iconRect.left, iconRect.top, iconRect.left + kIconSize, iconRect.top + kIconSize);
	if (fScreenDepth == 1)
	{
		// 1-bit bitmap
		DrawSicnFastBits(fPortBitsP, sicn[drawSelected], CRect(0, 0, kIconSize, kIconSize), theRect);
		return;
	}
	if (drawSelected)
		PlotIconID(theRect, 0, ttSelected, sicn[0].fRsrcID);
	else
		PlotIconID(theRect, 0, ttNone, sicn[0].fRsrcID);
}

void TGroupTreeView::DoTheGetCellInformation(GridCell aCell,
							ArrayIndex &nodeIndex, long &level, ArrayIndex &noVisibleSubLines,
							Boolean &isFolder,
							CRect &triangleRect, CRect &iconRect, CRect &textRect,
							CStr255 &text)
{
	VRect vr;
	CellToVRect(aCell, vr);
	CRect r;
	ViewToQDRect(vr, r);	
	nodeIndex = fGroupTree->FindSubGroupIndexFromLine(fExpandData, fWindowFolderIndex, aCell.v);
#if qDebugGroupTreeView
	fprintf(stderr, "line = %hd, wfi = %hd, nodeIndex = %hd\n", long(aCell.v), fWindowFolderIndex, nodeIndex);
#endif
	fGroupTree->GetDrawInfo(nodeIndex, level, text, isFolder);
	level += fLevelOffset;
	noVisibleSubLines = fExpandData->At(nodeIndex);
	short left = r.left + kIconSize + 4 + short(level - 1) * kIconSize;
	short iconTop = (r.top + r.bottom - kIconSize) / 2;
	triangleRect = CRect(r.left, iconTop, r.left + kIconSize, iconTop + kIconSize);
	iconRect = CRect(left, iconTop, left + kIconSize, iconTop + kIconSize);
	left += kIconSize + 2 + fGridViewTextStyle.fHorzOffset / 4;
	r.top += (fCellHeight - fGridViewTextStyle.fRowHeight) / 2;
	textRect = CRect(left, r.top, left + StringWidth(text), r.top + fGridViewTextStyle.fRowHeight);
}

void TGroupTreeView::GetCellInformation(GridCell aCell, 
							long &level, long &noVisibleSubLines,
							Boolean &isFolder,
							CRect &triangleRect, CRect &iconRect, CRect &textRect,
							CStr255 &text)
{
	Focus();
	TextStyle itsTextStyle = fGridViewTextStyle.fTextStyle;
	SetPortTextStyle(itsTextStyle);
	ArrayIndex nodeIndex;
	DoTheGetCellInformation(aCell, nodeIndex, level, noVisibleSubLines, isFolder, triangleRect, iconRect, textRect, text);
}

void TGroupTreeView::DrawTheCell(GridCell aCell)
{
	CStr255 text;
	ArrayIndex level, noVisibleSubLines, nodeIndex;
	Boolean isFolder;
	CRect triangleRect, iconRect, textRect;
	
	DoTheGetCellInformation(aCell, nodeIndex, level, noVisibleSubLines, isFolder, triangleRect, iconRect, textRect, text);

	if (isFolder)
	{
		if (noVisibleSubLines)
			DrawSmallIcon(gExpandedDiamondSicn, triangleRect, false);
		else
			DrawSmallIcon(gNonExpandedDiamondSicn, triangleRect, false);
		DrawSmallIcon(gFolderSicn, iconRect, false);
	}
	else
		DrawSmallIcon(gDocumentSicn, iconRect, false);
	MoveTo(textRect.left, textRect.top + fGridViewTextStyle.fVertOffset - 1);
	DrawString(text);
}

void TGroupTreeView::DrawTrackedTriangle(GridCell aCell, Boolean asExpanded, Boolean halfExpanded, Boolean highlight)
{
	UpdateDrawInfo();
	CStr255 text;
	ArrayIndex level, noVisibleSubLines, nodeIndex;
	Boolean isFolder;
	CRect triangleRect, iconRect, textRect;
	
	DoTheGetCellInformation(aCell, nodeIndex, level, noVisibleSubLines, isFolder, triangleRect, iconRect, textRect, text);
	if (fScreenDepth > 1)
		EraseRect(triangleRect);
	if (isFolder)
	{
		if (halfExpanded)
			DrawSmallIcon(gHalfExpandedDiamondSicn, triangleRect, highlight);
		else if (asExpanded)
			DrawSmallIcon(gExpandedDiamondSicn, triangleRect, highlight);
		else
			DrawSmallIcon(gNonExpandedDiamondSicn, triangleRect, highlight);
	}
}

void TGroupTreeView::HighlighTheCell(GridCell aCell, HLState fromHL, HLState toHL)
{
	CStr255 text;
	ArrayIndex level, noVisibleSubLines, nodeIndex;
	Boolean isFolder;
	CRect triangleRect, iconRect, textRect;
	
	DoTheGetCellInformation(aCell, nodeIndex, level, noVisibleSubLines, isFolder, triangleRect, iconRect, textRect, text);

	if (isFolder)
		DrawSmallIcon(gFolderSicn, iconRect, toHL == hlOn);
	else
		DrawSmallIcon(gDocumentSicn, iconRect, toHL == hlOn);
	UseSelectionColor();
	textRect.Inset(CPoint(-fGridViewTextStyle.fHorzOffset / 4, 1));
	switch (fromHL + toHL)
	{
		case hlOffDim:
			PenMode(patXor);
			FrameRect(textRect);
			break;

		case hlOnDim:
			textRect.Inset(CPoint(1, 1));
			InvertRect(textRect);
			break;

		case hlOffOn:
			InvertRect(textRect);
			break;
	}
}

pascal void TGroupTreeView::HighlightCells(RgnHandle theCells,
									  HLState fromHL,
									  HLState toHL)
{
	if (fromHL == toHL)
		return;
	UpdateDrawInfo();
	switch (fromHL + toHL)
	{
		case hlOffDim:
		case hlOnDim:
		case hlOffOn:
			{
				CCellInRegionIterator iter(this, theCells);
				for (GridCell aCell = iter.FirstCell(); iter.More(); aCell = iter.NextCell())
					HighlighTheCell(aCell, fromHL, toHL);
			}
			break;
	}
}

pascal GridViewPart TGroupTreeView::IdentifyPoint(const VPoint& thePoint,
											 GridCell& aCell)
{
	GridViewPart part = inherited::IdentifyPoint(thePoint, aCell);
	if (part == badChoice)
		return part;
	CStr255 text;
	ArrayIndex level, noVisibleSubLines, nodeIndex;
	Boolean isFolder;
	CRect triangleRect, iconRect, textRect;
	
	DoTheGetCellInformation(aCell, nodeIndex, level, noVisibleSubLines, isFolder, triangleRect, iconRect, textRect, text);
	CPoint pt = ViewToQDPt(thePoint);
	if (triangleRect.Contains(pt) || iconRect.Contains(pt) || textRect.Contains(pt))
		return part;
	else
		return badChoice;	
}

pascal void TGroupTreeView::DoShowHelp(const VPoint& localPoint, RgnHandle helpRegion)
{
#if qDebugBalloons
#define macroBalloonMsg(x) fprintf(stderr, x)
#else
#define macroBalloonMsg(x)
#endif
	Focus();
	GridCell theCell = VPointToCell(localPoint);
	if (theCell == GridCell(0, 0))
	{
		macroBalloonMsg("DoShowHelp: VPointToCell returned GridCell(0, 0)\n");
		return;
	}

	VRect vr;
	CellToVRect(theCell, vr);
	CRect qdCellRect;
	ViewToQDRect(vr, qdCellRect);
	RectRgn(helpRegion, qdCellRect);

	ArrayIndex nodeIndex = fGroupTree->FindSubGroupIndexFromLine(fExpandData, fWindowFolderIndex, theCell.v);
	if (nodeIndex == kEmptyIndex)
	{
		macroBalloonMsg("DoShowHelp: didn't find any groups (node) with for that line\n");
		return;
	}

	CStr255 text;
	ArrayIndex level, noVisibleSubLines;
	Boolean isFolder;
	CRect triangleRect, iconRect, textRect;
	GetCellInformation(theCell, level, noVisibleSubLines, isFolder, triangleRect, iconRect, textRect, text);
	if (localPoint.h < iconRect.left - 3)
	{
		qdCellRect.right = iconRect.left;
		RectRgn(helpRegion, qdCellRect);
		macroBalloonMsg("DoShowHelp: cursor to the left of iconRect\n");
		return;
	}
	if (localPoint.h > textRect.right + 3)
	{
		qdCellRect.left = textRect.right + 3;
		RectRgn(helpRegion, qdCellRect);
		macroBalloonMsg("DoShowHelp: cursor to the right of textRect\n");
		return;
	}
	qdCellRect.left = iconRect.left - 3;
	qdCellRect.right = textRect.right + 3;
	RectRgn(helpRegion, qdCellRect);

	CPoint tip(textRect.right, textRect.top + fGridViewTextStyle.fVertOffset);
	LocalToGlobal(tip);

	CStr255 helpText;
	fGroupTree->GetHelpText(nodeIndex, helpText);
	if (!helpText.Length())
	{
		macroBalloonMsg("DoShowHelp: had no help for this group\n");
		return;
	}
	HMMessageRecord msg;
	msg.hmmHelpType = khmmString;
	BytesMove(&helpText, msg.u.hmmString, helpText.Length() + 1);
	OSErr err = HMShowBalloon(msg, tip, nil, nil, 0, hmDefaultOptions, kHMRegularWindow);
	if (err != hmBalloonAborted)
		FailOSErr(err);
}


//=========================================================================
void TGroupTreeView::UpdateList()
{
	DelRowFirst(fNumOfRows); // slet alt
	short noGroups = short(fGroupTree->GetNoItems(fExpandData, fWindowFolderIndex));
	Focus();
	InsRowLast(short(noGroups), fCellHeight);
	ForceRedraw();
}

ArrayIndex TGroupTreeView::GetWindowFolderIndex()
{
	return fWindowFolderIndex;
}

TGroupTree *TGroupTreeView::GetGroupTree()
{
	return fGroupTree;
}

void TGroupTreeView::OpenSelection(Boolean updateDatabase)
{
	FailSpaceIsLow();
	CSelectedCellIterator iter(this);
	CStr255 text;
	ArrayIndex level, noVisibleSubLines, nodeIndex;
	Boolean isFolder;
	CRect triangleRect, iconRect, textRect;

	short numSelected = 0;
	for (GridCell aCell = iter.FirstCell(); iter.More(); aCell = iter.NextCell())
	{
		DoTheGetCellInformation(aCell, nodeIndex, level, noVisibleSubLines, isFolder, triangleRect, iconRect, textRect, text);
		if (!isFolder)
			numSelected++;
	}
	if (numSelected > kOpenManyGroupsLimit)
	{
		CStr255 num;
		NumToString(numSelected, num);
		ParamText(gEmptyString, gEmptyString, gEmptyString, num);
		if (MacAppAlert(phOpenManyGroups, nil) != ok)
			Failure(0, 0);
	}

	for (aCell = iter.FirstCell(); iter.More(); aCell = iter.NextCell())
	{
		
		DoTheGetCellInformation(aCell, nodeIndex, level, noVisibleSubLines, isFolder, triangleRect, iconRect, textRect, text);

		if (!isFolder)
		{
			fGroupTree->GetDotNameFromNodeIndex(nodeIndex, text);
			TOpenGroupCommand *aCommand = new TOpenGroupCommand();
			aCommand->IOpenGroupCommand(text, updateDatabase, false, false);
			gApplWideThreads->ExecuteCommand(aCommand, "TOpenGroupCommand (TGroupTreeView)");
			continue;
		}
		fGroupTreeDoc->CreateGroupTreeWindow(nodeIndex, this);
	}
}

void TGroupTreeView::DoSuperKey(TToolboxEvent *event)
{
	short row;
	if (IsAnyCellSelected())
		row = FirstSelectedCell().v + 1;
	else
		row = 1;
	if (row > fNumOfRows)
	{
		SetEmptySelection(true);
		return;
	}
	SelectCell(GridCell(1, row), false, true, true);
	ScrollSelectionIntoView(kRedraw);
	OpenSelection(!event->IsOptionKeyPressed());
}

void TGroupTreeView::ExpandCompactCell(GridCell aCell, Boolean doExpand, Boolean wayDown)
{
	long deltaLines;
	long nodeIndex = fGroupTree->FindSubGroupIndexFromLine(fExpandData, fWindowFolderIndex, aCell.v);
	fGroupTree->Expand(fExpandData, doExpand, wayDown, nodeIndex, deltaLines);
	DrawTrackedTriangle(aCell, doExpand, false, false);
	DirectDeltaRows(this, aCell.v + 1, short(deltaLines), fCellHeight);
}

void TGroupTreeView::ExpandCompactSelection(Boolean doExpand, Boolean wayDown)
{
	if (!IsAnyCellSelected())
		return;
	if (doExpand)
	{
		GridCell aCell(FirstSelectedCell());
		do
		{
			if (IsCellSelected(aCell))
				ExpandCompactCell(aCell, doExpand, wayDown);
			aCell.v++;
		} while (aCell.v <= LastSelectedCell().v);
	}
	else
	{
		GridCell aCell(LastSelectedCell());
		do
		{
			if (IsCellSelected(aCell))
				ExpandCompactCell(aCell, doExpand, wayDown);
			aCell.v--;
		} while (aCell.v >= FirstSelectedCell().v);
	}
}

pascal void TGroupTreeView::DoMouseCommand(VPoint &theMouse,
														TToolboxEvent *event, CPoint /* hysteresis */)
{
	GridCell aCell;
	if (IdentifyPoint(theMouse, aCell) == badChoice)
	{
		if (!event->IsShiftKeyPressed())
			SetEmptySelection(kHighlight);
		return;
	}
	CStr255 text;
	ArrayIndex level, noVisibleSubLines, nodeIndex;
	Boolean isFolder;
	CRect triangleRect, iconRect, textRect;
	
	DoTheGetCellInformation(aCell, nodeIndex, level, noVisibleSubLines, isFolder, triangleRect, iconRect, textRect, text);

	if (theMouse.h < 20)
	{
		if (!isFolder)
			return;
		VRect vr;
		CellToVRect(aCell, vr);
		vr.Inset(VPoint(fColInset / 2, fRowInset / 2));
		CRect r;
		ViewToQDRect(vr, r);
		TExpandGroupTracker *tracker = new TExpandGroupTracker();
		tracker->IExpandGroupTracker(this, theMouse, aCell, noVisibleSubLines != 0, event->IsOptionKeyPressed(), r);
		PostCommand(tracker);
		return;
	}
	
	if (event->fClickCount > 1) 
	{
		OpenSelection(!event->IsCommandKeyPressed());
		return;
	}

	Focus();
	if (event->IsShiftKeyPressed())
		SelectCell(aCell, true, true, !IsCellSelected(aCell));
	else if (!IsCellSelected(aCell))
		SelectCell(aCell, false, true, true);

	TSubscribeGroupTracker *tracker = new TSubscribeGroupTracker();
	tracker->ISubscribeGroupTracker(this, theMouse, event->IsOptionKeyPressed());
	PostCommand(tracker);
}

pascal void TGroupTreeView::DoKeyEvent(TToolboxEvent *event)
{
	if (!this->IsEnabled())
	{
		inherited::DoKeyEvent(event);
		return;
	}
	switch (event->fCharacter)
	{
		case chSpace:
			DoSuperKey(event);
			break;
		
		case chEnter:
		case chReturn:
			OpenSelection(true);
			break;

		case chTab:
			{
				TTreeTabKeyCommand *cmd = new TTreeTabKeyCommand();
				cmd->ITreeTabKeyCommand(this, fExpandData, fWindowFolderIndex, !event->IsShiftKeyPressed());
				PostCommand(cmd);
			}
			break;
			
		case chUp:
			{
				GridCell aCell;
				if (IsAnyCellSelected())
				{
					aCell = FirstSelectedCell();
					aCell.v--;
				}
				else
					aCell = GridCell(1, fNumOfRows);
				if (aCell.v >= 1)
				{
					SelectCell(aCell, false, true, true);
					ScrollSelectionIntoView(kRedraw);
					Focus();
					Update();
				}
			}
			break;

		case chDown:
			{
				GridCell aCell;
				if (IsAnyCellSelected())
				{
					aCell = LastSelectedCell();
					aCell.v++;
				}
				else
					aCell = GridCell(1, 1);
				if (aCell.v <= fNumOfRows)
				{
					SelectCell(aCell, false, true, true);
					ScrollSelectionIntoView(kRedraw);
					Focus();
					Update();
				}
			}
			break;

		default:
			if (event->fCharacter >= 32)
			{
				TTreeTypeName *cmd = new TTreeTypeName();
				cmd->ITreeTypeName(this, fExpandData, fWindowFolderIndex, event);
				PostCommand(cmd);
			}
			else
				inherited::DoKeyEvent(event);			
	} // case
}

pascal void TGroupTreeView::DoCommandKeyEvent(TToolboxEvent *event)
{
	if (!this->IsEnabled())
	{
		inherited::DoCommandKeyEvent(event);
		return;
	}
	switch (event->fCharacter)
	{
		case chEnter:
		case chReturn:
			OpenSelection(false);
			break;
		
		case chUp:
			if (event->IsShiftKeyPressed())
			{
				DoMenuCommand(cOpenListOfAllGroups);
				return;
			}
			if (fWindowFolderIndex != kFirstIndex)
			{
				ArrayIndex index = fGroupTree->GetParentIndex(fWindowFolderIndex);
				fGroupTreeDoc->CreateGroupTreeWindow(index, this);
			}
			break;

		case chDown:
			OpenSelection(true);
			break;

		case chLeft:
			ExpandCompactSelection(false, event->IsOptionKeyPressed());
			break;

		case chRight:
			ExpandCompactSelection(true, event->IsOptionKeyPressed());
			break;

		default:
			inherited::DoCommandKeyEvent(event);
	} // case
}


pascal void TGroupTreeView::DoMenuCommand(CommandNumber aCommandNumber)
{
	switch (aCommandNumber)
	{
		default:
			{
				StandardGridViewTextStyle gvts = fGridViewTextStyle;
				if (gNewsAppl->HandleFontMenu(aCommandNumber, gvts.fTextStyle))
				{
					fGridViewTextStyle = gvts;
					ChangedFont();
				}
				else
					inherited::DoMenuCommand(aCommandNumber);
			}
	}
}

pascal void TGroupTreeView::DoSetupMenus()
{
	gNewsAppl->EnableFontMenu(fGridViewTextStyle.fTextStyle);
	Enable(cSelectAll, true);
	inherited::DoSetupMenus();
	Enable(cSaveAs, false);
	Enable(cSaveCopy, false);
	Enable(cRevert, false);
}
