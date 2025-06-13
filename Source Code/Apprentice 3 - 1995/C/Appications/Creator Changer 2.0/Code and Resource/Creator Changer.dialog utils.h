#define	Escape_Key	0x1B
#define	Return_Key	0x0D
#define	Enter_Key	0x03

void					Draw_Border(DialogPtr, short, short);
pascal	Boolean			My_Dialog_Filter(DialogPtr, EventRecord *, short *);
void 					Handle_Key_Pressed(DialogPtr, int);
int						Check_Data(DialogPtr, int, short);
void					Set_DLOG_Text_Item(DialogPtr, short, StringPtr);
void					Draw_Dialog(void);

extern EventRecord		The_Event;
extern DialogPtr		Chng_Dialog;
extern DialogPtr		Edit_Dialog;
extern Str63			Types_Error_1;
extern Str32			Types_Error_2;
extern Str32			Descr_Error;
extern File_Union    	F_Type, C_Type;
extern short			Multiple_Files;
