#define USE_PICTS	1	/* 1 if PICTs are to be displayed, 0 for text only */
#define ONE_RESOURCE 1	/* 1 for Get1Resource, 0 for GetResource, etc. */
#define COMPRESSION	1	/* compressed TEXT/styl or PICTs */
#define STANDALONE	1

#define CHECKPOINTS 0
#define ASSERTIONS	0

#define	PREFLIGHT_MEMORY	20000L

#if ASSERTIONS
	#define	ASSERT_SET_NIL(lvalue)	lvalue = NIL
#else
	#define	ASSERT_SET_NIL(lvalue)
#endif

#if STANDALONE
	#define		Show_help	main
#else
	void	main( void );
#endif

#include "Show_help.h"
#include "Show_help typedefs.h"

/*
	Show_help by James W. Walker, June 1991
	
	version 2.0, updated July 1992
	
	This code is freely usable.  If you want to show your gratitude,
	you could send me a free copy of whatever program you develop
	with it.
	
	e-mail:
		Internet			76367.2271@compuserve.com
		CIS					76367,2271
		America Online		JWWalker
	
	This code displays scrolling text in a dialog box.  The text comes
	from TEXT/styl resources, which can be created with ResEdit 2.1 or
	with an accompanying HyperCard stack.
	The text cannot be edited, but one can select text and copy it to
	the clipboard using command-C, or save it as a TeachText file.
	
	Pictures can be included in the text using the same scheme as
	TeachText: Each option-space character indicates where the top
	edge of a picture should go, and pictures are centered horizontally.
	Pictures come from consecutively-numbered PICT resources.  
	
	A popup menu can be used to jump to "bookmarks", which are indicated
	by tab characters at ends of lines.
	
	Prototype:
	
	pascal void Show_help( short help_info_id,
				pascal void (*Handle_update)( WindowPtr ) );
		
	TO DO: error recovery, support for modeless use.
*/



#ifndef NIL
	#define		NIL		0L
#endif

#if ONE_RESOURCE
	#define GetResource			Get1Resource
	#define CountResources		Count1Resources
	#define GetNamedResource	Get1NamedResource
#endif ONE_RESOURCE

#if ASSERTIONS
	#define		ASSERT(x,y)		if (!(x)) {DebugStr("\p" y);}
#else
	#define		ASSERT(x,y)
#endif ASSERTIONS

#if CHECKPOINTS
	#define CKPT(x)		DebugStr( "\p" x )
#else
	#define CKPT(x)
#endif CHECKPOINTS

enum {
	c_OK = 1,	/* OK button */
	c_help,		/* userItem for our help display */
	c_save,		/* Button to save as TeachText */
	c_menu		/* userItem for popup menu */
};

#define		SCROLLBAR_WIDTH	16
#define		TEXT_INSET		4

#define		INITIAL_HIGHLIGHTS		8



/* Prototypes of private routines
*/
static pascal Boolean Help_filter( DialogPtr dialog,
	EventRecord	*event, short *itemHit);
static pascal void  Text_userItem_proc( WindowPtr the_window, short item_num );
static pascal void  Menu_userItem_proc( WindowPtr the_window, short item_num );
static pascal void Scroll_text( ControlHandle the_bar, short part_code );
static pascal Auto_scroll( void );
static void Handle_scroll( DialogPtr dialog,
	short the_part, Point local_point );
static void Adjust_text( DialogPtr dialog );
static void Save_text( TEHandle the_text, StringPtr default_filename );
static void Topic_menu( DialogPtr dlog, MenuHandle help_popup );
static MenuHandle Build_popup( TEHandle	the_text, StringPtr default_menuname );
static short	Find_char(
			Handle	data_h,		// handle to a block of characters
			short	offset,		// initial offset within block
			char	what );		// the character we're looking for

#if COMPRESSION
	static Handle Get_compressed_resource( ResType	the_type, short the_ID );
	static void Release_compressed_resource( Handle rsrc_h );
	static void Dispose_compressed_data( Handle the_handle );
#else
	#define Get_compressed_resource			GetResource
	#define Release_compressed_resource		ReleaseResource
	#define	Dispose_compressed_data(x)
#endif

#if USE_PICTS
static void Find_pictures( DialogPtr dlog, short first_pict_id );
static void Draw_picts( WindowPtr the_window, Rect *update_rect );
static pascal void High_hook( Rect *high_rect );
static void Do_deferred_hilites( help_ptr  hptr, Rect *update_rect );
static void Push_highlight( high_info **hh, Rect *rect );
static Boolean Pop_highlight( high_info **hh, Rect *rect );
#endif

/* ------------------------- Show_help --------------------------------- */
pascal void Show_help( short info_id,
				pascal void (*Handle_update)( WindowPtr what_window ) )
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
	CursHandle	watch_cursor;
	Show_help_info	**the_info;
	StringPtr	default_menuname, default_filename;
	short		which_pict;
	Handle		save_mbar;
	MenuHandle	fake_apple;
	
	/*
		I'm too lazy to check for a memory error in every possible
		place, so I'll just make sure there is a reasonable amount
		available before starting.
	*/
	item_h = NewHandle( PREFLIGHT_MEMORY );
	if (item_h == NIL)
	{
		SysBeep(1);
		return;
	}
	else
		DisposHandle( item_h );

	the_info = (Show_help_info **) GetResource( 'Hlp?', info_id );
	ASSERT( the_info != NIL, "Hlp? resource missing" );
	HLock( (Handle) the_info );
	default_filename = (**the_info).strings;
	default_menuname = default_filename + default_filename[0] + 1;

	/*
		Get the dialog, which has extra info tacked on the end.
	*/
	dptr = (DialogPtr) NewPtr( sizeof(help_record) );
	ASSERT( dptr != NIL, "NewPtr failed for help_record" );
	dptr = GetNewDialog( (**the_info).DLOG_ID, (DialogPeek) dptr, (WindowPtr)-1L );
	ASSERT( dptr != NIL, "Failed GetNewDialog" );
	GetPort( &save_port );
	SetPort( dptr );

	((help_ptr) dptr)->Handle_update = Handle_update;
	
	watch_cursor = GetCursor( watchCursor );
	HLock( (Handle) watch_cursor );
	SetCursor( *watch_cursor );
	((help_ptr) dptr)->ibeam_cursor = GetCursor( iBeamCursor );
	HLock( (Handle) ((help_ptr) dptr)->ibeam_cursor );
	
#if USE_PICTS
	((help_ptr) dptr)->high = (high_info **) NewHandle(
				sizeof(high_info) + INITIAL_HIGHLIGHTS * sizeof(Rect) );
	ASSERT( ((help_ptr) dptr)->high != NIL, "Failed to get highlight record");
	(**((help_ptr) dptr)->high).array_size = INITIAL_HIGHLIGHTS;
	(**((help_ptr) dptr)->high).high_waiting = 0;
#endif

	help_TEXT = Get_compressed_resource( 'TEXT', (**the_info).TEXT_ID );
	if (help_TEXT == NIL)
	{
		ASSERT(false, "Failed to find help TEXT resource" );
		goto getout;
	}
	help_styl = (StScrpHandle)
		Get_compressed_resource( 'styl', (**the_info).styl_ID );
	if (help_styl == NIL)
	{
		DisposHandle( help_TEXT );
		ASSERT_SET_NIL( help_TEXT );
		ASSERT( false, "Failed to find styl resource" );
		goto getout;
	}
	HLock( help_TEXT );
	
	GetDItem( dptr, c_help, &itype, &item_h, &help_item_box );
	SetDItem( dptr, c_help, itype, (Handle) Text_userItem_proc, &help_item_box );
	view = help_item_box;
	InsetRect( &view, 1, 1 );
	view.right -= SCROLLBAR_WIDTH;
	dest = view;
	InsetRect( &dest, TEXT_INSET, 0 );
	the_text = TEStylNew( &dest, &view );
	ASSERT( the_text != NIL, "Failed TEStylNew." );
	
	TEStylInsert( *help_TEXT, GetHandleSize(help_TEXT),
		help_styl, the_text );
	TEActivate( the_text );
	Release_compressed_resource( (Handle) help_styl );
	ASSERT_SET_NIL( help_styl );
	Release_compressed_resource( help_TEXT );
	ASSERT_SET_NIL( help_TEXT );
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
	Find_pictures( dptr, (**the_info).first_PICT_ID );
#endif
	help_popup = Build_popup( the_text, default_menuname );

	TEAutoView( TRUE, the_text );	/* Permit auto-scrolling */
	CKPT( "Installing ClikLoop" );
	(**the_text).clikLoop = (ProcPtr)Auto_scroll;
#if USE_PICTS
	(**the_text).highHook = (ProcPtr) High_hook;
	((help_ptr)dptr)->high_defer_flag = false;
#endif

	GetDItem( dptr, c_menu, &itype, &item_h, &box );
	SetDItem( dptr, c_menu, itype, (Handle) Menu_userItem_proc, &box );
	
	((help_ptr)dptr)->save_clip = NewRgn();	/* Used in Draw_picts */
	ASSERT( ((help_ptr)dptr)->save_clip != NIL,
		"NewRgn failed for save_clip" );
	
	/*
		My Fake_ModalDialog does not disable the menu bar the way
		the real one does.  I want to disable everything except the
		system menus: help, keyboard, application.  After clearing
		the menu bar, I have to add one menu, or else I wouldn't get the
		system menus.
	*/
	save_mbar = GetMenuBar();
	ClearMenuBar();
	fake_apple = NewMenu(1,"\p\024");
	InsertMenu( fake_apple, 0 );
	DisableItem( fake_apple, 0 );
	DrawMenuBar();

	ShowWindow( dptr );
	InitCursor();

	do {
		Fake_ModalDialog( (ProcPtr) Help_filter, &ihit );
		if (ihit == c_save)
			Save_text( the_text, default_filename );
		else if (ihit == c_menu)
			Topic_menu( dptr, help_popup );
	} while (ihit != c_OK);
	
	SetMenuBar( save_mbar );
	DisposeMenu( fake_apple );
	DrawMenuBar();
	DisposHandle( save_mbar );

	DisposeRgn( ((help_ptr)dptr)->save_clip );
	ASSERT_SET_NIL( ((help_ptr)dptr)->save_clip );
	ReleaseResource( (Handle) the_info );
	
#if USE_PICTS
#if COMPRESSION
	for (which_pict = 0; which_pict < ((help_ptr)dptr)->pict_count; ++which_pict)
	{
		Dispose_compressed_data( (Handle)
			((help_ptr)dptr)->pict_data[which_pict].pict );
	}
#endif /* COMPRESSION */
	DisposPtr( (Ptr) ((help_ptr)dptr)->pict_data );
	ASSERT_SET_NIL( ((help_ptr)dptr)->pict_data );
	DisposHandle( (Handle) ((help_ptr) dptr)->high );
	ASSERT_SET_NIL( ((help_ptr) dptr)->high );
#endif /* USE_PICTS */

	TEDispose( the_text );
	ASSERT_SET_NIL( the_text );
getout:
	DisposDialog( dptr );
	DisposeMenu( help_popup );
	SetPort( save_port );
}

#if USE_PICTS
/*	----------------------------------------------------------------
	High_hook			The highHook routine documented in IM IV
						(I think) expects the address of a Rect on
						top of the stack, and the return address
	next on the stack.  This is opposite from the Pascal or C calling
	protocol, so we have to use assembly language to swap those items.

	This deferred highlighting scheme is used to ensure that highlighting
	will be done after any pictures have been drawn, not before.  To do
	otherwise can cause pictures to be incorrectly highlighted during
	auto-scrolling.  This error can be seen in TeachText.
	----------------------------------------------------------------
*/
static pascal void High_hook( Rect *the_rect )
{
	register help_ptr	front;
	
	asm {
		moveM.L		4(A6), A0/A1	; load Rect address, return address
		EXG.L		A0, A1			; swap
		moveM.L		A0/A1, 4(A6)	; store return address, Rect address
	}
	CKPT( "High_hook");
	front = (help_ptr) FrontWindow();
	if ( (front != NIL) &&
		(GetPtrSize((Ptr)front) == sizeof(help_record)) &&
		!EmptyRect( the_rect ) )
	{
		if (!front->high_defer_flag)
		{
			HiliteMode &= ~(1 << hiliteBit);
			InvertRect( the_rect );
		}
		else
		{
			Push_highlight( front->high, the_rect );
		}
	}
}

/*	---------------------------------------------------------------------
	Push_highlight			Push a deferred highlight on the stack.
							Previously, the stack was a fixed size, because
							there never seemed to be more than 3 deferred
	highlights at a time.  But then it turned out that under the Japanese
	system software, there could be 7 highlights, even with the same
	English text.  So now the stack can grow.
	---------------------------------------------------------------------
*/
static void Push_highlight( high_info **hh, Rect *rect )
{
	if ( (**hh).high_waiting >= (**hh).array_size )
	{
		SetHandleSize( (Handle)hh, sizeof(high_info) +
			(INITIAL_HIGHLIGHTS + (**hh).array_size) * sizeof(Rect) );
		ASSERT( MemError() == noErr, "Can't expand highlight array" );
		(**hh).array_size = ( GetHandleSize( (Handle)hh ) -
									sizeof(high_info) ) / sizeof(Rect);
	}
	if ( (**hh).high_waiting < (**hh).array_size )
	{
		(**hh).high_rect[ (**hh).high_waiting ] = *rect;
		(**hh).high_waiting++;
	}
}

/* --------------------------- Pop_highlight -------------------------- */
static Boolean Pop_highlight( high_info **hh, Rect *rect )
{
	if ( (**hh).high_waiting > 0 )
	{
		(**hh).high_waiting--;
		*rect = (**hh).high_rect[ (**hh).high_waiting ];
		return true;
	}
	else
		return false;
}

/* ---------------------- Do_deferred_hilites ---------------------- */
static void Do_deferred_hilites( help_ptr  hptr, Rect *update_rect )
{
	Rect	hilite;
	
	while (Pop_highlight( hptr->high, &hilite ))
	{
		if (SectRect( &hilite, update_rect, &hilite ))
		{
			HiliteMode &= ~(1 << hiliteBit);
			InvertRect( &hilite );
		}
	}
	hptr->high_defer_flag = false;
}
#endif USE_PICTS

/* --------------------------- Find_char -------------------------- */
/*
	Find a character within a handle.  In a previous version I did this
	with Munger().
	
	returns: the offset of the character, or -1 if not found.
*/
static short	Find_char(
			Handle	data_h,		// handle to a block of characters
			short	offset,		// initial offset within block
			char	what )		// the character we're looking for
{
	Ptr		text;
	short	text_size, scan;
	
	text_size = (short) GetHandleSize( data_h );
	text = *data_h;
	for (scan = offset; (text[scan] != what) && (scan < text_size); ++scan)
		;
	if (scan == text_size) // not found
		scan = -1;
	return scan;
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
	ASSERT( popup != NIL, "NewMenu failed" );

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
				BlockMove( default_menuname, menu_data, 256 );
			else
			{
				menu_data[0] = title_length; // note: <= 255
				BlockMove( &text[line_start], &menu_data[1], menu_data[0] );
			}
			/*
				AppendMenu recognizes meta-characters like slash,
				which is probably not what we want in this case.  So
				we use SetItem, which does not use meta-characters.
			*/
			AppendMenu( popup, "\p " );
			SetItem( popup, CountMItems(popup), menu_data );
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
	register short		i;
	register TEHandle	the_text;
	Handle			text_h;
	register short		offset;
	TextStyle		what_style;
	short			line_height, font_ascent;
	
	if (menu == NIL) return;
	InsertMenu( menu, -1 );
	GetDItem( dptr, c_menu, &itype, &item_h, &box );
	where.h = box.left;
	where.v = box.bottom;
	LocalToGlobal( &where );
	HiliteMode &= ~(1 << hiliteBit);
	InvertRect( &box );
	menu_return = PopUpMenuSelect( menu, where.v, where.h, 0 );
	HiliteMode &= ~(1 << hiliteBit);
	InvertRect( &box );
	if (HiWord(menu_return))	/* Something selected */
	{
		menu_choice = LoWord( menu_return );
		bar = ((help_ptr)dptr)->scrollbar;
		the_text = (TEHandle) GetWRefCon( dptr );
		text_h = (**the_text).hText;
		
		/* Find tab character number menu_choice */
		offset = -1L;
		for (i = 1; i <= menu_choice; ++i)
		{
			++offset; /* so we don't find the same thing twice */
			offset = Find_char( text_h, offset, '\t' );
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
static void Save_text( TEHandle the_text, StringPtr default_filename )
{
	Point		where;
	SFReply		reply;
	OSErr		err;
	short		data_refnum, res_refnum, old_resfile;
	Handle		text_data;
	SignedByte	state;
	long		count;
	GrafPtr		save_port;
	short		save_vol;
#if USE_PICTS
	register short		num_picts, pict_index;
	Handle		old_pict, new_pict;
	help_ptr	dptr;
#endif
	
	GetPort( &save_port );
	where.h = where.v = 100;
	SFPutFile( where, "\pName of TeachText file:",
		default_filename, NIL, &reply );
	SetPort( save_port );

	if (reply.good)
	{
		old_resfile = CurResFile();
#if USE_PICTS
		dptr = (help_ptr) (**the_text).inPort;
		num_picts = dptr->pict_count;
#endif

		/*
			The reason I use Create before FSDelete is that FSDelete
			uses the PMSP and Create doesn't.  See TN 101.
			
			One reason that I use the old File Manager calls instead of
			the newer "H" calls is that they involve more glue code
			which adds to the size of the code resource.
		*/
		err = Create( reply.fName, reply.vRefNum, 'ttxt', 'ttro' );
		if (err == dupFNErr)
		{
			(void) FSDelete( reply.fName, reply.vRefNum );
			err = Create( reply.fName, reply.vRefNum, 'ttxt', 'ttro' );
		}
		ASSERT( err == noErr, "\p error in Create" );
		
		(void) FSOpen( reply.fName, reply.vRefNum, &data_refnum );
		
		text_data = (**the_text).hText;
		state = HGetState( text_data );
		ASSERT( MemError() == noErr, "HGetState text_data error" );
		HLock( text_data );
		
		count = GetHandleSize( text_data );
		ASSERT( MemError() == noErr, "GetHandleSize text_data error" );
		err = FSWrite( data_refnum, &count, *text_data );
		ASSERT( err == noErr, "FSWrite error" );
		err = FSClose( data_refnum );
		ASSERT( err == noErr, "FSClose error" );
		HSetState( text_data, state );

#if USE_PICTS
		if (num_picts > 0)
		{
			(void) GetVol( NIL, &save_vol );
			(void) SetVol( NIL, reply.vRefNum );
			CreateResFile( reply.fName );
			ASSERT( ResError() == noErr, "\pCreateResFile error" );
			res_refnum = OpenResFile( reply.fName );
			ASSERT( ResError() == noErr, "\pOpenResFile error" );
			(void) SetVol( NIL, save_vol );
			
			for (pict_index = 0;
				pict_index < num_picts; ++pict_index )
			{
				UseResFile( old_resfile );
				old_pict = (Handle) dptr->pict_data[pict_index].pict;
				if (old_pict == NIL)
					break;
				new_pict = old_pict;
				err == HandToHand( &new_pict );
				ASSERT( err == noErr, "HandToHand error" );
				UseResFile( res_refnum );
				AddResource( new_pict, 'PICT',
					pict_index + 1000, "\p" );
				ASSERT( ResError() == noErr, "AddResource error" );
			}
			CloseResFile( res_refnum );
			(void) FlushVol( NIL, reply.vRefNum );
			UseResFile( old_resfile );
		}
#endif
	}
}


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
	Called by Adjust_text and Text_userItem_proc to draw pictures.
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
		GetClip( ((help_ptr) the_window)->save_clip );
		ClipRect( update_rect );
		DrawPicture( the_pict, &pict_loc );
		SetClip( ((help_ptr) the_window)->save_clip );
	}
}

#define		OPTION_SPACE_CHAR	0xCA

/* ---------------------- Find_pictures ---------------------------- */
static void Find_pictures( DialogPtr dlog, short first_pict_id )
{
	register TEHandle	the_text;
	Handle		text_h;
	SignedByte	text_state;
	register short		offset;
	short		num_picts;
	register short		which_pict;
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
	offset = 0;
	num_picts = 0;
	offset = Find_char( text_h, offset, OPTION_SPACE_CHAR );
	while ( offset >= 0 )
	{
		num_picts++;
		offset++;
		offset = Find_char( text_h, offset, OPTION_SPACE_CHAR );
	}
	
	/* Allocate storage for an array of picture bounds. */
	pict = (pict_info *) NewPtr( sizeof(pict_info) * num_picts );
	ASSERT( pict != NIL, "NewPtr failed for pict_data" );
	((help_ptr)dlog)->pict_data = pict;
	
	/*
		Initialize the picture info.  For each picture we record the
		picture handle and its rectangle, in unscrolled window
		coordinates.
	*/
	offset = 0;
	for (which_pict = 0; which_pict < num_picts; which_pict++)
	{
		pict[which_pict].pict = (PicHandle) Get_compressed_resource( 'PICT',
			first_pict_id + which_pict );
		if ( pict[which_pict].pict == NIL )
			break;
		offset = Find_char( text_h, offset, OPTION_SPACE_CHAR );
		place = TEGetPoint( offset, the_text );
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
	if (part_code != 0)
	{
		the_display = (**the_bar).contrlOwner;
		the_text = (TEHandle) GetWRefCon( the_display );
		view = (**the_text).viewRect;
		place.h = view.left + TEXT_INSET;
		
		switch (part_code)
		{
			case inUpButton:
				place.v = view.top;
				/*
					If we get the offset of the left edge of the top line,
					then subtract 1, we should have an offset belonging
					to the previous line.
				*/
				offset = TEGetOffset( place, the_text ) - 1;
				if (offset == -1) offset = 0;
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
			GetClip( ((help_ptr) the_display)->save_clip );
			ClipRect( &the_display->portRect );
			SetCtlValue( the_bar, old_value + delta );
			SetClip( ((help_ptr) the_display)->save_clip );
		}
		Adjust_text( the_display );
	}
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
	
	CKPT( "Adjust_text");
	the_text = (TEHandle) GetWRefCon( dialog );
	old_scroll = (**the_text).viewRect.top - (**the_text).destRect.top;
	scroll_down = old_scroll -
		GetCtlValue( ((help_ptr) dialog)->scrollbar );
	if (scroll_down == 0)
		return;
#if USE_PICTS
	((help_ptr) dialog)->high_defer_flag = true;
	//((help_ptr) dialog)->high_waiting = 0;
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
#define		RETURN_CHAR		0x0D
#define		TILDE_CHAR		0x7E
#define		ENTER_CHAR		0x03
#define		ESCAPE_CHAR		0x1B

static pascal Boolean Help_filter( DialogPtr dialog,
	EventRecord	*event, short *itemHit)
{
	Point	local_point;
	short	the_part;
	ControlHandle	the_control;
	short	charcode;
	register TEHandle	the_text;
	Rect	item_box;
	short	cursor;
	WindowPtr	which_window;
	Boolean	retval;
	RgnHandle	gray_rgn;
	Rect		gray_rect;
	
	retval = false; // usually, let the Dialog Mgr do further processing
	the_text = (TEHandle) GetWRefCon( dialog );

	GetMouse( &local_point );
	if (PtInRect( local_point, &(**the_text).viewRect ))
		SetCursor( *(((help_ptr) dialog)->ibeam_cursor) );
	else
		InitCursor();
	TEIdle( the_text );
	
	switch (event->what) {
		case nullEvent:
			break;
		case updateEvt:
			which_window = (WindowPtr) event->message;
			if ( (which_window != dialog) &&
				(((help_ptr)dialog)->Handle_update != NIL) )
			{
				(((help_ptr)dialog)->Handle_update)( which_window );
				retval = true;	// done with this event
			}
			break;
		case mouseDown:
			CKPT( "Help_filter mousedown");
			the_part = FindWindow( event->where, &which_window );
			if (the_part == inMenuBar)
			{
				/*
					Calling MenuSelect lets one use the help and
					application menus.
				*/
				(void) MenuSelect( event->where );
				retval = true;	// get rid of the beep
			}
			else if ( (which_window == dialog) && (the_part == inContent) )
			{
				local_point = event->where;
				GlobalToLocal( &local_point );
				the_part = FindControl( local_point, dialog, &the_control );
				if (the_part && ((**the_control).contrlMax > 1) )
				{
					Handle_scroll( dialog, the_part, local_point );	
					*itemHit = 2;
					retval = true;
					break;
				}
				else if (PtInRect( local_point, &(**the_text).viewRect ))
				{
					if (event->modifiers & shiftKey)
						TEClick( local_point, true, the_text );
					else
						TEClick( local_point, false, the_text );
					retval = true;
				}
			}
			else if ( (which_window == dialog) && (the_part == inDrag) )
			{
				gray_rgn = GetGrayRgn();
				gray_rect = (**gray_rgn).rgnBBox;
				DragWindow( dialog, event->where, &gray_rect );
				retval = true;	// no beep
			}
			break;
		case keyDown :	
		case autoKey :
			charcode = event->message & charCodeMask;
			/*
				There's no Cancel button, so we treat the OK button
				the same as a Cancel button.
			*/
			if ( (charcode == RETURN_CHAR) || (charcode == ENTER_CHAR) ||
				(charcode == TILDE_CHAR) || (charcode == ESCAPE_CHAR) ||
				((charcode == '.') && (event->modifiers & cmdKey)) )
			{
				*itemHit = c_OK;	/* OK */
				Flash_button( dialog, *itemHit );
				retval = TRUE;
			}
			else if ( (charcode == 'c') && (event->modifiers & cmdKey) )
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
				retval = true;
			}
			break;
	} /* end switch */
	
	return retval;
}


/* ---------------------- Text_userItem_proc ------------------------- */
static pascal void  Text_userItem_proc( WindowPtr the_window, short item_num )
{
	Handle		item_h;
	Rect		item_box;
	short		item_type;
	TEHandle	the_text;
		
	CKPT( "Text_userItem_proc");
	the_text = (TEHandle) GetWRefCon( the_window );
	item_box = (**the_text).viewRect;
#if USE_PICTS
	((help_ptr) the_window)->high_defer_flag = true;
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
}

/* ---------------------- Menu_userItem_proc ------------------------- */
static pascal void  Menu_userItem_proc( WindowPtr the_window, short item_num )
{
	Handle		item_h;
	Rect		item_box;
	short		item_type;
	
	CKPT( "Menu_UserItem_proc");
	
	/*
		Get the item's rectangle, and frame it.
	*/
	GetDItem( the_window, item_num, &item_type, &item_h, &item_box );
	InsetRect(&item_box, -1, -1);
	FrameRect( &item_box );
	
	/* Draw the drop-shadow */
	MoveTo( item_box.left + 2, item_box.bottom );
	LineTo( item_box.right, item_box.bottom );
	LineTo( item_box.right, item_box.top + 2 );
}

/* ---------------------- Flash_button ----------------------------- */
pascal void Flash_button( DialogPtr the_dialog, short item_number )
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

#if COMPRESSION
/* ---------------------- Get_compressed_resource ----------------------- */
static Handle Get_compressed_resource( ResType the_type, short the_ID )
{
	register Handle		CNVT_h;
	ParmInfo		info;
	CNVT_routine	Converter;
	
	info.srcHandle = GetResource( '4CMP', the_ID );
	CNVT_h = GetNamedResource( 'CNVT', "\p4CMPUncompress 4CMP" );
	if ( (info.srcHandle == NIL) ||	/* maybe there's an uncompressed one */
		(CNVT_h == NIL) )
	{
		info.dstHandle = GetResource( the_type, the_ID );
	}
	else	/* found a compressed resource */
	{
		info.srcType = '4CMP';

		HLock(CNVT_h);
		Converter = (CNVT_routine) StripAddress( *CNVT_h );
		CKPT("\pAbout to call the CNVT");
		(void) Converter( NIL, &info );
		CKPT("\pAfter the CNVT");
		HUnlock(CNVT_h);
		
		ReleaseResource( info.srcHandle );
	}
	return	info.dstHandle;
}

#define		mem_resource	0x20

/* ------------------------- Release_compressed_resource ------------- */
/*
	If it's a resource handle, release it, otherwise dispose of it.
*/
static void Release_compressed_resource( Handle rsrc_h )
{
	if ( HGetState( rsrc_h ) & mem_resource )
		ReleaseResource( rsrc_h );
	else
		DisposHandle( rsrc_h );
}

/*	-------------------------------------------------------------------
	Dispose_compressed_data		If the handle is not a resource, hence
								the result of decompression, then
								dispose of it.  Otherwise (the case of
								an actual resource handle) do nothing.
	This is the what I do with PICTs, because a PICT may be doing double
	duty, say as a dialog item, and then it should not be released.
	-------------------------------------------------------------------
*/
static void Dispose_compressed_data( Handle the_handle )
{
	if ( !(HGetState( the_handle ) & mem_resource) )
		DisposHandle( the_handle );
}
#endif COMPRESSION