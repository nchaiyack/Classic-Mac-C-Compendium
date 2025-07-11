#include "SimpleWindowClass.h"

void	SimpleWindowClass::CreateWindow(short windowNumber, short resID)
{
	//	Allocate memory for window record on our own, then create new window
	
	theWindow=(WindowPtr) NewPtr(sizeof (WindowRecord));
	theWindow=GetNewWindow(resID, theWindow, (WindowPtr) -1L);
	if (!theWindow)
		ExitToShell();


	//	Convert the window number passed to us into a string and get its width in
	//	pixels, as well as the "This window..." width
		
	NumToString(windowNumber, sWindowNumber);
	GetIndString(sThisWindow, 128, 1);
	numWidth=StringWidth(sWindowNumber);
	titleWidth=StringWidth(sThisWindow);
	
	
	
	//	Determine height of current font, to be used when centering text later
	
	FontInfo	sysFont;
	
	GetFontInfo(&sysFont);
	fontHeight=sysFont.ascent+sysFont.descent;
	
	
	
	//	Show the window (not really necessary because resource is set to automatically show it
	
	SetPort(theWindow);
	ShowWindow(theWindow);
}

SimpleWindowClass::~SimpleWindowClass()
{
	//	Remove window from screen and QuickDraw's window list, then dispose of pointer 

	CloseWindow(theWindow);
	DisposePtr((Ptr) theWindow);
}

void	SimpleWindowClass::UpdateWindow()
{
	GrafPtr	oldPort;
	
	GetPort(&oldPort);
	SetPort(theWindow);
	
	BeginUpdate(theWindow);
	EraseRect(&theWindow->portRect);
	DrawWindowNumber();
	EndUpdate(theWindow);
	
	SetPort(oldPort);
}

void	SimpleWindowClass::ActivateWindow(Boolean activate)
{
	if (activate)
		SetPort(theWindow);
}

void	SimpleWindowClass::DrawWindowNumber()
{
	//	Calculate horizontal and vertical centers to center the strings in the window
	
	short hCenter=(theWindow->portRect.right-theWindow->portRect.left)/2;
	short vCenter=(theWindow->portRect.bottom-theWindow->portRect.top)/2;
	
	MoveTo(hCenter-(titleWidth/2), vCenter-fontHeight/2);
	DrawString(sThisWindow);
	MoveTo(hCenter-(numWidth/2), vCenter+fontHeight/2);
	DrawString(sWindowNumber);
}

void	SimpleWindowClass::Drag(Point where)
{
	RgnHandle greyRgn;
	
	greyRgn=GetGrayRgn();
	DragWindow(theWindow, where, &(**greyRgn).rgnBBox);
}