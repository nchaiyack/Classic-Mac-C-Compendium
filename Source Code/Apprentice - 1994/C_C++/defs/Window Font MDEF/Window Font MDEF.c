/*
	Window Font MDEF by James W. Walker, Nov. 1991, updated April 1992.
	76367.2271@compuserve.com
	
	This MDEF uses the font and size from the front window,
	rather than the system font, but otherwise calls the
	standard MDEF to do most of the work.  It also redraws the
	triangular scrolling indicators, which otherwise can get scaled
	with an ugly result.
*/

#include "Draw_arrow.h"


#if ASSERTING
	#define		ASSERT(cond,mess)	if (!(cond)) DebugStr("\p" mess)
#else
	#define		ASSERT(cond,mess)
#endif

#if TRACE
	#define 	TM(what)	DebugStr( "\p" #what )
#else
	#define		TM(what)
#endif

typedef pascal void (*Menu_def) (short, MenuHandle, Rect *, Point, short *);


pascal void main(
	short		message,
	MenuHandle	theMenu,
	Rect		*menuRect,
	Point		hitPt,
	short		*whichItem
	);

/*  Low-memory globals */
short	SysFontFam	: 0xBA6;
short	SysFontSize	: 0xBA8;
short	LastSpExtra	: 0xB4C;
short	CurFMInput	: 0x988;

/* global variables (A4-relative addressing) */
Boolean		has_up_arrow, has_down_arrow;
short		line_height;

/* -------------------------------------------------------------------- */
pascal void main(
	short		message,
	MenuHandle	theMenu,
	Rect		*menuRect,
	Point		hitPt,
	short		*whichItem
	)
{
	Menu_def	old_menudef;
	Handle		standard_MDEF_rsrc;
	SignedByte	menu_state;
	GrafPtr		wmgr_port, port;
	short		save_size;
	short		save_sysfam;
	FontInfo	fi;
	long		save_A4;
	Boolean		had_up_arrow, had_down_arrow;
	
	RomMapInsert = -1;
	standard_MDEF_rsrc = GetResource( 'MDEF', 0 );
	ASSERT( standard_MDEF_rsrc != NIL, "No MDEF" );
	menu_state = HGetState( standard_MDEF_rsrc );
	HLock( standard_MDEF_rsrc );
	old_menudef = (Menu_def) StripAddress( *standard_MDEF_rsrc );

	GetPort( &wmgr_port );
	save_size = wmgr_port->txSize;
	save_sysfam = SysFontFam;
	
	port = FrontWindow();

	/*
		Since the standard MDEF calls TextFont(0), we can't just set
		the font of the current port (the Window Manager port).  We
		must set the system font instead. On the other hand the standard
		MDEF does not call TextSize(), so we can set the text size in
		the port.
	*/
	SysFontFam = port->txFont;
	LastSpExtra = -1;
	
	/*
		If we change the font but not the size, the Font Manager sometimes
		seems to need an extra kick in the butt to notice the font change.
		This seems to do it.  Witchcraft.
		
		Setting CurFMInput to -1 may also work, except that I have not
		seen any documentation on this global.
	*/
	TextSize( 3 );
	GetFontInfo( &fi );

	TextSize( port->txSize );

	old_menudef( message, theMenu, menuRect, hitPt, whichItem );

	asm {
		move.L		A4, save_A4
		LEA			main, A4		; set up access to global variables
	}

	HSetState( standard_MDEF_rsrc, menu_state );
	
	/*
		The purpose of this switch group is to fix a cosmetic problem
		with the scrolling triangles that sometimes appear at the top
		or bottom of the menu.  Normally these are drawn by the standard
		MDEF using a CopyMask call.  When a different font size is used,
		this bitmap gets scaled, with an ugly result.  So we erase and
		redraw the scrolling triangle.
	*/
	
	switch (message)
	{
		case mChooseMsg:
			TM(choose);
			had_up_arrow = has_up_arrow;
			has_up_arrow = (TopMenuItem < menuRect->top);
			had_down_arrow = has_down_arrow;
			has_down_arrow = (AtMenuBottom > menuRect->bottom);
			if ( !had_up_arrow && has_up_arrow )
			{
				Draw_arrow( menuRect, -1 );
			}
			if ( !had_down_arrow && has_down_arrow )
			{
				Draw_arrow( menuRect, 1 );
			}
			break;
		case mDrawMsg:
			// SetPort( port );
			GetFontInfo( &fi );
			// SetPort( wmgr_port );
			line_height = fi.ascent + fi.descent + fi.leading;
			has_up_arrow = TopMenuItem < menuRect->top;
			has_down_arrow = AtMenuBottom > menuRect->bottom;
			if (has_up_arrow)
			{
				Draw_arrow( menuRect, -1 );
			}
			if (has_down_arrow)
			{
				Draw_arrow( menuRect, 1 );
			}
			break;
	}

	/*
		Restore the font and size.
	*/
	SysFontFam = save_sysfam;
	LastSpExtra = -1;

	TextSize( 3 );			// again, kick the font manager
	GetFontInfo( &fi );

	TextSize( save_size );
	
	asm {
		move.L		save_A4, A4
	}
}