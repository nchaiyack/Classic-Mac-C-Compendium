// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UDiscListView.h

#define __UDISCLISTVIEW__

#ifndef __UGRIDVIEW__
#include <UGridView.h>
#endif

#ifndef __UARTICLESTATUS__
#include "UArticleStatus.h"
#endif

class TGroupDoc;
class PDiscList;
class TTextScroller;

#ifndef __FONTTOOLS__
#include "FontTools.h"
#endif

class TDiscListView : public TGridView 
{
	public:
		pascal void DrawRangeOfCells(GridCell startCell,
										GridCell stopCell, const VRect &aRect);
		pascal void DrawCell(GridCell aCell, const VRect &aRect);
		pascal void HighlightCells(RgnHandle theCells, HLState fromHL, HLState toHL);
		void HighlighOneCell(GridCell aCell, HLState fromHL, HLState toHL);

		pascal Boolean CanSelectCell(GridCell aCell);
		pascal void DoCalcPageStrips(VPoint& pageStrips);
		void UpdateList();
		void UpdateDiscussion(ArrayIndex discIndex);
		
		pascal void DoMouseCommand(VPoint &theMouse,
									   TToolboxEvent *event, CPoint hysteresis);
		pascal void DoMenuCommand(CommandNumber aCommandNumber);
		pascal void DoKeyEvent(TToolboxEvent *event);
		pascal void DoSetupMenus();
		pascal void DoShowHelp(const VPoint& localPoint, RgnHandle helpRegion);

		pascal void Close();
		
		TDiscListView();
		void Startup(const CStr255 &groupName);
		void CloseDown();
		pascal void Initialize();
		pascal void ReadFields(TStream *aStream);
		pascal void DoPostCreate(TDocument *itsDocument);
		pascal void Free();
	private:
		friend class CDiscListArticleIterator;
		
		TGroupDoc *fDoc;
		CStr255 fGroupName;
		TWindow *fWindow;
		TTextScroller *fScroller;
		long fFirstAvaibleID, fLastAvaibleID;
		
		StandardGridViewTextStyle fGridViewTextStyle;
		short fSubjectWidth, fSubjectHorzOffset;
		PatHandle fDiscSeparatorPattern;
		
		DiscussionShowType fDiscToShow;
		TArticleStatus *fArticleStatus, *fOldArticleStatus;
		PDiscList *fDiscList;
		TLongintList *fDiscIndexList;

		Boolean ShowDiscussion(ArrayIndex discIndex);
		void MarkSelectionAs(ArticleStatus newStatus);
		
		TLongintList *GetSelection(); // returns list of discussion index
		TLongintList *GetListOfSelectedArticles();
		void OpenSelection(Boolean removeSelection = false);
		void DoSuperKey();
		
		void ChangedFont();
		Boolean FindHelp(const VPoint &localPoint, VRect &helpRect, short &balloonVariant);
};
