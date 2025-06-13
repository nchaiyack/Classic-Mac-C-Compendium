/* 22may95,ejo */
#include <Dialogs.h>

#define	LeftSlop	13			/* leave this much space on left of title */
#define	RightSlop	5			/* this much on right */
#define	BotSlop		5			/* this much below baseline */

#define	PopUpBaseItem	17
#define NumOfPopUps		3


typedef struct {
		Rect		MenuRect;
		MenuHandle	Menu;
		short		MenuID;
		short		Selection;
		}	PopUpType;
		
extern	PopUpType	PopUp[NumOfPopUps];

		
#if 1
/* 22may95,ejo: this one had wrong signature */
extern  pascal void DrawPopUp(DialogPtr theDialog, short theItem);
#else
extern  pascal void DrawPopUp(DialogPtr theDialog, int theItem);
#endif
extern	int DoPopUp(DialogPtr theDialog, int theItem);
