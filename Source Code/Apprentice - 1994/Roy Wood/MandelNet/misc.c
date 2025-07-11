#include "mandel.h"
#include <math.h>



pascal void HiliteDefaultButton(theDPtr, whichItem)
DialogPtr theDPtr; int whichItem;
{
int			type;
Handle		theItem;
Rect		theRect;

	GetDItem(theDPtr, ((DialogPeek) theDPtr)->aDefItem, &type, &theItem, &theRect);
	PenNormal();
	PenSize(3,3);
	InsetRect(&theRect,-4,-4);
	FrameRoundRect(&theRect,16,16);
	PenSize(1,1);
}



CenterWindow(wPtr)
WindowPtr	wPtr;
{
int		screenWidth,screenHeight,windowWidth,windowHeight;

	screenWidth = screenBits.bounds.right - screenBits.bounds.left;
	screenHeight = screenBits.bounds.bottom - screenBits.bounds.top - 20;
	windowWidth = wPtr->portRect.right - wPtr->portRect.left;
	windowHeight = wPtr->portRect.bottom - wPtr->portRect.top;
	
	if (wPtr && windowWidth<screenWidth && windowHeight<screenHeight)
		MoveWindow(wPtr,screenBits.bounds.left + (screenWidth - windowWidth)/2,screenBits.bounds.top + 20 + (screenHeight - windowHeight)/2,FALSE);
}



FatalError(p0)
char	*p0;
{
	ErrorAlert(p0);
	ExitToShell();
}



OptionKey()
{
char	theKeyMap[16];

	GetKeys(theKeyMap);
	
	if (theKeyMap[7] & 0x04)
		return(TRUE);
	else
		return(FALSE);
}



ErrorAlert(p0)
char	*p0;
{
GrafPtr			oldPort;
DialogPtr		theDPtr,tempDPtr;
int				itemHit, type;
Handle			theItem;
Rect			theRect;
EventRecord		theEvent;

	GetPort(&oldPort);
	
	InitCursor();
	
	if (!(theDPtr = GetNewDialog(errorDLOG, NULL,(WindowPtr) -1L)))
	{
		SysBeep(1);
		ExitToShell();
	}
	
	SetPort(theDPtr);
	
	CenterWindow(theDPtr);
	ShowWindow(theDPtr);
	
	((DialogPeek) theDPtr)->aDefItem = 1;
	
	GetDItem(theDPtr, 3, &type, &theItem, &theRect);
	SetDItem(theDPtr, 3, type, HiliteDefaultButton, &theRect);
	
	GetDItem(theDPtr, 2, &type, &theItem, &theRect);
	SetIText(theItem, p0);
	
	if (GetNextEvent(updateMask,&theEvent) && theEvent.message == (long) theDPtr)
		DialogSelect(&theEvent,&tempDPtr,&itemHit);
	
	SysBeep(1);
	
	do 
	{
		itemHit = 0;
		
		while (!GetNextEvent(everyEvent,&theEvent));
		
		if (theEvent.what==keyDown || theEvent.what==autoKey)
		{
			if ((theEvent.message & charCodeMask)=='\r' || (theEvent.message & charCodeMask)==0x03)
				itemHit = ((DialogPeek) theDPtr)->aDefItem;
			else
				SysBeep(1);
		}
		
		else
		{
			tempDPtr = (DialogPtr) 0L;
			
			if (!IsDialogEvent(&theEvent) || !DialogSelect(&theEvent,&tempDPtr,&itemHit) || tempDPtr!=theDPtr)
				itemHit = 0;
		}
		
	} while (itemHit!=1);
	
	DisposDialog(theDPtr);
	SetPort(oldPort);
}





GetInput(thePrompt, theText)
char	*thePrompt,*theText;
{
DialogPtr	theDPtr;
int			itemHit, type;
Handle		theItem;
Rect		theRect;
GrafPtr		oldPort;

	GetPort(&oldPort);

	ParamText(thePrompt,"\p","\p","\p");
	
	theDPtr = GetNewDialog(inputDLOG, NULL,(WindowPtr) -1L);
	
	if (theDPtr == 0L)
		FatalError("\p Can't allocate memory for window");
	
	SetPort(theDPtr);
	
	CenterWindow(theDPtr);
	ShowWindow(theDPtr);
	
	((DialogPeek) theDPtr)->aDefItem = 1;
	
	GetDItem(theDPtr, 5, &type, &theItem, &theRect);
	SetDItem(theDPtr, 5, type, HiliteDefaultButton, &theRect);
	
	GetDItem(theDPtr, 4, &type, &theItem, &theRect);
	CtoPstr(theText);
	SetIText(theItem, theText);
	SelIText(theDPtr, 4, 0, 256);
	PtoCstr(theText);
	
	do 
	{
		ModalDialog(0L,&itemHit);
	}	
	while ((itemHit!=1)&&(itemHit!=2));
	
	
	GetDItem(theDPtr, 4, &type, &theItem, &theRect);
	
	if (itemHit==1)
	{
		GetIText(theItem,  theText);
		PtoCstr(theText);
		
		if (theText[0] != '\000')
			itemHit = 0;
		
		else
			itemHit = -1;
	}
	else
	{
		theText[0] = '\000';
		itemHit = -1;
	}
	
	DisposDialog(theDPtr);
	SetPort(oldPort);
	
	return(itemHit);
}




SetUpMenus()
{	
	myMenus[appleMenu] = GetMenu(appleID);
	AddResMenu(myMenus[appleMenu],'DRVR');
	myMenus[fileMenu] = GetMenu(fileID);
	myMenus[editMenu] = GetMenu(editID);
	myMenus[modeMenu] = GetMenu(modeID);
	myMenus[graphMenu] = GetMenu(graphID);
	myMenus[controlMenu] = GetMenu(controlID);
	
	InsertMenu(myMenus[appleMenu],0);
	InsertMenu(myMenus[fileMenu],0);
	InsertMenu(myMenus[editMenu],0);
	InsertMenu(myMenus[modeMenu],0);
	InsertMenu(myMenus[graphMenu],0);
	
	DrawMenuBar();
}



int DoCommand(mResult)
long mResult;
{
int			theItem;
char		tempString[256];
WindowPeek 	wPtr;
	
	theItem = LoWord(mResult);
	
	switch (HiWord(mResult)) 
	{
		case appleID:
			if (theItem == aboutMe)
				DoAboutBox(0);
			
			else if (theItem == helpCommand)
				DoHelpBox(helpWIND,helpTEXT,helpSTYL);
				
			else
			{
				GetItem(myMenus[appleMenu], theItem, &tempString);
				OpenDeskAcc(&tempString);
				SetPort(theWPtr);
			}
			
			break;
			
		case fileID:
			alive = FALSE;
			break;
		
		case editID:
			SysBeep(1);
			break;
			
			
		case modeID:
			
			switch (theItem)
			{
				case slaveCommand:
					if (slaveMode == FALSE)
					{
						slaveMode = TRUE;
						
						if (ShutDownAppleTalk() != noErr)
						{
							ErrorAlert("\pError in attempt to revert to node mode (ShutDownAppleTalk failed)");
							
							slaveMode = FALSE;
						}
						
						else if (SetupAppleTalk() != noErr)
						{
							ErrorAlert("\pError in attempt to revert to node mode (SetupAppleTalk failed)");
							
							slaveMode = FALSE;
						}
						
						else if (AwaitRequest(receiveRecord,&theMessage) != noErr)
						{
							ErrorAlert("\pError in attempt to revert to node mode (AwaitRequest failed)");
							
							slaveMode = FALSE;
						}
						
						else
						{
							slaveMode = TRUE;
							
							DeleteMenu(controlID);
							DrawMenuBar();
						}
						
						DrawInfo();
					}
					
					break;
				
				case masterCommand:
					if (slaveMode == TRUE)
					{
						slaveMode = FALSE;
						
						if (KillRequest() != noErr)
						{
							ErrorAlert("\pError in attempt to enter master mode (can't kill outstanding ATP request)");
							
							slaveMode = TRUE;
						}
						
						else if (SetupAppleTalk() != noErr)
						{
							ErrorAlert("\pError in attempt to enter master mode (can't set up AppleTalk)");
									
							slaveMode = TRUE;
						}
						
						else
						{
							slaveMode = FALSE;
							
							LookupSlaves();
							
							InsertMenu(myMenus[controlMenu],0);
							DrawMenuBar();
						}
						
						DrawInfo();
					}
					
					break;
			}
			
			break;
			
		case graphID:
			
			switch (theItem)
			{	
				case setLimsCommand:
					if (SetGraphLimits() == 1)
						CopyBits(&(theWPtr->portBits),&graphBits,&graphRect,&graphRect,srcCopy,0L);
					break;
					
				case setIterCommand:
					SetMaxIterations();
					break;
				
				case setPixSizeCommand:
					SetPixelSize();
					break;
				
				case shadingCommand:
					shading = (shading) ? FALSE : TRUE;
					DrawInfo();
					break;
				
				case defaultLimsCommand:
					lims[xmin] = -2.0;
					lims[xmax] = 1.0;
					lims[ymin] = -1.5;
					lims[ymax] = 1.5;
					
					ClearGraph();
					DrawInfo();
					
					break;
				
				case clearGraphCommand:
					ClearGraph();
					CopyBits(&(theWPtr->portBits),&graphBits,&graphRect,&graphRect,srcCopy,0L);
					break;
				
				case mandelCommand:
					AssignMandelbrot();
					CopyBits(&(theWPtr->portBits),&graphBits,&graphRect,&graphRect,srcCopy,0L);
					break;
				
			}
			
			break;
		
		
		case controlID:
			
			switch (theItem)
			{	
				case checkSlavesCommand:
					LookupSlaves();
					DrawInfo();
					break;
				
				case stopSlavesCommand:
					StopSlaves();
					break;
				
				case slaveStatusCommand:
					CheckSlaves();
					break;
				
				case setDelayCommand:
					SetDelay();
					break;
				
				case meTooCommand:
					meToo = (meToo) ? FALSE : TRUE;
					DrawInfo();
					break;
			}
			
			break;
	}
	
	HiliteMenu(0);
}



DoContent(theEvent)
EventRecord	*theEvent;
{
GrafPtr			oldPort;


	GetPort(&oldPort);
	SetPort(theWPtr);
	GlobalToLocal(&theEvent->where);
	
	if (PtInRect(theEvent->where, &graphRect))
	{
		ZoomGraph(theEvent->where);
		
		DrawInfo();
	}
		
	else
		SysBeep(1);
	
	SetPort(oldPort);
}




InitWindow()
{
FontInfo	theFontInfo;


	theWPtr = GetNewWindow(mainWIND,&wRecord,-1L);
	
	if (theWPtr == 0L)
		FatalError("\p Can't allocate memory for window");
	
	SetPort(theWPtr);
	
	CenterWindow(theWPtr);
	ShowWindow(theWPtr);
	
	SetRect(&nullClipRect, -32768, -32768, 32767, 32767);
	
	infoRect = theWPtr->portRect;
	infoRect.right = infoRect.right - 256 - 2;
	infoRect.bottom = infoRect.top +256;
	
	messageRect = theWPtr->portRect;
	messageRect.top = messageRect.top + 256 + 2;
	
	graphRect = theWPtr->portRect;
	graphRect.left = graphRect.right - 256;
	graphRect.bottom = graphRect.top + 256;
	
	graphBits.rowBytes = (graphRect.right - graphRect.left + 1)/8;
	if (graphBits.rowBytes & 1) graphBits.rowBytes++;
	graphBits.bounds = graphRect;
	graphBits.baseAddr = NewPtr(graphBits.rowBytes * (graphRect.bottom - graphRect.top + 1));
	
	if (graphBits.baseAddr==0L)
		FatalError("\p Can't allocate memory for BitMap");
	
	
	ClearGraph();
	
	CopyBits(&(theWPtr->portBits),&graphBits,&graphRect,&graphRect,srcCopy,0L);
	
	TextFont(3);
	TextMode(srcCopy);
	TextSize(9);
	TextFace(0);
	GetFontInfo(&theFontInfo);
	
	coordRect.top = graphRect.top;
	coordRect.right = graphRect.right;
	coordRect.left = coordRect.right - StringWidth("\p (-0000.000,-0000.000) ");
	coordRect.bottom = coordRect.top + theFontInfo.ascent + theFontInfo.descent + 2;
	
	UpdateWindow();
}




UpdateWindow()
{
GrafPtr		oldPort;

	GetPort(&oldPort);
	SetPort(theWPtr);
	
	BeginUpdate(theWPtr);
	
	DrawInfo();
	
	ClearGraph();
	CopyBits(&graphBits,&(theWPtr->portBits),&graphRect,&graphRect,srcCopy,0L);
	
	DrawDisplayMessages();
	
	EndUpdate(theWPtr);
	
	SetPort(oldPort);
}




DrawInfo()
{
GrafPtr		oldPort;
char		tempString[256];
int			x,y;

	GetPort(&oldPort);
	SetPort(theWPtr);
	
	PenNormal();
	TextFont(3);
	TextFace(0);
	TextMode(srcCopy);
	TextSize(10);
	
	ClipRect(&infoRect);
	EraseRect(&infoRect);
	FrameRect(&infoRect);
	
	x = infoRect.left + 5;
	y = infoRect.top + 15;
	
	MoveTo(x,y);
	TextFace(bold);
	
	if (slaveMode==TRUE)
		DrawString("\pNode Mode");
	else
		DrawString("\pMaster Mode");
	
	sprintf(tempString,"Node #%d",myNode);
	CtoPstr(tempString);
	
	y += 12;
	MoveTo(x,y);
	TextFace(0);
	DrawString(tempString);
	
	sprintf(tempString,"Socket #%d",mySocket);
	CtoPstr(tempString);
	
	y += 12;
	MoveTo(x,y);
	DrawString(tempString);
	
	if (!slaveMode)
	{
		sprintf(tempString,"Node Count: %d",numSlaves);
		CtoPstr(tempString);
		y += 12;
		MoveTo(x,y);
		DrawString(tempString);
	}
	
	sprintf(tempString,"xmin: %lf",lims[xmin]);
	CtoPstr(tempString);
	y += 20;
	MoveTo(x,y);
	DrawString(tempString);
	
	sprintf(tempString,"xmax: %lf",lims[xmax]);
	CtoPstr(tempString);
	y += 12;
	MoveTo(x,y);
	DrawString(tempString);
	
	sprintf(tempString,"ymin: %lf",lims[ymin]);
	CtoPstr(tempString);
	y += 12;
	MoveTo(x,y);
	DrawString(tempString);
	
	sprintf(tempString,"ymax: %lf",lims[ymax]);
	CtoPstr(tempString);
	y += 12;
	MoveTo(x,y);
	DrawString(tempString);
	
	sprintf(tempString,"maximum iterations: %d",maxIterations);
	CtoPstr(tempString);
	y += 20;
	MoveTo(x,y);
	DrawString(tempString);
	
	sprintf(tempString,"pixel size: %d",pixelSize);
	CtoPstr(tempString);
	y += 12;
	MoveTo(x,y);
	DrawString(tempString);
	
	if (shading)
		sprintf(tempString,"shading: ENABLED");
	else
		sprintf(tempString,"shading: DISABLED");
	
	CtoPstr(tempString);
	y += 12;
	MoveTo(x,y);
	DrawString(tempString);
	
	if (slaveMode == FALSE)
	{
		sprintf(tempString,"polling delay: %.2lf",timeDelay);
		CtoPstr(tempString);
		y += 12;
		MoveTo(x,y);
		DrawString(tempString);
		
		if (meToo)
			sprintf(tempString,"master participation: ENABLED");
		else
			sprintf(tempString,"master participation: DISABLED");
		
		CtoPstr(tempString);
		y += 12;
		MoveTo(x,y);
		DrawString(tempString);
	}
	
	ClipRect(&nullClipRect);
	
	SetPort(oldPort);
}



ClearGraph()
{
GrafPtr		oldPort;

	GetPort(&oldPort);
	SetPort(theWPtr);
	
	PenNormal();
	
	ClipRect(&graphRect);
	EraseRect(&graphRect);
	FrameRect(&graphRect);
	
	ClipRect(&nullClipRect);
	
	SetPort(oldPort);
}




DisplayMessage(theString)
char	*theString;
{
	PtoCstr(theDisplayMessages[1]);
	PtoCstr(theDisplayMessages[2]);
	
	strcpy(theDisplayMessages[0],theDisplayMessages[1]);
	strcpy(theDisplayMessages[1],theDisplayMessages[2]);
	strcpy(theDisplayMessages[2],theString);
	
	CtoPstr(theDisplayMessages[0]);
	CtoPstr(theDisplayMessages[1]);
	CtoPstr(theDisplayMessages[2]);
	
	DrawDisplayMessages();
}



DrawDisplayMessages()
{
GrafPtr		oldPort;

	GetPort(&oldPort);
	SetPort(theWPtr);
	
	PenNormal();
	TextFont(3);
	TextFace(0);
	TextMode(srcCopy);
	TextSize(10);
	
	ClipRect(&messageRect);
	EraseRect(&messageRect);
	FrameRect(&messageRect);
	
	MoveTo(messageRect.left+5,messageRect.top + 12);
	DrawString(theDisplayMessages[0]);
	
	MoveTo(messageRect.left+5,messageRect.top + 24);
	DrawString(theDisplayMessages[1]);
	
	TextFace(bold);
	
	MoveTo(messageRect.left+5,messageRect.top + 36);
	DrawString(theDisplayMessages[2]);
	
	ClipRect(&nullClipRect);
	
	SetPort(oldPort);
}




ErrorMessage(routineName,errCode)
char	*routineName;int	errCode;
{
char	tempString[256];

	sprintf(tempString,"%s: error #%d",routineName,errCode);
	DisplayMessage(tempString);
}




ClearMessage()
{
	DisplayMessage("");
}



MaintainMenus()
{
	if (FrontWindow() != theWPtr) 
	{
		EnableItem(myMenus[editMenu],undoCommand);
		EnableItem(myMenus[editMenu],cutCommand);
		EnableItem(myMenus[editMenu],copyCommand);
		EnableItem(myMenus[editMenu],clearCommand);
		EnableItem(myMenus[editMenu],pasteCommand);
	}
	
	else 
	{
		DisableItem(myMenus[editMenu],undoCommand);
		DisableItem(myMenus[editMenu],cutCommand);
		DisableItem(myMenus[editMenu],copyCommand);
		DisableItem(myMenus[editMenu],clearCommand);
		DisableItem(myMenus[editMenu],pasteCommand);
		
		if (slaveMode)
		{
			CheckItem(myMenus[modeMenu],slaveCommand,TRUE);
			CheckItem(myMenus[modeMenu],masterCommand,FALSE);
		}
		else
		{
			CheckItem(myMenus[modeMenu],slaveCommand,FALSE);
			CheckItem(myMenus[modeMenu],masterCommand,TRUE);
			
			if (meToo)
				CheckItem(myMenus[controlMenu],meTooCommand,TRUE);
			else
				CheckItem(myMenus[controlMenu],meTooCommand,FALSE);
		}
		
		if (shading)
			CheckItem(myMenus[graphMenu],shadingCommand,TRUE);
		else
			CheckItem(myMenus[graphMenu],shadingCommand,FALSE);
	}
}



SetUpCursors()
{
CursHandle	hCurs;
	
	hCurs = GetCursor(watchCursor);
	waitCursor = **hCurs;
	hCurs = GetCursor(crossCursor);
	xCursor = **hCurs;
}



MaintainCursor()
{
Point		tempPoint;
GrafPtr		oldPort;
	
	if (FrontWindow() == theWPtr) 
	{
		GetPort(&oldPort);
		SetPort(theWPtr);
		
		GetMouse(&tempPoint);
		
		if (PtInRect(tempPoint,&graphRect))
		{
			SetCursor(&xCursor);
			
			ShowXY(tempPoint);
		}
		
		else
		{
			SetCursor(&arrow);
			
			if (coords)
			{
				InvalRect(&coordRect);
				
				UpdateWindow();
				
				coords = FALSE;
			}
		}
			
		SetPort(oldPort);
	}
}




SetGraphLimits()
{
DialogPtr	theDPtr;
int			itemHit, type, i,syntax;
Handle		theItem;
Rect		theRect;
GrafPtr		oldPort;
char		tempString[256];
double		tlims[4],delta;

	GetPort(&oldPort);
	
	theDPtr = GetNewDialog(limsDLOG, NULL,(WindowPtr) -1L);
	
	CenterWindow(theDPtr);
	ShowWindow(theDPtr);
	
	SetPort(theDPtr);
	InitCursor();
	
	GetDItem(theDPtr, 1, &type, &theItem, &theRect);
	PenNormal();
	PenSize(3,3);
	InsetRect(&theRect,-4,-4);
	FrameRoundRect(&theRect,16,16);
	PenSize(1,1);
	
	for (i=0;i<4;i++)
	{
		tlims[i] = lims[i];
		
		sprintf(tempString,"%lf",lims[i]);
		CtoPstr(tempString);
		GetDItem(theDPtr, 4+2*i, &type, &theItem, &theRect);
		SetIText(theItem,  tempString);
	}
	
	SelIText(theDPtr, 4, 0, 256);
	
	syntax = 0;
	
	do
	{
		do
		{
			ModalDialog(0L,&itemHit);
		
		} while (itemHit!=1 && itemHit!=2 && itemHit!=11);
		
		if (itemHit==1)
		{
			syntax = 1;
			
			for (i=0;i<4  &&  syntax==1; i++)
			{
				GetDItem(theDPtr, 4+2*i, &type, &theItem, &theRect);
				GetIText(theItem,  tempString);
				PtoCstr(tempString);
				syntax = sscanf(tempString,"%lf",&(tlims[i]));
				
				if (syntax != 1)
				{
					ErrorAlert("\pBad numeric input");
					SelIText(theDPtr, 4+2*i, 0, 256);
				}
			}
			
			
			if (tlims[xmin]>=tlims[xmax] || tlims[ymin]>=tlims[ymax])
			{
				ErrorAlert("\pMinimum limits must be less than maximum limits");
				syntax = 0;
			}
			
			else
			{
				delta = 0.0;
				
				for (i=0;i<4;i++)
				{
					if (lims[i] > tlims[i])
						delta += lims[i] - tlims[i];
					else
						delta += tlims[i] - lims[i];
					
					lims[i] = tlims[i];
				}
			}
		}
		
		else if (itemHit==2)
			syntax = 1;
		
		else if (itemHit==11)
		{
			tlims[xmin] = -2.0;
			tlims[xmax] = 1.0;
			tlims[ymin] = -1.5;
			tlims[ymax] = 1.5;
			
			for (i=0;i<4;i++)
			{
				sprintf(tempString,"%lf",tlims[i]);
				CtoPstr(tempString);
				GetDItem(theDPtr, 4+2*i, &type, &theItem, &theRect);
				SetIText(theItem,  tempString);
			}
			
			SelIText(theDPtr, 4, 0, 256);
		}
	
	} while (syntax != 1);
	
	
	DisposDialog(theDPtr);
	SetPort(oldPort);
	
	
	if (itemHit==1 && delta!=0.0) 
	{
		ClearGraph();
		DrawInfo();
	}
	else
		itemHit = 2;
	
	
	return(itemHit);
}


SetMaxIterations()
{
int		errCode;
char	tempString[256];

	sprintf(tempString,"%d",maxIterations);
	
	errCode = GetInput("\pEnter maximum number of iterations:", tempString);
	
	if (errCode == noErr)
	{
		if (sscanf(tempString,"%d",&maxIterations)!=1 || maxIterations<1)
		{
			ErrorAlert("\pBad numeric input for number of iterations");
		}
		else
			DrawInfo();
	}
}




SetPixelSize()
{
int		errCode;
char	tempString[256];

	sprintf(tempString,"%d",pixelSize);
	
	errCode = GetInput("\pEnter pixel size (1-254):", tempString);
	
	if (errCode == noErr)
	{
		if (sscanf(tempString,"%d",&pixelSize)!=1 || pixelSize<1 || pixelSize>254)
		{
			ErrorAlert("\ppixel size must be from 1 to 254");
		}
		else
			DrawInfo();
	}
}





SetDelay()
{
int		errCode;
char	tempString[256];

	sprintf(tempString,"%.2lf",timeDelay);
	
	errCode = GetInput("\pEnter minimum delay between node polls (seconds):", tempString);
	
	if (errCode == noErr)
	{
		if (sscanf(tempString,"%lf",&timeDelay)!=1 || timeDelay<=0)
		{
			ErrorAlert("\pdelay must be greater than zero seconds");
		}
		else
			DrawInfo();
	}
}




double XToD(x)
int x;
{
double	ratio;

	ratio = (double) (x - graphRect.left)/ (double) (graphRect.right - graphRect.left);
	ratio = ratio * (lims[xmax] - lims[xmin]) + lims[xmin];
	
	return(ratio);
}

double YToD(y)
int y;
{
double	ratio;

	ratio = (double) (y - graphRect.top)/ (double) (graphRect.bottom - graphRect.top);
	ratio = lims[ymax] - ratio * (lims[ymax] - lims[ymin]);
	
	return(ratio);
}


DToX(x)
double x;
{
double	ratio;

	ratio = (x - lims[xmin])/(lims[xmax] - lims[xmin]);
	ratio = ratio * (graphRect.right - graphRect.left) + graphRect.left;
	
	if (ratio < -32000.0)
		ratio = -32000.0;
	else if (ratio > 32000.0)
		ratio = 32000.0;
	
	return((int) ratio);
}


DToY(y)
double y;
{
double	ratio;

	ratio = (y - lims[ymin])/(lims[ymax] - lims[ymin]);
	ratio = graphRect.bottom - ratio * (graphRect.bottom - graphRect.top);
	
	if (ratio < -32000.0)
		ratio = -32000.0;
	else if (ratio > 32000.0)
		ratio = 32000.0;
	
	return((int) ratio);
}



ZoomGraph(thePoint)
Point	thePoint;
{
GrafPtr		oldPort;
Point		mousePoint,oldPoint;
Rect		theRect,tempGraphRect;
double		tlims[4],r;
int			delta;

	GetPort(&oldPort);
	SetPort(theWPtr);
	
	theRect.left  = thePoint.h;
	theRect.right = thePoint.h;
	theRect.top = thePoint.v;
	theRect.bottom = thePoint.v;
	
	GetMouse(&oldPoint);
	ShowXY(oldPoint);
	
	PenNormal();
	PenMode(patXor);
	PenPat(gray);
	FrameRect(&theRect);
	
	while (StillDown())
	{
		GetMouse(&mousePoint);
		
		if ((mousePoint.v!=oldPoint.v)||(mousePoint.h!=oldPoint.h))
		{
			FrameRect(&theRect);
			
			if (mousePoint.h < graphRect.left)
				mousePoint.h = graphRect.left;
			if (mousePoint.h > graphRect.right)
				mousePoint.h = graphRect.right;
			
			if (mousePoint.v < graphRect.top)
				mousePoint.v = graphRect.top;
			if (mousePoint.v > graphRect.bottom)
				mousePoint.v = graphRect.bottom;
			
			ShowXY(mousePoint);
			
			r = ((double) mousePoint.h - (double) thePoint.h)*((double) mousePoint.h - (double) thePoint.h) + ((double) mousePoint.v - (double) thePoint.v)*((double) mousePoint.v - (double) thePoint.v);
			delta = sqrt(r)/1.4142135624;
			
			if (mousePoint.v < thePoint.v)
			{
				theRect.top = thePoint.v - delta;
				theRect.bottom = thePoint.v;
			}
			else
			{
				theRect.bottom = thePoint.v + delta;
				theRect.top = thePoint.v;
			}
			
			if (mousePoint.h < thePoint.h)
			{
				theRect.left = thePoint.h - delta;
				theRect.right = thePoint.h;
			}
			else
			{
				theRect.right = thePoint.h + delta;
				theRect.left = thePoint.h;
			}
			
			FrameRect(&theRect);
		}
		
		oldPoint = mousePoint;
	}
	
	FrameRect(&theRect);
	
	if ((theRect.right!=theRect.left)&&(theRect.top!=theRect.bottom))
	{
		tlims[xmin] = XToD(theRect.left);
		tlims[xmax] = XToD(theRect.right);
		tlims[ymin] = YToD(theRect.bottom);
		tlims[ymax] = YToD(theRect.top);
		
		lims[xmin] = tlims[xmin];
		lims[xmax] = tlims[xmax];
		lims[ymin] = tlims[ymin];
		lims[ymax] = tlims[ymax];
		
		tempGraphRect = graphRect;
		InsetRect(&tempGraphRect,1,1);
		
		CopyBits(&(theWPtr->portBits),&graphBits,&theRect,&tempGraphRect,srcCopy,0L);
		CopyBits(&graphBits,&(theWPtr->portBits),&graphRect,&graphRect,srcCopy,0L);
	}
	
	SetPort(oldPort);
}



ShowXY(thePoint)
Point	thePoint;
{
double		x,y;
char		tempString[256];
FontInfo	theFontInfo;
GrafPtr		oldPort;
Rect		theRect;
RgnHandle	saveClip;
int			oldFont, oldMode, oldSize, oldFace;
PenState	oldPenState;

	if ((thePoint.h != oldPoint.h)||(thePoint.v != oldPoint.v))
	{
		GetPort(&oldPort);
		SetPort(theWPtr);
		saveClip = NewRgn();
		GetClip(saveClip);
		
		x = XToD(thePoint.h);
		y = YToD(thePoint.v);
		
		sprintf(tempString,"(%.5lf,%.5lf)",x,y);
		CtoPstr(tempString);
		
		oldFont = ((GrafPtr) theWPtr)->txFont;
		oldMode = ((GrafPtr) theWPtr)->txMode;
		oldSize = ((GrafPtr) theWPtr)->txSize;
		oldFace = ((GrafPtr) theWPtr)->txFace;
		
		TextFont(3);
		TextMode(srcOr);
		TextSize(9);
		TextFace(0);
		GetFontInfo(&theFontInfo);
		
		GetPenState(&oldPenState);
		
		PenNormal();
		
		FrameRect(&coordRect);
		theRect = coordRect;
		InsetRect(&theRect,1,1);
		EraseRect(&theRect);
		ClipRect(&theRect);
		
		MoveTo(coordRect.left + 3,coordRect.top + theFontInfo.ascent);
		DrawString(tempString);
		
		SetClip(saveClip);
		DisposeRgn(saveClip);
		
		SetPenState(&oldPenState);
		TextFont(oldFont);
		TextMode(oldMode);
		TextSize(oldSize);
		TextFace(oldFace);
		
		coords = TRUE;
		
		SetPort(oldPort);
	}
	
	oldPoint.h = thePoint.h;
	oldPoint.v = thePoint.v;
}




BreakKey()
{
EventRecord		theEvent;
char			theChar;
int				theResult;

	theResult = FALSE;
	
	if (GetNextEvent(keyDownMask+autoKeyMask,&theEvent))
	{
		theChar = theEvent.message & charCodeMask;
		
		if (theEvent.modifiers&cmdKey && theChar=='.')
			theResult = TRUE;
	}
		
	return(theResult);
}



DoDrag(whichWindow,theEvent)
WindowPtr	whichWindow;EventRecord	*theEvent;
{
Rect	tempRect;
	
	if (whichWindow == theWPtr)
	{
		SetRect(&tempRect,screenBits.bounds.left+10,screenBits.bounds.top+25,screenBits.bounds.right-10,screenBits.bounds.bottom-25);
		DragWindow(whichWindow,theEvent->where,&tempRect);
	}
}