#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <PrintTraps.h>
#include <MacProto.h>
#include "mac-specific.h"
#include "mac_printing.h"
#include "dvihead.h"
#include "gendefs.h"
#include "egblvars.h"
#include "m72.h"
#include "e_bitmap.h"
#include "TextDisplay.h"

#define PROFILE 0

#define REVEAL_DIALOG(d) {SelectWindow(d);ShowWindow(d);}
#define MOVE_TO_FRONT		-1L
#define MIN_SLEEP			3L
#define NIL_MOUSE_REGION	0L
#define NIL_POINTER			0L

#define WNE_TRAP_NUM		0x60
#define UNIMPL_TRAP_NUM		0x9F

#define APPLE_MENU_ID		1
#define FILE_MENU_ID		2
#define CONFIGURE_MENU_ID	3

#define ABOUT_ITEM		1

#define DRAG_THRESHOLD	30
#define IDLE_INTERVAL	10	/* Min. interval in ticks between printer idle stuff */

/* ------------- global variables ----------------- */
DialogPtr			cancel_dialog, configure_dialog;
Handle			cancel_dialog_text;
EventRecord		g_TheEvent;
Rect 				g_DragRect, g_screenrect;
MenuHandle		g_AppleMenu, g_FileMenu, g_ConfigureMenu;
Boolean			g_Done, g_WNEImplemented;
Boolean			g_abort_dvi; /* error cancels this dvi file */
int				g_dpi;	/* bitmap resolution */
int				g_printer_dpi;	/* device resolution */
int				g_print_res_file;
int				g_app_resfile;
GrafPort		g_offscreen_GrafPort;
WindowPtr		g_page_window,
				g_console_window;
THPrint			g_print_rec_h = NIL_POINTER;
Str255			g_other_commands;
WindowPtr		g_freemem_window;
MenuHandle		g_popup_menu;
Boolean			g_use_sysheap;
long				g_last_idle_time = 0L;

/* -------------- prototypes of private routines ------------ */
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
						int *resource_id);
void	Get_file_path( 	char *thepath );
Boolean	User_wants_out	( void );
void	init_per_dvi(	void );
pascal Boolean
		Dvi_filter( 	ParmBlkPtr param_block );
void	Get_dpi(		void );
Boolean Get_event(		int mask,
						EventRecord *what);
void	Get_dialogs(	void );
void	Init_offscreen(	void );
Point	Get_SF_place(	int rsrc_id );
void	Setup_drag_rect(	void );
void	Do_update(		WindowPtr  where );
void	Handle_print(	Boolean use_job_dialog );
void	Handle_event( void );
void	Do_page_setup(	void );
void	Init_print_rec(	void );
void	Set_draft_bits(	Boolean draft_bits );
void	Open_printer(	Boolean use_job_dialog,
						int *first,
						int *last );
void	HandleFileChoice( int theItem );
Boolean	get_bool_resource( char *rsrc_name );
int		get_int_resource( char *rsrc_name );
void		set_int_resource( char *rsrc_name, int new_value );
void	set_str_resource(
						char *rsrc_name,
						char *new_value );
char **
		get_str_resource( char *rsrc_name );

/* -------------------------------------------------------------------- */

/* ----- a couple of prototypes of external routines ------ */
void	HandleConfigureChoice( int theItem );
void	Do_configure	( void );
void	dvifile( char *filename );
void	option( char *an_option );
Boolean	setupbeachball();
void	rollbeachball();
pascal void	Printer_idle();
void	Reposition_windows( void );
void	Save_window_positions( void );
void	Center_window(	WindowPtr my_window );
void	Explain_print_error( OSErr the_error );
void	Update_freemem( void );
void	Update_memory_indicators();
void	openfont( char *fontname ); /* only used here for UnloadSeg */
void	special( char *s ); /* only used here for UnloadSeg */
ControlHandle	Get_Ditem_handle( DialogPtr theDialog, int item_no );
pascal void Get_PICT_data( Ptr data_ptr, int byte_count );

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
	Get_dialogs();
	Reposition_windows();
	printf( "\n\n\n\nWelcome to DVIM72-Mac.\n" );
	if (get_bool_resource("FREEMEM"))
		REVEAL_DIALOG( g_freemem_window );
	g_use_sysheap = get_bool_resource("sysheap");
	Get_dpi();
	Init_offscreen();
	(void) initglob();
	Clear_bitmap();
	REVEAL_DIALOG( g_page_window );
	Init_menubar();
	Init_print_rec();
	Update_freemem();
	InitCursor();
	(void) setupbeachball();
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
	g_DragRect = g_screenrect;
	InsetRect( &g_DragRect, DRAG_THRESHOLD, DRAG_THRESHOLD );
	g_DragRect.top -= DRAG_THRESHOLD;
}


/* ------------------------- Init_offscreen ---------------- */
void
Init_offscreen()
{
	GrafPtr save_port;
	char dpi_str[5];
	QDProcsPtr	my_procs;
	
	bitmap = (UNSIGN32*) NewPtr(
		((UNSIGN32)XBIT)*((UNSIGN32)YBIT)*sizeof(UNSIGN32));
	if (bitmap == (UNSIGN32*)NULL)
	{
		Show_error("Init_offscreen: cannot allocate enough"
			" space for the page image bitmap.  Reducing"
			" resolution to 72dpi.");
		g_dpi = 72;
		set_int_resource( "BITMAP DPI", g_dpi );
		bitmap = (UNSIGN32*) NewPtr(
			((UNSIGN32)XBIT)*((UNSIGN32)YBIT)*sizeof(UNSIGN32));
		if (bitmap == (UNSIGN32*)NULL)
		    (void)fatal(
				"Init_offscreen():  Cannot allocate space"
				" for page image bitmap");
	}
    
	GetPort( &save_port );
	OpenPort( &g_offscreen_GrafPort );
	g_offscreen_GrafPort.portBits.rowBytes = XWORDS * 4;
	SetRect( &g_offscreen_GrafPort.portBits.bounds,
		0, 0, XSIZE, YSIZE );
	g_offscreen_GrafPort.portBits.baseAddr = (Ptr) bitmap;
	g_offscreen_GrafPort.portRect =
		g_offscreen_GrafPort.portBits.bounds;
	/*
		The default visRgn is screenBits.bounds, which is
		too small for printing.
	*/
	DisposeRgn( g_offscreen_GrafPort.visRgn );
	g_offscreen_GrafPort.visRgn = g_offscreen_GrafPort.clipRgn;
	
	/* To support PICT \specials, we replace the getPicProc bottleneck routine. */
	my_procs = (QDProcsPtr) NewPtr( sizeof(QDProcs) );
	if (my_procs != 0)
	{
		SetStdProcs( my_procs );
		my_procs->getPicProc = (Ptr)&Get_PICT_data;
		g_offscreen_GrafPort.grafProcs = my_procs;
	}
	
	SetPort( save_port );
}

/* ------------------- Clear_bitmap ------------------------ */
void
Clear_bitmap()
{
	GrafPtr save_port;
	
	GetPort( &save_port );
	SetPort( &g_offscreen_GrafPort );
	EraseRect( &g_offscreen_GrafPort.portRect );
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
	cancel_dialog = GetNewDialog( cancel_DLOG_id,
		nil, (WindowPtr)MOVE_TO_FRONT );
	Center_window( cancel_dialog );
	cancel_dialog_text = (Handle)Get_Ditem_handle( cancel_dialog, 1 );
	
	WIND_id = Get_resource_id( 'WIND', "console" );
	g_console_window = TD_new( WIND_id, 5000,  4000, 9, &stat );
	if (stat != noErr)
		fatal("Error opening console window.");

	WIND_id = Get_resource_id( 'WIND', "FreeMem" );
	g_freemem_window = GetNewWindow( WIND_id,
		nil, (WindowPtr)MOVE_TO_FRONT );

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

	g_printer_dpi = get_int_resource("PRINTER DPI");
}


/* ------------------------ MainLoop ------------------- */

MainLoop()
{
	g_Done = FALSE;
	g_WNEImplemented = ( NGetTrapAddress(WNE_TRAP_NUM,ToolTrap) !=
		NGetTrapAddress(UNIMPL_TRAP_NUM, ToolTrap) );
	while ( g_Done == FALSE )
	{
		Handle_event();
	}
	/* fflush(stdout); */
}

#define	RESUME_MASK			1
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
			if ( (g_TheEvent.modifiers & cmdKey) != 0 )
			{
				if ( (theChar == '.') && ((WindowPeek)cancel_dialog)->visible )
				{
					g_abort_dvi = TRUE;
					PrSetError( iPrAbort );
					SetIText( cancel_dialog_text, "\pCancelling..." );
					DrawDialog( cancel_dialog );
				}
				else
					HandleMenuChoice( MenuKey( theChar ) );
			}
			else
				SysBeep(1);
			break;
		case updateEvt:
			Do_update( (WindowPtr)g_TheEvent.message );
			break;
		case activateEvt:
			SetPort( (WindowPtr)g_TheEvent.message );
			if (g_TheEvent.modifiers & activeFlag) /* activate event */
				TD_activate( TRUE );
			else
				TD_activate( FALSE );
			break;
		case app4Evt:
			SetPort( FrontWindow() );
			if (g_TheEvent.message & RESUME_MASK)
				TD_activate( TRUE );
			else
				TD_activate( FALSE );
			break;
	}
}

/* ------------------- Do_Update -------------------------- */
void
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
			from_rect = g_offscreen_GrafPort.portRect;
			if (g_printer_dpi < g_dpi)
			{
				from_rect.right = (int)((long)from_rect.right *
					(long)g_printer_dpi / g_dpi);
				from_rect.bottom = (int)((long)from_rect.bottom *
					(long)g_printer_dpi / g_dpi);
			}
			CopyBits( &g_offscreen_GrafPort.portBits,
				&g_page_window->portBits,
				&from_rect,
				&g_page_window->portRect,
				srcCopy, nil );
			DrawGrowIcon( where );
		}
		else if (where == g_freemem_window)
			Update_freemem();
		else if (where == cancel_dialog)
			DrawDialog( cancel_dialog );
		else if (where == g_console_window)
		{
			DrawControls( where );
			TD_update( );
		}
		EndUpdate( where );
		SetPort( save_port );
	}
}

/* ----------------- HandleMouseDown ----------------- */

HandleMouseDown()
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
				TD_resize();
				InvalRect( &whichWindow->portRect );
			}
	}
}

/* -------------------- HandleMenuChoice ----------------------- */

HandleMenuChoice( menuChoice )
long  menuChoice;
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
			case CONFIGURE_MENU_ID:
				HandleConfigureChoice( theItem );
				UnloadSeg( HandleConfigureChoice );
				break;
		}
		HiliteMenu( 0 );
	}
}

/* ------------------- HandleAppleChoice ------------------- */

HandleAppleChoice( theItem )
int  theItem;
{
	Str255		accName;
	int			accNumber;
	
	switch( theItem )
	{
		case ABOUT_ITEM:
			show_about();
			break;
		default:
			GetItem( g_AppleMenu, theItem, accName );
			accNumber = OpenDeskAcc( accName );
			break;
	}
}

#define USE_JOB_DIALOG	1
#define NO_JOB_DIALOG	0
/* --------------------- HandleFileChoice ------------------- */
typedef enum {
	PAGE_SETUP_ITEM = 1,
	/* ---- */
	PRINT_ITEM = 3,
	JUST_PRINT_ITEM,
	/* ----- */
	QUIT_ITEM = 6
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

IsDAWindow( whichWindow )
WindowPtr  whichWindow;
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

Init_menubar()
{
	int MBAR_id;
	Handle my_bar;
	int popup_id;
	
	MBAR_id = Get_resource_id( 'MBAR', "bar" );
	my_bar = GetNewMBar( MBAR_id );
	SetMenuBar( my_bar );
	g_AppleMenu = GetMHandle( APPLE_MENU_ID );
	g_FileMenu = GetMHandle( FILE_MENU_ID );
	g_ConfigureMenu = GetMHandle( CONFIGURE_MENU_ID );
	AddResMenu( g_AppleMenu, 'DRVR' );
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
		return;  /* Error or else user clicked Cancel */
	
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
	(void) PtoCstr( (char *)g_other_commands );
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
	if ( get_bool_resource( "REVPAGEORDER" ) )
		option( "-b" );
	
    if (npage == 0)		/* no page ranges given, make a large one */
    {
		page_begin[0] = 1;
		page_end[0] = 32767;	/* arbitrary large integer */
		page_step[0] = 1;
		npage = 1;
    }

	if (! g_abort_dvi)
	{
		DisableItem( g_FileMenu, 0 );	/* disable entire File menu */
		DisableItem( g_ConfigureMenu, 0 );	/* disable entire Configure menu */
		DrawMenuBar();
		dvifile( dvi_file );	/* go print it! */;
		InitCursor();
		Update_memory_indicators();
		EnableItem( g_FileMenu, 0 );	/* enable entire File menu */
		EnableItem( g_ConfigureMenu, 0 );	/* enable entire Configure menu */
		DrawMenuBar();
	}
}

/* ----------------- Init_print_rec() -------------------- */
void
Init_print_rec()
{
	TSetRslBlk set_resolution_block;
	Boolean	t_draft;
	
	g_print_rec_h = (THPrint) NewHandle( sizeof(TPrint) );
	if ( (g_print_rec_h == nil) || (MemError() != noErr) )
	{
		Show_error("Init_print_rec: NewHandle error.");
		alldone();
	}
	PrOpen();
	PrintDefault( g_print_rec_h );
	switch ( get_int_resource("PAPER FEED") )
	{
		case feedCut:
			(**g_print_rec_h).prStl.feed = feedCut;
			break;
		case feedFanfold:
			(**g_print_rec_h).prStl.feed = feedFanfold;
			break;
		default:
			break;	/* Use the driver's default */
	}
	switch ( get_int_resource("SPOOL") )
	{
		case bDraftLoop:
			(**g_print_rec_h).prJob.bJDocLoop = bDraftLoop;
			t_draft = TRUE;
			break;
		case bSpoolLoop:
			(**g_print_rec_h).prJob.bJDocLoop = bSpoolLoop;
			t_draft = FALSE;
			break;
		default:
			t_draft = FALSE;
			break;
	}
	Set_draft_bits( t_draft );

	set_resolution_block.iOpCode = setRslOp;
	set_resolution_block.hPrint = g_print_rec_h;
	set_resolution_block.iXRsl = g_printer_dpi;
	set_resolution_block.iYRsl = g_printer_dpi;
	PrGeneral( &set_resolution_block ); /* set device resolution */
	if ((PrError() != noErr) || set_resolution_block.iError)
	{
		Show_warning("Init_print_rec: PrGeneral resolution error.");
	}

	PrValidate( g_print_rec_h );
	PrClose();
}

/* ------------------ Set_draft_bits ---------------------- */
void
Set_draft_bits( Boolean draft_bits )
/* 
	Call PrGeneral to turn draft-mode bitmap printing on or off.
	The calling program should have called PrOpen.
*/
{
	TDftBitsBlk draft_bits_block;
	
	draft_bits_block.iOpCode = draft_bits?
		draftBitsOp : noDraftBitsOp;
	draft_bits_block.hPrint = g_print_rec_h;
	PrGeneral( &draft_bits_block );
}

/* ------------------ Do_page_setup ----------------------- */
void
Do_page_setup()
{
	PrOpen();
	PrStlDialog( g_print_rec_h );
	PrClose();
}

/* ----------------- Open_printer --------------------- */

void Open_printer( Boolean use_job_dialog,
	int *first_page, int *last_page )
{
	char *s_ptr;
	OSErr err;
	
	UnloadSeg( sprintf );	/* get rid of ANSI segment */
	UnloadSeg( special );	/* maybe we won't need it */
	PrOpen();
	if (PrError() != noErr)
	{
		Show_error("Open_printer: PrOpen error.");
		g_abort_dvi = TRUE;
		PrClose();
		return;
	}
	g_print_res_file = CurResFile();
	
	if (use_job_dialog == USE_JOB_DIALOG)
	{
		err = Special_job_dialog( g_print_rec_h );
		if (err != noErr)
		{
			g_abort_dvi = TRUE;
			PrClose();
			return;
		}
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
	g_print_port_p = PrOpenDoc( g_print_rec_h, nil, nil );
	if (PrError() != noErr)
	{
		Show_error("Open_printer: PrOpenDoc error.");
		g_abort_dvi = TRUE;
		return;
	}

	/* Set up the Cancel dialog */
	if ((**g_print_rec_h).prJob.bJDocLoop == bSpoolLoop)
		SetIText( cancel_dialog_text, "\pNow Spooling." );
	else
		SetIText( cancel_dialog_text, "\pNow Printing." );
	REVEAL_DIALOG( cancel_dialog );
	DrawDialog( cancel_dialog );
	
	Update_freemem();
}

/* -------------------- Print_page -------------------- */

void Print_page()
{
	GrafPtr	save_port_p;
	Rect		dest_rect; /* = {0, 0, 752, 576} at 72dpi */
	Rect		origin_rect;
	int i, strip_depth, page_top, page_bottom;
	long n;
	float		scale;
	int		int_scale;
	
	GetPort( &save_port_p );
	/* (void)fflush(stdout); */
	
	if (g_printer_dpi < g_dpi)	/* scale before printing */
	{
		scale = (float)g_dpi / (float)g_printer_dpi;
		int_scale = (int)(scale + 0.5);	/* round to nearest integer */
		if ( ABS(scale - (float)int_scale) < 0.1)	/* scale is nearly an integer */
		{
				SetRect( &dest_rect, 0, 0, 
					g_offscreen_GrafPort.portRect.right / int_scale,
					g_offscreen_GrafPort.portRect.bottom / int_scale );
				CopyBits( &g_offscreen_GrafPort.portBits,
					&g_offscreen_GrafPort.portBits,
					&g_offscreen_GrafPort.portRect, &dest_rect,
					srcCopy, nil );
		}
		else
		{
			SetRect( &dest_rect, 0, 0, 
 				(int)((float)g_offscreen_GrafPort.portRect.right / scale),
				(int)((float)g_offscreen_GrafPort.portRect.bottom / scale) );
			CopyBits( &g_offscreen_GrafPort.portBits, &g_offscreen_GrafPort.portBits,
				&g_offscreen_GrafPort.portRect, &dest_rect,
				srcCopy, nil );
		}
	}
	
	/* show contents of offscreen bitmap */
	SetPort( g_page_window );
	InvalRect( &g_page_window->portRect );
	
	SetPort( (GrafPtr) g_print_port_p );
	dest_rect.left = 0;
	dest_rect.right = g_offscreen_GrafPort.portRect.right;
	page_bottom = (**g_print_rec_h).prInfo.rPage.bottom;
	page_top = (**g_print_rec_h).prInfo.rPage.top;
	strip_depth = (**g_print_rec_h).prXInfo.iBandV;
	if (strip_depth < 8)
	{
		strip_depth = 3600;
	}
	if ((**g_print_rec_h).prJob.bJDocLoop == bSpoolLoop)
		copies = 1; /* Pr. Man. handles copies in spool mode */
	else
		copies = (**g_print_rec_h).prJob.iCopies;
	for (i = 0; i < copies; i++)
	{
		if (User_wants_out())
			break;
		PrOpenPage( g_print_port_p, nil );
		if (PrError() != noErr)
		{
			printf("PrOpenPage error %d.\n", PrError());
			/* (void)fflush(stdout); */
		}
		(**g_print_rec_h).prJob.pIdleProc = (ProcPtr)Printer_idle;
		if ((**g_print_rec_h).prStl.feed == feedCut) /* refresh cancel dialog */
		{
			DrawDialog( cancel_dialog );
			SetPort( (GrafPtr) g_print_port_p );
		}
		if ((**g_print_rec_h).prJob.bJDocLoop == bSpoolLoop)
			/* Tell the ImageWriter driver not to */
			PicComment(1000, 0, nil);	/* ..."thin the bits" */
		if (PrError() == noErr)
		{
			for (dest_rect.top = page_top;
				dest_rect.top < page_bottom;
				dest_rect.top += strip_depth)
			{
				dest_rect.bottom = MIN(dest_rect.top + strip_depth, page_bottom);
				rollbeachball();
				SetPort( (GrafPtr) g_print_port_p );
				CopyBits( &g_offscreen_GrafPort.portBits,
					&g_print_port_p->gPort.portBits,
					&dest_rect,
					&dest_rect,
					srcCopy, nil );
			}
		}
		if ((**g_print_rec_h).prJob.bJDocLoop == bSpoolLoop)
			PicComment(1001, 0, nil);
		PrClosePage( g_print_port_p );
		if (PrError() != noErr)
		{
			g_abort_dvi = TRUE;
			printf("PrClosePage error %d.\n", PrError());
			break;
		}
	}
	SetPort( save_port_p );
}

/* --------------------- User_wants_out -------------------- */
Boolean
User_wants_out()
{
	Printer_idle();
	return( g_abort_dvi );
}

/* ------------------- Printer_idle ---------------------- */
pascal void
Printer_idle()
{
	register  long		ticks;
	GrafPtr	save_port;
	
	GetPort( &save_port );
	rollbeachball();
	ticks = TickCount();
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
	PrCloseDoc( g_print_port_p );
	SetPort( FrontWindow() );
	if ( (PrError() != noErr) && (PrError() != iPrAbort) )
	{
		printf("\nPrCloseDoc error %d.\n", PrError());
		/* (void)fflush(stdout); */
	}
	else
		if ( ((**g_print_rec_h).prJob.bJDocLoop == bSpoolLoop)
			&& (PrError() != iPrAbort) )
		{
			SetIText( cancel_dialog_text, "\pNow Printing." );
			DrawDialog( cancel_dialog );
			(**g_print_rec_h).prJob.pIdleProc = (ProcPtr)Printer_idle;
			PrPicFile( g_print_rec_h, nil, nil, nil, &stat );
			if ( (PrError() != noErr) && (PrError() != iPrAbort) )
			{
				printf("\nPrPicFile error %d.\n", PrError());
				/* (void)fflush(stdout); */
			}
		}
	if (PrError() != noErr)
		Explain_print_error( PrError() );
	PrClose();
	HideWindow( cancel_dialog );
	SetPort ( FrontWindow() );
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
	int		type, hit;
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

/* ---------------------- Show_error ------------------------- */
void
Show_error( msg )
char *msg;
{
	char pmsg[256];
	
	(void) strncpy( pmsg, msg, (size_t) 255 );
	pmsg[255] = '\0';
	(void) CtoPstr( pmsg );
	ParamText( pmsg, "", "", "" );
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
	ParamText( pmsg, "", "", "" );
	TD_activate(FALSE);
	CautionAlert( Get_resource_id('ALRT', "warning message"), nil );
}

/* ------------------------ Get_resource_id -------------------- */
int
Get_resource_id(
	OSType		resource_type,
	char		*resource_name)
{
	char rsrc_name[80];
	Handle resource_handle;
	int resource_id;

	(void) strcpy( rsrc_name, resource_name );
	(void) CtoPstr( rsrc_name );
	resource_handle = GetNamedResource( resource_type, rsrc_name );
	if (resource_handle == nil)
	{
		SysBeep(1);
		alldone(0);
	};
	GetResInfo( resource_handle, &resource_id, nil, nil );
	return( resource_id );
}

/* ------------------------ Get_resource ----------------------------- */
void
Get_resource( /* find a resource by name */
	OSType	resource_type,		/* given type... */
	char		*resource_name,	/* and name... */
	Handle	*resource_handle,	/* it returns handle... */
	int		*resource_id		/* and id. */
	)
{
	char rsrc_name[80];
	char temp[5];
	int	save_resfile;

	(void) strcpy( rsrc_name, resource_name );
	(void) CtoPstr( rsrc_name );
	save_resfile = CurResFile();
	UseResFile( g_app_resfile );
	*resource_handle = Get1NamedResource( resource_type, rsrc_name );
	UseResFile( save_resfile );
	if (*resource_handle == nil)
	{
		(void) strncpy( temp, (char *)&resource_type, 4 );
		temp[4] = '\0';
		(void) sprintf( rsrc_name,
			"Couldn't find resource '%s' of type '%s'!",
			resource_name, temp );
		Show_error(rsrc_name);
		alldone();
	};
	GetResInfo( *resource_handle, resource_id, nil, nil );
} /* Get_resource */

/* --------------------- Get_resource_by_id ---------------------- */
Handle
Get_resource_by_id( /* find a resource by ID */
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
		Show_error(rsrc_name);
		alldone();
	};
	return( resource_handle );
} /* Get_resource_by_id */

/* ------------------------- init_toolbox --------------------------- */
void init_toolbox()
{
	long n;
	Ptr p;
	
	p = GetApplLimit();
	p -= 20000; /* extra stack space */
	SetApplLimit(p);
	n = MemError();
	MaxApplZone();
	MoreMasters(); MoreMasters(); MoreMasters();
	InitGraf( (Ptr)&thePort );
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
	int 	rsrc_id, new_len, i;

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
} /* set_str_resource */

/* ---------------------- set_int_resource ------------------------- */
void
set_int_resource( char *rsrc_name, int new_value )
{
	int	**r_handle;
	int 	rsrc_id, new_len, i;

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
} /* set_int_resource */


/* ---------------------- get_bool_resource ----------------- */
Boolean
get_bool_resource( char *rsrc_name )
{
	Handle r_handle;
	int	rsrc_id;

	Get_resource( 'Bool', rsrc_name, &r_handle, &rsrc_id );
	return( **(Boolean **)r_handle );
} /* get_bool_resource */

/* ---------------------- get_int_resource ----------------- */
int
get_int_resource( char *rsrc_name )
{
	Handle r_handle;
	int		rsrc_id;
	
	Get_resource( 'int ', rsrc_name, &r_handle, &rsrc_id );
	return( **(int **)r_handle );
} /* get_int_resource */

/*----------------------- get_str_resource -------------------------*/
char **
get_str_resource( char *rsrc_name )
{
	Handle r_handle;
	int	rsrc_id;
	
	Get_resource( 'Cstr', rsrc_name, &r_handle, &rsrc_id );
	return( (char **) r_handle );
} /* get_str_resource */

/* ---------------------- Get_SF_place ----------------------- */
Point
Get_SF_place( int rsrc_id )
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

	SFGetFile(where, "\pfile:", Dvi_filter, -1, nil, 0L, &reply);
	
	if (reply.good)
	{
		(void) PtoCstr( (char *)reply.fName );
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

/* ---------------------------- Dvi_filter --------------------------- */

pascal Boolean Dvi_filter( ParmBlkPtr param_block )
{
	static char suffix[5] = ".dvi";
	char *fname_ptr;
	char fname[64];
	int fname_len, i;
	
	if ( (*param_block).fileParam.ioFlFndrInfo.fdType == 'ODVI' )
		return( FALSE ); /* Display files of type ODVI */
	fname_ptr = (char *)(*param_block).fileParam.ioNamePtr;
	fname_len = *fname_ptr;
	if (fname_len < 4)
		return( TRUE ); /* can't end in ".dvi" with fewer than 4 bytes */
	for (i=0; i < fname_len; i++)
		fname[i] = *(++fname_ptr);
	fname[ fname_len ] = '\0';
	i = 3; /* strlen(suffix) - 1 */
	fname_len = fname_len - 1;
	while ( (i >= 0) && (tolower(fname[fname_len]) == suffix[i]) )
	{
		--fname_len;
		--i;
	};
	return( i >= 0 );
}


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
