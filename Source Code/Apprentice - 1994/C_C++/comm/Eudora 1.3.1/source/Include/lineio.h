/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
short OpenLine(short vRef,long dirId,UPtr name);
int GetLine(UPtr line,int size);
void CloseLine(void);
long TellLine(void);
int SeekLine(long spot);
typedef enum {LINE_START=1, LINE_MIDDLE} GetLineEnum;


