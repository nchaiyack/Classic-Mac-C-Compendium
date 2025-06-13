/*
** main.h
**
** This is the main header file
*/

/********
  Application macros...
*********/


#define MENU_BAR	128
#define APPLE_MENU	128
#define FILE_MENU	129
#define EDIT_MENU	130
#define LISTEN_MENU	131

#define AppleAboutItem	1		/* Apple menu options */

#define FileNewItem		1		/* File menu options */
#define FileOpenItem	2
#define	FileCloseItem	3
#define FileSaveItem	5
#define FileSaveAsItem	6
#define	FileQuitItem	8

#define EditUndoItem	1		/* Edit menu options */
#define EditCutItem		3
#define EditCopyItem	4
#define EditPasteItem	5
#define EditClearItem	6

#define ListenBegin		1		/* Listen menu options */
#define ListenStop		2

#define MainWindID		128		/* main window info */
#define MainWindHt		364
#define MainWindWd		512

#define ErrorDlogID		128		/* Error dialog window */
#define ErrorOKButt		1
#define AboutDlogID		129		/* dialog window ID */
#define AboutOKButt		1

#define kVScrollBarID	128     /* window's vert scroll bar */
#define kDrawIconOffset	13		/* scroll bar offset from draw icon */
#define kScrollBorderOffset	15	/* scroll bar offset from border */

#define kIndStringID		128		/* our STR# res ID */
#define kPromptString		1		/* Now for the strings we need */

/********
  Data Structures
********/

/********
  Global Variables... located in globals.c
********/

extern MenuHandle	gFileM, gEditM,
					gAppleM, gListenM;	/* handles to menus */
extern WindowPtr	gMainWindow;		/* the main window  */
extern int			gListening;			/* Are we listening to input? */

/*** circle drawing stuff ***/

#define PI 3.1415926
extern double gAngleStep;  /* radians per step, initial=30¡ */
extern short gOffset;      /* offset from radius */
extern short gRadius;    /* radius size, default is 100 */

/********
  Functions
********/

void Cleanup(void);                  /* main.c */

Boolean TrapAvailable(int theTrap); /* trapavail.c */

void DoAbout(void);        /* dostuff.c */
void DoEvent(EventRecord *);
void DoMouseDown(EventRecord *);
void DoCommand(long);
void DoFileNew(void);
void DoFileClose(void);
void DoContent(EventRecord *);
void DoKey(EventRecord *);
void Error(Str255, Str255, Str255, Str255);

void DrawStep(void);         /* drawing.c */

void InitSound(void);     /* sound.c */
void KillSound(void);
void SndListenStop(void);
void SndListenBegin(void);
int FetchSndOffset(void);
