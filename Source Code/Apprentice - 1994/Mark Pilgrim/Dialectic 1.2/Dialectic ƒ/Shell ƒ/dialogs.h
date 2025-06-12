/**********************************************************************\

File:		dialogs.h

Purpose:	This is the header file for dialogs.c

\**********************************************************************/

enum
{
	smallAlert = 128,		/* for small stuff */
	largeAlert,				/* for large stuff (like prefs errors) */
	personalDialog = 131	/* personalization dialog box */
};

void PositionDialog(ResType theType, short theID);
pascal void OutlineDefaultButton(DialogPtr myDlog, short itemNum);
pascal Boolean ProcOFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem);
