/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * declarations for finding things
 ************************************************************************/
typedef struct
{
	short item;
	long dirId;
} BoxCountElem,*BoxCountPtr,**BoxCountHandle;
void DoFind(short item);
void EnableFindMenu(void);
void BuildBoxCount(void);
