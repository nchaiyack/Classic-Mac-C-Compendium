//� FontShow.h

#include <Printing.h>		//� #included by kal. 

//� #exclude <ctype.h>		//� #excluded by kal.
//� #exclude <unix.h>		//� #excluded by kal. 

#define OK 1
#define Cancel 2

//� Menu stuff.
#define APPLE 		0
#define FILE		1
#define EDIT		2
#define OPTIONS	3
#define SIZE		4

#define APPLE_M 	128
#define FILE_M		129
#define EDIT_M		130
#define OPTIONS_M	131
#define SIZE_M		132
#define TEMP_M		200	//� Temporary menu for list creating.

#define SIZE9		1
#define SIZE10		2
#define SIZE12		3
#define SIZE14		4
#define SIZE18		5
#define SIZE20		6
#define SIZE24		7

#define INFO		1
#define SETUP 		1
#define DISPLAY	2
#define PRINT		3
#define QUIT		5
#define SAMPLE		1
#define STRING		2
#define MAP			3

//� Dialog stuff.
#define FONTLIST		128
#define WAIT			129
#define ABOUT			130
#define SCREENBREAK	131
#define PRINTBREAK	132
#define SPOOLINFO		133
#define PRINTINFO		134

#define INALL		4
#define EXALL		5
#define REVERT		6
#define FLIST	7

//� Output stuff.
#define FONTWINDOW 128
#define STRINGS	 129

#define TOPH	8
#define TOPV	24
#define TOPF	16

//� General stuff.
#define NIL (void *)0L

//� Window stuff.
WindowPtr theWindow;

//� Error messages.
#define ERR_NOROOM	1
#define ERR_PRINT	2

short PrintErr : 0x944;

//�-----------------------------------------------------------------------�//
//�-------------------- Global variables ---------------------------------�//
//�-----------------------------------------------------------------------�//

typedef struct 
{
	Str255	fontName;			//� font name.
	short		fontNum;				//� font number.
	short		lineHeight;			//� line height.
	short 		widMax;				//� maximal character width.
	Boolean	flag;					//� flag set if font is selected.
}myFontInfo;						//� infos about each font.

myFontInfo	**fontHdl;			//� pointer to dynamic array of myFontInfos.
ListHandle 	myList;				//� list of font names.
CursHandle	myCursor[4];		//� busy cursor.
MenuHandle 	myMenus[5];			//� menu bar.
THPrint		prRecHdl;			//� print info.
Handle		myText;				//� handle to sample text.
short			textLength;			//� sample text length.
short 			fontCount;			//� nr of fonts in system.
short			opt;					//� sample text or character map.
Boolean 		prFlag;				//� printer or screen output.
Boolean 		quit;					//� quit main loop.
short			fontSize;			//� selected font size.
short			oldItem;				//� font size menu item.

//�-----------------------------------------------------------------------�//
//� Prototypes -----------------------------------------------------------�//
//�-----------------------------------------------------------------------�//

extern void ErrorMsg (short index);
extern void ShowInfo (void);
extern Boolean BreakCheck (void);
extern void DrawPage (short i);
extern void ShowFonts (void);
extern short HowMany (void);
extern void PrintFonts (void);
extern pascal void DrawList (WindowPtr theWindow, short itemNo);
extern pascal Boolean myFilter (DialogPtr theDialog, EventRecord *theEvent, 
										short *itemHit);
extern void InitList (WindowPtr theWindow);
extern Boolean SelectFonts (void);
extern void HandleMenu (long sel);
extern void HandleMouseDown (EventRecord event);
extern void MainLoop (void);
extern void InitMenu (void);
extern void InitText (void);
extern void InitPrint (void);
extern void BuildList (void);
extern void InitThings (void);
extern void CleanUp (void);
extern void main (void);
