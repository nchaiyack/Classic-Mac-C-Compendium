/*********************************************************************

	miniedit.h
	
	header file for Miniedit
	
	Copyright (c) 1989 Symantec Corporation.  All rights reserved.
	
*********************************************************************/

#define ErrorAlert		256

/* resource IDs of menus */
#define appleID			128
#define fileID			129
#define editID			130


/* Edit menu command indices */
#define undoCommand 	1
#define cutCommand		3
#define copyCommand		4
#define pasteCommand	5
#define clearCommand	6

/* Menu indices */
#define appleM			0
#define fileM			1
#define editM			2

#define SBarWidth	15

#ifndef NULL
#define NULL 0L
#endif


/*------------------------------------------------------------------------------
#
#	Apple Macintosh Developer Technical Support
#
#	MultiFinder-Aware TextEdit Sample Application
#
#	TESample
#
#	TESample.h	-	Rez and C Include Source
#
#	Copyright © 1989 Apple Computer, Inc.
#	All rights reserved.
#
------------------------------------------------------------------------------*/

/* kOSEvent is the event number of the suspend/resume and mouse-moved events sent
   by MultiFinder. Once we determine that an event is an OSEvent, we look at the
   high byte of the message sent to determine which kind it is. To differentiate
   suspend and resume events we check the resumeMask bit. */
#define	kOSEvent				app4Evt	/* event used by MultiFinder */
#define	kSuspendResumeMessage	1		/* high byte of suspend/resume event message */
#define	kResumeMask				1		/* bit of message field for resume vs. suspend */
#define	kMouseMovedMessage		0xFA	/* high byte of mouse-moved event message */
#define	kNoEvents				0		/* no events mask */


/*******************************************************************************\

include file

suntar, ©1991 Sauro & Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/

#ifndef LF
#define LF 	'\012'	/*	'\n' in Think C, '\r' in MPW	*/
#define CR 	'\015'	/*	'\r' in Think C, '\n' in MPW 	*/
#endif
#define enter_key	3

#define opClose		1
#define opQuit		2

typedef struct windowdef{
	WindowRecord	wRecord; /* è importante sia al primo posto, secondo il solito
								criterio di costruzioni di sottoclassi
								-- it must be the first field, according to the usual 
								object-oriented concept of inheriting a struct and 
								adding new fields */
	TEHandle		TEH;
	ControlHandle 	vScroll,hScroll;
	short				linesInWindow;	/* il numero di linee nella finestra */
	short				char_width;		/*larghezza in pixel dei caratteri: non è essenziale sia il
									vero valore, determina l'ampiezza dello scroll orizzontale 
									-- need not be the true width in pixels, it's used to choose
									the width of horizontal scroll*/
	char			dirty;
	char			used;
	short				flags;
	short				lastPrompt;		/* serve in modalità console (per semplicità,
									readonly è una console con lastPrompt=maxint)
									-- for "readonly" windows: a readonly window is readonly 
									till this point, read/write after that. You should set this
									to a value different from 32767 only in console windows */
	ProcPtr			itsClickLoop;
	} window_def;

#define n_max_windows 2	/* per suntar, ne uso una sola .....
						-- suntar uses only one window, but the module was 
						debugged as a text editor having 5 windows, and for some
						debugging we've used suntar with two */
#define n_max_foreign 8
#define maxTElength 32760
#define DelChar 8
#define prefM	3
#define prefID	131
#define fontID	142
#define sizeID	143
#define styleID 144

#define pmAutowrap 1
#define pmFont 2
#define pmSize 3
#define pmStyle 4


#define vecchiaROM (ROM85==-1)		/* 64K ROMs... prehistoric things */
/*#define vecchiaROM 1 /*debugging*/

extern window_def	my_windows[n_max_windows];
extern short		n_currently_open;
extern window_def	*curr_window;
#define myWindow ((WindowPtr)curr_window)
extern TEHandle		TEH;

#define hiword(x)		(((short *) &(x))[0])
#define loword(x)		(((short *) &(x))[1])
#define min(x,y)		((x)<(y)?(x):(y))
#define max(x,y)		((x)>(y)?(x):(y))

#ifdef may_run_on_64K_ROM
#define	MBARHEIGHT	(vecchiaROM ? 20:MBarHeight)
#define SCR_BAR_AND_TEXT_IN_SYNC	\
	if(!vecchiaROM)							\
		MaintainScrollBars(curr_window);	\
	else									\
		ShowSelect();
#else
#define	MBARHEIGHT MBarHeight
#define SCR_BAR_AND_TEXT_IN_SYNC MaintainScrollBars(curr_window);
#endif

#define screenHeight	(screenBits.bounds.bottom)
#define screenWidth 	(screenBits.bounds.right)

/* masks for flag bits: */
#define HSCROLL 1	/* autowrap or horizontal scroll */
#define	READONLY 2
#define CONSOLE 4
	/* the two bits READONLY and CONSOLE and the lastprompt field are used to 
	identify the three states of a window: read-only (e.g. a console without a prompt),
	fully read/write, console with a prompt currently on screen */
#define NOCLOSEBOX 16

#define isDAwindow(window) ((window)!=NULL && ((WindowPeek)(window))->windowKind < 0)
#define our(window) ((window)!=NULL && ((WindowPeek)(window))->windowKind >= userKind )
#define ourTE(window) (((WindowPtr)window)!=NULL && ((WindowPeek)(window))->windowKind == userKind && ((WindowPeek)(window))->refCon>=0)
/* l'applicazione che si appoggia a questo modulo può definirsi delle finestre,
ma o le distrugge prima di richiamare MainEvent (tipico per dialoghi modali) o
deve mettere windowkind > userKind o refCon < 0 
-- the application which uses the routines in MainEvent.c and windows.c may 
have its own windows, but in order to tell to MainEvent that it must not
touch them they must have either windowKind!=userkind or refCon<0 */
#define myIndex(window) ((short)(((window_def*)(window))->refCon))

#define AUTOSELECT /* usato solo per #ifdef */

#define ROOM_BEFORE_HSCROLL 150  /* suntar 2.01 uses that space for the "open file" name */

typedef void (*upd_proc)(EventRecord*);

extern void (*my_add_menus)(void);
extern void (*my_handle_menus)(long);
extern void (*my_event_filter)(EventRecord*);
extern short (*my_at_exit)(void);
extern Boolean	gInBackground;
extern Boolean sto_lavorando;
extern Cursor editCursor,waitCursor;
extern unsigned char window_title[];
extern short default_flags;
extern short command_modifiers;


extern unsigned char PNS[];	/* Pascal Null String: */
	/* it's silly to have tenths of null strings, rather refer to the same string */


#define SUNTAR

/* prototypes for MainEvent.c */
void InitConsole(void);
void install_handlers(WindowPtr,upd_proc,upd_proc);
void remove_handlers(WindowPtr);
void UpdateFilter (EventRecord *);
void MainEvent(void);
Boolean get_event(EventRecord*);
void handle_event(EventRecord*);
void DoKeyDown(long,short);
void DoActivate(window_def*,short);
void DoCommand(long);
void en_dis_edit(short,Boolean);
short SilentSuppression(short);
long pstrtoi(Str255);

/* prototypes for windows.c */
void new_window(void);
short get_window_index(void);
void AdjustText (window_def *);
void apply_preferences(window_def *);
void MaintainScrollBars(window_def *);
void UpdateWindow(WindowPtr);
void DoContent(WindowPtr,EventRecord*);
void MyGrowWindow(WindowPtr,Point);
void myZoomWindow(WindowPtr,Point,short);
void CloseMyWindow(void);
void PositionDialog(Rect *);
/* debug only: */
void print_string(char*);
void print_int(short);
void print_number(char*,short);

/* prototypes for suntar's functions */
short going_to_background(void);
void unexpected_disk_insertion(long);
void MaintainApplSpecificMenus(void);

/* prototypes for printf.c */
void disable_autoflush(short);
void enable_autoflush(void);
void flush_console(void);
void start_of_line(void);
void vai_a_capo(void);
void one_empty_line(void);
void prompt(char*,short);
void printf(const char*,...);
void print_chars(char*,short);
void put_char(char);
void update_console(void);
void aggiorna_nome_aperto(void);
void aggiorna_nome_file_aperto(void);
