// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleManageView.h

#define __UARTICLEMANAGEVIEW__

class TArticleView;
class TGroupDoc;
class TArticle;
class TTriangleControl;

#ifndef __FONTTOOLS__
#include "FontTools.h"
#endif

class TArticleManageView : public TView
{
	public:
		pascal void DoEvent(EventNumber eventNumber, TEventHandler* source, TEvent* event);
		pascal void Draw(const VRect& area);
		pascal void SubViewChangedFrame(TView* theSubView,
											const VRect& oldFrame, const VRect& newFrame, Boolean invalidate);

		TArticleView *GetArticleView(); // can be nil
		void SetNewFont(const TextStyle &textStyle);
		void BuildDisplay(Boolean expand, TArticle *article = nil);
		
		pascal void Initialize();
		void IArticleManageView(TGroupDoc *doc, TView *superView, VCoordinate top,
											long articleID, TArticle *article, Boolean showExpanded);
		pascal void Free();
	private:
		TGroupDoc *fDoc;
		long fArticleID;
		TArticleView *fArticleView;
		TTriangleControl *fTriControl;
		Boolean fIsExpanded;
		StandardGridViewTextStyle fGridViewTextStyle;

		void DoShowArticleView(TArticle *article);
		void DoShowHeaderView();
};
