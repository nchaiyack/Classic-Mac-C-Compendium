// ===========================================================================
//	main.cp
//
//	This file contains the main program for the example application to
//	demonstrate the functionality of the CModalProgress class.
//
//	Copyright 1994 Alysoft Solutions. All rights reserved.
//
// ===========================================================================

#include "CTestDialogs.h"

#define kDialogResID						6000
#define	kProgressBarItem					4
#define kProgressTextItem					5
#define kInfiniteBarItem					7
#define	kMaxNum								1000

#define	kMenubarResID						128
#define	kAppleMenu							128
#define	kFileMenu							129
#define	kEditMenu							130
#define	kDialogMenu							131

#define	kQuitItem							1

#define	kTestDialog1						1
#define	kTestDialog2						2
#define	kTestDialog3						3
#define	kTestDialog4						4

Boolean	keepGoing = true ;

void InitMacintosh(void) ;
void HandleMenuSelection(short menu, short item) ;

void SetupMenus()
{
	Handle ourMenus;

	ourMenus = GetNewMBar(kMenubarResID);
	if (ourMenus)
	{
		SetMenuBar(ourMenus);
		AddResMenu(GetMHandle(kAppleMenu),'DRVR') ;
		DrawMenuBar();
	}
	else
	{
		ExitToShell();
	}
}

void HandleMenuSelection(short menu, short item)
{
	if (menu != 0)
	{
		switch (menu)
		{
			case kFileMenu:
				if (item == kQuitItem)
					keepGoing = false ;
				break ;
			
			case kDialogMenu:
				switch (item)
				{
					case kTestDialog1:
						TestDialog1() ;
						break ;
						
					case kTestDialog2:
						TestDialog2() ;
						break ;
						
					case kTestDialog3:
						TestDialog3() ;
						break ;
						
					case kTestDialog4:
						TestDialog4() ;
						break ;
						
				}
				break ;
		}
		
	}
	HiliteMenu(0) ;
}


void InitMacintosh()
{
	MaxApplZone();
	
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
}

main()
{
	WindowPtr					theWindow ;
	EventRecord					theEvent ;
	Boolean						ok ;
	
	InitMacintosh() ;
	
	SetupMenus() ;

	while (keepGoing)
	{
		ok = GetNextEvent(everyEvent, &theEvent) ;
		if (ok)
			switch(theEvent.what)
			{
				case mouseDown:
					if (FindWindow(theEvent.where, &theWindow) == inMenuBar)
					{
						long  menuSelection ;
						menuSelection = MenuSelect(theEvent.where) ;
						HandleMenuSelection(HiWord(menuSelection), LoWord(menuSelection)) ;
					}
					break ;
				
				case keyDown:
					char	theChar = theEvent.message & charCodeMask;
					long	menuSelection = MenuKey(theChar);
					
					if (HiWord(menuSelection) != 0)
						HandleMenuSelection(HiWord(menuSelection), LoWord(menuSelection)) ;
					break ;
					
				case updateEvt:
					/* Update what ?? */
					break ;
			}
	}
	return 0 ;
}
