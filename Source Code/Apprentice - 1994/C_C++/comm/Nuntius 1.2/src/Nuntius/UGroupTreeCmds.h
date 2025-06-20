// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupTreeCmds.h

#define __UGROUPTREECMDS__

class TGroupTreeView;
class TGroupTree;
class TGroupTreeDoc;

#ifndef __UGRIDVIEW__
#include <UGridView.h>
#endif

#ifndef __UGROUPVIEWCMDS__
#include "UGroupViewCmds.h"
#endif

class TExpandGroupTracker : public TTracker
{
	public:
		pascal void DoIt(); // Override
			
		pascal void TrackFeedback(TrackPhase aTrackPhase,
										const VPoint &anchorPoint,
										const VPoint &previousPoint,
										const VPoint &nextPoint,
										Boolean mouseDidMove,
										Boolean turnItOn); // Override
			
		pascal TTracker *TrackMouse(TrackPhase aTrackPhase,
											 VPoint &anchorPoint,
											 VPoint &previousPoint,
											 VPoint &nextPoint,
											 Boolean mouseDidMove); // Override

		TExpandGroupTracker();
		pascal void Initialize();
		pascal void IExpandGroupTracker(
												TGroupTreeView *groupTreeView,
												const VPoint &itsMouse,
												GridCell cell,
												Boolean isExpanded,
												Boolean wayDown,
												const Rect &cellRect);
	protected:
		TGroupTreeView *fGroupTreeView;
		GridCell fCell;
		Boolean fWasExpanded;
		CRect fCellRect;
		Boolean fLastHighlighted;
		Boolean fWayDown;
};

class TSubscribeGroupTracker : public TGroupTracker
{	
	public:
			
		TSubscribeGroupTracker();
		virtual pascal void Initialize();
		virtual pascal void ISubscribeGroupTracker(
												TGroupTreeView *groupTreeView,
												const VPoint &localMouse,
												Boolean makingCopy);
		virtual pascal void Free();
	protected:
		virtual void MakeGrayPict();
		virtual void CreateListOfDraggedGroups();

	private:
		TGroupTreeView *fGroupTreeView;
		Boolean fHasPrevCell;
		long fPrevLevel;
		ArrayIndex fPrevIndex;
		CRect fPrevIconRect;
		
		void DrawGroupIcon(ArrayIndex line, long level, CRect textRect, CRect iconRect);
};

class TTreeTypeName : public TGroupViewTypeNameCommand
{
	public:
		TTreeTypeName();
		virtual pascal void Initialize();
		void ITreeTypeName(TGroupTreeView *gtv, TLongintList *expandData, ArrayIndex windowFolderIndex, TToolboxEvent *event);
		virtual pascal void Free();
	protected:
		TGroupTreeView *fGroupTreeView;
		TGroupTree *fGroupTree;
		TLongintList *fExpandData;
		ArrayIndex fWindowFolderIndex;

		virtual void SetUp();
		virtual void SetDown();
		virtual void GetLineText(ArrayIndex line, CStr255 &text);
};

class TTreeTabKeyCommand : public TGroupViewTabKeyCommand
{
	public:
		TTreeTabKeyCommand();
		virtual pascal void Initialize();
		void ITreeTabKeyCommand(TGroupTreeView *gtv, TLongintList *expandData, ArrayIndex windowFolderIndex, Boolean forward);
		virtual pascal void Free();
	protected:
		TGroupTreeView *fGroupTreeView;
		TGroupTree *fGroupTree;
		TLongintList *fExpandData;
		ArrayIndex fWindowFolderIndex;

		virtual void GetLineText(ArrayIndex line, CStr255 &text);
};

class TUpdateListOfAllGroupsCommand : public TCommand
{
	public:
		pascal void DoIt();
		
		pascal void Initialize();
		void IUpdateListOfAllGroupsCommand(TGroupTreeDoc *gtd, Boolean rebuild);
		pascal void Free();
	private:
		TGroupTreeDoc *fGroupTreeDoc;
		Boolean fRebuild;
};
