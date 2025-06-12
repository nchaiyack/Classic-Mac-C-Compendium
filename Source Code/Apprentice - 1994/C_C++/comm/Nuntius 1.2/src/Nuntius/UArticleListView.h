// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleListView.h

#define __UARTICLELISTVIEW__

#ifndef __UVIEWLISTVIEWVIEW__
#include "UViewListView.h"
#endif

#ifndef __UARTICLESTATUS__
#include "UArticleStatus.h"
#endif

class TNavigatorArrowView;
class TArticleStatus;
class PDiscList;
class TDiscListView;
class TGroupDoc;
class TTextScroller;
class TThreadList;
class TShowDiscJunkCommand;
class TSaveArticlesCommand;
class TMultiSuperNextKeyCommand;
class TArticleTextSelectCommand;
class TCopyArticleTextCommand;
class TUpdateArticleViewCommand;
class TStoreNotesCommand;

class TArticleListView : public TViewListView
{
	public:
		pascal void DoCalcPageStrips(VPoint& pageStrips);

		pascal void DoKeyEvent(TToolboxEvent *event);
		pascal void DoMenuCommand(CommandNumber aCommandNumber);
		pascal void DoSetupMenus();
		pascal Boolean HandleMouseDown(const VPoint& theMouse,
									  TToolboxEvent* event, CPoint hysteresis);

		void SetArticleList(TLongintList *discIndexList);
		// TArticleView _owns_ the lists

		TLongintList *GetListOfExpandedArticles(); // caller owns list
		TLongintList *GetListOfAllArticles(); // caller owns list
		TLongintList *GetListOfSelectedArticles(); // caller owns list

		Handle GetSelectionAsQuotedTextForMail();
		Handle GetSelectionAsTextForClipboard(Boolean asQuote, Boolean addHeader);
		Handle GetSelectionAsQuoteText(Boolean asQuote, 
													Boolean addHeader, short quoteID);

		TArticleListView();
		pascal void Initialize();
		pascal void ReadFields(TStream *aStream);
		pascal void DoPostCreate(TDocument *itsDocument);
		pascal void Close();
		pascal void Free();
	private:
		friend class TShowDiscJunkCommand;
		friend class TSaveArticlesCommand;
		friend class TMultiSuperNextKeyCommand;
		friend class TArticleTextSelectCommand;
		friend class TCopyArticleTextCommand;
		friend class TUpdateArticleViewCommand;
		friend class TStoreNotesCommand;

		TWindow *fWindow;
		TNavigatorArrowView *fPrefDiscArrow, *fNextDiscArrow;
		TTextScroller *fScroller;

		TThreadList *fUpdateThreadList;
		Boolean fIsClosing;
		TGroupDoc *fDoc;
		TArticleStatus *fArticleStatus, *fOldArticleStatus;

		PDiscList *fDiscList;
		TDiscListView *fDiscListView;
		ArrayIndex fDiscIndex; // into fDiscList
		TLongintList *fDiscIndexList;
		ArrayIndex fArticleIndex; // into fDiscIndexList
		TextStyle fTextStyle;
		Boolean fShowsHeaders;
		Boolean fUseROT13;

		VCoordinate GetSeparatorHeight();
		void CheckNewFrame(VRect &newFrame);	
		void DrawViewSeparator(VCoordinate vOffset);

		void DeleteDisplayedArticles(Boolean isDoneReadingArticles);
		void KillUpdateListThread();
		Boolean ShouldExpandArticle(ArticleShowType whatToShow, ArrayIndex index, long theID);

		void ChangedFont();
		void SetDiscussionDisplayed(long index);
		void UpdateList(ArrayIndex newArticleListIndex, ArticleShowType whatToShow);
		void DoTheUpdateList(ArticleShowType whatToShow);
		void CreateNewArticleView(long theID, Boolean showIt);
		void ExpandExistingArticleView(ArrayIndex index, Boolean showIt);
		void CancelSelectedArticles();
};
