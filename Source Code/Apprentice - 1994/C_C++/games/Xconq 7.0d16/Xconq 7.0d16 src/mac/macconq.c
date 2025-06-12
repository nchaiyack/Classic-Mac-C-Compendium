/* Copyright (c) 1992, 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Main Mac interface. */

#include "conq.h"
#include "mac.h"

extern int beforestart, endofgame;

#include <GestaltEqu.h>
#include <PPCToolbox.h>
#include <AppleEvents.h>

#include <time.h>

/* The console is a useful debugging hack.  When not in use, anything that uses stdout
   should be disabled. */

#ifdef USE_CONSOLE
#ifdef THINK_C
#include <console.h>
#include <Folders.h>
#else
#ifndef printf
printf() {}
#endif
#endif
#endif

/* (from DTS sample code) */
/* kOSEvent is the event number of the suspend/resume and mouse-moved events sent
   by MultiFinder. Once we determine that an event is an osEvent, we look at the
   high byte of the message sent to determine which kind it is. To differentiate
   suspend and resume events we check the resumeMask bit. */

#define	kOSEvent				app4Evt

/* high byte of suspend/resume event message */

#define	kSuspendResumeMessage	1

/* bit of message field for resume vs. suspend */

#define	kResumeMask				1

/* high byte of mouse-moved event message */

#define	kMouseMovedMessage		0xFA

#define keyReplyErr        'errn'

/* This is the id of any modal tool currently in effect. */

int modaltool = 0;

int inbackground;

/* This is the list of maps that we're using. */

struct a_map *maplist;

/* This is the list of lists. */

struct a_list *listlist;

/* This is the list of unit closeups. */

struct a_unit_closeup *unitcloseuplist;

struct a_side_closeup *sidecloseuplist;	 /* chain of side closeups */

int foundresourcesfile = FALSE;

/* This flag indicates whether the image etc resource file(s) were found. */

int foundimagesfile = FALSE;

/* The usual width of a scrollbar. */

int sbarwid = 15;

int defaultdrawgrid = TRUE;		   /* Display grid on the map? */
int defaultdrawnames = FALSE;		   /* Display unit names/numbers on the map? */

/* True if we're going to use WaitNextEvent. */

int useWNE = FALSE;

/* Rectangle that constrains window dragging. */

Rect dragrect;

/* Rectangle that constrains window resizing. */

Rect sizerect;

/* This is the side that is using this Mac as its display. */

Side *dside = NULL;

/* This points to a spare block of memory that is freed so shutdown code can
   use it (no guarantee that it will tho). */

Handle spare;

/* This is true if savable prefs etc have been saved since being changed. */
/* (always true for now, nothing being remembered) */

int interfacestatesafe = TRUE;

/* This is true when a single click suffices to move a unit. */

int defaultmoveonclick = TRUE;

int defaultautoselect = TRUE;

int wasingame = TRUE;

int suppresswarnings = FALSE;

int playsounds = TRUE;

/* Set to true if Color QuickDraw is installed. */

int hasColorQD;

/* The range of screen pixel depths that the display has to cope with. */

int minscreendepth = -1;

int maxscreendepth = -1;

/* This is true if AppleEvents are available. */

int hasAppleEvents;

/* This is true if the PPC toolbox is available. */

int hasPPCToolbox;

time_t realstarttime;

/* The HFS volume that the program started with. */

short initialvrefnum;

char *curdatestr = NULL;

int eventloopdone = FALSE;

int inputinvalid = FALSE;

/* The main Mac program. */

main()
{
	/* Do the most basic Macintosh setup. */
	init_toolbox();
	init_cursors();
	init_menus();
	init_rects();
	init_ae();
#ifdef USE_CONSOLE
	/* Make sure the console window is up before anybody tries to write to it. */
	freopenc(NULL, stdout);
	freopenc(stdout, stderr);
#endif
	/* Acquire Mac-specific files (preferences and resources). */
	get_files();
	/* Put the Xconq kernel into a known state. */
	clear_game_modules();
	init_data_structures();
	init_library_path();
	/* A hack to ensure some memory available for error handling. */
	spare = NewHandle(2000);
	/* If no Apple Events, go to the splash screen now, otherwise we'll wait
	   for an oapp/odoc/pdoc event to decide what to do. */
	if (!hasAppleEvents) {
		if (splash_dialog() == diSplashQuit) return;
	}
	/* All essential init done, jump into the main event loop. */
	event_loop();
}

splash_dialog()
{
	switch (do_splash_box()) {
		case diSplashNew:
			new_game_dialog();
			break;
		case diSplashOpen:
			open_game_dialog();
			break;
		case diSplashConnect:
			connect_game_dialog();
			break;
		case diSplashQuit:
			return diSplashQuit;
	}
	return -1;
}

/* (just a place holder for now) */

connect_game_dialog()
{
	LocationNameRec foo;
	PortInfoRec bar;
	
	PPCBrowser("\pEventually you'll be able to choose an Xconq game from here!",
			   "\pXconq", 0, &foo, &bar, NULL, "\p?");
}

/* Do the usual Mac setup calls. */

init_toolbox()
{
	SysEnvRec se;

	InitGraf(&QD(thePort));
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	InitCursor();

	SysEnvirons(2, &se);
	hasColorQD = se.hasColorQD;
	DGprintf("%s Color QuickDraw\n", (hasColorQD ? "Using" : "Not using"));
	recalc_depths();
}

recalc_depths()
{
	int depth, oldmin = minscreendepth, oldmax = maxscreendepth;
	GDHandle gdev;

	if (hasColorQD) {
		gdev = GetDeviceList();
		minscreendepth = maxscreendepth = (*((*gdev)->gdPMap))->pixelSize;
		while ((gdev = GetNextDevice(gdev)) != nil) {
			depth = (*((*gdev)->gdPMap))->pixelSize;
			if (depth < minscreendepth) minscreendepth = depth;
			if (depth > maxscreendepth) maxscreendepth = depth;
		}
	} else {
		minscreendepth = maxscreendepth = 1;
	}
	if (minscreendepth != oldmin || maxscreendepth != oldmax) {
		DGprintf("Screen depths range from %d to %d\n", minscreendepth, maxscreendepth);
	}
}

/* Set up the generic dragging and sizing rects. */

init_rects()
{
	RgnHandle screenrgn;

	screenrgn = GetGrayRgn();
	dragrect = (*screenrgn)->rgnBBox;
	SetRect(&sizerect, 50, 50, (*screenrgn)->rgnBBox.right,  (*screenrgn)->rgnBBox.bottom);
}

pascal OSErr do_ae_open_application(AppleEvent *message, AppleEvent *reply, long refcon);
pascal OSErr do_ae_open_documents(AppleEvent *message, AppleEvent *reply, long refcon);
pascal OSErr do_ae_print_documents(AppleEvent *message, AppleEvent *reply, long refcon);
pascal OSErr do_ae_quit_application(AppleEvent *message, AppleEvent *reply, long refcon);

typedef struct triplets{
	AEEventClass	theEventClass;
	AEEventID		theEventID;
	ProcPtr			theHandler;
} triplets;

triplets keywordsToInstall[] = {
	{ kCoreEventClass, kAEOpenApplication, (ProcPtr) do_ae_open_application },
	{ kCoreEventClass, kAEOpenDocuments, (ProcPtr) do_ae_open_documents },
	{ kCoreEventClass, kAEPrintDocuments, (ProcPtr) do_ae_print_documents },
	{ kCoreEventClass, kAEQuitApplication, (ProcPtr) do_ae_quit_application }
};

init_ae()
{
	OSErr	err;
	long	result;
	short	i;

	/* (should check that we have Gestalt first) */
	hasPPCToolbox  = (Gestalt(gestaltPPCToolboxAttr, &result) ? false : result != 0);
	hasAppleEvents = (Gestalt(gestaltAppleEventsAttr, &result) ? false : result != 0);

	if (hasAppleEvents) {
		for (i = 0; i < (sizeof(keywordsToInstall) / sizeof(triplets)); ++i) {
			err = AEInstallEventHandler(
				keywordsToInstall[i].theEventClass,	/* What class to install.  */
				keywordsToInstall[i].theEventID,	/* Keywords to install.    */
				keywordsToInstall[i].theHandler,	/* The AppleEvent handler. */
				0L,									/* Unused refcon.		   */
				false								/* Only for our app.	   */
			);

			if (err) {
/*				Alert(rErrorAlert, nil);  */
				SysBeep(20);
				return;
			}
		}
	}
}

/* Open and/or load any files that we might need, such as preferences
   and resources. */

get_files()
{
	short vref, refnum;
	long dirid;
	Str255 filename;
	FSSpec spec;

	/* Capture the current vrefnum. */
	GetVol(NULL, &initialvrefnum);
	/* Load up any preferences. */
	GetIndString(filename, sFilenames, siPreferences);
#ifndef THINK_C
	if (FindFolder(kOnSystemDisk, kPreferencesFolderType, kDontCreateFolder,
				   &vref, &dirid) == noErr) {
		if (FSMakeFSSpec(vref, dirid, filename, &spec) == noErr) {
			refnum = FSpOpenResFile(&spec, fsCurPerm);
			CloseResFile(refnum);
		}
	}
#endif
	/* Look for and open game library resource file(s). */
	foundresourcesfile = FALSE;
	GetIndString(filename, sFilenames, siResources);
	if (OpenResFile(filename) != -1) {
		foundresourcesfile = TRUE;
	}
	foundimagesfile = FALSE;
	GetIndString(filename, sFilenames, siImages);
	if (OpenResFile(filename) != -1) {
		foundimagesfile = TRUE;
	}
	/* Note that we don't complain yet if the resource/image files are missing,
	   since we don't yet know whether we actually need anything from them.
	   (Images etc might be built into app or game module, for instance.) */
}

/* Since Mac programs effectively take over the entire machine, we depend on
   this event loop to handle everything that might come along.  */

event_loop()
{
	int done = FALSE;
	Boolean gotevent;
	Point mouse;
	EventRecord	event;
	WindowPtr win;
	RgnHandle cursorRgn;

	/* Figure out if the WaitNextEvent Trap is available. */
	useWNE = (NGetTrapAddress(0x60, ToolTrap) != NGetTrapAddress(0x9f, ToolTrap));
	/* Pass WNE an empty region the 1st time thru. */
	cursorRgn = NewRgn();
	/* Loop (almost) forever. */
	while (!eventloopdone) {
		/* Use WaitNextEvent if it is available, otherwise GetNextEvent. */
		if (useWNE) {
			get_global_mouse(&mouse);
			adjust_cursor(mouse, cursorRgn);
			gotevent = WaitNextEvent(everyEvent, &event, 0L, cursorRgn);
		} else {
			SystemTask();
			gotevent = GetNextEvent(everyEvent, &event);
		}
		/* First decide if the event is for a dialog or is just any old event. */
		if (FrontWindow() != nil && IsDialogEvent(&event)) {
			short itemhit;
			DialogPtr dialog;

			/* Handle all the modeless dialogs here. */
			if (DialogSelect(&event, &dialog, &itemhit)) {
				if (dialog == instructionswin) {
					hit_instructions_dialog(dialog, itemhit, &event);
				/* } else if (dialog == helpwin) {
					hit_help_dialog(dialog, itemhit, &event); */
				} else if (dside != NULL) {
					if (hit_in_side_closeup(dialog, itemhit, &event)) {
					} else {
						DGprintf("random dialog hit?\n");
					}
				} else {
					DGprintf("really random dialog hit?\n");
				}
			} else {
				/* ??? */
			}
		} else if (gotevent) {
			/* Make sure we have the right cursor before handling the event. */
			adjust_cursor(event.where, cursorRgn);
			do_event(&event);
		} else if (!beforestart && !endofgame) {
			/* On null events, give the kernel a chance to run things. */
			/* Don't automatically go to a watch cursor, since run_game often
			   returns very quickly.  Instead, long-running subroutines should
			   call back to put a watch cursor up. */
			run_game(1);
			maybe_select_next_unit();
			/* If the game ended, force various changes in interaction. */
			if (endofgame) {
				Map *map;

				for_all_maps(map) {
					map->moveonclick = map->autoselect = FALSE;
				}
			}
		}
	}
}

get_global_mouse(mouse)
Point *mouse;
{
	EventRecord	evt;
	
	OSEventAvail(0, &evt);
	*mouse = evt.where;
}

Point lastmouse;

char mouseoverbuf[100];

/* Change the cursor to reflect the context. */

adjust_cursor(mouse, region)
Point mouse;
RgnHandle region;
{
	int x, y, approxdir = 1, usual = TRUE;
	Unit *unit = NULL;
	extern char *mouseover;
	extern int beforestart, endofgame;
	Map *map;
	CursPtr adjust_designer_cursor();
	GrafPtr oldport;
	extern CursHandle firecursor;

	if ((map = map_from_window(FrontWindow())) != NULL) {
		GetPort(&oldport);
		SetPort(map->window);
		GlobalToLocal(&mouse);
		if (PtInRect(mouse, &(map->contentrect))) {
			if (modaltool > 0) {
				switch (modaltool) {
					case 1:
						SetCursor(*opencrosscursor);
						break;
					case 2:
						SetCursor(*firecursor);
						break;
					default:
						/* (should error out here) */
						break;
				}
				usual = FALSE;
#ifdef DESIGNERS
			} else if (dside->designer && tooltype != notool) {
				SetCursor(adjust_designer_cursor(mouse, region));  usual = FALSE;
#endif DESIGNERS
			} else if (map->moveonclick) {
				if (map->numselections == 1
				    && (unit = map->selections[0]) != NULL) {
					/* Calculate the approx dir to here from selected unit. */
					nearest_cell(map, mouse.h, mouse.v, &x, &y);
					if (mobile(unit->type)
					    && (approxdir = approx_dir(x - unit->x, y - unit->y)) >= 0) {
						SetCursor(*(movecursors[approxdir]));  usual = FALSE;
					} else {
						SetCursor(*nomovecursor);  usual = FALSE;
					}
				} else if (map->numselections > 1) {
					SetCursor(*allmovecursor);  usual = FALSE;
				} else {
					/* (this is a little confusing here if no units are selected, since
				   	will just be arrow cursor) */
				}
			}
			/* This isn't really "cursor adjustment", but this is the right place
			   to do it - change the topline of the map to indicate what the cursor
			   is over. */
			if (map->toph > 0 && !EqualPt(mouse, lastmouse)) {
				oneliner(map, mouse.h, mouse.v);
				if (strcmp(tmpbuf, mouseoverbuf) != 0) {
					strcpy(mouseoverbuf, tmpbuf);
					mouseover = mouseoverbuf;
					draw_top_line(map);
				}
				lastmouse = mouse;
			}
		} else {
			if (map->toph > 0) {
				if (mouseover != NULL) {
					mouseover = NULL;
					draw_top_line(map);
				}
			}
		}
		SetPort(oldport);
	}
	
	if (endofgame || (!beforestart && dside && !dside->ingame)) {
		SetCursor(*grayarrowcursor);
		return;
	}
	/* If we got here and no cursor has been set already, go with the basic arrow. */
	if (usual) SetCursor(&QD(arrow));
}

/* Decipher an event. */

do_event(event)
EventRecord *event;
{
	short part, err, rslt = 0;
	WindowPtr win;
	Boolean hit;
	char key;
	Point pnt;
	GrafPtr oldport;

	switch (event->what) {
		case mouseDown:
			/* See if the click happened in a special part of the screen. */
			part = FindWindow(event->where, &win);
			switch (part) {
				case inMenuBar:
					adjust_menus();
					do_menu_command(MenuSelect(event->where));
					break;
				case inSysWindow:
					SystemClick(event, win);
					break;
				case inContent:
					if (win != FrontWindow()) {
						/* Bring the clicked-on window to the front. */
						SelectWindow(win);
						/* Fix the menu to match the new front window. */
						adjust_menus();
						/* We always want to discard the event now, since clicks in a
						   windows are often irreversible actions. */
					} else {
						/* Mouse clicks in the front window do something useful. */
						do_mouse_down(win, event);
					}
					break;
				case inDrag:
					/* Standard drag behavior, no tricks necessary. */
					DragWindow(win, event->where, &dragrect);
					break;
				case inGrow:
					grow_window(win, event->where);
					break;
				case inZoomIn:
				case inZoomOut:
					zoom_window(win, event->where, part);
					break;
				case inGoAway:
					close_window(win);
					break;
			}
			break;
		case keyDown:
		case autoKey:
			key = event->message & charCodeMask;
			/* Check for menukey equivalents. */
			if (event->modifiers & cmdKey) {
				if (event->what == keyDown) {
					adjust_menus();
					do_menu_command(MenuKey(key));
				}
			} else {
				if (event->what == keyDown) {
					/* Random keypress, interpret it. */
					do_keyboard_command(key);
				}
			}
			break;
		case activateEvt:
			activate_window((WindowPtr) event->message, event->modifiers & activeFlag);
			break;
		case updateEvt:
			update_window((WindowPtr) event->message);
			break;
		case diskEvt:
			/*	Call DIBadMount in response to a diskEvt, so that the user can format
				 a floppy. (from DTS Sample) */
			if (HiWord(event->message) != noErr) {
				SetPt(&pnt, 50, 50);
				err = DIBadMount(pnt, event->message);
			}
			break;
		case kOSEvent:
		    /* Grab only a single byte. */
			switch ((event->message >> 24) & 0x0FF) {
				case kMouseMovedMessage:
					break;
				case kSuspendResumeMessage:
					inbackground = !(event->message & kResumeMask);
					activate_window(FrontWindow(), !inbackground);
					break;
			}
			break;
#ifdef THINK_C
		case kHighLevelEvent:
			AEProcessAppleEvent(event);
			break;
#endif
		case nullEvent:
			rslt = 1;
			break;
		default:
			break;
	}
#ifdef DEBUGGING
	/* This just forces output into the file. */
	update_debugging();
#endif
	return rslt;
}

/* Handle window growing by mindlessly tracking via GrowWindow,
   then passing the chosen size to specific window resize handlers
   or else doing the generic resize. */

grow_window(win, where)
WindowPtr win;
Point where;
{
	long winsize;
	short winh, winv;
	GrafPtr oldport;

	if ((winsize = GrowWindow(win, where, &sizerect)) != 0) {
		GetPort(&oldport);
		SetPort(win);
		winh = LoWord(winsize);  winv = HiWord(winsize);
		if (map_from_window(win)) {
			grow_map(map_from_window(win), winh, winv);
		} else if (list_from_window(win)) {
			grow_list(list_from_window(win), winh, winv);
		} else if (win == historywin) {
			grow_history(winh, winv);
		} else if (win == constructionwin) {
			grow_construction(winh, winv);
		} else if (win == helpwin) {
			grow_help(winh, winv);
		}
		SetPort(oldport);
	}
}

zoom_window(win, where, part)
WindowPtr win;
Point where;
short part;
{
	GrafPtr oldport;

	if (TrackBox(win, where, part)) {
		GetPort(&oldport);
		/* The window must be the current port. (ZoomWindow bug) */
		SetPort(win);
		if (map_from_window(win)) {
			zoom_map(map_from_window(win), part);
		} else if (list_from_window(win)) {
			zoom_list(list_from_window(win), part);
		} else if (win == historywin) {
			zoom_history(part);
		} else {
			/* Generic window zooming. */
			EraseRect(&win->portRect);
			ZoomWindow(win, part, true);
			InvalRect(&win->portRect);
		}
		SetPort(oldport);
	}
}

close_window(win)
WindowPtr win;
{
	if (is_da_window(win)) {
		CloseDeskAcc(((WindowPeek) win)->windowKind);
	} else if (is_app_window(win)) {
		/* Remove from the windows menu (OK to call even if window not in menu). */
		remove_window_menu_item(win);
		/* Do special activities for some window subtypes. */
		if (map_from_window(win)) {
			destroy_map(map_from_window(win));
		} else if (list_from_window(win)) {
			destroy_list(list_from_window(win));
		} else if (unit_closeup_from_window(win)) {
			destroy_unit_closeup(unit_closeup_from_window(win));
#ifdef DESIGNERS
		} else if (win == designwin) {
			/* Closing the design palette implies we're done designing. */
			disable_designing();
#endif /* DESIGNERS */
		}
		/* Remove the window from our sight, will provoke update events. */
		HideWindow(win);
		/* At least for now, don't actually dispose of the window. */
	}
}

/* This just dispatches to the appropriate window handler. */

do_mouse_down(window, event)
WindowPtr window;
EventRecord *event;
{
	Point mouse;
	Map *map;
	List *list;
	UnitCloseup *unitcloseup;

	if (is_app_window(window)) {
		SetPort(window);
		mouse = event->where;
		GlobalToLocal(&mouse);
		/* Locate the interface object that this is on. */
		if ((map = map_from_window(window)) != NULL) {
			do_mouse_down_map(map, mouse, event->modifiers); 
		} else if ((list = list_from_window(window)) != NULL) {
			do_mouse_down_list(list, mouse, event->modifiers); 
		} else if ((unitcloseup = unit_closeup_from_window(window)) != NULL) {
			do_mouse_down_unit_closeup(unitcloseup, mouse, event->modifiers); 
		} else if (window == gamewin) {
			do_mouse_down_game(mouse, event->modifiers);
		} else if (window == historywin) {
			do_mouse_down_history(mouse, event->modifiers);
		} else if (window == constructionwin) {
			do_mouse_down_construction(mouse, event->modifiers);
		} else if (window == helpwin) {
			do_mouse_down_help(mouse, event->modifiers);
#ifdef DESIGNERS
		} else if (window == designwin) {
			do_mouse_down_design(mouse, event->modifiers);
#endif /* DESIGNERS */
		}
	} else {
		/* ??? */
	}
}

/* Bringing a window to the front may entail messing with the menu. */

activate_window(win, activate)
WindowPtr win;
int activate;
{
	Map *map;
	List *list;

	if (win == nil) return;
	if ((map = map_from_window(win)) != NULL) {
		activate_map(map, activate);
	} else if ((list = list_from_window(win)) != NULL) {
		activate_list(list, activate);
	} else if (win == constructionwin) {
		activate_construction(activate);
	} else if (win == helpwin) {
		activate_help(activate);
	} else {
		DGprintf("%sactivating random window\n", (activate ? "" : "de"));
	}
	if (activate) {
		/* It's convenient to make the activated window also be the current GrafPort. */
		SetPort(win);
	}
	adjust_menus();
}

/* Update a given window.  This is the main routine that causes drawing into
   all the different kinds of windows. */

update_window(win)
WindowPtr win;
{
	int controls = TRUE, growbox = FALSE;
	GrafPtr oldport;
	Map *map;
	List *list;
	SideCloseup *sidecloseup;
	UnitCloseup *unitcloseup;

	/* Set the updating window to be the current grafport. */
	GetPort(&oldport);
	SetPort(win);
	recalc_depths();
	BeginUpdate(win);
	if ((map = map_from_window(win)) != NULL) {
		erase_map(map);
		draw_map(map);
		growbox = TRUE;
	} else if ((list = list_from_window(win)) != NULL) {
		draw_list(list);
		growbox = TRUE;
	} else if ((unitcloseup = unit_closeup_from_window(win)) != NULL) {
		draw_unit_closeup(unitcloseup);
	} else if ((sidecloseup = side_closeup_from_window(win)) != NULL) {
		draw_side_closeup(sidecloseup);
	} else if (win == gamewin) {
		draw_game();
		controls = FALSE;
	} else if (win == historywin) {
		draw_history();
		growbox = TRUE;
	} else if (win == constructionwin) {
		draw_construction();
		growbox = TRUE;
	} else if (win == helpwin) {
		draw_help();
		growbox = TRUE;
#ifdef DESIGNERS
	} else if (win == designwin) {
		draw_design_window();
		controls = FALSE;
#endif /* DESIGNERS */
	} else {
		controls = FALSE;
	}
	if (controls) {
		UpdateControls(win, win->visRgn);
	}
	if (growbox) {
		DrawGrowIcon(win);
	}
	EndUpdate(win);
	SetPort(oldport);
}

maybe_select_next_unit()
{
	Map *map;

	if ((!beforestart && !endofgame)
	    && (map = map_from_window(FrontWindow())) != NULL
	    && map->autoselect) {
		map->curunit = autonext_unit(dside, map->curunit);
		select_exactly_one_unit(map, map->curunit);
	}
}

/* Used to check for any unread required parameters. Returns true if we
   missed at least one. */

Boolean
MissedAnyParameters(AppleEvent *message)
{
	OSErr		err;
	DescType	ignoredActualType;
	AEKeyword	missedKeyword;
	Size		ignoredActualSize;
	EventRecord	event;

	err = AEGetAttributePtr(	/* SEE IF PARAMETERS ARE ALL USED UP.		  */
		message,				/* AppleEvent to check.						  */
		keyMissedKeywordAttr,	/* Look for unread parameters.				  */
		typeKeyword,			/* So we can see what type we missed, if any. */
		&ignoredActualType,		/* What is would have been if not coerced.	  */
		(Ptr)&missedKeyword,	/* Data area.  (Keyword not handled.)		  */
		sizeof(missedKeyword),	/* Size of data area.						  */
		&ignoredActualSize		/* Actual data size.						  */
	);
	/* No error means that we found some unused parameters. */
	if (err == noErr) {
		event.message = *(long *) &ignoredActualType;
		event.where = *(Point *) &missedKeyword;
		err = errAEEventNotHandled;
	}
	/* errAEDescNotFound means that there are no more parameters.  If we get
	   an error code other than that, flag it. */
	return(err != errAEDescNotFound);
}

pascal OSErr
do_ae_open_application(AppleEvent *message, AppleEvent *reply, long refcon)
{
#pragma unused (message, refcon)
	OSErr err;

	if (splash_dialog() == diSplashQuit) {
		/* Set the global that lets the whole program exit. */
		eventloopdone = TRUE;
	}
	AEPutParamPtr(reply, keyReplyErr, typeShortInteger, (Ptr) &err, sizeof(short));
	return err;
}

/* Called when we receive an AppleEvent with an ID of "kAEOpenDocuments".
   This routine gets the direct parameter, parses it up into little FSSpecs,
   and opens the first indicated file.  It also shows the technique to be used in
   determining if you are doing everything the AppleEvent record is telling
   you.  Parameters can be divided up into two groups: required and optional.
   Before executing an event, you must make sure that you've read all the
   required events.  This is done by making an "any more?" call to the
   AppleEvent manager. */

pascal OSErr
do_ae_open_documents(AppleEvent *message, AppleEvent *reply, long refcon)
{
#pragma unused (refcon)

	OSErr err;
#if 0
	gCurrentCursor = nil;
#endif
	OSErr		err2;
	AEDesc		theDesc;
	FSSpec		theFSS;
	short		loop;
	long		numFilesToOpen;
	AEKeyword	ignoredKeyWord;
	DescType	ignoredType;
	Size		ignoredSize;
/*	FileRecHndl	frHndl; */
	WindowPtr	docWindow;

	theDesc.dataHandle = nil;
		/* Make sure disposing of the descriptors is okay in all cases.
		** This will not be necessary after 7.0b3, since the calls that
		** attempt to create the descriptors will nil automatically
		** upon failure. */

	if (err = AEGetParamDesc(message, keyDirectObject, typeAEList, &theDesc))
		return err;
	if (!MissedAnyParameters(message)) {
		/* Got all the parameters we need.  Now, go through the direct object,
		   see what type it is, and parse it up. */
		if (!(err = AECountItems(&theDesc, &numFilesToOpen))) {
			/* We have numFilesToOpen that need opening, as either a window
			   or to be printed.  Go to it... */
			for (loop = 1; ((loop <= numFilesToOpen) && (!err)); ++loop) {
				err = AEGetNthPtr(		/* GET NEXT IN THE LIST...		 */
					&theDesc,			/* List of file names.			 */
					loop,				/* Item # in the list.			 */
					typeFSS,			/* Item is of type FSSpec.		 */
					&ignoredKeyWord,	/* Returned keyword -- we know.  */
					&ignoredType,		/* Returned type -- we know.	 */
					(Ptr) &theFSS,		/* Where to put the FSSpec info. */
					sizeof(theFSS),		/* Size of the FSSpec info.		 */
					&ignoredSize		/* Actual size -- we know.		 */
				);
				if (err) break;
				if (open_game_from_fsspec(&theFSS)) break;
			}
		}
	}
	err2 = AEDisposeDesc(&theDesc);
	err = (err ? err : err2);
	AEPutParamPtr(reply, keyReplyErr, typeShortInteger, (Ptr) &err, sizeof(short));
	return err;
}

pascal OSErr
do_ae_print_documents(AppleEvent *message, AppleEvent *reply, long refcon)
{
	OSErr err;

	AEPutParamPtr(reply, keyReplyErr, typeShortInteger, (Ptr) &err, sizeof(short));
	return err;
}

pascal OSErr
do_ae_quit_application(AppleEvent *message, AppleEvent *reply, long refcon)
{
	OSErr err = noErr;

	/* Set the global that lets the whole program exit. */
	eventloopdone = TRUE;
	AEPutParamPtr(reply, keyReplyErr, typeShortInteger, (Ptr) &err, sizeof(short));
	return noErr;
}

/* A warning just gets displayed, no other action is taken. */

init_warning(str, a1, a2, a3, a4, a5, a6)
char *str;
long a1, a2, a3, a4, a5, a6;
{
	char buf[BUFSIZE], buf2[BUFSIZE];

	if (suppresswarnings) return;
#ifdef USE_CONSOLE
	fprintf(stderr, "Warning: ");
	fprintf(stderr, str, a1, a2, a3, a4, a5, a6);
	fprintf(stderr, "\n");
	fflush(stderr);
#endif
	/* Cursor may be weird from loading, reset it. */
	SetCursor(&qd.arrow);
	sprintf(buf, str, a1, a2, a3, a4, a5, a6);
	c2p(buf, buf2);
	ParamText(buf2, "\p", "\p", "\p");
	switch (CautionAlert(aInitWarning, nil)) {
		case 1:
			/* Just keep going, hope that warning was a false alarm. */
			if (0 /* option key or some such */) {
				suppresswarnings = TRUE;
			}
			break;
		case 2:
			/* (should undo everything and blast back to initial choices) */
			ExitToShell();
			break;
	}
}

/* An init error is not necessarily fatal, but we still have to start over. */

init_error(str, a1, a2, a3, a4, a5, a6)
char *str;
long a1, a2, a3, a4, a5, a6;
{
	char buf[BUFSIZE], buf2[BUFSIZE];

	/* Make some space available, in case this is a memory exhaustion error. */
	if (spare != nil) {
		DisposHandle(spare);
		spare = nil;
	}
#ifdef USE_CONSOLE
	fprintf(stderr, "Error: ");
	fprintf(stderr, str, a1, a2, a3, a4, a5, a6);
	fprintf(stderr, "\n");
	fflush(stderr);
#endif
	/* Cursor may be weird from loading, reset it. */
	SetCursor(&qd.arrow);
	sprintf(buf, str, a1, a2, a3, a4, a5, a6);
	c2p(buf, buf2);
	ParamText(buf2, "\p", "\p", "\p");
	StopAlert(aInitError, nil);
	/* This is a bad time to die, no way to recover.  Fortunately,
	   it's not a big loss, since there's no game yet to lose,
	   and so we can just exit directly. */
	ExitToShell();
}

/* Runtime warnings are for when it's important to bug the players,
   but doesn't necessarily mean imminent danger of a crash. */

run_warning(str, a1, a2, a3, a4, a5, a6)
char *str;
long a1, a2, a3, a4, a5, a6;
{
	char buf[BUFSIZE], buf2[BUFSIZE];

	/* If we're not actually in the game yet, this is the same as an init warning. */
	if (beforestart) {
		init_warning(str, a1, a2, a3, a4, a5, a6);
		return;
	}
	if (suppresswarnings) return;
#ifdef USE_CONSOLE
	fprintf(stderr, "\nWarning: ");
	fprintf(stderr, str, a1, a2, a3, a4, a5, a6);
	fprintf(stderr, "\n");
	fflush(stderr);
#endif
	sprintf(buf, str, a1, a2, a3, a4, a5, a6);
	c2p(buf, buf2);
	ParamText(buf2, "\p", "\p", "\p");
	switch (CautionAlert(aRunWarning, nil)) {
		case 1:
			if (0 /* option key?? */) {
				suppresswarnings = TRUE;
			}
			break;
		case 2:
			save_the_game(TRUE, TRUE);
			ExitToShell();
			break;
		case 3:
			/* Just blast out of here. */
			ExitToShell();
			break;
	}
}

/* An run error is fatal, but allow an emergency save, might be able to salvage. */

run_error(str, a1, a2, a3, a4, a5, a6)
char *str;
long a1, a2, a3, a4, a5, a6;
{
	char buf[BUFSIZE], buf2[BUFSIZE];

	/* If we're not actually in the game yet, this is the same as an init error. */
	if (beforestart) {
		init_error(str, a1, a2, a3, a4, a5, a6);
		return;
	}
	/* Make some space available, in case this is a memory exhaustion error. */
	if (spare != nil) {
		DisposHandle(spare);
		spare = nil;
	}
#ifdef USE_CONSOLE
	fprintf(stderr, "\nError: ");
	fprintf(stderr, str, a1, a2, a3, a4, a5, a6);
	fprintf(stderr, "\n");
	fflush(stderr);
#endif
	sprintf(buf, str, a1, a2, a3, a4, a5, a6);
	c2p(buf, buf2);
	ParamText(buf2, "\p", "\p", "\p");
	switch (StopAlert(aRunError, nil)) {
		case 1:
			break;
		case 2:
			save_the_game(TRUE, TRUE);
			break;
	}
	/* We're outta here - just ahead of scrambled heaps and dangling ptrs! */
	ExitToShell();
}

/* This is true when a side has a display that may be safely written to. */

active_display(side)
Side *side;
{
	return (side && side->ui && side->ui->active);
}

/* The Mac never has any display buffers to flush. */

flush_display_buffers() {}

record_real_start_time()
{
	time(&realstarttime);
}

/* Detect types of windows. */

is_da_window(win)
WindowPtr win;
{
	return (win != nil && ((WindowPeek) win)->windowKind < 0);
}

is_app_window(win)
WindowPtr win;
{
	return (win != nil && ((WindowPeek) win)->windowKind >= 0);
}

char *
newsfile_name()
{
	return "Xconq News"; /* get from resource */
}

char *
savefile_name()
{
	return "Saved Game"; /* get from resource */
}

FILE *
open_library_file(module)
Module *module;
{
	short curvrefnum;
	char fullnamebuf[255];
	FILE *fp = NULL;
	
	/* Can't open anonymous library modules. */
	if (module->name == NULL) return NULL;
	/* Generate library pathname. */
	make_pathname(xconqlib, module->name, "g", fullnamebuf);
	/* Now try to open the file. */
	if ((fp = fopen(fullnamebuf, "r")) != NULL) {
		/* Remember the filename where we found it. */
		module->filename = copy_string(fullnamebuf);
	} else {
		GetVol(NULL, &curvrefnum);
		SetVol(NULL, initialvrefnum);
		if ((fp = fopen(fullnamebuf, "r")) != NULL) {
			/* Remember the filename (what about volume?) where we found it. */
			module->filename = copy_string(fullnamebuf);
		}
		SetVol(NULL, curvrefnum);
	}
	return fp;
}

FILE *
open_explicit_file(module)
Module *module;
{
	short curvrefnum;
	char fullnamebuf[255];
	FILE *fp = NULL;

	if (module->filename == NULL) {
		if (module->name != NULL) {
			make_pathname(xconqlib, module->name, "g", fullnamebuf);
			if ((fp = fopen(fullnamebuf, "r")) != NULL) {
				return fp;
			} else {
				GetVol(NULL, &curvrefnum);
				SetVol(NULL, initialvrefnum);
				fp = fopen(fullnamebuf, "r");
				SetVol(NULL, curvrefnum);
			}
		}
	} else {
		sprintf(fullnamebuf, "%s", module->filename);
		if ((fp = fopen(module->filename, "r")) != NULL) {
			return fp;
		}
		sprintf(fullnamebuf, ":%s", module->filename);
		if ((fp = fopen(fullnamebuf, "r")) != NULL) {
			return fp;
		}
		sprintf(fullnamebuf, "%s%s", ":lib:", module->filename);
		if ((fp = fopen(fullnamebuf, "r")) != NULL) {
			return fp;
		}
		/* Try opening a library module under where the program started. */
		GetVol(NULL, &curvrefnum);
		SetVol(NULL, initialvrefnum);
		fp = fopen(fullnamebuf, "r");
		SetVol(NULL, curvrefnum);
	}
    return fp;
}

make_pathname(path, name, extn, pathbuf)
char *path, *name, *extn, *pathbuf;
{
    sprintf(pathbuf, "");
    if (!empty_string(path)) {
	sprintf(pathbuf+strlen(pathbuf), "%s:", path);
    }
    sprintf(pathbuf+strlen(pathbuf), "%s", name);
    /* Don't add a second identical extension, but do add if extension
       is different (in case we want "foo.12" -> "foo.12.g" for instance) */
    if (strrchr(name, '.') && strcmp(strrchr(name, '.')+1, extn) == 0)
      return;
    if (!empty_string(extn)) {
	sprintf(pathbuf+strlen(pathbuf), ".%s", extn);
    }
}

/* Kernel callback to update info about the given side. */

update_side_display(side, side2, rightnow)
Side *side, *side2;
int rightnow;
{
	GrafPtr oldport;
	extern int mayseeall;

	if (active_display(side) && side2 != NULL) {
		GetPort(&oldport);
		if (gamewin != nil && ((WindowPeek) gamewin)->visible) {
			SetPort(gamewin);
			draw_side_status(side2);
		}
		/* (should do side closeups etc) */
		SetPort(oldport);
		if (side2 == dside && !side->ingame && side->status == 0 && wasingame) {
			/* (should be able to quit from here?) */
			CautionAlert(aOutOfGame, nil); 
			wasingame = FALSE;
			mayseeall = TRUE;
		}
	}
}

/* Kernel callback to show the current turn. */

update_turn_display(side, rightnow)
Side *side;
int rightnow;
{
	GrafPtr oldport;
	Map *map;

	if (active_display(side)) {
		c2p(absolute_date_string(curturn), curdatestr);
		GetPort(&oldport);
		if (gamewin != nil && ((WindowPeek) gamewin)->visible) {
			SetPort(gamewin);
			draw_game_date();
		}
		for_all_maps(map) {
			if (map->toph > 0) {
				SetPort(map->window);
				draw_top_line(map);
			}
		}
		SetPort(oldport);
	}
}

/* Callback that gets run once after all turn setup is done but before any movement. */

update_action_display(side, rightnow)
Side *side;
int rightnow;
{
	GrafPtr oldport;
	Map *map;
	UnitCloseup *unitcloseup;

	if (active_display(side)) {
		GetPort(&oldport);
		for_all_maps(map) {
			draw_selections(map);
			if (map->autoselect) {
				unselect_all(map);
				select_next_actor(map);
			}
		}
		for_all_unit_closeups(unitcloseup) {
			force_update(unitcloseup->window);
		}
		SetPort(oldport);
	}
}

update_action_result_display(side, unit, rslt, rightnow)
Side *side;
Unit *unit;
int rslt, rightnow;
{
	Action *action;

    if (active_display(side)) {
    	DGprintf("%s %s result is %s\n",
    			unit_desig(unit),
    			action_desig((unit->act ? &(unit->act->nextaction) : NULL)),
    			hevtdefns[rslt].name);
    	/* (should handle errors specially) */
    	action = (unit->act ? &(unit->act->nextaction) : NULL);
    	if (action == NULL) return;
    	switch (action->type) {
    		case A_CREATE_IN:
    		case A_CREATE_AT:
    		case A_BUILD:
    			if (rslt == A_ANY_DONE) {
    				update_construction_type_list();
    			}
    			break;
    	}
    }
}

int toldoutcome = FALSE;

update_event_display(side, hevt)
Side *side;
HistEvent *hevt;
{
	Side *side2;
	extern int mayseeall;

	if (active_display(side)) {
		switch (hevt->type) {
		  case H_SIDE_LOST:
		    if (hevt->data[0] == side_number(side)) {
		    	lost_game_dialog();
		    	toldoutcome = TRUE;
		    	mayseeall = TRUE;
		    } else if ((side2 = side_n(hevt->data[0])) != NULL) {
		    	/* Indicate that some other side lost. */
		    }
		  	break;
		  case H_SIDE_WON:
		    if (hevt->data[0] == side_number(side)) {
		    	won_game_dialog();
		    	toldoutcome = TRUE;
		    	mayseeall = TRUE;
		    } else if ((side2 = side_n(hevt->data[0])) != NULL) {
		    	/* Indicate that some other side won. */
		    }
		  	break;
		  case H_GAME_ENDED:
		  	if (!toldoutcome) game_over_dialog();
		    mayseeall = TRUE;
		    break;
		  default:
		  	/* No special display. */
		  	break;
		}
		if (historywin != nil) {
			SetPort(historywin);
			calc_history_layout();
			force_update(historywin);
		}
	}
}

update_fire_at_display(side, unit, unit2, m, rightnow)
Side *side;
Unit *unit, *unit2;
int m, rightnow;
{
	int i, sx1, sy1, sw1, sh1, sx2, sy2, sw2, sh2, dx, dy, xx, yy;
	int startticks, innerticks;
	Map *map;
	GrafPtr oldport, curport = NULL;

	if (active_display(side)) {
		GetPort(&oldport);
		startticks = TickCount();
		i = 0;
		/* Tweak the pen modes of all the maps. */
		for_all_maps(map) {
			SetPort(map->window);
			PenMode(patXor);
			if (map->hw > 10) PenSize(2, 2);
			else PenSize(1, 1);
		}
		while (TickCount() < startticks + 32) {
			innerticks = TickCount();
			for_all_maps(map) {
				if (curport != map->window) {
					SetPort(map->window);
					curport = map->window;
				}
				xform_unit(map, unit, &sx1, &sy1, &sw1, &sh1);
				xform_unit(map, unit2, &sx2, &sy2, &sw2, &sh2);
				/* Offset to draw lines from the middle of the units' images. */
				sx1 += sw1 / 2;  sy1 += sh1 / 2;
				sx2 += sw2 / 2;  sy2 += sh2 / 2;
				/* Draw one segment of a line between the units. */
				dx = (sx2 - sx1) / 4;  dy = (sy2 - sy1) / 4;
				xx = sx1 + ((i / 2) % 4) * dx;  yy = sy1 + ((i / 2) % 4) * dy;
				MoveTo(xx, yy);  Line(dx, dy);
			}
			while (TickCount() < innerticks + 1); /* 2 here seems a bit slowish */
			++i;
		}
		/* Restore the pen modes of all the maps. */
		for_all_maps(map) {
			SetPort(map->window);
			PenNormal();
		}
		SetPort(oldport);
	}
}

/* Update any displayed info about the given unit. */

update_unit_display(side, unit, rightnow)
Side *side;
Unit *unit;
int rightnow;
{
	UnitCloseup *unitcloseup;

	if (active_display(side) && unit != NULL) {
		if (side != unit->side) update_unit_in_maps(unit);
		if (1 /* unit visible to side in any way */ && inside_area(unit->x, unit->y)) {
			update_cell_display(side, unit->x, unit->y, TRUE);
		}
		update_unit_in_lists(unit);
		if ((unitcloseup = find_unit_closeup(unit)) != NULL
		    && 1 /* window is visible */) {
			draw_unit_closeup(unitcloseup);
		}
		if (unit->side != NULL && unit->act != NULL) {
			update_side_display(side, unit->side, rightnow);
		}
		if (constructionwin != nil
		    && ((WindowPeek) constructionwin)->visible) {
			update_construction_unit_list(unit);
		}
	}
}

update_unit_in_maps(unit)
Unit *unit;
{
	int i;
	Map *map;

	if (side_controls_unit(dside, unit)) return;
	for_all_maps(map) {
		unselect_unit_on_map(map, unit);
	}
}

update_clock_display(side, rightnow)
Side *side;
int rightnow;
{
	GrafPtr oldport;
	Map *map;
	time_t now;
	extern time_t lastnow;

	if (active_display(side)) {
		time(&now);
		/* If no changes since the last draw, jump out of here. */
		if (now == lastnow) return;
		GetPort(&oldport);
		if (gamewin != nil && ((WindowPeek) gamewin)->visible) {
			SetPort(gamewin);
			draw_game_clocks();
		}
#if 0
		for_all_maps(map) {
			if (map->toph > 0) {
				SetPort(map->window);
				draw_top_line(map);
			}
		}
#endif
		SetPort(oldport);
	}
}

update_all_progress_displays(str, s)
char *str;
int s;
{
	GrafPtr oldport;
	extern char *gameprogressstr;

	if (!active_display(dside)) return;
	gameprogressstr = str;
	GetPort(&oldport);
	if (gamewin != nil && ((WindowPeek) gamewin)->visible) {
		SetPort(gamewin);
		draw_game_progress();
	}
	SetPort(oldport);
}

won_game_dialog()
{
	int done = FALSE;
	short ditem;
	WindowPtr win;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	win = GetNewDialog(dWinGame, NULL, (DialogPtr) -1L);
	ShowWindow(win);
	while (!done) {
		draw_default_button(win, diWinGameQuit);
		ModalDialog(NULL, &ditem);
		switch (ditem) {
			case diWinGameQuit:
				ExitToShell();
				break;
			case diWinGameKeepGoing:
				done = TRUE;
				break;
			default:
				break;
		}
	}
	DisposDialog(win);
}

lost_game_dialog()
{
	int done = FALSE;
	short ditem;
	WindowPtr win;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	win = GetNewDialog(dLoseGame, NULL, (DialogPtr) -1L);
	ShowWindow(win);
	while (!done) {
		draw_default_button(win, diLoseGameQuit);
		ModalDialog(NULL, &ditem);
		switch (ditem) {
			case diLoseGameQuit:
				ExitToShell();
				break;
			case diLoseGameKeepGoing:
				done = TRUE;
				break;
			default:
				break;
		}
	}
	DisposDialog(win);
}

game_over_dialog()
{
	int done = FALSE;
	short ditem;
	WindowPtr win;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	win = GetNewDialog(dGameOver, NULL, (DialogPtr) -1L);
	ShowWindow(win);
	while (!done) {
		draw_default_button(win, diGameOverQuit);
		ModalDialog(NULL, &ditem);
		switch (ditem) {
			case diGameOverQuit:
				ExitToShell();
				break;
			case diGameOverKeepGoing:
				done = TRUE;
				break;
			default:
				break;
		}
	}
	DisposDialog(win);
}

/* Move the window to a position staggered from the given last position. */

stagger_window(win, lasthp, lastvp)
WindowPtr win;
int *lasthp, *lastvp;
{
	int h, v;
	Rect winrect;
	GrafPtr oldport;

	if (*lasthp > 0) {
		h = *lasthp + 20;  v = *lastvp + 20;
		/* (should that no other window here, iterate stagger if so) */
		MoveWindow(win, h, v, FALSE);
		*lasthp = h;  *lastvp = v;
	} else {
		/* Don't move the first window, but do record its position. */
		GetPort(&oldport);
		SetPort(win);
		winrect = win->portRect;
		LocalToGlobal(&top_left(winrect));
		*lasthp = winrect.left;  *lastvp = winrect.top;
		SetPort(oldport);
	}
}

/* General routine to outline the given item of a given dialog. */

draw_default_button(dialog, ditem)
DialogPtr dialog;
short ditem;
{
	GrafPtr oldport;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	GetPort(&oldport);
	SetPort(dialog);
	GetDItem(dialog, ditem, &itemtype, &itemhandle, &itemrect);
	PenSize(3, 3);
	InsetRect(&itemrect, -4, -4);
	FrameRoundRect(&itemrect, 16, 16);
	PenNormal();
	SetPort(oldport);
}

/* Cause an update of a window's entire contents. */

force_update(win)
WindowPtr win;
{
	GrafPtr oldport;

	if (win == nil) return;
	GetPort(&oldport);
	SetPort(win);
	EraseRect(&win->portRect);
	InvalRect(&win->portRect);
	SetPort(oldport);
}

force_overall_update()
{
	Map *map;
	List *list;
	UnitCloseup *unitcloseup;

	force_update(gamewin);
	force_update(historywin);
	force_update(constructionwin);
	force_update(helpwin);
	for_all_maps(map) {
		force_update(map->window);
	}
	for_all_lists(list) {
		force_update(list->window);
	}
	for_all_unit_closeups(unitcloseup) {
		force_update(unitcloseup->window);
	}
}

beep()
{
	SysBeep(20);
}

/* C-to-Pascal string conversion. */

c2p(register const char *s, register char *t)
{
	strcpy(t + 1, s);
	t[0] = strlen(s);
}

/* Shut a single game display. */

close_display(side)
Side *side;
{
	dside->ui->active = FALSE;
	/* And close some windows? */
	Dprintf("Mac display closed.\n");
	/* should attempt to release all memory. */
}

/* (should get name from a preference) */

char *
checkpoint_name()
{
	return "Checkpoint";
}

update_everything()
{
	if (active_display(dside)) {
		force_overall_update();
	}
}

setfiletype(char *name)
{
#ifdef THINK_C
	FileParam pb;
	Str255 tmpstr;
	
	c2p(name, tmpstr);
	pb.ioNamePtr = tmpstr;
	pb.ioVRefNum = 0;
	pb.ioFVersNum = 0;
	pb.ioFDirIndex = 0;
	if (PBGetFInfoSync(&pb) == noErr) {
		pb.ioFlFndrInfo.fdType = 'TEXT';
		pb.ioFlFndrInfo.fdCreator = 'XCNQ';
		PBSetFInfoSync(&pb);
	}
#endif
}

/* Returns true if we can connect to another game. */

can_connect()
{
	return TRUE;
}