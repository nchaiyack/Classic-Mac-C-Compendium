#define A4_PROJECT	0	/* Set to 1 for a code resource (DA, cdev, etc.) */
#define USE_PICTS	0	/* 1 if PICTs are to be displayed, 0 for text only */
#define ONE_RESOURCE 1	/* 1 for Get1Resource, 0 for GetResource, etc. */


/*
	Show_help by James W. Walker, June 1991
	This code is freely usable.
	
	e-mail:
		Internet			76367.2271@compuserve.com
		CIS					76367,2271
		America Online		JWWalker
	
	This code displays scrolling text in a dialog box.  The text comes
	from TEXT/styl resources, which can be created with ResEdit 2.1.
	Pictures can be included in the text using the same scheme as
	TeachText: Each option-space character indicates where the top
	edge of a picture should go, and pictures are centered horizontally.
	Pictures come from consecutively-numbered PICT resources.  The text
	cannot be edited, but one can select text and copy it to the
	clipboard using command-C, or save it as a TeachText file.
	
	The fact that our window is not resizeable has its advantages.  We
	can compute where the pictures go, once and for all, except to take
	vertical scrolling into account.
	
	A popup menu can be used to jump to "bookmarks", which are indicated
	by tab characters at ends of lines.
	
	Prototype:
	
	void Show_help( short help_dlog_id, short help_text_id,
		short base_pict_id, StringPtr default_filename,
		StringPtr default_menuname );
	
	help_dlog_id  is the resource ID of the DLOG resource.  The dialog
	              should have an OK button as item 1 and a userItem, to
	              display the text, in item 2.
	
	help_text_id is the resource ID of the TEXT and styl resources.
	
	base_pict_id is the resource ID of the first PICT resource.

	default_filename is the filename initially presented in the Save dialog
	              when the user saves the help text as TeachText.
	
	default_menuname is the menu item used for a line that consists of
	              a tab and nothing else.
*/


#ifndef NIL
#define		NIL		0L
#endif

#if A4_PROJECT
#include <SetUpA4.h>
#endif

#if ONE_RESOURCE
#define GetResource			Get1Resource
#define CountResources		Count1Resources
#endif

#define CHECKPOINTS 0
#define ASSERTIONS	0

#if ASSERTIONS
#define		ASSERT(x,y)		if (!(x)) {DebugStr("\p" y)}
#else
#define		ASSERT(x,y)
#endif

#if CHECKPOINTS
#define CKPT(x)		DebugStr( "\p" x );
#else
#define CKPT(x)
#endif

enum {
	c_OK = 1,	/* OK button */
	c_help,		/* userItem for our help display */
	c_save,		/* Button to save as TeachText */
	c_menu		/* userItem for popup menu */
};

#define		SCROLLBAR_WIDTH	16
#define		TEXT_INSET		4

typedef struct {
	Rect		bounds;
	PicHandle	pict;
} pict_info;

typedef struct {
	DialogRecord	dialog;
	ControlHandle	scrollbar;
#if USE_PICTS
	Boolean			high_defer_flag;
	Rect			high_rect[3];
	short			high_waiting;
	short			pict_count;	/* how many pictures */
	pict_info		*pict_data;	/* pointer to an array */
#endif
}	help_record, *help_ptr;

/* private global variables */
static	RgnHandle	save_clip;
static	CursHandle	watch_cursor, ibeam_cursor;

/* Prototypes of public routines */

#include "Show_help.h"

/* Prototypes of private routines
 */
static pascal Boolean Help_filter( DialogPtr dialog,
	EventRecord	*event, short *itemHit);
static pascal void  userItem_proc( WindowPtr the_window, short item_num );
static pascal void  Menu_userItem_proc( WindowPtr the_window, short item_num );
static pascal void Scroll_text( ControlHandle the_bar, short part_code );
static pascal Auto_scroll( void );
static void Handle_scroll( DialogPtr dialog,
	short the_part, Point local_point );
static void Adjust_text( DialogPtr dialog );
static void Save_text( TEHandle the_text, short base_pict_id,
	StringPtr default_filename );
static void Topic_menu( DialogPtr dlog, MenuHandle help_popup );
static MenuHandle Build_popup( TEHandle	the_text, StringPtr default_menuname );
static void Flash_button( DialogPtr the_dialog, short item_number );

#if USE_PICTS
static void Find_pictures( DialogPtr dlog, short first_pict_id );
static void Draw_picts( WindowPtr the_window, Rect *update_rect );
static pascal void High_hook( Rect *high_rect );
static void High_hook_glue( void );
static void Do_deferred_hilites( help_ptr  hptr, Rect *update_rect );
#endif


/* ------------------------- Show_help --------------------------------- */
void Show_help( short help_dlog_id, short help_text_id,
	short base_pict_id, StringPtr default_filename,
	StringPtr default_menuname )
{
	register	DialogPtr	dptr;
	register	TEHandle	the_text;
	short		itype, ihit;
	Handle		item_h;
	Rect		help_item_box, box;
	Handle			help_TEXT;
	StScrpHandle	help_styl;
	GrafPtr		save_port;
	Rect		dest, view;
	ControlHandle	the_bar;
	short		max_scroll, nLines;
	Point		place;
	MenuHandle	help_popup;
	
	watch_cursor = GetCursor( watchCursor );
	SetCursor( *watch_cursor );
	ibeam_cursor = GetCursor( iBeamCursor );
	dptr = (DialogPtr) NewPtr( sizeof(help_record) );
	dptr = GetNewDialog( help_dlog_id, (DialogPeek) dptr, (WindowPtr)-1L );
	ASSERT( dptr != NIL, "Failed GetNewDialog" );
	GetPort( &save_port );
	SetPort( dptr );
	
	help_TEXT = GetResource( 'TEXT', help_text_id );
	if (help_TEXT == NIL)
	{
		ASSERT(false, "Failed to find help TEXT resource" );
		SysBeep(1);
		goto getout;
	}
	help_styl = (StScrpHandle) GetResource( 'styl', help_text_id );
	if (help_styl == NIL)
	{
		DisposHandle( help_TEXT );
		ASSERT( false, "Failed to find styl resource" );
		SysBeep(1);
		goto getout;
	}
	HLock( help_TEXT );
	
	GetDItem( dptr, c_help, &itype, &item_h, &help_item_box );
	SetDItem( dptr, c_help, itype, (Handle) userItem_proc, &help_item_box );
	view = help_item_box;
	InsetRect( &view, 1, 1 );
	view.right -= SCROLLBAR_WIDTH;
	dest = view;
	InsetRect( &dest, TEXT_INSET, 0 );
	the_text = TEStylNew( &dest, &view );
	ASSERT( the_text != NIL, "Failed TEStylNew." );
	
	TEStylInsert( *help_TEXT, GetHandleSize(help_TEXT),
		help_styl, the_text );
	TECalText( the_text );	/* Maybe not needed, but can't hurt. */
	TEActivate( the_text );
	ReleaseResource( (Handle) help_styl );
	ReleaseResource( help_TEXT );
	nLines = (**the_text).nLines;
	SetWRefCon( dptr, (long)the_text );
	max_scroll = TEGetHeight( (long) nLines, 1L, the_text )
		- (view.bottom - view.top);
	
	help_item_box.left = help_item_box.right - SCROLLBAR_WIDTH;
	the_bar = NewControl( dptr, &help_item_box, "\p", true,
		0, 0, max_scroll, scrollBarProc, NIL );
	ASSERT( the_bar != NIL, "Failed NewControl for scroll bar." );
	((help_ptr) dptr)->scrollbar = the_bar;

#if USE_PICTS
	Find_pictures( dptr, base_pict_id );
#endif
	help_popup = Build_popup( the_text, default_menuname );

	TEAutoView( TRUE, the_text );	/* Permit auto-scrolling */
	CKPT( "Installing ClikLoop" );
	(**the_text).clikLoop = (ProcPtr)Auto_scroll;
#if USE_PICTS
	(**the_text).highHook = (ProcPtr) High_hook_glue;
	((help_ptr)dptr)->high_defer_flag = false;
#endif

	GetDItem( dptr, c_menu, &itype, &item_h, &box );
	SetDItem( dptr, c_menu, itype, (Handle) Menu_userItem_proc, &box );
	
	save_clip = NewRgn();	/* Used in Draw_picts */
	ShowWindow( dptr );
#if A4_PROJECT
	RememberA4();
#endif
	InitCursor();

	do {
		ModalDialog( (ProcPtr) Help_filter, &ihit );
		if (ihit == c_save)
			Save_text( the_text, base_pict_id, default_filename );
		else if (ihit == c_menu)
			Topic_menu( dptr, help_popup );
	} while (ihit != c_OK);
	
	
	DisposeRgn( save_clip );
#if USE_PICTS
	DisposPtr( (Ptr) ((help_ptr)dptr)->pict_data );
#endif
	TEDispose( the_text );
getout:
	DisposDialog( dptr );
	DisposeMenu( help_popup );
	SetPort( save_port );
}

/* --------------------------- Build_popup ------------------------- */
/*
	Build a popup menu of the sections of the help text.  We scan for
	tab characters.  The text between the tab character and the preceding
	line break will be a menu item, unless it is the null string; then we
	use the default menu name that was passed to Show_help.
*/
static MenuHandle Build_popup( TEHandle	the_text, StringPtr default_menuname )
{
	MenuHandle	popup;
	short		menu_id;
	SignedByte	text_state;
	Handle		text_h;	/* handle to just the text */
	Str255		menu_data;
	char		*text;	/* pointer to the help text */
	register short		scan, line_start;
	short		text_size, title_length;
	
	/* Find an unused menu ID */
	menu_id = 1300; /* no particular reason */
	while (GetMHandle(menu_id))
		++menu_id;
	
	popup = NewMenu( menu_id, "\p" );

	text_h = (**the_text).hText;
	text_state = HGetState( text_h );
	HLock( text_h );
	text = *text_h;
	text_size = (short) GetHandleSize( text_h );
	line_start = 0;
	for (scan = 0; scan < text_size; scan++ )
	{
		if (text[scan] == '\r')
		{
			line_start = scan + 1;
		}
		else if (text[scan] == '\t')
		{
			title_length = scan - line_start;
			if (title_length == 0)
				AppendMenu( popup, default_menuname );
			else if (title_length <= 255)
			{
				menu_data[0] = title_length;
				BlockMove( &text[line_start], &menu_data[1], title_length );
				AppendMenu( popup, menu_data );
			}
		}
	}
	
	HSetState( text_h, text_state );
	return popup;
}

/* ------------------------- Topic_menu ------------------------ */
/*
	This routine is called when the menu title is clicked.
	It pops up the menu and scrolls to the indicated tab character.
*/
static void Topic_menu( DialogPtr dptr, MenuHandle menu )
{
	short			menu_id;
	Handle			item_h;
	short			itype;
	Rect			box;
	Point			where;
	long			menu_return;
	short			menu_choice;
	ControlHandle	bar;
	char			tab_char[2] = "\t";
	register short		i;
	register TEHandle	the_text;
	Handle			text_h;
	SignedByte		text_state;
	register long		offset;
	TextStyle		what_style;
	short			line_height, font_ascent;
	
	if (menu == NIL) return;
	InsertMenu( menu, -1 );
	GetDItem( dptr, c_menu, &itype, &item_h, &box );
	where.h = box.left;
	where.v = box.bottom;
	LocalToGlobal( &where );
	InvertRect( &box );
	menu_return = PopUpMenuSelect( menu, where.v, where.h, 0 );
	InvertRect( &box );
	if (HiWord(menu_return))	/* Something selected */
	{
		menu_choice = LoWord( menu_return );
		bar = ((help_ptr)dptr)->scrollbar;
		the_text = (TEHandle) GetWRefCon( dptr );
		text_h = (**the_text).hText;
		text_state = HGetState( text_h );
		HLock( text_h );
		
		/* Find tab character number menu_choice */
		offset = -1L;
		for (i = 1; i <= menu_choice; ++i)
		{
			++offset; /* so we don't find the same thing twice */
			offset = Munger(text_h, offset, tab_char, 1L, NIL, NIL);
		}

		where = TEGetPoint( (short)offset, the_text );
		TEGetStyle( (short)offset, &what_style, &line_height,
			&font_ascent, the_text );
		where.v -= line_height;	/* align to TOP of tab */
		/*
			Now where.v is in local coordinates.
		*/
		where.v -= (**the_text).destRect.top;
		SetCtlValue( bar,  where.v );
		HSetState( text_h, text_state );
		
		Adjust_text( dptr );
	}
	menu_id = (**menu).menuID;
	DeleteMenu( menu_id );
}

/* ------------------------- Save_text ------------------------ */
/*
	This is called when the user clicks on the "Save as TeachText"
	button.
*/
static void Save_text( TEHandle the_text, short base_pict_id,
	StringPtr default_filename )
{
	Point	where;
	static Str255	prompt = "\pName of TeachText file:";
	SFReply		reply;
	OSErr		err;
	short		data_refnum, res_refnum, old_resfile;
	Handle		text_data;
	SignedByte	state;
	long		count;
#if USE_PICTS
	register short		num_picts, pict_id;
	Handle		old_pict, new_pict;
#endif
	
	where.h = where.v = 100;
	SFPutFile( where, prompt, default_filename, NIL, &reply );
	if (reply.good)
	{
		old_resfile = CurResFile();
#if USE_PICTS
		num_picts = CountResources( 'PICT' );
#endif
		
		err = FSDelete( reply.fName, reply.vRefNum );
		err = Create( reply.fName, reply.vRefNum, 'ttxt', 'ttro' );
		err = FSOpen( reply.fName, reply.vRefNum, &data_refnum );
		text_data = (**the_text).hText;
		state = HGetState(text_data );
		HLock( text_data );
		count = GetHandleSize( text_data );
		err = FSWrite( data_refnum, &count, *text_data );
		err = FSClose( data_refnum );
		HSetState( text_data, state );

#if USE_PICTS
		if (num_picts > 0)
		{
			err = SetVol( NIL, reply.vRefNum );
			CreateResFile( reply.fName );
			err = ResError();
			res_refnum = OpenResFile( reply.fName );
			err = ResError();
			for (pict_id = base_pict_id;
				pict_id < base_pict_id + num_picts; ++pict_id )
			{
				UseResFile( old_resfile );
				old_pict = GetResource( 'PICT', pict_id );
				if (old_pict == NIL)
					break;
				new_pict = old_pict;
				err = HandToHand( &new_pict );
				UseResFile( res_refnum );
				AddResource( new_pict, 'PICT',
					pict_id - base_pict_id + 1000, "\p" );
				err = ResError();
			}
			CloseResFile( res_refnum );
			err = FlushVol( NIL, reply.vRefNum );
		}
#endif
	}
}

#if USE_PICTS
/* ------------------------- High_hook_glue ------------------------ */
static void High_hook_glue( void )
{
	asm {
		move.L	(SP)+, A0	; get address of rectangle
		movem.L	A2-A5/D3-D7, -(SP)	; save registers
		move.L	A0, -(SP)
	}
	CKPT( "High_hook_glue" );
	asm {
		JSR		High_hook
		movem.L	(SP)+, A2-A5/D3-D7	; restore registers
		RTS
	}
}

/* ------------------------- High_hook -------------------------- */
/*
	This deferred highlighting scheme is used to ensure that highlighting
	will be done after any pictures have been drawn, not before.  To do
	otherwise can cause pictures to be incorrectly highlighted during
	auto-scrolling.  This effect can be seen in TeachText.
*/
static pascal void High_hook( Rect *the_rect )
{
	register help_ptr	front;
	
	CKPT( "High_hook");
	front = (help_ptr) FrontWindow();
	if ( (front == NIL) ||
		(GetPtrSize((Ptr)front) != sizeof(help_record)) )
		return;
	if (!front->high_defer_flag)
		InvertRect( the_rect );
	else
	{
		if ( front->high_waiting >= 3 )
			DebugStr("\pHighlight overflow");
		else
		{
			front->high_rect[ front->high_waiting ]
				= *the_rect;
			front->high_waiting++;
		}
	}
}
#endif /* USE_PICTS */

/* ------------------------- Auto_scroll ----------------------------- */
/*
	This is a ClikLoop routine, called repeatedly by TEClick when the
	mouse is down.
*/
static pascal Auto_scroll()
{
	register	WindowPtr	the_display;
	register	ControlHandle	the_bar;
	Point					mouse_point;
	Rect					view_rect;
	register 	TEHandle	the_text;
	
	asm {
		movem.l		a1-a5/d1-d7, -(SP)
	}
	CKPT( "Auto_scroll");
	the_display = FrontWindow();
	if ( (the_display != NIL) &&
		(GetPtrSize((Ptr)the_display) == sizeof(help_record)) )
	{
		the_text = (TEHandle) GetWRefCon( the_display );
		the_bar = ((help_ptr) the_display)->scrollbar;
		
		GetMouse( &mouse_point );
		view_rect = (**the_text).viewRect;
		if (mouse_point.v < view_rect.top)
			Scroll_text( the_bar, inUpButton );
		else if (mouse_point.v > view_rect.bottom)
			Scroll_text( the_bar, inDownButton );
	}
	asm {
		movem.L		(SP)+, a1-a5/d1-d7
		moveQ		#1, D0
	}
}

#if USE_PICTS
/* ------------------------- Draw_picts --------------------------------- */
/*
	Called by Adjust_text and userItem_proc to draw pictures.
*/
static void Draw_picts( WindowPtr the_window, Rect *update_rect )
{
	register TEHandle	the_text;
	register short		pict_count, pict_index;
	PicHandle	the_pict;
	short 		v_offset;
	Rect		pict_loc, dummy;
	
	CKPT( "Draw_picts");
	the_text = (TEHandle) GetWRefCon( the_window );
	v_offset = (**the_text).destRect.top - (**the_text).viewRect.top
		- TEXT_INSET;
	pict_count = ((help_ptr) the_window)->pict_count;
	for (pict_index = 0; pict_index < pict_count; pict_index++)
	{
		pict_loc = ((help_ptr) the_window)->pict_data[pict_index].bounds;
		OffsetRect( &pict_loc, 0, v_offset );
		if (!SectRect( &pict_loc, update_rect, &dummy ))
			continue;
		the_pict = ((help_ptr) the_window)->pict_data[pict_index].pict;
		LoadResource( (Handle) the_pict );
		HLock( (Handle) the_pict );
		GetClip( save_clip );
		ClipRect( update_rect );
		DrawPicture( the_pict, &pict_loc );
		SetClip( save_clip );
		HUnlock( (Handle) the_pict );
	}
}

/* ---------------------- Find_pictures ---------------------------- */
static void Find_pictures( DialogPtr dlog, short first_pict_id )
{
	register TEHandle	the_text;
	Handle		text_h;
	SignedByte	text_state;
	register long		offset;
	long		text_size;
	short		num_picts;
	register short		which_pict;
	char		option_space[2] = "\xCA";
	pict_info	*pict;
	Point		place;
	short		line_height, font_ascent;
	TextStyle	what_style;
	
	CKPT( "Find_pictures");
	the_text = (TEHandle) GetWRefCon( dlog );
	text_h = (**the_text).hText;
	text_state = HGetState( text_h );
	HLock( text_h );
	
	/* Count option-space characters in the text. */
	text_size = GetHandleSize( text_h );
	offset = 0L;
	num_picts = 0;
	offset = Munger(text_h, offset, option_space, 1L, NIL, NIL );
	while ( (offset >= 0L) && (offset <= text_size) )
	{
		num_picts++;
		offset++;
		offset = Munger(text_h, offset, option_space, 1L, NIL, NIL );
		
	}
	
	/* Allocate storage for an array of picture bounds. */
	pict = (pict_info *) NewPtr( sizeof(pict_info) * num_picts );
	((help_ptr)dlog)->pict_data = pict;
	
	/*
		Initialize the picture info.  For each picture we record the
		picture handle and its rectangle, in unscrolled window
		coordinates.
	*/
	offset = 0L;
	for (which_pict = 0; which_pict < num_picts; which_pict++)
	{
		pict[which_pict].pict = (PicHandle) GetResource( 'PICT',
			first_pict_id + which_pict );
		if ( pict[which_pict].pict == NIL )
			break;
		offset = Munger(text_h, offset, option_space, 1L, NIL, NIL );
		place = TEGetPoint( (short)offset, the_text );
		TEGetStyle( offset, &what_style, &line_height,
			&font_ascent, the_text );
		place.v -= line_height;	/* align picture with TOP of option-space */
		offset++;
		pict[which_pict].bounds = (**pict[which_pict].pict).picFrame;
		OffsetRect( &pict[which_pict].bounds,
			( ((**the_text).destRect.right + (**the_text).destRect.left) -
			(pict[which_pict].bounds.right + pict[which_pict].bounds.left)
			) / 2,
			- pict[which_pict].bounds.top + place.v );
	}
	((help_ptr)dlog)->pict_count = which_pict;
	
getout:
	HSetState( text_h, text_state );
}
#endif /* USE_PICTS */

/* ---------------------- Scroll_text ---------------------------- */
/*
	This is used as a TrackControl actionProc for scrolling, and also
	called by Auto_scroll for automatic scrolling.
*/
static pascal void Scroll_text( ControlHandle the_bar, short part_code )
{
	register TEHandle	the_text;
	register short		delta;
	register WindowPtr	the_display;
	short				old_value;
	short				offset, line;
	Point				place;
	Rect				view;
	TextStyle			style;
	short				line_height, font_ascent;
	
	CKPT( "Scroll_text");
#if A4_PROJECT
	SetUpA4();
#endif
	if (part_code != 0)
	{
		the_display = (**the_bar).contrlOwner;
		the_text = (TEHandle) GetWRefCon( the_display );
		view = (**the_text).viewRect;
		place.h = view.left + TEXT_INSET;
		
		switch (part_code)
		{
			case inUpButton:
				place.v = view.top - 4;
				offset = TEGetOffset( place, the_text );
				place = TEGetPoint( offset, the_text );
				TEGetStyle( offset, &style, &line_height, &font_ascent,
					the_text );
				delta = place.v - line_height - view.top;
				break;
			case inDownButton:
				place.v = view.bottom + 2;
				offset = TEGetOffset( place, the_text );
				place = TEGetPoint( offset, the_text );
				/* Now place.v is at the baseline of the border line. */
				delta = place.v - view.bottom;
				break;
			case inPageUp:
				/*
					I want top border line to remain visible, and
					the top of a line should end up at view.top.
				*/
				place.v = view.top + 2;
				offset = TEGetOffset( place, the_text );
				place = TEGetPoint( offset, the_text );
				/* place.v is at the baseline of the top border line. */
				TEGetStyle( offset, &style, &line_height, &font_ascent,
					the_text );
				place.v += line_height - font_ascent;
				place.v -= view.bottom - view.top;
				offset = TEGetOffset( place, the_text );
				place = TEGetPoint( offset, the_text );
				TEGetStyle( offset, &style, &line_height, &font_ascent,
					the_text );
				delta = place.v - view.top;
				if (offset == 0)
					delta -= line_height;
				break;
			case inPageDown:
				/*
					I want bottom border line to remain visible, and
					the bottom of a line should end up at view.bottom.
				*/
				place.v = view.bottom - 2;
				offset = TEGetOffset( place, the_text );
				place = TEGetPoint( offset, the_text );
				/* place.v is at the baseline of the bottom border line. */
				TEGetStyle( offset, &style, &line_height, &font_ascent,
					the_text );
				place.v -= font_ascent; /* Top edge of bottom border line */
				place.v += view.bottom - view.top;
				/* We're looking at the bottom border of the next page. */
				offset = TEGetOffset( place, the_text );
				place = TEGetPoint( offset, the_text );
				TEGetStyle( offset, &style, &line_height, &font_ascent,
					the_text );
				delta =  place.v - line_height - view.bottom;
				if (offset == (**the_text).teLength)
					delta += line_height;
				break;
		}
		old_value = GetCtlValue( the_bar );
		if ( ((delta < 0) && (old_value > 0)) ||
			((delta > 0) && (old_value < GetCtlMax(the_bar))) )
		{
			/*
				When this routine is called, TextEdit may have set the
				clipping region to the view rectangle, so we reset it
				here to make sure the scroll bar gets drawn.
			*/
			GetClip( save_clip );
			ClipRect( &the_display->portRect );
			SetCtlValue( the_bar, old_value + delta );
			SetClip( save_clip );
		}
		Adjust_text( the_display );
	}
#if A4_PROJECT
	RestoreA4();
#endif
}

/* ---------------------- Adjust_text ---------------------------- */
/*
	Called by Handle_scroll and Scroll_text to scroll the text and
	pictures into sync with the scroll bar's control value.
*/
static void Adjust_text( DialogPtr	dialog )
{
	register	TEHandle	the_text;
	register	short	scroll_down;
	short			old_scroll;
	Rect			update_rect;
	ControlHandle	the_bar;
	
	CKPT( "Adjust_text");
	the_text = (TEHandle) GetWRefCon( dialog );
	the_bar = ((help_ptr) dialog)->scrollbar;
	old_scroll = (**the_text).viewRect.top - (**the_text).destRect.top;
	scroll_down = old_scroll - GetCtlValue( the_bar );
	if (scroll_down == 0)
		return;
#if USE_PICTS
	((help_ptr) dialog)->high_defer_flag = true;
	((help_ptr) dialog)->high_waiting = 0;
#endif
	TEScroll( 0, scroll_down, the_text );
#if USE_PICTS
	update_rect = (**the_text).viewRect;
	if (scroll_down > 0)
	{
		if (scroll_down < (update_rect.bottom - update_rect.top))
			update_rect.bottom = update_rect.top + scroll_down;
	}
	else
		if (- scroll_down < (update_rect.bottom - update_rect.top))
			update_rect.top = update_rect.bottom + scroll_down;
	Draw_picts( dialog, &update_rect );
	Do_deferred_hilites( (help_ptr) dialog, &update_rect );
#endif
}

#if USE_PICTS
/* ---------------------- Do_deferred_hilites ---------------------- */
static void Do_deferred_hilites( help_ptr  hptr, Rect *update_rect )
{
	Rect			*hilite;
	
	while (hptr->high_waiting > 0)
	{
		hptr->high_waiting--;
		hilite =
			&hptr->high_rect[hptr->high_waiting];
		if (SectRect( hilite, update_rect, hilite ))
			InvertRect( hilite );
	}
	hptr->high_defer_flag = false;
}
#endif

/* ---------------------- Handle_scroll ---------------------------- */
/*
	Called by Help_filter to handle mouseDown events in the scroll bar.
*/
static void Handle_scroll( DialogPtr dialog, short the_part, Point where )
{
	register	ControlHandle the_bar;
	
	CKPT( "Handle_scroll"); SetPort( dialog );
	the_bar = ((help_ptr) dialog)->scrollbar;
	if (the_part == inThumb)
	{
		(void) TrackControl( the_bar, where, NIL );
		Adjust_text( dialog );
	}
	else
		(void) TrackControl( the_bar, where, (ProcPtr)Scroll_text );
	
}

/* ---------------------- Help_filter ------------------------- */
/*
	This is the dialog event filter for our help window.
*/
#define RETURN_KEY	0x24
#define ENTER_KEY	0x4C
#define TILDE_KEY	0x32
#define ESCAPE_KEY	0x35


static pascal Boolean Help_filter( DialogPtr dialog,
	EventRecord	*event, short *itemHit)
{
	Point	local_point;
	short	the_part;
	ControlHandle	the_control;
	short	keycode, charcode;
	char	the_char;
	register TEHandle	the_text;
	Rect	item_box;
	short	cursor;
	
#if A4_PROJECT
	SetUpA4();
#endif
	the_text = (TEHandle) GetWRefCon( dialog );
	GetMouse( &local_point );
	if (PtInRect( local_point, &(**the_text).viewRect ))
		SetCursor( *ibeam_cursor );
	else
		InitCursor();
	TEIdle( the_text );
	switch (event->what) {
		case nullEvent:
			break;
		case mouseDown:
			CKPT( "Help_filter mousedown");
			local_point = event->where;
			GlobalToLocal( &local_point );
			the_part = FindControl( local_point, dialog, &the_control );
			if (the_part && ((**the_control).contrlMax > 1) )
			{
				Handle_scroll( dialog, the_part, local_point );	
				*itemHit = 2;
				break;
			}
			if (PtInRect( local_point, &(**the_text).viewRect ))
			{
				if (event->modifiers & shiftKey)
					TEClick( local_point, true, the_text );
				else
					TEClick( local_point, false, the_text );
			}
			break;
		case keyDown :	
		case autoKey :
			keycode = (event->message & keyCodeMask) >> 8;
			charcode = event->message & charCodeMask;
			/*
				There's no Cancel button, so we treat the OK button
				the same as a Cancel button.
			*/
			if ( (keycode == RETURN_KEY) || (keycode == ENTER_KEY) ||
				(keycode == TILDE_KEY) || (keycode == ESCAPE_KEY) ||
				((charcode == '.') && (event->modifiers & cmdKey)) )
			{
				*itemHit = c_OK;	/* OK */
				Flash_button( dialog, *itemHit );
#if A4_PROJECT
				RestoreA4();
#endif
				return( TRUE );
			}
			if ( (charcode == 'c') && (event->modifiers & cmdKey) )
			{
				(void) ZeroScrap();
				TECopy( the_text );
				SystemEdit(3);
				local_point = (**the_text).selPoint;
				*(long *)&local_point = PinRect( &(**the_text).viewRect,
					local_point );
				cursor = TEGetOffset( local_point, the_text );
				TESetSelect( (long)cursor, (long)cursor, the_text );
				event->what = nullEvent;
			}
			break;
	} /* end switch */
	
	/* tell the Dialog Manager that the event has NOT been handled and that it should
	** take further action on this event.
	*/
#if A4_PROJECT
	RestoreA4();
#endif
	return false;
}


/* ---------------------- userItem_proc ------------------------- */
static pascal void  userItem_proc( WindowPtr the_window, short item_num )
{
	Handle		item_h;
	Rect		item_box;
	short		item_type;
	TEHandle	the_text;
	
	asm {
		MOVEM.L	a1-a5/d0-d7, -(SP)
	}
	
	CKPT( "UserItem_proc");
#if A4_PROJECT
	SetUpA4();
#endif
	the_text = (TEHandle) GetWRefCon( the_window );
	item_box = (**the_text).viewRect;
#if USE_PICTS
	((help_ptr) the_window)->high_defer_flag = true;
	((help_ptr) the_window)->high_waiting = 0;
#endif
	TEUpdate( &item_box, the_text );
	
#if USE_PICTS
	Draw_picts( the_window, &item_box );
	Do_deferred_hilites( (help_ptr) the_window, &item_box );
#endif

	/*
		Get the item's rectangle, and frame it.
	*/
	GetDItem( the_window, item_num, &item_type, &item_h, &item_box );
	FrameRect( &item_box );

#if A4_PROJECT
	RestoreA4();
#endif
	asm {
		movem.l	(SP)+, a1-a5/d0-d7		; restore registers
	}
}

/* ---------------------- Menu_userItem_proc ------------------------- */
static pascal void  Menu_userItem_proc( WindowPtr the_window, short item_num )
{
	Handle		item_h;
	Rect		item_box;
	short		item_type;
	
	asm {
		MOVEM.L	a1-a5/d0-d7, -(SP)
	}
	
	CKPT( "Menu_UserItem_proc");
#if A4_PROJECT
	SetUpA4();
#endif
	
	/*
		Get the item's rectangle, and frame it.
	*/
	GetDItem( the_window, item_num, &item_type, &item_h, &item_box );
	InsetRect(&item_box, -1, -1);
	FrameRect( &item_box );
	
	/* Draw the drop-shadow */
	MoveTo( item_box.left + 3, item_box.bottom );
	LineTo( item_box.right, item_box.bottom );
	LineTo( item_box.right, item_box.top + 3 );		

#if A4_PROJECT
	RestoreA4();
#endif
	asm {
		movem.l	(SP)+, a1-a5/d0-d7		; restore registers
	}
}

/* ---------------------- Flash_button ----------------------------- */
static void Flash_button( DialogPtr the_dialog, short item_number )
{
	ControlHandle	item_h;
	long	time;
	short	itype;
	Rect	box;
	
	GetDItem( the_dialog, item_number, &itype, (Handle *)&item_h, &box );
	HiliteControl( item_h, inButton );
	Delay( 9L, &time );
	HiliteControl( item_h, 0 );
}
