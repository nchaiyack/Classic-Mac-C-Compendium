/**********************************************************************\

File:		error.h

Purpose:	This is the header file for error.c

\**********************************************************************/

extern	NMRec		gMyNotification;
extern	int			gPendingResultCode;

void HandleError(int resultCode, Boolean exitToShell);
