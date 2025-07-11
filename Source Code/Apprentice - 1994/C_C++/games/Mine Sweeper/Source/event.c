/*	event.c
 *
 *		The event manager routines
 */


#include <stdio.h>
#include "event.h"
#include "mines.h"

/****************************************************************/
/*																*/
/*							Forwards							*/
/*																*/
/****************************************************************/

int GetEvent(EventRecord *);			/* Fetch Event Stuff	*/
void DoMenu(long);						/* Process events		*/
void DoMouse(EventRecord *);
void DoAboutMe(void);					/* Put up about me box	*/

/****************************************************************/
/*																*/
/*							Globals								*/
/*																*/
/****************************************************************/

short		quitFlag;					/* Event Quit Flag		*/

short		SizeX = 30;					/* size bitmaps			*/
short		SizeY = 15;
short		SizeFlag = 2;				/* Easy mode			*/
short		Bombs = 90;					/* # of bombs			*/
short		BombFlag = 1;				/* Easy mode (again)	*/
short		BRatio = 5;					/* Bomb ratio			*/

short		BStatus[40][40];			/* Bomb status			*/
short		BPosition[40][40];			/* Bomb info flags		*/
short		BState = 0;					/* Game state			*/
short		BFirst = 1;					/* First click?			*/
short		BCount = 0;					/* Bomb count value		*/

long		LogTime;
long		CurTime;

short		XLoc,YLoc;					/* X, Y location		*/

ScoreRec	Scores[6];					/* High scores			*/

WindowPtr	MineWindow = NULL;			/* Mines window			*/

short		Cruse = 0;					/* Cruse Control		*/

/****************************************************************/
/*																*/
/*	Window Dispatcher Routines									*/
/*																*/
/****************************************************************/

/*	DoClose
 *
 *		Handle closing this window
 */

void DoClose(WindowPtr w)
{
	short i;
	
	if (w == NULL) return;
	
	i = ((WindowPeek)w)->windowKind;
	if (i < 0) CloseDeskAcc(i);
	else switch (i) {
		case WK_GAME:
			quitFlag = 1;
			break;
		default:
			CloseWindow(w);
			break;
	}
}

/*	DoUpdate
 *
 *		This handles the update event
 */

void DoUpdate(WindowPtr w)
{
	short i = ((WindowPeek)w)->windowKind;
	
	BeginUpdate(w);
	SetPort(w);
	switch (i) {
		case WK_GAME:
			UpdateMines(w);
			break;
		default:
			break;
	}
	EndUpdate(w);
}

/*	DoClick
 *
 *		Handle mouse down
 */

void DoClick(WindowPtr w, Point pt, short mods)
{
	short i = ((WindowPeek)w)->windowKind;
	
	SetPort(w);
	GlobalToLocal(&pt);
	switch (i) {
		case WK_GAME:
			ClickMines(w,pt,mods);
			break;
	}
}



/****************************************************************/
/*																*/
/*						Event Dispatcher						*/
/*																*/
/****************************************************************/

/*	DoEvent
 *
 *		Get the next event and process it
 */

int DoEvent()
{
	EventRecord e;
	char c;
	
	quitFlag = 0;						/* Clear the quit status flag */
		
	/*
	 *	Handle the next event in the event queue
	 */
	
	DoPeriodic();
	SystemTask();
	GetNextEvent(everyEvent,&e);
	switch (e.what) {
		case mouseDown:					/* Handle mouse down events */
			DoMouse(&e);
			break;
		case keyDown:					/* Handle keyboard events */
		case autoKey:
			c = e.message & charCodeMask;
			if (e.modifiers & cmdKey) {
				if (e.what == keyDown) DoMenu(MenuKey(c));
			}
			break;
		case updateEvt:					/* Handle window update events */
			DoUpdate((WindowPtr)(e.message));
			break;
		case activateEvt:				/* Handle window activate/deactivate events */
			break;
	}
	
	return quitFlag;					/* Return the quit status flag */
}


/****************************************************************/
/*																*/
/*						Mouse Dispatcher						*/
/*																*/
/****************************************************************/

/*	DoMouse
 *
 *		This does the right thing when the mouse button is
 *		clicked
 */

static void DoMouse(event)
EventRecord *event;
{
	int i;
	WindowPtr w;
	GrafPtr foo;
	Rect r;
	
	i = FindWindow(event->where,&w);
	if (w != NULL) SetPort(w);								/* Assure current port */
	
	switch (i) {
		case inDesk:
			break;
		case inMenuBar:
			DoMenu(MenuSelect(event->where));
			break;
		case inSysWindow:
			SystemClick(event,w);
			break;
		case inDrag:
			GetWMgrPort(&foo);
			r = foo->portRect;
			InsetRect(&r,4,4);
			r.top += 20;
			DragWindow(w,event->where,&r);
			break;
		case inGoAway:
			if (w != NULL) {
				if (TrackGoAway(w,event->where)) DoClose(w);
			}
			break;
		case inContent:
			DoClick(w,event->where,event->modifiers);
			break;
	}
}

/****************************************************************/
/*																*/
/*						Menu Dispatcher							*/
/*																*/
/****************************************************************/



/*	DoMenu
 *
 *	How to dispatch menu events
 */

static void DoMenu(l)
long l;
{
	short hi,lo;
	unsigned char buffer[255];
	MenuHandle mh;
		
	hi = (short)(l >> 16);
	lo = (short)(l & 0x0FFFF);
		
	switch (hi) {
		case APPLMENU:
			switch (lo) {
				case ABOUTME:
					DoAboutMe();
					break;
				default:
					mh = GetMHandle(APPLMENU);
					if (mh != NULL) {
						GetItem(mh,lo,buffer);
						OpenDeskAcc(buffer);
					}
					break;
			}
			break;
		case EDITMENU:
			SystemEdit(lo-1);
			break;
		case FILEMENU:
			switch (lo) {
				case QUIT:				/* Quit application */
					quitFlag = 1;
					return;
				case CLOSE:
					DoClose(FrontWindow());
					break;
				case NEW:
					NewMines();
					break;
				case PREF:
					Preferences();
					break;
			}
	}
	HiliteMenu(0);
}


/****************************************************************/
/*																*/
/*						About Me Display						*/
/*																*/
/****************************************************************/


/*	CenterWindow
 *
 *		Move this window to make it in the center of the main
 *	screen
 */

static void CenterWindow(w)
WindowPtr w;
{
	GrafPtr foo;
	Rect r;
	short x,y;
	
	r = w->portRect;
	x = r.right - r.left;
	y = r.bottom - r.top;
	GetWMgrPort(&foo);
	r = foo->portRect;
	x = r.right - r.left - x;
	y = r.bottom - r.top - y - 22;
	x /= 2;
	y /= 2;
	y += 22;
	
	MoveWindow(w,x,y,1);
}


/*	AboutMeDH
 *
 *	Handle the ModalDialog for the 'about me' dialog
 */

pascal Boolean _AboutMeDH(DialogPtr theDialog,EventRecord *theEvent,short *itemHit)
{
#	pragma unused(theDialog)

	if (theEvent->what == mouseDown) {
		*itemHit = 1;
		return true;
	} else return false;
}

/*	BufferScore
 *
 *		Put into a buffer the score
 */

void BufferScore(char *b, short i)
{
	short n;
	
	if (Scores[i].time >= 1000) {
		b[0] = '\0';
		return;
	}
	NumToString(Scores[i].time,(unsigned char *)b);
	while (b[0] < 5) b[++b[0]] = ' ';
	for (n = 1; n <= Scores[i].name[0]; n++) b[++b[0]] = Scores[i].name[n];
}

/*	DoHelp
 *
 *		Display help dialogs
 */

static void DoHelp(void)
{
	DialogPtr dlog;
	short i;
	
	dlog = GetNewDialog(130,NULL,(WindowPtr)-1);
	if (dlog == NULL) return;
	SetPort(dlog);
	CenterWindow(dlog);
	ShowWindow(dlog);

	for (;;) {
		ModalDialog(_AboutMeDH,&i);
		if (i == 1) break;
	}

	DisposDialog(dlog);
}


/*	DoAboutMe
 *
 *	How to handle the 'about me' box
 */

static void DoAboutMe()
{
	DialogPtr dlog;
	short i1;
	Rect r;
	Handle i2;
	char buffer[48];
	
	/*
	 *	About me...
	 */
	
	dlog = GetNewDialog(128,NULL,(WindowPtr)-1);
	if (dlog == NULL) return;
	SetPort(dlog);
	CenterWindow(dlog);
	ShowWindow(dlog);
	GetDItem(dlog,1,&i1,&i2,&r);
	InsetRect(&r,-4,-4);
	PenSize(3,3);
	FrameRoundRect(&r,16,16);
	PenSize(1,1);
	
	/*
	 *	Draw high scores
	 */
	
	GetDItem(dlog,4,&i1,&i2,&r);
	FrameRect(&r);
	TextFont(1);
	TextSize(9);
	
	MoveTo(r.left + 5, r.top + 12);
	DrawString("\pHigh Scores for Large Field/High Density:");
	TextFont(4);
	MoveTo(r.left + 10, r.top + 26);
	TextFace(4);
	DrawString("\pTime Name");
	TextFace(0);
	for (i1 = 0; i1 < 6; i1++) {
		MoveTo(r.left + 10, r.top + 36 + 10 * i1);
		BufferScore(buffer,i1);
		DrawString((unsigned char *)buffer);
	}
	
	TextFont(0);
	TextSize(12);
	
	for (;;) {
		ModalDialog(NULL,&i1);
		if ((i1 == 1) || (i1 == 2)) break;
	}
	DisposDialog(dlog);
	if (i1 == 1) return;
	if (i1 == 2) DoHelp();
}


/****************************************************************/
/*																*/
/*	Main entry point											*/
/*																*/
/****************************************************************/

main()
{
	InitApp();						/* Start me up				*/
	NewMines();
	while (!DoEvent());
	EndApp();						/* Finish me off			*/
}
