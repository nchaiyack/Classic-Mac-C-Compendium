/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * function prototypes
 **********************************************************************/
void BoxUpdate(MyWindowPtr win);
void BoxClick(MyWindowPtr win,EventRecord *event);
void BoxActivate(MyWindowPtr win);
Boolean BoxMenu(MyWindowPtr win,int menu,int item,short modifiers);
Boolean BoxClose(MyWindowPtr win);
void BoxOpen(MyWindowPtr win);
void BoxKey(MyWindowPtr win, EventRecord *event);
void SelectBoxRange(TOCType **tocH,int start,int end,Boolean cmd,int eStart,int eEnd);
void BoxCenterSelection(MyWindowPtr win);
void BoxSelectAfter(MyWindowPtr win, short mNum);
Boolean BoxPosition(Boolean save,MyWindowPtr win);
void MakeMessFileName(TOCHandle tocH,short sumNum, UPtr name);
void BoxHelp(MyWindowPtr win, Point mouse);
void BoxCursor(Point mouse);
void BoxDidResize(MyWindowPtr win, Rect *oldContR);
void InvalBoxSizeBox(MyWindowPtr win);
void BoxGonnaShow(MyWindowPtr win);
UPtr PriorityString(UPtr string,Byte priority);
void SetPriority(TOCHandle tocH,short sumNum,short priority);
#define Prior2Display(p) ((p)?RoundDiv(p,40):3)
#define Display2Prior(p) ((p)*40)
void InvalTocBox(TOCHandle tocH,short sumNum,short box);
void RedoTOC(TOCHandle tocH);