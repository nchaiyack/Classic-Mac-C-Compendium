/*  Menu_ARTAbrot								 Handle a selection of the menu bar */

/* File name:  Menu_ARTAbrot.c   
 Function:  This module calls the appropiate menu list 
      handler routine. 
History: 8/18/93 Original by George Warner
   */


#include "ComUtil_ARTAbrot.h"	/* Common */

#include "AB_Alert.h"	/* Alert */
#include "About_ARTAbrot.h"	/* Modeless Dialog */
#include "Enter_Coordinates.h"	/* Modeless Dialog */
#include "ARTAbrot.h"	/* Window */
#include "BrotCode.h"
#include "Menu_ARTAbrot.h"	/* This file */


#pragma segment Menus

extern int processing_brot;

/* Routine: Init_My_Menus */
/* Purpose: Load in the menu lists and initialize the menubar */

void Init_My_Menus()
{

ClearMenuBar();									/* Clear any old menu bars */
/* This menu is the APPLE menu, used for About and desk accessories */
Menu_Apple = GetMenu(Res_Menu_Apple);				/* Get the menu from the resource file */
InsertMenu(Menu_Apple,0);							/* Insert this menu into the menu bar */

AddResMenu(Menu_Apple,'DRVR');						/* Add in DAs */

/* This menu is  File */
Menu_File = GetMenu(Res_Menu_File);				/* Get the menu from the resource file */
InsertMenu(Menu_File,0);							/* Insert this menu into the menu bar */

/* This menu is  Commands */
Menu_Commands = GetMenu(Res_Menu_Commands);		/* Get the menu from the resource file */
InsertMenu(Menu_Commands,0);						/* Insert this menu into the menu bar */


DrawMenuBar();										/* Draw the menu bar */
}

/* ======================================================= */
void DoMenuApple(short theItem);					/* Handle this menu list */
void DoMenuFile(short theItem);					/* Handle this menu list */
void DoMenuCommands(short theItem);				/* Handle this menu list */
/* ======================================================= */

/* Routine: Pre_Do_MenuListApple */
/* Purpose: Handle all menu items in this list */

void DoMenuApple(short theItem)
{
Boolean	SkipProcessing;							/* TRUE says skip processing */
short	DNA;										/* For opening DAs */
Str255	DAName;									/* For getting DA name */
GrafPtr	SavePort;								/* Save current port when opening DAs */

SkipProcessing = false;							/* Set to not skip the processing of this menu item */

if (SkipProcessing == false)						/* See if process the menu list */
	{

	switch (theItem)								/* Handle all commands in this menu list */
		{
		case MItem_About:
			Add_UserEvent(UserEvent_Open_Window,Res_MD_About_ARTAbrot,0,0,NIL);/* Open a modeless dialog */
			break;
		default:								/* allow other buttons, trap for debug */
			GetPort(&SavePort);					/* Save the current port */
			GetItem(Menu_Apple, theItem, DAName);	/* Get the name of the DA selected */
			DNA = OpenDeskAcc(DAName);				/* Open the DA selected */
			SetPort(SavePort);						/* Restore to the saved port */
			break;							/* end of otherwise */
		}										/* end of switch */

	}											/* End of IF */

}

/* ======================================================= */

/* Routine: Pre_Do_MenuListFile */
/* Purpose: Handle all menu items in this list */

void DoMenuFile(short theItem)
{
Boolean	SkipProcessing;							/* TRUE says skip processing */

SkipProcessing = false;							/* Set to not skip the processing of this menu item */

if (SkipProcessing == false)						/* See if process the menu list */
	{

	switch (theItem)								/* Handle all commands in this menu list */
		{
		case MItem_Quit2:
			if (processing_brot)
				finish_brot();
			doneFlag = true;
			break;
		default:								/* allow other buttons, trap for debug */
			break;							/* end of otherwise */
		}										/* end of switch */

	}											/* End of IF */

}

/* ======================================================= */

/* Routine: Pre_Do_MenuListCommands */
/* Purpose: Handle all menu items in this list */

void DoMenuCommands(short theItem)
{
Boolean	SkipProcessing;							/* TRUE says skip processing */
Boolean	BoolHolder;								/* For SystemEdit result */

SkipProcessing = false;							/* Set to not skip the processing of this menu item */

if (SkipProcessing == false)						/* See if process the menu list */
	{

	BoolHolder = SystemEdit(theItem - 1);			/* Let the DA do the edit to itself */

	if  (BoolHolder == false)						/* If not a DA then we get it */
		{

		switch (theItem)							/* Handle all commands in this menu list */
			{
			case MItem_Enter_Coordinat:
				Add_UserEvent(UserEvent_Open_Window,Res_MD_Enter_Coordinat,0,0,NIL);/* Open a modeless dialog */
				break;
			case MItem_Display:
				Add_UserEvent(UserEvent_Open_Window,Res_W_ARTAbrot,0,0,NIL);/* Open a Window */
				break;
			default:							/* allow other buttons, trap for debug */
				break;						/* end of otherwise */
			}									/* end of switch */
		}										/* End of IF */

	}											/* End of IF */

}

/* ======================================================= */

/* Routine: Handle_My_Menu */
/* Purpose: Vector off to the appropiate menu list handler */

void Handle_My_Menu(short theMenu,short theItem)
{

switch (theMenu)									/* Do selected menu list */
	{
	case Res_Menu_Apple:
		DoMenuApple(theItem);						/* Go handle this menu list */
		break;
	case Res_Menu_File:
		DoMenuFile(theItem);						/* Go handle this menu list */
		break;
	case Res_Menu_Commands:
		DoMenuCommands(theItem);					/* Go handle this menu list */
		break;
	default:									/* allow other buttons, trap for debug */
		break;								/* end of otherwise */
	}											/* end of switch */

HiliteMenu(0);										/* Turn menu selection off */
}

/* ======================================================= */

