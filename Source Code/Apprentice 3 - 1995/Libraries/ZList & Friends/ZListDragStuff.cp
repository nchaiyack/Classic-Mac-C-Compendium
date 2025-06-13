/*
	ZListDragStuff
	©1994 Chris K. Thomas.  All Rights Reserved.
	
	Drag Manager support.
	
	It seems as though Apple could have eliminated most of this work if
	they'd really wanted to.
	
	Some of this should be factored out into a “ZDropZone” class.
	Particularly the logic in TryDragTrack().
	
	29 Nov 94  <ckt>  Created and moved drag stuff over here.
*/

#include "ZList.h"


// * Static Vars

// ≠ Note:	these first two aren't used currently, as they're quite silly
//			and useless if you've got more than one drag area per
//			window.  You can still call them from your window's drag
//			tracking handler with the appropriate messages, and that's
//			actually probably a good idea for future compatibility.

// * Call upon a drag entering the window
void
ZList::TryDragEnter(DragReference/* inDragRef*/)
{
	/*Point mouseLoc;
	
	GetMouse(&mouseLoc);
	if(PtInRect(mouseLoc,&mViewRect))
	{
		mDragging = true;
		mLastDragMouse.h = -1;
		mLastDragMouse.v = -1;
	}*/
}


void
ZList::TryDragExit(DragReference /*inDragRef*/)
{
}

void
ZList::TryDragTrack(DragReference inDragRef)
{
	if(!WeCanReceiveDrag(inDragRef)) return;

	Point		mouseLoc, pinnedMouseLoc;
	RgnHandle	hiliteRgn = NewRgn();
	
	(void) GetDragMouse(inDragRef,&mouseLoc,&pinnedMouseLoc);
	GlobalToLocal(&mouseLoc);
		
	SetRectRgn( hiliteRgn,
				mViewRect.left,
				mViewRect.top,
				mViewRect.right,
				mViewRect.top+mVisibleLength);
	
	InsetRgn(hiliteRgn,1,1);

	if (PtInRgn(mouseLoc,hiliteRgn))
	{
		if(mDragAlready == true && mLastDragMouse.v != mouseLoc.v)
		{
			ToggleInsertionPoint(mLastDragMouse);
			ToggleInsertionPoint(mouseLoc);
		}
		else if(mDragAlready == false)
		{
			ToggleInsertionPoint(mouseLoc);
		}
		
		mLastDragMouse = mouseLoc;
		
		if(mDragAlready == false)
			::ShowDragHilite(inDragRef,hiliteRgn,true);
			
		mDragAlready = true;
	}	
	else
	{
		if(mDragAlready == true)
		{
			ToggleInsertionPoint(mLastDragMouse);
			::HideDragHilite(inDragRef);
			mDragAlready = false;
		}
	}
	
	DisposeRgn(hiliteRgn);
}

void ZList::ToggleInsertionPoint(Point inInsertWhere)
{
	Int32 itemIdx;
	
	if(GetItemUnderPoint(inInsertWhere,itemIdx,NULL))
	{
		Rect ourRect;
		GetItemRect(itemIdx,ourRect);
		
		::PenMode(srcXor);
		::MoveTo(ourRect.left,ourRect.top);
		::LineTo(ourRect.right,ourRect.top);
		::LineTo(ourRect.right,ourRect.top);
		::PenMode(srcOr);
	}
}

void
ZList::TryDragDrop(DragReference inDragRef)
{
	OSErr			theErr;
	unsigned short	theShort,theFlavors;
	FlavorType		shallWeType;
	Point			ourGetPoint;
	
	if(mDragAlready == true)
	{
		ToggleInsertionPoint(mLastDragMouse);
		::HideDragHilite(inDragRef);
		mDragAlready = false;
	}
		
	::GetMouse(&ourGetPoint);
	
	if(!PtInRect(ourGetPoint,&mViewRect)) return;
	
	theErr = ::CountDragItems(inDragRef,&theShort);
	
	if(theErr != noErr) return;
	
	ItemReference	ourItemRef;
	
	for(long i=1; i<=theShort; i++)
	{
		theErr = ::GetDragItemReferenceNumber(inDragRef, i, &ourItemRef);
		if(theErr != noErr) continue;
		
		theErr = ::CountDragItemFlavors(inDragRef, ourItemRef, &theFlavors);
		if(theErr != noErr) continue;
		
		for(long j=1; j<=theFlavors; j++)
		{
			Size	flavorSize;
			Ptr		flavorData;
			
			theErr = ::GetFlavorType(inDragRef, ourItemRef, j, &shallWeType);
			theErr = ::GetFlavorDataSize(inDragRef,ourItemRef,shallWeType,&flavorSize);
			if(theErr == noErr)
			{
				flavorData = ::NewPtr(flavorSize);
				if(flavorData)
				{
					theErr = ::GetFlavorData(inDragRef,ourItemRef,shallWeType, flavorData, &flavorSize,0);
					if(theErr == noErr)
					{
						ZListItem *ourDragItem = DispatchDataConversion(shallWeType,flavorData);
						
						if(ourDragItem){
							Int32 whichItem;
							if(GetItemUnderPoint(ourGetPoint,whichItem,NULL))
							{
								AddItem(whichItem,ourDragItem);
							}
							else
							{
								AddItem(GetNumItems()+1,ourDragItem);
							}
						}
					}
					
					DisposePtr(flavorData);
				}
			}
		}//for
	}
}

inline void NumNotify(long num)
{
	Str255 str;
	
	NumToString(num,str);
	DebugStr(str);
}

Boolean
ZList::WeCanReceiveDrag(DragReference inDragRef)
{
	OSErr			theErr;
	unsigned short	numItems,theFlavors;
	FlavorType		shallWeType;
	
	HLockHi((Handle)mConversionTable);
	
	theErr = CountDragItems(inDragRef,&numItems);
	
	if(theErr != noErr) return false;
	
	ItemReference	ourItemRef;
		
	for(long i=1; i<=numItems; i++)
	{
		theErr = GetDragItemReferenceNumber(inDragRef, i, &ourItemRef);
		if(theErr != noErr) continue;
		
		theErr = CountDragItemFlavors(inDragRef, ourItemRef, &theFlavors);
		if(theErr != noErr) continue;
		
		Int32 convTableLength = GetHandleSize((Handle)mConversionTable)/sizeof(ZConvTableEntry);
		
		for(long j=1; j<=theFlavors; j++)
		{
			theErr = GetFlavorType(inDragRef, ourItemRef, j, &shallWeType);
			
			for(Int32 k = 0; k < convTableLength; k++)
			{
				if((**mConversionTable).entry[k].theType == shallWeType)
				{
					return true;
				}
			}
		}
	}
	HUnlock((Handle)mConversionTable);
	return false;
}

void ZList::AddDataConverter(FlavorType inFlavor,DragItemToZItemProc inProc)
{
	if(mConversionTable == NULL) mConversionTable = (ZConvTable) NewHandle(0L);
	if(mConversionTable == NULL) {SysBeep(32);return;} // !!!
	
	SetHandleSize((Handle)mConversionTable,GetHandleSize((Handle)mConversionTable)+sizeof(ZConvTableEntry));
	if(MemError() != noErr) return;
	HLockHi((Handle)mConversionTable);
	
	Int32 convTableLength = GetHandleSize((Handle)mConversionTable)/sizeof(ZConvTableEntry);
	
	(**mConversionTable).entry[convTableLength-1].theType = inFlavor;
	(**mConversionTable).entry[convTableLength-1].theProc = inProc;
	
	HUnlock((Handle)mConversionTable);
}

ZListItem *ZList::DispatchDataConversion(FlavorType inFlavor,Ptr inData)
{
	Int32 convTableLength = GetHandleSize((Handle)mConversionTable)/sizeof(ZConvTableEntry);

	for(Int32 i = 0; i <convTableLength; i++)
	{
		if((**mConversionTable).entry[i].theType == inFlavor)
		{
			return (**mConversionTable).entry[i].theProc(inFlavor,inData);
		}
	}
	return NULL;
}

inline Boolean
KeyIsDown(short keyCode)
{
	union {
		KeyMap			asMap;
		unsigned char	asBytes[16];
	} myMap;
	
	GetKeys(myMap.asMap);
	return ((myMap.asBytes[keyCode >> 3] >> (keyCode & 0x07)) & 1) != 0;
}

void
ZList::TryDrag(EventRecord &theEvent)
{
	Point	initMouse;
	OSErr	theErr;
	
	::GetMouse(&initMouse);

	if(GetNumSelectedItems() > 0)
	{
		ZListItem	*theItem;
		
		DragReference	theDrag;
		theErr = ::NewDrag(&theDrag);
		
		if(theErr == noErr)
		{
			RgnHandle trackRgn = NewRgn();
			
			for(Int32 i = 1; i <= GetNumSelectedItems(); i++)
			{
				GetSelectedItem(i, &theItem);
				
				FlavorType	expType;
				void		*exDataPtr;
				long		exDataLen;
				
				theItem->Export(&expType,&exDataPtr,&exDataLen);
				theErr = ::AddDragItemFlavor(theDrag, i, expType, exDataPtr, exDataLen, 0);
				
				if(theErr == noErr)
				{
					RgnHandle	ourRgn = ::NewRgn();
					RgnHandle	ourRgn2 = ::NewRgn();
					Rect		ourRect;
					
					if(GetItemRect(i, ourRect))
					{
						::SetRectRgn(ourRgn,ourRect.left,ourRect.top,ourRect.right,ourRect.bottom);
						::CopyRgn(ourRgn,ourRgn2);
						::InsetRgn(ourRgn,1,1);
						::XorRgn(ourRgn,ourRgn2,ourRgn);
						
						::UnionRgn(trackRgn, ourRgn, trackRgn);
						::DisposeRgn(ourRgn);
						::DisposeRgn(ourRgn2);
					}
				}
				
				theErr = ::TrackDrag(theDrag, &theEvent, trackRgn);
				::DisposeRgn(trackRgn);
			}
		}
		::DisposeDrag(theDrag);
	}
}