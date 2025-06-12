// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupViewCmds.h

#define __UGROUPVIEWCMDS__

class TGroupListView;
class TGroupTreeView;
class TGroupTree;
class TGroupList;

#ifndef __UGRIDVIEW__
#include <UGridView.h>
#endif

class TGroupTracker : public TTracker
{	
	public:
		virtual pascal void DoIt(); // Override
		virtual pascal void DoFocus();
			
		virtual pascal void TrackConstrain(TrackPhase aTrackPhase,
										const VPoint &anchorPoint,
									  	const VPoint &previousPoint,
									    VPoint &nextPoint,
										Boolean mouseDidMove);
		virtual pascal void ConstrainOnce(Boolean didMouseMove);

		virtual pascal void TrackFeedback(TrackPhase aTrackPhase,
										const VPoint &anchorPoint,
										const VPoint &previousPoint,
										const VPoint &nextPoint,
										Boolean mouseDidMove,
										Boolean turnItOn); // Override
			
		virtual pascal TTracker *TrackMouse(TrackPhase aTrackPhase,
											 VPoint &anchorPoint,
											 VPoint &previousPoint,
											 VPoint &nextPoint,
											 Boolean mouseDidMove); // Override

		TGroupTracker();
		virtual pascal void Initialize();
		virtual pascal void IGroupTracker(
												CommandNumber itsCommandNumber,
												const VPoint &itsMouse,
												Boolean makingCopy);
		virtual pascal void Free();

	protected:
		PicHandle fGrayPictH;
		CPoint fMouseOffsetInGrayPict;
		TGroupList *fDraggedGroups;
		
		virtual void MakeGrayPict();
		virtual void WasDraggedToGroupTree();
		virtual void CreateListOfDraggedGroups();
		virtual void CheckForExistingGroups(TGroupListView *hitGroupListView, Boolean makingCopy);
		virtual void GetBeforeGroupName(TGroupListView *groupListView, short beforeRow, CStr255 &name);

		
	private:
		Boolean fMakingCopy;
		Boolean fDoesDragGray;
		Boolean fHasDrawedOnDesktop;
		CPoint fCurrentGrayPos;

		TGroupListView *fHitGroupListView;
		TGroupList *fHitGroupList;
		VRect fOldInsertMarkRect; // in local GridView coords
		CStr255 fInsertBeforeName; // empty if append
		Boolean fHasDrawedInsertMark;

		void StartGrayDrag();
		void DrawGray();

		void DrawInsertMark();
		Boolean CalcNewHit(VPoint globalMouse, TGroupListView *&groupListView, 
																	VRect &newMarkRect, VPoint &localMouse);
		void RemoveOldInsertMark();
		void TestForHit(VPoint globalMouse);

		void DragToGroupList();
		void DidNotHitWindow();
};


//--------------------------------------------------------------------
class TGroupViewKeyCommand : public TCommand
{
	public:
		virtual pascal void DoIt(); // Override
		
		TGroupViewKeyCommand();
		virtual pascal void Initialize();
		void IGroupViewKeyCommand(TGridView *gv);
		virtual pascal void Free();
	protected:
		TGridView *fGridView;
		ArrayIndex fLastLine;
		CStr255 fCurrentSelectedName;
		ArrayIndex fCurrentSelectedLine;
		CStr255 fCandidateName;
		ArrayIndex fCandidateLine;

		virtual void SetUp();
		virtual void DoLine(ArrayIndex line, const CStr255 &text);
		virtual void SetDown();
		virtual void HandleNoFound();
		virtual void DoAllLines();
		virtual void DoOneLine(ArrayIndex line);
		virtual void GetLineText(ArrayIndex line, CStr255 &text);
};


class TGroupViewTypeNameCommand : public TGroupViewKeyCommand
{
	public:
		TGroupViewTypeNameCommand();
		virtual pascal void Initialize();
		void IGroupViewTypeNameCommand(TGridView *gv, TToolboxEvent *event);
		virtual pascal void Free();
	protected:
		CStr255 fTypeChars;
		unsigned long fLastTick, fCharTick;
		char fChar;

		virtual void SetUp();
		virtual void SetDown();
		virtual void DoLine(ArrayIndex line, const CStr255 &text);
		virtual void HandleNoFound();
};

class TGroupViewTabKeyCommand : public TGroupViewKeyCommand
{
	public:
		TGroupViewTabKeyCommand();
		virtual pascal void Initialize();
		void IGroupViewTabKeyCommand(TGridView *gv, Boolean forward);
		virtual pascal void Free();
	protected:
		Boolean fForward;
		CStr255 fFirstLastName;
		ArrayIndex fFirstLastLine;
		
		virtual void DoLine(ArrayIndex line, const CStr255 &text);
		virtual void DoForwardLine(ArrayIndex line, const CStr255 &text);
		virtual void DoBackwardLine(ArrayIndex line, const CStr255 &text);
		virtual void HandleNoFound();
};
