/*
** File:		Menu.c
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright © 1995 Nikol Software
** All rights reserved.
*/



/*****************************************************************************/



#include "Constants.h"
#include "Menu.h"
#include "StringUtils.h"
#include "MacWT.h"

#ifndef __BALLOONS__
#include <Balloons.h>
#endif

#ifndef __DESK__
#include <Desk.h>
#endif

#ifndef __ERRORS__
#include <Errors.h>
#endif

#ifndef __FONTS__
#include <Fonts.h>
#endif

#ifndef __MEMORY__
#include <Memory.h>
#endif

#ifndef __MENUS__
#include <Menus.h>
#endif

#ifndef __TOOLUTILS__
#include <ToolUtils.h>
#endif


/*****************************************************************************/



static Boolean	DoAdjustFileMenu(WindowPtr window);
static Boolean	DoAdjustEditMenu(WindowPtr window);

static void		EnableOrDisableItem(MenuHandle menu, short item, Boolean enable);

static Boolean	IsDAWindow(WindowPtr window);


/*****************************************************************************/



void	DoAdjustMenus(void)
{
	WindowPtr	window;
	Boolean		redrawMenuBar;

	window = FrontWindow();

	redrawMenuBar  = DoAdjustFileMenu(window);
	redrawMenuBar |= DoAdjustEditMenu(window);

	if (redrawMenuBar)
		DrawMenuBar();
}



/*****************************************************************************/



/* This is called when an item is chosen from the menu bar (after calling
** MenuSelect or MenuKey).  It performs the right operation for each command.
** It is good to have both the result of MenuSelect and MenuKey go to one
** routine like this to keep everything organized. */

void	DoMenuCommand(long menuResult)
{
	short			menuID, menuItem, daRefNum;
	Str255			str;


	menuID = HiWord(menuResult);	/* Use macros for efficiency to get  */
	menuItem = LoWord(menuResult);	/* menu item number and menu number. */

	switch (menuID) {

		case mApple:
			switch (menuItem) {
				case kAbout:		/* Bring up alert for About. */
					{
					char	dateTimeStr[256], compilerStr[256];
					
					#if   applec
					#define	idStr	"MPW C"
					#elif __MWERKS__
					#define	idStr	"Metrowerks"
					#elif THINK_C || THINK_CPLUS
					#define	idStr	"THINK C"
					#elif	__powerc
					#define	idStr	"PPCC"
					#endif
					
					ccpy(dateTimeStr, __DATE__);
					ccat(dateTimeStr, ", ");
					ccat(dateTimeStr, __TIME__);
					ccpy(compilerStr, "Compiler: ");
					ccat(compilerStr, idStr);
					ccat(compilerStr, 
					#if	__powerc
					" PowerPC");
					#else
					" 68K");
					#endif
					c2p(compilerStr);
					c2p(dateTimeStr);
					ParamText(gWTVersion, (StringPtr)compilerStr, (StringPtr)dateTimeStr, "\p");
					Alert(rAboutAlert, nil);
					#undef	idStr
					}
					break;
					
				default:			/* All non-About items in this menu are DAs. */
					GetMenuItemText(GetMenuHandle(mApple), menuItem, str);
					daRefNum = OpenDeskAcc(str);
					break;
				}
			break;

		case mFile:
			switch (menuItem)
				{
				case kNew:
					BeginGame();
					break;
					
				case kShowFPS:
					{
					MenuHandle		menu;

					menu = GetMenuHandle(mFile);
					gShowFPS = !gShowFPS;
					SetItemMark(menu, kShowFPS, (gShowFPS) ? checkMark : noMark);
					}
					break;

				case kUseQuickdraw:
					{
					MenuHandle		menu;

					menu = GetMenuHandle(mFile);
					gUseQuickdraw = !gUseQuickdraw;
					SetItemMark(menu, kUseQuickdraw, (gUseQuickdraw) ? checkMark : noMark);
					}
					break;
					
				case kPause:
					TogglePause();
					break;
					
				case kQuit:
					quitting = TRUE;
					break;
				}
			break;

		case mEdit:
			switch (menuItem)
				{
				case kCut:
				case kCopy:
				case kPaste:
				case kClear:
					SystemEdit(menuItem - 1);
					break;
				}
			break;
	}

	HiliteMenu(0);		/* Unhighlight what MenuSelect (or MenuKey) hilited. */
}



/*****************************************************************************/



Boolean	DoAdjustFileMenu(WindowPtr window)
{
	MenuHandle	menu;


	menu = GetMenuHandle(mFile);
	EnableItem(menu, kQuit);			/* Gotta be able to quit. */

	EnableOrDisableItem(menu, kNew, !gGameOn);
	EnableOrDisableItem(menu, kPause, gGameOn);
	EnableOrDisableItem(menu, kUseQuickdraw, !gTrueColor);

	return(false);
}



/*****************************************************************************/



Boolean	DoAdjustEditMenu(WindowPtr window)
{
	MenuHandle		menu;


	menu = GetMenuHandle(mEdit);
	
	EnableOrDisableItem(menu, kUndo, IsAppWindow(window));
	EnableOrDisableItem(menu, kCut, IsAppWindow(window));
	EnableOrDisableItem(menu, kCopy, IsAppWindow(window));
	EnableOrDisableItem(menu, kPaste, IsAppWindow(window));
	
	if (IsDAWindow(window))
		{
		EnableItem(menu, kUndo);
		EnableItem(menu, kCut);
		EnableItem(menu, kCopy);
		EnableItem(menu, kPaste);
		EnableItem(menu, kClear);
		}

	return(false);
}



/*****************************************************************************/




/* This function either enables or disables a menu item. */

static	void	EnableOrDisableItem(MenuHandle menu, short item, Boolean enable)
{
	if (enable)
		EnableItem(menu, item);
	else
		DisableItem(menu, item);
}




/*****************************************************************************/



/* Check to see if a window belongs to a desk accessory. */

static	Boolean	IsDAWindow(WindowPtr window)
{
	if (window)	/* DA windows have negative windowKinds */
		return(((WindowPeek) window)->windowKind < 0);
	else
		return(false);
}



/*****************************************************************************/



/* Check to see if a window belongs to the application.  If the window pointer
** passed was nil, then it could not be an application window.  WindowKinds
** that are negative belong to the system and windowKinds less than userKind
** are reserved by Apple except for windowKinds equal to dialogKind, which
** mean it is a dialog. For this app, we use only dialogs for our windows. */

static	Boolean	IsAppWindow(WindowPtr window)
{
	if (window)
		return(((WindowPeek)window)->windowKind == dialogKind);
	else
		return(false);
}
