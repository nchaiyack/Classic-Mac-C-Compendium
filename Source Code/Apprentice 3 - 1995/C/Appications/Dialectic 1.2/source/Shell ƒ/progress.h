/**********************************************************************\

File:		progress.h

Purpose:	This is the header file for progress.c

\**********************************************************************/

extern	DialogPtr		gProgressDlog;

DialogPtr OpenProgressDialog(unsigned long max, Str255 theTitle);
void UpdateProgressDialog(unsigned long cur);
void SetProgressText(Str255 p1, Str255 p2, Str255 p3, Str255 p4);
void DismissProgressDialog(void);
Boolean DealWithOtherPeople(void);
