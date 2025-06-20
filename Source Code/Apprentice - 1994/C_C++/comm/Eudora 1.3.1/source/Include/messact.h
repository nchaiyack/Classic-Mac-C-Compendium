/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * prototypes
 **********************************************************************/
Boolean MessClose(MyWindowPtr win);
Boolean MessMenu(MyWindowPtr win,int menu,int item,short modifiers);
void SaveMessageAs(MessHandle messH);
void MessKey(MyWindowPtr win, EventRecord *event);
short SaveAsToOpenFile(short refN,MessHandle messH);
pascal Boolean SaveAsFilter(DialogPtr dgPtr,EventRecord *event,short *item);
void NextMess(TOCHandle tocH,MessHandle messH,short whichWay,long modifiers);
void MessClick(MyWindowPtr win,EventRecord *event);
void MessGonnaShow(MyWindowPtr win);
void MessDidResize(MyWindowPtr win, Rect *oldContR);
void MessSwapTXE(MessHandle messH);
void MessCursor(Point mouse);
void PriorMenuHelp(MyWindowPtr win,Rect *priorRect);
Boolean GetPriorityRect(MyWindowPtr win,Rect *pr);
void DrawPriority(Rect *pr,short p);
short PriorityMenu(MyWindowPtr win);
int UnwrapSave(UPtr text, short length, short offset, short refN);
Boolean MessApp1(MyWindowPtr win,EventRecord *event);