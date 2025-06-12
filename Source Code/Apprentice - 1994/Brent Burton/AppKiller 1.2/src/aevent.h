/*
** aevent.h
**
** definitions for aevent.c -- my apple events code
*/

#define AK_EventClass  'APKL'

void InitAEs(void);
void DoAppleEvent(EventRecord*);

