#pragma once

#define qZDebug 1

#ifndef __PowerPlant__
// anybody else *really* anxious for namespaces?
// and exceptions?  Templates are useful, but not essential!!
typedef long Int32;
typedef short Int16;
#endif

#undef GetItem

#include <Drag.h>

class ZListItem{
	protected:
		Boolean	itemSelected;
	
	public:
		ZListItem()	{itemSelected = false;}
		
		virtual void Click(EventRecord &,Rect&) {}
		virtual void Activate(Boolean) {}
		virtual void Draw(Rect &r) {InvertRect(&r);}
		
		virtual void Export(FlavorType */*outOurFlavor*/,void **/*outOurData*/,long */*outDataLen*/)
		{
			DebugStr("\pYou didn't provide an export proc!");
		}
	
	friend class ZList;
};

enum SelectionMode{
	selOnlyOne					= 1,
	selExtendDrag				= 2,
	selDisjoint					= 3,
	selExtend					= 4
};

enum ListFeature{
		// autoHilite * if set, ZList will automatically (de)hilite
		//				the selection correctly.  Default: set.
	zfAutoHilite = (unsigned long)(1<<0),
	
		// supportThreads * if set, ZList will call YieldToAnyThread.
		//	Default: set.
	zfSupportThreads = (1<<1),
	
		// supportDrags * if set, ZList supports drag manaager for import/export
		//	Default: set.
	zfSupportDrags = (1<<2),
	
		// dragsWithinList * if set, dragging items within the list is enabled
		// (doesn't need drag mgr) unimplemented
	zfDragsWithinList = (1<<3),
	
		// leaveSpaceForGrowBox * bottom of scrollbar moves up 16 pixels
	zfLeaveSpaceForGrowBox = (1<<4),
	
		// supportSystem75 * a quick way to support threads + drags
	zfSupportSystem75 = (zfSupportThreads|zfSupportDrags),
	endlistfeatures
};

typedef ZListItem *(*DragItemToZItemProc)(FlavorType,Ptr);

typedef struct{
	FlavorType			theType;
	DragItemToZItemProc	theProc;
}ZConvTableEntry;
	
typedef struct{
	ZConvTableEntry	entry[1];
}**ZConvTable;

class ZList{
	protected:
		
			// � display variables
		Rect			mViewRect;
		Int32			mItemSizeV,
						mVisibleLength,
						mMaxVisibleItems,
						mTopVisibleItem,
						mLastClickTicks;
		ControlHandle	mScrollHandle;
		
			// � Drag �n� Drop variables
		ZConvTable		mConversionTable;
		Boolean			mDragAlready;		//have we sent ourselves a dragenter message?
		Point			mLastDragMouse;
		
			// � data variables
		Handle			mItems;
			
			// � selection/mode variables
		SelectionMode	mSelectionMode;
		ListFeature		mListFeatures;
		
	public:
						ZList(Rect &inViewRect, short inVerticalItemSize);
		virtual			~ZList();
		
		virtual void	DrawSelf();
		virtual Boolean	ClickSelf(EventRecord&);
		virtual void	Activate(Boolean active);
		
		Int32			GetNumItems();
		void			AddItem(Int32 inWhere, ZListItem* inNewItem);
		void			AppendItem(ZListItem* inNewItem);
		void			ClearAllItems();
		void			DeleteItem(Int32 inIndex);
		void			GetItem(Int32 inIndex, ZListItem **);
		Boolean			GetItemRect(Int32 inItemIndex,Rect &outItemRect);
		
		void			SetListItemSize(Int32 inSizeV) {mItemSizeV = inSizeV;}
		Int32			GetListItemSize() {return mItemSizeV;}
		
		void			SetViewRect(Rect& inViewRect);
		void			PubGetViewRect(Rect& outViewRect)
						{
							outViewRect = mViewRect;
							outViewRect.right += 15;
						}
		
		Boolean			GetItemUnderPoint(Point inPt, Int32 &outItemNum, ZListItem** theObj);
		
		// � Selection
		Int32			GetNumSelectedItems();
		Boolean			GetSelectedItem(Int32 inSelIndex,ZListItem **);
		long			GetSelectedItemIndex(Int32 inCount);
		
		void			SelectItem(Int32 inIndex);
		void			SetSelectionMode(SelectionMode inWhichMode);
		SelectionMode	GetSelectionMode();
		
		// � Features
		void			ToggleFeature(ListFeature inFeature)
		{
			mListFeatures = (ListFeature)(inFeature ^ mListFeatures);
		}
		
		Boolean			DoesFeature(ListFeature inFeature)
		{
			return (mListFeatures & inFeature);
		}
		
		// � Scrolling
		void			ScrollUp(Int16 inNumToScroll);
		void			ScrollDown(Int16 inNumToScroll);
		
		// ��Dragging
		void			TryDragEnter(DragReference inDragRef);
		void			TryDragExit(DragReference /*inDragRef*/);
		void			TryDragTrack(DragReference inDragRef);
		void			TryDragDrop(DragReference inDragRef);
		Boolean			WeCanReceiveDrag(DragReference inDragRef);
		void			TryDrag(EventRecord &);
		
		// � Drag item conversion (could work for clipboard data too)
		void			AddDataConverter(FlavorType inFlavor,DragItemToZItemProc inProc);
		ZListItem *		DispatchDataConversion(FlavorType inFlavor,Ptr inData);
		
	private:
		void			GetViewRect(Rect& outViewRect)
						{
							outViewRect = mViewRect;
						}
						
		void RedrawVisibleItems(Int32 startIdx);
		void SynchLength();
		void SynchScroll();
		void SynchScrollValues();
		void DrawItem(ZListItem *inItem,Rect &inItemRect,Boolean inHilight);
		void MoveItems(Int32 inSource,Int32 inSourceEnd, Int32 inDest);
		void ToggleInsertionPoint(Point inInsertWhere);		
		static pascal void ScrollActionProcV(ControlHandle theControl,short partCode);
};
