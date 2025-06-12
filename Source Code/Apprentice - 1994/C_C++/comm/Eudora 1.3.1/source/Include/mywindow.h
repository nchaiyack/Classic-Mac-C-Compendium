/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * my own window package
 **********************************************************************/
typedef struct
{
	union
	{
		DialogRecord dRec;			/* the dialog manager record */
		WindowRecord wRec;			/* and the quickdraw window structure */
	} winUnion;
#define qWindow winUnion.wRec
#define qDialog winUnion.dRec
	Boolean isDialog; 				/* is this a dialog? */
	Boolean isRunt; 					/* should not grow */
	ControlHandle vBar; 			/* vertical scroll bar, if any */
	ControlHandle hBar; 			/* horizontal scroll bar, if any */
	int 			vMax;
	int 			hMax;
	Byte			hPitch;
	Byte			vPitch;
	short topMargin;					/* distance from top of window to top of contR */
	Rect			contR;					/* real content rectangle */
	Boolean 		isActive; 		/* the window is topmost */
	Boolean 		isDirty;			/* the contents of the window are dirty */
	Boolean 		hasSelection; /* the window contains a selection */
	void (*update)(); 				/* handle update events */
	void (*click)();					/* handle clicks in the content region */
	void (*activate)(); 			/* handle activation/deactivation */
	Boolean (*scroll)();			/* handle scrolling in lieu of normal mechanism */
	void (*didResize)();			/* act after a window has been resized */
	Boolean (*menu)();				/* handle a menu selection */
	Boolean (*close)(); 			/* close the window */
	Boolean (*key)(); 				/* we saw a keystroke */
	Boolean (*position)();		/* save/restore window position */
	void (*cursor)(); 				/* set the cursor  and mouse region */
	void (*button)(); 				/* a button was hit */
	void (*showInsert)(); 		/* show the insertion point */
	void (*help)(); 					/* show help balloon for the window */
	void (*gonnaShow)();			/* get ready to become visible */
	void (*textChanged)();		/* called after a text has changed */
	void (*zoomSize)();				/* size the zoom rectangle */
	Boolean (*app1)();				/* handle an app1 (page keys) */
	Boolean (*hit)(); 				/* for dialog windows */
	TEHandle txe; 						/* handle to textedit, if any */
	TEHandle ste;							/* handle to scrolling TE field, if any */
	Boolean ro; 							/* current txe is ro */
	Boolean dontControl;			/* leave control handling to the click routine */
	Boolean saveSize; 				/* save the size when the window is closed */
	Boolean hot;							/* pass click through after activate */
	Point minSize;						/* how small to allow the window to be */
	Boolean inUse;						/* is the window in use? */
} MyWindow, *MyWindowPtr;

/**********************************************************************
 * prototypes
 **********************************************************************/
MyWindowPtr GetNewMyWindow(short resId,UPtr wStorage,WindowPtr behind,Boolean hBar, Boolean vBar);
void UpdateMyWindow(MyWindowPtr win);
void ScrollIt(MyWindowPtr win,int deltaH,int deltaV);
void EraseUpdateRgn(MyWindowPtr win);
void DragMyWindow(MyWindowPtr win,Point thePt);
void GrowMyWindow(MyWindowPtr win,Point thePt);
void ZoomMyWindow(MyWindowPtr win,Point thePt,int partCode);
void OffsetWindow(WindowPtr win);
void MyWindowDidResize(MyWindowPtr win,Rect *oldContR);
void MoveMyCntl(MyWindowPtr win,ControlHandle cntl,int h,int v,int w,int t);
short IncMyCntl(ControlHandle cntl,short inc);
void ScrollMyWindow(MyWindowPtr win,int h,int v);
void MyWindowMaxes(MyWindowPtr win,int hMax,int vMax);
int BarMax(ControlHandle cntl,int max,int winSize,int pitch);
pascal void ScrollAction(ControlHandle theCtl,short partCode);
void ActivateMyWindow(MyWindowPtr win,Boolean active);
void InvertLine(MyWindowPtr win,int line);
Boolean CloseMyWindow(MyWindowPtr win);
void GoAwayMyWindow(WindowPtr win,Point pt);
void DrawMyGrow(MyWindowPtr win);
#define IsMyWindow(wp) ((wp) && ((MyWindowPtr)wp)->qWindow.windowKind==dialogKind &&\
												((MyWindowPtr)wp)->qWindow.refCon==CREATOR || \
												((MyWindowPtr)wp)->qWindow.windowKind>=userKind)
void InvalContent(MyWindowPtr win);
void ShowMyWindow(MyWindowPtr win);
void InfiniteClip(MyWindowPtr win);
pascal Boolean MyClikLoop(void);
short CalcCntlInc(ControlHandle cntl,short tentativeInc);
TEHandle WinTEH(MyWindowPtr win);
Boolean ScrollIsH(ControlHandle cntl);
void UsingWindow(MyWindowPtr win);
void NotUsingWindow(MyWindowPtr win);
void NotUsingAllWindows(void);
void FigureZoom(MyWindowPtr win);
short GDIndexOf(GDHandle gd);
