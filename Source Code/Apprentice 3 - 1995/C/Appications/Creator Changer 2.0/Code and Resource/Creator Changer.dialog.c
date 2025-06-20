/**********************************************************************
 *	This file contains the bulk of the program.  All of the dialog box
 *	stuff is performed in this file.
 **********************************************************************/

#include "Creator Changer.h"
#include "Creator Changer.dialog.h"


/**********************************************************************
 *	Function Open_Changer_DLOG(), this function gets the info from the
 *	Chng_Dialog box.
 **********************************************************************/

void Open_Changer_DLOG(AEDescList *the_files)
	{
	
	short			item_hit, the_item;
	Handle			menu_handle;
	Rect			the_rect;
	Boolean			done_with_dialog;
	GrafPtr			old_port;
	File_Union		temp;
	
	GetPort(&old_port);
	done_with_dialog=FALSE;
	Chng_Dialog=GetNewDialog(CHNG_TYPE_DLOG_ID, Chng_Storage, IN_FRONT);
	
	Make_Pop_Up_Menu();
	GetDItem(Chng_Dialog, POP_UP_MENU, &the_item, &menu_handle, &the_rect);
	(**(ControlHandle)menu_handle).contrlValue=0;
	Draw1Control((ControlHandle)menu_handle);
	
	ShowWindow(Chng_Dialog);
	if(!Multiple_Files)
		{
		Set_Type_Text(&F_Type, File_Info.fdType);
		Set_Type_Text(&C_Type, File_Info.fdCreator);
		}
	
	while(!done_with_dialog)
		{
		ModalDialog(My_Dialog_Filter, &item_hit);
		switch(item_hit)
			{
			case CHNG_OK:
				done_with_dialog=Handle_OK_Change(Chng_Dialog, CHNG_CREATOR, CHNG_FILE, the_files);
				break;
			case POP_UP_MENU:
				Handle_Pop_Up_Menu(menu_handle, Chng_Dialog, CHNG_CREATOR, CHNG_FILE, NO);
				break;
			case CHNG_CANCEL:
				done_with_dialog=TRUE;
				break;
			case CHNG_MKLK:
				Handle_Make_Like(Chng_Dialog, CHNG_FILE, CHNG_CREATOR);
				break;
			case CHNG_CRNT_CREATOR:
				if(!Multiple_Files) Set_DLOG_Text_Item(Chng_Dialog, CHNG_CREATOR, C_Type.TEXT);
				break;
			case CHNG_CRNT_TYPE:
				if(!Multiple_Files) Set_DLOG_Text_Item(Chng_Dialog, CHNG_FILE, F_Type.TEXT);
				break;
			defualt:
				break;
			}
		}
	
	CloseDialog(Chng_Dialog);
	SetPort(old_port);
	
	}



/**********************************************************************
 *	Function Handle_OK_Change(), this function calls the functions
 *	which do the changing of the creator and file types.  However, 
 *	if there is no information entered in the text fields the function
 *	will alert the user.
 **********************************************************************/
 
Boolean Handle_OK_Change(DialogPtr the_dialog, int creator, int file, AEDescList *the_files)
 	{
 	
 	int 		temp, index;
 	AEKeyword	key_word;
	DescType	the_type;
	Size		the_size;
	OSType		the_creator, the_file;
 	
 	temp=Check_Data(the_dialog, file, NO)+Check_Data(the_dialog, creator, NO);
 	
	if(temp<TYPE_LEN)
		{
		the_file=Set_Long_Type(the_dialog, file);
		the_creator=Set_Long_Type(the_dialog, creator);
		for(index=1;index<=Num_Of_Files;index++)
			{
			if(Multiple_Files)
				{
				AEGetNthPtr(the_files, index, typeFSS, &key_word, &the_type, &The_File_Spec, sizeof(The_File_Spec), &the_size);
				FSpGetFInfo(&The_File_Spec, &File_Info);
				}
			File_Info.fdType=the_file;
			File_Info.fdCreator=the_creator;
			FSpSetFInfo(&The_File_Spec, &File_Info);
			}
		Force_Finder_Update(&The_File_Spec);
		if(Multiple_Files) AEDisposeDesc(the_files);
		return(GOOD);
		}
	else
		{
		ParamText(Types_Error_1, "\p", Types_Error_2, "\p");
		Alert(ERROR_ALERT_ID, NIL_PTR);
		return(BAD);
		}
	}



/**********************************************************************
 *	Function Set_Long_Type(), this function sets the file type of the
 *	specified file.  (converts a string in a edit field to an 
 *	unsigned decimal equivalent).
 **********************************************************************/

OSType Set_Long_Type(DialogPtr the_dialog, int ID)
	{
	
	File_Union	file_type, temp;
	Handle		item_handle;
	short		item_type;
	Rect		item_rect;
	int			i;
	
	GetDItem(the_dialog, ID, &item_type, &item_handle, &item_rect);
	GetIText(item_handle, temp.TEXT);
	
	for(i=0;i<=TYPE_LEN;i++) file_type.TEXT[i]=temp.TEXT[i+1];
	return(file_type.LONG);
	
	}



/**********************************************************************
 *	Function Handle_Make_Like(), this function picks a file to use as
 *	a template.
 **********************************************************************/

void Handle_Make_Like(DialogPtr the_dialog, short file, short creator)
	{
	
	File_Union	temp, f_type, c_type;
	FInfo		ml_file_info;
	FSSpec		ml_file_spec;
	
	if(Pick_File(&ml_file_spec, &ml_file_info))
		{
		Set_Type_Text(&f_type, ml_file_info.fdType);
		Set_Type_Text(&c_type, ml_file_info.fdCreator);
		
		Set_DLOG_Text_Item(the_dialog, file,    f_type.TEXT);
		Set_DLOG_Text_Item(the_dialog, creator, c_type.TEXT);
		
		if(the_dialog==Edit_Dialog) Item_To_Edit=0;
		}
	
	}



/**********************************************************************
 *	Function Set_Type_Text(), this function sets the text for the file
 *	and creator types.
 **********************************************************************/

void Set_Type_Text(File_Union *the_type, OSType the_long_type)
	{
	
	File_Union	temp;
	int			i;
	
	(*the_type).TEXT[0]=TYPE_LEN;
	temp.LONG=the_long_type;
	for(i=0;i<=TYPE_LEN;i++) (*the_type).TEXT[i+1]=temp.TEXT[i];
	
	}



/**********************************************************************
 *	Function Open_Edit_DLOG(), this function lets the user type in their
 *	own creator and file types, and a brief description of the file
 *	type.
 **********************************************************************/

void Open_Edit_DLOG(void)
	{
	
	short		item_hit, the_item;
	Handle		menu_handle;
	Rect		the_rect;
	Boolean		done_with_dialog;
	GrafPtr		old_port;
	
	GetPort(&old_port);
	done_with_dialog=FALSE;
	Item_To_Edit=0;
	Edit_Dialog=GetNewDialog(EDIT_LIST_DLOG_ID, Edit_Storage, IN_FRONT);
	
	Make_Pop_Up_Menu();
	GetDItem(Edit_Dialog, POP_UP_MENU, &the_item, &menu_handle, &the_rect);
	(**(ControlHandle)menu_handle).contrlValue=0;
	Draw1Control((ControlHandle)menu_handle);
	
	ShowWindow(Edit_Dialog);
	while(!done_with_dialog)
		{
		ModalDialog(My_Dialog_Filter, &item_hit);
		switch(item_hit)
			{
			case EDIT_CLOSE:
				done_with_dialog=TRUE;
				break;
			case EDIT_ADD:
				Handle_Add_Item(Edit_Dialog, EDIT_CREATOR, EDIT_FILE, EDIT_DESCR);
				Draw1Control((ControlHandle)menu_handle);
				break;
			case EDIT_DELETE:
				Handle_Delete_Item();
				Draw1Control((ControlHandle)menu_handle);
				break;
			case POP_UP_MENU:
				Handle_Pop_Up_Menu(menu_handle, Edit_Dialog, EDIT_CREATOR, EDIT_FILE, EDIT_DESCR);
				break;
			case EDIT_GET_FILE:
				Handle_Make_Like(Edit_Dialog, EDIT_FILE, EDIT_CREATOR);
				break;
			defualt:
				break;
			}
  		}

	CloseDialog(Edit_Dialog);
	SetPort(old_port);
	
	}
	


/**********************************************************************
 *	Function Handle_Add_Item(), this function actually does the adding
 *	of the item to the preferences file.
 **********************************************************************/

void Handle_Add_Item(DialogPtr the_dialog, int creator, int file, int description)
	{
	
	prefs		**rsrc_handle;
	Handle		item_handle;
	short		item_type;
	Rect		item_rect;
	Str4		file_type, creator_type;
	Str32		description_type;
	int			temp;
	
	temp=Check_Data(the_dialog, file, NO)+Check_Data(the_dialog, creator, NO)+Check_Data(the_dialog, description, YES);
	
	if(temp<TYPE_LEN)
		{
		UseResFile(CreatorChangerPref);
			rsrc_handle=(prefs **)NewHandle(sizeof(prefs));
		
			GetDItem(the_dialog, description, &item_type, &item_handle, &item_rect);
			GetIText(item_handle, description_type);
			GetDItem(the_dialog, creator, &item_type, &item_handle, &item_rect);
			GetIText(item_handle, creator_type);
			GetDItem(the_dialog, file, &item_type, &item_handle, &item_rect);
			GetIText(item_handle, file_type);
		
			p_Str_Copy(description_type, (**rsrc_handle).Description);
			p_Str_Copy(creator_type, (**rsrc_handle).CreatorType);
			p_Str_Copy(file_type, (**rsrc_handle).FileType);
			
			AddResource((Handle)rsrc_handle, 'CrPf', Num_Of_Types==0 ? Strt_Rsrc : The_Type[Num_Of_Types]+1, description_type);
			ChangedResource((Handle)rsrc_handle);
			UpdateResFile(CreatorChangerPref);
			Num_Of_Types=CountResources('CrPf');
			ReleaseResource((Handle)rsrc_handle);
		UseResFile(CreatorChangerApp);
		Make_Pop_Up_Menu();
		}
	else if(temp<=TYPE_LEN+TYPE_LEN+1)
		{
		ParamText(Types_Error_1, "\p", Types_Error_2, "\p");
		Alert(ERROR_ALERT_ID, NIL_PTR);
		}
	else if(temp==DESC_LEN+2)
		{
		ParamText(Descr_Error, "\p", "\p", "\p");
		Alert(ERROR_ALERT_ID, NIL_PTR);
		}
	else
		{
		ParamText(Types_Error_1, Descr_Error, "\p", "\p");
		Alert(ERROR_ALERT_ID, NIL_PTR);
		}
	
	}



/**********************************************************************
 *	Function Handle_Delete_Item(), this function actually does the
 *	deleting of the item from the preferences file.
 **********************************************************************/

void Handle_Delete_Item()
	{
	
	prefs		**rsrc_handle;
	short		resource_number, i;
	
	if(Item_To_Edit)
		{
		UseResFile(CreatorChangerPref);
			resource_number=The_Type[Item_To_Edit];
			rsrc_handle=(prefs **)NewHandle(sizeof(prefs));
			rsrc_handle=(prefs **)GetResource('CrPf', resource_number);
			RmveResource((Handle)rsrc_handle);
			ReleaseResource((Handle)rsrc_handle);
			UpdateResFile(CreatorChangerPref);
		
			for(i=(resource_number+1);i<=The_Type[Num_Of_Types];i++)
				{
				rsrc_handle=(prefs **)GetResource('CrPf', i);
				DetachResource((Handle)rsrc_handle);
				AddResource((Handle)rsrc_handle, 'CrPf', i-1, NIL_PTR);
				ChangedResource((Handle)rsrc_handle);
				ReleaseResource((Handle)rsrc_handle);
				
				rsrc_handle=(prefs **)GetResource('CrPf', i);
				RmveResource((Handle)rsrc_handle);
				ChangedResource((Handle)rsrc_handle);
				ReleaseResource((Handle)rsrc_handle);
				}
			
			UpdateResFile(CreatorChangerPref);
			Num_Of_Types=CountResources('CrPf');
		UseResFile(CreatorChangerApp);
		Make_Pop_Up_Menu();
		Item_To_Edit=0;
		}
	else
		{
		ParamText(Bad_Item_Error, "\p", "\p", "\p");
		Alert(ERROR_ALERT_ID, NIL_PTR);
		}
		
	}