#include "AppleEvents.h"
#include "Folders.h"

void				Init_Toolbox(void);
void				Get_Error_Messages(void);
void				Check_Sys_Type(void);
void				Install_AE_Handlers(void);
void				Open_Preferences(void);
void				Set_Up_Menu_Bar(void);

Str63				Types_Error_1;
Str32				Types_Error_2;
Str32				Descr_Error;
Str32				Sys7_Error_1;
Str32				Sys7_Error_2;
Str32				Bad_Item_Error;
Str32				Pref_File_Name="\pCreator Changer Preferences";

MenuHandle			Apple_Menu;
MenuHandle			Options_Menu;

short				Strt_Rsrc=128;
short				CreatorChangerApp, CreatorChangerPref;
short				Pref_VRef;
short				Num_Of_Types;

long				Pref_Dir_ID;
FSSpec				Pref_Spec;
prefs				**Types_Handle;

Ptr					Chng_Storage;
Ptr					Edit_Storage;
Ptr					About_Storage;

extern Boolean		All_Done;
extern pascal		OSErr	Open_AE( AppleEvent *, AppleEvent *, long);
extern pascal		OSErr	Quit_AE( AppleEvent *, AppleEvent *, long);
extern pascal		OSErr	Print_AE(AppleEvent *, AppleEvent *, long);
extern pascal		OSErr	Start_AE(AppleEvent *, AppleEvent *, long);
