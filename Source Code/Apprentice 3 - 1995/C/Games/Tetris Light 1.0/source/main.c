/**********************************************************************\

File:		main.c

Purpose:	This is the main module for the Tetris program.
			

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

#include "alert.h"
#include "controls.h"
#include "game.h"
#include "highscore.h"
#include "windows.h"
#include "menu_da.h"
#include "env.h"
#include "resources.h"
#include "pref_file.h"

/*--------------------------------------------------------------------*/

/* Globals */

static EventRecord	evt;
static Boolean		in_foreground = TRUE;

static Boolean		program_finished = FALSE;

static INTEGER pref_file;
static Boolean pref_file_valid = FALSE;

/*--------------------------------------------------------------------*/

/* Local Prototypes */

static void handle_mouseDown(void);
static void handle_mf_event(void);

static void start_up(void);
static void shut_down(void);

/*--------------------------------------------------------------------*/

static pascal void sys_error_resume(void)
/* Resume routine for system error. Just exits, but is better than 
   forcing users to reboot. */
{
	ExitToShell();
}

static void tool_box_init(void)
/* General initialization calls.  Called at the beginning of the program. */
{
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);	/* Remove all events */
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(sys_error_resume);
	InitCursor();
}

/*--------------------------------------------------------------------*/

static void process_arg_files(void)
/* Process the file supplied when opening the application. */
{
	INTEGER message;
	INTEGER count;
	
	CountAppFiles(&message, &count);
	
	if (count > 0) {
		INTEGER index;
		
		if (message != appOpen)
			ExitToShell();

		for (index = 1; index <= count; index++) {
			AppFile info;
			
			GetAppFiles(index, &info);
			if (info.fType == SAVE_FILE_SIGNATURE)
				game_load(info.fName, info.vRefNum);
			ClrAppFiles(index);
		}
	}
}


/*--------------------------------------------------------------------*/

void main(void)
/* The main function. Includes the main event loop. */
{
	Boolean WNE_implemented = FALSE;
	
	/* Initialization */
	
	tool_box_init();
	
	if (menu_init())
		alert_fatal(1);
	if (game_init())
		alert_fatal(2);
	if (highscore_init())
		alert_fatal(3);		
	if (controls_init())
		alert_fatal(4);
	
	WNE_implemented = env_WaitNextEvent_available();
	GetDateTime(&randSeed);	/* Initialize random number seed */
	
	game_new();
	process_arg_files();

	start_up();
	
	/* Main event loop */
	
	while (! program_finished) {
		if (WNE_implemented)
			WaitNextEvent(everyEvent, &evt, 0L, NIL);
		else {
			SystemTask();
			GetNextEvent(everyEvent, &evt);
		}
		
		switch (evt.what) {
		case mouseDown:
			handle_mouseDown();
			break;
		case mouseUp:
			/* ignored */
			break;
		case autoKey:
		case keyDown:			
			if (evt.modifiers & cmdKey) {
				adjust_menus();
				program_finished = menu_choice(MenuKey(evt.message &
													   charCodeMask));
			}
			else {
				register WindowPtr wind = FrontWindow();
				
				if (wind && ! is_DA_window(wind))
					window_key(wind, (evt.message & keyCodeMask) >> 8,
							   evt.message & charCodeMask);
			}
			break;
		case updateEvt:
			window_update((WindowPtr) evt.message);
			break;
		case activateEvt:
			if (evt.modifiers & activeFlag)
				window_activate((WindowPtr) evt.message);
			else
				window_deactivate((WindowPtr) evt.message);
			break;
		case osEvt:
			handle_mf_event();
			break;
		}
		
		game_periodic();
	}
	
	shut_down();
	
	game_term();
	highscore_term();
	
	ExitToShell();
}

/*--------------------------------------------------------------------*/

static void handle_mouseDown(void)
/* Handles mouseDown events from the main event loop. Dispatches them
   to the appropriate handler. */
{
	WindowPtr	wind;

	switch (FindWindow(evt.where, &wind)) {
	case inDesk:
		/* ignore */
		break;
	case inMenuBar:
		adjust_menus ();
		program_finished = menu_choice(MenuSelect(evt.where));
		break;
	case inSysWindow:
		SystemClick(&evt, wind);
		break;
	case inContent:
		if (FrontWindow() == wind)
			window_mouseDown(wind, evt.where);
		else
			SelectWindow(wind);
		break;
	case inDrag:
		DragWindow(wind, evt.where, &((*GrayRgn)->rgnBBox));
	case inGrow:
		break;
	case inGoAway:
		program_finished = TrackGoAway(wind, evt.where);
		break;
	default:
		/* Should not get here */
		SysBeep(5);
		break;
	}
}

/*--------------------------------------------------------------------*/

static void handle_mf_event(void)
/* Handles the multifinder events. */
{
	register unsigned char type = (evt.message & osEvtMessageMask) >> 24;
	
	switch (type) {
  	case suspendResumeMessage:
  		if (evt.message & resumeFlag) {
  			/* Resume */
  			register WindowPtr front = FrontWindow();
  			
  			if (! is_DA_window(front))
				window_activate(front);
  			in_foreground = TRUE;
  		}
  		else {
  			/* Suspend */
  			register WindowPtr front = FrontWindow();
  			
  			if (! is_DA_window(front))
				window_deactivate(front);
  			in_foreground = FALSE;
  		}
  		break;
	case mouseMovedMessage:
		/* Ignore */
		break;
	}
}

/*--------------------------------------------------------------------*/

static Boolean option_key_pressed(void)
/* Determines if the option key is pressed.  Returns true if it is. */
{
	static const k_option = 58;
	unsigned char map[16];
	
	GetKeys(&map);
	
	if ((map[k_option >> 3] >> (k_option & 7)) & 1) 
		return TRUE;
	else 
		return FALSE;
}

/*--------------------------------------------------------------------*/

static void start_up(void)
/* Opens the preference file, and loads up the information from it. */
{
	register OSErr erc;
	
	erc = pref_open(&pref_file, PREF_FILE_STR_ID,
					CREATOR_SIGNATURE, PREF_FILE_SIGNATURE, FALSE);
	if (erc == noErr) {
		pref_file_valid = TRUE;
		if (high_score_load())
			alert_caution(1);
		controls_load();
		game_load_location();
		highscore_load_location();
	}
	
	game_begin();
}

/*--------------------------------------------------------------------*/

static void shut_down(void)
/* Writes out information to the preference file (creating one if it
   did not exist). */
{
	register OSErr erc;

	if (! pref_file_valid) {
		erc = pref_open(&pref_file, PREF_FILE_STR_ID,
						CREATOR_SIGNATURE, PREF_FILE_SIGNATURE, TRUE);
		if (erc == noErr)
			pref_file_valid = TRUE;
		else {
			StringHandle sh = GetString(PREF_FILE_STR_ID);
			
			HLock(sh);
			ParamText(*sh, 0, 0, 0);
			alert_erc(2, erc);
			HUnlock(sh);
		}
	}
	
	/* Write out preference information */
	
	if (pref_file_valid) {
		erc = high_score_save(pref_file);
		if (erc != noErr)
			alert_erc(6, erc);
		erc = controls_save(pref_file);
		if (erc != noErr)
			alert_erc(6, erc);
		erc = game_save_location(pref_file);
		if (erc != noErr)
			alert_erc(6, erc);
		erc = highscore_save_location(pref_file);
		if (erc != noErr)
			alert_erc(6, erc);
	}
	
	/* Close the preference file */
	
	if (pref_file_valid) {
		CloseResFile(pref_file);
		erc = ResError();
		if (erc != noErr) {
			StringHandle sh = GetString(PREF_FILE_STR_ID);
			
			HLock(sh);
			ParamText(*sh, 0, 0, 0);
			alert_erc(5, erc);
			HUnlock(sh);
		}		
		pref_file_valid = FALSE;
	}
}

/*--------------------------------------------------------------------*/
