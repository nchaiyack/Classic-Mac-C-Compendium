#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <PrintTraps.h>
#include "TextDisplay.h"
#include "dvihead.h"
#include "gendefs.h"
#include "egblvars.h"
#include "mac-specific.h"
#include "Copy_banded.h"

extern int			g_dpi;
extern WindowPtr	g_freemem_window;
extern THPrint		g_print_rec_h;
extern MenuHandle	g_popup_menu;
extern int			g_printer_dpi;	/* device resolution */

#define SFGET_ID	4045
#define REVEAL_DIALOG(d) {SelectWindow(d);ShowWindow(d);}
#define MOVE_TO_FRONT	-1L

Point			g_popup_corner;

/* --------------------- Prototypes --------------------------- */
void	Handle_settings( void );
pascal Boolean Dialog_filter( DialogPtr the_dialog,
							EventRecord *the_event,
							int			*item_number );

void	Update_freemem( void );

pascal
void	Draw_popup(		WindowPtr whatwindow,
							int whatitem );

void	Get_folder_pathname(
						char *PathNamePtr); /* put it here */
pascal int
		dialog_hook(	int item,
						DialogPtr dialog_ptr );
ControlHandle
		Get_Ditem_handle( DialogPtr theDialog,
						int item_no );
Handle	Get_resource_by_id( OSType rsrc_type, int r_id );
void	set_bool_resource( char *rsrc_name,
						Boolean new_value );
Boolean	get_bool_resource( char *rsrc_name );
char  **get_str_resource( char *rsrc_name );
void	set_str_resource(
						char *rsrc_name,
						char *new_value );
int		get_int_resource( char *rsrc_name );
void	set_int_resource(
						char *rsrc_name,
						int	 new_value );
void	Show_warning( 	char *msg );
void	Show_error( 	char *msg );
void	Tune_radio( 	DialogPtr what_dialog,
						int on_button, ...);
char	*get_dialog_text(
						DialogPtr thedialog,
						int item_num,
						char *thetext );
void	set_dialog_text(
						DialogPtr thedialog,
						int item_num,
						char *thetext );
void	Print_options(	void );
void	Set_resolution(	void );
void	Memory_options(	void );
void	Font_path(		void );
void	Init_print_rec(	void );
void	Center_window(	WindowPtr my_window );
void Select_folder( char *folder_path );


/* ------------------------- Handle_settings ---------------------------- */
enum {
	s_OK = 1,
	s_cancel,
	fp_button,
	fp_text,
	fp_popup,
	po_hoffset,
	po_voffset,
	sr_font_360,
	sr_font_144,
	sr_font_300,
	sr_font_other,
	sr_font_text,
	s_page_preview,
	s_draw_offscreen,
	s_max_printer_resolution,
	s_banding
};
void	Handle_settings( void )
{
	int			DLOG_id;
	DialogPtr	the_DLOG;
	char		*s_ptr;
	Boolean		dialog_done;
	short		item_hit;
	ControlHandle	font_text_h, cntrl;
	char		t_str[256];
	short		t_font_dpi, dpi_item;

	Boolean		t_pathflag, t_popupflag;
	Rect		popup_rect;
	Handle		popup_handle;
	short			item_type;
	char		nameformat[30];
	char		t_pathstr[256];
	short		menu_choice, mark_char;
	long		menulong;
	Point		where;
	SFReply		reply;
	int			i;
	Boolean		t_preview, t_draw_offscreen, t_max_res, t_banding;
		
	DLOG_id = Get_resource_id( 'DLOG', "settings" );
	the_DLOG = GetNewDialog( DLOG_id, nil, (WindowPtr)MOVE_TO_FRONT );
	Center_window( the_DLOG );

	/* Initialize TE blanks */
	s_ptr = *get_str_resource("HOFFSET");
	set_dialog_text( the_DLOG, po_hoffset, s_ptr );
	s_ptr = *get_str_resource("VOFFSET");
	set_dialog_text( the_DLOG, po_voffset, s_ptr );

	t_font_dpi = g_dpi;
	switch (t_font_dpi)
	{
		case 360:
			dpi_item = sr_font_360;  break;
		case 144:
			dpi_item = sr_font_144;  break;
		case 300:
			dpi_item = sr_font_300;  break;
		default:
			dpi_item = sr_font_other;  break;
	}
	Tune_radio( the_DLOG, dpi_item,
		sr_font_360, sr_font_144, sr_font_300, sr_font_other, 0 );
	font_text_h = Get_Ditem_handle( the_DLOG, sr_font_text );
	if (dpi_item == sr_font_other)
	{
		NumToString( (long)t_font_dpi, (StringPtr) t_str );
		SetIText( (Handle)font_text_h, (StringPtr) t_str );	
	}
	else
		SetIText( (Handle)font_text_h, "\p" );

	s_ptr = *get_str_resource("TEXFONTS");
	set_dialog_text( the_DLOG, fp_text, s_ptr );
	t_pathflag = FALSE;

	GetDItem( the_DLOG, fp_popup, &item_type,
		&popup_handle, &popup_rect );
	SetDItem( the_DLOG, fp_popup, item_type,
		(Handle)Draw_popup, &popup_rect );
	g_popup_corner.h = popup_rect.left + 2;
	g_popup_corner.v = popup_rect.bottom;
	SetPort( the_DLOG );
	LocalToGlobal( &g_popup_corner );

	/* font name format popup menu */
	(void) strcpy( nameformat, *get_str_resource("NAMEFORMAT") );
	for (i = 1; i <= strlen(nameformat); i++)
	{
		CheckItem( g_popup_menu, i, (nameformat[i-1] == '1') );
	}
	
	t_preview = g_preview;
	SetCtlValue( Get_Ditem_handle(the_DLOG, s_page_preview),
		t_preview );
	t_draw_offscreen = g_draw_offscreen;
	SetCtlValue( Get_Ditem_handle(the_DLOG, s_draw_offscreen),
		t_draw_offscreen );
	t_banding = g_print_by_bands;
	cntrl = Get_Ditem_handle( the_DLOG, s_banding);
	SetCtlValue( cntrl, t_banding );
	if (!t_draw_offscreen)
		HiliteControl( cntrl, 255 ); /* inactive */
	t_max_res = get_bool_resource("MAX RESOLUTION");
	SetCtlValue( Get_Ditem_handle(the_DLOG, s_max_printer_resolution),
		 t_max_res );

	dialog_done = FALSE;
	REVEAL_DIALOG( the_DLOG );
	TD_activate(FALSE);
	while( dialog_done == FALSE )
	{
		ModalDialog( (ProcPtr)Dialog_filter, &item_hit );
		switch( item_hit )
		{
			case s_OK:
				dialog_done = TRUE;
				break;
			case s_cancel:
				dialog_done = TRUE;
				break;
			case sr_font_360:
				t_font_dpi = 360;
				Tune_radio( the_DLOG, item_hit,
					sr_font_360, sr_font_144, sr_font_300, sr_font_other, 0 );
				break;
			case sr_font_144:
				t_font_dpi = 144;
				Tune_radio( the_DLOG, item_hit,
					sr_font_360, sr_font_144, sr_font_300, sr_font_other, 0 );
				break;
			case sr_font_300:
				t_font_dpi = 300;
				Tune_radio( the_DLOG, item_hit,
					sr_font_360, sr_font_144, sr_font_300, sr_font_other, 0 );
				break;
			case sr_font_other:
			case sr_font_text:
				t_font_dpi = -1; /* We'll look at the text later */
				if (item_hit == sr_font_other)
					SelIText( the_DLOG, sr_font_text, 0, 32767 );
				else
					item_hit = sr_font_other;
				Tune_radio( the_DLOG, item_hit,
					sr_font_360, sr_font_144, sr_font_300, sr_font_other, 0 );
				break;
			case fp_button:
				Select_folder( t_pathstr );
				if (t_pathstr[0] != 0)
				{
					t_pathflag = TRUE;
					set_dialog_text( the_DLOG,
						fp_text, t_pathstr );
				}
				break;
			case fp_popup:
				InvertRect( &popup_rect );
				menulong = PopUpMenuSelect( g_popup_menu,
					g_popup_corner.v, g_popup_corner.h, 0 );
				if ( LoWord(menulong) > 0 )
				{
					menu_choice = LoWord(menulong);
					GetItemMark( g_popup_menu, menu_choice, &mark_char );
					CheckItem( g_popup_menu, menu_choice,
						(mark_char == noMark) );
				}
				InvertRect( &popup_rect );
				break;
			case s_page_preview:
				t_preview = !t_preview;
				SetCtlValue( Get_Ditem_handle(the_DLOG, s_page_preview),
					t_preview );
				break;
			case s_draw_offscreen:
				t_draw_offscreen = !t_draw_offscreen;
				SetCtlValue( Get_Ditem_handle(the_DLOG, s_draw_offscreen),
					t_draw_offscreen );
				HiliteControl( Get_Ditem_handle(the_DLOG, s_banding),
					t_draw_offscreen? 0 : 255 );
				break;
			case s_banding:
				t_banding = !t_banding;
				SetCtlValue( Get_Ditem_handle(the_DLOG, s_banding),
					t_banding );
				break;
			case s_max_printer_resolution:
				t_max_res = !t_max_res;
				SetCtlValue( Get_Ditem_handle(the_DLOG, s_max_printer_resolution),
					 t_max_res );
				break;
		}
	}
	if (item_hit == s_OK)
	{
		set_str_resource( "HOFFSET",
			get_dialog_text( the_DLOG, po_hoffset, t_str ) );
		set_str_resource( "VOFFSET",
			get_dialog_text( the_DLOG, po_voffset, t_str ) );
		if (t_font_dpi < 0)	/* "other" */
		{
			get_dialog_text( the_DLOG, sr_font_text, t_str );
			(void) sscanf( t_str, "%d", &t_font_dpi );
		}
		if ( t_font_dpi != g_dpi ) /* font dpi changed? */
		{
			set_int_resource( "BITMAP DPI", t_font_dpi );
			g_dpi = t_font_dpi;
		}
		if (t_pathflag)
		{
			set_str_resource( "TEXFONTS", t_pathstr );
			(void)strcpy(subpath,t_pathstr); /* update globals derived */
			(void)strcpy(fontpath,t_pathstr); /* from TEXFONTS */
		}
		
		t_popupflag = FALSE;
		for (i = 1; i <= strlen(nameformat); i++)
		{
			GetItemMark( g_popup_menu, i, &mark_char );
			if ((nameformat[i-1] == '0') && (mark_char != noMark))
			{
				t_popupflag = TRUE;
				nameformat[i-1] = '1';

			};
			if ((nameformat[i-1] == '1') && (mark_char == noMark))
			{
				t_popupflag = TRUE;
				nameformat[i-1] = '0';
			}
		}
		if (t_popupflag)
			set_str_resource( "NAMEFORMAT", nameformat );
		if (t_preview != g_preview)
		{
			g_preview = t_preview;
			set_bool_resource( "PREVIEW", t_preview );
			if (g_preview)
				ShowWindow( g_page_window );
			else
				HideWindow( g_page_window );
		}
		if (t_draw_offscreen != g_draw_offscreen)
		{
			g_draw_offscreen = t_draw_offscreen;
			set_bool_resource( "DRAW OFFSCREEN", g_draw_offscreen );
		}
		if ( (t_banding != g_print_by_bands) && g_draw_offscreen )
		{
			g_print_by_bands = t_banding;
			set_bool_resource( "BANDING", g_print_by_bands );
		}
		if (t_max_res != get_bool_resource("MAX RESOLUTION"))
		{
			set_bool_resource( "MAX RESOLUTION", t_max_res );
		}
	}
	DisposDialog( the_DLOG );
}



/* ------------------------- Draw_popup ---------------------------- */
pascal void
Draw_popup( what_window, what_item )
  WindowPtr what_window;
  int what_item;
{
	Rect popup_rect;
	Handle item_h;
	short popup_type;
	
	GetDItem( what_window, what_item, &popup_type, &item_h,
		&popup_rect );
	FrameRect( &popup_rect );
	MoveTo( popup_rect.left + 3, popup_rect.bottom );
	LineTo( popup_rect.right, popup_rect.bottom );
	LineTo( popup_rect.right, popup_rect.top + 3 );
}

/* --------------------- Get_Ditem_handle -------------------------- */

ControlHandle
Get_Ditem_handle( DialogPtr theDialog, int item_no )
{
	short item_type;
	Rect box;
	Handle item_handle;
	
	GetDItem( theDialog, item_no, &item_type, &item_handle, &box );
	return( (ControlHandle) item_handle );
}

/* ----------------------- Tune_radio ----------------------- */
/* 
*	Switch one radio button on and some others off.
*	Input: a handle to the dialog, the item number of the button
*	to turn on, a list of item numbers for buttons to turn off,
*	and a zero to terminate the list.  The "on" button isn't turned
*	on until the end, so if the first item also occurs later on
*	the list, it will end up turned on.  Thus, to reset a radio
*	group, you can list the entire group after the first item.
*/
void
Tune_radio( DialogPtr what_dialog, int on_button, ...)
{
	va_list	arg_ptr;
	ControlHandle	button_h;
	int 	off_button;
	
	va_start( arg_ptr, on_button );
	while ( (off_button = va_arg(arg_ptr, int)) > 0 )
	{
		button_h = Get_Ditem_handle( what_dialog, off_button );
		SetCtlValue( button_h, (int)FALSE );
	}
	va_end( arg_ptr );
	button_h = Get_Ditem_handle( what_dialog, on_button );
	SetCtlValue( button_h, (int)TRUE );
}


/* ----------------------- get_dialog_text ------------------------- */
char *get_dialog_text(
		DialogPtr thedialog,
		int item_num,
		char *thetext )
{
	short item_type;
	Handle item_handle;
	Rect item_box;
	
	GetDItem( thedialog, item_num, &item_type, &item_handle, 
		&item_box );
	GetIText( item_handle, (StringPtr)thetext );
	(void)PtoCstr( (StringPtr) thetext );
	return( thetext );
} /* get_dialog_text */

/* ----------------------- set_dialog_text ------------------------- */
void
set_dialog_text(
		DialogPtr thedialog,
		int item_num,
		char *thetext )
{
	short item_type;
	Handle item_handle;
	Rect item_box;
	char t_str[255];
	
	strcpy( t_str, thetext );
	GetDItem( thedialog, item_num, &item_type, &item_handle, 
		&item_box );
	SetIText( item_handle, CtoPstr(t_str) );
} /* set_dialog_text */

/* --------------------- set_bool_resource ------------------ */
void
set_bool_resource( char *rsrc_name, Boolean new_value )
{
	Handle r_handle;
	char rsrc[100]; /* a copy of the string, since CtoPstr
					converts in place */

	strcpy( rsrc, rsrc_name );
	r_handle = GetNamedResource( 'Bool', CtoPstr(rsrc) );
	
	**(Boolean **)r_handle = new_value;
	
	/* Update the resource file. */
	ChangedResource( r_handle );
	if (ResError() != noErr)
		Show_error("set_bool_resource: ChangedResource error");
	WriteResource( r_handle );
	if (ResError() != noErr)
		Show_error("set_bool_resource: WriteResource error");
	UpdateResFile( g_pref_resfile );
	(void) FlushVol( NIL, g_pref_vRefNum );
}
