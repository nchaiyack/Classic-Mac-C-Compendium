// Copyright � 1992-1994 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleView.h

#define __UARTICLEVIEW__

#if qDebug
#include "UArticleNewView.h"
#else

#ifndef __UGRIDVIEW__
#include <UGridView.h>
#endif

class TGroupDoc;
class CArticleEntry;
class TArticle;
class TArticleTextSelectCommand;
class TCopyArticleTextCommand;

#ifndef __FONTTOOLS__
#include "FontTools.h"
#endif

class TArticleView : public TGridView
{
	DeclareClass(TArticleView);
	public:
		void DrawRangeOfCells(GridCell startCell,
										GridCell stopCell,
										const VRect &aRect);
		void DrawCell(GridCell aCell, const VRect &aRect);
		void DrawLine(short anItem, CRect &r);

		void DoMouseCommand(VPoint &theMouse,
									   TToolboxEvent *event,
									   CPoint hysteresis);

		void SetNewFont(const TextStyle &textStyle);
		CArticleEntry *GetArticleEntry();
		TArticle *GetArticle();
		void AppendSelectionAsTextForClipboard(Handle h, Boolean asQuote, 
													Boolean addHeader, short quoteTemplateID);
		void UpdateDisplay();
		void SetUseROT13(Boolean useROT13);
		
		TArticleView();
		void Close();
		void IArticleView(TGroupDoc *doc, TView *superView, VPoint location,
											CArticleEntry *articleEntry);
		void Free();
	private:
		friend class TArticleTextSelectCommand;
		friend class TCopyArticleTextCommand;

		CArticleEntry *fArticleEntry;
		TArticle *fArticle;
		TGroupDoc *fDoc;
		ArrayIndex fBodyStartLineNo;
		Boolean fShowsAllHeaders;
		Boolean fUseROT13;
		Boolean fUseJapaneseFont;

		StandardGridViewTextStyle fGridViewTextStyle;
		
		void GetLineText(short lineNo, Boolean asQuote, CStr255 &text);
		Boolean DoGetLineText(short lineNo, Boolean asQuote, CStr255 &text);
		void GetQuoteFromTextLine(CStr255 &text, short quoteTemplateID);
		void SetArticleDisplayed(long articleID);
		void DoOptionClick(VPoint &theMouse, GridCell aCell);
};
#endif
