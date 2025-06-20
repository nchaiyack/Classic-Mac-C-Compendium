// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupViewCmds.cp

#include "UGroupViewCmds.h"
#include "UGroupTreeDoc.h"
#include "UGroupTree.h"
#include "UGroupTreeView.h"
#include "UGroupList.h"
#include "UGroupListView.h"
#include "UNewsAppl.h"
#include "UGroupDoc.h"
#include "UGroupListDoc.h"
#include "Tools.h"
#include "ViewTools.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>
#include <ToolUtils.h>
#include <Packages.h>

#pragma segment MyGroupList

#define qDebugGroupDrag qDebug & 0

#define qDebugTreeKey qDebug & 0
#define qDebugTreeKeyVerbose qDebugTreeKey

//========================================================================
TGroupTracker::TGroupTracker()
{
}


pascal void TGroupTracker::Initialize()
{
	inherited::Initialize();
	fDoesDragGray = false;
	fHasDrawedOnDesktop = false;
	fCurrentGrayPos = CPoint(0,0);
	fGrayPictH = nil;
	fMouseOffsetInGrayPict = CPoint(0, 0);
	fHasDrawedInsertMark = false;
	fOldInsertMarkRect = VRect(0, 0, 0, 0);
	fHitGroupListView = nil;
	fHitGroupList = nil;
	fDraggedGroups = nil;
	fMakingCopy = false;
	fInsertBeforeName = "";
}

pascal void TGroupTracker::IGroupTracker(
												CommandNumber itsCommandNumber,
												const VPoint &itsMouse,
												Boolean makingCopy)
{
	this->ITracker(itsCommandNumber, nil, kCantUndo, kDoesNotCauseChange, NULL, 
					nil, nil, itsMouse);
	fMakingCopy = makingCopy;
#if qDebugGroupDrag
	if (fMakingCopy)
		fprintf(stderr, "fMakingCopy = %hd\n", qDebugGroupDrag);
#endif
}

pascal void TGroupTracker::Free()
{
#if qDebugGroupDrag
	fprintf(stderr, "TGroupTracker::Free, done tracking\n");
#endif
	DisposeIfHandle(Handle(fGrayPictH)); fGrayPictH = nil;
	FreeIfObject(fDraggedGroups); fDraggedGroups = nil;
	inherited::Free();
}

pascal void TGroupTracker::DoIt()
{
}

pascal void TGroupTracker::DoFocus()
{
	inherited::DoFocus();
	if (fDeskTopTrackingPort)
		SetPort(fDeskTopTrackingPort);
}

pascal void TGroupTracker::TrackConstrain(TrackPhase /* aTrackPhase */,
										const VPoint& /* anchorPoint */,
								  	const VPoint& /* previousPoint */,
								    VPoint& /* nextPoint */,
										Boolean /* mouseDidMove */)
{
	// don't constrain anything!
}

pascal void TGroupTracker::ConstrainOnce(Boolean /* didMouseMove */)
{
	// don't constrain anything!
}


pascal void TGroupTracker::TrackFeedback(TrackPhase /* aTrackPhase */,
										const VPoint& /* anchorPoint */,
										const VPoint& /* previousPoint */,
										const VPoint& /* nextPoint */,
										Boolean /* mouseDidMove */,
										Boolean /* turnItOn */)
{
}

pascal TTracker *TGroupTracker::TrackMouse(TrackPhase aTrackPhase,
											 VPoint &anchorPoint,
											 VPoint &previousPoint,
											 VPoint &nextPoint,
											 Boolean /* mouseDidMove */) // override 
{
// da der blev specificeret nil som view til ITracker, er coordinaterne globale!
#if qDebugGroupDrag & 0
	GrafPtr gp;
	GetPort(gp);
	fprintf(stderr, "At TrackMouse: GrafPtr: $%lx\n", long(gp));
#endif
#if qDebugGroupDrag & 0
	fprintf(stderr, "aTrackPhase = %hd, ", short(aTrackPhase));
	fprintf(stderr, "anc = %s, ", (char*)anchorPoint);
	fprintf(stderr, "prev = %s, ", (char*)previousPoint);
	fprintf(stderr, "next = %s\n", (char*)nextPoint);
#endif
	VPoint diff;
	if (!fDoesDragGray)
	{
		diff = nextPoint - anchorPoint;
#if qDebugGroupDrag & 0
		fprintf(stderr, "TGroupTracker::TrackMouse, non-gray diff: %s\n", (char*)diff);
#endif
		if (diff.v > -3 && diff.v < 3 && diff.h > -3 && diff.h < 3)
		{
			if (aTrackPhase == trackEnd)
				return nil;
			else
				return this;
		}
		StartGrayDrag();
		fCurrentGrayPos = CPoint(0, 0);
		diff = nextPoint;
	}
	else
	{
		diff = nextPoint - previousPoint;
	}

	if (diff != VPoint(0, 0))
	{
		if (fHasDrawedOnDesktop)
			DrawGray();
		fCurrentGrayPos += diff.ToPoint();
	}

	TestForHit(nextPoint.ToPoint());

	if (diff != VPoint(0, 0) || !fHasDrawedOnDesktop)
		DrawGray();
	
	if (aTrackPhase != trackEnd)
		return this;

	if (fHasDrawedOnDesktop)
		DrawGray(); // remove it
	if (fHasDrawedInsertMark)
		DrawInsertMark(); // remove it

	if (fHitGroupListView)
		DragToGroupList();
	else
		DidNotHitWindow();
		
	return nil; // no more tacking
}

short MyAbs(short a)
{
	return a < 0 ? -a : a;
}

void TGroupTracker::DidNotHitWindow()
{
	if (fHasDrawedOnDesktop)
		DrawGray();
	CPoint diff( (fNextPoint - fAnchorPoint).ToPoint() );
	short dh = MyAbs(diff.h);
	short dv = MyAbs(diff.v);
	short noSteps = 1 + short(Min(6, (dh + dv) / 8));
#if qDebugGroupDrag
	fprintf(stderr, "TGroupTracker::DidNotHitWindow, does animation with %hd steps\n", noSteps);
#endif
	CPoint delta;
	delta.h = -diff.h / noSteps;
	delta.v = -diff.v / noSteps;
	for (long i = 0; i < noSteps; i++)
	{
		fCurrentGrayPos += delta;
		DrawGray();
		long li;
		Delay(1, li);
		DrawGray();
	}
}

void TGroupTracker::StartGrayDrag()
{
#if qDebugGroupDrag
	fprintf(stderr, "TGroupTracker::StartGrayDrag, Starts gray drag\n");
#endif
	fCurrentGrayPos = CPoint(0, 0);
	MakeGrayPict();
	DoFocus();
	CopyRgn(GetGrayRgn(), fDeskTopTrackingPort->clipRgn);
	fDoesDragGray = true;
}

void TGroupTracker::DrawGray()
{
#if qDebug
	if (!fGrayPictH)
		ProgramBreak("DrawGray called when fGrayPictH == nil");
#endif
	DoFocus();
#if qDebugGroupDrag & 0
	GrafPtr gp;
	GetPort(gp);
	fprintf(stderr, "At DrawGray: GrafPtr: $%lx\n", long(gp));
#endif
	CRect r( (**fGrayPictH).picFrame );
	r -= r[topLeft];
	r -= fMouseOffsetInGrayPict;
	r += fCurrentGrayPos;
	DrawPicture(fGrayPictH, r);
	fHasDrawedOnDesktop = !fHasDrawedOnDesktop;
}

Boolean TGroupTracker::CalcNewHit(VPoint globalMouse, TGroupListView *&groupListView, 
																	VRect &newMarkRect, VPoint &localMouse)
{
	groupListView = nil;
	WindowPtr theWindowP;
	short part = FindWindow(globalMouse.ToPoint(), theWindowP);
	TWindow *window = WMgrToWindow(theWindowP);
	if (!window)
		return false;
#if qDebug
	if (!IsObject(window))
		ProgramBreak("Got bogous window from WMgrToWindow");
#endif
	if (!window->fDocument)
		return false; // document had no window, so not a GroupListWindow
#if qDebug
	if (!IsObject(window->fDocument))
		ProgramBreak("Window has bogous document");
#endif
	if (window->fDocument->fIdentifier != kGroupListDocFileType)
		return false; // not a GroupListWindow
	TGroupListDoc *doc = (TGroupListDoc*) window->fDocument;	
	groupListView = doc->GetGroupListView();
#if qDebug
	if (!IsObject(groupListView))
		ProgramBreak("TGroupListDoc returned bogous TGroupListView");
#endif
	localMouse = globalMouse - window->fLocation;
#if qDebugGroupDrag & 0
	fprintf(stderr, "TGroupTracker::CalcNewHit, localWindowPos = %s", (char*)localMouse);
#endif
	groupListView->Focus();
	groupListView->WindowToLocal(localMouse);

#if qDebugGroupDrag & 0
	fprintf(stderr, ", localGridViewPos = %s", (char*)localMouse);
	fprintf(stderr, ", GV.fSize = %s\n", (char*)VPoint(groupListView->fSize));
#endif
	VPoint localPos(localMouse);
	localPos.h = 1;
	localPos.v += groupListView->fGridViewTextStyle.fRowHeight / 2;
	short insertBeforeRow;
	if (localPos.v < 0)
		insertBeforeRow = 1;
	else if (localPos.v >= groupListView->fSize.v)
		insertBeforeRow = groupListView->fNumOfRows + 1;
	else
	{
		GridCell cell = groupListView->VPointToCell(localPos);
		insertBeforeRow = cell.v;
#if qDebug
		if (insertBeforeRow == 0)
			fprintf(stderr, "TGroupTracker::CalcNewHit, insertBeforeRow == 0 in GridViewHit\n");
#endif
	}
#if qDebugGroupDrag & 0
	fprintf(stderr, "-  insertBeforeRow = %hd", insertBeforeRow);
#endif
	CStr255 s;
	GetBeforeGroupName(groupListView, insertBeforeRow, s);
	fInsertBeforeName = s;
	const kHitRectHeight = 1;
	newMarkRect.left = 0;
	newMarkRect.top = (insertBeforeRow - 1) * groupListView->fGridViewTextStyle.fRowHeight;
	newMarkRect.right = groupListView->fSize.h;
	newMarkRect.bottom = newMarkRect.top + kHitRectHeight;
	return true;
}

void TGroupTracker::GetBeforeGroupName(TGroupListView *groupListView, short beforeRow, CStr255 &name)
{
	if (beforeRow > groupListView->fNumOfRows)
		name = "";
	else
		groupListView->GetGroupList()->GetGroupAt(beforeRow, name);
}

void TGroupTracker::DrawInsertMark()
{
	DoFocus();
	PenNormal();
	VRect markRect(fOldInsertMarkRect);
	fHitGroupListView->LocalToSuperVRect(markRect);
	fHitGroupListView->fSuperView->Focus();
	CRect r(markRect.ToRect());
	PenMode(patXor);
	PenPat(&qd.ltGray);
	PaintRect(r);
	PenNormal();
	fHasDrawedInsertMark = !fHasDrawedInsertMark;
}

void TGroupTracker::TestForHit(VPoint globalMouse)
{
/*
havde    nyt    forskellig
ja       ja      ja         g + n
ja       ja      nej				-
ja       nej     ?          g
nej      nej     ?          -
nej      ja      ?          n
*/
	Boolean hadOld = fHasDrawedInsertMark;
	VRect newMarkRect(0, 0, 0, 0);
	TGroupListView *groupListView = nil;
	VPoint localMouse;
	Boolean gotRect = CalcNewHit(globalMouse, groupListView, newMarkRect, localMouse);

	Boolean needsToScroll = false;
	VPoint delta;
	if (gotRect)
	{
		groupListView->GetScroller(true)->AutoScroll(localMouse, delta);
		needsToScroll = (delta.v != 0);
	}

	Boolean isNewRect = needsToScroll || (newMarkRect != fOldInsertMarkRect);

	Boolean removeOld = hadOld  && (!gotRect || isNewRect);
	Boolean drawNew   = gotRect && (!hadOld  || isNewRect);

#if qDebug
	if (hadOld && !IsObject(fHitGroupListView))
		ProgramBreak("Had old and fHitGroupListView is not object");
	if (gotRect && !IsObject(groupListView))
		ProgramBreak("gotRect && groupListView is not object");
#endif
#if qDebugGroupDrag & 0
	fprintf(stderr, "%s", (char*)VRect(fOldInsertMarkRect));
	fprintf(stderr, " -> %s", (char*)newMarkRect);
	fprintf(stderr, ", hadOld = %hd, got = %hd, isNew = %hd", hadOld, gotRect, isNewRect);
	fprintf(stderr, ", -Old = %hd, +New = %hd\n", removeOld, drawNew);
#endif

	
	if (removeOld)
	{
		DrawInsertMark();
		fHasDrawedInsertMark = false;
	}

	if (needsToScroll)
	{
		if (fHasDrawedOnDesktop)
			DrawGray();
		groupListView->GetScroller(true)->ScrollBy(VPoint(0, delta.v), kRedraw);
		groupListView->GetWindow()->Focus();
		groupListView->GetWindow()->Update();
	}

	if (drawNew)
	{
		fOldInsertMarkRect = newMarkRect;
		fHitGroupListView = groupListView;
		fHitGroupList = fHitGroupListView->GetGroupList();;
		DrawInsertMark();
		fHasDrawedInsertMark = true;
	}
	
	if (!gotRect)
	{
		fHitGroupListView = nil;
		fHitGroupList = nil;
	}
}

void TGroupTracker::CreateListOfDraggedGroups()
{
	Failure(minErr, 0);
}

void TGroupTracker::CheckForExistingGroups(TGroupListView *hitGroupListView, Boolean makingCopy)
{
	if (hitGroupListView->GetGroupList()->HasOneOfTheGroupsInList(fDraggedGroups))
	{
		short index = makingCopy ? kCopyGroupsWordIndex : kMoveGroupsWordIndex;
		CStr255 moveCopyText;
		MyGetIndString(moveCopyText, index);
		ParamText(gEmptyString, gEmptyString, gEmptyString, moveCopyText);
		if (MacAppAlert(kOverwriteExistingGroupsAlert, nil) != ok)
			Failure(0, 0);
	}
}

void TGroupTracker::DragToGroupList()
{
	CreateListOfDraggedGroups();
	if (fDraggedGroups->GetSize() > 25)
	{
		CStr255 s;
		NumToString(fDraggedGroups->GetSize(), s);
		ParamText(gEmptyString, gEmptyString, gEmptyString, s);
		if (MacAppAlert(phSubscribeVeryManyGroups, nil) != ok)
			Failure(0, 0);
	}
#if qDebug
	if (!IsObject(fHitGroupListView))
		ProgramBreak("fHitGroupListView is not object");
	if (!IsObject(fDraggedGroups))
		ProgramBreak("fDraggedGroups is not object");
#endif
	CheckForExistingGroups(fHitGroupListView, fMakingCopy);
	fHitGroupListView->DeleteGroupsFromList(fDraggedGroups);

	ArrayIndex index;
	if (fInsertBeforeName == "")
		index = fHitGroupList->GetSize() + 1;
	else
		index = fHitGroupList->FindIndexFromName(CStr255(fInsertBeforeName));
	if (index == kEmptyIndex)
	{
#if qDebug
		ProgramBreak("before index is kEmptyIndex");
#endif
		index = 1;
	}
	fHitGroupListView->AddGroupsFromListBefore(index, fDraggedGroups);
}

void TGroupTracker::MakeGrayPict()
{
	Failure(minErr, 0);
}

void TGroupTracker::WasDraggedToGroupTree() //@@ not called yet
{
	gApplication->Beep(2);
}
//========================================================================
//========================================================================
TGroupViewKeyCommand::TGroupViewKeyCommand()
{
}


pascal void TGroupViewKeyCommand::Initialize()
{
	inherited::Initialize();
	fGridView = nil;
}

void TGroupViewKeyCommand::IGroupViewKeyCommand(TGridView *gv)
{
	inherited::ICommand(cGenericCommand, nil, false, false, nil);
	fGridView = gv;
#if qDebug
	if (!IsObject(fGridView))
		ProgramBreak("fGridView is not object");
#endif
}

pascal void TGroupViewKeyCommand::Free()
{
	fGridView = nil;
	inherited::Free();
}

pascal void TGroupViewKeyCommand::DoIt()
{
	if (fGridView->fNumOfRows)
	{
		SetUp();
		DoAllLines();
		SetDown();
	}
	fGridView->ScrollSelectionIntoView(kRedraw);
}

void TGroupViewKeyCommand::SetUp()
{
	fCandidateName = "";
	fCandidateLine = kEmptyIndex;
	fLastLine = fGridView->fNumOfRows;
	if (fGridView->IsAnyCellSelected())
	{
		fCurrentSelectedLine = fGridView->FirstSelectedCell().v;
		CStr255 s;
		GetLineText(fCurrentSelectedLine, s);
		fCurrentSelectedName = s;
	}
	else
	{
		fCurrentSelectedName = "";
		fCurrentSelectedLine = kEmptyIndex;
	}
}

void TGroupViewKeyCommand::SetDown()
{
	if (fCandidateLine == kEmptyIndex)
		HandleNoFound();
	if (fCandidateLine != kEmptyIndex)
	{
		GridCell cell(GridCell(1, short(fCandidateLine)));
		fGridView->SelectCell(cell, false, true, true);
	}
}

void TGroupViewKeyCommand::DoAllLines()
{
	for (ArrayIndex line = 1; line <= fLastLine; line++)
		DoOneLine(line);
}

void TGroupViewKeyCommand::DoOneLine(ArrayIndex line)
{
	CStr255 text;
	GetLineText(line, text);
	DoLine(line, text);
}

void TGroupViewKeyCommand::GetLineText(ArrayIndex /* line */, CStr255 & /* text */)
{
	SubClassResponsibility();
}

void TGroupViewKeyCommand::DoLine(ArrayIndex /* line */, const CStr255 & /* text */)
{
	SubClassResponsibility();
}

void TGroupViewKeyCommand::HandleNoFound()
{
}

//--------------------------------------------------------------------------------
TGroupViewTypeNameCommand::TGroupViewTypeNameCommand()
{
}


pascal void TGroupViewTypeNameCommand::Initialize()
{
	inherited::Initialize();
	fLastTick = 0;
	fTypeChars = "";
	fChar = 32;
}

void TGroupViewTypeNameCommand::IGroupViewTypeNameCommand(TGridView *gv, TToolboxEvent *event)
{
	inherited::IGroupViewKeyCommand(gv);
	fChar = event->fCharacter;
	fCharTick = event->fEventRecord.when;
}

pascal void TGroupViewTypeNameCommand::Free()
{
	inherited::Free();
}

void TGroupViewTypeNameCommand::SetUp()
{
	inherited::SetUp();
	if (fCharTick - fLastTick > kKeyTimeout)
	{
		fTypeChars = "";
#if qDebugTreeKey
		fprintf(stderr, "TypeName: timeout for old name\n");
#endif
	}
	fTypeChars += fChar;
#if qDebugTreeKey
	fprintf(stderr, "TypeName: Current typed chars: '%s'\n", (char*)fTypeChars);
#endif
}

void TGroupViewTypeNameCommand::SetDown()
{
	inherited::SetDown();
	fLastTick = fCharTick;
}

void TGroupViewTypeNameCommand::DoLine(ArrayIndex line, const CStr255 &text)
{
#if qDebugTreeKeyVerbose
	fprintf(stderr, "TGroupListTypeNameCommand::DoLine, line = %hd, text = '%s'\n", line, (char*)text);
#endif
	if (text < fTypeChars)
	{
#if qDebugTreeKeyVerbose
		fprintf(stderr, "-  text < fTypeChars\n");
#endif
		return;
	}
	if (text < fCandidateName || !fCandidateName.Length())
	{
#if qDebugTreeKey
		fprintf(stderr, "TGroupListTypeNameCommand: Found new candidate: '%s'\n", (char*)text);
#endif
		fCandidateName = text;
		fCandidateLine = line;
	}
}


void TGroupViewTypeNameCommand::HandleNoFound()
{
	inherited::HandleNoFound();
	fCandidateLine = fLastLine;
#if qDebugTreeKey
	fprintf(stderr, "TGroupListTypeNameCommand: Found no candidate, selects last cell\n");
#endif
}

//-------------------------------------------------------------
TGroupViewTabKeyCommand::TGroupViewTabKeyCommand()
{
}


pascal void TGroupViewTabKeyCommand::Initialize()
{
	inherited::Initialize();
	fFirstLastLine = kEmptyIndex;
}

void TGroupViewTabKeyCommand::IGroupViewTabKeyCommand(TGridView *gv, Boolean forward)
{
	inherited::IGroupViewKeyCommand(gv);
	fForward = forward;
}

pascal void TGroupViewTabKeyCommand::Free()
{
	inherited::Free();
}

void TGroupViewTabKeyCommand::DoLine(ArrayIndex line, const CStr255 &text)
{
	if (fForward)
		DoForwardLine(line, text);
	else
		DoBackwardLine(line, text);
}

void TGroupViewTabKeyCommand::DoForwardLine(ArrayIndex line, const CStr255 &text)
{
#if qDebugTreeKeyVerbose
	fprintf(stderr, "TGroupViewTabKeyCommand::DoLine, line = %hd, text = '%s'\n", line, (char*)text);
#endif
	Boolean isLarger;
	if (line <= fCurrentSelectedLine)
		isLarger = text > fCurrentSelectedName;
	else
		isLarger = text >= fCurrentSelectedName;
	Boolean gotNew = isLarger && (text < fCandidateName || !fCandidateName.Length());
	if (gotNew)
	{
		fCandidateName = text;
		fCandidateLine = line;
	}
	if (fFirstLastLine == kEmptyIndex || text < fFirstLastName)
	{
		fFirstLastName = text;
		fFirstLastLine = line;
	}
}

void TGroupViewTabKeyCommand::DoBackwardLine(ArrayIndex line, const CStr255 &text)
{
	Boolean isSmaller;
	if (line < fCurrentSelectedLine)
		isSmaller = text <= fCurrentSelectedName;
	else
		isSmaller = text < fCurrentSelectedName;
	Boolean gotNew;
	if (line < fCurrentSelectedLine)
		gotNew = isSmaller && (text >= fCandidateName || !fCandidateName.Length());
	else
		gotNew = isSmaller && (text >  fCandidateName || !fCandidateName.Length());
	if (gotNew)
	{
		fCandidateName = text;
		fCandidateLine = line;
	}
	if (fFirstLastLine == kEmptyIndex || text > fFirstLastName)
	{
		fFirstLastName = text;
		fFirstLastLine = line;
	}
}

void TGroupViewTabKeyCommand::HandleNoFound()
{
	fCandidateLine = fFirstLastLine;
#if qDebugTreeKey
	fprintf(stderr, "TGroupViewTabKeyCommand: Found no candidate, selects first/last cell\n");
#endif
}
