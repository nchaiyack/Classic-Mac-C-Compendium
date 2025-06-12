/* Copyright (c) 1992 by Qualcomm, Inc. */
/* written by Steven Dorner */

/************************************************************************
 * Scrolling TE field in a window
 ************************************************************************/

typedef struct
{
	TEHandle te;
	ControlHandle vBar;
	ControlHandle hBar;
	Boolean canEdit;
	Boolean dontFrame;
	Boolean growBox;
	Rect encloseR;
}	STE, *STEPtr, **STEHandle;

STEHandle NewSTE(MyWindowPtr win,Rect *enclosingRect,Boolean hBar,Boolean vBar,Boolean canEdit);
void STEDispose(STEHandle ste);
Boolean STEClick(STEHandle ste,EventRecord *event);
void ResizeSTE(STEHandle ste,Rect *newRect);
void STESetText(UPtr text,short length,STEHandle ste);
void STEInstallText(Handle text,STEHandle ste);
Boolean STECursor(STEHandle ste);
void STEShowInsert(STEHandle ste);
void STEUpdate(STEHandle ste);
void STEScroll(STEHandle ste,short h,short v);
void STETextChanged(STEHandle ste);
void STEApp1(STEHandle ste,EventRecord *event);
void STEAppendText(UPtr text,short length,STEHandle ste);
void STESetScrolls(STEHandle ste);
