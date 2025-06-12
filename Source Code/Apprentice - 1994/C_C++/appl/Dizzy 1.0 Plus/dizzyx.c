/*
>>	Dizzy 1.0
>>
>>	A digital circuit simulator & design program for the X Window System
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
>>
>>	Please read the included file called "DizzyDoc" for information on
>>	what your rights are concerning this product.
>>
>>	This file contains the main program and even handling for the X window system.
>>	Consult DizzyMac.c, if you want the mac versions of the same routines.
>>
*/

#include "dizzy.h"
#ifndef MACINTOSH
#define XMAIN_PROGRAM	YES
#include <sys/time.h>
#include "X11/cursorfont.h"
#include "xstuff.h"
#include "menu.xbm"
#include "tool.xbm"
#include "misc.xbm"
#include "splash.xbm"

XRectangle	XClipR;		/*	Another representation of PortRect.	*/

/*
>>	This routines determines what to do when a button is clicked
>>	on our window. If the file selection dialog is active, nothing
>>	is done.
*/
void	ButtonDownEvent()
{
	Point	MousePoint;
	
	if(MainDisable) return;
	
	DownTrigger=TRUE;
	GetMouseDownPoint(&MousePoint);
	if(PtInRect(MousePoint,&ToolR))
		DoToolClick();
	else
	{	CancelDoubleToolClick();
		if(PtInRect(MousePoint,&MenuR))
			DoMenuClick();
		else
		if(PtInRect(MousePoint,&EditR))
			DoEditClick();
	}
}
/*
>>	This routine is called when a expose is received. It first
>>	combines all available expose events and then updates
>>	our window.
*/
void	DoRedisplay(w, ignore, calldata)
Widget						w;
char						*ignore;
XmDrawingAreaCallbackStruct *calldata;
{
	int 			i;
	Rect			bar;
	Region			temp1,temp2;
	XRectangle		tempr;
	Region			UpRegion;
	
	UpRegion=XCreateRegion();

	XtAddExposureToRegion(calldata->event, UpRegion);

	while(XCheckTypedWindowEvent(Disp, Wind, Expose, &MyEvent))
	{	XtAddExposureToRegion(&MyEvent, UpRegion);
	}

	XSetRegion(Disp, Mode, UpRegion);

	/*	Paint gray background.	*/
	PenBGray();
	SetRect(&bar,0,0,EditR.left,PortRect.bottom);						PaintRect(&bar);
	SetRect(&bar,EditR.left,0,EditR.right,EditR.top);					PaintRect(&bar);
	SetRect(&bar,EditR.left,EditR.bottom,EditR.right,PortRect.bottom);	PaintRect(&bar);
	SetRect(&bar,EditR.right,0,PortRect.right,PortRect.bottom); 		PaintRect(&bar);
	PenBlack();

	FrameRect(&EditOutline);		/*	Frame edit area outline.		*/
	PixCopy(MenuPix,0,0,&MenuR);	/*	Paint menu area.				*/
	PixCopy(ToolPix,0,0,&ToolR);	/*	Paint tool area.				*/
	InvertRect(&MenuHilite);		/*	Invert menu selection (if any).	*/
	UpdateButtonTags();				/*	Show which tools are active.	*/

	ClipEditArea();					/*	Just sets the coord origin.		*/
	temp1=XCreateRegion();			/*	Leave only edit area in the		*/
	temp2=XCreateRegion();			/*	clipping region by doing some	*/
	tempr.x=EditR.left;				/*	region arithmetic.				*/
	tempr.y=EditR.top;
	tempr.width=EditR.right-EditR.left;
	tempr.height=EditR.bottom-EditR.top;
	XUnionRectWithRegion(&tempr,temp2,temp1);
	XIntersectRegion(UpRegion,temp1,temp2);
	XSetRegion(Disp, Mode, temp2);

	UpdateSim();					/*	Update simulation contents.		*/
	
	RestoreClipping();				/*	Restore coordinate origin.		*/
	XSetRegion(Disp,Mode,UpRegion);	/*	Restore to expose clipping.		*/

	if(SplashVisible)				/*	Display copyright, if necessary.*/
	{	Rect	SplashBitRect,SplashRect;
		
		SplashBitRect.left=(EditR.left+EditR.right-splash_width)/2;
		SplashBitRect.top=(EditR.top+EditR.bottom-splash_height)/2;
		SplashBitRect.right=SplashBitRect.left+splash_width;
		SplashBitRect.bottom=SplashBitRect.top+splash_height;
		SplashRect=SplashBitRect;
		InsetRect(&SplashRect,-31,-31);
		EraseRect(&SplashRect);					/*	Erase background.	*/
		InsetRect(&SplashRect,-1,-1);
		FrameRect(&SplashRect);					/*	Draw a frame.		*/
		PixCopy(SplashPix,0,0,&SplashBitRect);	/*	Splash!				*/
		SetTrashRect(&SplashRect);				/*	Prepare to erase.	*/
	}

	XDestroyRegion(temp1);			/*	Destroy temporary regions.		*/
	XDestroyRegion(temp2);
	XDestroyRegion(UpRegion);

	/*	Restore clipping to normal.	*/
	XSetClipRectangles(Disp,Mode,0,0,&XClipR,1,Unsorted);
}
/*
>>	When a resize is reported, some data structures need to be
>>	recalculated and the screen needs an update.
*/
void	DoResize(w, ignore, ignoremetoo)
Widget	w;
char	*ignore;
char	*ignoremetoo;
{
	Dimension	h,v;

	if(XtIsRealized(Canvas))
	{	XClearArea(Disp, XtWindow(Canvas), 0,0,0,0, TRUE);
	}
	XtSetArg(Wargs[0], XtNwidth, &h);
	XtSetArg(Wargs[1], XtNheight, &v);
	XtGetValues(Canvas,Wargs,2);

	PortRect.left=0;
	PortRect.top=0;
	PortRect.right=h;
	PortRect.bottom=v;

	XClipR.x=0;
	XClipR.y=0;
	XClipR.width=PortRect.right;
	XClipR.height=PortRect.bottom;
	XSetClipRectangles(Disp,Mode,0,0,&XClipR,1,Unsorted);
	
	SetRect(&MenuR,4,4,menu_width+4,menu_height+4);
	SetRect(&ToolR,4,8+menu_height,tool_width+4,menu_height+8+tool_height);

	EditR=PortRect;
	EditR.left=(ToolR.right>MenuR.right) ? ToolR.right : MenuR.right;
	InsetRect(&EditR,5,5);
	EditOutline=EditR;
	InsetRect(&EditOutline,-1,-1);
}
/*
>>	This initializes X, Xt and Motif dependent stuff.
>>	Called only once from main.
*/
void	UISetup(argc,argv)
int 	argc;
char	*argv[];
{
	int 		i;

	Toplevel=XtInitialize(argv[0],"X Dizzy",NULL,0,&argc,argv);

	Disp=XtDisplay(Toplevel);

	Black=BlackPixel(Disp,DefaultScreen(Disp));
	White=WhitePixel(Disp,DefaultScreen(Disp));

	XtSetArg(Wargs[0], XtNwidth, INITIAL_WIDTH);
	XtSetArg(Wargs[1], XtNheight, INITIAL_HEIGHT);
	XtSetArg(Wargs[2], XtNbackground, White);
	XtSetArg(Wargs[3], XtNforeground, Black);
	
	Canvas = XtCreateManagedWidget("Canvas",xmDrawingAreaWidgetClass,
									Toplevel, Wargs, 4);

	Wind=XtWindow(Canvas);
	Disp=XtDisplay(Canvas);

	XtAddCallback(Canvas, XmNexposeCallback, DoRedisplay, 0L);
	XtAddCallback(Canvas, XmNresizeCallback, DoResize, 0L);
	XtAddEventHandler(Canvas, ButtonPressMask, FALSE, ButtonDownEvent, &Canvas);

	Mode=XCreateGC(XtDisplay(Canvas),DefaultRootWindow(XtDisplay(Canvas)),NULL,NULL);

	MyFont=XLoadQueryFont(Disp,"-*-helvetica-medium-r-*--10-*");
	XSetFont(Disp,Mode,MyFont->fid);

	XSetFunction(Disp,Mode,GXcopy);
	XSetForeground(Disp,Mode,Black);
	XSetBackground(Disp,Mode,White);

	XtRealizeWidget(Toplevel);
	MenuPix=CreatePixMap(menu_bits,menu_width,menu_height);
	ToolPix=CreatePixMap(tool_bits,tool_width,tool_height);
	MiscPix=CreatePixMap(misc_bits,misc_width,misc_height);
	SplashPix=CreatePixMap(splash_bits,splash_width,splash_height);
	SplashVisible= -1;

	InitXGraf();		/*	More graphics stuff preparation.	*/

	DoResize(Canvas,0,0);

/*	Preset tool palette functions to defaults:					*/
	InputSelector=2;
	InputFrame.left=ToolR.left+34;
	InputFrame.right=InputFrame.left+21;
	InputFrame.top=ToolR.top+50;
	InputFrame.bottom=InputFrame.top+21;
	
	for(i=0;i<MOUSE_BUTTONS;i++)
	{	ToolButtons[i].Function=17+i;
		SetRect(&ToolButtons[i].Prime,
					ToolR.left+33+i*24,ToolR.bottom-26,
					ToolR.left+56+i*24,ToolR.bottom-3);
		ToolLocks[i]=ToolButtons[i];
	}
	XGrabButton(Disp, AnyButton, AnyModifier, Wind, TRUE,
				ButtonPressMask | ButtonMotionMask | ButtonReleaseMask,
				GrabModeAsync, GrabModeAsync,
				Wind,XCreateFontCursor(XtDisplay(Canvas),XC_left_ptr));

}
/*
>>	What can I say? A main is a main. It contains a main even loop
>>	and calls a few initialization routines.
*/
int 	main(argc,argv)
int 	argc;
char	*argv[];
{
	QuitNow=0;			/*	Do not quite immediatelly.		*/
	MainDisable=0;		/*	Do not disable main window.		*/
	SimSetup(); 		/*	Start up simulator variables.	*/
	UISetup(argc,argv); /*	Start up user interface stuff.	*/
	FileIsNamed=0;		/*	File is now unnamed.			*/
	CurFileName[0]=0;	/*	Zero length string.				 */


	do
	{	if(XtPending())
		{	XtNextEvent(&MyEvent);
			XtDispatchEvent(&MyEvent);
		}
		else
		{	struct timeval	timeout;
			int 			readfds=0;
			int 			maxfds=1+ConnectionNumber(Disp);
			
			SimLevel=0;
			SimTimer++;
			ClipEditArea();
			RunSimulation();
			RestoreClipping();
			XFlush(Disp);

			timeout.tv_sec=0;
			timeout.tv_usec=100000;
			
			readfds= 1 << ConnectionNumber(Disp);
			select(maxfds, &readfds, NULL, NULL, &timeout);
		}
	} while(!QuitNow);

	if(QuitNow)
	{	XtCloseDisplay(Disp);
		exit(0);
	}
}
#endif
