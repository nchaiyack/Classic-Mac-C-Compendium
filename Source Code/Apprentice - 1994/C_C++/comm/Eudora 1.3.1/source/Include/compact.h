/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * declarations for functions relating to composition window actions
 ************************************************************************/
Boolean CompClose(MyWindowPtr win);
Boolean CompScroll(MyWindowPtr win,int h,int v);
void CompDidResize(MyWindowPtr win, Rect *oldContR);
void CompClick(MyWindowPtr win, EventRecord *event);
Boolean CompMenu(MyWindowPtr win, int menu, int item, short modifiers);
void CompUpdate(MyWindowPtr win);
void CompKey(MyWindowPtr win, EventRecord *event);
void CompTxChanged(MyWindowPtr win,int oldNl, int newNl,Boolean scroll);
void CompActivateField(MessType **messH, short newtx);
void CompTxeTo(MyWindowPtr win,TEHandle teh);
void CompUnattach(MyWindowPtr win);
void AttachSelect(TEHandle teh);
void CompButton(MyWindowPtr win,ControlHandle buttonHandle,long modifiers,short part);
void CompHelp(MyWindowPtr win,Point mouse);
Boolean ModifyQueue(short *state,uLong *when);
void WarpQueue(uLong secs);
void CompHasChanged(MyWindowPtr win,TEHandle teh,short oldNl,short newNl);
void CompZoomSize(MyWindowPtr win,Rect *zoom);