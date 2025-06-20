#include "AppleEvents.h"

void				Handle_One_Event(void);
void					Handle_Mouse_Down(void);
void					Handle_Menu_Choice(long);
void				Handle_Apple_Choice(int);
void				Handle_Options_Choice(int);
void				Handle_Pop_Up_Choice(int);

Boolean				All_Done;
Boolean				Multifinder_Active;
EventRecord			The_Event;
FInfo				File_Info;
FSSpec				The_File_Spec;

extern				pascal OSErr Open_AE(AppleEvent *, AppleEvent *, long);
extern MenuHandle	Apple_Menu;
extern MenuHandle	Options_Menu;
extern Ptr			About_Storage;
extern short		Multiple_Files;
extern long			Num_Of_Files;

