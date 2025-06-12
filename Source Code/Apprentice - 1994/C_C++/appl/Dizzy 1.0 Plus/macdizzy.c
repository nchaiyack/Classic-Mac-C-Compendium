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
>>	This file contains the main program and even handling for the Macintosh.
>>	Consult DizzyX.c, if you want the Xt/Motif versions of similar routines.
>>
*/

#define MAIN_PROGRAM
#include	"dizzy.h"
#ifdef MACINTOSH

void	CalculatePanelPlaces()
{
	/*	Find out menu pane size from picture size.		*/
	MenuR=(*MenuP)->picFrame;
	OffsetRect(&MenuR,4-MenuR.left,4-MenuR.top);
	
	/*	Find tool pane size and move pane below menus.	*/
	ToolR=(*ToolP)->picFrame;
	OffsetRect(&ToolR,4-ToolR.left,MenuR.bottom+4-ToolR.top);
	
	/*	Set up the input pin count selector display.	*/
	InputSelector=2;
	InputFrame.left=ToolR.left+34;
	InputFrame.right=InputFrame.left+21;
	InputFrame.top=ToolR.top+50;
	InputFrame.bottom=InputFrame.top+21;
	
	/*	Set up the editor pane to take up the rest of window space. */
	EditR=PortRect;
	EditR.left=(ToolR.right>MenuR.right) ? ToolR.right : MenuR.right;
	InsetRect(&EditR,5,5);
	EditOutline=EditR;
	InsetRect(&EditOutline,-1,-1);
	
	/*	Create a region to contain the gray area outside all panes. */
	OpenRgn();
	FrameRect(&PortRect);
	FrameRect(&ToolR);
	FrameRect(&MenuR);
	FrameRect(&EditR);
	CloseRgn(BackRegion);
}
/*
>>	When a mousedown is detected, one has to find what was clicked
>>	and act accordingly. This is actually very simple stuff...nuff said.
*/
void	DoMouseDown()
{
	int 		Place;
	Point		MousePoint;
	WindowPtr	TheWind;
	Rect		BigGrow={200,180,32000,32000};
	Rect		DragRect={-32000,-32000,32000,32000};
	long		NewWindowSize;
	
	Place=FindWindow(MyEvent.where,&TheWind);
	switch(Place)
	{	case inMenuBar:
			break;
		case inDrag:
			DragWindow(TheWind,MyEvent.where,&DragRect);
			break;
		case inGrow:
			NewWindowSize = GrowWindow(TheWind,MyEvent.where,&BigGrow);
			SizeWindow(TheWind,LoWord(NewWindowSize),HiWord(NewWindowSize),0);
			SetPort(TheWind);
			InvalRect(&TheWind->portRect);
			PortRect = TheWind->portRect;
			CalculatePanelPlaces();
			break;
		case inZoomIn:
		case inZoomOut:
			if(TrackBox(TheWind,MyEvent.where,Place))
			{	SetPort(TheWind);
				ZoomWindow(TheWind,Place,0);
				InvalRect(&TheWind->portRect);
				PortRect = TheWind->portRect;
				CalculatePanelPlaces();
			}
			break;
		case inContent:
			if(TheWind==FrontWindow())
			{	MousePoint=MyEvent.where;
				SetPort(MyWind);
				GlobalToLocal(&MousePoint);
				if(PtInRect(MousePoint,&MenuR))
					DoMenuClick();
				else
				if(PtInRect(MousePoint,&ToolR))
					DoToolClick();
				else
				if(PtInRect(MousePoint,&EditR))
					DoEditClick();
			}
			else
			{	SelectWindow(TheWind);
			}
			break;
		case inGoAway:
			if(TrackGoAway(TheWind,MyEvent.where))
			{	HideWindow(TheWind);
				QuitNow= -1;
			}
			break;
		case inSysWindow:
			SystemClick(&MyEvent,TheWind);
			break;
	}
}

/*
>>	Redraw main window contents when an update event is received.
>>	If this update is for some other window, we're in deep
>>	sh*t, since there should be just one window.
*/
void	DoUpdate()
{
	WindowPtr	TheWind;
	
	TheWind=(WindowPtr)MyEvent.message;

	BeginUpdate(MyWind);
	SetPort(MyWind);

	if(TheWind==MyWind)
	{	
		DrawPicture(MenuP,&MenuR);
		HILITEMODE;
		InvertRect(&MenuHilite);
		DrawPicture(ToolP,&ToolR);
		UpdateButtonTags();
		FillRgn(BackRegion,gray);
		FrameRect(&EditOutline);
		ClipEditArea(); 	
		UpdateSim();
		RestoreClipping();
	}
	else
	{	EraseRect(&PortRect);
	}
	EndUpdate(TheWind);
}
/*
>>	UISetup loads PICTs and bitmaps, sets up some
>>	rectangle structures that divide our window into
>>	subpanes. If you want to add resizing windows,
>>	some of this code has to be moved to a resize
>>	handler. See X-version for details on how to do it.
*/
void	UISetup()
{
	int 		i;
	Handle		filenamer;
	Rect		TitleRect,TitleFrame;
	PicHandle	TitleHand;

	/*	Create our main window and find its size.		*/
	MyWind=GetNewWindow(128,0,(WindowPtr)-1);
	SetPort(MyWind);
	PortRect=MyWind->portRect;

	/*	Load pictures for tool and menu panes.			*/
	MenuP=(PicHandle)GetResource('PICT',128);
	ToolP=(PicHandle)GetResource('PICT',129);
	HLock(MenuP);
	HLock(ToolP);
	
	BackRegion=NewRgn();
	CalculatePanelPlaces();
	
	SetRect(&NilRect,0,0,0,0);	/*	Just a convenience. */

	/*	Load a bitmap with misc stuff. Equivalent of misc.xbm		*/
	PictBit(&ButtonBits,130);

	/*	Set up default tools and lock them to three mouse buttons.	*/
	for(i=0;i<MOUSE_BUTTONS;i++)
	{	ToolButtons[i].Function=17+i;
		SetRect(&ToolButtons[i].Prime,
					ToolR.left+33+i*24,ToolR.bottom-26,
					ToolR.left+56+i*24,ToolR.bottom-3);
		ToolLocks[i]=ToolButtons[i];
	}
	
	/*	Default file name is "Untitled". Read name from resource.	*/
	filenamer=(Handle)GetString(128);
	BlockMove(*filenamer,NameOfOpenFile.fName,GetHandleSize(filenamer));
	FileIsNamed=0;
	
	/*	Draw window contents.										*/
	MyEvent.message=(long)MyWind;
	DoUpdate();
	
	/*	Draw a splash screen containing a copyright message.		*/
	SetPort(MyWind);
	TitleHand=(PicHandle)GetResource('PICT',131);
	HLock(TitleHand);
	TitleFrame=(*TitleHand)->picFrame;
	OffsetRect(&TitleFrame,-TitleFrame.left,-TitleFrame.top);
	OffsetRect(&TitleFrame,-TitleFrame.right/2,-TitleFrame.bottom/2);
	OffsetRect(&TitleFrame, EditR.left+(EditR.right-EditR.left)/2,
							EditR.top+(EditR.bottom-EditR.top)/2);
	TitleRect=TitleFrame;
	InsetRect(&TitleFrame,-32,-32);
	EraseRect(&TitleFrame);
	FrameRect(&TitleFrame);
	DrawPicture(TitleHand,&TitleRect);
	HUnlock(TitleHand);
	ReleaseResource(TitleHand);
	SetTrashRect(&TitleFrame);
	SplashVisible=-1;
}

void	main()
{
	char			thechar;
	unsigned long	NextSimulation;

	NextSimulation=0;

	DoInits();		/*	Initialize toolbox managers 		*/
	SimSetup(); 	/*	Start up simulator variables		*/
	UISetup();		/*	Set up windows and stuff like that	*/
	
	QuitNow=0;		/*	Exit application, when QuitNow= -1	*/
	
	while(!QuitNow)
	{	SystemTask();
		if(GetNextEvent(everyEvent,&MyEvent))
		{	switch(MyEvent.what)
			{	case mouseDown:
					DoMouseDown();
					break;
				case keyDown:
				case autoKey:
					thechar=MyEvent.message;
					if(thechar>='0' && thechar <= '9')
					{	SimSpeed=3*(thechar-'0');
					}
					else
					if(thechar=='f')
					{	ConnectorFrames = ! ConnectorFrames;
						InvalRect(&EditR);
					}
					break;
				case updateEvt:
					DoUpdate();
					break;
				case activateEvt:
					SetPort(&MyWind);
					break;
			}
		}
		
		if(MyEvent.when>=NextSimulation)
		{	SimLevel=0; 		/*	Top level simulation.		*/
			SimTimer++; 		/*	Counter for clock chips.	*/
			ClipEditArea();
			RunSimulation();
			RestoreClipping();
			NextSimulation=MyEvent.when+SimSpeed;
		}
	}
}

#endif
