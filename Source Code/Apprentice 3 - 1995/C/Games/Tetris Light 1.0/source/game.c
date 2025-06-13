/**********************************************************************\

File:		game.c

Purpose:	This screen interface module for the Tetris program.  It is
			also where the playing field is stored.  This was placed
			here to simplify things if we want to port Tetris to 
			a character based platform, where we could just read and
			write characters to screen memory and use it to store the
			grid!
			

``Tetris Light'' - a simple implementation of a Tetris game.
Copyright (C) 1993 Hoylen Sue

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the
Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

\**********************************************************************/

#include "local.h"

#include <Sound.h>

#include "alert.h"
#include "controls.h"
#include "dialutil.h"
#include "env.h"
#include "game.h"
#include "highscore.h"
#include "pstring.h"
#include "resources.h"
#include "tetris.h"
#include "windows.h"

/*--------------------------------------------------------------------*/

/* Coordinates of where to draw things (in pixels) */

#define FIELD_TOP	3
#define FIELD_LEFT	3
#define CELL_SIZE	16		/* Size of cell */

#define SCORE_VALUE_LEFT	170
#define SCORE_VALUE_TOP		36

#define SCORE_TITLE_LEFT	170
#define SCORE_TITLE_TOP		20

#define GAME_OVER_X			170
#define GAME_OVER_Y			150

/* The following two are in cell units */

#define NEXT_PIECE_OFFSET_X	(NUMBER_COLS + 1)
#define NEXT_PIECE_OFFSET_Y	3

/*--------------------------------------------------------------------*/

/* This macro symbol, if defined, compiles the code to draw using
   primitive colours.  This will work on all Macs, but can be left
   out to increase drawing efficiency. */
   
#define COLOUR_GRAPHICS 1

/*--------------------------------------------------------------------*/

/* Drawing and sound resources */

static Handle hit_snd = 0;	/* if null, use SysBeep */
static Handle end_snd = 0;	/* if null, use SysBeep */

static unsigned char *score_str;
static unsigned char *game_over_str;

static unsigned char *pause_str;
static unsigned char *continue_str;

static Pattern cell_patterns[NUMBER_BLOCK_TYPES];

/*--------------------------------------------------------------------*/

/* The playing field and next block records */

static Rect field_rect = {
	FIELD_TOP, FIELD_LEFT, FIELD_TOP + CELL_SIZE * NUMBER_ROWS, 
	FIELD_LEFT + CELL_SIZE * NUMBER_COLS
};

static unsigned char field[NUMBER_COLS][NUMBER_ROWS];
static unsigned char next_block[MAX_BLOCK_SIZE][MAX_BLOCK_SIZE];

/*--------------------------------------------------------------------*/

static Boolean game_running;
static Boolean game_paused;
static Boolean game_window_active;	/* Records if game window is active */

static WindowPtr game_wind;
static ControlHandle game_pause_ctrlh;

static RgnHandle rgn1;
static RgnHandle rgn2;
static RgnHandle rgn_top_row;

/*--------------------------------------------------------------------*/

/* Game file based saving and restoring routines */

static Boolean saved_file_known = FALSE;
static Str255 saved_name;
static INTEGER saved_vref;

struct Save_header {
	unsigned long magic;
	struct Tetris_state state;
};

#define SAVE_MAGIC_NUMBER	('TLsv' | 0x80808080)

/* These locations are based on centering the dialog in the classic
   sized screen.  Dialog dimensions are based on those in Inside
   Macintosh I. */

static const Point SFGetFile_where = {82, 103};
static const Point SFPutFile_where = {104, 119};

/*--------------------------------------------------------------------*/

/* Local prototypes */

static void game_mouseDown(Point);
static void game_key(unsigned char code, unsigned char ascii);
static void game_update(void);
static void game_activate(void);
static void game_deactivate(void);

/*--------------------------------------------------------------------*/

/* Window dispatch table for game window */

static Wind_table game_dispatch_table = {
	game_mouseDown,
	game_key,
	game_update,
	game_activate,
	game_deactivate
};

/*--------------------------------------------------------------------*/

static void load_string(unsigned char **str, INTEGER resid)
/* Tries to load the string resource with the given `resid' and sets
   the pointer *str to it.  If it cannot be found, the pointer is set
   to a default string.  Used by `game_init' to simplify code. */
{
	static unsigned char *default_string = "\p?";
	
	register StringHandle hand = GetString(resid);
	if (hand != 0) {
		HLock(hand);
		*str = *hand;
	} else
		*str = default_string;
}

/*--------------------------------------------------------------------*/

Boolean game_init(void)
/* Initialization routine for game window.  Must be called at the
   beginning of the program. Returns TRUE if it failed. */
{
	register int p;
	Rect top_row;
	
	/* Load block patterns and strings */
	
	for (p = 0; p < NUMBER_BLOCK_TYPES; p ++)
		GetIndPattern(cell_patterns + p, PATTERN_ID, p + 1);
	
	load_string(&score_str, SCORE_STR_ID);
	load_string(&game_over_str, GAME_OVER_STR_ID);
	load_string(&pause_str, PAUSE_STR_ID);
	load_string(&continue_str, CONTINUE_STR_ID);

	/* Load sounds only if we can play them */
	
	if (env_SndPlay_available()) {
		hit_snd = GetResource('snd ', HIT_SND_ID);
		if (hit_snd != 0)
			HLock(hit_snd);
		end_snd = GetResource('snd ', END_SND_ID);
		if (end_snd != 0)
			HLock(end_snd);
	}
	
	/* Set up drawing variables */
	
	rgn1 = NewRgn();
	rgn2 = NewRgn();
	rgn_top_row = NewRgn();
	top_row = field_rect;
	top_row.bottom = top_row.top + CELL_SIZE;
	RectRgn(rgn_top_row, &top_row);
	
	/* Create and setup window and its button */
	
	game_wind = GetNewWindow(GAME_WINDOW_ID, NIL, (WindowPtr) -1);
	if (game_wind == 0)
		return TRUE;
	SetWRefCon(game_wind, (LONGINT) &game_dispatch_table);
	game_window_active = FALSE;
	
	game_pause_ctrlh = GetNewControl(GAME_PAUSE_CNTL_ID, game_wind);
	if (game_pause_ctrlh == 0)
		return TRUE;
	
	/* Set up */
	
	SetPort(game_wind);
	TextFont(0); /* System font */
	game_running = FALSE;
	game_paused = FALSE;
	
	return FALSE; /* Success */
} 

/*--------------------------------------------------------------------*/

void game_begin(void)
/* Show the game window.  This is a separate routine so that we can
   optionally load the location information from the preferences file
   before showing the window. */
{
	ShowWindow(game_wind);
}

/*--------------------------------------------------------------------*/

void game_term(void)
/* To be called at the end of the program. */
{
	CloseRgn(rgn1);
	CloseRgn(rgn2);
	CloseRgn(rgn_top_row);
	DisposeWindow(game_wind);	/* Controls automatically deleted */
}

/*--------------------------------------------------------------------*/

static saved_file_set(unsigned char *name, INTEGER vref)
{
	if (name) {
		pstrcpy(saved_name, name);
		saved_vref = vref;
		saved_file_known = TRUE;
	} else {
		pstrcpy(saved_name, "\pUntitled");
		saved_file_known = FALSE;
	}
	
	SetWTitle(game_wind, saved_name);
}

/*====================================================================*/

/* Internal drawing routines */

static void draw_game_over(void)
/* If the game is over, the game over message is drawn, otherwise the
   location where that string goes is erased.  This routine is used to
   both draw and erase this message. */
{
	if (! game_running) {
		MoveTo(GAME_OVER_X, GAME_OVER_Y);
		DrawString(game_over_str);
	} else {
		/* Determine where that string will be drawn and erase it. */
		Rect r;
		FontInfo fi;
		
		GetFontInfo(&fi);
		
		r.left = GAME_OVER_X;
		r.right = GAME_OVER_X + StringWidth(game_over_str);
		r.top = GAME_OVER_Y - fi.ascent;
		r.bottom = GAME_OVER_Y + fi.descent;
		
		EraseRect(&r);
	}
}

/*--------------------------------------------------------------------*/

static void draw_score(void)
/* Draws the value of the score in the game window. */
{
	Str255 score_string;
	Rect	bound;
	
	bound.left = SCORE_VALUE_LEFT;
	bound.top = SCORE_VALUE_TOP - 10;
	bound.right = SCORE_VALUE_LEFT + 50;
	bound.bottom = SCORE_VALUE_TOP + 4;
	
	SetPort(game_wind);

	EraseRect(&bound);
	NumToString(tetris_score(), score_string);
	MoveTo(SCORE_VALUE_LEFT, SCORE_VALUE_TOP);
	DrawString(score_string);
}

/*--------------------------------------------------------------------*/

static void draw_cell(int x, int y, unsigned char pattern)
/* Internal routine to draw a cell on the screen display. Assumes
   that the drawing port has already been set up.  If the pattern is
   the empty cell (pattern == 0), the cell is erased. */
{
	Rect cell;
	
	cell.left = x * CELL_SIZE + FIELD_LEFT;
	cell.top = y * CELL_SIZE + FIELD_TOP;
	cell.right = cell.left + CELL_SIZE;
	cell.bottom = cell.top + CELL_SIZE;
		
	if (pattern == 0)
		EraseRect(&cell);
	else {
#ifdef COLOUR_GRAPHICS
		ForeColor(greenColor);
#endif

		FillRect(&cell, cell_patterns[pattern - 1]);
		FrameRect(&cell);

#ifdef COLOUR_GRAPHICS
		ForeColor(blackColor);
#endif
	}
}

/*====================================================================*/

/* Dispatch table routines */

static void game_mouseDown(Point where)
/* Handler for mouseDown events in the game window. Determines if the
   pause/continue control has been hit, and processes it appropriately. */
{
	ControlHandle ctrl;
	INTEGER part;
	
	SetPort(game_wind);
	GlobalToLocal(&where);
	part = FindControl(where, game_wind, &ctrl);

	if (ctrl != 0 && TrackControl(ctrl, where, NIL) != 0) {
		if (game_paused) {
			tetris_pause(FALSE);
			SetCTitle(game_pause_ctrlh, pause_str);
			game_paused = FALSE;
		} else {
			tetris_pause(TRUE);
			SetCTitle(game_pause_ctrlh, continue_str);
			game_paused = TRUE;
		}
	}
}

/*--------------------------------------------------------------------*/

static void game_key(unsigned char code, unsigned char ascii)
/* Handler for key presses when the game window is in front.
   Recognizes the four control keys and processes them appropriately. */
{
	if (code == ctrls.code[KEY_LEFT])
		tetris_try_move(move_left);
	else if (code == ctrls.code[KEY_ROT])
		tetris_try_move(move_anticlockwise);
	else if (code == ctrls.code[KEY_RIGHT])
		tetris_try_move(move_right);
	else if (code == ctrls.code[KEY_DROP])
		tetris_try_move(move_drop);
	else {
		/* Ignore all other keys */
	}
}

/*--------------------------------------------------------------------*/

static void game_update(void)
/* Handler for update events to the game window.  Redraws the entire
   game window. */
{
	register int x, y;
	Rect r = field_rect;

	SetPort(game_wind);
	DrawControls(game_wind);	
	InsetRect(&r, -1, -1);
#ifdef COLOUR_GRAPHICS
	ForeColor(blueColor);
#endif
	FrameRect(&r);			/* Border to the playing field */
#ifdef COLOUR_GRAPHICS
	ForeColor(blackColor);
#endif

	/* Show the score */
	
	MoveTo(SCORE_TITLE_LEFT, SCORE_TITLE_TOP);
	DrawString(score_str);
	draw_score();
	
	/* Field contents (draw from bottom up to look nice) */
	
	for (y = NUMBER_ROWS - 1; y >= 0; y--)
		for (x = 0; x < NUMBER_COLS; x++)
			if (field[x][y] != 0)
				draw_cell(x, y, field[x][y]);
				
	/* The next block */
	
	for (x = 0; x < 4; x++)
		for (y = 0; y < 4; y++)
			if (next_block[x][y])
				draw_cell(x + NEXT_PIECE_OFFSET_X, y + NEXT_PIECE_OFFSET_Y,
				          next_block[x][y]);

	/* Game over text, if appropriate */
	
	draw_game_over();
}

/*--------------------------------------------------------------------*/

static void game_activate(void)
/* Handler for activate events to the game window.  Takes game out of
   any pause mode that was automatically entered when it went behind
   another window. */
{
	HiliteControl(game_pause_ctrlh, 0); /* Activate it */
	if (game_running && !game_paused)
		tetris_pause(FALSE);

	game_window_active = TRUE;
}

/*--------------------------------------------------------------------*/

static void game_deactivate(void)
/* Handler for deactivate events to the game window.  Automatically
   pauses the game. */
{
	HiliteControl(game_pause_ctrlh, 255);
	if (game_running && !game_paused)
		tetris_pause(TRUE);

	game_window_active = FALSE;
}

/*====================================================================*/

/* Support routines for the tetris module */

void game_del_row(int victim)
/* Called by the tetris module to remove a complete row from the playing
   field. It is here that sounds are played and the display and field
   updated. */
{
	register int row, col;
	Rect area;

#ifdef COLOUR_GRAPHICS
	/* Draws the border of the completed row in red before playing the
	   sound.  Should be nice if you have a colour Mac. */
	   
	ForeColor(redColor);

	area.top = FIELD_TOP + CELL_SIZE * victim;
	area.left = FIELD_LEFT;
	area.bottom = area.top + CELL_SIZE; 
	area.right = FIELD_LEFT + CELL_SIZE * NUMBER_COLS;
	FrameRect(&area);

	ForeColor(blackColor);
#endif

	/* Play the sound */
	
	if (ctrls.sound_on)
		if (hit_snd == 0 || SndPlay(NIL, hit_snd, FALSE) != noErr)
			SysBeep(5);
	
	/* Delete row from field array */
		
	for (row = victim - 1; row >= 0; row--)
		for (col = 0; col < NUMBER_COLS; col++)
			field[col][row + 1] = field[col][row];
			
	for (col = 0; col < NUMBER_COLS; col++)
		field[col][0] = 0;
	
	/* Display this on the screen */
	
	area.top = FIELD_TOP;
	area.left = FIELD_LEFT;
	area.bottom = FIELD_TOP + CELL_SIZE * (victim + 1); 
	area.right = FIELD_LEFT + CELL_SIZE * NUMBER_COLS;

	SetPort(game_wind);
	ScrollRect(&area, 0, CELL_SIZE, rgn1);

	/* Need to redraw entire field if more than just the top becomes
	   invalid.  This is because we do not know if more rows will be
	   deleted after this, and hence we may be scrolling invalid
	   regions around.  We can't just invalidate this rgn1 because
	   the next scroll will move the bad pixels from under it. */
	
	DiffRgn(rgn1, rgn_top_row, rgn2);
	if (! EmptyRgn(rgn2))
		game_update();
}

/*--------------------------------------------------------------------*/

void game_set(int x, int y, unsigned char pattern)
/* Called by the tetris module to set a field element. If the cell is
   valid, its value is stored and it is redrawn. */
{
	if (x < 0 || x >= NUMBER_COLS || y < 0 || y >= NUMBER_ROWS)
		return;
	
	field[x][y] = pattern;
	SetPort(game_wind);
	draw_cell(x, y, pattern);
}

/*--------------------------------------------------------------------*/

unsigned char game_get(int x, int y)
/* Called by the tetris module to get the value of a field element.  If
   the coordinates of the cell are invalid, 0 is returned. */
{
	if (x < 0 || x >= NUMBER_COLS || y < 0 || y >= NUMBER_ROWS)
		return 0;
	else
		return field[x][y];
}

/*--------------------------------------------------------------------*/

void game_score_changed(void)
/* Called by the tetris module to indicate that the score has changed.
   This function draws the new score on the screen. */
{
	draw_score();
}

/*--------------------------------------------------------------------*/

void game_over(void)
/* Called by the tetris module to indicate the game is over.  Plays
   a sound and updates the display, trying to record the score in the
   high score list. */
{
	if (ctrls.sound_on)
		if (end_snd == 0 || SndPlay(NIL, end_snd, FALSE) != noErr)
			SysBeep(5);
	
	game_running = FALSE;
	SetPort(game_wind);
	draw_game_over();
	
	HiliteControl(game_pause_ctrlh, 255); /* make inactive */
	
	highscore_add(tetris_score());
}

/*--------------------------------------------------------------------*/

void describe_next_begin(void)
/* Called by the tetris module to indicate that it is about to describe
   the cells of the next block to be played (by calling the 
   `describe_next_cell' below).  We clear the local record of the next
   block. */
{
	register int x, y;
	
	SetPort(game_wind);

	for (x = 0; x < 4; x++)
		for (y = 0; y < 4; y++)
			next_block[x][y] = 0;
}

/*--------------------------------------------------------------------*/

void describe_next_cell(int x, int y, unsigned char pattern)
/* Used by the tetris module to describe the cells of the next block.
   Will be called between calls to `describe_next_begin' and 
   `describe_next_end'. */
{
	if (ctrls.show_next_piece)
		next_block[x][y] = pattern;
}

/*--------------------------------------------------------------------*/

void describe_next_end(void)
/* Used by the tetris module to indicate that it is finished describing
   the next block.  Invalidates the next block's display rectangular
   area so that it will be updated. */
{
	Rect r;
	
	r.left = NEXT_PIECE_OFFSET_X * CELL_SIZE + FIELD_LEFT;
	r.top = NEXT_PIECE_OFFSET_Y * CELL_SIZE + FIELD_TOP;
	r.right = r.left + CELL_SIZE * MAX_BLOCK_SIZE;
	r.bottom = r.top + CELL_SIZE * MAX_BLOCK_SIZE;
	
	SetPort(game_wind);
	EraseRect(&r);
	InvalRect(&r);
}

/*====================================================================*/

/* Other interfaces to higher modules */

void game_periodic(void)
/* This routine is called as often as possible.  It calls the tetris
   peridic routine as often as possible when the game is played. */
{
	if (game_window_active && !game_paused && game_running)
		tetris_periodic();
}

/*--------------------------------------------------------------------*/

void game_new(void)
/* This routine starts a new game, and resets the save file name and
   window title. It does not ask the user whether to save the current
   game or not - it is discarded. */
{
	register int x, y;
	
	/* Clears the field and next block to be empty */
	
	for (y = 0; y < NUMBER_ROWS; y++)
		for (x = 0; x < NUMBER_COLS; x++)
			field[x][y] = 0;
		
	/* Reset the pause push button */
	
	if (game_window_active)
		HiliteControl(game_pause_ctrlh, 0);
	
	/* Start up the game */
	
	tetris_start(0);
	game_running = TRUE;
	game_paused = FALSE;
	SetCTitle(game_pause_ctrlh, pause_str);
	
	/* Fix up the display */
	
	SetPort(game_wind);
	EraseRect(&field_rect);
	InvalRect(&field_rect);
	draw_game_over();
	draw_score();

	saved_file_set(0, 0);
}

/*--------------------------------------------------------------------*/

void game_save_as(void)
/* Save the current game under a new file, ignoring the file it might
   have come from. */
{
	register INTEGER erc;
	INTEGER file_ref;
	SFReply reply;
	
	/* Get the file to save to */
	
	SFPutFile(SFPutFile_where, "\pSave game as:", "\pUntitled", NIL, &reply);
	if (reply.good) {
		saved_file_set(reply.fName, reply.vRefNum);
		game_save();
	}
}

/*--------------------------------------------------------------------*/

void game_save(void)
/* Creates and writes out the game information to the file specified
   in `saved_reply'. If the file name is not already known, it calls
   `game_save_as' to find it first. */
{
	OSErr erc;
	LONGINT size;
	INTEGER file_ref;
	struct Save_header header;
	
	if (! saved_file_known) {
		game_save_as();
		return;
	}
		
	/* Create the file, ignore error if it says it already exists */
	
	erc = Create(saved_name, saved_vref, CREATOR_SIGNATURE, SAVE_FILE_SIGNATURE);
	if (erc != noErr && erc != dupFNErr) {
		ParamText(saved_name, 0, 0, 0);
		alert_erc(1, erc);
		saved_file_set(0, 0);
		return;
	}

	erc = FSOpen(saved_name, saved_vref, &file_ref);
	if (erc != noErr) {
		ParamText(saved_name, 0, 0, 0);
		alert_erc(2, erc);
		saved_file_set(0, 0);
		return;
	}
	
	/* Fill in the header information */
	
	header.magic = SAVE_MAGIC_NUMBER;
	tetris_state_get(&header.state);
	
	/* Write out the data */
	
	size = sizeof(header);
	erc = FSWrite(file_ref, &size, &header);
	if (erc != noErr || size != sizeof(header)) {
		ParamText(saved_name, 0, 0, 0);
		alert_erc(4, erc);
		(void) FSClose(file_ref);
		saved_file_set(0, 0);
		return;
	}

	size = sizeof(field);
	erc = FSWrite(file_ref, &size, field);
	if (erc != noErr || size != sizeof(field)) {
		ParamText(saved_name, 0, 0, 0);
		alert_erc(4, erc);
		(void) FSClose(file_ref);
		saved_file_set(0, 0);
		return;
	}
	
	/* Close file */
	
	erc = FSClose(file_ref);
	if (erc != noErr) {
		ParamText(saved_name, 0, 0, 0);
		alert_erc(5, erc);
		saved_file_set(0, 0);
		return;
	}
}

/*--------------------------------------------------------------------*/

void game_open(void)
/* Asks the user for a game save file.  If the user supplies one, it
   is loaded up as the game to be played. */
{
	static SFTypeList sig_list = { SAVE_FILE_SIGNATURE };
	SFReply reply;
	
	SFGetFile(SFGetFile_where, NIL, NIL, 1, sig_list, NIL, &reply);
	if (reply.good)
		game_load(reply.fName, reply.vRefNum);		
}

/*--------------------------------------------------------------------*/

void game_load(Str255 fname, INTEGER vref)
/* Loads the game from the given file. */
{
	OSErr erc;
	INTEGER file_ref;
	LONGINT size;
	struct Save_header header;
	unsigned char new_field[NUMBER_COLS][NUMBER_ROWS];
	register int x, y;
	
	/* Open the save file */
	
	erc = FSOpen(fname, vref, &file_ref);
	if (erc != noErr) {
		ParamText(fname, 0, 0, 0);
		alert_erc(2, erc);
		return;
	}
	
	/* Load the game */
	
	size = sizeof(header);
	erc = FSRead(file_ref, &size, &header);
	if (erc != noErr || size != sizeof(header)) {
		ParamText(fname, 0, 0, 0);
		alert_erc(3, erc);
		(void) FSClose(file_ref);
		return;
	}
	
	size = sizeof(new_field);
	erc = FSRead(file_ref, &size, new_field);
	if (erc != noErr || size != sizeof(new_field)) {
		ParamText(fname, 0, 0, 0);
		alert_erc(3, erc);
		return;
	}
	
	/* Close the file */
	
	erc = FSClose(file_ref);
	if (erc != noErr) {
		ParamText(fname, 0, 0, 0);
		alert_erc(5, erc);
		return;
	}

	/* Verify header */
	
	if (header.magic != SAVE_MAGIC_NUMBER) {
		ParamText(fname, 0, 0, 0);
		alert_caution(2);
		return;
	}
	
	/* Use field values, filtering out invalid values */
	
	for (x = 0; x < NUMBER_COLS; x++)
		for (y = 0; y < NUMBER_ROWS; y++)
			field[x][y] = new_field[x][y] % (NUMBER_BLOCK_TYPES + 1);
	
	/* Record this file as the saved file */
	
	saved_file_set(fname, vref);
	
	/* Start play */
	
	tetris_start(&header.state);
	game_running = TRUE;
	game_paused = FALSE;
	SetCTitle(game_pause_ctrlh, pause_str);

	/* Update the screen */
		
	SetPort(game_wind);
	draw_score();
	draw_game_over();
	EraseRect(&field_rect);
	InvalRect(&field_rect);
}

/*--------------------------------------------------------------------*/

OSErr game_save_location(INTEGER pref_file)
/* Saves the game controls and user settings to the given resource
   file. */
{
	OSErr erc;
	LONGINT size;
	Point **handle;
	Rect r;
	
	r = (**(*(WindowPeek)game_wind).contRgn).rgnBBox;
	
	handle = (Point **) GetResource(PREF_RSRC_TYPE, GAME_WPOS_PREF_ID);
	if (handle != NIL && HomeResFile(handle) == pref_file) {
		RmveResource(handle);
		erc = ResError();
		if (erc != noErr)
			return erc;
		
		DisposHandle(handle);
	}
	
	/* Create new resource */
	
	erc = PtrToHand(&r, &handle, sizeof(Rect));
	if (erc != noErr)
		return erc;
		
	AddResource(handle, PREF_RSRC_TYPE, GAME_WPOS_PREF_ID, "\p");
	erc = ResError();
	if (erc != noErr)
		return erc;
	
	return noErr;
}

/*--------------------------------------------------------------------*/

void game_load_location(void)
/* Tries to read the window position information resource.  This is
   found in the preference file, so it should be open at this stage.
   If it is found and the location places the top right or the top left
   corner in the desktop, it is moved to that location, otherwise it is
   left alone. */
{
	register OSErr erc;
	LONGINT size;
	struct Rect **handle;
	
	handle = (struct Rect **) GetResource(PREF_RSRC_TYPE, GAME_WPOS_PREF_ID);
	if (handle) {
		Point ul, ur;
		
		ul.h = (*handle)->left;
		ul.v = (*handle)->top;
		ur.h = (*handle)->right;
		ur.v = (*handle)->top;
		
		if (PtInRgn(ul, GrayRgn) || PtInRgn(ur, GrayRgn))
			MoveWindow(game_wind, (*handle)->left, (*handle)->top, TRUE);
	}
}

/*--------------------------------------------------------------------*/

extern Boolean game_is_over(void)
{
	return (! game_running);
}

/*--------------------------------------------------------------------*/
