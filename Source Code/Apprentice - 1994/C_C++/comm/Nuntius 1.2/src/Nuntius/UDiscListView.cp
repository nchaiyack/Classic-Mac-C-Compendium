// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UDiscListView.cp

#include "UDiscListView.h"
#include "UDiscList.h"
#include "UDiscListCmds.h"
#include "UBinariesCmds.h"
#include "UPostArticleCmds.h"
#include "NetAsciiTools.h"
#include "UNewsAppl.h"
#include "UGroupDoc.h"
#include "UThread.h"
#include "UPrefsDatabase.h"
#include "URowSelection.h"
#include "UTextScroller.h"
#include "ViewTools.h"
#include "Tools.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>

#include <Packages.h>
#include <ToolUtils.h>

#pragma segment MyGroup

#define qPrintDottedLine 1

#define qDebugArticleIterator qDebug & 0

//-------------------------------------------------------------------
const short kSeparatorHeight = 2; // should be in 'View'
const short kExtraHeight = 2;

CStr255 gNewString, gSeenString, gReadString, gExpiredSubjectString, gExpiredAuthorString;
Boolean gHasFetchedStatusStrings = false;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CDiscListArticleIterator : public CIterator
{
	public:
		CDiscListArticleIterator(TDiscListView *dlv, GridCell cell);
		~CDiscListArticleIterator();
		
		long FirstArticleID();
		Boolean More();
		long NextArticleID();
		ArrayIndex GetIndex();
		Boolean ArticleIsAvaible();
		
		void GetArticleVRects(CRect &subjectRect, CRect &statusRect, CRect &authorRect);
	protected:
		void Advance();

	private:
		TDiscListView *fDiscListView;
		PDiscList *fDiscList;
		ArrayIndex fDiscIndex;
		ArrayIndex fNoArticles, fArticleIndex;
		long fArticleID;
		CRect fCellRect;
};

CDiscListArticleIterator::CDiscListArticleIterator(TDiscListView *dlv, GridCell cell)
{
	fDiscListView = dlv;
	fDiscList = dlv->fDiscList;
	VRect vr;
	dlv->CellToVRect(cell, vr);
	dlv->ViewToQDRect(vr, fCellRect);
	fDiscIndex = dlv->fDiscIndexList->At(cell.v);
	fNoArticles = fDiscList->GetNoArticles(fDiscIndex);
#if qDebugArticleIterator
	fprintf(stderr, "DLAI: Constructor, cellRect = %s, discIndex = %ld, noArticles = %ld\n", (char*)fCellRect, fDiscIndex, fNoArticles);
#endif
}

CDiscListArticleIterator::~CDiscListArticleIterator()
{
#if qDebugArticleIterator
	fprintf(stderr, "DLAI: Destruct\n");
#endif
}

long CDiscListArticleIterator::FirstArticleID()
{
	fArticleIndex = 0;
	Advance();
#if qDebugArticleIterator
	fprintf(stderr, "DLAI: returned %ld as first article id\n", fArticleID);
#endif
	return fArticleID;
}

Boolean CDiscListArticleIterator::More()
{
	return fArticleIndex <= fNoArticles;
}

long CDiscListArticleIterator::NextArticleID()
{
	if (More())
		Advance();
#if qDebugArticleIterator
	fprintf(stderr, "DLAI: returns %ld as next article id\n", fArticleID);
#endif
	return fArticleID;
}

void CDiscListArticleIterator::Advance()
{
	++fArticleIndex;
	if (!More())
		return;
	fArticleID = fDiscList->GetArticleID(fDiscIndex, fArticleIndex);
#if qDebugArticleIterator
	fprintf(stderr, "DLAI: Advance: new fArticleIndex = %ld, new fArticleID = %ld\n", fArticleIndex, fArticleID);
#endif
}

ArrayIndex CDiscListArticleIterator::GetIndex()
{
#if qDebugArticleIterator
	fprintf(stderr, "DLAI: GetIndex() returns %ld\n", fArticleIndex);
#endif
	return fArticleIndex;
}

Boolean CDiscListArticleIterator::ArticleIsAvaible()
{
	return fArticleID >= fDiscListView->fFirstAvaibleID && fArticleID <= fDiscListView->fLastAvaibleID;
}
	
void CDiscListArticleIterator::GetArticleVRects(CRect &subjectRect, CRect &statusRect, CRect &authorRect)
{
	short top = fCellRect.top + kSeparatorHeight;
	top += (short(fArticleIndex) - 1) * fDiscListView->fGridViewTextStyle.fRowHeight;
	short bottom = top + fDiscListView->fGridViewTextStyle.fRowHeight;
	subjectRect.top = statusRect.top = authorRect.top = top;
	subjectRect.bottom = statusRect.bottom = authorRect.bottom = bottom;
	
	short h = fDiscListView->fGridViewTextStyle.fHorzOffset;
	short col1 = h;
	short col2 = col1 + fDiscListView->fSubjectWidth;
	short col3 = col2 + 2 * h;
	subjectRect.left = col1;
	subjectRect.right = col2 - h;
	if (fArticleIndex > 1)
		subjectRect.left += fDiscListView->fSubjectHorzOffset;
	statusRect.left = col2;
	statusRect.right = col3;
	authorRect.left = col3;
	authorRect.right = short(fDiscListView->fSize.h);
#if qDebugArticleIterator
	fprintf(stderr, "DLAI: GetRects, subject = %s, ", (char*)subjectRect);
	fprintf(stderr, "status = %s, ", (char*)statusRect);
	fprintf(stderr, "author = %s\n", (char*)authorRect);
#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
TDiscListView::TDiscListView()
{
}

pascal void TDiscListView::Initialize()
{
	inherited::Initialize();
	fDoc = nil;
	fWindow = nil;
	fArticleStatus = nil;
	fOldArticleStatus = nil;
	fDiscList = nil;
	fDiscIndexList = nil;
	fScroller = nil;
	fFirstAvaibleID = 0;
	fLastAvaibleID = 0;
	fDiscSeparatorPattern = nil;
	fDiscToShow = kShowAllDiscs;
}

pascal void TDiscListView::ReadFields(TStream *aStream)
{
	inherited::ReadFields(aStream);
	FailInfo fi;
	if (fi.Try())
	{
#if qDebug
		if (fNumOfCols != 1)
			ProgramBreak("No of cols not 1 in TDiscListView");
#endif
		fSubjectWidth = 370;
		fSubjectHorzOffset = 40;
		fi.Success();
	}
	else // fail
	{
		Free();
		fi.ReSignal();
	}
}

pascal void TDiscListView::DoPostCreate(TDocument *itsDocument)
{
	inherited::DoPostCreate(itsDocument);
	fWindow = GetWindow();
	fDoc = (TGroupDoc*) itsDocument;
	fDiscList = fDoc->GetDiscList();
	fArticleStatus = fDoc->GetArticleStatus();
	fOldArticleStatus = fDoc->GetOldArticleStatus();

	TLongintList *lList = new TLongintList();
	lList->ILongintList();
	fDiscIndexList = lList;

	fScroller = (TTextScroller*)fWindow->FindSubView('SCL1');
	
	if (!gHasFetchedStatusStrings)
	{
		MyGetIndString(gNewString, kDiscListStatusNew);
		MyGetIndString(gSeenString, kDiscListStatusSeen);
		MyGetIndString(gReadString, kDiscListStatusRead);
		MyGetIndString(gExpiredSubjectString, kDiscListStatusExpiredSubject);
		MyGetIndString(gExpiredAuthorString, kDiscListStatusExpiredAuthor);
		gHasFetchedStatusStrings = true;
	}

	StandardGridViewTextStyle gvts;
	gPrefs->GetTextStylePrefs('TSdi', gvts.fTextStyle);
	CalcStandardGridViewFont(gvts);
	fGridViewTextStyle = gvts;
#if qPrintDottedLine
	fDiscSeparatorPattern = GetPattern(kDiscSeparatorPattern);
	FailNILResource(Handle(fDiscSeparatorPattern));
	HLockHi(Handle(fDiscSeparatorPattern));
#endif
	fDiscToShow = DiscussionShowType(gPrefs->GetShortPrefs('disS'));
}

pascal void TDiscListView::Free()
{
	fDoc = nil;
	fWindow = nil;
	fDiscList = nil; // not mine
	fArticleStatus = nil;
	fOldArticleStatus = nil; // not mine
	FreeIfObject(fDiscIndexList); fDiscIndexList = nil;
	inherited::Free();
}

pascal void TDiscListView::DoCalcPageStrips(VPoint& pageStrips)
{
	inherited::DoCalcPageStrips(pageStrips);
	pageStrips.v = 1;
}

void TDiscListView::Startup(const CStr255 &groupName)
{
	fGroupName = groupName;
}

void TDiscListView::CloseDown()
{
// mark the articles in the shown disc's as being seen
/*
	CLongintIterator discIndexIter(fDiscIndexList);
	for (long discIndex = discIndexIter.FirstLong(); discIndexIter.More(); discIndex = discIndexIter.NextLong())
	{
		short noArticles = fDiscList->GetNoArticles(discIndex);
		for (short artIndex = 1; artIndex <= noArticles; artIndex++)
		{
			long id = fDiscList->GetArticleID(discIndex, artIndex);
			fArticleStatus->SetMinStatus(id, kArticleSeen);
		}
	}
*/
}

pascal void TDiscListView::Close()
{
	gPrefs->SetWindowPosPrefs('WDis', fWindow);
	inherited::Close();
}

void TDiscListView::ChangedFont()
{
	StandardGridViewTextStyle gvts = fGridViewTextStyle;
	CalcStandardGridViewFont(gvts);
	fGridViewTextStyle = gvts;
	UpdateList();
}

pascal Boolean TDiscListView::CanSelectCell(GridCell aCell)
{
	if (aCell.h != 1) 
		return false;
	if (aCell.v < 1 || aCell.v > fNumOfRows) 
		return false;
	return true;
}

//================================================================================================
pascal void TDiscListView::DrawCell(GridCell aCell, const VRect &aRect)
{
#if qPrintDottedLine
	if (aCell.v > 1)
	{
		CRect r;
		ViewToQDRect(aRect, r);
		MoveTo(r.left, r.top);
		PenSize(1, 1);
		PenPat(*fDiscSeparatorPattern);
		PenMode(patCopy);
		Line(short(fSize.h), 0);
	}
#endif

	CStr255 lastSubject;
	CStr255 s;
	CRect subjectRect, statusRect, authorRect;

	CDiscListArticleIterator iter(this, aCell);
	for (long id = iter.FirstArticleID(); iter.More(); id = iter.NextArticleID())
	{
		iter.GetArticleVRects(subjectRect, statusRect, authorRect);
		short v = subjectRect.top + fGridViewTextStyle.fVertOffset;

		if (!iter.ArticleIsAvaible())
		{
			MoveTo(subjectRect.left, v);
			DrawString(gExpiredSubjectString);
			MoveTo(authorRect.left, v);
			DrawString(gExpiredAuthorString);
			continue;
		}

		fArticleStatus->SetMinStatus(id, kArticleSeen); // test

		HandleOffsetLength hol;
		CStr255 subject("");
		if (fDoc->GetSubject(id, hol))
			CopyHolToCStr255(hol, subject);
		
		short i = subject.Length();
		while (i && subject[i] <= 32)
			--i;
		subject.Length() = i;
		while (i && subject[1] <= 32)
		{
			subject.Delete(1, 1); // expensive but seldom
			--i;
		}

		Boolean drawSubject;
		if (iter.GetIndex() == 1)
			drawSubject = true;
		else
		{
			drawSubject = (subject != lastSubject);
			if (drawSubject)
			{
				if (subject.Length() > 4 && subject[1] == 'R' && subject[2] == 'e' && subject[3] == ':')
				{
					i = (subject[4] == 32) ? 5 : 4;
					while (subject.Length() >= 5 && subject[5] <= 32)
						subject.Delete(5, 1);
					if (
						subject.Length() == lastSubject.Length() + 4 && 
						EqualBlocks(Ptr(&subject[5]), Ptr(&lastSubject[1]), lastSubject.Length()))
					{
							drawSubject = false; // ignore "Re:Ê"
					}
					else if (
						subject.Length() + 4 == lastSubject.Length() &&
						EqualBlocks(Ptr(&subject[1]), Ptr(&lastSubject[5]), subject.Length()))
					{
						drawSubject = false; // ignore removed "Re:Ê"
					}
				}
			}
		}

		lastSubject = subject;
		if (drawSubject)
		{
			MoveTo(subjectRect.left, v);
			TruncString(subjectRect.GetLength(hSel), subject, smTruncMiddle);
			DrawString(subject);
		}
		
		MoveTo(statusRect.left, v);
		ArticleStatus stat = fOldArticleStatus->GetStatus(id);
//		ArticleStatus stat = fArticleStatus->GetStatus(id);
		if (stat == kArticleNew)
			DrawString(gNewString);
		else if (stat == kArticleSeen)
			DrawString(gSeenString);
		else
			DrawString(gReadString);

		MoveTo(authorRect.left, v);
		CStr255 email;
		if (fDoc->GetFrom(id, hol))
		{
			CStr255 text;
			CopyHolToCStr255(hol, text);
			GetPrintableAuthorName(text, s, email);
		}
		else
			s = gExpiredAuthorString;
		DrawString(s);
#if qDebug
		DrawString(", ");
		NumToString(id, s);
		DrawString(s);
#endif
	}
}

pascal void TDiscListView::DrawRangeOfCells(GridCell startCell,
										GridCell stopCell,
										const VRect &aRect)
{
	TextStyle itsTextStyle = fGridViewTextStyle.fTextStyle;
	SetPortTextStyle(itsTextStyle);
	inherited::DrawRangeOfCells(startCell, stopCell, aRect);
}

//================================================================================================
void TDiscListView::HighlighOneCell(GridCell aCell, HLState fromHL, HLState toHL)
{
	VRect vr;
	CellToVRect(aCell, vr);
	CRect r;
	ViewToQDRect(vr, r);
	r.top += kSeparatorHeight - 1; // -1 ??
//	r.bottom -= 1;
	PenNormal();
	PenMode(patXor);
	UseSelectionColor();
	switch (fromHL + toHL)
	{
		case hlOnDim:
		case hlOffOn:
			InvertRect(r);
			break;
/*
		case hlOnDim:
			r.Inset(CPoint(1, 1));
			InvertRect(r);
			break;

		case hlOffDim:
			FrameRect(r);
			break;

		case hlOffOn:
			InvertRect(r);
			break;
*/
	}
}

pascal void TDiscListView::HighlightCells(RgnHandle theCells,
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


//================================================================================================
Boolean TDiscListView::FindHelp(const VPoint &localPoint, VRect &helpRect, short &balloonVariant)
{
	GridCell aCell = VPointToCell(localPoint);
	CDiscListArticleIterator iter(this, aCell);
	CPoint pt = localPoint.ToPoint();
	for (long id = iter.FirstArticleID(); iter.More(); id = iter.NextArticleID())
	{
		CRect subjectRect, statusRect, authorRect;
		iter.GetArticleVRects(subjectRect, statusRect, authorRect);
		if (pt.v < subjectRect.top)
		{
			helpRect.left = 0;
			helpRect.right = fSize.h;
			helpRect.top = 0;
			helpRect.bottom = subjectRect.top;
			fHelpIndex = 1;
			balloonVariant = kHMEnabledItem;
			return false;
		}

		if (iter.ArticleIsAvaible())
			balloonVariant = kHMEnabledItem;
		else	
			balloonVariant = kHMDisabledItem;

		if (subjectRect.Contains(pt))
		{
			helpRect = subjectRect;
			fHelpIndex = 2;
			return true;
		}
		if (statusRect.Contains(pt))
		{
			helpRect = statusRect;
			ArticleStatus stat = fOldArticleStatus->GetStatus(id);
			if (stat == kArticleNew)
				fHelpIndex = 3;
			else if (stat == kArticleSeen)
				fHelpIndex = 4;
			else
				fHelpIndex = 5;
			return true;
		}
		if (authorRect.Contains(pt))
		{
			helpRect = authorRect;
			fHelpIndex = 6;
			return true;
		}
	} // iter
	helpRect[topLeft] = localPoint;
	helpRect[botRight] = localPoint + VPoint(1, 1);
	return false;
}

pascal void TDiscListView::DoShowHelp(const VPoint& localPoint, RgnHandle helpRegion)
{
	VRect helpRect;
	short balloonVariant;
	if (!FindHelp(localPoint, helpRect, balloonVariant))
	{
		RectRgn(helpRegion, helpRect.ToRect());
		return;
	}
	
	HMMessageRecord helpMessage;
	CPoint tip;
	CRect hotRect;
	short dummyBalloonVariant;

	GetHelpParameters(localPoint, helpRegion, helpMessage, tip, hotRect, dummyBalloonVariant);
	
	hotRect = helpRect.ToRect();
	RectRgn(helpRegion, hotRect);
	
//	tip.v = short(helpRect.top + helpRect.bottom) / 2;
	tip.h = short(helpRect.left + helpRect.right) / 2;
//	tip.h = hotRect.left;
	tip.v = hotRect.bottom;
	
	DoShowBalloon(localPoint, helpRegion, helpMessage, tip, hotRect, balloonVariant);
}

//================================================================================================
Boolean TDiscListView::ShowDiscussion(ArrayIndex discIndex)
{
	switch (fDiscToShow)
	{
		case kShowAllDiscs:
			return true;
		
		case kShowTodaysDiscs:
			{
				const unsigned long noDaySecs = 24 * 60 * 60;
				unsigned long dayStart;
				GetDateTime(dayStart);
				dayStart = dayStart - (dayStart % noDaySecs);
				return fDiscList->GetLastActiveDateTime(discIndex) > dayStart;
			}
			
		case kShowDiscsWithUnreadArticles:
			{
				long noArticles = fDiscList->GetNoArticles(discIndex);
				for (long artIndex = noArticles; artIndex; artIndex--)
				{
					long id = fDiscList->GetArticleID(discIndex, artIndex);
					if (id >= fDoc->GetFirstArticleID() && fOldArticleStatus->GetStatus(id) <= kArticleSeen)
						return true;
				}
			}
			return false;
			
		case kShowDiscsWithNewArticles:
			{
				long noArticles = fDiscList->GetNoArticles(discIndex);
				for (long artIndex = noArticles; artIndex; artIndex--)
				{
					long id = fDiscList->GetArticleID(discIndex, artIndex);
					if (id >= fDoc->GetFirstArticleID() && fOldArticleStatus->GetStatus(id) == kArticleNew)
						return true;
				}
			}
			return false;
	} // case
}

void TDiscListView::UpdateList()
{
	SetEmptySelection(kRedraw);
	DelRowFirst(fNumOfRows); // slet alt
	fDiscIndexList->DeleteAll();
	fFirstAvaibleID = fDoc->GetFirstArticleID();
	fLastAvaibleID = fDoc->GetLastArticleID();
	

#if qDebug & 0
	DateTimeRec dtr;
	Secs2Date(dayStart, dtr);
	fprintf(stderr, "dayStart = %lx = %hd/%hd-%hd %hd:%hd:%hd\n", 
		dayStart, dtr.day, dtr.month, dtr.year, dtr.hour, dtr.minute, dtr.second);
#endif

	long maxIndex = fDiscList->GetSize();
	for (long discIndex = 1; discIndex <= maxIndex; discIndex++)
	{
		if (fDiscList->EntryIsFree(discIndex))
			continue;
		if (!ShowDiscussion(discIndex))
			continue;
		
		fDiscIndexList->InsertLast(discIndex);
		short noArticles = fDiscList->GetNoArticles(discIndex);
/*
		InsRowLast(1, kSeparatorHeight + kExtraHeight + noShownArticles * fGridViewTextStyle.fRowHeight);
		optimized to:  (avoiding AdjustFrame for each inserted row)
*/
		short aHeight = kSeparatorHeight + kExtraHeight + noArticles * fGridViewTextStyle.fRowHeight;
		fRowHeights->InsertItems(fNumOfRows + 1, 1, aHeight);
		fNumOfRows++;
	}
// afterburner for optimizer:
	AdjustFrame();
	Focus();
	ForceRedraw();
}

void TDiscListView::MarkSelectionAs(ArticleStatus newStatus)
{
	if (!IsAnyCellSelected())
		return;
	Focus();
	short numSelected = 0;
	CSelectedCellIterator iter(this);
	for (iter.FirstCell(); iter.More(); iter.NextCell())
		++numSelected;
	Boolean useRedraw = (numSelected > 5);
	GridCell firstSelCell = FirstSelectedCell();
	GridCell aCell = LastSelectedCell();
	if (!useRedraw)
	{
		VRect area;
		CellToVRect(aCell, area);
		RevealRect(area, VPoint(20, 20), kRedraw);
	}
	while (aCell.v >= firstSelCell.v)
	{
		if (!IsCellSelected(aCell))
		{
			aCell.v--;
			continue;
		}
		CDiscListArticleIterator iter(this, aCell);
		for (long id = iter.FirstArticleID(); iter.More(); id = iter.NextArticleID())
		{
			fArticleStatus->SetStatus(id, newStatus);
			fOldArticleStatus->SetStatus(id, newStatus);
		}
		if (ShowDiscussion(fDiscIndexList->At(aCell.v)))
		{
			if (!useRedraw)
				InvalidateCell(aCell);
		}
		else
		{
			if (useRedraw)
			{
				SelectCell(aCell, true, true, false);
				DelRowAt(aCell.v, 1);
			}
			else
				DirectDeltaRows(this, aCell.v, -1, 0);
			fDiscIndexList->DeleteElementsAt(aCell.v, 1);
		}
		aCell.v--;
	}
	if (useRedraw)
	{
		Focus();
		ForceRedraw();
	}
}

void TDiscListView::UpdateDiscussion(ArrayIndex discIndex)
{
	ArrayIndex index = fDiscIndexList->GetEqualItemNo(discIndex);
	if (index == kEmptyIndex)
		return;
	GridCell aCell(1, short(index));
	Boolean doSel = (aCell == FirstSelectedCell()) && (aCell == LastSelectedCell());
	if (ShowDiscussion(discIndex))
	{
		Focus();
		InvalidateCell(aCell);
	}
	else
	{
		DirectDeltaRows(this, aCell.v, -1, 0);
		fDiscIndexList->DeleteElementsAt(index, 1); 
	}
	if (doSel && aCell.v >= 1 && aCell.v <= fNumOfRows)
		SelectCell(aCell, true, true, true);
}

TLongintList *TDiscListView::GetSelection()
{
	TLongintList *list = nil;
	VOLATILE(list);
	FailInfo fi;
	if (fi.Try())
	{
		TLongintList *aList = new TLongintList();
		aList->ILongintList();
		list = aList;
		CSelectedCellIterator iter(this);
		for (GridCell aCell = iter.FirstCell(); iter.More(); aCell = iter.NextCell())
			list->InsertLast(fDiscIndexList->At(aCell.v));
		fi.Success();
		return list;
	}
	else  // fail
	{
		FreeIfObject(list); list = nil;
		fi.ReSignal();
	}
}

TLongintList *TDiscListView::GetListOfSelectedArticles()
{
	TLongintList *discIndexList = nil;
	VOLATILE(discIndexList);
	TLongintList *discIDList = nil;
	VOLATILE(discIDList);
	TLongintList *idList = nil;
	VOLATILE(idList);
	FailInfo fi;
	if (fi.Try())
	{
		TLongintList *llist = new TLongintList();
		llist->ILongintList();
		idList = llist;
		discIndexList = GetSelection();
		for (short discIndex = 1; discIndex <= discIndexList->GetSize(); discIndex++)
		{
			discIDList = fDiscList->GetArticleIDList(discIndexList->At(discIndex));
			for (short index = 1; index <= discIDList->GetSize(); index++)
				idList->InsertLast(discIDList->At(index));
			discIDList->Free(); discIDList = nil;
		}
		discIndexList->Free(); discIndexList = nil;
		fi.Success();
		return idList;
	}
	else // fail
	{
		FreeIfObject(discIDList); discIDList = nil;
		FreeIfObject(idList); idList = nil;
		FreeIfObject(discIndexList); discIndexList = nil;
		FailNewMessage(fi.error, fi.message, messageCannotExtractBinaries);
	}
}

void TDiscListView::OpenSelection(Boolean removeSelection)
{
	TLongintList *list = GetSelection();
	if (list->GetSize() == 0) 
		list->Free(); // empty selection
	else
	{
		TOpenArticleCommand *aCommand = new TOpenArticleCommand();
		aCommand->IOpenArticleCommand(fDoc, list, removeSelection);
		PostCommand(aCommand);
	}
}

void TDiscListView::DoSuperKey()
{
	if (IsAnyCellSelected())
	{
		OpenSelection(true);
		return;
	}
	TScrollerScrollBar *sb = fScroller->fScrollBars[vSel];
	if (sb->fLongVal < sb->fLongMax)
	{
		fScroller->DoPageDown();
		return;
	}
	fWindow->CloseByUser();
}

pascal void TDiscListView::DoKeyEvent(TToolboxEvent *event)
{
	if (!IsEnabled())
	{
		inherited::DoKeyEvent(event);
		return;
	}
	switch (event->fCharacter)
	{
		case chEnter:
		case chReturn:
			OpenSelection(true);
			break;

		case chSpace:
			DoSuperKey();
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

		case chBackspace:
			SetEmptySelection(kRedraw); // should be KILL one day
			break;
			

		default:
			inherited::DoKeyEvent(event);
	}
}

pascal void TDiscListView::DoMouseCommand(VPoint &theMouse,
														TToolboxEvent *event, CPoint /* hysteresis */ )
{
	GridCell aCell;
	if (IdentifyPoint(theMouse, aCell) == badChoice)
		return;
	if (event->fClickCount > 1 && IsAnyCellSelected())
		OpenSelection();
	else // not double-click
	{
		TStickySelectCommand *aCommand = new TStickySelectCommand();
		aCommand->IStickySelectCommand(this, theMouse, event->IsShiftKeyPressed(), event->IsCommandKeyPressed()); 
		PostCommand(aCommand);
	}
}


pascal void TDiscListView::DoMenuCommand(CommandNumber aCommandNumber)
{
	if (!IsEnabled())
	{
		inherited::DoMenuCommand(aCommandNumber);
		return;
	}
	switch (aCommandNumber)
	{
		case cShowAllDiscussions:
			if (fDiscToShow != kShowAllDiscs)
			{
				fDiscToShow = kShowAllDiscs;
				UpdateList();
			}
			break;

		case cShowOnlyTodayDiscussions:
			if (fDiscToShow != kShowTodaysDiscs)
			{
				fDiscToShow = kShowTodaysDiscs;
				UpdateList();
			}
			break;

		case cShowDiscsWithUnreadArticles:
			if (fDiscToShow != kShowDiscsWithUnreadArticles)
			{
				fDiscToShow = kShowDiscsWithUnreadArticles;
				UpdateList();
			}
			break;

		case cShowDiscsWithNewArticles:
			if (fDiscToShow != kShowDiscsWithNewArticles)
			{
				fDiscToShow = kShowDiscsWithNewArticles;
				UpdateList();
			}
			break;

		case cSave:
		case cSaveAs:
		case cSaveCopy:
			{
				TSaveArticlesCommand *aCommand = new TSaveArticlesCommand();
				aCommand->ISaveArticlesCommand(cSaveAs, fDoc, GetListOfSelectedArticles());
				gApplWideThreads->ExecuteCommand(aCommand, "TSaveArticlesCommand (TDiscListView)");
			}
			break;
			
		case cExtractBinaries:
			ExtractBinaries(cExtractBinaries, fDoc, GetListOfSelectedArticles());
			break;
		
		case cPostNewDiscussion:
			TCreateNewDiscussionCommand *command = new TCreateNewDiscussionCommand();
			command->ICreateNewDiscussionCommand(fDoc);
			PostCommand(command);
			break;
			
		case cMarkThreadAsNew:
			MarkSelectionAs(kArticleNew);
			break;
			
		case cMarkThreadAsSeen:
			MarkSelectionAs(kArticleSeen);
			break;
			
		case cMarkThreadAsRead:
			MarkSelectionAs(kArticleRead);
			break;

		default:
			{
				StandardGridViewTextStyle gvts = fGridViewTextStyle;
				if (gNewsAppl->HandleFontMenu(aCommandNumber, gvts.fTextStyle))
				{
					CalcStandardGridViewFont(gvts);
					fGridViewTextStyle = gvts;
					gPrefs->SetTextStylePrefs('TSdi', gvts.fTextStyle);
					ChangedFont();
				}
				else
					inherited::DoMenuCommand(aCommandNumber);
			}
	}
}

pascal void TDiscListView::DoSetupMenus()
{
	Boolean lowSpace = MemSpaceIsLow();
	gNewsAppl->EnableFontMenu(fGridViewTextStyle.fTextStyle);
	Boolean gotSelection = IsAnyCellSelected();
	Enable(cExtractBinaries, !lowSpace && gotSelection);
	if (gPrefs->GetBooleanPrefs('BiAs'))
		SetMenuState(cExtractBinaries,	kDynamicMenuItemNames, kExtractAsMenuItemText, kExtractAsMultipleAsMenuItemText, IsOptionKeyDown());
	else
		SetMenuState(cExtractBinaries,	kDynamicMenuItemNames, kExtractMenuItemText, kExtractAsMultipleMenuItemText, IsOptionKeyDown());

	Enable(cMarkThreadAsNew, gotSelection);
	Enable(cMarkThreadAsSeen, gotSelection);
	Enable(cMarkThreadAsRead, gotSelection);

	Enable(cSelectAll, true);

	Enable(cPostNewDiscussion, !lowSpace);

	EnableCheck(cShowAllDiscussions, true, fDiscToShow == kShowAllDiscs);
	EnableCheck(cShowOnlyTodayDiscussions, true, fDiscToShow == kShowTodaysDiscs);
	EnableCheck(cShowDiscsWithUnreadArticles, true, fDiscToShow == kShowDiscsWithUnreadArticles);
	EnableCheck(cShowDiscsWithNewArticles, true, fDiscToShow == kShowDiscsWithNewArticles);

	inherited::DoSetupMenus();
	Enable(cSave, gotSelection);
	Enable(cSaveAs, false);
	Enable(cSaveCopy, gotSelection);
	Enable(cRevert, false);
}
