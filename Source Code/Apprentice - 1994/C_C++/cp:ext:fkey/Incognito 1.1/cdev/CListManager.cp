#ifndef __CLISTMANAGER__
#include "CListManager.h"
#endif

#ifndef __TEXTUTILS__
#include <TextUtils.h>
#endif

#ifndef __DIALOGUTIL__
#include <DialogUtil.h>
#endif

#ifndef __FONTS__
#include <Fonts.h>
#endif

#ifndef __STRING__
#include <String.h>
#endif

/*
  MyCompareProc does a straight memcompare, because who really gives a damn if the
  script isn't english?
*/

static pascal Boolean MyCompareProc(const void* a, const void* b, short aLen, short bLen)
{
	if (!aLen || !bLen) return true;
	return !memcmp(a, b, aLen > bLen ? aLen : bLen);
//	return !EqualString((ConstStr255Param) a, (ConstStr255Param) b, false, true);
}

void CListManager::RemoveItem(StringPtr theItem)
{
	Cell	theCell;
	
	if (FindItem(theItem, theCell))
	{
		RemoveItem(theCell);
	}
}

Boolean CListManager::IsItemSelected(StringPtr theItem)
{
	Cell	theCell;
	
	if (FindItem(theItem, theCell))
	{
		return IsItemSelected(theCell);
	}
	return false;
}

/*
  Inserts an item into the list in sorted order. How about that?
*/

Boolean CListManager::InsertItem(StringPtr theItem)
{
	Boolean	good;
	Cell	theCell = {0,0};
	
	if (good = BinarySearch(theItem, theCell))
	{
		AddItem(theItem, theCell);
	}
	return good;
}

/*
  inserts a row into the list, and sets the row to the data passed
  in.
*/

void CListManager::AddItem(StringPtr name, Cell theCell)
{
	short length = name[0];

	LAddRow(1, theCell.v, fTheList);
	LSetCell(name + 1, length, theCell, fTheList);
}

/*
  retrieves an item from the list.
*/

Boolean CListManager::GetItem(StringPtr theData, Cell &theCell)
{
	short length = 31;
	
	if (GetHeight() && GetWidth() && (theCell.v >= 0) && (theCell.v < GetHeight()) && (theCell.h >= 0) && (theCell.h < GetWidth()))
	{
		LGetCell(theData + 1, &length, theCell, fTheList);
		theData[0] = length;
		return true;
	}
	else return false;
}

/*
  Removes an item from the list. Destroys the row, basically.
*/

void CListManager::RemoveItem(Cell theCell)
{
	LDelRow(1, theCell.v, fTheList);
}

/*
  Finds theData if it is in the list. Returns true if the item is
  present, and sets theCell to its cell.
*/

Boolean CListManager::FindItem(StringPtr theData, Cell& theCell)
{
	return !BinarySearch(theData, theCell);
}


/*
  Is there a selection in the list? If so, it returns the first selected
  item. Destroys the contents of theCell.
*/

Boolean CListManager::IsSelection(Cell& theCell)
{
	theCell.h = theCell.v = 0;
	return LGetSelect(true, &theCell, fTheList);
}

/*
  is the specified cell selected? returns true if it is.
*/

Boolean CListManager::IsItemSelected(Cell& theCell)
{
	return LGetSelect(false, &theCell, fTheList);
}

/*
  Finds the next selected item. Returns true if there is one, setting
  theCell to the item.
*/

Boolean CListManager::FindNextSelection(Cell& theCell)
{
	return LGetSelect(true, &theCell, fTheList);
}

/*
	Deselects a list
*/

void CListManager::DeselectList()
{
	Cell	theCell = {0, 0};
	
	while (LGetSelect(true, &theCell, fTheList))
	{
		LSetSelect(false, theCell, fTheList);
	}
}

void CListManager::SelectItem(StringPtr theItem)
{
	Cell	theCell;
	
	if (FindItem(theItem, theCell))
	{
		SelectItem(theCell);
	}
}

Boolean CListManager::GetCurrentSelection(StringPtr theItem)
{
	Cell	theCell;
	
	if (IsSelection(theCell))
	{
		return GetItem(theItem, theCell);
	}
	return false;
}

#pragma segment Main
CListManager::CListManager(short dialogItem, DialogPtr theDialog, short width, short height)
{
	Rect		dataBounds, rView;
	Point		cellSize;
	GrafPtr		oldPort;
	FontInfo	theInfo;
	
	GetPort(&oldPort);
	SetPort(theDialog);
	
	TextFont(geneva);
	TextSize(9);
	GetFontInfo(&theInfo);
	SetRect(&dataBounds, 0, 0, width, height);
	getBox(theDialog, dialogItem, &rView);
	rView.right -= 15;
//	cellSize.h = width.v = 0;
	cellSize.h = rView.right - rView.left;
	cellSize.h /= width;
	cellSize.v = theInfo.ascent + theInfo.descent + theInfo.leading;
	
	fTheList = LNew(&rView, &dataBounds, cellSize, 0, theDialog, true, false, false, true);
	if (fTheList)
	{
		(**fTheList).selFlags = lUseSense + lNoRect + lNoExtend;
		(**fTheList).listFlags = 2;
		LSetDrawingMode(true, fTheList);
	}
	SetPort(oldPort);
}


#pragma segment Main
CListManager::~CListManager()
{
	LDispose(fTheList);
}


#pragma segment Main
void	CListManager::Activate()
{
	LActivate(true, fTheList);
}


#pragma segment Main
void	CListManager::Deactivate()
{
	LActivate(false, fTheList);
}


#pragma segment Main
void	CListManager::Update()
{
	LUpdate((*fTheList)->port->visRgn, fTheList);
}


#pragma segment Main
Boolean	CListManager::Click(Point thePoint, short modifiers)
{
	Point	localPoint = thePoint;
	
	GlobalToLocal(&localPoint);
	return LClick(localPoint, modifiers, fTheList);
}


#pragma segment Main
Boolean CListManager::IsSelection()
{
	Cell	theCell = {0,0};
	
	return LGetSelect(true, &theCell, fTheList);
}

#pragma segment Main
short  CListManager::GetHeight()
{
	return (**fTheList).dataBounds.bottom;
}


#pragma segment Main
short  CListManager::GetWidth()
{
	return (**fTheList).dataBounds.right;
}

#pragma segment Main



#pragma segment Main
Boolean  CListManager::GetNextItem(StringPtr theItem, Cell &theCell)
{
	if ((theCell.h+1) < GetWidth()) theCell.h++;
	else
	{
		theCell.h = 0;
		theCell.v++;
	}
	return GetItem(theItem, theCell);
}



#pragma segment Main
void CListManager::AutoScroll()
{
	LAutoScroll(fTheList);
}


#pragma segment Main
void CListManager::SelectItem(Cell &theCell)
{
	DeselectList();
	LSetSelect(true, theCell, fTheList);
	AutoScroll();
}


#pragma segment Main
void  CListManager::Empty()
{
	LDelRow(0, 0, fTheList);
}

#pragma segment Main
Boolean CListManager::GetFirstItem(StringPtr theItem, Cell &theCell)
{
	theCell.h = 0;
	theCell.v = 0;
	return GetItem(theItem, theCell);
}

#pragma segment Main
void CListManager::DoDraw(Boolean drawStuff)
{
	LSetDrawingMode(drawStuff, fTheList);
}


#pragma segment Main
void CListManager::DeleteSelection(void)
{
	Cell	theCell = {0,0};
	
	while (IsSelection(theCell))
	{
		RemoveItem(theCell);
	}
}


#pragma segment Main


#pragma segment Main
Boolean CListManager::IsItemPresent(StringPtr theItem)
{
	Cell	theCell = {0,0};
	
	return FindItem(theItem, theCell);
}


#pragma segment Main
short CListManager::CompareInfo(Cell theCell, short offset, short length, StringPtr theData)
{
	short	result;
	char	*theCellData;
	
	HLock((*fTheList)->cells);
	theCellData = (*(*fTheList)->cells)+offset;
	result = IUMagString(theData+1, theCellData, theData[0], length);
	HUnlock((*fTheList)->cells);
	return result;
}


#pragma segment Main
Boolean CListManager::BinarySearch(StringPtr theItem, Cell &theCell)
{
	short middle, top = (**fTheList).dataBounds.top, bottom = (**fTheList).dataBounds.bottom;
	short result;
	short offset, length;		// 4 lfind
	
	theCell.h = theCell.v = 0;
	middle = (bottom - top) / 2;
	while (bottom != middle)
	{
		theCell.v = middle;
		LGetCellDataLocation(&offset, &length, theCell, fTheList);
		if (length == -1) return false;
		
		result = CompareInfo(theCell, offset, length, theItem);
		if (result < 0)
		{
			bottom = middle;
			middle -= ((bottom - top) + 1) / 2;
		}
		else if (result > 0)
		{
			top = middle;
			middle += ((bottom - top) + 1) / 2;
		}
		else
		{
			theCell.v = middle;
			return false;							// no duplicates!
		}
	}
	theCell.v = bottom;
	return true;
}

#pragma segment Main
void CListManager::DeselectItem(Cell &theCell)
{
	LSetSelect(false, theCell, fTheList);
}

#pragma segment Main
Boolean CListManager::BinarySearchInsensitive(StringPtr theData, Cell &theCell)
{
	short	middle, top = (**fTheList).dataBounds.top, bottom = (**fTheList).dataBounds.bottom;
	short	result;
	Str32	theItem;
	
	theCell.h = theCell.v = 0;
	middle = (bottom - top) / 2;
	while (bottom != middle)
	{
		theCell.v = middle;
		GetItem(theItem, theCell);
		result = RelString(theData, theItem, false, true);
		if (result < 0)
		{
			bottom = middle;
			middle -= ((bottom - top) + 1) / 2;
		}
		else if (result > 0)
		{
			top = middle;
			middle += ((bottom - top) + 1) / 2;
		}
		else
		{
			theCell.v = middle;
			return false;							// no duplicates!
		}
	}
	theCell.v = bottom;
	return true;
}

#pragma segment Main
void CListManager::SetItem(StringPtr theString, Cell theCell)
{
	short	length = 0;
	
	length = theString[0];
	LSetCell(theString+1, length, theCell, fTheList);
}





#pragma segment Main





