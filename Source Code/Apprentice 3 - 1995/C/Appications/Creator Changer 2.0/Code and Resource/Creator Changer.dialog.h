#include "AppleEvents.h"

extern pascal Boolean	My_Dialog_Filter(DialogPtr, EventRecord *, short *);
void					Open_Changer_DLOG(AEDescList *);
Boolean						Handle_OK_Change(DialogPtr, int, int, AEDescList *);
OSType							Set_Long_Type(DialogPtr, int);
void							Set_Type_Text(File_Union *, OSType);
void							Handle_Make_Like(DialogPtr, short, short);
extern pascal Boolean	My_Dialog_Filter(DialogPtr, EventRecord *, short *);
void					Open_Edit_DLOG(void);
void 					Handle_Add_Item(DialogPtr, int, int, int);
void	 				Handle_Delete_Item();

extern FInfo		File_Info;
extern short		The_Type[100];
extern short		Multiple_Files;
extern Ptr			Chng_Storage;
extern FSSpec		The_File_Spec;
extern long			Num_Of_Files;
extern EventRecord		The_Event;
extern int				Strt_Rsrc;
extern short			Num_Of_Types;
extern short			The_Type[100];
extern short			CreatorChangerApp, CreatorChangerPref;
extern short			Item_To_Edit;
extern Boolean			Done_With_Dialog;
extern Str63			Types_Error_1;
extern Str32			Types_Error_2;
extern Str32			Descr_Error;
extern Str32			Bad_Item_Error;
extern Ptr				Edit_Storage;

File_Union    		F_Type, C_Type;
DialogPtr			Chng_Dialog, Edit_Dialog;
