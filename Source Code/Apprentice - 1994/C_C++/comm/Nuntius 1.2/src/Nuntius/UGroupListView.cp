// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupListView.cp

#include "UGroupListView.h"
#include "UGroupListCmds.h"
#include "UGroupListDoc.h"
#include "UGroupList.h"
#include "UNewsAppl.h"
#include "UGroupDocCmds.h"
#include "UPrefsDatabase.h"
#include "Tools.h"
#include "UThread.h"

#include <RsrcGlobals.h>

#include <Packages.h>

#pragma segment MyGroupList

TGroupListView::TGroupListView()
{
}

pascal void TGroupListView::Initialize()
{
	inherited::Initialize();
	fWindow = nil;
	fGroupList = nil;
	fDoc = nil;
	fLastTypeKeyTick = 0;
}

pascal void TGroupListView::ReadFields(TStream *aStream)
{
	inherited::ReadFields(aStream);
}

pascal void TGroupListView::DoPostCreate(TDocument *itsDocument)
{
	inherited::DoPostCreate(itsDocument);
	fWindow = GetWindow();
	fDoc = (TGroupListDoc*) itsDocument;
	fGroupList = fDoc->GetGroupList();
#if qDebug
	if (!IsObject(fDoc))
		ProgramBreak("fDoc is not object");
	if (!IsObject(fGroupList))
		ProgramBreak("fGroupList is not object");
#endif
	StandardGridViewTextStyle gvts;
	gPrefs->GetTextStylePrefs('TSgl', gvts.fTextStyle);
	CalcStandardGridViewFont(gvts);
	fGridViewTextStyle = gvts;
}

pascal void TGroupListView::Free()
{
	inherited::Free();
}

pascal void TGroupListView::Close()
{
	inherited::Close();
}

pascal void TGroupListView::ShowReverted()
{
	SetEmptySelection(kHighlight);
	UpdateList();
}

void TGroupListView::ChangedFont()
{
	StandardGridViewTextStyle gvts = fGridViewTextStyle;
	CalcStandardGridViewFont(gvts);
	fGridViewTextStyle = gvts;
	if (fNumOfRows)
		SetRowHeight(1, fNumOfRows, fGridViewTextStyle.fRowHeight);
	Focus();
	ForceRedraw();
}

void TGroupListView::AddGroup(CStr255 &name)
{
	if (!fGroupList->AppendGroup(name))
		return;
	InsRowLast(1, fGridViewTextStyle.fRowHeight);
	fDoc->Changed(cGroupListChange, this);
}

pascal void TGroupListView::DrawRangeOfCells(GridCell startCell,
										 GridCell stopCell,
										 const VRect &aRect)
{
	TextStyle itsTextStyle = fGridViewTextStyle.fTextStyle;
	SetPortTextStyle(itsTextStyle);
	inherited::DrawRangeOfCells(startCell, stopCell, aRect);
}

pascal void TGroupListView::DrawCell(GridCell aCell, const VRect &aRect)
{
	CRect r;
	ViewToQDRect(aRect, r);
	short h = r.left + fGridViewTextStyle.fHorzOffset;
	short v = r.top + fGridViewTextStyle.fVertOffset - 1;
	MoveTo(r.left + fGridViewTextStyle.fHorzOffset, r.top + fGridViewTextStyle.fVertOffset - 1);
	CStr255 dotName;
	fGroupList->GetGroupAt(aCell.v, dotName);
	NewArticleStat stat = fGroupList->GetNewArticleStat(aCell.v);
	short id;
	switch (stat)
	{
		case kUnknown:
			id = kGroupListNAUnknown;
			break;
		
		case kNoNew:
			id = kGroupListNANoNew;
			break;
		
		case kHasNew:
			id = kGroupListNAHasNew;
			break;
		
		default:
#if qDebug
			fprintf(stderr, "TGroupListView::DrawCell: Bad NewArticleStat = %ld\n", long(stat));
			ProgramBreak(gEmptyString);
#endif
			id = kGroupListNAUnknown;
			break;
	}
	CStr255 statString;
	MyGetIndString(statString, id);
	MoveTo(h, v);
	DrawString(statString);
	MoveTo(h + 2 * fGridViewTextStyle.fHorzOffset, v);
	DrawString(dotName);
}

void TGroupListView::GetCellTextRect(GridCell aCell, CRect &r)
{
	Focus();
	TextStyle itsTextStyle = fGridViewTextStyle.fTextStyle;
	SetPortTextStyle(itsTextStyle);
	VRect vr;
	CellToVRect(aCell, vr);
	ViewToQDRect(vr, r);
	CStr255 text;
	fGroupList->GetGroupAt(aCell.v, text);
	r.left += 3 * fGridViewTextStyle.fHorzOffset - 1;
	r.top += 1;
	r.right = r.left + StringWidth(text) + 2;
	r.bottom -= 1;
}

void TGroupListView::HighlighOneCell(GridCell aCell, HLState fromHL, HLState toHL)
{
	CRect r;
	GetCellTextRect(aCell, r);
	PenNormal();
	UseSelectionColor();
	switch (fromHL + toHL)
	{
		case hlOffDim:
			PenMode(patXor);
			FrameRect(r);
			break;

		case hlOnDim:
			r.Inset(CPoint(1, 1));
			InvertRect(r);
			break;

		case hlOffOn:
			InvertRect(r);
			break;
	}
}

pascal void TGroupListView::HighlightCells(RgnHandle theCells,
									  HLState fromHL,
									  HLState toHL)
{
	if (fromHL == toHL)
		return;
	Focus();
	switch (fromHL + toHL)
	{
		case hlOffDim:
		case hlOnDim:
		case hlOffOn:
			{
				CCellInRegionIterator iter(this, theCells);
				for (GridCell aCell = iter.FirstCell(); iter.More(); aCell = iter.NextCell())
					HighlighOneCell(aCell, fromHL, toHL);
			}
			break;
	}
}

pascal GridViewPart TGroupListView::IdentifyPoint(const VPoint& thePoint,
											 GridCell& aCell)
{
	GridViewPart part = inherited::IdentifyPoint(thePoint, aCell);
	if (part == badChoice)
		return part;
	CRect r;
	GetCellTextRect(aCell, r);
	CPoint pt = ViewToQDPt(thePoint);
	if (!r.Contains(pt))
		return badChoice;
	return part;	
}
//========================================================================
void TGroupListView::UpdateList()
{
	if (fNumOfRows)
		DelRowFirst(fNumOfRows);
	InsRowFirst(short(fGroupList->GetSize()), fGridViewTextStyle.fRowHeight);
}

void TGroupListView::UpdateGroupStatus(const CStr255 &dotName, Boolean hasNewArticles)
{
	ArrayIndex listIndex = fGroupList->FindIndexFromName(dotName);
	if (listIndex == kEmptyIndex)
		return;
	NewArticleStat newStat = (hasNewArticles ? kHasNew : kNoNew);
	NewArticleStat oldStat = fGroupList->GetNewArticleStat(listIndex);
	if (newStat == oldStat)
		return;
	fGroupList->SetNewArticleStat(listIndex, newStat);
	Focus();
	VRect vr;
	CellToVRect(GridCell(1, short(listIndex)), vr);
	vr.left = fGridViewTextStyle.fHorzOffset;
	vr.right = vr.left + 2 * fGridViewTextStyle.fHorzOffset - 2;
	InvalidateVRect(vr);
}

//========================================================================
TGroupList *TGroupListView::GetSelectionAsList()
{
	TGroupList *groupList = nil;
	VOLATILE(groupList);
	FailInfo fi;
	if (fi.Try())
	{
		TGroupList *gl = new TGroupList();
		gl->IGroupList(nil);
		groupList = gl;
		CSelectedCellIterator iter(this);
		for (GridCell aCell = iter.FirstCell(); iter.More(); aCell = iter.NextCell())
		{
			CStr255 name;
			fGroupList->GetGroupAt(aCell.v, name);
			groupList->InsertGroupBefore(groupList->fSize + 1, name);
		}
		fi.Success();
		return groupList;
	}
	else // fail
	{
		FreeIfObject(groupList); groupList = nil;
		fi.ReSignal();
	}
}

void TGroupListView::AddGroupsFromListBefore(ArrayIndex index, TGroupList *groupList)
{
	SetEmptySelection(kHighlight);
	short noGroups = short(fGroupList->AddGroupsFromListBefore(index, groupList));
	InsRowBefore(short(index), noGroups, fGridViewTextStyle.fRowHeight);
	if (fNumOfRows && noGroups > 0 && noGroups < fGroupList->GetSize())
	{
		CRect r(1, short(index), 1, short(index) + noGroups - 1);
		SetSelectionRect(r, false, true, true);
	}
}

void TGroupListView::OpenSelection(Boolean updateDatabase)
{
	FailSpaceIsLow();
	short numSelected = 0;
	CSelectedCellIterator iter(this);
	for (GridCell aCell = iter.FirstCell(); iter.More(); aCell = iter.NextCell())
		numSelected++;
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
		CStr255 name;
		fGroupList->GetGroupAt(aCell.v, name);
		TOpenGroupCommand *aCommand = new TOpenGroupCommand();
		aCommand->IOpenGroupCommand(name, updateDatabase, false, true);
		gApplWideThreads->ExecuteCommand(aCommand, "TOpenGroupCommand (TGroupListView)");
	}
}

void TGroupListView::DeleteGroup(short line)
{
	fGroupList->DeleteGroupAt(line);
	DelRowAt(line, 1);
}

void TGroupListView::DeleteSelection()
{
	CSelectedCellIterator iter(this, false, true, true);
	for (GridCell aCell = iter.FirstCell(); iter.More(); aCell = iter.NextCell())
	{
		SelectCell(aCell, true, false, false);
		DeleteGroup(aCell.v);
	}
}

void TGroupListView::DeleteGroupsFromList(TGroupList *groupList)
{
	CArrayIterator iter(groupList);
	for (ArrayIndex delIndex = iter.FirstIndex(); iter.More(); delIndex = iter.NextIndex())
	{
		CStr255 name;
		groupList->GetGroupAt(delIndex, name);
		ArrayIndex index = fGroupList->FindIndexFromName(name);
		if (index != kEmptyIndex)
			DeleteGroup(short(index));
	}
}

TGroupList *TGroupListView::GetGroupList()
{
	return fGroupList;
}

pascal void TGroupListView::DoKeyEvent(TToolboxEvent *event)
{
	if (!this->IsEnabled())
	{
		inherited::DoKeyEvent(event);
		return;
	}
	switch (event->fCharacter)
	{
		case chSpace:
			{
				if (!fNumOfRows)
					break;
				OpenSelection(!event->IsOptionKeyPressed());
				GridCell aCell;
				if (IsAnyCellSelected())
					aCell = FirstSelectedCell();
				else
					aCell = GridCell(1, 0);
				while (++aCell.v <= fNumOfRows)
				{
					if (fGroupList->GetNewArticleStat(aCell.v) != kNoNew)
						break;
				}
				if (aCell.v <= fNumOfRows)
					SelectCell(aCell, false, true, true);
				else
					SetEmptySelection(kHighlight);
				ScrollSelectionIntoView(kRedraw);
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

		case chEnter:
		case chReturn:
			OpenSelection(!event->IsOptionKeyPressed());
			break;
			
		case chBackspace:
			{
				GridCell cell = FirstSelectedCell();
				DeleteSelection();
				if (event->IsOptionKeyPressed() && cell.v > 0 && cell.v <= fNumOfRows)
					SelectCell(cell, false, true, true);
				ScrollSelectionIntoView(kRedraw);
			}
			break;
	
		case chTab:
			{
				TListTabKeyCommand *cmd = new TListTabKeyCommand();
				cmd->IListTabKeyCommand(this, !event->IsShiftKeyPressed());
				PostCommand(cmd);
			}
			break;
			
		default:
			if (event->fCharacter >= 32)
			{
				TListTypeName *cmd = new TListTypeName();
				cmd->IListTypeName(this, event);
				PostCommand(cmd);
			}
			else
				inherited::DoKeyEvent(event);			
	} // case
}

pascal void TGroupListView::DoMouseCommand(VPoint &theMouse,
								 TToolboxEvent *event,
								 CPoint /* hysteresis */)
{
	GridCell aCell;
	if (IdentifyPoint(theMouse, aCell) == badChoice)
	{
		if (!event->IsShiftKeyPressed())
			SetEmptySelection(kHighlight);
		return;
	}
	if (event->fClickCount > 1) 
	{
		OpenSelection(!event->IsOptionKeyPressed());
		return;
	}
	Focus();
	if (event->IsShiftKeyPressed())
		SelectCell(aCell, true, true, !IsCellSelected(aCell));
	else if (!IsCellSelected(aCell))
		SelectCell(aCell, false, true, true);

	TUnsubscribeGroupTracker *tracker = new TUnsubscribeGroupTracker();
	tracker->IUnsubscribeGroupTracker(this, theMouse, event->IsOptionKeyPressed());
	PostCommand(tracker);
}


pascal void TGroupListView::DoMenuCommand(CommandNumber aCommandNumber)
{
	switch (aCommandNumber)
	{
		case cClear:
			DeleteSelection();
			break;

		case cCheckForNewArticles:
			fDoc->CheckForNewArticlesNow();
			break;
			
		default:
			{
				StandardGridViewTextStyle gvts = fGridViewTextStyle;
				if (gNewsAppl->HandleFontMenu(aCommandNumber, gvts.fTextStyle))
				{
					CalcStandardGridViewFont(gvts);
					fGridViewTextStyle = gvts;
					gPrefs->SetTextStylePrefs('TSgl', gvts.fTextStyle);
					ChangedFont();
				}
				else
					inherited::DoMenuCommand(aCommandNumber);
			}
	}
}

pascal void TGroupListView::DoSetupMenus()
{
	gNewsAppl->EnableFontMenu(fGridViewTextStyle.fTextStyle);
	Enable(cSelectAll, true);
	Enable(cClear, IsAnyCellSelected());
	Enable(cCheckForNewArticles, fNumOfRows && !fDoc->IsCheckingForNewArticlesNow());
	inherited::DoSetupMenus();
}
