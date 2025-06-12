/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
Boolean SaveComp(MyWindowPtr win);
int CountCompLines(MessType **messH);
int FindCompTx(MessType **messH, TEHandle txe);
MyWindowPtr OpenComp(TOCType **tocH, int sumNum, MyWindowPtr win, Boolean showIt);
int QueueSelectedMessages(TOCType **tocH,short toState,uLong when);
void CreateMessageBody(UPtr buffer);
long CountCompBytes(MessType **messH);
void UpdateSum(MessType **messH, long offset, long length);
