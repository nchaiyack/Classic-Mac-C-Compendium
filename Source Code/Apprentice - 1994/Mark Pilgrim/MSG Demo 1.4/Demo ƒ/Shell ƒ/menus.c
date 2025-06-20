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
#include "prefs.h"
#include "environment.h"
#include "sounds.h"
#include "error.h"
#include "file interface.h"
#include "program globals.h"
#include "demo.h"
#include "demo crash.h"
#include "file management.h"
#include "debinhex dispatch.h"

extern	long		menuDisable : 0x0b54;

Boolean			gMenuEnabled;
MenuHandle		gAppleMenu;
MenuHandle		gFileMenu;
MenuHandle		gEditMenu;
MenuHandle		gWipesMenu;
MenuHandle		gAdditionsMenu;
MenuHandle		gScrollMenu;
MenuHandle		gFluffMenu;
MenuHandle		gHelpMenu;
MenuHandle		gCrashMenu;

#define NUM_ORIGINALS	CountMItems(gWipesMenu)
#define NUM_ADDITIONS	CountMItems(gAdditionsMenu)
#define	NUM_SCROLLS		CountMItems(gScrollMenu)

enum
{
	helpMenu = 200, crashMenu,
	
	appleMenu = 400, fileMenu, editMenu, wipesMenu, additionsMenu, scrollMenu, fluffMenu,
	
	aboutItem = 1, aboutMSGItem, helpPointerItem,
	
	openItem = 1, closeItem, deBinHexItem = 4, disableQuitItem = 6, enableQuitItem,
		forceQuitItem = 9, quitItem,
	
	undoItem = 1, cutItem = 3, copyItem, pasteItem, clearItem,
	
	spiralItem = 3, caste1Item = 5, rippleItem = 8, dissolveItem = 9, caste2Item = 11,
	
	serendipityItem = 9, bulgeItem = 10, splitScrollItem = 5, scrollInItem = 6,
	
	repeatItem = 1, reverseToggle, effectsOnlyItem = 4, effectsPlusReverseEffectsItem,
		effectsPlusFadesItem, reverseEffectsPlusFadesItem, fullScreenFadesItem,
		soundToggle = 10, crashPointerItem = 12
};

/*-----------------------------------------------------------------------------------*/
/* internal stuff for menus.c                                                        */

void HandleAppleMenu(int menuItem);
void HandleFileMenu(int menuItem);
void HandleEditMenu(int menuItem);
void HandleTheWipe(int menuItem, int offset, int maxItem);
void HandleHelpMenu(int menuItem);
void HandleFluffMenu(int menuItem);
void HandleCrashMenu(int menuItem);


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
	if ((gWipesMenu=GetMHandle(wipesMenu))==0L)
		return FALSE;
	if ((gAdditionsMenu=GetMHandle(additionsMenu))==0L)
		return FALSE;
	if ((gScrollMenu=GetMHandle(scrollMenu))==0L)
		return FALSE;
	if ((gFluffMenu=GetMHandle(fluffMenu))==0L)
		return FALSE;
	
	if ((gHelpMenu=GetMenu(helpMenu))==0L)
		return FALSE;
	if ((gCrashMenu=GetMenu(crashMenu))==0L)
		return FALSE;
	InsertMenu(gHelpMenu, -1);
	InsertMenu(gCrashMenu, -1);
	
	AddResMenu(gAppleMenu, 'DRVR');				/* adds control panels to apple menu */
	
	AdjustMenus();								/* dim/enable/check/mark menus/items */
	DrawMenuBar();								/* draws the actual menu bar */
	
	return TRUE;
}

void AdjustMenus(void)
{
	WindowPeek	theWindow;
	int			kind;
	
	if (gInProgress)
	{
		DisableItem(gAppleMenu, aboutItem);
		DisableItem(gAppleMenu, aboutMSGItem);
		DisableItem(gAppleMenu, helpPointerItem);
		if (gSystemSevenOrLater)
		{
			DisableItem(gFileMenu, openItem);
			DisableItem(gFileMenu, closeItem);
			DisableItem(gFileMenu, deBinHexItem);
			DisableItem(gFileMenu, disableQuitItem);
			DisableItem(gFileMenu, enableQuitItem);
			DisableItem(gFileMenu, quitItem);
		}
		else DisableItem(gFileMenu, 0);
		
		DisableItem(gEditMenu, 0);
		DisableItem(gWipesMenu, 0);
		DisableItem(gAdditionsMenu, 0);
		DisableItem(gScrollMenu, 0);
		DisableItem(gFluffMenu, 0);
	}
	else
	{
		EnableItem(gAppleMenu, aboutItem);
		EnableItem(gAppleMenu, aboutMSGItem);
		EnableItem(gAppleMenu, helpPointerItem);
		EnableItem(gFileMenu, deBinHexItem);
		EnableItem(gWipesMenu, 0);
		EnableItem(gAdditionsMenu, 0);
		EnableItem(gScrollMenu, 0);
		EnableItem(gFluffMenu, 0);
		
		theWindow = (WindowPeek)FrontWindow();
		kind = theWindow ? theWindow->windowKind : 0;
		
		if(kind < 0)
			EnableItem(gEditMenu, 0);
		else
			DisableItem(gEditMenu, 0);
		
		if (theWindow)
			EnableItem(gFileMenu, closeItem);
		else
			DisableItem(gFileMenu, closeItem);
		
		if(gTheWindow[kMainWindow])
			DisableItem(gFileMenu, openItem);
		else
			EnableItem(gFileMenu, openItem);
		
		if (gDisableQuit)
		{
			DisableItem(gFileMenu, quitItem);
			DisableItem(gFileMenu, disableQuitItem);
			EnableItem(gFileMenu, enableQuitItem);
		}
		else
		{
			EnableItem(gFileMenu, quitItem);
			EnableItem(gFileMenu, disableQuitItem);
			DisableItem(gFileMenu, enableQuitItem);
		}
		if (gLastWipe==-1)
			DisableItem(gFluffMenu, repeatItem);
		else
			EnableItem(gFluffMenu, repeatItem);
		
		if (gWipeStatus==kFullScreen)
		{
			DisableItem(gWipesMenu, caste1Item);
			DisableItem(gWipesMenu, rippleItem);
			DisableItem(gWipesMenu, dissolveItem);
			DisableItem(gWipesMenu, caste2Item);
		}
		else
		{
			EnableItem(gWipesMenu, caste1Item);
			EnableItem(gWipesMenu, rippleItem);
			EnableItem(gWipesMenu, dissolveItem);
			EnableItem(gWipesMenu, caste2Item);
		}
		
		if (gIsReversed)
		{
			DisableItem(gWipesMenu, spiralItem);
			DisableItem(gAdditionsMenu, serendipityItem);
			DisableItem(gAdditionsMenu, bulgeItem);
			DisableItem(gScrollMenu, scrollInItem);
			DisableItem(gScrollMenu, splitScrollItem);
		}
		else
		{
			EnableItem(gWipesMenu, spiralItem);
			EnableItem(gAdditionsMenu, serendipityItem);
			EnableItem(gAdditionsMenu, bulgeItem);
			EnableItem(gScrollMenu, scrollInItem);
			EnableItem(gScrollMenu, splitScrollItem);
		}
		if (gSoundAvailable)
			EnableItem(gFluffMenu, soundToggle);
		else
			DisableItem(gFluffMenu, soundToggle);
	}
	
	CheckItem(gFluffMenu, effectsOnlyItem, (gWipeStatus==kEffectsOnly));
	CheckItem(gFluffMenu, effectsPlusReverseEffectsItem, (gWipeStatus==kEffectsPlusReverseEffects));
	CheckItem(gFluffMenu, effectsPlusFadesItem, (gWipeStatus==kFadesPlusEffects));
	CheckItem(gFluffMenu, reverseEffectsPlusFadesItem, (gWipeStatus==kFadesPlusReverseEffects));
	CheckItem(gFluffMenu, fullScreenFadesItem, (gWipeStatus==kFullScreen));
	CheckItem(gFluffMenu, soundToggle, gSoundToggle&&gSoundAvailable);
}

void HandleMenu(long mSelect)
{
	int			menuID = HiWord(mSelect);
	int			menuItem = LoWord(mSelect);
	
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
		case wipesMenu:
			HandleTheWipe(menuItem, 0, NUM_ORIGINALS);
			break;
		case additionsMenu:
			HandleTheWipe(menuItem, NUM_ORIGINALS, NUM_ADDITIONS);
			break;
		case scrollMenu:
			HandleTheWipe(menuItem, NUM_ORIGINALS+NUM_ADDITIONS, NUM_SCROLLS);
			break;
		case helpMenu:
			HandleHelpMenu(menuItem);
			break;
		case fluffMenu:
			HandleFluffMenu(menuItem);
			break;
		case crashMenu:
			HandleCrashMenu(menuItem);
			break;
	}
}

void DoTheCloseThing(WindowPeek theWindow)
/* a standard close procedure, called when "close" is chosen from File menu and when
   a window is closed through its close box */
{
	Boolean			gotone;
	int				i;
	int				kind;
	
	kind = theWindow ? theWindow->windowKind : 0;
	if (kind<0)		/* DA window or other system window */
		CloseDeskAcc(kind);
	else
	{
		gotone=FALSE;
		/* see if it's one of ours */
		for (i=0; (i<NUM_WINDOWS) && (!gotone); i++)
			gotone=(theWindow==gTheWindow[i]);
		
		if (gotone)		/* if it's one of ours...  see graphics.c */
			CloseTheWindow(gTheWindowData[i-1]);	/* this may return FALSE = not closed */
		else
			DisposeWindow(theWindow);		/* not one of ours, so just close it */
	
		AdjustMenus();	/* may affect which menu items or menus are available, etc */
	}
}

void HandleAppleMenu(int menuItem)
{
	GrafPtr		savePort;
	Str255		name;
	
	switch (menuItem)
	{
		case aboutItem:
			if (gMenuEnabled)
				OpenTheWindow(kAbout);
			else DoSound(sound_fluff, TRUE);
			break;
		case aboutMSGItem:
			if (gMenuEnabled)
				OpenTheWindow(kAboutMSG);
			else DoSound(sound_fluff, TRUE);
			break;
		case helpPointerItem:
			if (gMenuEnabled)
				HandleHelpMenu(1);
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

void HandleFileMenu(int menuItem)
{
	WindowPtr			theWindow;
	int					i;
	Boolean				gotone;
	
	switch (menuItem)
	{
		case openItem:
			if (gMenuEnabled)
				OpenTheWindow(kMainWindow);
			else
				DoSound(sound_fluff, TRUE);
			break;
		case closeItem:
			if (FrontWindow()!=0L)
				DoTheCloseThing(FrontWindow());
			else
				DoSound(sound_fluff, TRUE);
			break;
		case deBinHexItem:
			if (gMenuEnabled)
			{
				if (GetSourceFile(&inputFS))
					HandleError(DeBinHexDispatch(), FALSE);
			}
			else DoSound(sound_fluff, TRUE);
			break;
		case disableQuitItem:
			if (gMenuEnabled)
			{
				gDisableQuit=TRUE;
				AdjustMenus();
			}
			else DoSound(sound_fluff, TRUE);
			break;
		case enableQuitItem:
			if (gMenuEnabled)
			{
				gDisableQuit=FALSE;
				AdjustMenus();
			}
			else DoSound(sound_fluff, TRUE);
			break;
		case forceQuitItem:
			if (gMenuEnabled)
				SysError(0x4e22);
			else DoSound(sound_fluff, TRUE);
			break;
		case quitItem:
			if (!gInProgress)
				gDone=TRUE;
			else DoSound(sound_fluff, TRUE);
			break;
	}
}

void HandleEditMenu(int menuItem)
{
	if ((menuItem>0) && (menuItem!=2))
	{
		if (gMenuEnabled)
			SystemEdit(menuItem - 1);
		else DoSound(sound_fluff, TRUE);
	}
}

void HandleTheWipe(int menuItem, int offset, int maxItem)
{
	if ((menuItem>0) && (menuItem<=maxItem))
	{
		if (gMenuEnabled)
		{
			if (gWipeStatus==kFullScreen)
				DoFullScreenFade(menuItem+offset);
			else
			{
				OpenTheWindow(kMainWindow);
				gWhichWipe=menuItem+offset;
				OpenTheWindow(kMainWindow);
				if (gWipeStatus==kEffectsPlusReverseEffects)
				{
					gIsReversed=!gIsReversed;
					gWhichWipe=menuItem+offset;
					OpenTheWindow(kMainWindow);
					gIsReversed=!gIsReversed;
				}
			}
		}
		else DoSound(sound_fluff, TRUE);
	}
}

void HandleHelpMenu(int menuItem)
{
	gLastHelp=gWhichHelp;
	gWhichHelp=menuItem;
	OpenTheWindow(kHelp);
}

void HandleFluffMenu(int menuItem)
{
	switch (menuItem)
	{
		case repeatItem:
			if (gMenuEnabled)
				HandleTheWipe(gLastWipe, 0, 999);
			else DoSound(sound_fluff, TRUE);
			break;
		case soundToggle:
			if (gMenuEnabled)
			{
				gSoundToggle=!gSoundToggle;
				DoSound(sound_on, TRUE);
			}
			else DoSound(sound_fluff, TRUE);
			break;
		case effectsOnlyItem:
			if (gMenuEnabled)
				gWipeStatus=kEffectsOnly;
			else DoSound(sound_fluff, TRUE);
			break;
		case effectsPlusReverseEffectsItem:
			if (gMenuEnabled)
				gWipeStatus=kEffectsPlusReverseEffects;
			else DoSound(sound_fluff, TRUE);
			break;
		case effectsPlusFadesItem:
			if (gMenuEnabled)
				gWipeStatus=kFadesPlusEffects;
			else DoSound(sound_fluff, TRUE);
			break;
		case reverseEffectsPlusFadesItem:
			if (gMenuEnabled)
				gWipeStatus=kFadesPlusReverseEffects;
			else DoSound(sound_fluff, TRUE);
			break;
		case fullScreenFadesItem:
			if (gMenuEnabled)
				gWipeStatus=kFullScreen;
			else DoSound(sound_fluff, TRUE);
			break;
		case reverseToggle:
			if (gMenuEnabled)
			{
				gIsReversed=!gIsReversed;
				ReverseAllWipes();
			}
			else DoSound(sound_fluff, TRUE);
			break;
		case crashPointerItem:
			if (!gMenuEnabled)
				DoSound(sound_fluff, TRUE);
			break;
	}
}

void HandleCrashMenu(int menuItem)
{
	if (menuItem>0)
		CrashAndBurn(menuItem);
}

void ReverseAllWipes(void)
{
	int				whichMenu, whichItem, i;
	MenuHandle		thisMenu;
	Str255			thisName;
	char			thisChar;
	
	for (whichMenu=0; whichMenu<3; whichMenu++)
	{
		switch (whichMenu)
		{
			case 0:	thisMenu=gWipesMenu;		break;
			case 1:	thisMenu=gAdditionsMenu;	break;
			case 2:	thisMenu=gScrollMenu;		break;
		}
		
		for (whichItem=CountMItems(thisMenu); whichItem>0; whichItem--)
		{
			GetItem(thisMenu, whichItem, thisName);
			for (i=thisName[0]; i>thisName[0]/2; i--)
			{
				thisChar=thisName[i];
				thisName[i]=thisName[thisName[0]-i+1];
				thisName[thisName[0]-i+1]=thisChar;
			}
			SetItem(thisMenu, whichItem, thisName);
		}
	}
}
