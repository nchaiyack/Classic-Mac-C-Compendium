/**********************************************************************\

File:		highscore.c

Purpose:	This module handles the high scores.
			

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

#include <Packages.h>

#include "dialutil.h"
#include "resources.h"
#include "highscore.h"
#include "windows.h"
#include "pstring.h"

/*--------------------------------------------------------------------*/

#define NUMBER_HIGH_SCORES	10		/* Maximum number scores recorded */

/* Horizontal positions of columns for displaying high scores */

#define SCORE_RX			40		/* score right */
#define NAME_X				50		/* name text */
#define DATE_TIME_RX		290		/* date right */
#define DATE_TIME_MAX_WIDTH	80		/* width of date */

/*--------------------------------------------------------------------*/

/* This structure stores the high score entry whilst it is in memory. */

typedef struct {
	LONGINT			date_time;
	unsigned int	score;
	Str255			name;
} High_score;

/*--------------------------------------------------------------------*/

/* Static globals */

static High_score high_score[NUMBER_HIGH_SCORES];

static INTEGER base_line_sep; /* Vertical separtion for lines of text */

static WindowPtr		high_wind;
static ControlHandle	high_ok_ctrlh;
static Boolean			high_active;

/*--------------------------------------------------------------------*/

/* Local prototypes */

static void highscore_mouseDown(Point);
static void highscore_key(unsigned char code, unsigned char ascii);
static void highscore_update(void);
static void highscore_activate(void);
static void highscore_deactivate(void);

static void high_score_button_highlight(Boolean active);

/*--------------------------------------------------------------------*/

/* Dispatch table for high score window */

static Wind_table high_dispatch_table = {
	highscore_mouseDown,
	highscore_key,
	highscore_update,
	highscore_activate,
	highscore_deactivate
};

/*--------------------------------------------------------------------*/

static void clear_high_scores(void)
/* Reset all high scores to zero, clearing names and dates. */
{
	register int x;
	
	for (x = 0; x < NUMBER_HIGH_SCORES; x++) {
		high_score[x].score = 0;
		high_score[x].date_time = 0;
		*(high_score[x].name) = 0;
	}
}

/*--------------------------------------------------------------------*/

Boolean highscore_init (void)
/* Initializes the high score module.  Returns FALSE on success, TRUE
   if it failed. */
{
	FontInfo info;
	
	/* Clear scores */
	
	clear_high_scores();

	/* Create and setup the display window */
	
	high_wind = GetNewWindow(HIGHSCORE_WINDOW_ID, NIL, NIL);
	if (high_wind == 0)
		return TRUE; /* Failed */
	SetWRefCon(high_wind, (LONGINT) &high_dispatch_table);
	high_active = FALSE;
	
	/* Create the button control that goes inside it */
	
	high_ok_ctrlh = GetNewControl(HIGH_OK_CNTL_ID, high_wind);
	if (high_ok_ctrlh == 0)
		return TRUE; /* Failed */
		
	/* Font baseline separation calculations */
	
	SetPort(high_wind);	
	TextFont(1); /* Application font */
	GetFontInfo(&info);
	base_line_sep = info.ascent + info.descent + info.leading;
	
	return FALSE; /* Success */
}

/*--------------------------------------------------------------------*/

void highscore_term(void)
/* Finishes up the high score module.  Disposes of the window and 
   control that was created. */
{
	DisposeWindow(high_wind);	/* Controls automatically deleted */	
}

/*--------------------------------------------------------------------*/

void highscore_start(void)
/* Brings up the high score display window.  Call this routine and
   return back to the main event loop to process the activate and 
   update events that will come. */
{
	SelectWindow(high_wind);
	ShowWindow(high_wind);
}

/*--------------------------------------------------------------------*/

static void highscore_end(void)
/* Removes the high score display window from view. */
{
	HideWindow(high_wind);
}

/*--------------------------------------------------------------------*/

static void highscore_mouseDown(Point where)
/* Processes mouseDown events to the high score display window. */
{
	ControlHandle ctrl;
	INTEGER part;
	
	SetPort(high_wind);
	GlobalToLocal(&where);
	
	part = FindControl(where, high_wind, &ctrl);
	if (ctrl != 0)
		if (TrackControl(ctrl, where, NIL))
			highscore_end();
}

/*--------------------------------------------------------------------*/

static void highscore_key(unsigned char code, unsigned char ascii)
/* Handles key presses when the high score window receives them.  Makes
   the window go away when the return or enter key is pressed. */
{
	if (ascii == RETURN_CODE || ascii == ENTER_CODE) {
		LONGINT final;
		
		HiliteControl(high_ok_ctrlh, 1);
		Delay(8, &final);
		HiliteControl(high_ok_ctrlh, 0);
		
		highscore_end();
	}
}

/*--------------------------------------------------------------------*/

static void highscore_update(void)
/* Handles update events to the high score display window.  Redraws it. */
{
	register int x;
	register int base_line = base_line_sep;
	Str255 buffer;
	
	SetPort(high_wind);
	EraseRect(&(high_wind->portRect));
	
	DrawControls(high_wind);
	high_score_button_highlight(high_active);
	
	for (x = 0; x < NUMBER_HIGH_SCORES && high_score[x].score > 0; x++) {
		register int len;
	
		/* Display the score */
	
		NumToString(high_score[x].score, buffer);
		MoveTo(SCORE_RX - StringWidth(buffer), base_line);	
		DrawString(buffer);
	
		/* Display the name */
		
		MoveTo(NAME_X, base_line);
		for (len = *(high_score[x].name); len > 0; len--) {
			if (TextWidth(high_score[x].name, 1, len) <
			    DATE_TIME_RX - DATE_TIME_MAX_WIDTH - NAME_X) {
				DrawText(high_score[x].name, 1, len);
				
				if (len != *(high_score[x].name))
					DrawChar(0xC9); /* ellipsis */
				break;
			}
		}
		
		/* Display the date */
		
		IUDateString(high_score[x].date_time, shortDate, buffer);
		MoveTo(DATE_TIME_RX - StringWidth(buffer), base_line);
		DrawString(buffer);

		base_line += base_line_sep;
	}
}

/*--------------------------------------------------------------------*/

static void highscore_activate(void)
{
	high_active = TRUE;
	
	SetPort(high_wind);
	high_score_button_highlight(TRUE);
	HiliteControl(high_ok_ctrlh, 0);
}

/*--------------------------------------------------------------------*/

static void highscore_deactivate(void)
{
	high_active = FALSE;
	
	SetPort(high_wind);
	high_score_button_highlight(FALSE);
	HiliteControl(high_ok_ctrlh, 255);
}

/*--------------------------------------------------------------------*/

static void high_score_button_highlight(Boolean active)
/* Draws the default button border around the OK button in black or gray
   depending whether `active' is true or not.  Assumed the grafPort has
   been setup ready for drawing. */
{
	Rect rect = (*high_ok_ctrlh)->contrlRect;
	
	if (! active)
		PenPat(gray);

	InsetRect(&rect, -4, -4);
	PenSize(3, 3);
	FrameRoundRect(&rect, 16, 16);
	
	PenSize(1, 1);
	PenPat(black);
}

/*--------------------------------------------------------------------*/

static pascal Boolean hs_filter_proc(DialogPtr dp, EventRecord *evt,
									 INTEGER *itemhit)
/* Event filter proc for the high score user name entry dialog. */
{
	switch (evt->what) {
	case keyDown:
		if ((evt->message & keyCodeMask) >> 8 == ESC_KEY) {
			simulate_key_hit(dp, Cancel);
			*itemhit = Cancel;
			return TRUE;
		}
		
		switch (evt->message & charCodeMask) {
		case ENTER_CODE:
		case RETURN_CODE:
			simulate_key_hit(dp, OK);
			*itemhit = OK;
			return TRUE;
		case '.':
			if ((evt->modifiers & cmdKey)) {
				simulate_key_hit(dp, Cancel);
				*itemhit = Cancel;
				return TRUE;
			}
			break;
		}
		break;
	case activateEvt:
		if (dp != (WindowPtr) evt->message) {
			if (evt->modifiers & activeFlag)
				window_activate((WindowPtr) evt->message);
			else
				window_deactivate((WindowPtr) evt->message);
		}
		break;
	case updateEvt:
		if (dp != (WindowPtr) evt->message) 
			window_update((WindowPtr) evt->message);
		break;
	}

	return FALSE;
}

void highscore_add(unsigned int score)
/* Tries to enter the `score' in the high score table.  If it will go,
   prompts the user for their name.  If they enter it successfully, the
   table is modified. */
{
	DialogPtr dial;
	INTEGER item;
	INTEGER dummy;
	Handle h;
	Rect box;

	if (score <= high_score[NUMBER_HIGH_SCORES - 1].score)
		/* Score too low to get into table - do nothing */
		return;
	
	/* Bring up the name getting dialog */
	
	dial = GetNewDialog(GET_NAME_DIAL_ID, NIL, (WindowPtr) -1);
	GetDItem(dial, NAME_ITEM_NO, &dummy, &h, &box);
	install_hilight_button(dial, OK, NAME_HIGHLIGHT_ITEM_NO);
	SelIText(dial, NAME_ITEM_NO, 0, 32767);
	
	ModalDialog(hs_filter_proc, &item);
	
	if (item == OK) {
		register int index = 0;
		register int i;
		Rect area;
		
		/* Find where new entry goes */
		
		while (score <= high_score[index].score)
			index++;
		
		/* Make room for it */
		
		for (i = NUMBER_HIGH_SCORES - 1; i > index; i--)
			high_score[i] = high_score[i - 1];
		
		/* Put it in */
			
		GetIText(h, high_score[index].name);
		GetDateTime(&(high_score[index].date_time));
		high_score[index].score = score;

		/* Force update of display to reflect new scores */
		
		area.left = 0;
		area.right = DATE_TIME_RX;
		area.top = 0;
		area.bottom = area.top + NUMBER_HIGH_SCORES * base_line_sep;
		SetPort(high_wind);
		InvalRect(&area);
		
		/* Display the high score list */
		
		highscore_start();
	}

	DisposDialog(dial);
}

/*--------------------------------------------------------------------*/

Boolean high_score_load(void)
/* Load the high scores from the preference file. Returns FALSE on 
   succes, TRUE if the information was corrupted. */
{
	register int x;
	register unsigned short prev;
	Handle handle;
	Boolean corrupted = FALSE;
	
	/* Read in the score information */
		
	for (x = 0; x < NUMBER_HIGH_SCORES; x++) {
		handle = GetResource(PREF_RSRC_TYPE, HIGH_SCORE_BASE_PREF_ID + x);
		if (handle)
			BlockMove(*handle, high_score + x, SizeResource(handle));
		else
			high_score[x].score = 0;
	}
	
	/* Check correctness */
	
	prev = high_score[0].score;
	for (x = 1; x < NUMBER_HIGH_SCORES; x++) {
		if (high_score[x].score > prev) {
			/* Order has been corrupted, wipe rest */
			corrupted = TRUE;
			high_score[x].score = 0;
			prev = 0;
		} else
			prev = high_score[x].score;
	}
	
	return corrupted;
}

/*--------------------------------------------------------------------*/

OSErr high_score_save(INTEGER pref_file)
/* Save the high scores to the preference file. */
{
	struct keys **handle;
	OSErr erc;
	register int x;
	
	/* Remove all the entries in the resource file */
	
	for (x = 0; x < NUMBER_HIGH_SCORES; x++) {
		handle = (struct keys **) GetResource(PREF_RSRC_TYPE, HIGH_SCORE_BASE_PREF_ID + x);
		if (handle != NIL && HomeResFile(handle) == pref_file) {
			RmveResource(handle);
			erc = ResError();
			if (erc != noErr)
				return erc;
			
			DisposHandle(handle);
		}
	}
	
	/* Create the new resource */
	
	for (x = 0; x < NUMBER_HIGH_SCORES && high_score[x].score > 0; x++) {
		LONGINT size = sizeof(LONGINT) + sizeof(unsigned int) + 
					   high_score[x].name[0] + 1;
		
		erc = PtrToHand(high_score + x, &handle, size);
		if (erc != noErr)
			return erc;
		
		AddResource(handle, PREF_RSRC_TYPE, HIGH_SCORE_BASE_PREF_ID + x, "\p");
		erc = ResError();
		if (erc != noErr)
			return erc;
	}

	return noErr;
}

/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/

OSErr highscore_save_location(INTEGER pref_file)
/* Saves the saves the position of the high score window. */
{
	OSErr erc;
	LONGINT size;
	Point **handle;
	Rect r;
	
	r = (**(*(WindowPeek)high_wind).contRgn).rgnBBox;
	
	handle = (Point **) GetResource(PREF_RSRC_TYPE, HI_WPOS_PREF_ID);
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
		
	AddResource(handle, PREF_RSRC_TYPE, HI_WPOS_PREF_ID, "\p");
	erc = ResError();
	if (erc != noErr)
		return erc;
	
	return noErr;
}

/*--------------------------------------------------------------------*/

void highscore_load_location(void)
/* Tries to read the window position information resource.  This is
   found in the preference file, so it should be open at this stage.
   If it is found and the location places the top right or the top left
   corner in the desktop, it is moved to that location, otherwise it is
   left alone. */
{
	register OSErr erc;
	LONGINT size;
	struct Rect **handle;
	
	handle = (struct Rect **) GetResource(PREF_RSRC_TYPE, HI_WPOS_PREF_ID);
	if (handle) {
		Point ul, ur;
		
		ul.h = (*handle)->left;
		ul.v = (*handle)->top;
		ur.h = (*handle)->right;
		ur.v = (*handle)->top;
		
		if (PtInRgn(ul, GrayRgn) || PtInRgn(ur, GrayRgn))
			MoveWindow(high_wind, (*handle)->left, (*handle)->top, TRUE);
	}
}
