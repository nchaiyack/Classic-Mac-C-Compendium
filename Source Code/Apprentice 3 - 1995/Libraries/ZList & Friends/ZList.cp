/*
	ZList
	©1994 Chris K. Thomas.  All Rights Reserved.
	
	Version 1.0d3
	
	So MWERKS isn't doing useful classes at this point.
	That's quite all right, we don't need them.
	
	Written by Chris Thomas on a cold Saturday afternoon.
	<thunderone@delphi.com>
	
	Yes, this originally would have required PowerPlant.  It retains some
	PP concepts.  I decided to generalize it, so it's not directly
	usable as an LPane, but it shouldn't be too much trouble to 
	wrap a pane around it.
	
	Permission is granted to use this only if I am sent one (1) free, fully
	registered copy of any product using it.
		
	This source code is provided AS IS, and the author is therefore not
	responsible for any damage incurred through the  use of this source.
	
	11 Nov 94  ckt  Created
*/

#include "ZList.h"
#include "UniversalProc.h"
#include <Threads.h>

#ifdef GetItem	// Universal Headers define this with OBSOLETE_NAMES on!!
#undef GetItem
#endif

inline void NumNotify(long num)
{
	Str255 str;
	
	NumToString(num, str);
	DebugStr(str);
}

/*
	??? Finish complex selection support!
		??? in particular, add support for shift-disjointed selections without
		??? adding complexity to the current simple interface.
	??? Add key processing
	??? Add rect parameter to all event dispatch -update: partially done
	??? Add horizontal scrolling
	??? Make scrollbars optional
	??? outline selection(s) when deactivated
	??? factor drag mgr stuff into seperate class
*/

//Ê¥ put ZList in current port
ZList::ZList(Rect &inViewRect, short inVerticalItemSize)
{
	mItemSizeV			= inVerticalItemSize;
	mItems				= NewHandle(0);
	mTopVisibleItem		= 1;
	mLastClickTicks		= TickCount();
	mListFeatures		= zfAutoHilite;
	mConversionTable	= NULL;
	
	ToggleFeature(zfSupportSystem75);
	
	// create scrollbar
	GrafPtr	theGraf;	
	::GetPort(&theGraf);
	
	Rect scrollRect = inViewRect;
	scrollRect.left = inViewRect.right - 16;
	
	mScrollHandle = ::NewControl(theGraf , &scrollRect, "\pMarlon Brando, Pocahontas, and Me", 
								true, 0, 0, 0, scrollBarProc, (long)this);
	
	::SetControlMinimum(mScrollHandle, 1);
#if qZDebug
	if(mScrollHandle == NULL) DebugStr("\pZList::ZList :\r   Got NULL ScrollHandle");
#endif

//	mNumItems = 0;
	SetSelectionMode(selOnlyOne);
	
	mViewRect = inViewRect;
	SetViewRect(inViewRect);
	::SetControlValue(mScrollHandle, 1);
	
	mDragAlready = false;
}

ZList::~ZList()
{
	ClearAllItems();
	
	::DisposeHandle((Handle)mConversionTable);
	::DisposeHandle(mItems);
	::DisposeControl(mScrollHandle);
}

Int32 ZList::GetNumItems()
{
	return (::GetHandleSize((Handle)mItems)/sizeof(ZListItem *));
}

inline void SetHiliteMode()
{
	LMSetHiliteMode(LMGetHiliteMode() & !(1 << hiliteBit));
}

inline void HiliteRect(Rect &inRectToXor)
{
	LMSetHiliteMode(LMGetHiliteMode() & !(1 << hiliteBit));
	::InvertRect(&inRectToXor);
}

// ¥ Self_Drawing_Obfuscation
void ZList::DrawSelf()
{
	Rect		ourCurRect;
	ZListItem	*ourItem;
	
	Draw1Control(mScrollHandle);
	GetViewRect(ourCurRect);
	ourCurRect.bottom = ourCurRect.top + mVisibleLength;
	
	FrameRect(&ourCurRect);
	InsetRect(&ourCurRect, 1, 1);
	EraseRect(&ourCurRect);
	
	for(Int32 i = mTopVisibleItem; i <= (mTopVisibleItem + mMaxVisibleItems - 1); i++)
	{
		if(i > GetNumItems()) break;
		
		GetItemRect(i, ourCurRect);
		GetItem(i, &ourItem);
				
		DrawItem(ourItem, ourCurRect, ourItem->itemSelected);
	}
	
}

// ­ Draw visible items starting from given number

void ZList::RedrawVisibleItems(Int32 startIdx)
{
	Rect		ourCurRect;
//	ZListItem	*ourItem;
	
	for(Int32 i = startIdx; i <= (mTopVisibleItem + mMaxVisibleItems - 1); i++)
	{
		if(i > GetNumItems()) break;
		
		GetItemRect(i, ourCurRect);
		InvalRect(&ourCurRect);
		//EraseRect(&ourCurRect);
		//GetItem(i, &ourItem);
		
		//DrawItem(ourItem, ourCurRect, ourItem->itemSelected);
	}
}

// ¥ Click event handler
Boolean ZList::ClickSelf(EventRecord &event)
{
	Point 		where = event.where, maus;
	ZListItem	*theItem;
	Int32		itemNum, lastItemNum = 0;
	long		scrollTicks = TickCount();
	const short	kSmoothScrollTicks = 30;
	
	Boolean		out = true;
	
	// we can't ever use mViewRect directly because it may not correspond
	// to the current displayed list, since we size the list to fit an exact
	// number of items in SynchLength.  We do it this way to enable runtime
	// changing of the default item height.
	
	Rect	trueView = mViewRect;
	trueView.bottom = trueView.top + mVisibleLength;
	
	InsetRect(&trueView, 1, 1);
	
	if(PtInRect(where, &trueView))
	{
		while(StillDown())
		{
			if(DoesFeature(zfSupportThreads))
				YieldToAnyThread();
				
			SystemTask();
			GetMouse(&maus);
			
				// if mouse is above or below viewrect, scroll apprptly.			
			if(maus.v <= trueView.top)
			{
				if(TickCount() - scrollTicks >= kSmoothScrollTicks / ((trueView.top - maus.v)+1)) //we do this to ensure smoothness
				{													//and to slow down on fast Macs
					ScrollUp(1);
					SelectItem(mTopVisibleItem);
					scrollTicks = TickCount();
				}
			}
			else if (maus.v > (trueView.bottom))
			{
				if(TickCount() - scrollTicks >= kSmoothScrollTicks / ((maus.v - trueView.bottom)+1))
				{
					ScrollDown(1);
					
					if(mTopVisibleItem + mMaxVisibleItems - 1 <= GetNumItems())
					{
						SelectItem(mTopVisibleItem+(mMaxVisibleItems-1));
						scrollTicks = TickCount();
					}
				}
			}
			else if(maus.h >= trueView.right || maus.h <= trueView.left)
			{
				if(DoesFeature(zfSupportDrags))
					TryDrag(event);
			}
			else
			{
				if(GetItemUnderPoint(maus, itemNum, &theItem))
				{
					if(lastItemNum!=itemNum)
					{
						SelectItem(itemNum);
						lastItemNum = itemNum;
					}
				}
			}
		}
		
		//on double click, dispatch click message
		if(TickCount() - mLastClickTicks <= GetDblTime())
		{
			Rect ourItemRect;
			GetMouse(&maus);
			if(GetItemUnderPoint(maus, itemNum, &theItem))
			{
				GetItemRect(itemNum, ourItemRect);
				
				mLastClickTicks = TickCount();
				theItem->Click(event, ourItemRect);
			}
		}
		else if(TickCount() - mLastClickTicks > GetDblTime())
		{		// start counting again
			mLastClickTicks = TickCount();
		}
	}
	else
	{
		trueView.right+=15;
		if(PtInRect(where, &trueView))
		{
			ControlHandle		theHandle;
			GrafPtr				thePort;
			UniversalProc		scrollProc((ProcPtr)ScrollActionProcV, uppControlActionProcInfo);
			
			GetPort(&thePort);
			short	hitControl=FindControl(event.where, thePort, &theHandle);
			
			switch (hitControl)	{
				case inUpButton:
				case inDownButton:
				case inPageUp:
				case inPageDown:
					TrackControl(mScrollHandle, where, scrollProc);
					break;
				case inThumb:
					//Dynamic Thumb!
					
					Point	mrSoul;
					short	oldValue, curThumbLoc;
					short	scrollArea	= mVisibleLength - 32;
					short	thumbSize	= 16;
					short	valueMax	= GetControlMaximum(mScrollHandle);
					short	thumbInc	= scrollArea/valueMax; //increment in which thumb travels
					
					while(StillDown())
					{
						GetMouse(&mrSoul);
						oldValue = GetControlValue(mScrollHandle);
						
						curThumbLoc = oldValue*thumbInc;
						curThumbLoc += (mViewRect.top + 16); //16 is size of the scroll arrow
						
						if(mrSoul.v > curThumbLoc)
							ScrollDown(1);
						else if(mrSoul.v < (curThumbLoc - thumbInc))
							ScrollUp(1);
						
						if(DoesFeature(zfSupportThreads))
							YieldToAnyThread();				
					}
					break;
			}
		}
		else
		{
		
			//point is not in rect
			out = false;
		}
	}
	
	return out;
}

// ­ scroll procedures
pascal void ZList::ScrollActionProcV(ControlHandle theControl, short partCode)
{
	ZList * ourList = (ZList *)GetControlReference(theControl);
	
	switch (partCode)
	{
		case inUpButton:
			ourList->ScrollUp(1);
			break;
		case inDownButton:
			ourList->ScrollDown(1);
			break;
		case inPageUp:
			ourList->ScrollUp(ourList->mMaxVisibleItems);
			break;
		case inPageDown:
			ourList->ScrollDown(ourList->mMaxVisibleItems);
			break;
		case inThumb:
			break;
	}
	
	if(ourList->DoesFeature(zfSupportThreads))
		YieldToAnyThread();				
}

void ZList::ScrollUp(Int16 inNumToScroll)
{
	if(GetControlValue(mScrollHandle) == GetControlMinimum(mScrollHandle))
		return;
		
	while(mTopVisibleItem - inNumToScroll < 1)
	{
		inNumToScroll -= 1;
	}
	
//	if(mTopVisibleItem - inNumToScroll - GetControlMaximum(mScrollHandle) >= 1)
	{
	
		RgnHandle	ourRgn = NewRgn();
		Rect		ourRect	= mViewRect;
		
		ourRect.bottom = ourRect.top + mVisibleLength;
				
		mTopVisibleItem -= inNumToScroll;
	
#if qZDebug
		if(mTopVisibleItem <= 0)
		{
			DebugStr("\pZList::ScrollUp :\r  mTopVisibleItem too low");
			NumNotify(mTopVisibleItem);
		}
#endif
	
		InsetRect(&ourRect, 1, 1);
		ScrollRect(&ourRect, 0, mItemSizeV*inNumToScroll, ourRgn);
		DisposeRgn(ourRgn);
		
			// draw the newly revealed items
		for(Int32 i = mTopVisibleItem; i < inNumToScroll+mTopVisibleItem; i++)
		{
			ZListItem *myItem;
			GetItem(i, &myItem);
			GetItemRect(i, ourRect);
			
			DrawItem(myItem, ourRect, myItem->itemSelected);
		}
	
		SynchScrollValues();
	}
}

void ZList::ScrollDown(Int16 inNumToScroll)
{
	if(GetControlValue(mScrollHandle) == GetControlMaximum(mScrollHandle))
		return;
		
	while(inNumToScroll + mTopVisibleItem > GetControlMaximum(mScrollHandle))
	{
		inNumToScroll -= 1;
	}
	
	//if((mTopVisibleItem + (mMaxVisibleItems-1)) + inNumToScroll <= GetNumItems())
	{
		RgnHandle	ourRgn = NewRgn();
		Rect		ourRect	= mViewRect;
		
		ourRect.bottom = ourRect.top + mVisibleLength;
		
		mTopVisibleItem += inNumToScroll;
		InsetRect(&ourRect, 1, 1);
		ScrollRect(&ourRect, 0, -(mItemSizeV*inNumToScroll), ourRgn);
		DisposeRgn(ourRgn);
		
		for(Int32 i = mTopVisibleItem+(mMaxVisibleItems-inNumToScroll); i <= mTopVisibleItem + mMaxVisibleItems - 1; i++)
		{
			ZListItem *myItem;
			GetItem(i, &myItem);
			GetItemRect(i, ourRect);
			
			DrawItem(myItem, ourRect, myItem->itemSelected);
		}
	}
	
	SynchScrollValues();
}

// ¥ Activate event handler
void
ZList::Activate(Boolean active)
{
	ZListItem	*ourItem;
	
	HiliteControl(mScrollHandle, active ? 0 : 255);
	
	for(long i = mTopVisibleItem; i <= (mTopVisibleItem + mMaxVisibleItems - 1); i++)
	{
		if(i > GetNumItems()) break;
		GetItem(i, &ourItem);
		ourItem->Activate(active);
	}	
	
	DrawSelf();
}

// ¥ Get the list object currently in given point
	// returns true if there's anything under the point
Boolean
ZList::GetItemUnderPoint(Point inPt, Int32 &outItemNum, ZListItem** outItem)
{
	Rect	rectToGet;
	Boolean	gotSomething = false;
	
	*outItem = NULL;
	
	for(Int32 itemToGet = mTopVisibleItem; itemToGet < (mTopVisibleItem+mMaxVisibleItems); itemToGet++)
	{
		if(itemToGet > GetNumItems()) return false;
		
		GetItemRect(itemToGet, rectToGet);
		
		if(PtInRect(inPt, &rectToGet))
		{
			gotSomething = true;
			break;
		}
	}
	
	if(gotSomething == true)
	{
		if(itemToGet > GetNumItems())
			itemToGet = GetNumItems();
		
		if(outItem)
			GetItem(itemToGet, outItem);
		outItemNum = itemToGet;
	}
	
	return gotSomething;
}


// ­ set the visible length of the list to the max it can be and still fit
// within our pane size
void
ZList::SynchLength()
{
	Rect	ourRect;
	Int32	ourV;
	Int32	ourGetV = 1;//GetNumItems();
	
	GetViewRect(ourRect);
	ourV = ourRect.bottom - ourRect.top;
	
	while((ourGetV * mItemSizeV) < ourV)
	{
		ourGetV++;
	}
	
	ourGetV -= 1;
	
	mVisibleLength		= (ourGetV * mItemSizeV) + 2;
	mMaxVisibleItems	= ourGetV;
	
	SetControlMaximum(mScrollHandle, GetNumItems() - mMaxVisibleItems+1);
	
	SynchScroll();
}

void ZList::SynchScroll()
{
	(**mScrollHandle).contrlRect = mViewRect;
	(**mScrollHandle).contrlRect.left = mViewRect.right - 1;
	
	SizeControl(mScrollHandle, 16, mVisibleLength);
	
	if(GetNumItems() <= mMaxVisibleItems)
	{
		HiliteControl(mScrollHandle, 255);
	}
	else
	{
		HiliteControl(mScrollHandle, 0);
		SynchScrollValues();
	}
}

void ZList::SynchScrollValues()
{
		//set value to the item in the center of the visible list
/*	if(mTopVisibleItem == 1) SetControlValue(mScrollHandle, 1);
	else if(mTopVisibleItem + (mMaxVisibleItems-1) == GetNumItems()) SetControlValue(mScrollHandle, GetNumItems());
	else*/ SetControlValue(mScrollHandle, mTopVisibleItem/* + (mMaxVisibleItems/2)*/);
}

// ¥ Selection
void ZList::SelectItem(Int32 inIndex)
{
	ZListItem *ourItem, *ourTempItem;
	
	switch(mSelectionMode)
	{
		case selOnlyOne:
			Rect	ourRect;
			
			GetItem(inIndex, &ourItem);
			
			if(ourItem->itemSelected == false)
			{
				GetItemRect(inIndex, ourRect);
				
//				if(DoesFeature(zfAutoHilite))
//					HiliteRect(ourRect);
				
				ourItem->itemSelected = true;
				
				DrawItem(ourItem, ourRect, true);
				
//				ourItem->Draw(ourRect);
			}
			for(long i = 1; i <= GetNumItems(); i++)
			{
				GetItem(i, &ourTempItem);
				if(ourTempItem->itemSelected && i != inIndex)
				{
					ourTempItem->itemSelected = false;
					
						// if item i is visible
					if(i >= mTopVisibleItem && i <= (mTopVisibleItem+mMaxVisibleItems-1))
					{
						GetItemRect(i, ourRect);
						
						DrawItem(ourTempItem, ourRect, true);
					/*	if(DoesFeature(zfAutoHilite))
							HiliteRect(ourRect);
						
						ourTempItem->Draw(ourRect);*/
					}
				}
			}
			break;
		default:
			break;
	}
}

void ZList::DrawItem(ZListItem *inItem, Rect &inItemRect, Boolean inHilight)
{
	RgnHandle saveRgn = NewRgn();
	
	if(inHilight){
		if(DoesFeature(zfAutoHilite))
			HiliteRect(inItemRect);
	}
	
	GetClip(saveRgn);
	ClipRect(&inItemRect);

	inItem->Draw(inItemRect);
	SetClip(saveRgn);
//	??? memory leak
}

void ZList::ClearAllItems()
{
	ZListItem *ourItem;
	
	for(Int32 i = 1; i <= GetNumItems(); i++)
	{
		GetItem(i, &ourItem);
		delete ourItem;
	}

	//delete all items
	SetHandleSize(mItems, 0);
	
	Rect newRect = mViewRect;
	
	newRect.bottom = newRect.top + mVisibleLength;
	
	InsetRect(&newRect, 2, 2);
	EraseRect(&newRect);
	InvalRect(&newRect);
}

Int32 ZList::GetNumSelectedItems()
// return the number of selected items
{
	long		outNumSelected = 0;
	ZListItem	*ourTempItem;
	
	for(long i = 1; i <= GetNumItems(); i++)
	{
		GetItem(i, &ourTempItem);
		if(ourTempItem->itemSelected)
		{
			outNumSelected++;
		}
	}
	return outNumSelected;
}

Boolean ZList::GetSelectedItem(Int32 inCount, ZListItem **z)
{	
	*z = NULL;
	
	Boolean out = false;
	
	if(GetNumItems()>0)
	{
		Int32 selIndex = 0;
		
		for(Int32 i = 1; i <= GetNumItems(); i++)
		{
			GetItem(i, z);
			if((*z)->itemSelected)
			{
				selIndex++;
				if(selIndex == inCount)
				{
					out = true;
					break;
				}
			}
		}
	}
	
	return out;
}

long ZList::GetSelectedItemIndex(Int32 inCount)
{	
	ZListItem *z = NULL;
	
	long out = 0;
	
	if(GetNumItems()>0)
	{
		Int32 selIndex = 0;
		
		for(Int32 i = 1; i <= GetNumItems(); i++)
		{
			GetItem(i, &z);
			if(z->itemSelected)
			{
				selIndex++;
				if(selIndex == inCount)
				{
					out = i;
				}
			}
		}
	}
	
	return out;
}

void ZList::SetSelectionMode(SelectionMode inWhichMode)
{
	mSelectionMode = inWhichMode;
}

SelectionMode ZList::GetSelectionMode()
{
	return mSelectionMode;
}

void ZList::SetViewRect(Rect& inViewRect)
{
	EraseRect(&mViewRect);
	mViewRect = inViewRect;
	mViewRect.right -= 15;
	SynchLength();
	//Draw1Control(mScrollHandle);
	DrawSelf();
}

// ¥ List Item Obfuscation
Boolean ZList::GetItemRect(Int32 inItemIndex, Rect &outItemRect)
{
	Boolean		outFoundRect = false;
	Rect		ourItemRect = mViewRect;
	
	InsetRect(&ourItemRect, 1, 1);

//#if qZDebug	
	if(inItemIndex >= mTopVisibleItem && inItemIndex <= mTopVisibleItem + mMaxVisibleItems - 1)
	{
//#endif
		
		ourItemRect.bottom = ourItemRect.top + mItemSizeV;
		OffsetRect(&ourItemRect, 0, mItemSizeV * ((inItemIndex) - mTopVisibleItem));
		outItemRect = ourItemRect;
		outFoundRect = true;
//#if qZDebug
	}
//#endif

	return outFoundRect;
}


void ZList::GetItem(Int32 inObjIndex, ZListItem** outObj)
{
#if qZDebug
	if(inObjIndex < 1 || inObjIndex > GetNumItems())
	{
		DebugStr("\pZList::GetItem :\r   range error 1");
		NumNotify(inObjIndex);
	}
#endif
	
	inObjIndex -= 1;
	BlockMove(&((*mItems)[inObjIndex * sizeof(ZListItem*)]), outObj, sizeof(ZListItem*));
}

void ZList::AddItem(Int32 inWhere, ZListItem* inNewObj)
{
	SetHandleSize((Handle)mItems, GetHandleSize(mItems)+sizeof(ZListItem*));
	if(MemError()!=noErr) return;
	
	HLockHi(mItems);
	
	if(inWhere >= GetNumItems()){
		inWhere = GetNumItems();	//get num items does gethandlesize!
	}else{
		MoveItems(inWhere-1, GetNumItems() - 1, inWhere);
	}
	
	inWhere -= 1;
	BlockMoveData(&inNewObj, &(*mItems)[inWhere * sizeof(ZListItem*)], sizeof(ZListItem*));

	HUnlock(mItems);
	
	if(GetNumItems() > mMaxVisibleItems && (**mScrollHandle).contrlHilite == 255)
		HiliteControl(mScrollHandle, 0);

	SetControlMaximum(mScrollHandle, GetNumItems() - mMaxVisibleItems+1);
	
	SynchScrollValues();
	
	//Rect rectWhere;
	
	//GetItemRect(inWhere, rectWhere);
	
	//if(inWhere >= mTopVisibleItem && inWhere <= mTopVisibleItem + mMaxVisibleItems)
	//NumNotify(inWhere);
	RedrawVisibleItems(inWhere);
	//DrawItem(inNewObj, rectWhere, inNewObj->itemSelected);
}

// ­ Move raw item data from place to place (0-based counts)
void ZList::MoveItems(Int32 inSource, Int32 inSourceEnd, Int32 inDest)
{
	BlockMoveData(&(*mItems)[sizeof(ZListItem *) * inSource], 
					&(*mItems)[sizeof(ZListItem *) * inDest], 
					sizeof(ZListItem *) * (inSourceEnd - inSource));
}

// ¥ delete list item

inline void ZList::DeleteItem(Int32 inIndex)
{
	ZListItem *temp;

	GetItem(inIndex, &temp);
	delete temp;
	
	BlockMoveData(&(*mItems)[sizeof(ZListItem *) * inIndex], 
				&(*mItems)[sizeof(ZListItem *) * (inIndex - 1)], 
				(GetNumItems() - inIndex) * sizeof(ZListItem *));
				
	SetHandleSize(mItems, GetHandleSize(mItems) - sizeof(ZListItem *));
}