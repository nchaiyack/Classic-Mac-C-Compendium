/*
>>	This file handles a file selection box for xdizzy
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
>>
>>	Please read the included file called "DizzyDoc" for information on
>>	what your rights are concerning this product.
>>
*/

#include "dizzy.h"
#ifndef MACINTOSH
#include "xstuff.h"
#include <Xm/FileSB.h>
#include <X11/Shell.h>

static	Widget	MyFileSel;			/*	The file selection widget.		*/
static	int 	StopFileSelect=0;	/*	True, when user has responded.	*/
static	Widget	top2;				/*	MyFileSel top level widget.		*/

/*
>>	User clicked ok, and this callback was called.
*/
void	DoOk(w,ignore,calldata)
Widget						w;
char						*ignore;
XmFileSelectionBoxCallbackStruct	*calldata;
{
	StopFileSelect= -2;
	strncpy(DestFileName,XmTextGetString(
						 XmFileSelectionBoxGetChild(
						  MyFileSel,XmDIALOG_TEXT)),
						  MAXFILENAME-1);
	XtUnmapWidget(top2);
	XFlush(Disp);
	MainDisable=0;
}

/*
>>	User clicked cancel, and this callback was called.
*/
void	DoCancel(w,ignore,calldata)
Widget						w;
char						*ignore;
XmFileSelectionBoxCallbackStruct	*calldata;
{
	StopFileSelect= -1;
	XtUnmapWidget(top2);
	XFlush(Disp);
	MainDisable=0;
}
/*
>>	Create a file selection widget.
*/
void	InitFileSelection()
{
	top2=XtCreateApplicationShell("FileSel",topLevelShellWidgetClass,NULL, 0);
	MyFileSel=XmCreateFileSelectionBox(top2,"MyFileSel",Wargs,0);

	XtAddCallback(MyFileSel, XmNokCallback, DoOk, 0L);
	XtAddCallback(MyFileSel, XmNcancelCallback, DoCancel, 0L);
	XtUnmanageChild(XmFileSelectionBoxGetChild(MyFileSel,XmDIALOG_HELP_BUTTON));
	XtManageChild(MyFileSel);
	XtRealizeWidget(top2);
}

static	int 	needsinit= -1;	/*	Has the above routine been called?	*/

/*
>>	DestFileName is changed to reflect the file name the user chose.
>>	Returns true, if the user didn't cancel.
*/
int 	DoFileSelection()
{
	if(needsinit)	InitFileSelection();
	else			XtManageChild(MyFileSel);

	MainDisable= -1;
	StopFileSelect=0;
	XmTextSetString(XmFileSelectionBoxGetChild(MyFileSel,XmDIALOG_TEXT),DestFileName);

#ifdef	RESETFILTER
	XmTextSetString(XmFileSelectionBoxGetChild(MyFileSel,XmDIALOG_FILTER_TEXT),"./*");
#endif

	if(needsinit)	needsinit=0;
	else			XtMapWidget(top2);

	do
	{	if(XtPending())
		{	XtNextEvent(&MyEvent);
			XtDispatchEvent(&MyEvent);
		}
	} while(!StopFileSelect);
	
	XtUnmanageChild(MyFileSel);
	return (StopFileSelect == -2);
}
#endif
