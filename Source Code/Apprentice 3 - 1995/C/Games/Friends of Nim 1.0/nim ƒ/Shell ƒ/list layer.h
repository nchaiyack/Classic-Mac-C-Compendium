#ifndef __MY_LIST_LAYER_H__
#define __MY_LIST_LAYER_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	ListHandle MyCreateVerticalScrollingList(WindowRef theWindow, Rect boundsRect,
	short columnsInList, short theLDEF, short cellHeight);
extern	void MyDrawListBorder(ListHandle theList);
extern	void MyDrawActiveListBorder(ListHandle theList, Boolean isActive);
extern	void MyAddStr255ToList(ListHandle theList, Str255 theStr);
extern	void MyDeleteItemFromList(ListHandle theList, short index);
extern	Boolean MyHandleMouseDownInList(ListHandle theList, Point thePoint, short modifiers);
extern	void MyUpdateList(ListHandle theList);
extern	void MyActivateList(ListHandle theList, Boolean isActive);
extern	Boolean MyGetFirstSelectedCell(ListHandle theList, Cell *theCell);
extern	void MySelectOneCell(ListHandle theList, Cell theCell);
extern	void MyMakeCellVisible(ListHandle theList, Cell theCell);
extern	void MyDeselectAllCells(ListHandle theList);
extern	void MyClearAllCells(ListHandle theList);
extern	void MyGetCellData(ListHandle theList, Cell theCell, Str255 theName);
extern	void MySetCellData(ListHandle theList, Cell theCell, Str255 theName);
extern	void MyScrollList(ListHandle theList, short diff);
extern	void MySetListSelectionFlags(ListHandle theList, short flags);
extern	void MySetListSize(ListHandle theList, short width, short height, short cellHeight);
extern	void MyGetListRect(ListHandle theList, Rect *boundsRect);
extern	void MySetListRect(ListHandle theList, Rect boundsRect);

#ifdef __cplusplus
}
#endif

#endif
