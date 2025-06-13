
/* uuhost.c */
void sgenv(char **thename, char *envname, char *dflt);
void sloadenv(void);
pascal void Draw_Ring(WindowPtr W, int Item);
void Set_Main_State(int New_State);
void Hilite_Buttons(DialogPtr Dlog, int Min, int Max, int Value);
void Set_RButton(DialogPtr Dlog, int Item, int Min, int Max);
int Get_Debug_Level(void);
int Get_Command_Options(void);
void Do_Command(long Menu_Command, int modifiers);
void Do_Mouse_Action(EventRecord *Cur_Event);
int Check_Events(long Sleep_Time);
int Count_Appl_Files(void);
void AppendToCallMenu(void);
int main(int argc, char **argv);
void PlayNamed(char *theName);
void CTB_Setup(int item, int modifiers);
