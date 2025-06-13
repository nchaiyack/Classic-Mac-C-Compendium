/**********************************************************************
 *	This file contains the functions which initialize the toolbox
 *	and check to see if System 7 is present, and loads the error 
 *	strings into memory.
 **********************************************************************/

#include "Creator Changer.h"
#include "Creator Changer.init.h"


/**********************************************************************
 *	Function Init_Toolbox(), this function initializes the Macintosh 
 *	toolbox so that all of the parts of the program will work.
 **********************************************************************/

void Init_Toolbox(void)
	{
	
	MaxApplZone();
	
	MoreMasters();
	MoreMasters();
	
	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NIL_PTR);
	FlushEvents(everyEvent, REMOVE_EVENTS);
	InitCursor();
	
	Chng_Storage=NewPtr(sizeof(DialogRecord));
	Edit_Storage=NewPtr(sizeof(DialogRecord));
	About_Storage=NewPtr(sizeof(DialogRecord));
	
	All_Done=FALSE;
	
	}



/**********************************************************************
 *	Function Get_Error_Messages(), this function gets the error
 *	messages that are stored in the resources.
 **********************************************************************/
 
void Get_Error_Messages(void)
	{
	
	GetIndString( Types_Error_1,  ERROR_STRING_LIST, TYPES_ERROR_1  );
	GetIndString( Types_Error_2,  ERROR_STRING_LIST, TYPES_ERROR_2  );
	GetIndString( Descr_Error,    ERROR_STRING_LIST, DESC_ERROR     );
	GetIndString( Bad_Item_Error, ERROR_STRING_LIST, BAD_ITEM_ERROR );
	
	}



/**********************************************************************
 *	Function Check_Sys_Type(), this function checks to see if System
 *	seven or later is present, if it isn't then the program quits.
 **********************************************************************/

void Check_Sys_Type(void)
	{
	
	short		sys_7=0x0700;
	short		vers_requsted=1;
	SysEnvRec	the_environ;
	
	SysEnvirons(vers_requsted, &the_environ);
	
	if(the_environ.systemVersion>=sys_7) All_Done=FALSE;
	else
		{
		GetIndString( Sys7_Error_1, ERROR_STRING_LIST, SYS7_ERROR_1);
		GetIndString( Sys7_Error_2, ERROR_STRING_LIST, SYS7_ERROR_2);
		ParamText(Sys7_Error_1, "\p", Sys7_Error_2, "\p");
		Alert(ERROR_ALERT_ID, NIL_PTR);
		All_Done=TRUE;
		}
	
	}


/**********************************************************************
 *	Function Install_AE_Handlers(), this function sets up the 
 *	AppleEvents for the program.
 **********************************************************************/

void Install_AE_Handlers(void)
	{
	
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,   (AEEventHandlerUPP)Open_AE,  FALSE, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, (AEEventHandlerUPP)Quit_AE,  FALSE, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, (AEEventHandlerUPP)Start_AE, FALSE, FALSE);
	AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,  (AEEventHandlerUPP)Print_AE, FALSE, FALSE);
	
	}



/**********************************************************************
 *	Function Open_Preferences(), this function opens the preference
 *	file that is used to set creator types.
 **********************************************************************/

void Open_Preferences(void)
	{
	
	OSErr	file_Err;
	prefs	**pref_handle_1, **pref_handle_2, **pref_handle_3, **pref_handle_4;
	Handle	**pref_string, **TMPL_handle;

	CreatorChangerApp=CurResFile();

	file_Err=FindFolder(kOnSystemDisk, kPreferencesFolderType, kDontCreateFolder, &Pref_VRef, &Pref_Dir_ID);
	if(file_Err==noErr) file_Err=FSMakeFSSpec(Pref_VRef, Pref_Dir_ID, Pref_File_Name, &Pref_Spec);
	if(file_Err==fnfErr)
		{
		FSpCreateResFile(&Pref_Spec, 'CPrf', kPreferencesFolderType, -1);

		CreatorChangerPref=FSpOpenResFile(&Pref_Spec, 3);
		
		TMPL_handle=(Handle **)GetResource('TMPL', 138);
		DetachResource((Handle)TMPL_handle);
		
		pref_string=(Handle **)GetResource('STR ', -16397);
		DetachResource((Handle)pref_string);

		pref_handle_1=(prefs **)GetResource('pref', Strt_Rsrc  );
		DetachResource((Handle)pref_handle_1);

		pref_handle_2=(prefs **)GetResource('pref', Strt_Rsrc+1);
		DetachResource((Handle)pref_handle_2);

		pref_handle_3=(prefs **)GetResource('pref', Strt_Rsrc+2);
		DetachResource((Handle)pref_handle_3);

		pref_handle_4=(prefs **)GetResource('pref', Strt_Rsrc+3);
		DetachResource((Handle)pref_handle_4);

		UseResFile(CreatorChangerPref);
			AddResource((Handle)TMPL_handle, 'TMPL', 138, "\pCrPf");
			AddResource((Handle)pref_string, 'STR ', -16397, NIL_PTR);
			AddResource((Handle)pref_handle_1, 'CrPf', Strt_Rsrc  , "\pS.T. Text");
			AddResource((Handle)pref_handle_2, 'CrPf', Strt_Rsrc+1, "\pS.T. Read Only");
			AddResource((Handle)pref_handle_3, 'CrPf', Strt_Rsrc+2, "\pS.T. Picture");
			AddResource((Handle)pref_handle_4, 'CrPf', Strt_Rsrc+3, "\pS.T. Movie");
			UpdateResFile(CreatorChangerPref);
			ReleaseResource((Handle)TMPL_handle);
			ReleaseResource((Handle)pref_string);
			ReleaseResource((Handle)pref_handle_1);
			ReleaseResource((Handle)pref_handle_2);
			ReleaseResource((Handle)pref_handle_3);
			ReleaseResource((Handle)pref_handle_4);
			Num_Of_Types=CountResources('CrPf');
 		UseResFile(CreatorChangerApp);
		}
	else
		{
		CreatorChangerPref=FSpOpenResFile(&Pref_Spec, 3);

		UseResFile(CreatorChangerPref);
			Num_Of_Types=CountResources('CrPf');
		UseResFile(CreatorChangerApp);
		}
	
	}



/**********************************************************************
 *	Function Set_Up_Menu_Bar(), this function displays the menu bar
 *	for the program. 
 **********************************************************************/

void Set_Up_Menu_Bar(void)
	{
	
	Handle	menu_bar;
	
	menu_bar=GetNewMBar(MAIN_MENU_BAR_ID);
	SetMenuBar(menu_bar);
	Apple_Menu=GetMHandle(APPLE_MENU_ID);
	Options_Menu=GetMHandle(OPTIONS_MENU_ID);
	AddResMenu(Apple_Menu, 'DRVR');
	DrawMenuBar();
	
	}
