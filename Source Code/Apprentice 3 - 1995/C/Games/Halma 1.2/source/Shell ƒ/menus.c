/**********************************************************************\

File:		menus.c

Purpose:	This module handles menu selections.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "graphics.h"
#include "menus.h"
#include "help.h"
#include "environment.h"
#include "sounds.h"
#include "error.h"
#include "launch.h"
#include "file interface.h"
#include "halma load-save.h"
#include "halma.h"
#include "debinhex dispatch.h"
#include "file management.h"
#include "program globals.h"
#include "halma snow.h"

extern	long		menuDisable : 0x0b54;

Boolean			gMenuEnabled;
MenuHandle		gAppleMenu;
MenuHandle		gFileMenu;
MenuHandle		gEditMenu;
MenuHandle		gOptionsMenu;
MenuHandle		gSnowMenu;

enum
{
	appleMenu = 400, fileMenu, editMenu, optionsMenu, snowMenuColor, snowMenuBW,
	
	aboutItem = 1, aboutMSGItem, helpPointerItem,
	
	newItem = 1, openItem, closeItem, saveItem, saveAsItem, deBinHexItem = 7,
		launchItem=9, forceQuitItem = 11, quitItem,
	
	undoItem = 1, cutItem = 3, copyItem, pasteItem, clearItem, selectAllItem,
	
	boardItem = 1, soundToggle = 3, hideItem = 5,
	
	slowItem=1, mediumItem, fastItem, unused1, redItem, greenItem, blueItem, cyanItem,
		magentaItem, yellowItem, whiteItem, unused2, mutateItem
};

/*-----------------------------------------------------------------------------------*/
/* internal stuff for menus.c                                                        */

void HandleAppleMenu(short menuItem);
void HandleFileMenu(short menuItem);
void HandleEditMenu(short menuItem);
void HandleHelpMenu(void);
void HandleOptionsMenu(short menuItem);
void HandleSnowMenu(short menuItem);

Boolean InitTheMenus(void)
{
	Handle		MBARHandle;
	
	if ((MBARHandle=GetNewMBar(400))==0L)		/* sez which menus are in menu bar. */
		return FALSE;
	SetMenuBar(MBARHandle);						/* set this to be THE menu bar to use. */
	
	if ((gAppleMenu=GetMHandle(appleMenu))==0L)	/* GetNewMBar also got menu handles of */
		return FALSE;
	if ((gFileMenu=GetMHandle(fileMenu))==0L)	/* every menu it includes, so just */
		return FALSE;
	if ((gEditMenu=GetMHandle(editMenu))==0L)	/* grab these handles and assign them */
		return FALSE;
	if ((gOptionsMenu=GetMHandle(optionsMenu))==0L)
		return FALSE;
	if (gHasColorQD)
	{
		if ((gSnowMenu=GetMenu(snowMenuColor))==0L)
			return FALSE;
	}
	else
	{
		if ((gSnowMenu=GetMenu(snowMenuBW))==0L)
			return FALSE;
	}
	
	InsertMenu(gSnowMenu, 0);
	
	if (gSystemSevenOrLater)
	{
		AppendMenu(gOptionsMenu, "\p-");
		AppendMenu(gOptionsMenu, "\pHide/1");
	}
	
	AddResMenu(gAppleMenu, 'DRVR');				/* adds control panels to apple menu */
	
	AdjustMenus();								/* dim/enable/check/mark menus/items */
	DrawMenuBar();								/* draws the actual menu bar */
	
	return TRUE;
}

void AdjustMenus(void)
{
	WindowPeek	theWindow;
	short			kind;
	
	if (gInProgress)
	{
		DisableItem(gAppleMenu, aboutItem);
		DisableItem(gAppleMenu, aboutMSGItem);
		DisableItem(gAppleMenu, helpPointerItem);
		if (gSystemSevenOrLater)
		{
			DisableItem(gFileMenu, newItem);
			DisableItem(gFileMenu, openItem);
			DisableItem(gFileMenu, closeItem);
			DisableItem(gFileMenu, deBinHexItem);
			DisableItem(gFileMenu, launchItem);
			DisableItem(gFileMenu, quitItem);
		}
		else DisableItem(gFileMenu, 0);
		
		DisableItem(gEditMenu, 0);
		DisableItem(gOptionsMenu, 0);
		DisableItem(gSnowMenu, 0);
	}
	else
	{
		EnableItem(gAppleMenu, aboutItem);
		EnableItem(gAppleMenu, aboutMSGItem);
		EnableItem(gAppleMenu, helpPointerItem);
		if (gSystemSevenOrLater)
		{
			EnableItem(gFileMenu, newItem);
			EnableItem(gFileMenu, openItem);
			EnableItem(gFileMenu, closeItem);
			EnableItem(gFileMenu, deBinHexItem);
			EnableItem(gFileMenu, launchItem);
			EnableItem(gFileMenu, quitItem);
		}
		else EnableItem(gFileMenu, 0);
		EnableItem(gEditMenu, 0);
		EnableItem(gOptionsMenu, 0);
		EnableItem(gSnowMenu, 0);
		
		theWindow = (WindowPeek)FrontWindow();
		kind = theWindow ? theWindow->windowKind : 0;
		
		if (kind < 0)
		{
			EnableItem(gEditMenu, undoItem);
			EnableItem(gEditMenu, cutItem);
			EnableItem(gEditMenu, copyItem);
			EnableItem(gEditMenu, pasteItem);
			EnableItem(gEditMenu, clearItem);
		}
		else
		{
			if ((theWindow!=0L) && ((WindowPtr)theWindow==GetIndWindowGrafPtr(kMainWindow)) &&
				((gNumMoves>0) || (gThisJumpString[0]>0x00)))
				EnableItem(gEditMenu, undoItem);
			else
				DisableItem(gEditMenu, undoItem);
			DisableItem(gEditMenu, cutItem);
			DisableItem(gEditMenu, copyItem);
			DisableItem(gEditMenu, pasteItem);
			DisableItem(gEditMenu, clearItem);
		}
		
		if(theWindow)
			EnableItem(gFileMenu, closeItem);
		else
			DisableItem(gFileMenu, closeItem);
		
		if (GetIndWindowGrafPtr(kMainWindow))
		{
			DisableItem(gFileMenu, newItem);
			DisableItem(gFileMenu, openItem);
			DisableItem(gOptionsMenu, boardItem);
			EnableItem(gFileMenu, saveItem);
			EnableItem(gFileMenu, saveAsItem);
		}
		else
		{
			EnableItem(gFileMenu, newItem);
			EnableItem(gFileMenu, openItem);
			EnableItem(gOptionsMenu, boardItem);
			DisableItem(gFileMenu, saveItem);
			DisableItem(gFileMenu, saveAsItem);
		}
		
		if (gSystemSevenOrLater)
			EnableItem(gFileMenu, forceQuitItem);
		else
			DisableItem(gFileMenu, forceQuitItem);
		
		if (gSoundAvailable)
			EnableItem(gOptionsMenu, soundToggle);
		else
			DisableItem(gOptionsMenu, soundToggle);
	}
	CheckItem(gOptionsMenu, soundToggle, gSoundToggle&&gSoundAvailable);
	CheckItem(gSnowMenu, slowItem, gSnowSpeed==0x01);
	CheckItem(gSnowMenu, mediumItem, gSnowSpeed==0x02);
	CheckItem(gSnowMenu, fastItem, gSnowSpeed==0x03);
	CheckItem(gSnowMenu, redItem, gSnowColor==0x01);
	CheckItem(gSnowMenu, greenItem, gSnowColor==0x02);
	CheckItem(gSnowMenu, blueItem, gSnowColor==0x03);
	CheckItem(gSnowMenu, cyanItem, gSnowColor==0x04);
	CheckItem(gSnowMenu, magentaItem, gSnowColor==0x05);
	CheckItem(gSnowMenu, yellowItem, gSnowColor==0x06);
	CheckItem(gSnowMenu, whiteItem, gSnowColor==0x07);
	CheckItem(gSnowMenu, mutateItem, gSnowMutates);
}

void HandleMenu(long mSelect)
{
	short			menuID = HiWord(mSelect);
	short			menuItem = LoWord(mSelect);
	
	if (menuID==0)
	{
		menuID=HiWord(menuDisable);
		menuItem=LoWord(menuDisable);
		gMenuEnabled=FALSE;
	}
	else gMenuEnabled=TRUE;
	menuDisable=0L;

	switch (menuID)
	{
		case appleMenu:
			HandleAppleMenu(menuItem);
			break;
		case fileMenu:
			HandleFileMenu(menuItem);
			break;	
		case editMenu:
			HandleEditMenu(menuItem);
			break;
		case optionsMenu:
			HandleOptionsMenu(menuItem);
			break;
		case snowMenuColor:
		case snowMenuBW:
			HandleSnowMenu(menuItem);
			break;
	}
}

void DoTheCloseThing(WindowPeek theWindow)
/* a standard close procedure, called when "close" is chosen from File menu and when
   a window is closed through its close box */
{
	Boolean			gotone;
	short			i;
	short			kind;
	
	if (theWindow==0L)
		return;
	
	kind = theWindow ? theWindow->windowKind : 0;
	if (kind<0)		/* DA window or other system window */
		CloseDeskAcc(kind);
	else
	{
		gotone=FALSE;
		/* see if it's one of ours */
		for (i=0; (i<NUM_WINDOWS) && (!gotone); i++)
			gotone=((WindowPtr)theWindow==GetIndWindowGrafPtr(i));
		
		if (gotone)		/* if it's one of ours...  see graphics.c */
			CloseTheIndWindow(i-1);			/* this may return FALSE = not closed */
		else
			DisposeWindow((WindowPtr)theWindow);	/* not one of ours, so just close it */
	
		AdjustMenus();	/* may affect which menu items or menus are available, etc */
	}
}

void HandleAppleMenu(short menuItem)
{
	GrafPtr		savePort;
	Str255		name;
	
	switch (menuItem)
	{
		case aboutItem:
			if (gMenuEnabled)
				OpenTheIndWindow(kAbout);
			else DoSound(sound_fluff, TRUE);
			break;
		case aboutMSGItem:
			if (gMenuEnabled)
				OpenTheIndWindow(kAboutMSG);
			else DoSound(sound_fluff, TRUE);
			break;
		case helpPointerItem:
			if (gMenuEnabled)
				HandleHelpMenu();
			else DoSound(sound_fluff, TRUE);
			break;
		default:
			if (menuItem > helpPointerItem+1)
			{
				GetPort(&savePort);
				GetItem(gAppleMenu, menuItem, name);
				OpenDeskAcc(name);
				SetPort(savePort);
			}
			break;
	}
}

void HandleFileMenu(short menuItem)
{
	WindowPtr			theWindow;
	short				i;
	Boolean				gotone;
	
	switch (menuItem)
	{
		case newItem:
			if (gMenuEnabled)
				NewGame();
			else DoSound(sound_fluff, TRUE);
			break;
		case openItem:
			if (gMenuEnabled)
				LoadSaveDispatch(TRUE, FALSE);
			else DoSound(sound_fluff, TRUE);
			break;
		case closeItem:
			if (gMenuEnabled)
				DoTheCloseThing((WindowPeek)FrontWindow());
			else DoSound(sound_fluff, TRUE);
			break;
		case saveItem:
			if (gMenuEnabled)
				LoadSaveDispatch(FALSE, TRUE);
			else DoSound(sound_fluff, TRUE);
			break;
		case saveAsItem:
			if (gMenuEnabled)
				LoadSaveDispatch(FALSE, FALSE);
			else DoSound(sound_fluff, TRUE);
			break;
		case deBinHexItem:
			if (gMenuEnabled)
			{
				if (GetSourceFile(&inputFS, TRUE, FALSE))
					HandleError(DeBinHexDispatch(), FALSE);
			}
			else DoSound(sound_fluff, TRUE);
			break;
		case launchItem:
			if (gMenuEnabled)
			{
				LaunchDispatch();
			}
			else DoSound(sound_fluff, TRUE);
			break;
		case forceQuitItem:
			if (gMenuEnabled)
				SysError(0x4e22);
			else DoSound(sound_fluff, TRUE);
			break;
		case quitItem:
			if (gMenuEnabled)
				gDone = TRUE;
			else DoSound(sound_fluff, TRUE);
			break;
	}
}

void HandleEditMenu(short menuItem)
{
	if ((gMenuEnabled) && (menuItem!=2))
	{
		if (gFrontWindowIsOurs)
		{
			switch (menuItem)
			{
				case undoItem:		CallIndDispatchProc(gFrontWindowIndex, kUndo, 0L);	break;
				case cutItem:		CallIndDispatchProc(gFrontWindowIndex, kCut, 0L);	break;
				case copyItem:		CallIndDispatchProc(gFrontWindowIndex, kCopy, 0L);	break;
				case pasteItem:		CallIndDispatchProc(gFrontWindowIndex, kPaste, 0L);	break;
				case clearItem:		CallIndDispatchProc(gFrontWindowIndex, kClear, 0L);	break;
				case selectAllItem:	CallIndDispatchProc(gFrontWindowIndex, kSelectAll, 0L);	break;
			}
		}
		else SystemEdit(menuItem-1);
	}
	else if (menuItem!=2)
		DoSound(sound_fluff, TRUE);
}

void HandleOptionsMenu(short menuItem)
{
	switch (menuItem)
	{
		case boardItem:
			if (gMenuEnabled)
				OpenTheIndWindow(kBoardSize);
			else DoSound(sound_fluff, TRUE);
			break;
		case soundToggle:
			if (gMenuEnabled)
			{
				gSoundToggle=!gSoundToggle;
				DoSound(sound_on, TRUE);
			}
			break;
		case hideItem:
			if (gMenuEnabled)
			{
				MenuKey(0);
			}
			else DoSound(sound_fluff, TRUE);
			break;
	}
}

void HandleSnowMenu(short menuItem)
{
	switch (menuItem)
	{
		case slowItem:		gSnowSpeed=0x01;	GetRidOfSnowflake((WindowDataHandle)GetIndWindowDataHandle(kMainWindow));	break;
		case mediumItem:	gSnowSpeed=0x02;	GetRidOfSnowflake((WindowDataHandle)GetIndWindowDataHandle(kMainWindow));	break;
		case fastItem:		gSnowSpeed=0x03;	GetRidOfSnowflake((WindowDataHandle)GetIndWindowDataHandle(kMainWindow));	break;
		case redItem:		gSnowColor=0x01;	break;
		case greenItem:		gSnowColor=0x02;	break;
		case blueItem:		gSnowColor=0x03;	break;
		case cyanItem:		gSnowColor=0x04;	break;
		case magentaItem:	gSnowColor=0x05;	break;
		case yellowItem:	gSnowColor=0x06;	break;
		case whiteItem:		gSnowColor=0x07;	break;
		case mutateItem:	gSnowMutates^=0xFF;	break;
	}
}

void HandleHelpMenu(void)
{
	OpenTheIndWindow(kHelp);
}
