/**********************************************************************
 *	This file contains the functions which start off the program.  The
 *	main event loop and Macintosh toolbox definitions are in this
 *	file.
 **********************************************************************/

#include "Creator Changer.h"
#include "Creator Changer.start.h"


/**********************************************************************
 *	Function main(), this is the main function of the program.  The 
 *	major parts of the program are called in this function.
 **********************************************************************/

void main(void)
	{
	
	Init_Toolbox();
	Get_Error_Messages();
	Check_Sys_Type();
	Install_AE_Handlers();
	Set_Up_Menu_Bar();
	Open_Preferences();
	
	UnloadSeg(Open_Preferences);
	
	while(!All_Done) Handle_One_Event();

	}
	

/**********************************************************************
 *	Function Handle_One_Event(), this function handles one event.  
 *	Depending on what the event is, it gets directed to the 
 *	appropriate place.
 **********************************************************************/

void Handle_One_Event(void) 
	{
	
	Boolean  event_was_dialog;
	
	WaitNextEvent(everyEvent, &The_Event, SLEEP_TICKS, MOUSE_REGION);
		
	switch(The_Event.what)
		{
		case mouseDown: 
			Handle_Mouse_Down();
			break;
		case keyDown:
		case autoKey:
			{
			register char command_key=The_Event.message;	
			
			if(The_Event.modifiers!=0) Handle_Menu_Choice(MenuKey(command_key));
			}
			break;
		case kHighLevelEvent:
			AEProcessAppleEvent(&The_Event);
			UnloadSeg(Open_AE);
			break;
		}
	
	}



/**********************************************************************
 *	Function Handle_Mouse_Down(), this function handles any mouse down
 *	event, such as a window select, in the menu bar etc.
 **********************************************************************/

void Handle_Mouse_Down(void)
	{
	
	WindowPtr	which_window;
	short		the_part;
	long		menu_choice;
	Rect		drag_rect;

	the_part=FindWindow(The_Event.where, &which_window);  
    
	switch(the_part)
		{
		case inMenuBar:
			menu_choice=MenuSelect(The_Event.where);
			Handle_Menu_Choice(menu_choice);              
			break;
		case inSysWindow: 
			SystemClick(&The_Event, which_window);
			break;
		case inContent:
			SelectWindow(which_window);
			break;
		default:
			break;
		}

	}



/**********************************************************************
 *	Function Handle_Menu_Choice(), this function handles the menu 
 *	choice made, then it directs it to the appropriate place.
 **********************************************************************/

void Handle_Menu_Choice(long menu_choice)
	{
	
	int the_menu;
	int the_menu_item;
	
	if(menu_choice!=0)
		{
		the_menu=HiWord(menu_choice);
		the_menu_item=LoWord(menu_choice);
		
		switch(the_menu)
			{
			case APPLE_MENU_ID:
				Handle_Apple_Choice(the_menu_item);
				break;
			case OPTIONS_MENU_ID:
				Handle_Options_Choice(the_menu_item);
				break;
			defualt:
				break;
			}
		HiliteMenu(0);
		}
	
	}



/**********************************************************************
 *	Function Handle_Apple_Choice(), this function decides what item 
 *	to execute under the apple menu.
 **********************************************************************/

void Handle_Apple_Choice(int the_item)
	{
	
	Str32	desk_acc_name;
	int		desk_acc_number;
	short	the_dialog_item;
	
	switch(the_item)
		{
		case A_ABOUT_ITEM:
			{
			DialogPtr	about_dialog;
			Boolean		done=FALSE;
			GrafPtr		old_port;
			
			GetPort(&old_port);
			about_dialog=GetNewDialog(ABOUT_DLOG_ID, About_Storage, IN_FRONT);
			ShowWindow(about_dialog);
			while(!done)
				{
				Draw_Border(about_dialog, ABOUT_OK, YES);
				ModalDialog(NIL_PTR, &the_dialog_item);
				switch(the_dialog_item)
					{
					case ABOUT_OK:
						done=TRUE;
						break;
					default:
						break;
					}
				}
			CloseDialog(about_dialog);
			SetPort(old_port);
			}
			break;
		default:
			GetItem(Apple_Menu, the_item, desk_acc_name);
			desk_acc_number=OpenDeskAcc(desk_acc_name);
			break;
		}
	
	}



/**********************************************************************
 *	Function Handle_Options_Choice(), this function decides what item
 *	to execute under the options menu.
 **********************************************************************/

void Handle_Options_Choice(int the_item)
	{
	
	switch(the_item)
		{
		case O_OPEN_ITEM:
			if(Pick_File(&The_File_Spec, &File_Info))
				{
				Multiple_Files=NO;
				Num_Of_Files=1;
				Open_Changer_DLOG(NIL_PTR);
				}
			break;
		case O_EDIT_LIST:
			Open_Edit_DLOG();
			break;
		case O_QUIT_ITEM:
			All_Done=TRUE;
			break;
		default:
			break;
		}
	
	}
