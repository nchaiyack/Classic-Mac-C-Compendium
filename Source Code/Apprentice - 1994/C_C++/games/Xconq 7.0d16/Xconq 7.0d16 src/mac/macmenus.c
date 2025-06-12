/* Copyright (c) 1992, 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Mac menu handling. */

#include "conq.h"
#include "mac.h"

#undef DEF_CMD
#define DEF_CMD(letter,oldletter,name,args,FN,help) int FN();

#include "cmd.def"

/* The following is generic code. */

int constructioneverpossible = -1;

construction_ever_possible()
{
	int u, u2;

	if (constructioneverpossible < 0) {
		constructioneverpossible = FALSE;
		for_all_unit_types(u) {
			for_all_unit_types(u2) {
				if (uu_acp_to_create(u, u2) > 0) {
					constructioneverpossible = TRUE;
					return constructioneverpossible;
				}
			}
		}
	}
	return constructioneverpossible;
}

extern Feature *featurelist;

extern int gameinited, beforestart, endofgame;

#include <profile.h>

int Profile = FALSE;

#define SetMenuItemText SetItem
#define InsertMenuItem InsMenuItem

#define side_may_select(unit) (in_play(unit) && ((unit)->side == dside || dside->designer))

#define valid_selection(unit) (in_play(unit) && ((unit)->side == dside || dside->designer))

#define menus_tweakable() ((!beforestart && !endofgame) || (gameinited && dside->designer))

#define MAXWINDOWS 100

extern int gamestatesafe;
extern int interfacestatesafe;

char *cursavename = NULL;

MenuHandle mapviewmenu = nil;
MenuHandle listviewmenu = nil;
MenuHandle closeupviewmenu = nil;

MenuHandle utypemenu = nil;
MenuHandle ttypemenu = nil;
MenuHandle mtypemenu = nil;
MenuHandle sidemenu = nil;
MenuHandle aitypemenu = nil;

MenuHandle featuremenu = nil;

WindowPtr *winmenuwins;

int numwindows;

Map *worldmap = NULL;

WindowPtr worldmapwin = nil;

/* Set up all the menus, at least in their basic form. */

init_menus()
{
	int i;
	Handle menubar;
	MenuHandle menu;

	menubar = GetNewMBar(mbMain);
	SetMenuBar(menubar);
	/* Add the DAs etc as usual. */
	if ((menu = GetMHandle(mApple)) != nil) {
		AddResMenu(menu, 'DRVR');
	}
	/* Get the different versions of the view menu. */
	mapviewmenu = GetMenu(mViewMap);
	listviewmenu = GetMenu(mViewList);
	closeupviewmenu = GetMenu(mViewCloseup);
	/* Init submenus and popups. */
	if ((menu       = GetMenu(mViewFontSizes)) != nil)  InsertMenu(menu, -1);
	if ((menu       = GetMenu(mMagnifications)) != nil) InsertMenu(menu, -1);
	if ((menu       = GetMenu(mViewWeather)) != nil)    InsertMenu(menu, -1);
	if ((sidemenu   = GetMenu(mSides)) != nil)          InsertMenu(sidemenu, -1);
	if ((utypemenu  = GetMenu(mUnitTypes)) != nil)      InsertMenu(utypemenu, -1);
	if ((mtypemenu  = GetMenu(mMaterialTypes)) != nil)  InsertMenu(mtypemenu, -1);
	if ((ttypemenu  = GetMenu(mTerrainTypes)) != nil)   InsertMenu(ttypemenu, -1);
	if ((aitypemenu = GetMenu(mAITypes)) != nil)        InsertMenu(aitypemenu, -1);
	if ((featuremenu = GetMenu(mFeatures)) != nil)      InsertMenu(featuremenu, -1);
	/* Init the support for the Windows menu. */
	winmenuwins = (WindowPtr *) xmalloc(MAXWINDOWS * sizeof(WindowPtr));
	for (i = 0; i < MAXWINDOWS; ++i) {
		winmenuwins[i] = nil;
	}
	numwindows = 0;
	/* Done fiddling with menus, draw them. */
	DrawMenuBar();
}

/* Add a menu item that can be used to go to the given window.  Set the window title
   here too, ensures that the two match. */

add_window_menu_item(name, win)
char *name;
WindowPtr win;
{
	int i;
	MenuHandle menu;
	Str255 tmpstr;

	if (numwindows < MAXWINDOWS) {
		c2p(name, tmpstr);
		SetWTitle(win, tmpstr);
		winmenuwins[numwindows++] = win;
		if ((menu = GetMHandle(mWindows)) != nil) {
			AppendMenu(menu, tmpstr);
		}
	}
}

/* Remove the menu item that can be used to select a given window. */

remove_window_menu_item(win)
WindowPtr win;
{
	int i, found = FALSE, next = 0;
	Str255 tmpstr;
	MenuHandle menu;

	/* Search for the window and remove it from the array of windows
	   that have menu items. */
	for (i = 0; i < numwindows; ++i) {
		if (!found && winmenuwins[i] == win) {
			found = TRUE;
		}
		if (found && i < numwindows - 1) {
			/* Shift the other windows down. */
			winmenuwins[i] = winmenuwins[i + 1];
		}
	}
	/* This routine gets called for all windows, so get out of here
	   if this window is not one of those in the menu. */
	if (!found) return;
	--numwindows;
	/* Trash the existing window menu. */
	if ((menu = GetMHandle(mWindows)) != nil) {
		DeleteMenu(mWindows);
		ReleaseResource(menu);
	}
	/* Build a new version starting with the resource. */
	if ((menu = GetMenu(mWindows)) != nil) {
		for (i = 0; i < numwindows; ++i) {
			GetWTitle(winmenuwins[i], tmpstr);
			AppendMenu(menu, tmpstr);
		}
		if (GetMHandle(mViewMap) != nil) next = mViewMap;
		if (GetMHandle(mViewList) != nil) next = mViewList;
		InsertMenu(menu, next);
	}
	DrawMenuBar();
}

/* Fill up the side menu. */

build_side_menu()
{
	Side *side2;
	Str255 tmpstr;

	if (sidemenu != nil && CountMItems(sidemenu) < 1) {
		for_all_sides(side2) {
			shortest_side_title(side2, spbuf);
			sanitize_for_menu(spbuf);
			c2p(spbuf, tmpstr);
			AppendMenu(sidemenu, tmpstr);
			EnableItem(sidemenu, side_number(side2));
		}
		if (1 /* independent units possible in this game */) {
			AppendMenu(sidemenu, "\pindependent");
		}
	}
}

update_side_menu(side2)
Side *side2;
{
	Str255 tmpstr;

	shortest_side_title(side2, spbuf);
	sanitize_for_menu(spbuf);
	c2p(spbuf, tmpstr);
	SetMenuItemText(sidemenu, side_number(side2), tmpstr);
}

build_unit_type_menu()
{
	int u;
	Str255 tmpstr;

	if (utypemenu != nil && CountMItems(utypemenu) < 1) {
		for_all_unit_types(u) {
			strcpy(spbuf, u_type_name(u));
			sanitize_for_menu(spbuf);
			c2p(spbuf, tmpstr);
			AppendMenu(utypemenu, tmpstr);
			EnableItem(utypemenu, u + 1);
		}
	}
}

build_material_type_menu()
{
	int m;
	Str255 tmpstr;

	if (mtypemenu != nil && CountMItems(mtypemenu) < 1 && nummtypes > 0) {
		for_all_material_types(m) {
			strcpy(spbuf, m_type_name(m));
			sanitize_for_menu(spbuf);
			c2p(spbuf, tmpstr);
			AppendMenu(mtypemenu, tmpstr);
			EnableItem(mtypemenu, m + 1);
		}
	}
}

build_terrain_type_menu()
{
	int t;
	Str255 tmpstr;

	if (ttypemenu != nil && CountMItems(ttypemenu) < 1) {
		for_all_terrain_types(t) {
			strcpy(spbuf, t_type_name(t));
			sanitize_for_menu(spbuf);
			c2p(spbuf, tmpstr);
			AppendMenu(ttypemenu, tmpstr);
			EnableItem(ttypemenu, t);
		}
	}
}

build_ai_type_menu()
{
	if (aitypemenu != nil && CountMItems(aitypemenu) < 1) {
		AppendMenu(aitypemenu, "\pNone");
		EnableItem(aitypemenu, 1);
		AppendMenu(aitypemenu, "\pMplayer");
		EnableItem(aitypemenu, 2);
	}
}

build_feature_menu()
{
	int i = 1;
	Str255 tmpstr;
	Feature *feature;

	if (featuremenu != nil && CountMItems(featuremenu) < 1) {
		AppendMenu(featuremenu, "\pNo Feature");
		EnableItem(featuremenu, i++);
    	for (feature = featurelist; feature != NULL; feature = feature->next) {
    		feature_desc(feature, spbuf);
			sanitize_for_menu(spbuf);
			c2p(spbuf, tmpstr);
    		AppendMenu(featuremenu, tmpstr);
    		EnableItem(featuremenu, i++);
    	}
	}
}

/* Alter the feature menu to reflect a changed or new feature. */

update_feature_menu(feature)
Feature *feature;
{
	Str255 tmpstr;

    feature_desc(feature, spbuf);
	sanitize_for_menu(spbuf);
	c2p(spbuf, tmpstr);
	if (feature->id + 1 > CountMItems(featuremenu)) {
		AppendMenu(featuremenu, tmpstr);
		EnableItem(featuremenu, feature->id + 1);
	} else {
		SetMenuItemText(featuremenu, feature->id + 1, tmpstr);
	}
}

/* This removes chars that are specially recognized by the Menu Manager.
   Has to be done so that strange game-defined names don't scramble the
   menus; innocuous because this affects only the appearance in the menu. */

sanitize_for_menu(buf)
char *buf;
{
	int i;

	/* Replace a leading hyphen with an underscore. */
	if (buf[0] == '-') buf[0] = '_';
	/* Replace special chars with blanks. */
	for (i = 0; buf[i] != '\0'; ++i) {
		switch (buf[i]) {
			case ';':
			case '!':
			case '<':
			case '/':
			case '(':
			/* I don't think closing parens are special, but since the
			   (presumed) matching open paren is gone, might as well get
			   rid of the close also. */
			case ')':
				buf[i] = ' ';
				break;
			default:
				break;
		}
	}
}

/* Decipher and do a menu command. */

do_menu_command(which)
long which;
{
	short menuid, menuitem, itemHit, daRefNum, ditem, i;
	Str255 daname;
	Boolean handledbyda;
	WindowPtr win;

	menuid = HiWord(which);
	menuitem = LoWord(which);
	DGprintf("menu %d, item %d\n", menuid, menuitem);
	switch (menuid) {
		case mApple:
			switch (menuitem) {
				case miAppleAbout:
					do_about_box();
					break;
				case miAppleHelp:
					help_dialog();
					break;
				default:
					GetItem(GetMHandle(mApple), menuitem, daname);
					daRefNum = OpenDeskAcc(daname);
			}
			break;
		case mFile:
			switch (menuitem) {
				case miFileNew:
					/* (should reset state of everything first) */
					new_game_dialog();
					break;
				case miFileOpen:
					/* (should reset state of everything first) */
					open_game_dialog();
					break;
				case miFileConnect:
					connect_game_dialog();
					break;
				case miFileSave:
					save_the_game(FALSE, FALSE);
					break;
				case miFileSaveAs:	
					save_the_game(TRUE, FALSE);
					break;
				case miFilePreferences:
					set_preferences();
					break;
				case miFilePageSetup:
					do_page_setup_mi();
					break;
				case miFilePrintWindow:
					do_print_mi();
					break;
				case miFileQuit:
					quit_the_game();
					break;
			}
			break;
		case mEdit:
			/* handledbyda = SystemEdit(menuitem-1); */
			switch (menuitem)  {
				case miEditCut:
					/* disband_selection(); */
					/* (should depend on whether this is a map or a text field) */
					break;
				case miEditSelectAll:
					do_select_all_mi();
					break;
#ifdef DESIGNERS
				case miEditDesign:
					if (!dside->designer) {
						enable_designing(FALSE);
					} else {
						disable_designing();
					}
					break;
#endif
			}
			break;
		case mFind:
			switch (menuitem) {
				case miFindPrevious:
					do_find_previous_mi();
					break;
				case miFindNext:
					do_find_next_mi();
					break;
				case miFindLocation:
					do_find_location_mi();
					break;
				case miFindUnitByName:
					do_find_unit_by_name_mi();
					break;
				case miFindSelected:
					do_find_selected_mi();
					break;
			}
			break;
		case mPlay:
			switch (menuitem) {
				case miPlayCloseup:
					do_closeup_mi();
					break;
				case miPlayMove:
					do_move_mi();
					break;
				case miPlayPatrol:
					do_patrol_mi();
					break;
				case miPlayReturn:
					do_return_mi();
					break;
				case miPlayWake:
					do_wake_mi();
					break;
				case miPlaySleep:
					do_sleep_mi();
					break;
				case miPlayDoneMoving:
					do_done_moving_mi();
					break;
				case miPlayBuild:
					do_construction_mi();
					break;
				case miPlayRepair:
					do_repair_mi();
					break;
				case miPlayAttack:
					do_attack_mi();
					break;
				case miPlayOverrun:
					do_overrun_mi();
					break;
				case miPlayFire:
					do_fire_mi();
					break;
				case miPlayDetonate:
					do_detonate_mi();
					break;
				case miPlayTake:
					break;
				case miPlayDrop:
					break;
				case miPlayGive:
					/* has submenu mSides */
					break;
				case miPlayDisband:
					do_disband_mi();
					break;
				case miPlayRename:
					do_rename_mi();
					break;
			}
			break;
		case mSides:
			do_give_unit_mi(menuitem);
			break;
		case mSide:
			switch (menuitem) {
				case miSideCloseup:
					break;
				case miSideFinishedTurn:
					finish_turn(dside);
					break;
				case miSideMoveOnClick:
					moveonclick_mi();
					break;
				case miSideAutoSelect:
					autoselect_mi();
					break;
				case miSideAutoFinish:
					/* Toggle auto-finish for turns. */
					set_autofinish(dside, !dside->autofinish);
					break;
				case miSideSound:
					playsounds = !playsounds;
					break;
				case miSideRename:
					side_rename_dialog(dside);
					break;
			}
			break;
		case mWindows:
			switch (menuitem) {
				case miWindowsGame:
					if (gamewin == nil) {
						create_game_window();
					}
					ShowWindow(gamewin);
					SelectWindow(gamewin);
					break;
				case miWindowsHistory:
					if (historywin == nil) {
						create_history_window();
					}
					ShowWindow(historywin);
					SelectWindow(historywin);
					break;
				case miWindowsConstruction:
					enable_construction();
					break;
				/* should have agreements list etc handling here also */
				case miWindowsNewList:
					create_list();
					break;
				case miWindowsNewMap:
					create_map(5 /* should be a pref */);
					break;
				case miWindowsWorldMap:
					if (worldmapwin == nil) {
						int power, winwid = 200;

						for (power = 0; power < NUMPOWERS; ++power) {
							if (area.width * hws[power] <= winwid
								&& area.width * hws[power+1] > winwid) break;
						}
						worldmap = create_map(power);
						worldmap->drawothermaps = TRUE;
						/* (should be in survey mode too?) */
						worldmapwin = worldmap->window;
					}
					ShowWindow(worldmapwin);
					SelectWindow(worldmapwin);
					break;
				default:
					win = winmenuwins[menuitem - miWindowsFirst];
					if (win != nil) {
						SelectWindow(win);
					}
					break;
			}
			break;
		case mViewMap:
			switch (menuitem)  {
				case miViewCloser:
					magnify_map(map_from_window(FrontWindow()), 1);
					break;
				case miViewFarther:
					magnify_map(map_from_window(FrontWindow()), -1);
					break;
				case miViewNames:
					toggle_map_names(map_from_window(FrontWindow()));
					break;
				case miViewPeople:
					toggle_map_people(map_from_window(FrontWindow()));
					break;
				case miViewElevations:
					toggle_map_elevations(map_from_window(FrontWindow()));
					break;
				case miViewDaylight:
					toggle_map_lighting(map_from_window(FrontWindow()));
					break;
				case miViewGrid:
					toggle_map_grid(map_from_window(FrontWindow()));
					break;
				case miViewTopline:
					toggle_map_topline(map_from_window(FrontWindow()));
					break;
				case miViewOtherMaps:
					toggle_map_other_maps(map_from_window(FrontWindow()));
					break;
			}
			break;
		case mViewWeather:
			switch (menuitem)  {
				case miWeatherTemperature:
					toggle_map_temperature(map_from_window(FrontWindow()));
					break;
				case miWeatherWinds:
					toggle_map_winds(map_from_window(FrontWindow()));
					break;
				case miWeatherClouds:
					toggle_map_clouds(map_from_window(FrontWindow()));
					break;
				case miWeatherStorms:
					toggle_map_storms(map_from_window(FrontWindow()));
					break;
			}
			break;
		case mViewList:
			switch (menuitem)  {
				case miViewByType:
					set_list_sorting(list_from_window(FrontWindow()), bytype, menuitem);
					break;
				case miViewByName:
					set_list_sorting(list_from_window(FrontWindow()), byname, menuitem);
					break;
				case miViewBySide:
					set_list_sorting(list_from_window(FrontWindow()), byside, menuitem);
					break;
				case miViewByActOrder:
					set_list_sorting(list_from_window(FrontWindow()), byactorder, menuitem);
					break;
				case miViewByLocation:
					set_list_sorting(list_from_window(FrontWindow()), bylocation, menuitem);
					break;
				case miViewIconSize:
					toggle_list_large_icons(list_from_window(FrontWindow()));
					break;
			}
			break;
		case mAITypes:
			switch (menuitem) {
				case 1:
					if (side_has_ai(dside)) {
						set_side_ai(dside, NULL);
					}
					break;
				case 2:
					if (!side_has_ai(dside)) {
						set_side_ai(dside, "mplayer");
					}
					break;
			}
			break;
		case mMagnifications:
			set_map_mag(map_from_window(FrontWindow()), menuitem - 1);
			break;
		case mMaterialTypes:
			toggle_map_materials(map_from_window(FrontWindow()), menuitem - 1);
			break;
	}
	HiliteMenu(0);
}

/* Display the "About..." box. */

/* This dialog does double duty - initially it chooses New/Open/Connect/Quit,
   later on it's just a normal About box. */

do_about_box()
{
	short ditem;
	Str255 tmpstr;
	WindowPtr win;
	PicHandle pic;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	win = GetNewDialog(dAbout, NULL, (DialogPtr) -1L);
	/* Fill in the kernel's version and copyright. */
	GetDItem(win, diAboutVersion, &itemtype, &itemhandle, &itemrect);
	c2p(version_string(), tmpstr);
	SetIText(itemhandle, tmpstr);
	GetDItem(win, diAboutCopyright, &itemtype, &itemhandle, &itemrect);
	c2p(copyright_string(), tmpstr);
	SetIText(itemhandle, tmpstr);
	/* Substitute a color picture if possible. */
	if (hasColorQD) {
		GetDItem(win, diAboutPicture, &itemtype, &itemhandle, &itemrect);
		if ((pic = (PicHandle) GetResource('PICT', pSplashColor)) != nil) {
			SetDItem(win, diAboutPicture, itemtype, (Handle) pic, &itemrect);
		}
	}
	ShowWindow(win);
	SelectWindow(win);
	draw_default_button(win, diAboutOK);
	ModalDialog(NULL, &ditem);
	/* Just return, no matter what input. */
	HideWindow(win);
	DisposDialog(win);
}

/* This routine does both "save" and "save as". */

save_the_game(askname, quitting)
int askname, quitting;
{
	extern CursHandle watchcursor;

#ifdef DESIGNERS
	/* Do the selective save only if we're not shutting down Xconq. */
	if (dside->designer && !quitting) {
		designer_save_dialog();
	} else
#endif
	if (askname || cursavename == NULL) {
		if (cursavename == NULL) cursavename = savefile_name();
		/* should bring up dialog to choose name */
		/* Do the save proper. */
		SetCursor(*watchcursor);
		write_entire_game_state(cursavename);
		setfiletype(cursavename);
	} else {
		/* Just save to the same file as before. */
		SetCursor(*watchcursor);
		write_entire_game_state(cursavename);
		setfiletype(cursavename);
	}
/*	adjust_cursor(?, ?); */
}

set_preferences()
{
	short done = FALSE, ditem;
	DialogPtr win;
	short itemtype;  Handle itemhandle;  Rect itemrect;
	extern int checkpointinterval;

	/* (should have gotten prefs from file already) */
	win = GetNewDialog(dPreferences, NULL, (DialogPtr) -1L);
	/* Set the current preferences into the items. */
	GetDItem(win, diPrefsGrid, &itemtype, &itemhandle, &itemrect);
	SetCtlValue((ControlHandle) itemhandle, defaultdrawgrid);
	GetDItem(win, diPrefsNames, &itemtype, &itemhandle, &itemrect);
	SetCtlValue((ControlHandle) itemhandle, defaultdrawnames);
	GetDItem(win, diPrefsCheckpoint, &itemtype, &itemhandle, &itemrect);
	SetCtlValue((ControlHandle) itemhandle, (checkpointinterval > 0));
	ShowWindow(win);
	while (!done) {
		ModalDialog(NULL, &ditem);
		switch (ditem) {
			case diPrefsOK:
				/* Actually change the program's variables. */
				GetDItem(win, diPrefsGrid, &itemtype, &itemhandle, &itemrect);
				defaultdrawgrid = GetCtlValue((ControlHandle) itemhandle);
				GetDItem(win, diPrefsNames, &itemtype, &itemhandle, &itemrect);
				defaultdrawnames = GetCtlValue((ControlHandle) itemhandle);
				GetDItem(win, diPrefsCheckpoint, &itemtype, &itemhandle, &itemrect);
				checkpointinterval = (GetCtlValue((ControlHandle) itemhandle) ? 5 : 0);
				/* Fall into next case. */
			case diPrefsCancel:
				done = TRUE;
				break;
			case diPrefsGrid:
			case diPrefsNames:
				/* Toggle check boxes. */
				GetDItem(win, ditem, &itemtype, &itemhandle, &itemrect);
				SetCtlValue((ControlHandle) itemhandle, !GetCtlValue((ControlHandle) itemhandle));
				break;
			case diPrefsCheckpoint:
				GetDItem(win, ditem, &itemtype, &itemhandle, &itemrect);
				SetCtlValue((ControlHandle) itemhandle, !GetCtlValue((ControlHandle) itemhandle));
				/* Should enable/disable interval-setting ditems. */
				break;
			case diPrefsInterval:
				break;
		}
	}
	DisposDialog(win);
}

/*THPrint printrecordhandle = nil;*/

maybe_init_print()
{
/*	if (printrecordhandle == nil) {
		printrecordhandle = (THPrint) NewHandle(sizeof(TPrint));
		PrOpen();
		PrintDefault(printrecordhandle);
	} */
}

do_page_setup_mi()
{
	maybe_init_print();
/*	PrStlDialog(printrecordhandle); */
}

/* Handle printing.  What gets printed depends on which window is in front. */

do_print_mi()
{
	Map *map;
	List *list;

	maybe_init_print();
/*	if (!PrJobDialog(printrecordhandle)) return; */
	if ((map = map_from_window(FrontWindow())) != NULL) {
		print_map(map);
	} else if ((list = list_from_window(FrontWindow())) != NULL) {
		print_list(list);
	} else {
		/* (should at least be able to print game desc) */
	}
}

/* Attempt to quit. */

/* (should warn if quitting is tantamount to resigning) */
/* (should close any open windows left) */

quit_the_game()
{
	extern int gamestatesafe, interfacestatesafe;
	extern int endofgame;
	Boolean closed;
	WindowPtr win;

	if (endofgame) {
		/* Game is over, nothing to save. */
		ExitToShell();
	} else if (!gamestatesafe || !interfacestatesafe) {
		switch (CautionAlert(aQuitGame, nil)) {
			case aiQuitGameOK:
				/* (should maybe do game resignation, update other sides) */
				SetCursor(*watchcursor);
				save_the_game(FALSE, TRUE);
				SetCursor(&qd.arrow);
				ExitToShell();
				break;
			case aiQuitGameCancel:
				break;
			case aiQuitGameDontSave:
#if 0
	switch (CautionAlert(aResignGame, nil)) {
		case aiResignOK:
			/* (should do game resignation, update other sides) */
			break;
		case aiResignCancel:
			break;
	}
#endif
				ExitToShell();
				break;
		}
	} else {
		/* (should still confirm that we're not avoiding our fate...) */
		ExitToShell();
	}
}

do_select_all_mi()
{
	int i;
	Map *map;
	Unit *unit;
	Side *side;
	
	if ((map = map_from_window(FrontWindow())) != NULL) {
		for_all_units(side, unit) {
			if (side_may_select(unit)) {
				select_unit_on_map(map, unit);
			}
		}
		/* Do all the drawing at once. */
		draw_selections(map);
	}
}

do_find_previous_mi()
{
	Map *map;
	
	if ((map = map_from_window(FrontWindow())) != NULL) {
		select_previous_unit(map);
	}
}

do_find_next_mi()
{
	Map *map;
	
	if ((map = map_from_window(FrontWindow())) != NULL) {
		select_next_mover(map);
	}
}

/* (should ask for x,y coords, focus/center on that position) */

do_find_location_mi()
{
	Map *map;
	
	if ((map = map_from_window(FrontWindow())) != NULL) {
	}
}

/* (should ask for a textual name/select from a gazetteer, scroll to that unit) */

do_find_unit_by_name_mi()
{
	Map *map;
	
	if ((map = map_from_window(FrontWindow())) != NULL) {
	}
}

do_find_selected_mi()
{
	Map *map;
	List *list;
	
	if ((map = map_from_window(FrontWindow())) != NULL) {
	} else if ((list = list_from_window(FrontWindow())) != NULL) {
		scroll_to_selected_unit_in_list(list);
	} else {
		/* nothing to do */
	}
}

/* Menu items/commands for unit play action. */

apply_to_all_selected(fn)
int (*fn)();
{
	int i, numcould = 0, numnot = 0;
	Map *map;  List *list;  UnitCloseup *closeup;
	Unit *unit;

	if (fn == NULL) return;
	if ((map = map_from_window(FrontWindow())) != NULL) {
		for (i = 0; i < map->numselections; ++i) {
			if ((unit = map->selections[i]) != NULL) {
				(*fn)(unit);
			}
		}
	} else if ((list = list_from_window(FrontWindow())) != NULL) {
		if ((unit = (Unit *) selected_unit_in_list(list)) != NULL) {
			(*fn)(unit);
		}
	} else if ((closeup = unit_closeup_from_window(FrontWindow())) != NULL) {
		if ((unit = closeup->unit) != NULL) {
			(*fn)(unit);
		}
	}
}

do_closeup_mi()
{
	int i;
	Map *map;  List *list;  UnitCloseup *unitcloseup;
	Unit *unit;
	
	if ((map = map_from_window(FrontWindow())) != NULL) {
		for (i = 0; i < map->numselections; ++i) {
			if ((unit = map->selections[i]) != NULL) {
				if ((unitcloseup = find_unit_closeup(unit)) != NULL) {
					ShowWindow(unitcloseup->window);
					SelectWindow(unitcloseup->window);
				} else {
					create_unit_closeup(unit);
				}
			}
		}
	} else if ((list = list_from_window(FrontWindow())) != NULL) {
		if ((unit = (Unit *) selected_unit_in_list(list)) != NULL) {
			if ((unitcloseup = find_unit_closeup(unit)) != NULL) {
				ShowWindow(unitcloseup->window);
				SelectWindow(unitcloseup->window);
			} else {
				create_unit_closeup(unit);
			}
		}
	}
}

do_move_mi()
{
	SysBeep(20);
}

do_patrol_mi()
{
	SysBeep(20);
}

do_one_return(unit)
Unit *unit;
{
	set_resupply_task(unit);
}

do_return_mi()
{
	apply_to_all_selected(do_one_return);	
	/* If everybody that was asked to do the action couldn't, beep once. */
	/* if (numcould == 0 && numnot > 0) SysBeep(20); */
}

do_wake_mi()
{
	do_reserve_command(FALSE, FALSE);
	do_sleep_command(FALSE, FALSE);
}

do_sleep_mi()
{
	do_sleep_command(TRUE, FALSE);
}

do_done_moving_mi()
{
	do_reserve_command(TRUE, FALSE);
}

int tmpcmdarg;
int tmprecurse;

do_one_reserve(unit)
Unit *unit;
{
	set_unit_reserve(dside, unit, tmpcmdarg, tmprecurse);
}

do_reserve_command(value, recurse)
int value, recurse;
{
	tmpcmdarg = value;
	tmprecurse = recurse;
	apply_to_all_selected(do_one_reserve);	
}

do_one_asleep(unit)
Unit *unit;
{
	set_unit_asleep(dside, unit, tmpcmdarg, tmprecurse);
}

do_sleep_command(value, recurse)
int value, recurse;
{
	tmpcmdarg = value;
	tmprecurse = recurse;
	apply_to_all_selected(do_one_asleep);	
}

do_one_clear_plan(unit)
Unit *unit;
{
	if (unit->plan) {
		set_unit_plan_type(dside, unit, PLAN_NONE);
	}
}

do_clear_plan_command()
{
	apply_to_all_selected(do_one_clear_plan);	
}

do_one_ai_control(unit)
Unit *unit;
{
	if (unit->plan) {
		set_unit_ai_control(dside, unit, !unit->plan->aicontrol);
	}
}

do_ai_control_command()
{
	apply_to_all_selected(do_one_ai_control);	
}

do_construction_mi()
{
	int i;
	Map *map;  List *list;  UnitCloseup *closeup;
	Unit *unit;
	
	if ((map = map_from_window(FrontWindow())) != NULL) {
		for (i = 0; i < map->numselections; ++i) {
			if ((unit = map->selections[i]) != NULL) {
				if (can_build(unit)) {  /* and completers and researchers too? */
					enable_construction();
					select_unit_in_construction_window(unit);
					return;
				}
			}
		}
	} else if ((list = list_from_window(FrontWindow())) != NULL) {
		if ((unit = (Unit *) selected_unit_in_list(list)) != NULL) {
				if (can_build(unit)) {  /* and completers and researchers too? */
					enable_construction();
					select_unit_in_construction_window(unit);
					return;
				}
		}
	} else if ((closeup = unit_closeup_from_window(FrontWindow())) != NULL) {
		if ((unit = closeup->unit) != NULL) {
				if (can_build(unit)) {  /* and completers and researchers too? */
					enable_construction();
					select_unit_in_construction_window(unit);
					return;
				}
		}
	}
}

/* Create and/or bring up the construction planning window. */

enable_construction()
{
	if (constructionwin == nil) {
		create_construction_window();
	}
	if (constructionwin != nil) {
		reinit_construction_lists();
		ShowWindow(constructionwin);
		SelectWindow(constructionwin);
	}
}

do_repair_mi()
{
	apply_to_all_selected(NULL);	
}

/* (should find one unit to attack, do modal dialog if != 1) */

do_attack_mi()
{
	int i;
	Map *map;
	Unit *atker, *other;
	Side *side;
	
	if ((map = map_from_window(FrontWindow())) != NULL) {
		for (i = 0; i < map->numselections; ++i) {
			if ((atker = map->selections[i]) != NULL) {
				for_all_units(side, other) {
					if (other != atker
						&& other->side != atker->side
						&& distance(atker->x, atker->y, other->x, other->y) == 1) {
							prep_attack_action(atker, atker, other, 100);
							break;
					}
				}
			}
		}
	}
}

do_attack_command()
{
	int i, x, y, numcould = 0, numnot = 0;
	Point target;
	Map *map;
	Unit *atker, *other;
	Side *side;
	
	if ((map = map_from_window(FrontWindow())) != NULL) {
		GetMouse(&target);
		nearest_unit(map, target.h, target.v, &other);
		if (other != NULL) {
			for (i = 0; i < map->numselections; ++i) {
				if ((atker = map->selections[i]) != NULL) {
					if (other != atker
						&& other->side != atker->side) {
						if (distance(atker->x, atker->y, other->x, other->y) <= 1
							&& valid(check_attack_action(atker, atker, other, 100))) {
							prep_attack_action(atker, atker, other, 100);
							++numcould;
						} else if (valid(check_fire_at_action(atker, atker, other, -1))) {
							prep_fire_at_action(atker, atker, other, -1);
							++numcould;
						} else {
							++numnot;
						}
					} else {
					}
				}
			}
		} else {
			/* (should let units fire into empty cell) */
		}
	}
	/* If nobody could do the action, beep once. */
	if (numcould == 0 && numnot > 0) SysBeep(20);
}

do_overrun_mi()
{
	/* (should loop until mouse clicked in map) */
	SysBeep(20);
}

do_fire_mi()
{
	if (modaltool == 0) {
		modaltool = 2;
	}
}

do_fire_command()
{
	int i, x, y, numcould = 0, numnot = 0;
	Point target;
	Map *map;
	Unit *atker, *other;
	Side *side;
	
	if ((map = map_from_window(FrontWindow())) != NULL) {
		GetMouse(&target);
		nearest_unit(map, target.h, target.v, &other);
		if (other != NULL) {
			for (i = 0; i < map->numselections; ++i) {
				if ((atker = map->selections[i]) != NULL && valid_selection(atker)) {
					if (other != atker
						&& other->side != atker->side
						&& valid(check_fire_at_action(atker, atker, other, -1))) {
						prep_fire_at_action(atker, atker, other, -1);
						++numcould;
					} else {
						++numnot;
					}
				}
			}
		} else {
			/* (should let units fire into empty cell) */
		}
	}
	/* If nobody could do the action, beep once. */
	if (numcould == 0 && numnot > 0) {
		SysBeep(20);
		return FALSE;
	} else {
		return TRUE;
	}
}

/* (should decide how to detonate based on selections and modal input) */

do_one_detonate(unit)
Unit *unit;
{
	prep_detonate_action(unit, unit, unit->x, unit->y, unit->z);
}

do_detonate_mi()
{
	apply_to_all_selected(do_one_detonate);
}

/* This command samples the current mouse to get the desired attack location. */

do_detonate_command()
{
	int i, x, y;
	Point detpoint;
	Map *map;
	Unit *unit;
	
	if ((map = map_from_window(FrontWindow())) != NULL) {
		GetMouse(&detpoint);
		nearest_cell(map, detpoint.h, detpoint.v, &x, &y);
		for (i = 0; i < map->numselections; ++i) {
			if ((unit = map->selections[i]) != NULL) {
				prep_detonate_action(unit, unit, x, y, unit->z);
			}
		}
	}
}

Side *tmpcmdside;

do_one_give_unit(unit)
Unit *unit;
{
	if (unit->side == tmpcmdside) return;
#ifdef DESIGNERS
	if (dside->designer) {
		unit_changes_side(unit, tmpcmdside, -1, -1);
		update_unit_display(dside, unit, TRUE);
	} else
#endif /* DESIGNERS */
	{
		prep_change_side_action(unit, unit, tmpcmdside);
	}
}

do_give_unit_mi(mi)
int mi;
{
	Side *side = side_n(mi); /* note that side_n returns NULL for mi == numsides */
	
	apply_to_all_selected(do_one_give_unit);
}

do_one_disband(unit)
Unit *unit;
{
	/* (should call a designer_disband routine) */
#ifdef DESIGNERS
	if (dside->designer) {
		kill_unit(unit, -1);
	} else
#endif /* DESIGNERS */
	if (!completed(unit)) {
		kill_unit(unit, H_UNIT_DISBANDED);
	} else if (unit->act && unit->act->acp > 0) {
		prep_disband_action(unit, unit);
	} else {
		/* try to find a nearby unit to do it */
	}
}

do_disband_mi()
{
	apply_to_all_selected(do_one_disband);
}

do_one_rename(unit)
Unit *unit;
{
	unit_rename_dialog(unit);
}

/* This just renames the first of the selected units. */

do_rename_mi()
{
	apply_to_all_selected(do_one_rename);
}

moveonclick_mi()
{
	Map *frontmap = map_from_window(FrontWindow());

	if (frontmap) {
		frontmap->moveonclick = !frontmap->moveonclick;
		/* (should force updates of map windows to reflect?) */
	} else {
		defaultmoveonclick = !defaultmoveonclick;
	}
}

autoselect_mi()
{
	Map *frontmap = map_from_window(FrontWindow());
	extern Unit *curunit;

	if (frontmap) {
		frontmap->autoselect = !frontmap->autoselect;
		frontmap->curunit = NULL; /* maybe not a good idea? */
		/* (should force updates of map windows to reflect?) */
	} else {
		defaultautoselect = !defaultautoselect;
	}
}

adjust_menu_item(menu, item, value)
MenuHandle menu;
int item, value;
{
	if (value) {
		EnableItem(menu, item);
	} else {
		DisableItem(menu, item);
	}
}

/* The state of menu items changes to reflect selected units, etc. */

adjust_menus()
{
	int i, m, nextmi, numwins;
	MenuHandle menu;
	Map *frontmap;
	List *frontlist;
	UnitCloseup *frontcloseup;
	Unit *unit;

	frontmap = map_from_window(FrontWindow());
	frontlist = list_from_window(FrontWindow());
	frontcloseup = unit_closeup_from_window(FrontWindow());
	/* Certain menus need to be built after the game is running. */
	if (menus_tweakable()) {
		build_material_type_menu();
		build_side_menu();
		build_ai_type_menu();
	}
#if 0
	if (is_da_window(FrontWindow())) {
		if ((menu = GetMHandle(mEdit)) != nil) {
		}
		return;
	}
#endif
	if ((menu = GetMHandle(mFile)) != nil) {
		/* Availability of file menu items depends on whether we're in a game. */
		adjust_menu_item(menu, miFileNew, !gamedefined);
		adjust_menu_item(menu, miFileOpen, !gamedefined);
		adjust_menu_item(menu, miFileConnect, !gamedefined && can_connect());
		adjust_menu_item(menu, miFileSave, menus_tweakable());
		adjust_menu_item(menu, miFileSaveAs, menus_tweakable());
	}
	if ((menu = GetMHandle(mEdit)) != nil) {
		/* Edit menu is always available, but most items may be disabled. */
		/* (should do the other items also) */
		adjust_menu_item(menu, miEditSelectAll, menus_tweakable());
		if (menus_tweakable()) {
#ifdef DESIGNERS
			CheckItem(menu, miEditDesign, dside->designer);
#else
			DisableItem(menu, miEditDesign);
#endif /* DESIGNERS */
		}
	}
	if ((menu = GetMHandle(mFind)) != nil) {
		if (gameinited) {
			EnableItem(menu, 0);
			adjust_menu_item(menu, miFindPrevious, menus_tweakable());
			adjust_menu_item(menu, miFindNext, menus_tweakable());
			adjust_menu_item(menu, miFindLocation, menus_tweakable());
			adjust_menu_item(menu, miFindUnitByName, TRUE);
			adjust_menu_item(menu, miFindSelected, TRUE);
		} else {
			/* We're not even in a game yet, turn entire menu off. */
			DisableItem(menu, 0);
		}
	}
	if ((menu = GetMHandle(mPlay)) != nil) {
		if (menus_tweakable()) {
			EnableItem(menu, 0);
			/* Disable everything first, then selectively re-enable. */
			DisableItem(menu, miPlayCloseup);
			DisableItem(menu, miPlayMove);
			DisableItem(menu, miPlayPatrol);
			DisableItem(menu, miPlayReturn);
			DisableItem(menu, miPlayWake);
			DisableItem(menu, miPlaySleep);
			DisableItem(menu, miPlayDoneMoving);
			DisableItem(menu, miPlayBuild);
			DisableItem(menu, miPlayRepair);
			DisableItem(menu, miPlayAttack);
			DisableItem(menu, miPlayOverrun);
			DisableItem(menu, miPlayFire);
			DisableItem(menu, miPlayDetonate);
			DisableItem(menu, miPlayTake);
			DisableItem(menu, miPlayDrop);
			DisableItem(menu, miPlayGive);
			DisableItem(menu, miPlayDisband);
			DisableItem(menu, miPlayRename);
			/* Note that command enabling is accumulative for all units. */
			if (frontmap != NULL) {
				for (i = 0; i < frontmap->numselections; ++i) {
					if ((unit = frontmap->selections[i]) != NULL) {
						enable_commands_for_unit(menu, unit);
					}
				}
			} else if (frontlist != NULL) {
				if ((unit = (Unit *) selected_unit_in_list(frontlist)) != NULL) {
					enable_commands_for_unit(menu, unit);
				}
			} else if (frontcloseup != NULL) {
				if ((unit = frontcloseup->unit) != NULL) {
					enable_commands_for_unit(menu, unit);
				}
			}
		} else {
			/* We're not even in a valid game state yet, turn entire menu off. */
			DisableItem(menu, 0);
		}
	}
	if ((menu = GetMHandle(mSide)) != nil) {
		if (menus_tweakable()) {
			EnableItem(menu, 0);
			CheckItem(menu, miSideMoveOnClick,
					  (frontmap ? frontmap->moveonclick : defaultmoveonclick));
			CheckItem(menu, miSideAutoSelect,
					  (frontmap ? frontmap->autoselect : defaultautoselect));
			CheckItem(menu, miSideAutoFinish, dside->autofinish);
			CheckItem(menu, miSideSound, playsounds);
			if (dside->nameslocked) {
				DisableItem(menu, miSideRename);
			} else {
				EnableItem(menu, miSideRename);
			}
		} else {
			/* We're not even in a game yet, turn entire menu off. */
			DisableItem(menu, 0);
		}
	}
	if ((menu = GetMHandle(mWindows)) != nil) {
		if (gameinited) {
			EnableItem(menu, 0);
			/* Every item is always enabled. (should be, anyway) */
			CheckItem(menu, miWindowsGame,
				(gamewin && ((WindowPeek) gamewin)->visible));
			CheckItem(menu, miWindowsHistory,
				(historywin && ((WindowPeek) historywin)->visible));
			if (construction_ever_possible()) {
				EnableItem(menu, miWindowsConstruction);
				CheckItem(menu, miWindowsConstruction,
					(constructionwin && ((WindowPeek) constructionwin)->visible));
			} else {
				DisableItem(menu, miWindowsConstruction);
			}
			numwins = CountMItems(menu) - miWindowsFirst;
			for (i = 0; i < numwins; ++i) {
				CheckItem(menu, i + miWindowsFirst, (winmenuwins[i] == FrontWindow()));
			}
		} else {
			/* We're not even in a game yet, turn entire menu off. */
			DisableItem(menu, 0);
		}
	}
	/* If a map window is frontmost, install and adjust the map view menu. */
	if (frontmap != NULL) {
		/* Delete the list view menu if that's what's installed currently. */
		/* (should be able to flush any non-map-view menu) */
		if ((menu = GetMHandle(mViewList)) != nil) {
			DeleteMenu(mViewList);
		}
		/* Make sure the map view menu is installed (is always at the end). */
		InsertMenu(mapviewmenu, 0);
		if ((menu = GetMHandle(mViewMap)) != nil) {
			EnableItem(menu, 0);
			if (frontmap->power == 0) {
				DisableItem(menu, miViewFarther);
			} else {
				EnableItem(menu, miViewFarther);
			}
			if (frontmap->power == NUMPOWERS-1) {
				DisableItem(menu, miViewCloser);
			} else {
				EnableItem(menu, miViewCloser);
			}
			EnableItem(menu, miViewGrid);
			CheckItem(menu, miViewGrid, frontmap->drawgrid);
			EnableItem(menu, miViewTopline);
			CheckItem(menu, miViewTopline, (frontmap->toph > 0));
			EnableItem(menu, miViewNames);
			CheckItem(menu, miViewNames, frontmap->drawnames);
			if (people_sides_defined()) {
				EnableItem(menu, miViewPeople);
				CheckItem(menu, miViewPeople, frontmap->drawpeople);
			} else {
				DisableItem(menu, miViewPeople);
			}
			if (elevations_defined()) {
				EnableItem(menu, miViewElevations);
				CheckItem(menu, miViewElevations, frontmap->drawelevations);
			} else {
				DisableItem(menu, miViewElevations);
			}
			if (1 /* any weather defined */) {
				EnableItem(menu, miViewWeather);
			} else {
				DisableItem(menu, miViewWeather);
			}
			if (nummtypes > 0) {
				EnableItem(menu, miViewMaterials);
				/* (should do checkmark if anything in submenu is on) */
			} else {
				DisableItem(menu, miViewMaterials);
			}
			if (world.daylength != 1) {
				EnableItem(menu, miViewDaylight);
				CheckItem(menu, miViewDaylight, frontmap->drawlighting);
			} else {
				DisableItem(menu, miViewDaylight);
			}
			if (nummaps > 1) {
				EnableItem(menu, miViewOtherMaps);
				CheckItem(menu, miViewOtherMaps, frontmap->drawothermaps);
			} else {
				DisableItem(menu, miViewOtherMaps);
			}
			if ((menu = GetMHandle(mViewWeather)) != nil) {
				EnableItem(menu, 0);
				if (temperatures_defined()) {
					EnableItem(menu, miWeatherTemperature);
					CheckItem(menu, miWeatherTemperature, frontmap->drawtemperature);
				} else {
					DisableItem(menu, miWeatherTemperature);
				}
				if (winds_defined()) {
					EnableItem(menu, miWeatherWinds);
					CheckItem(menu, miWeatherWinds, frontmap->drawwinds);
				} else {
					DisableItem(menu, miWeatherWinds);
				}
				if (clouds_defined()) {
					EnableItem(menu, miWeatherClouds);
					CheckItem(menu, miWeatherClouds, frontmap->drawclouds);
				} else {
					DisableItem(menu, miWeatherClouds);
				}
				/* should define what this means */
				DisableItem(menu, miWeatherStorms);
/*				CheckItem(menu, miWeatherStorms, frontmap->drawstorms); */
			}
		}
	} else if (frontlist != NULL) {
		/* Do the list view menu similarly. */
		if ((menu = GetMHandle(mViewMap)) != nil) {
			DeleteMenu(mViewMap);
		}
		/* Make sure the menu is actually installed (is always at the end). */
		InsertMenu(listviewmenu, 0);
		if ((menu = GetMHandle(mViewList)) != nil) {
			EnableItem(menu, 0);
			if (1 /* at least one unit in list */) {
				EnableItem(menu, miViewByType);
				CheckItem(menu, miViewByType, (frontlist->mainsortmi == miViewByType));
				EnableItem(menu, miViewByName);
				CheckItem(menu, miViewByName, (frontlist->mainsortmi == miViewByName));
				EnableItem(menu, miViewBySide);
				CheckItem(menu, miViewBySide, (frontlist->mainsortmi == miViewBySide));
				EnableItem(menu, miViewByActOrder);
				CheckItem(menu, miViewByActOrder, (frontlist->mainsortmi == miViewByActOrder));
				EnableItem(menu, miViewByLocation);
				CheckItem(menu, miViewByLocation, (frontlist->mainsortmi == miViewByLocation));
				/* should mark the secondary sort? */
				EnableItem(menu, miViewWithTransport);
				DisableItem(menu, miViewWithCommander);
				EnableItem(menu, miViewIconSize);
				CheckItem(menu, miViewIconSize, frontlist->largeicons);
			} else {
				DisableItem(menu, miViewByType);
				DisableItem(menu, miViewByName);
				DisableItem(menu, miViewBySide);
				DisableItem(menu, miViewByActOrder);
				DisableItem(menu, miViewByLocation);
				DisableItem(menu, miViewWithTransport);
				DisableItem(menu, miViewWithCommander);
				DisableItem(menu, miViewIconSize);
			}
		}
	} else {
		/* For any other window, disable any or all of the view menus, as needed. */
		if ((menu = GetMHandle(mViewList)) != nil) {
			DisableItem(menu, 0);
		}
		if ((menu = GetMHandle(mViewMap)) != nil) {
			DisableItem(menu, 0);
		}
	}
	if ((menu = GetMHandle(mMagnifications)) != nil) {
		/* Always on. */
		EnableItem(menu, 0);
		if (frontmap != NULL) {
			for (i = 0; i < NUMPOWERS; ++i) {
				CheckItem(menu, i + 1, (i == frontmap->power));
			}
		}
	}
	if ((menu = GetMHandle(mMaterialTypes)) != nil) {
		/* Always on, if any material types defined. */
		if (nummtypes > 0) {
			EnableItem(menu, 0);
		} else {
			DisableItem(menu, 0);
		}
		if (frontmap != NULL) {
			for_all_material_types(m) {
				if (any_cell_materials_defined() && cell_material_defined(m)) {
					EnableItem(menu, m + 1);
					CheckItem(menu, m + 1, frontmap->drawmaterials[m]);
				} else {
					DisableItem(menu, m + 1);
				}
			}
		}
	}
	if ((menu = GetMHandle(mAITypes)) != nil) {
		/* Always on. */
		EnableItem(menu, 0);
		CheckItem(menu, 1, (!side_has_ai(dside)));
		CheckItem(menu, 2, (side_has_ai(dside)));
	}
	/* Everything has been tweaked, redraw the menu bar. */
	DrawMenuBar();
}

/* Tweak the command menu according to what the unit can do. */
/* (some of these should have the menu text tweaked also, terms weird sometimes) */

enable_commands_for_unit(menu, unit)
MenuHandle menu;
Unit *unit;
{
	int u = unit->type, u2;

	/* If we can select the unit at all, we can always get a closeup of it. */
	EnableItem(menu, miPlayCloseup);
	/* (how to do checkmarks if multiple units selected? use '-' as per HIG) */
	CheckItem(menu, miPlayCloseup, (find_unit_closeup(unit) != NULL));
	/* If we don't actually have any control over the unit, nothing more to do. */
	if (!side_controls_unit(dside, unit)) return;
	if (can_move_at_all(unit)) {
		EnableItem(menu, miPlayMove);
		EnableItem(menu, miPlayPatrol);
		/* also check for places to return to? */
		EnableItem(menu, miPlayReturn);
	}
	if (unit->plan) {
		EnableItem(menu, miPlayWake);
		CheckItem(menu, miPlayWake, !unit->plan->asleep);
		EnableItem(menu, miPlaySleep);
		CheckItem(menu, miPlaySleep, unit->plan->asleep);
		EnableItem(menu, miPlayDoneMoving);
		CheckItem(menu, miPlayDoneMoving, unit->plan->reserve);
	}
	if (can_create(unit) || can_complete(unit) /* or research? */) {
		EnableItem(menu, miPlayBuild);
	}
	if (can_repair(unit)) {
		EnableItem(menu, miPlayRepair);
	}
	if (can_attack(unit)) {
		EnableItem(menu, miPlayAttack);
	}
	if (can_attack(unit)) {
		EnableItem(menu, miPlayOverrun);
	}
	if (can_attack(unit)) {
		EnableItem(menu, miPlayFire);
	}
	if (can_detonate(unit)) {
		EnableItem(menu, miPlayDetonate);
	}
	if (1 /*dside->designer*/) {
		EnableItem(menu, miPlayGive);
	}
	if (can_disband_at_all(dside, unit)) {
		EnableItem(menu, miPlayDisband);
	}
	if (1 /* !unitnameslocked */) {
		EnableItem(menu, miPlayRename);
	}
}

/* Keyboard command handling. */

#ifdef PROFILING
extern int _trace;
#endif

typedef struct cmdtab {
    char fchar;                 /* character to match against */
    char ochar;                 /* 5.x character to match against */
    char *name;                 /* Full name of command */
    char *argtypes;
    int (*fn)();                /* pointer to command's function */
    char *help;                 /* short documentation string */
} CmdTab;

#define C(c) ((c)-0x40)

char tmpkey;

/* Start of alphabetized commands. */

int
do_add_player()
{
	SysBeep(20);
}

int
do_ai_side()
{
	SysBeep(20);
}

int
do_attack()
{
	do_attack_command();
}

int
do_auto()
{
	SysBeep(20);
}

int
do_build()
{
	do_construction_mi();
}

int
do_detonate()
{
	do_detonate_command();
}

char dirchars[] = "ulnbhy";

int tmpdir;

do_one_dir_move(unit)
Unit *unit;
{
	order_movedir(unit, tmpdir, 1);
}

int
do_dir()
{
	char *rawdir;
	Map *map;

	if ((map = map_from_window(FrontWindow())) != NULL
		&& map->moveonclick) {
		rawdir = strchr(dirchars, tmpkey);
		if (!rawdir) {
			SysBeep(20);
			return;
		}
		tmpdir = rawdir - dirchars;
		apply_to_all_selected(do_one_dir_move);
	}
}

int
do_dir_multiple()
{
}

int
do_disband()
{
	SysBeep(20);
}

int
do_distance()
{
	SysBeep(20);
}

int
do_end_turn()
{
	/* Special-case this, depending on which window was in front. */
	if (FrontWindow() == constructionwin) {
		Point pt;
		extern ControlHandle constructbutton;
		
		pt.h = (*constructbutton)->contrlRect.left + 8;
		pt.v = (*constructbutton)->contrlRect.top + 8;
		do_mouse_down_construction(pt, 0);
	} else {
		finish_turn(dside);
	}
}

int
do_fire()
{
	do_fire_command();
}

int
do_give()
{
	SysBeep(20);
}

int
do_give_unit()
{
	SysBeep(20);
}

int doingother = FALSE;

int
do_help()
{
	help_dialog();
}

int
do_message()
{
	SysBeep(20);
}

int
do_move_to()
{
	SysBeep(20);
}

int
do_name()
{
	SysBeep(20);
}

int
do_other()
{
	/* Don't allow recursion with this command. */
	if (!doingother) {
		doingother = TRUE;
		command_dialog();
		doingother = FALSE;
	} else {
		SysBeep(20);
	}
}

command_dialog()
{
	short done = FALSE, ditem;
	char *cmd = NULL;
	Str255 tmpstr;
	DialogPtr win;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	win = GetNewDialog(dCommand, NULL, (DialogPtr) -1L);
	ShowWindow(win);
	while (!done) {
		draw_default_button(win, diCommandOK);
		ModalDialog(NULL, &ditem);
		switch (ditem) {
			case diCommandOK:
				GetDItem(win, diCommandText, &itemtype, &itemhandle, &itemrect);
				cmd = get_string_from_item(itemhandle);
				/* Fall into next case. */
			case diCommandCancel:
				done = TRUE;
				break;
		}
	}
	DisposDialog(win);
	if (cmd != NULL) {
		execute_named_command(cmd);
	}
}

int
do_produce()
{
	SysBeep(20);
}

int
do_quit()
{
	quit_the_game();
}

int
do_recenter()
{
	SysBeep(20);
}

int
do_refresh()
{
	Map *map;
	List *list;

	reset_coverage();
	reset_all_views();
	/* (should just iterate through raw windows) */
	for_all_maps(map) {
		force_update(map->window);
	}
	for_all_lists(list) {
		force_update(list->window);
	}
	if (gamewin != nil) {
		force_update(gamewin);
	}
	if (historywin != nil) {
		force_update(historywin);
	}
	if (constructionwin != nil) {
		force_update(constructionwin);
	}
	if (helpwin != nil) {
		force_update(helpwin);
	}
}

int
do_reserve()
{
	do_reserve_command(TRUE);
}

int
do_return()
{
	do_return_mi();
}

int
do_sleep()
{
	SysBeep(20);
}

int 
do_survey()
{
	Map *map;

	if ((map = map_from_window(FrontWindow())) != NULL) {
		toggle_survey(map);
	}
}

int
do_take()
{
	SysBeep(20);
}

int
do_take_unit()
{
	SysBeep(20);
}

int
do_version()
{
	do_about_box();
}

int
do_wake()
{
	do_sleep_command(FALSE, FALSE);
	do_reserve_command(FALSE, FALSE);
}

int
do_wake_all()
{
	do_sleep_command(FALSE, TRUE);
	do_reserve_command(FALSE, TRUE);
}

#ifdef DESIGNERS

int
do_design()
{
	SysBeep(20);
}

#endif

#ifdef DEBUGGING

int
do_debug()
{
	SysBeep(20);
}

int
do_debugg()
{
	SysBeep(20);
}

int
do_debugm()
{
	SysBeep(20);
}


#endif

/* End of alphabetized commands. */

CmdTab commands[] = {

#undef DEF_CMD
#define DEF_CMD(LETTER,OLDLETTER,NAME,ARGS,FN,HELP) { LETTER, OLDLETTER, NAME, ARGS, FN, HELP},

#include "cmd.def"

  { 0, 0, NULL, NULL, NULL, NULL }
};

do_keyboard_command(key)
char key;
{
	CmdTab *cmd;
    int (*fn)();

	DGprintf("Typed '%c' (0x%x)\n", key, key);
	switch (key) {
		case 'd':  do_find_next_mi();  break;
		case 's':  do_sleep_command(TRUE, FALSE);  break;
		case 'c':  do_ai_control_command();  break;
		case 'C':  do_clear_plan_command();  break;
		case '*':  force_global_replan(dside);  break;
		case '\033': modaltool = 0;  break;
#ifdef DEBUGGING
		case 'D':  toggle_debugging(&Debug);  break;
		case 'M':  toggle_debugging(&DebugM); break;
		case 'G':  toggle_debugging(&DebugG); break;
/*		case 'P':  toggle_debugging(&Profile); break; */
		case 'T':
			toggle_debugging(&Profile);
#ifdef PROFILING
			_trace = 1;
#endif
			break;
#endif /* DEBUGGING */
		default:
			/* Look through the generic command table. */
		    for (cmd = commands; cmd->name != NULL; ++cmd) {
				if (key == cmd->fchar) {
			    	if ((fn = cmd->fn) == NULL) {
						run_warning("no command function for %s (0x%x)?", cmd->name, key);
						return;
			    	}
			    	tmpkey = key;
					(*fn)();
					return;
				}
			}
			/* Silently ignore unrecognized chars? */
			break;
	}
}

execute_named_command(cmdstr)
char *cmdstr;
{
	CmdTab *cmd;
    int (*fn)();

    for (cmd = commands; cmd->name != NULL; ++cmd) {
		if (strcmp(cmdstr, cmd->name) == 0) {
	    	if ((fn = cmd->fn) == NULL) {
				run_warning("no command function for %s?", cmd->name);
				return;
	    	}
	    	tmpkey = '\0';
			(*fn)();
			return;
		}
	}
	SysBeep(20);
}

#ifdef DEBUGGING

/* Junk associated with debug output. */

/* This is all fairly elaborate because we need to be able to collect
   detailed logs of AI activity over different periods of time, and just
   dumping to stdout doesn't work in a window system. */

#ifdef USE_CONSOLE
#ifdef THINK_C
#include <console.h>
#endif
#endif

extern FILE *pfp;

FILE *ffp = NULL;

int firstdebug = TRUE;

update_debugging()
{
	if (Debug || DebugG || DebugM || Profile) {
		/* Always close the file if open, forces to desirable state. */
		if (ffp != NULL) {
			fclose(ffp);
			ffp = NULL;
		}
		/* Reopen the file. */
		if (ffp == NULL) {
			ffp = fopen("Xconq.DebugOut", "a");
		}
		if (ffp != NULL) {
			if (Debug) dfp = ffp;
			if (DebugG) dgfp = ffp;
			if (DebugM) dmfp = ffp;
			if (Profile) pfp = ffp;
		}
	}
}

/* Debug output goes to a file. */

toggle_debugging(flagp)
int *flagp;
{
#ifdef PROFILING
	extern int _profile;
#endif

	/* Always close the file if open, forces to desirable state. */
	if (ffp != NULL) {
		fclose(ffp);
		ffp = NULL;
	}
	/* Flip the state of the debugging flag, if supplied. */ 
	if (flagp != NULL) {
		*flagp = ! *flagp;
	}
	/* (Re-)open the debugging transcript file. */
	if (ffp == NULL) {
		ffp = fopen("Xconq.DebugOut", (firstdebug ? "w" : "a"));
		firstdebug = FALSE;
	}
	if (flagp != NULL) {
		/* Indicate which flags are now on. */
		fprintf(ffp, "\n\n*********** %s %s %s %s **********\n\n",
				(Debug ? "Debug" : ""), (DebugM ? "DebugM" : ""),
				(DebugG ? "DebugG" : ""), (Profile ? "Profile" : ""));
		/* Indicate this in a window also. */
		draw_game();
	}
	/* Set specific debug file pointers to be the same as the
	   pointer to the file. */
	if (ffp != NULL) {
		if (Debug) dfp = ffp;
		if (DebugG) dgfp = ffp;
		if (DebugM) dmfp = ffp;
		if (Profile) pfp = ffp;
	}
#ifdef PROFILING
#ifdef THINK_C
	if (Profile && !_profile) {
		InitProfile(1000, 100);
	}
	if (!Profile && _profile) {
		EndProfile();
	}
#endif
#endif
	/* If all debugging flags have been turned off, close the file too. */
	if (!Debug && !DebugG && !DebugM && !Profile) {
		if (ffp != NULL) {
			fclose(ffp);
			ffp = NULL;
		}
	}
}

#endif /* DEBUGGING */
