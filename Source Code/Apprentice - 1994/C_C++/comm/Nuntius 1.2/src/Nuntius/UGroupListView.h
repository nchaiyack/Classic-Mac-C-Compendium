// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupListView.h

#define __UGROUPLISTVIEW__

#ifndef __UGRIDVIEW__
#include <UGridView.h>
#endif

class TGroupList;
class TGroupListDoc;
class TGroupTree;
class TGroupTracker;
class TListTypeName;

#ifndef __FONTTOOLS__
#include "FontTools.h"
#endif

class TGroupListView : public TGridView 
{
	public:
		pascal void DoKeyEvent(TToolboxEvent *event);
		pascal void DrawRangeOfCells(GridCell startCell,
										 GridCell stopCell, const VRect &aRect);
		pascal void DrawCell(GridCell aCell, const VRect &aRect);
		void GetCellTextRect(GridCell aCell, CRect &r);
		pascal GridViewPart IdentifyPoint(const VPoint& thePoint, GridCell& aCell);
		pascal void HighlightCells(RgnHandle theCells,
									  HLState fromHL, HLState toHL);
		void HighlighOneCell(GridCell aCell, HLState fromHL, HLState toHL);
		void UpdateGroupStatus(const CStr255 &dotName, Boolean hasNewArticles);

		TGroupList *GetGroupList();
		void UpdateList();
		void AddGroup(CStr255 &name);
		void DeleteGroup(short line);
		void DeleteSelection();

		TGroupList *GetSelectionAsList();
		void AddGroupsFromListBefore(ArrayIndex index, TGroupList *groupList);
		void DeleteGroupsFromList(TGroupList *groupList);

		pascal void DoMouseCommand(VPoint &theMouse,
									   TToolboxEvent *event, CPoint hysteresis);
		pascal void DoMenuCommand(CommandNumber aCommandNumber);
		pascal void DoSetupMenus();
		pascal void ShowReverted();

		TGroupListView();
		pascal void Close();
		pascal void Initialize();
		pascal void ReadFields(TStream *aStream);
		pascal void DoPostCreate(TDocument *itsDocument);
		pascal void Free();
	private:
		friend class TGroupTracker;
		friend class TListTypeName;
		
		TWindow *fWindow;
		TGroupList *fGroupList;
		TGroupListDoc *fDoc;
		StandardGridViewTextStyle fGridViewTextStyle;
		unsigned long fLastTypeKeyTick;
		CStr255 fKeyTypeChars;

		void OpenSelection(Boolean updateDatabase);
		void ChangedFont();
};
