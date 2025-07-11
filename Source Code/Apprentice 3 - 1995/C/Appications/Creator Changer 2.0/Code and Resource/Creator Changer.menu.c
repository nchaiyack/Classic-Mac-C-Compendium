/**********************************************************************
 *	This file handles all of the pop-up menu options.  It updates the
 *	menu when needed, and decides what parts were clicked on.
 **********************************************************************/

#include "Creator Changer.h"
#include "Creator Changer.menu.h"


/**********************************************************************
 *	Function Handle_pop_up_menu(), this function handles the events
 *	given to the pop-up menu.
 **********************************************************************/

void Handle_Pop_Up_Menu(Handle menu_handle, DialogPtr the_dialog, int creator, int file, int description)
	{
	
	Str32	description_type;
	Str4	creator_type, file_type;
	short	item_type, the_choice;
	Handle	creator_handle, file_handle, desc_handle;
	prefs	**rsrc_handle;
	Rect	creator_rect, file_rect;
	
	the_choice=GetCtlValue((ControlHandle)menu_handle);
	if(the_choice)
		{
		UseResFile(CreatorChangerPref);
			rsrc_handle=(prefs **)NewHandle(sizeof(prefs));
			rsrc_handle=(prefs **)GetResource('CrPf', The_Type[the_choice]);
			if(description) p_Str_Copy((**rsrc_handle).Description, description_type);
			p_Str_Copy((**rsrc_handle).CreatorType, creator_type);
			p_Str_Copy((**rsrc_handle).FileType, file_type);
		UseResFile(CreatorChangerApp);
		if(description)
			{
			GetDItem(the_dialog, description, &item_type, &desc_handle, &file_rect);
			SetIText(desc_handle, description_type);
			}
		
		GetDItem(the_dialog, creator, &item_type, &creator_handle, &creator_rect);
		SetIText(creator_handle, creator_type);
	
		GetDItem(the_dialog, file, &item_type, &file_handle, &file_rect);
		SetIText(file_handle, file_type);
		ReleaseResource((Handle)rsrc_handle);
		}
	(**(ControlHandle)menu_handle).contrlValue=0;
	if(!description) Draw1Control((ControlHandle)menu_handle);
	Item_To_Edit=the_choice;
		
	}



/**********************************************************************
 *	Function Make_Pop_Up_Menu(), this function 
 **********************************************************************/

void Make_Pop_Up_Menu(void)
	{
	
	short		item;
	prefs		**rsrc_handle;
	MenuHandle	menu_handle;
	
		//	I do this to reset the menu.
	menu_handle=GetMenu(POP_UP_MENU_ID);
	DisposeMenu(menu_handle);
	
	menu_handle=GetMenu(POP_UP_MENU_ID);
	UseResFile(CreatorChangerPref);
		for(item=0;item<Num_Of_Types;item++)
			{
			The_Type[item+1]=item+Strt_Rsrc;
			rsrc_handle=(prefs **)GetResource('CrPf', item+128);
			InsMenuItem(menu_handle, (**rsrc_handle).Description, item);
			ReleaseResource((Handle)rsrc_handle);
			}
	UseResFile(CreatorChangerApp);
	
	}
