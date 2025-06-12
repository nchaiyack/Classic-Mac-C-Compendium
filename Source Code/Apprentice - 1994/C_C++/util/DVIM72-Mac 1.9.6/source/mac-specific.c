#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <PrintTraps.h>
#include <Balloons.h>
#include "mac-specific.h"
#include "mac_printing.h"
#include "dvihead.h"
#include "gendefs.h"
#include "egblvars.h"
#include "gblprocs.h"
#include "m72.h"
#include "TextDisplay.h"
#include "sillymalloc.h"
#include "Copy_banded.h"

#define PROFILE 0

#define REVEAL_DIALOG(d) {SelectWindow(d);ShowWindow(d);}
#define MOVE_TO_FRONT		-1L
#define MIN_SLEEP			3L
#define NIL_MOUSE_REGION	0L
#define NIL_POINTER			0L
#define GROW_ICON_SIZE		16

#define WNE_TRAP_NUM		0xA860

#define APPLE_MENU_ID		1
#define FILE_MENU_ID		401

#define HELP_ID			128

#define ABOUT_ITEM			1
#define APPLE_HELP_ITEM		2

#define DRAG_THRESHOLD	30
#define IDLE_INTERVAL	10	/* Min. interval in ticks between printer idle stuff */

/* ------------- global variables ----------------- */
DialogPtr			g_cancel_dialog;
Handle			cancel_dialog_text;
EventRecord		g_TheEvent;
Rect 				g_DragRect, g_screenrect;
MenuHandle		g_AppleMenu, g_FileMenu;
Boolean			g_Done, g_WNEImplemented;
Boolean			g_abort_dvi; /* error cancels this dvi file */
int				g_dpi;	/* bitmap resolution */
int				g_printer_dpi;	/* device resolution */
int				g_print_res_file;
short				g_app_resfile;
GrafPort		g_offscreen_GrafPort;
Boolean		g_draw_offscreen;
Boolean		g_preview;	/* use preview window */
WindowPtr		g_page_window,
				g_console_window;
THPrint			g_print_rec_h = NIL_POINTER;
Str255			g_other_commands;
WindowPtr		g_freemem_window;
MenuHandle		g_popup_menu;
Boolean			g_use_sysheap;
long				g_last_idle_time = 0L;
Boolean			g_help_under_apple;
short				g_paper_width;
short				g_paper_length;
QDProcs			offscreen_QDprocs;
short				g_print_status; // 1=driver open, 2=doc open, 3=page open

/* -------------- prototypes of private routines ------------ */
void MainLoop( void );
void HandleMouseDown( void );
void HandleAppleChoice( short theItem );
void	show_about		( void );
void	Show_warning( 	char *msg );
void	init_toolbox	( void );
int		Get_resource_id(
						OSType rsrc_type,
						char *rsrc_name);
Handle Get_resource_by_id( OSType rsrc_type, int r_id );
void	Get_resource(				/* find a resource by name */
						OSType resource_type,
						char *resource_name,
						Handle *resource_handle,
						short *resource_id);
void	Get_file_path( 	char *thepath );
Boolean	User_wants_out	( void );
void	init_per_dvi(	void );
pascal Boolean
		Dvi_filter( 	ParmBlkPtr param_block );
void	Get_dpi(		void );
Boolean Get_event(		int mask,
						EventRecord *what);
void	Get_dialogs(	void );
void	Setup_drag_rect(	void );
pascal Boolean	Do_update( WindowPtr  where );
void	Handle_print(	Boolean use_job_dialog );
void HandleMenuChoice( long menuChoice );
void	Handle_event( void );
void	Do_page_setup(	void );
void	Init_print_rec(	void );
void	Open_printer(	Boolean use_job_dialog,
						int *first,
						int *last );
void	HandleFileChoice( int theItem );
Boolean	IsDAWindow( WindowPtr whichWindow );
Boolean	get_bool_resource( char *rsrc_name );
int		get_int_resource( char *rsrc_name );
void		set_int_resource( char *rsrc_name, int new_value );
void	set_str_resource(
						char *rsrc_name,
						char *new_value );
char **
		get_str_resource( char *rsrc_name );
pascal void	Printer_idle( void );
void Update_PREC( void );
void  Set_pic_procs( void );
void	Do_help( void );
void Init_menubar( void );
void Draw_grow_icon( void );

/* -------------------------------------------------------------------- */

/* ----- a couple of prototypes of external routines ------ */
void	Handle_settings( void );
void	dvifile( char *filename );
void	option( char *an_option );
void	rollbeachball( void );
Boolean setupbeachball( void );
void	Reposition_windows( void );
void	Save_window_positions( void );
void	Center_window(	WindowPtr my_window );
void	Explain_print_error( OSErr the_error );
void	Update_freemem( void );
void	Update_memory_indicators( void );
void	openfont( char *fontname ); /* only used here for UnloadSeg */
void	special( char *s ); /* only used here for UnloadSeg */
ControlHandle	Get_Ditem_handle( DialogPtr theDialog, int item_no );
pascal void Get_PICT_data( Ptr data_ptr, int byte_count );
void	Install_help( void );
void Maximize_resolution( void );
void Reset_resolution( void );
void Find_prefs( void );
Boolean Cancel_key( EventRecord *theEvent );
Boolean TrapAvailable( short trapnum );

/* ----------------------------- main ------------------------------ */
void
main()
{
	init_toolbox();
	(void) strcpy(g_progname, "DVIM72-Mac");
	g_app_resfile = CurResFile();
	Setup_drag_rect();
	EventAvail( everyEvent, &g_TheEvent ); /* Move to the foreground */
	EventAvail( everyEvent, &g_TheEvent ); /* Move to the foreground */
	EventAvail( everyEvent, &g_TheEvent ); /* Move to the foreground */
	Find_prefs();
	Get_dialogs();
	Reposition_windows();
	printf( "\n\n\n\nWelcome to DVIM72-Mac.\n" ); CHK("begin");
	REVEAL_DIALOG( g_freemem_window );
	silly_get_MF_mem_status();
	g_use_sysheap = MF_mem_status >= sm_temp_mem;
	g_print_status = 0;
	Get_dpi();
	(void) initglob();
	if (g_preview)
		REVEAL_DIALOG( g_page_window );
	Init_menubar();
	
	/*
		Offscreen drawing initialization
	*/
	g_draw_offscreen = get_bool_resource("DRAW OFFSCREEN");
	g_print_by_bands = get_bool_resource("BANDING") && g_draw_offscreen;
	OpenPort( &g_offscreen_GrafPort );
	CopyRgn( g_offscreen_GrafPort.clipRgn, g_offscreen_GrafPort.visRgn ); /* wide open */
	g_offscreen_GrafPort.portBits.baseAddr = NIL;
	SetStdProcs( &offscreen_QDprocs );
	offscreen_QDprocs.getPicProc = (Ptr) &Get_PICT_data;
	g_offscreen_GrafPort.grafProcs = &offscreen_QDprocs;

	Update_freemem();
	InitCursor();
	UseResFile( g_app_resfile );
	(void) setupbeachball(); CHK("before main loop");
	UseResFile( g_pref_resfile );
	MainLoop();
	Save_window_positions();
	TD_close_log();
}


/* ------------------------ Setup_drag_rect ----------------- */
void
Setup_drag_rect()
{
	SysEnvRec	the_world;
	OSErr 			err;

	err = SysEnvirons(1, &the_world );
	if ( (err != noErr) || !the_world.hasColorQD )
	{
		g_screenrect = screenBits.bounds;
	}
	else
		g_screenrect = (**GetMainDevice()).gdRect;
	if (EmptyRect(&g_screenrect))
		fatal("Can't get the screen rectangle!\n");
	g_screenrect.top += GetMBarHeight();
	g_DragRect = (**GetGrayRgn()).rgnBBox;
	InsetRect( &g_DragRect, DRAG_THRESHOLD, DRAG_THRESHOLD );
	g_DragRect.top -= DRAG_THRESHOLD;
}

/* ------------------- Set_pic_procs ------------------------ */
/*
	If we are not drawing offscreen, set the getPicProc in the printing GrafPort
	and in the page preview window, in order to draw pictures from PICT files.
*/
void  Set_pic_procs( void )
{
	GrafPtr save_port;
	QDProcsPtr	my_procs;
	
	GetPort( &save_port );
	SetPort( (GrafPtr)g_print_port_p );
	if (g_print_port_p->gPort.grafProcs == NIL)
	{
		/*
			One case in which this happens is when the StyleWriter driver
			has background printing turned on.  Then PrOpenPage just does an
			OpenPicture.
		*/
		my_procs = (QDProcsPtr) MALLOC( sizeof(QDProcs) );
		if (my_procs != NIL)
		{
			SetStdProcs( my_procs );
			my_procs->getPicProc = (Ptr)&Get_PICT_data;
			g_print_port_p->gPort.grafProcs = my_procs;
		}
	}
	else
		g_print_port_p->gPort.grafProcs->getPicProc = (Ptr)&Get_PICT_data;
	
	SetPort( g_page_window );
	if (g_page_window->grafProcs == NIL)
	{
		my_procs = (QDProcsPtr) MALLOC( sizeof(QDProcs) );
		if (my_procs != NIL)
		{
			SetStdProcs( my_procs );
			my_procs->getPicProc = (Ptr)&Get_PICT_data;
			g_page_window->grafProcs = my_procs;
		}
	}
	SetPort( save_port );
}

/* ---------------------------- Get_dialogs ------------------------- */
void
Get_dialogs()
{
	GrafPtr save_port;
	int cancel_DLOG_id;
	int WIND_id;
	OSErr	stat;
	
	GetPort( &save_port );

	cancel_DLOG_id = Get_resource_id( 'DLOG', "cancel print" );
	CouldDialog( cancel_DLOG_id );
	g_cancel_dialog = GetNewDialog( cancel_DLOG_id,
		nil, (WindowPtr)MOVE_TO_FRONT );
	cancel_dialog_text = (Handle)Get_Ditem_handle( g_cancel_dialog, 1 );
	
	WIND_id = Get_resource_id( 'WIND', "console" );
	g_console_window = TD_new( WIND_id, 5000,  4000, 9, &stat );
	if (stat != noErr)
		fatal("Error opening console window.");

	WIND_id = Get_resource_id( 'WIND', "FreeMem" );
	g_freemem_window = GetNewWindow( WIND_id,
		nil, (WindowPtr)MOVE_TO_FRONT );

	g_preview = get_bool_resource("PREVIEW");
	WIND_id = Get_resource_id( 'WIND', "page" );
	g_page_window = GetNewWindow( WIND_id,
		nil, (WindowPtr)MOVE_TO_FRONT );
	
	SetPort( save_port );
}

/* --------------------------- Get_dpi ----------------------------- */
void
Get_dpi()
{
	g_dpi = get_int_resource("BITMAP DPI");
}


/* ------------------------ MainLoop ------------------- */

void MainLoop( void )
{
	g_Done = FALSE;
	g_WNEImplemented = TrapAvailable( WNE_TRAP_NUM );
	while ( g_Done == FALSE )
	{
		Handle_event();
	}
	/* fflush(stdout); */
}

#define	RESUME_MASK			1
#define	HELP_CHAR			0x05
/* ----------------- Handle_event ----------------- */

void Handle_event()
{
	char theChar;

	(void)Get_event( everyEvent, &g_TheEvent );
	
	switch (g_TheEvent.what)
	{
#if NEVER
		case nullEvent:
			/* HandleNull(); */
			break;
#endif
		case mouseDown:
			HandleMouseDown();
			break;
		case keyDown:
		case autoKey:
			theChar = g_TheEvent.message & charCodeMask;
			if (Cancel_key(&g_TheEvent)
				&& ((WindowPeek)g_cancel_dialog)->visible )
			{
				g_abort_dvi = TRUE;
				PrSetError( iPrAbort );
				SetIText( cancel_dialog_text, "\pCancelling..." );
				SetPort( g_cancel_dialog );
				DrawDialog( g_cancel_dialog );
			}
			else if ( (g_TheEvent.modifiers & cmdKey) != 0 )
				HandleMenuChoice( MenuKey( theChar ) );
			else if (theChar == HELP_CHAR)
				Do_help();
			else
				SysBeep(1);
			break;
		case updateEvt:
			Do_update( (WindowPtr)g_TheEvent.message );
			break;
		case activateEvt:
			SetPort( (WindowPtr)g_TheEvent.message );
			if (thePort == g_console_window)
			{
				if (g_TheEvent.modifiers & activeFlag) /* activate event */
					TD_activate( TRUE );
				else
					TD_activate( FALSE );
				Draw_grow_icon();
			}
			break;
		case app4Evt:
			SetPort( FrontWindow() );
			if (g_TheEvent.message & RESUME_MASK)
			{
				TD_activate( TRUE );
				Update_memory_indicators();
				InitCursor();
			}
			else
				TD_activate( FALSE );
			break;
	}
}

/* ------------------- Do_Update -------------------------- */
pascal Boolean
Do_update( where )
	WindowPtr where;
{
	Rect	from_rect;
	GrafPtr	save_port;
	
	if ( !IsDAWindow( where ) )
	{
		GetPort( &save_port );
		SetPort( (GrafPtr)where );
		BeginUpdate( where );
		if (where == g_page_window)
		{
			DrawGrowIcon( where );
		}
		else if (where == g_cancel_dialog)
			DrawDialog( g_cancel_dialog );
		else if (where == g_console_window)
		{
			EraseRect( &where->portRect );
			Draw_grow_icon();
			DrawControls( where );
			TD_update( );
		}
		EndUpdate( where );
		/*
			Update the free memory window outside BeginUpdate--EndUpdate
			in order to redraw the whole thing.  Otherwise the numbers get
			"fractured".
		*/
		if (where == g_freemem_window)
			Update_freemem();
		SetPort( save_port );
	}
	return true;
}

/* ----------------- Draw_grow_icon ----------------- */
/*
	Draw the grow icon with clipping so that it does not draw scroll bar lines.
*/
void Draw_grow_icon( void )
{
	Rect icon_rect;
	RgnHandle	save_clip;
	
	icon_rect = thePort->portRect;	/* get right and bottom */
	icon_rect.top = icon_rect.bottom - GROW_ICON_SIZE + 1;
	icon_rect.left = icon_rect.right - GROW_ICON_SIZE + 1;
	
	save_clip = NewRgn();
	GetClip( save_clip );
	ClipRect( &icon_rect );
	DrawGrowIcon( thePort );
	SetClip( save_clip );
	DisposeRgn( save_clip );
}


/* ----------------- HandleMouseDown ----------------- */

void HandleMouseDown( void )
{
	WindowPtr	whichWindow;
	int			thePart;
	long		menuChoice, windSize;
	long new_size;
	
	thePart = FindWindow( g_TheEvent.where, &whichWindow );
	switch (thePart)
	{
		case inMenuBar:
			menuChoice = MenuSelect( g_TheEvent.where );
			HandleMenuChoice( menuChoice );
			break;
		case inSysWindow:
			SystemClick( &g_TheEvent, whichWindow );
			break;
		case inDrag:
			SelectWindow( whichWindow );
			DragWindow( whichWindow, g_TheEvent.where, &g_DragRect );
			break;
		case inContent:
			SelectWindow( whichWindow );
			SetPort( whichWindow );
			if (whichWindow == g_console_window)
				TD_click(g_TheEvent.where);
			else
				DragWindow( whichWindow, g_TheEvent.where, &g_DragRect );
			break;
		case inGrow:
			SelectWindow( whichWindow );
			new_size = GrowWindow( whichWindow, g_TheEvent.where,
				&g_DragRect );
			if (new_size)
			{
				SizeWindow( whichWindow, LoWord(new_size),
					HiWord(new_size), TRUE );
				SetPort( whichWindow );
				if (whichWindow != g_console_window)
					InvalRect( &whichWindow->portRect );
				else
					TD_resize();
			}
	}
}

/* -------------------- HandleMenuChoice ----------------------- */

void HandleMenuChoice( long menuChoice )
{
	int  theMenu, theItem;
	
	if ( menuChoice != 0 )
	{
		theMenu = HiWord( menuChoice );
		theItem = LoWord( menuChoice );
		switch( theMenu )
		{
			case APPLE_MENU_ID:
				HandleAppleChoice( theItem );
				break;
			case FILE_MENU_ID:
				HandleFileChoice( theItem );
				break;
			case kHMHelpMenuID:
				Do_help();
		}
	}
	HiliteMenu( 0 );
}

/* --------------------- Do_help ------------------------- */
void	Do_help( void )
{
	pascal void (*Show_help)( short help_id, 
		pascal Boolean (*Update_proc)(WindowPtr) );
	Handle	help_CUST;
	short		save_resfile;
	
	save_resfile = CurResFile();
	UseResFile( g_app_resfile );
	help_CUST = Get1NamedResource( 'CUST', "\pShow_help" );
	HLock( help_CUST );
	Show_help = (pascal void (*)(short, pascal Boolean (*Update_proc)(WindowPtr)))
				StripAddress( *help_CUST );
	(*Show_help)( HELP_ID, Do_update );
	UseResFile( save_resfile );
	HUnlock( help_CUST );
}


/* ------------------- HandleAppleChoice ------------------- */

void HandleAppleChoice( short theItem )
{
	Str255		accName;
	short			accNumber;
	
	switch( theItem )
	{
		case ABOUT_ITEM:
			show_about();
			break;
		default:
			if (g_help_under_apple && (theItem == APPLE_HELP_ITEM))
				Do_help();
			else
			{
				GetItem( g_AppleMenu, theItem, accName );
				accNumber = OpenDeskAcc( accName );
			}
			break;
	}
}

#define USE_JOB_DIALOG	1
#define NO_JOB_DIALOG	0
/* --------------------- HandleFileChoice ------------------- */
typedef enum {
	PAGE_SETUP_ITEM = 1,
	SETTINGS_ITEM,
	/* ---- */
	PRINT_ITEM = 4,
	JUST_PRINT_ITEM,
	/* ----- */
	QUIT_ITEM = 7
};

void
HandleFileChoice( theItem )
int	theItem;
{
	switch( theItem )
	{
		case PAGE_SETUP_ITEM:
			Do_page_setup();
			break;
		case SETTINGS_ITEM:
			Handle_settings();
			UnloadSeg( Handle_settings );
			break;
		case PRINT_ITEM:
			Handle_print( USE_JOB_DIALOG );
			break;
		case JUST_PRINT_ITEM:
			Handle_print( NO_JOB_DIALOG );
			break;
		case QUIT_ITEM:
			g_Done = TRUE;
			break;
	}
}


/* ---------------------- IsDAWindow ------------------------ */

Boolean	IsDAWindow( WindowPtr whichWindow )
{
	if ( whichWindow == NIL_POINTER )
		return( FALSE );
	else
		/* DA windows have negative WindowKinds  */
		return( ( ((WindowPeek)whichWindow) -> windowKind ) < 0 );
}


/* ------------------- Get_event ------------------ */
Boolean
Get_event( int event_mask, EventRecord *what_event )
{
	register  Boolean val;
	
	if ( g_WNEImplemented )
		val = WaitNextEvent( event_mask, what_event, MIN_SLEEP,
			NIL_MOUSE_REGION );
	else
	{
		SystemTask();
		val = GetNextEvent( event_mask, what_event );
	}
	return( val );
}


/* ----------------------- Init_menubar ------------------- */

void Init_menubar( void )
{
	int MBAR_id;
	Handle my_bar;
	int popup_id;
	
	MBAR_id = Get_resource_id( 'MBAR', "bar" );
	my_bar = GetNewMBar( MBAR_id );
	SetMenuBar( my_bar );
	g_AppleMenu = GetMHandle( APPLE_MENU_ID );
	g_FileMenu = GetMHandle( FILE_MENU_ID );
	AddResMenu( g_AppleMenu, 'DRVR' );
	Install_help();
	DrawMenuBar();
	
	popup_id = Get_resource_id( 'MENU', "popup" );
	g_popup_menu = GetMenu( popup_id );
	InsertMenu( g_popup_menu, -1 );
}

/* --------------------- Handle_print --------------------- */
void
Handle_print( Boolean use_job_dialog )
{
	char	t_str[256], t_str2[256];
	char	dvi_file[256];
	char	option_string[256];
	char	*t_ptr;
	int		first_page, last_page;
	
	g_abort_dvi = FALSE;
	Get_file_path( dvi_file );
	if (strlen(dvi_file) == 0)		/* user chose "cancel" in SF dialog */
		return;
	
	/* Initialize defaults */
	init_per_dvi();
	
	Open_printer( use_job_dialog, &first_page, &last_page );
	if (g_abort_dvi)
	{
		InitCursor(); /* in case of beach ball */
		return;  /* Error or else user clicked Cancel */
	}
	
	/* Get info out of options dialog send to option() */
	if ( (first_page != 1) || (last_page != iPrPgMax) )
	{
		(void)sprintf( t_str, "%d", first_page );
		(void)sprintf( t_str2, "%d", last_page );
		(void) strcpy(option_string, "-o");
		(void) strcat( option_string, t_str );
		(void) strcat( option_string, ":" );
		(void) strcat( option_string, t_str2 );
		option( option_string );
	}
	rollbeachball();
	(void) PtoCstr( g_other_commands );
	if (strlen((char *)g_other_commands))
	{
		t_ptr = strtok( (char *)g_other_commands, " \t" );
		while( t_ptr != NULL )
		{
			option( t_ptr );
			t_ptr = strtok( NULL, " \t" );
		}
	}
	
	/* Send offset info from config to option() */
	t_ptr = *get_str_resource( "HOFFSET" );
	if (strlen(t_ptr))
	{
		(void)strcpy( option_string, "-x" );
		(void)strcat( option_string, t_ptr );
		option( option_string );
	}
	t_ptr = *get_str_resource( "VOFFSET" );
	if (strlen(t_ptr))
	{
		(void)strcpy( option_string, "-y" );
		(void)strcat( option_string, t_ptr );
		option( option_string );
	}
	
    if (npage == 0)		/* no page ranges given, make a large one */
    {
		page_begin[0] = 1;
		page_end[0] = 32767;	/* arbitrary large integer */
		page_step[0] = 1;
		npage = 1;
    }

	if (! g_abort_dvi)
	{
		SetPort( g_cancel_dialog );
		DisableItem( g_FileMenu, 0 );	/* disable entire File menu */
		DrawMenuBar();
		SetPort( (GrafPtr) g_print_port_p );
		dvifile( dvi_file );	/* go print it! */;
		SetPort( FrontWindow() );
		InitCursor();
		Update_memory_indicators();
		EnableItem( g_FileMenu, 0 );	/* enable entire File menu */
		DrawMenuBar();
	}
	else
		Close_printer();
}

/* ----------------- Init_print_rec() -------------------- */
void
Init_print_rec()
{
	short		save_resfile;
	
	save_resfile = CurResFile();
	UseResFile( g_pref_resfile );
	if (g_print_rec_h != NIL)
		ReleaseResource( (Handle)g_print_rec_h );
	g_print_rec_h = (THPrint) Get1Resource( 'Prec', 128 );
	if (g_print_rec_h == nil)
	{
		fatal("Init_print_rec: error getting Prec resource.\r"
			"Try deleting the DVIM72-Mac Prefs file.\r");
	}
	PrOpen();
	Reset_resolution();
	if (PrValidate( g_print_rec_h ))
		Update_PREC();
	PrClose();
	UseResFile( save_resfile );
}


/* ------------------ Do_page_setup ----------------------- */
void
Do_page_setup()
{
	Init_print_rec();
	PrOpen();
	if (PrStlDialog( g_print_rec_h ))
		Update_PREC();
	PrClose();
}

/* ----------------- Open_printer --------------------- */

void Open_printer( Boolean use_job_dialog,
	int *first_page, int *last_page )
{
	char *s_ptr;
	OSErr err;
	
	CHK("Open_printer");
	UnloadSeg( sprintf );	/* get rid of ANSI segment */
	UnloadSeg( special );	/* maybe we won't need it */
	Init_print_rec();
	CHK("Open_printer before PrOpen");
	PrOpen();
	g_print_status = 1;
	CHK("Open_printer after PrOpen");
	if (PrError() != noErr)
	{
		Show_error("Open_printer: PrOpen error.");
		g_abort_dvi = TRUE;
		Close_printer();
		return;
	}
	g_print_res_file = CurResFile();
	
	if (get_bool_resource("MAX RESOLUTION"))
		Maximize_resolution();
	
	if (use_job_dialog == USE_JOB_DIALOG)
	{
		err = Special_job_dialog( g_print_rec_h );
		if (err != noErr)
		{
			g_abort_dvi = TRUE;
			Close_printer();
			return;
		}
		Update_PREC();
	}
	else
	{
		(**g_print_rec_h).prJob.iFstPage = 1;
		(**g_print_rec_h).prJob.iLstPage = iPrPgMax;
		(**g_print_rec_h).prJob.iCopies = 1;
	}
	
	*first_page = (**g_print_rec_h).prJob.iFstPage;
	*last_page = (**g_print_rec_h).prJob.iLstPage;
	(**g_print_rec_h).prJob.iFstPage = 1;
	(**g_print_rec_h).prJob.iLstPage = iPrPgMax;
	
	if (PrValidate(g_print_rec_h))
		printf("PrValidate changed something!\n");

	UseResFile(g_print_res_file);
	rollbeachball();
	CHK("Open_printer before PrOpenDoc");
	g_print_port_p = PrOpenDoc( g_print_rec_h, nil, nil );
	CHK("Open_printer after PrOpenDoc");
	if ((PrError() != noErr) || (g_print_port_p == NIL))
	{
		Show_error("Open_printer: PrOpenDoc error.");
		g_abort_dvi = TRUE;
		return;
	}
	g_print_status = 2;
	g_printer_dpi = (**g_print_rec_h).prInfo.iVRes;
	g_paper_width = ((**g_print_rec_h).prInfo.rPage.right -
		(**g_print_rec_h).prInfo.rPage.left) * (long)g_dpi / g_printer_dpi;
	g_paper_length = ((**g_print_rec_h).prInfo.rPage.bottom -
		(**g_print_rec_h).prInfo.rPage.top) * (long)g_dpi / g_printer_dpi;
	if (!g_draw_offscreen)
		Set_pic_procs();
	
	if (g_draw_offscreen)
	{
		SetRect( &g_offscreen_GrafPort.portRect, 0, 0,
			g_paper_width, g_paper_length );
		g_offscreen_GrafPort.portBits.bounds = g_offscreen_GrafPort.portRect;
		g_offscreen_GrafPort.portBits.rowBytes = ((g_paper_width + 31) >> 5) << 2;
		g_offscreen_GrafPort.portBits.baseAddr =
			MALLOC( g_offscreen_GrafPort.portBits.rowBytes
				* (long)g_paper_length );
		if ( g_offscreen_GrafPort.portBits.baseAddr == NIL )
		{
			Show_error("Open_printer: Not enough memory for offscreen bitmap.");
			g_abort_dvi = TRUE;
			return;
		}
	}

	/* Set up the Cancel dialog */
	if ((**g_print_rec_h).prJob.bJDocLoop == bSpoolLoop)
		SetIText( cancel_dialog_text, "\pNow Spooling." );
	else
		SetIText( cancel_dialog_text, "\pNow Printing." );
	REVEAL_DIALOG( g_cancel_dialog );
	DrawDialog( g_cancel_dialog );
	
	Update_freemem();
	CHK("Open_printer end");
}

/* --------------------- User_wants_out -------------------- */
Boolean
User_wants_out( void )
{
	Printer_idle();
	return( g_abort_dvi );
}

/* ------------------- Printer_idle ---------------------- */
pascal void
Printer_idle(void )
{
	register  long		ticks;
	GrafPtr	save_port;
	
	GetPort( &save_port );
	rollbeachball();
	ticks = TickCount();
	if (Stack_space() < 5000L)
	{
		g_abort_dvi = TRUE;
		PrSetError( iPrAbort );
		printf( "\n[Sorry, getting low on stack space: %ld remains.]\n", Stack_space());
	}
	if (ticks - g_last_idle_time > IDLE_INTERVAL)
	{
		Update_memory_indicators();
		Handle_event();
		g_last_idle_time = ticks;
	}
	SetPort( save_port );
}

/* --------------------- Close_printer -------------------- */
/* Called by dvifile */
void
Close_printer()
{
	TPrStatus stat;
	
	UnloadSeg( sprintf );
	UnloadSeg( special );
	SetPort( FrontWindow() );
	if (g_print_status == 3)
	{
		PrClosePage( g_print_port_p );
		g_print_status = 2;
	}
	if (g_print_status >= 2)
	{
		PrCloseDoc( g_print_port_p ); 
		g_print_status = 1;
		if ( (PrError() != noErr) && (PrError() != iPrAbort) )
		{
			printf("\nPrCloseDoc error %d.\n", PrError());
			/* (void)fflush(stdout); */
		}
		else
			if ( ((**g_print_rec_h).prJob.bJDocLoop == bSpoolLoop)
				&& (!g_abort_dvi) )
			{
				SetIText( cancel_dialog_text, "\pNow Printing." );
				DrawDialog( g_cancel_dialog );
				(**g_print_rec_h).prJob.pIdleProc = (ProcPtr)Printer_idle;
				PrPicFile( g_print_rec_h, nil, nil, nil, &stat );
				if ( (PrError() != noErr) && (PrError() != iPrAbort) )
				{
					printf("\nPrPicFile error %d.\n", PrError());
					/* (void)fflush(stdout); */
				}
			}
		if (PrError() != noErr)
		{
			Explain_print_error( PrError() );
		}
	}
	if (g_print_status == 1)
	{
		PrClose(); 
		g_print_status = 0;
	}
	HideWindow( g_cancel_dialog );
	SetPort ( FrontWindow() );
	InitCursor();  /* in case of beach balls */
}

#define	VERSION_ITEM	1
#define	VERS_ID		1

typedef struct	vers {
	unsigned char version_number;
	unsigned char revision_number;
	unsigned char revision_stage;
	unsigned char build_number;
	int	language_integer;
	unsigned char string_data[];	/* two Pascal strings: abbreviated, GetInfo */
} vers, **vers_h;

/* ----------------------- show_about() ---------------------------- */
void
show_about()
{
	DialogPtr	about_dlog;
	short		type, hit;
	Handle	stat_h;
	Rect		box;
	vers_h	v_h;
	int	about_id = Get_resource_id('DLOG', "about box");
	
	about_dlog = GetNewDialog( about_id, nil, (WindowPtr)MOVE_TO_FRONT );
	GetDItem( about_dlog, VERSION_ITEM,  &type, &stat_h, &box );
	v_h = (vers_h) Get_resource_by_id('vers', VERS_ID);
	SetIText( stat_h, (**v_h).string_data );
	ShowWindow( about_dlog );
	TD_activate( FALSE );
	ModalDialog( nil, &hit );
	DisposDialog( about_dlog );
	ReleaseResource( (Handle)v_h );
} /* show_about */

/* ---------------------- Kill_dvi ------------------------- */
//  Use this routine when we have to abort the printing job, but need not
//  bail out of the program entirely.
void Kill_dvi( char *msg )
{
	sillyfree();
	g_abort_dvi = true;
	if (g_print_status > 0)
		PrSetError( iPrAbort );
	printf("Printing aborted--");
	printf("%s\n", msg);
	printf( "Current TeX page counters: [%s]\n",tctos());
	Show_error( msg );
}

/* ---------------------- Show_error ------------------------- */
void
Show_error( msg )
char *msg;
{
	char pmsg[256];
	
	(void) strncpy( pmsg, msg, (size_t) 255 );
	pmsg[255] = '\0';
	(void) CtoPstr( pmsg );
	ParamText( (StringPtr)pmsg, "\p", "\p", "\p" );
	TD_activate(FALSE);
	StopAlert( Get_resource_id('ALRT', "error message"), nil );
}

/* ---------------------- Show_warning ------------------------- */
void
Show_warning( msg )
char *msg;
{
	char pmsg[256];
	
	(void) strncpy( pmsg, msg, (size_t) 255 );
	pmsg[255] = '\0';
	(void) CtoPstr( pmsg );
	ParamText( (StringPtr)pmsg, "\p", "\p", "\p" );
	TD_activate(FALSE);
	CautionAlert( Get_resource_id('ALRT', "warning message"), nil );
}

/* ------------------------ Get_resource_id -------------------- */
int
Get_resource_id(
	OSType		resource_type,
	char		*resource_name)
{
	char rsrc_name[80], errmsg[150];
	Handle resource_handle;
	short		resource_id;

	(void) strcpy( rsrc_name, resource_name );
	(void) CtoPstr( rsrc_name );
	resource_handle = GetNamedResource( resource_type, (StringPtr)rsrc_name );
	if (resource_handle == nil)
	{
		SysBeep(1);
		sprintf( errmsg, "Get_resource_id error getting '%4.4s' '%s'",
			(char *)&resource_type, resource_name );
		fatal(errmsg);
	};
	GetResInfo( resource_handle, &resource_id, nil, nil );
	return( resource_id );
}

/* ------------------------ Get_resource ----------------------------- */
void
Get_resource( /* find a resource by name (in the preferences file)*/
	OSType	resource_type,		/* given type... */
	char		*resource_name,	/* and name... */
	Handle	*resource_handle,	/* it returns handle... */
	short		*resource_id		/* and id. */
	)
{
	char rsrc_name[255];
	char temp[5];
	int	save_resfile;

	(void) strcpy( rsrc_name, resource_name );
	(void) CtoPstr( rsrc_name );
	save_resfile = CurResFile();
	UseResFile( g_pref_resfile );
	*resource_handle = Get1NamedResource( resource_type, (StringPtr)rsrc_name );
	UseResFile( save_resfile );
	if (*resource_handle == nil)
	{
		(void) strncpy( temp, (char *)&resource_type, 4 );
		temp[4] = '\0';
		(void) sprintf( rsrc_name,
			"Couldn't find resource '%s' of type '%s'!\r"
			"Try deleting the DVIM72-Mac Prefs file.\r",
			resource_name, temp );
		fatal(rsrc_name);
	};
	GetResInfo( *resource_handle, resource_id, nil, nil );
} /* Get_resource */

/* --------------------- Get_resource_by_id ---------------------- */
Handle
Get_resource_by_id( /* find a resource by ID (in the application) */
	OSType	resource_type,		/* given type... */
	int		resource_id		/* and id. */
	)
{
	char rsrc_name[80];
	char temp[5];
	Handle	resource_handle;
	int	save_resfile;

	save_resfile = CurResFile();
	UseResFile( g_app_resfile );
	resource_handle = Get1Resource( resource_type, resource_id );
	UseResFile( save_resfile );
	if (resource_handle == nil)
	{
		(void) strncpy( temp, (char *)&resource_type, 4 );
		temp[4] = '\0';
		(void) sprintf( rsrc_name,
			"Couldn't find resource %d of type '%s'!",
			resource_id, temp );
		fatal(rsrc_name);
	};
	return( resource_handle );
} /* Get_resource_by_id */

/* ------------------------- init_toolbox --------------------------- */
void init_toolbox()
{
	long extra_stack;
	Ptr p;
	long	**stack_space;
	
	stack_space = (long **) Get1NamedResource( 'LONG', "\pstack space" );
	if (stack_space != NIL)
	{
		extra_stack = **stack_space - Stack_space();
		if (extra_stack > 0)
		{
			SetApplLimit( GetApplLimit() - extra_stack );
		}
		ReleaseResource( (Handle) stack_space );
	}
	MaxApplZone();
	MoreMasters(); MoreMasters(); MoreMasters();
	InitGraf( &thePort );
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
#if PROFILE
	InitProfile( 300, 300 );
#endif
} /* init_toolbox */

/* ---------------------- set_str_resource ------------------------- */
void
set_str_resource( char *rsrc_name, char *new_value )
{
	char	**r_handle;
	short 	rsrc_id, new_len, i;
	
	/* Get a handle to the resource. */
	Get_resource( 'Cstr', rsrc_name, &r_handle, &rsrc_id );
	
	new_len = strlen( new_value );
	
	/* Resize the handle. */
	if (new_len + 1 != GetHandleSize( r_handle ))
		SetHandleSize( r_handle, new_len + 1 );
	if (MemError() != noErr)
		Show_warning("set_str_resource: SetHandleSize error");
	
	/* Copy the new value to the new handle. */
	(void) strcpy( *r_handle, new_value );
	
	/* Update the resource file. */
	ChangedResource( r_handle );
	if (ResError() != noErr)
		Show_warning("set_str_resource: ChangedResource error");
	WriteResource( r_handle );
	if (ResError() != noErr)
		Show_warning("set_str_resource: WriteResource error");
	UpdateResFile( g_pref_resfile );
	(void) FlushVol( NIL, g_pref_vRefNum );
} /* set_str_resource */

/* ------------------------ Update_PREC ------------------------ */
void Update_PREC( void )
{
	ChangedResource( (Handle) g_print_rec_h );
	UpdateResFile( g_pref_resfile );
	(void) FlushVol( NIL, g_pref_vRefNum );
}

/* ---------------------- set_int_resource ------------------------- */
void
set_int_resource( char *rsrc_name, int new_value )
{
	int	**r_handle;
	short 	rsrc_id, new_len, i;
	short		save_resfile;

	save_resfile = CurResFile();
	UseResFile( g_pref_resfile );

	/* Get a handle to the resource. */
	Get_resource( 'int ', rsrc_name, (Handle *)&r_handle, &rsrc_id );
	
	/* Copy the new value to the new handle. */
	**r_handle = new_value;
	
	/* Update the resource file. */
	ChangedResource( (Handle)r_handle );
	if (ResError() != noErr)
		Show_warning("set_int_resource: ChangedResource error");
	WriteResource( (Handle)r_handle );
	if (ResError() != noErr)
		Show_warning("set_int_resource: WriteResource error");
	UpdateResFile( g_pref_resfile );
	(void) FlushVol( NIL, g_pref_vRefNum );
} /* set_int_resource */


/* ---------------------- get_bool_resource ----------------- */
Boolean
get_bool_resource( char *rsrc_name )
{
	Handle r_handle;
	short		rsrc_id;

	Get_resource( 'Bool', rsrc_name, &r_handle, &rsrc_id );
	return( **(Boolean **)r_handle );
} /* get_bool_resource */

/* ---------------------- get_int_resource ----------------- */
int
get_int_resource( char *rsrc_name )
{
	Handle r_handle;
	short		rsrc_id;
	
	Get_resource( 'int ', rsrc_name, &r_handle, &rsrc_id );
	return( **(int **)r_handle );
} /* get_int_resource */

/*----------------------- get_str_resource -------------------------*/
char **
get_str_resource( char *rsrc_name )
{
	Handle r_handle;
	short	rsrc_id;
	
	Get_resource( 'Cstr', rsrc_name, &r_handle, &rsrc_id );
	return( (char **) r_handle );
} /* get_str_resource */

/* ---------------------- Get_SF_place ----------------------- */
Point Get_SF_place( short rsrc_id )
{
	Point where;
	DialogTHndl sfget_h;
	int sfget_width, sfget_depth;

/* Figure out how to center the SFGetFile dialog on the screen.*/
	sfget_h = (DialogTHndl) GetResource( 'DLOG', rsrc_id );
	if (ResError())
		Show_warning("SFGet resource error 1.");
	LoadResource( (Handle)sfget_h );
	if (ResError())
		Show_warning("SFGet resource error 2.");
	sfget_width = (**sfget_h).boundsRect.right
		- (**sfget_h).boundsRect.left;
	sfget_depth = (**sfget_h).boundsRect.bottom
		- (**sfget_h).boundsRect.top;
	where.h = (g_screenrect.right
		- g_screenrect.left - sfget_width) / 2
		+ g_screenrect.left;
	where.v = (g_screenrect.bottom
		- g_screenrect.top - sfget_depth) / 3
		+ g_screenrect.top;
	return( where );
}

/* ------------------------- Get_file_path -------------------------*/
void
Get_file_path( char *thepath )
{
Point	where;
SFReply reply;
int errcode;

	where = Get_SF_place( getDlgID );	

/* Put up standard file dialog box for the user to pick a file.  Show
only files of type ODVI or with names ending in ".dvi" or ".DVI" */

	SFGetFile(where, (StringPtr)"\pfile:", (ProcPtr)Dvi_filter, -1, nil, 0L, &reply);
	
	if (reply.good)
	{
		(void) PtoCstr( reply.fName );
		(void) strcpy( thepath, (char *)reply.fName );
		errcode = SetVol( nil, reply.vRefNum );
		if (errcode != noErr)
			printf("SetVol error %d.\n", errcode );
	}
	else
	{
		(void)strcpy( thepath, "" );
	}
} /* Get_file_path */


void
init_per_dvi()			/* initialize global variables */
{
    register INT16 i;		/* loop index */

/***********************************************************************
    Set up masks such that rightones[k]  has 1-bits at the right end  of
    the word from k ..	 HOST_WORD_SIZE-1, where bits are numbered  from
    left (high-order) to right (lower-order), 0 .. HOST_WORD_SIZE-1.

    img_mask[k] has  a 1-bit  in  position k,  bits numbered  as  above.
    power[k] is  1  <<  k,  and gpower[k]  is  2**k-1  (i.e.  1-bits  in
    low-order k positions).  These 3 require only 32 entries each  since
    they deal with 32-bit words from the PK and GF font files.

    These are set  at run-time, rather  than compile time  to avoid  (a)
    problems with C  preprocessors which sometimes  do not handle  large
    constants correctly, and (b) host byte-order dependence.
***********************************************************************/


    debug_code = 0;

    npage = 0;
    topmargin = 1.0;			/* DVI driver standard default */
    leftmargin = 1.0;			/* DVI driver standard default */

    subfile[0] = '\0';

#if    VIRTUAL_FONTS
    virt_font = FALSE;
#endif

}
