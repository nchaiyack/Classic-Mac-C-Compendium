/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * declarations for dealing with POP server
 ************************************************************************/
short GetMyMail(Boolean quietly);
int POPError(void);
void AddAttachNote(short wdRef, UPtr name, long creator, long type);
void AddAttachInfo(int theIndex, long result);
int POPMessageBody(long estSize);
#ifdef POPSECURE
short VetPOP(void);
#endif
