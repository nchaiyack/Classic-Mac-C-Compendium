// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleListView.cp

#include "UArticleListView.h"
#include "UArticleManageView.h"
#include "UArticleView.h"
#include "UArticleCache.h"
#include "UArticle.h"
#include "UDiscList.h"
#include "UDiscListView.h"
#include "UArticleCmds.h"
#include "UBinariesCmds.h" 
#include "UMailCmds.h"
#include "UPrefsDatabase.h"
#include "UGroupDoc.h"
#include "UNewsAppl.h"
#include "UPostArticleCmds.h"
#include "UTextScroller.h"
#include "UNavigatorView.h"
#include "UThread.h"
#include "Tools.h"
#include "UFatalError.h"
#include "NetAsciiTools.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>

#include <ToolUtils.h>
#include <Packages.h>
#include <OSUtils.h>

#pragma segment MyArticle

#define qDebugUpdateListThread qDebug
//============================ ARTICLE ITERATOR =========================

class CArticleViewIterator : public CSubViewIterator
{
	public:
		CArticleViewIterator(TArticleListView *alv);
		TArticleView *FirstArticleView();
		TArticleView *NextArticleView();
		TArticleView *CurrentArticleView();
	protected:
		virtual void Reset();
		virtual void Advance();

	private:
		TArticleView *fCurrentArticleView;
		
		void FindArticleView();
};

CArticleViewIterator::CArticleViewIterator(TArticleListView *alv) :
	CSubViewIterator(alv)
{
	fCurrentArticleView = nil;
}

TArticleView *CArticleViewIterator::FirstArticleView()
{
	FirstSubView();
	return CurrentArticleView();
}

TArticleView *CArticleViewIterator::NextArticleView()
{
	NextSubView();
	return CurrentArticleView();
}

TArticleView *CArticleViewIterator::CurrentArticleView()
{
	return fCurrentArticleView;
}

void CArticleViewIterator::Reset()
{
	CArrayIterator::Reset();
	FindArticleView();
}

void CArticleViewIterator::Advance()
{
	CArrayIterator::Advance();
	FindArticleView();
}

void CArticleViewIterator::FindArticleView()
{
	fCurrentArticleView = nil;
	while (true)
	{
		TArticleManageView *amv = (TArticleManageView*)CurrentSubView();
		if (!amv)
			break;
		TArticleView *av = amv->GetArticleView();
		if (av)
		{
			fCurrentArticleView = av;
			break;
		}
		CArrayIterator::Advance();
		if (!More())
			return;
	}
}

//--------------------------------------------------------------
class CSelectedArticleViewIterator : public CArticleViewIterator
{
	public:
		CSelectedArticleViewIterator(TArticleListView *alv);

	protected:
		void Reset();
		void Advance();
};

CSelectedArticleViewIterator::CSelectedArticleViewIterator(TArticleListView *alv) :
	CArticleViewIterator(alv)
{
}

void CSelectedArticleViewIterator::Reset()
{
	CArticleViewIterator::Reset();
	TArticleView *av = CurrentArticleView();
	if (av && !av->IsAnyCellSelected())
		Advance();
}
void CSelectedArticleViewIterator::Advance()
{
	while (true)
	{
		CArticleViewIterator::Advance();
		if (!More())
			return;
		TArticleView *av = CurrentArticleView();
		if (av && av->IsAnyCellSelected())
			return;
	}
}
//============================ ARTICLE LIST VIEW ========================
TArticleListView::TArticleListView()
{
}

pascal void TArticleListView::Initialize()
{
	inherited::Initialize();
	fWindow = nil;
	fDiscList = nil;
	fDiscIndexList = nil;
	fArticleIndex = 0;
	fDoc = nil;
	fArticleStatus = fOldArticleStatus = nil;
	fPrefDiscArrow = nil;
	fNextDiscArrow = nil;
	fScroller = nil;
	fUpdateThreadList = nil;
	fDiscIndex = 0;
	fIsClosing = false;
	fUseROT13 = false;
}

pascal void TArticleListView::ReadFields(TStream *aStream)
{
	inherited::ReadFields(aStream);
	fUpdateThreadList = NewThreadList("ArticleListView update thread");
}


pascal void TArticleListView::DoPostCreate(TDocument *itsDocument)
{
	inherited::DoPostCreate(itsDocument);
	fWindow = GetWindow();
	fScroller = (TTextScroller*)(fWindow->FindSubView('SCL1'));
	fDoc = (TGroupDoc*) itsDocument;
	fDiscListView = fDoc->GetDiscListView();
	fArticleStatus = fDoc->GetArticleStatus();
	fOldArticleStatus = fDoc->GetOldArticleStatus();
	
	fDiscIndexList = nil;
	fDiscList = fDoc->GetDiscList();
	fPrefDiscArrow = (TNavigatorArrowView*) fWindow->FindSubView(kNavigatorLeftArrowView);
	fNextDiscArrow = (TNavigatorArrowView*) fWindow->FindSubView(kNavigatorRightArrowView);

	fShowsHeaders = gPrefs->GetBooleanPrefs('Head');

	TextStyle ts;
	gPrefs->GetTextStylePrefs('TSar', ts);
	fTextStyle = ts;
}

pascal void TArticleListView::Free()
{
	if (fUpdateThreadList && fUpdateThreadList->fSize)
		PanicExitToShell("In TArticleListView: fUpdateThreadList was not empty");
	FreeIfObject(fUpdateThreadList); fUpdateThreadList = nil;
	fDiscList = nil; // not mine
	FreeIfObject(fDiscIndexList); fDiscIndexList = nil;
	fScroller = nil; // view
	fWindow = nil; // not mine
	fDoc = nil; // not mine
	fArticleStatus = fOldArticleStatus = nil; // not mine
	inherited::Free();
}

pascal void TArticleListView::Close()
{
	FailInfo fi;
	if (fi.Try())
	{
		fIsClosing = true;
		KillUpdateListThread();
		gPrefs->SetWindowPosPrefs('WArt', fWindow);
		DeleteDisplayedArticles(true); // update fArticleStatus
		inherited::Close();
		fi.Success();
	}
	else // fail
	{
		fIsClosing = false;
		fi.ReSignal();
	}
}

void TArticleListView::KillUpdateListThread()
{
	if (fUpdateThreadList->fSize)
	{
#if qDebugUpdateListThread
		fprintf(stderr, "TArticleListView::KillUpdateListThread: Killing updateListThread at $%lx\n", long(fUpdateThreadList));
#endif
		fUpdateThreadList->KillAll();
		if (fUpdateThreadList->fSize)
			PanicExitToShell("In TArticleListView::KillUpdateListThread, fUpdateThreadList didn't became empty");
	}
}

VCoordinate TArticleListView::GetSeparatorHeight()
{
	return 8;
}

void TArticleListView::CheckNewFrame(VRect &newFrame)
{
#if 0
	VCoordinate width = GetPrintHandler()->fViewPerPage.h;
	newFrame.right = newFrame.left + width;
#else
	inherited::CheckNewFrame(newFrame);
#endif
}

pascal void TArticleListView::DoCalcPageStrips(VPoint& pageStrips)
{
	inherited::DoCalcPageStrips(pageStrips);
	pageStrips.v = 1;
}

void TArticleListView::ChangedFont()
{
	TextStyle ts = fTextStyle;
	CSubViewIterator iter(this);
	for (TView *view = iter.FirstSubView(); iter.More(); view = iter.NextSubView())
	{
		TArticleManageView *amv = (TArticleManageView *)view;
		amv->SetNewFont(ts);
	}
}

void TArticleListView::DeleteDisplayedArticles(Boolean isDoneReadingArticles)
{
	if (!fArticleIndex)
		return; // has nothing
	if (isDoneReadingArticles)
	{
		TLongintList *theIDList = nil;
		VOLATILE(theIDList);
		FailInfo fi;
		if (fi.Try())
		{	
			theIDList = GetListOfAllArticles();
			{
				CLongintIterator iter(theIDList);
				for (long id = iter.FirstLong(); iter.More(); id = iter.NextLong())
					fOldArticleStatus->SetMinStatus(id, fArticleStatus->GetStatus(id));
			}
			theIDList->Free(); theIDList = nil;
			fi.Success();
		}
		else // fail
		{
			FreeIfObject(theIDList); theIDList = nil;
			fi.ReSignal();
		}
		fDiscListView->UpdateDiscussion(fDiscIndex);
	}
}

//============================================== DRAWING ================================
void TArticleListView::DrawViewSeparator(VCoordinate vOffset)
{
	short vOff = short(vOffset);
	MoveTo(0, vOff + 3);
	PenPat(&qd.gray);
	PenSize(3, 3);
	Line(1000, 0);
	MoveTo(0, vOff + 4);
	PenPat(&qd.black);
	PenSize(1, 1);
	Line(1000, 0);
	PenNormal();
}


//======================== MANAGEMENT ==================================================
void TArticleListView::SetArticleList(TLongintList *discIndexList)
{
#if qDebug
	if (!IsObject(discIndexList))
		ProgramBreak("discIndexList is not object");
#endif
	if (discIndexList->GetSize() == 0) {
#if qDebug
		ProgramBreak("No articles in discIndexList");
#endif
		return;
	}
	// throw old stuff away
	FreeIfObject(fDiscIndexList); fDiscIndexList = nil;	
	// use new stuff
	fDiscIndexList = discIndexList;
	UpdateList(1, ArticleShowType(gPrefs->GetShortPrefs('artS')));
}

TLongintList *TArticleListView::GetListOfAllArticles()
{
	return fDiscList->GetArticleIDList(fDiscIndex);
}

TLongintList *TArticleListView::GetListOfExpandedArticles()
{
	TLongintList *lList = new TLongintList();
	lList->ILongintList();
	CArticleViewIterator iter(this);
	for (TArticleView *articleView = iter.FirstArticleView(); iter.More(); articleView = iter.NextArticleView())
		lList->InsertLast(articleView->GetArticleID());
	return lList;
}

TLongintList *TArticleListView::GetListOfSelectedArticles()
{
	TLongintList *lList = new TLongintList();
	lList->ILongintList();
	CSelectedArticleViewIterator iter(this);
	for (TArticleView *articleView = iter.FirstArticleView(); iter.More(); articleView = iter.NextArticleView())
		lList->InsertLast(articleView->GetArticleID());
	return lList;
}

void TArticleListView::SetDiscussionDisplayed(long index)
{
	index = MinMax(1, index, fDiscIndexList->GetSize());
	if (fArticleIndex != index) 
		UpdateList(index, ArticleShowType(gPrefs->GetShortPrefs('artS')));
}

void TArticleListView::UpdateList(ArrayIndex newArticleListIndex, ArticleShowType whatToShow)
{
	KillUpdateListThread();
	if (newArticleListIndex != fArticleIndex)
	{
		DeleteDisplayedArticles(true);
		{
			// stuff from DeleteDisplayedArticles, moved to here as I got a serious crash
			// when resizing view in Close()
			while (fSubViews && fSubViews->GetSize() > 0)
				fSubViews->Last()->Free();
			//	AdjustSubViewFrames();
			SetFrame(VRect(0, 0, 0, 0), kRedraw);
		}
		fArticleIndex = newArticleListIndex;
	}
	TUpdateArticleViewCommand *cmd = new TUpdateArticleViewCommand();
	cmd->IUpdateArticleViewCommand(this, whatToShow);
	fUpdateThreadList->ExecuteCommand(cmd, "TUpdateArticleViewCommand (TArticleListView)");
#if qDebugUpdateListThread
	fprintf(stderr, "Created new UpdateListThread\n");
#endif
}

void TArticleListView::DoTheUpdateList(ArticleShowType whatToShow)
{
	TLongintList *theIDList = nil;
	VOLATILE(theIDList);
	FailInfo fi;
	if (fi.Try())
	{	
		fDiscIndex = fDiscIndexList->At(fArticleIndex);
		fPrefDiscArrow->SetEnable(fArticleIndex > 1);
		fNextDiscArrow->SetEnable(fArticleIndex < fDiscIndexList->GetSize());

		theIDList = GetListOfAllArticles();
		CStr255 title;
		// find name for it
		fDiscList->GetName(fDiscIndex, title);

		// display window
		fWindow->SetTitle(title);
		fWindow->Open();

		long maxIndex = theIDList->GetSize();
		for (ArrayIndex index = 1; index <= maxIndex; index++)
		{
			long theID = theIDList->At(index);
			Boolean showIt = ShouldExpandArticle(whatToShow, index, theID);
			fAddSeparatorAtBottom = index < maxIndex;
			if (fSubViews && fSubViews->GetSize() >= index)
				ExpandExistingArticleView(index, showIt);
			else
				CreateNewArticleView(theID, showIt);
			if (showIt)
				gCurThread->CheckYield();
		}
		theIDList->Free(); theIDList = nil;
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(theIDList); theIDList = nil;
		fi.ReSignal();
	}
}

void TArticleListView::ExpandExistingArticleView(ArrayIndex index, Boolean showIt)
{
	FailInfo fi;
	if (fi.Try())
	{
		TArticleManageView *amv = (TArticleManageView*) fSubViews->At(index);
		amv->BuildDisplay(showIt);
		fi.Success();
	}
	else // fail
	{
		if (fi.error < kFirstNntpError && fi.error > kLastNntpError) // negative!
		{
			// silent, just don't show the article
			if (showIt)
			{
				TArticleManageView *amv = (TArticleManageView*) fSubViews->At(index);
				amv->BuildDisplay(false);
			}
		}
		else
			fi.ReSignal(); // aborting, must resignal
	}
}

void TArticleListView::CreateNewArticleView(long theID, Boolean showIt)
{
	TArticle *article = nil;
	VOLATILE(article);
	VOLATILE(showIt);
	FailInfo fi;
	if (fi.Try())
	{
		if (showIt)
		{
			CStr255 groupDotName;
			fDoc->GetGroupDotName(groupDotName);
			article = gArticleCache->GetArticle(groupDotName, theID);
		}
		fi.Success();
	}
	else // fail
	{
		if (fi.error < kFirstNntpError && fi.error > kLastNntpError) // negative!
		{
			// silent, just don't show the article
			showIt = false;
		}
		else
			fi.ReSignal(); // aborting, must resignal
	}
	TArticleManageView *amv = new TArticleManageView();
	amv->IArticleManageView(fDoc, this, fSize.v, theID, article, showIt);
}

Boolean TArticleListView::ShouldExpandArticle(ArticleShowType whatToShow, ArrayIndex index, long theID)
{
	if (theID < fDoc->GetFirstArticleID() || theID > fDoc->GetLastArticleID())
		return false;
	Boolean showIt = true;
	switch (whatToShow)
	{
		case kShowUnreadArticles:
			return fOldArticleStatus->GetStatus(theID) <= kArticleSeen;

		case kShowNewArticles:
			return fOldArticleStatus->GetStatus(theID) == kArticleNew;
		
		case kShowAllArticles:
			return true;
			
		case kShowFirstArticleOnly:
			return index == 1;
			
		case kShowNoArticles:
			return false;

#if qDebug
		default:
			fprintf(stderr, "Invalid ArticleStatus: %ld\n", long(whatToShow));
			ProgramBreak(gEmptyString);
			return false;
#endif
	}
}
//--------------------------------------------------------
Handle TArticleListView::GetSelectionAsQuotedTextForMail()
{
	short quoteID = kArticleQuoteLine;
	CSelectedArticleViewIterator iter(this);
	long numSelected = 0;
	for (TArticleView *articleView = iter.FirstArticleView(); iter.More(); articleView = iter.NextArticleView())
		numSelected++;
	if (numSelected == 1)
		quoteID = kArticleQuoteYouLine;
	return GetSelectionAsQuoteText(true, true, quoteID);
}

Handle TArticleListView::GetSelectionAsTextForClipboard(Boolean asQuote, Boolean addHeader)
{
	short quoteID = kArticleQuoteLine;
	return GetSelectionAsQuoteText(asQuote, addHeader, quoteID);
}


Handle TArticleListView::GetSelectionAsQuoteText(Boolean asQuote, 
											Boolean addHeader, short quoteID)
{
	Handle h = nil;
	VOLATILE(h);
	FailInfo fi;
	if (fi.Try())
	{
		h = NewPermHandle(0);
		// block for failure handling
		{
			CSelectedArticleViewIterator iter(this);
			Boolean gotOne = false;
			for (TArticleView *articleView = iter.FirstArticleView(); iter.More(); articleView = iter.NextArticleView())
			{
				if (gotOne)
					AppendStringToHandle("\n", h);
				gotOne = true;
				articleView->AppendSelectionAsTextForClipboard(h, asQuote, addHeader, quoteID);
			}
		}
		fi.Success();
		return h;
	}
	else // fail
	{
		h = DisposeIfHandle(h);
		fi.ReSignal();
	}
}


void TArticleListView::CancelSelectedArticles()
{
	if (MacAppAlert(phCancelArticle, nil) != ok)
		Failure(0, 0);
	FailInfo fi;
	if (fi.Try())
	{
		{ // block for list iter failure handling
			CStr255 myEmail, s, realname, email;
			gPrefs->GetStringPrefs('@adr', myEmail);
			CSelectedArticleViewIterator iter(this);
			for (TArticleView *articleView = iter.FirstArticleView(); iter.More(); articleView = iter.NextArticleView())
			{
				articleView->GetArticle()->GetHeader("From", s);
				GetAuthorName(s, realname, email);
				if (!EqualString(myEmail, email, false, false))
					FailOSErr(errCancelNotYourArticle);
			}
			for (articleView = iter.FirstArticleView(); iter.More(); articleView = iter.NextArticleView())
			{
				articleView->GetArticle()->GetHeader("Message-ID", s);
				TCancelArticleCommand *cmd = new TCancelArticleCommand();
				cmd->ICancelArticleCommand(s);
				gApplWideThreads->ExecuteCommand(cmd, "TCancelArticleCommand (TArticleListView)");
			}
		}
		fi.Success();
	}
	else // fail
	{
		FailNewMessage(fi.error, fi.message, messageCancelArticle);
	}
}

//================================================================
pascal Boolean TArticleListView::HandleMouseDown(const VPoint& theMouse,
								TToolboxEvent* event, CPoint hysteresis)
{
	if (fIsClosing)
		return false;
	if (!event->IsCommandKeyPressed() && !event->IsShiftKeyPressed() &&
			!event->IsOptionKeyPressed() && !event->IsControlKeyPressed())
	{
		// deselect all other TArticleViews

// burde vist kun laves, hvis man laver mousedown i noget text som
// kan v�lges, og ikke fx i en triangle

		TArticleView *selectedArticleView = nil;
		CArticleViewIterator iter(this);
		for (TArticleView *articleView = iter.FirstArticleView(); iter.More(); articleView = iter.NextArticleView())
		{
			TView *view = articleView;
			while (view->fSuperView != this)
				view = view->fSuperView;
			VPoint subViewPt = theMouse;
			view->SuperToLocal(subViewPt);
			if (view->ContainsMouse(subViewPt))
			{
				selectedArticleView = articleView;
				break;
			}
		}
		for (articleView = iter.FirstArticleView(); iter.More(); articleView = iter.NextArticleView())
		{
			if (articleView != selectedArticleView)
				articleView->SetEmptySelection(kRedraw);
		}
	}
	return inherited::HandleMouseDown(theMouse, event, hysteresis);
}

pascal void TArticleListView::DoKeyEvent(TToolboxEvent *event)// override 
{
	if (!IsEnabled() || fIsClosing)
	{
		if (qDebug && fIsClosing)
			fprintf(stderr, "Ignored: is closing\n");
		inherited::DoKeyEvent(event);
		return;
	}
	switch (event->fCharacter)
	{
		case chLeft:
			DoMenuCommand(cPreviousDiscussion);
			break;

		case chRight:
			DoMenuCommand(cNextDiscussion);
			break;

		case chUp:
			if (event->IsOptionKeyPressed() && !event->IsCommandKeyPressed())
				DoMenuCommand(cPreviousArticle);
			else
				inherited::DoKeyEvent(event);
			break;
		case chDown:
			if (event->IsOptionKeyPressed() && !event->IsCommandKeyPressed())
				DoMenuCommand(cNextArticle);
			else
				inherited::DoKeyEvent(event);
			break;
				
		case chSpace:
			DoMenuCommand(cMultiSuperNextKey);
			break;

		case chBackspace:
			{
				CArticleViewIterator iter(this);
				for (TArticleView *articleView = iter.FirstArticleView(); iter.More(); articleView = iter.NextArticleView())
					articleView->SetEmptySelection(kRedraw);
			}
			break;

		default:
			inherited::DoKeyEvent(event);
			break;
	}
}

pascal void TArticleListView::DoMenuCommand(CommandNumber aCommandNumber)
{
	if (!IsEnabled() || fIsClosing)
	{
		if (qDebug && fIsClosing)
			fprintf(stderr, "Ignored: is closing\n");
		inherited::DoMenuCommand(aCommandNumber);
		return;
	}
//	ArticleShowType ast;
	switch (aCommandNumber)
	{
		case cPostFollowUpArticle: // most importent first!
			{
				Handle h = GetSelectionAsTextForClipboard(!IsOptionKeyDown(), !IsShiftKeyDown());
				TCreateFollowupCommand *command = new TCreateFollowupCommand();
				command->ICreateFollowupCommand(fDoc, fDiscIndex, h);
				PostCommand(command);
			}
			break;
		
		case cShowHideHeadersCommand:
			{
				fShowsHeaders = !fShowsHeaders;
				gPrefs->SetBooleanPrefs('Head', fShowsHeaders);
				CArticleViewIterator iter(this);
				for (TArticleView *articleView = iter.FirstArticleView(); iter.More(); articleView = iter.NextArticleView())
					articleView->UpdateDisplay();
			}
			break;
		
		case cUseROT13:
			{
				fUseROT13 = !fUseROT13;
				CArticleViewIterator iter(this);
				for (TArticleView *articleView = iter.FirstArticleView(); iter.More(); articleView = iter.NextArticleView())
					articleView->SetUseROT13(fUseROT13);
			}
			break;
		

		case cShowAllArticles:
			UpdateList(fArticleIndex, kShowAllArticles);
			break;

		case cShowOnlyFirstArticle:
			UpdateList(fArticleIndex, kShowFirstArticleOnly);
			break;

		case cShowOnlyUnreadArticles:
			UpdateList(fArticleIndex, kShowUnreadArticles);
			break;

		case cShowOnlyNewArticles:
			UpdateList(fArticleIndex, kShowNewArticles);
			break;

		case cShowNoneArticles:
			UpdateList(fArticleIndex, kShowNoArticles);
			break;

		case cSave:
		case cSaveAs:
		case cSaveCopy:
			{
				TSaveArticlesCommand *aCommand = new TSaveArticlesCommand();
				aCommand->ISaveArticlesCommand(cSaveAs, fDoc, GetListOfExpandedArticles());
				gApplWideThreads->ExecuteCommand(aCommand, "TSaveArticlesCommand (TArticleListView)");
				break;
			}

		case cExtractBinaries:
			{
				TLongintList *lList;
				if (IsOptionKeyDown())
					lList = GetListOfSelectedArticles();
				else
					lList = GetListOfAllArticles();
				ExtractBinaries(cExtractBinaries, fDoc, lList);
				break;
			}

		case cNextDiscussion:
			{
				TShowDiscJunkCommand *aCommand = new TShowDiscJunkCommand();
				aCommand->IShowDiscJunkCommand(this, fArticleIndex + 1);
				PostCommand(aCommand);
				break;
			}
		case cPreviousDiscussion:
			{
				TShowDiscJunkCommand *aCommand = new TShowDiscJunkCommand();
				aCommand->IShowDiscJunkCommand(this, fArticleIndex - 1);
				PostCommand(aCommand);
				break;
			}
			
		case cMultiSuperNextKey:
			{
				TScrollerScrollBar *sb = fScroller->fScrollBars[vSel];
				if (sb->fLongVal < sb->fLongMax)
				{
					fScroller->DoPageDown();
					break;
				}
				long maxIndex = fDiscIndexList->GetSize();
				if (fArticleIndex < maxIndex)
				{
					DoMenuCommand(cNextDiscussion);
					break;
				}
				else
				{
					TDiscListView *dlv = fDoc->GetDiscListView();
					if (dlv)
						dlv->SetEmptySelection(kHighlight);
					TCloseWindowCommand *cmd = new TCloseWindowCommand();
					cmd->ICloseWindowCommand(aCommandNumber, fWindow);
					PostCommand(cmd);
				}
			}
			break;

		case cCopy:
			{
				TCopyArticleTextCommand *command = new TCopyArticleTextCommand();
				command->ICopyArticleTextCommand(this, !IsOptionKeyDown(), !IsShiftKeyDown());
				PostCommand(command);
			}
			break;

		case cSelectAll:
			{
				CArticleViewIterator iter(this);
				for (TArticleView *articleView = iter.FirstArticleView(); iter.More(); articleView = iter.NextArticleView())
					articleView->DoMenuCommand(cSelectAll);
			}
			break;

		case cMailLetter:
			{
				TMailToAuthorCommand *command = new TMailToAuthorCommand();
				command->IMailToAuthorCommand(fDoc, this, !IsOptionKeyDown(), !IsShiftKeyDown());
				PostCommand(command);
			}
			break;
		
		case cStoreNotes:
			{
				TStoreNotesCommand *cmd = new TStoreNotesCommand();
				cmd->IStoreNotesCommand(this);
				PostCommand(cmd);
			}
			break;

		case cCancelArticle:
			CancelSelectedArticles();
			break;
			
		default:
			{
				TextStyle ts = fTextStyle;
				if (gNewsAppl->HandleFontMenu(aCommandNumber, ts))
				{
					fTextStyle = ts;
					gPrefs->SetTextStylePrefs('TSar', ts);
					ChangedFont();
				}
				else
					inherited::DoMenuCommand(aCommandNumber);
			}
	}
}

pascal void TArticleListView::DoSetupMenus()
{
	if (fIsClosing)
	{
		inherited::DoSetupMenus();
		Enable(cRevert, false);
		return;
	}
	Boolean lowSpace = MemSpaceIsLow();
	Boolean anySelected = false;
	CSelectedArticleViewIterator iter(this);
	for (TArticleView *articleView = iter.FirstArticleView(); iter.More(); articleView = iter.NextArticleView())
	{
		anySelected = true;
		break;
	}
	TextStyle ts = fTextStyle;
	gNewsAppl->EnableFontMenu(ts);
	Enable(cSave, true);
	Enable(cSaveAs, true);
	Enable(cSaveCopy, true);
	Enable(cExtractBinaries, !lowSpace);
	Enable(cSelectAll, true);
	EnableCheck(cShowHideHeadersCommand, true, fShowsHeaders);
	EnableCheck(cUseROT13, true, fUseROT13);
	
	Enable(cCopy, !lowSpace && anySelected);

	Enable(cPostFollowUpArticle, !lowSpace && anySelected);
	Enable(cMailLetter, !lowSpace && anySelected);
	Enable(cCancelArticle, !lowSpace && anySelected);

	Enable(cShowAllArticles, true);
	Enable(cShowOnlyFirstArticle, true);
	Enable(cShowOnlyUnreadArticles, true);
	Enable(cShowOnlyNewArticles, true);
	Enable(cShowNoneArticles, true);

	Enable(cStoreNotes, !lowSpace && anySelected);

	inherited::DoSetupMenus();
	Enable(cRevert, false);
}
