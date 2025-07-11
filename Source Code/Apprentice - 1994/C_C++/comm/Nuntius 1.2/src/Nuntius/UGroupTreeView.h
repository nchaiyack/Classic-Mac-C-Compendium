// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupTreeView.h

#define __UGROUPTREEVIEW__

#ifndef __UGRIDVIEW__
#include <UGridView.h>
#endif

#ifndef __FASTBITS__
#include "FastBits.h"
#endif

typedef SicnFastBits TreeSicn[2];

class TGroupTreeDoc;
class TGroupTree;
class TExpandGroupTracker;
class TSubscribeGroupTracker;
class TTreeTypeName;
class TTreeTabKeyCommand;

#ifndef __FONTTOOLS__
#include "FontTools.h"
#endif

class TGroupTreeView : public TGridView 
{
	public:
		pascal void DoKeyEvent(TToolboxEvent *event);
		pascal void DoCommandKeyEvent(TToolboxEvent *event);
		pascal void HighlightCells(RgnHandle theCells, HLState fromHL, HLState toHL);
		pascal GridViewPart IdentifyPoint(const VPoint& thePoint, GridCell& aCell);

		void GetCellInformation(GridCell aCell, 
							long &level, long &noVisibleSubLines,
							Boolean &isFolder,
							CRect &triangleRect, CRect &iconRect, CRect &textRect,
							CStr255 &text);

		pascal void DoShowHelp(const VPoint& localPoint, RgnHandle helpRegion);
		pascal void DrawCell(GridCell aCell, const VRect &aRect);
		pascal void DrawRangeOfCells(GridCell startCell,
										 GridCell stopCell,
										 const VRect &aRect);
		void DrawTrackedTriangle(GridCell aCell, Boolean asExpanded, Boolean halfExpanded, Boolean highlight);
		void ExpandCompactCell(GridCell aCell, Boolean doExpand, Boolean wayDown);
		void ExpandCompactSelection(Boolean doExpand, Boolean wayDown);

		void UpdateList();
		ArrayIndex GetWindowFolderIndex();
		TGroupTree *GetGroupTree();
		
		void WriteWindowInfo(TStream *aStream);
		void ReadWindowInfo(TStream *aStream);
		
		pascal void DoMouseCommand(VPoint &theMouse, TToolboxEvent *event, CPoint hysteresis);
		pascal void DoMenuCommand(CommandNumber aCommandNumber);
		pascal void DoSetupMenus();
		pascal void ShowReverted();

		void SetWindowFolder(ArrayIndex windowFolderIndex, TGroupTreeView *openFromView = nil);
		
		TGroupTreeView();
		pascal void Close();
		pascal void Initialize();
		pascal void ReadFields(TStream *aStream);
		pascal void DoPostCreate(TDocument *itsDocument);
		pascal void Free();
	private:
		friend class TExpandGroupTracker;
		friend class TSubscribeGroupTracker;
		friend class TTreeTypeName;
		friend class TTreeTabKeyCommand;
		
		TWindow *fWindow;
		TGroupTreeDoc *fGroupTreeDoc;
		TGroupTree *fGroupTree;
		TLongintList *fExpandData;
		ArrayIndex fWindowFolderIndex;
		long fLevelOffset;

		StandardGridViewTextStyle fGridViewTextStyle;
		BitMap *fPortBitsP;
		short fScreenDepth;
		short fCellHeight;
		
		unsigned long fLastTypeKeyTick;
		CStr255 fKeyTypeChars;

		void OpenSelection(Boolean updateDatabase);
		void DoSuperKey(TToolboxEvent *event);
		
		void ChangedFont();
		void UpdateDrawInfo();
		void DrawSmallIcon(const SicnFastBits *sicn, CRect iconRect, Boolean drawSelected);
		void DrawTheCell(GridCell aCell);
		void HighlighTheCell(GridCell aCell, HLState fromHL, HLState toHL);

		void DoTheGetCellInformation(GridCell aCell,
							ArrayIndex &nodeIndex, long &level, ArrayIndex &noVisibleSubLines,
							Boolean &isFolder,
							CRect &triangleRect, CRect &iconRect, CRect &textRect,
							CStr255 &text);
};
