// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleView.h

#define __UARTICLEVIEW__

#ifndef __UGRIDVIEW__
#include <UGridView.h>
#endif

class TGroupDoc;
class TArticle;
class TArticleTextSelectCommand;
class TCopyArticleTextCommand;

#ifndef __FONTTOOLS__
#include "FontTools.h"
#endif

class TArticleView : public TGridView
{
	public:
		pascal void DrawRangeOfCells(GridCell startCell,
										GridCell stopCell,
										const VRect &aRect);
		pascal void DrawCell(GridCell aCell, const VRect &aRect);
		void DrawLine(short anItem, CRect &r);

		pascal void DoMouseCommand(VPoint &theMouse,
									   TToolboxEvent *event,
									   CPoint hysteresis);

		pascal void DoKeyEvent(TToolboxEvent *event);
		pascal void DoMenuCommand(CommandNumber aCommandNumber);
		pascal void DoSetupMenus();

		void SetNewFont(const TextStyle &textStyle);
		long GetArticleID();
		TArticle *GetArticle();
		void AppendSelectionAsTextForClipboard(Handle h, Boolean asQuote, 
													Boolean addHeader, short quoteTemplateID);
		void UpdateDisplay();
		void SetUseROT13(Boolean useROT13);
		
		TArticleView();
		pascal void Close();
		pascal void Initialize();
		void IArticleView(TGroupDoc *doc, TView *superView, VPoint location,
											long articleID, TArticle *article);
		pascal void ReadFields(TStream *aStream);
		pascal void DoPostCreate(TDocument *itsDocument);
		pascal void Free();
	private:
		friend class TArticleTextSelectCommand;
		friend class TCopyArticleTextCommand;

		TArticle *fArticle;
		TGroupDoc *fDoc;
		long fArticleID;
		ArrayIndex fBodyStartLineNo;
		Boolean fShowsAllHeaders;
		Boolean fUseROT13;
		Boolean fUseJapaneseFont;

		StandardGridViewTextStyle fGridViewTextStyle;
		
		void GetLineText(short lineNo, Boolean asQuote, CStr255 &text);
		Boolean DoGetLineText(short lineNo, Boolean asQuote, CStr255 &text);
		void GetQuoteFromTextLine(CStr255 &text, short quoteTemplateID);
		void SetArticleDisplayed(long articleID);
};
