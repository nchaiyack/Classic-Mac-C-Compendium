#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define TD_COPYABLE		0	/* Allow selecting and copying of text. */

#include "TextDisplay.h"
#include "mac-specific.h"
char **
		get_str_resource( char *rsrc_name );

/* =====================================================================
	TextDisplay provides a window for the display of non-editable text.
The idea is similar to THINK C's console window, except that TextDisplay
windows are scrollable.  The console window also takes care of more stuff
behind your back, which can be good for quick and dirty projects but can
get in the way once you start to learn the toolbox.

Call TD_new once to create a TextDisplay window.  It returns a WindowPtr,
which you can use like any other WindowPtr.  The window it returns is
initially hidden, giving you a chance to move it or resize it before
showing it, but you must remember to show it with ShowWindow.

Call TD_activate on every activate/deactivate or suspend/resume event,
with a second parameter of TRUE or FALSE according to whether the window
is activating.  You don't need to check whether the event is for the
TextDisplay window or some other window, TD_activate checks that.  You
should do a SetPort to the window being activated/deactivated before
calling TD_activate, however.

For best results, also call TD_activate (with second parameter FALSE)
before showing a modal dialog or alert.

Call TD_update whenever you handle an update event.

Call TD_click when you receive a mousedown event in the content region of
the front window.  This is for scrolling and selection.

Call TD_resize after you grow or zoom a window.

Call TD_copy when "Copy" is selected from the Edit menu.

Finally, TD_printf actually puts text in the window.  Its first parameter
is a pointer to a TextDisplay window, and its other parameters are
those of printf.

In this sample, the TextDisplay window uses a custom WDEF for a movable,
resizable window without a title bar.  You could use an ordinary window,
but then you would have to be careful about how you draw the grow icon.
My custom WDEF has the unusual feature that the grow region is part of the
window frame, so it is always drawn automatically.
======================================================================= */

/* ------------------- global variables ------------------------------ */
short	g_logfile_refnum;
short	g_app_wdrefnum;	/* working directory ref num */

/* ----------------- Prototypes of private routines ------------------ */
#if !TD_COPYABLE
void	High_hook( void );	/* Prevents selection from being hilited */
#endif
static void	TD_adjust_scrollbar_max( TEHandle the_text,
									ControlHandle the_bar );
static void	Adjust_text( TEHandle the_text, ControlHandle the_bar );
static pascal void Scroll_text( ControlHandle what, int part_code );
static void	Handle_scroll( TEHandle the_text, int the_part, Point where,
			ControlHandle scrollbar );
static pascal	Boolean Auto_scroll( void );


/* ------------------------- TD_new ----------------------------- */
WindowPtr TD_new(
	int		window_id,		/* ID of WIND resource */
	int		max_text,		/* keep at most this many characters */
	int		min_text,		/* don't discard any text before this */
	int		text_size,		/* font size */
	OSErr	*status )		/* error code */
{
	WindowPtr	the_window;
	Handle		a_handle;
	Rect		dest_rect, view_rect;
	TEHandle	text_h;
	ControlHandle	the_bar;
	Ptr			window_storage;
	OSErr		err;
	char		**t_str;
	char		message[80];
	Str255		volname;
	
	
	/* Enough memory? */
	if (max_text > MaxBlock())
	{
		*status = memFullErr;
		return nil;
	}
	
	/* First, get a window. */
	a_handle = GetResource( 'WIND', window_id );
	if ( (ResError() != noErr) || (a_handle == nil) )
	{
		*status = ResError();
		return nil;
	}
	window_storage = NewPtr( sizeof(TD_record) );
	if (MemError() != noErr)
	{
		*status = MemError();
		return nil;		
	}
	the_window = GetNewWindow( window_id, window_storage, (WindowPtr)-1L );
	SetPort( the_window );
	TextSize( text_size );
	
	((TD_peek)the_window)->max_text = max_text;
	((TD_peek)the_window)->min_text = min_text;
	
	/* Get a TE record. */
	dest_rect = view_rect = the_window->portRect; /* Adjust later */
	((TD_peek)the_window)->text = text_h = TENew( &dest_rect, &view_rect );
#if TD_COPYABLE
	(**text_h).highHook = nil;
#else
	(**text_h).highHook = (ProcPtr)High_hook;
#endif
	
	/* Make a scroll bar. */
	the_bar = NewControl( the_window, &dest_rect, "\p", FALSE,
		0, 0, 0, scrollBarProc, 0L );
	g_console_window = the_window;
	TD_resize();
	
	TEAutoView( TRUE, text_h );	/* Permit auto-scrolling */
	SetClikLoop( (ProcPtr)Auto_scroll, text_h );
	ShowControl( the_bar );
	
	t_str = get_str_resource( "CONSOLE_LOG_FILE" );
	HLock( t_str );
	(void) CtoPstr( *t_str );
	(void)	GetVol( volname, &g_app_wdrefnum );
	err = FSDelete( (StringPtr)*t_str, g_app_wdrefnum );
	if ( (err != noErr) && (err != fnfErr) )
	{
		(void) sprintf(message, "Error %d deleting old console log file.", err);
		Show_error( message );
	}
	g_logfile_refnum = 0;
	err = Create( (StringPtr)*t_str, g_app_wdrefnum, 'dviM', 'TEXT' );
	if (err != noErr)
	{
		(void) sprintf(message, "Error %d creating console log file.", err);
		Show_error( message );
	}
	else
	{
		err = FSOpen( (StringPtr)*t_str, g_app_wdrefnum, &g_logfile_refnum );
		if (err != noErr)
		{
			(void) sprintf(message, "Error %d opening console log file.", err);
			Show_error( message );
		}
	}
	HUnlock( t_str );
	ReleaseResource( t_str );

	*status = noErr;
	return the_window;
}

/* --------------------------- TD_close_log --------------------------- */
void	TD_close_log( void )
{
	(void) FSClose( g_logfile_refnum );
	(void) FlushVol( nil, g_app_wdrefnum );
}


/* ------------------------ TD_printf --------------------------- */
void TD_printf( char *format, ... )
{
	va_list		arg_ptr;
	char		str[256];
	int			i;
	register int	amount_to_keep;
	register int	line_len;
	int				max_text;
	TEHandle		the_text;
	ControlHandle	the_bar;
	long			bytes;
	
	va_start( arg_ptr, format );
	i = vsprintf( str, format, arg_ptr ); /* Find string to display. */
	va_end( arg_ptr );
	for (i--; i >= 0; i--)	/* Convert newlines to carriage returns. */
		if (str[i] == '\n')
			str[i] = '\r';
	
	
	line_len = strlen(str);
	the_text = ((TD_peek)g_console_window)->text;
	the_bar = ((WindowPeek)g_console_window)->controlList;
	max_text = ((TD_peek)g_console_window)->max_text;
	
	/*
		Is there enough room in our text block? if not shift the text
		up in the block.
	*/
	if ( (**the_text).teLength > max_text - line_len )
	{
		amount_to_keep = ((TD_peek)g_console_window)->min_text;
		if ( amount_to_keep > (max_text - line_len) )
			amount_to_keep = max_text - line_len;
		TESetSelect( 0L, (long)((**the_text).teLength - amount_to_keep),
			the_text );
		TEDelete( the_text );
	}

	TESetSelect( (long)(**the_text).teLength, (long)(**the_text).teLength,
		the_text );
	TEInsert( str, (long)line_len, the_text );
	TEPinScroll( 0, -1000, the_text );
	TD_adjust_scrollbar_max( the_text, the_bar );
	SetCtlValue( the_bar, GetCtlMax( the_bar ) );
	
	if (g_logfile_refnum != 0)
	{
		bytes = line_len;
		(void) FSWrite( g_logfile_refnum, &bytes, (Ptr)str );
		(void) FlushVol( nil, g_app_wdrefnum );
	}
}

/* ---------------------- TD_resize ----------------------------------- */
void TD_resize( )
{
	register TEHandle	the_text;
	ControlHandle		the_bar;
	Rect				grow_rect;
	
	if ((WindowPtr)thePort == g_console_window)
	{
		the_text = ((TD_peek)g_console_window)->text;
		the_bar = ((WindowPeek)g_console_window)->controlList;
		
		grow_rect = (**the_bar).contrlRect;
		grow_rect.top = grow_rect.bottom;
		grow_rect.bottom += GROW_ICON_SIZE;
		InvalRect( &grow_rect );

		(**the_text).viewRect = thePort->portRect;
		(**the_text).viewRect.right -= GROW_ICON_SIZE;
		(**the_text).viewRect.bottom -= TEXT_MARGIN;
		(**the_text).viewRect.top += TEXT_MARGIN;
		(**the_text).destRect = (**the_text).viewRect;
		InsetRect( &(**the_text).destRect, TEXT_MARGIN, 0 );
		TECalText( the_text );
		TD_adjust_scrollbar_max( the_text, the_bar );
		Adjust_text( the_text, the_bar );
		
		MoveControl( the_bar,
			thePort->portRect.right - GROW_ICON_SIZE + 1, -1 );
		SizeControl( the_bar, GROW_ICON_SIZE,
			thePort->portRect.bottom - GROW_ICON_SIZE + 3 );
		grow_rect = (**the_bar).contrlRect;
		grow_rect.top = grow_rect.bottom;
		grow_rect.bottom += GROW_ICON_SIZE;
		InvalRect( &grow_rect );
	}
}

/* ---------------------- TD_activate ------------------------ */
void TD_activate( Boolean activate )
{
	Rect	scrollrect;
	ControlHandle	the_bar;
	int		old_hilite, new_hilite;
	TEHandle	the_text;
	
	if (thePort == g_console_window)
	{
		the_bar = ((WindowPeek)g_console_window)->controlList;
		the_text = ((TD_peek)g_console_window)->text;
		if (activate)
			TEActivate( the_text );
		else
			TEDeactivate( the_text );
		scrollrect = (**the_bar).contrlRect;
		old_hilite = (**the_bar).contrlHilite;
		new_hilite = activate? ACTIVE : INACTIVE;
		if (old_hilite != new_hilite)
		{
			HiliteControl( the_bar, new_hilite );
			ValidRect( &scrollrect );
		}
		/*	Under certain circumstances, this routine may be called to
			deactivate the window when it's already inactive.
			HiliteControl does not redraw a control if the hilite
			value doesn't change, so in that case we should not
			validate the scroll bar. 
		*/
	}
}

/* --------------------- TD_update ---------------------------- */
void TD_update( )
{
	register TEHandle	the_text;
	
	if ((WindowPtr)thePort == g_console_window)
	{
		the_text = ((TD_peek)g_console_window)->text;
		TEUpdate( &(**the_text).viewRect, the_text );
	}
}

/* -------------------- TD_click ------------------------------ */
void TD_click( Point where )
{
	Point local_point;
	int		the_part;
	ControlHandle	the_control;
	register TEHandle	the_text;
	
	if ((WindowPtr)thePort == g_console_window)
	{
		the_text = ((TD_peek)g_console_window)->text;
		local_point = where;
		GlobalToLocal( &local_point );
		the_part = FindControl( local_point, thePort, &the_control );
		if (the_part)
			Handle_scroll( the_text, the_part, local_point,
				((WindowPeek)g_console_window)->controlList );
		else
			TEClick( local_point, FALSE, the_text );
	}
}

#if TD_COPYABLE
/* ------------------------ TD_copy ------------------------------ */
void TD_copy( )
{
	TEHandle	the_text;
	OSErr		err;
	
	if ((WindowPtr)thePort == g_console_window)
	{
		the_text = ((TD_peek)g_console_window)->text;
		TECopy( the_text );
		err = ZeroScrap();
		if (err == noErr)
			err = TEToScrap();
	}
}

#else /* not TD_COPYABLE */
void foo( void );
/* ----------------------- High_hook ------------------------------ */
/* 
	The highHook routine is called by TextEdit to highlight the selection
	range.  In this case I want no highlighting.
*/
void foo(){
	asm {
		extern High_hook:
		ADDQ.L	#4,SP	/* Throw away the argument, the address of a Rect */
		RTS
	}
}
#endif

/* ------------------- TD_adjust_scrollbar_max -------------------------- */
void TD_adjust_scrollbar_max( 
	TEHandle the_text, ControlHandle the_bar )
{
	int		window_height;
	int 	lines_above;
	
	window_height = ( (**the_text).viewRect.bottom
		- (**the_text).viewRect.top ) / (**the_text).lineHeight;
	lines_above = (**the_text).nLines - window_height;
	if (lines_above <= 0)
	{
		HiliteControl( the_bar, INACTIVE );
		lines_above = 0;
	}
	else if ( g_console_window == FrontWindow() )
		HiliteControl( the_bar, ACTIVE );
	SetCtlMax( the_bar, lines_above );
}

/* ---------------------- Handle_scroll -------------------------------- */
static void Handle_scroll( TEHandle the_text, int the_part, Point where,
	ControlHandle the_bar )
{
	if (the_part == inThumb)
	{
		(void) TrackControl( the_bar, where, nil );
		Adjust_text( the_text, the_bar );
	}
	else
		(void) TrackControl( the_bar, where, (ProcPtr)Scroll_text );
}

/* ---------------------- Adjust_text ----------------------------- */
static void Adjust_text( TEHandle the_text, ControlHandle the_bar )
{
	int old_scroll, new_scroll;
	
	old_scroll = (**the_text).viewRect.top - (**the_text).destRect.top;
	new_scroll = GetCtlValue( the_bar ) * (**the_text).lineHeight;
	TEScroll( 0, old_scroll - new_scroll, the_text );
}

/* ------------------------ Scroll_text ----------------------------- */
static pascal void Scroll_text( ControlHandle the_bar, int part_code )
{
	int		delta, old_value;
	WindowPtr	the_display;
	TEHandle	the_text;
	
	if (part_code != 0)
	{
		the_display = (**the_bar).contrlOwner;
		the_text = ((TD_peek)the_display)->text;
		
		switch (part_code)
		{
			case inUpButton:
				delta = -1;
				break;
			case inDownButton:
				delta = 1;
				break;
			case inPageUp:
				delta = ( (**the_text).viewRect.top
					- (**the_text).viewRect.bottom )
					/ (**the_text).lineHeight + 1;
				break;
			case inPageDown:
				delta = ( (**the_text).viewRect.bottom
					- (**the_text).viewRect.top )
					/ (**the_text).lineHeight - 1;
				break;
		}
		old_value = GetCtlValue( the_bar );
		if ( ((delta < 0) && (old_value > 0)) ||
			((delta > 0) && (old_value < GetCtlMax(the_bar))) )
			SetCtlValue( the_bar, old_value + delta );
		Adjust_text( the_text, the_bar );
	}
}

/* ------------------------- Auto_scroll ----------------------------- */
static pascal Boolean Auto_scroll()
{
	Point	mouse_point;
	Rect	view_rect;
	RgnHandle	save_clip;
	WindowPtr	the_display;
	TEHandle	the_text;
	ControlHandle	the_bar;
	
	the_display = FrontWindow();
	the_text = ((TD_peek)the_display)->text;
	the_bar = ((WindowPeek)the_display)->controlList;
	
	save_clip = NewRgn();
	GetClip( save_clip );
	ClipRect( &the_display->portRect );
	GetMouse( &mouse_point );
	view_rect = (**the_text).viewRect;
	if (mouse_point.v < view_rect.top)
		Scroll_text( the_bar, inUpButton );
	else if (mouse_point.v > view_rect.bottom)
		Scroll_text( the_bar, inDownButton );
	SetClip( save_clip );
	DisposeRgn( save_clip );
	return TRUE;
}
