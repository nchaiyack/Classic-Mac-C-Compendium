/* Copyright (c) 1992, 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Initialization of the Mac interface. */

#include "conq.h"
#include "mac.h"

#define MAXCHECKBOXES 10
#define MAXSLIDERS 3

#define toggle_checkbox(ih) \
  SetCtlValue((ControlHandle) (ih), !GetCtlValue((ControlHandle) (ih)))

/* (not quite right - come up with a better test) */

#define game_already_loaded() (numutypes > 0)

#define t_color_defined(t) (hasColorQD && tcolors != NULL && tcolors[t] != NULL)

#undef p2c
#define p2c(pstr,buf)  \
  strncpy(buf, pstr+1, pstr[0]);  \
  /* Make sure this name is terminated properly. */  \
  buf[pstr[0]] = '\0';

extern Assign *assignments;
extern int foundimagesfile;
extern int allbedesigners;

char *copy_string();

BitMap *bordbitmaps;
BitMap *connbitmaps;

char *firstgamename = "intro"; /* into a special str resource?? */

int varyseen;
int varyallseen;
int varyworld;
int anyvariants;

/* This is the dialog used to select a game to play. */

WindowPtr newgamewin = nil;

ListHandle newgamelist = nil;

TEHandle newgametext = nil;

PicHandle newgamepicture = nil;

Module **possiblegames = NULL;

int numgames = 0;

Module *selectedgame = NULL;

/* This is the dialog used to set the size and shape of the world. */

DialogPtr worldshapewin = nil;

/* These are the values that will be used to set world geometry. */

long newcircumference;
long newwidth;
long newheight;
long newlatitude;
long newlongitude;
long newxwrap;

int newseen;
int newseeall;

/* This is the variant-setting dialog. */

DialogPtr variantswin = nil;

int numcheckboxes;
int numsliders;

Obj *checkboxes[MAXCHECKBOXES];
char *checkboxnames[MAXCHECKBOXES];
Obj *sliders[MAXSLIDERS];
char *slidernames[MAXSLIDERS];

Obj *variants;

/* This is the player setup dialog. */

WindowPtr playersetupwin = nil;

PicHandle updownpicture = nil;
PicHandle updownpictureup = nil;
PicHandle updownpicturedown = nil;

int playerh = 22;
int playerbaseline = 14;

int selectedplayer = -1;

#define selected_a_player() (between(0, selectedplayer, numsides - 1))

#define selected_player()  \
  (selected_a_player() ? assignments[selectedplayer].player : NULL)

#define selected_side() (selected_a_player() ? assignments[selectedplayer].side : NULL)

ImageFamily *uimages = NULL;
ImageFamily *timages = NULL;
ImageFamily *eimages = NULL;

ImageColor **tcolors = NULL;

CursHandle readprogressors[NUMcParens];
CursHandle progressors[NUMcSynth];
CursHandle movecursors[NUMcMoves];
CursHandle nomovecursor;
CursHandle allmovecursor;
CursHandle grayarrowcursor;
CursHandle opencrosscursor;
CursHandle firecursor;
CursHandle watchcursor;

/* This value is the current progress cursor. */

int curcurs = 0;

/* The initialization progress dialog. */

DialogPtr progresswin = nil;

/* The current value of the percent progress. */

int progress;

/* The previous value of the percent progress. */

int lastprogress;

enum grays gridgray = whitegray;
enum grays unseengray = whitegray;
enum grays bggray = mediumgray;

RGBColor gridcolor;
RGBColor unseencolor;
RGBColor bgcolor;
RGBColor blackcolor;

int gridmatchesunseen = FALSE;

PicHandle dotdotdotpicture = nil;

/* Set up the various cursors we'll be using. */

init_cursors()
{
	int i;

	/* Get all the cursors used to indicate reader progress. */
	for (i = 0; i < NUMcParens; ++i) {
		readprogressors[i] = GetCursor(cParens1 + i);
	}
	/* Get all the cursors used to indicate synthesis progress. */
	for (i = 0; i < NUMcSynth; ++i) {
		progressors[i] = GetCursor(cSynth1 + i);
	}
	/* Get all the cursors indicating the move direction. */
	for (i = 0; i < NUMcMoves; ++i) {
		movecursors[i] = GetCursor(cMove1 + i);
	}
	/* Miscellaneous cursors. */
	nomovecursor = GetCursor(cNoMove);
	allmovecursor = GetCursor(cAllMove);
	grayarrowcursor = GetCursor(cGrayArrow);
	opencrosscursor = GetCursor(cOpenCross);
	firecursor = GetCursor(138);
	watchcursor = GetCursor(watchCursor);
	/* Designer-related cursors will be done later, if needed at all. */

	dotdotdotpicture = (PicHandle) GetResource('PICT', 134);

}

pascal Boolean
filter_splash(dialog, evt, itemhit)
DialogPtr dialog;
EventRecord *evt;
short *itemhit;
{
	GrafPtr oldport;
	Point pt;
	short ditem;
	char ch;

	/* Look for the right kind of event. */
	switch (evt->what) {
		case keyDown:
			ch = evt->message & charCodeMask;
			if (ch == 3 || ch == 13) {
				*itemhit = diSplashNew;
				return TRUE;
			}
#ifdef DEBUGGING
			/* A secret way to get debugging at startup. */
			/* (should have some sort of feedback?) */
			if (ch == 'D') {
				toggle_debugging(&Debug);
			}
			if (ch == 'M') {
				toggle_debugging(&DebugM);
			}
			if (ch == 'G') {
				toggle_debugging(&DebugG);
			}
#endif
			break;
	}
	return FALSE;
}

/* Display the initial splash screen, and let the player choose New/Open/Connect/Quit. */

do_splash_box()
{
	short ditem;
	Str255 tmpstr;
	WindowPtr win;
	PicHandle pic;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	win = GetNewDialog(dSplash, NULL, (DialogPtr) -1L);
	/* Fill in the kernel's version and copyright. */
	GetDItem(win, diSplashVersion, &itemtype, &itemhandle, &itemrect);
	c2p(version_string(), tmpstr);
	SetIText(itemhandle, tmpstr);
	GetDItem(win, diSplashCopyright, &itemtype, &itemhandle, &itemrect);
	c2p(copyright_string(), tmpstr);
	SetIText(itemhandle, tmpstr);
	/* Substitute a color picture if possible. */
	if (hasColorQD) {
		GetDItem(win, diSplashPicture, &itemtype, &itemhandle, &itemrect);
		if ((pic = (PicHandle) GetResource('PICT', pSplashColor)) != nil) {
			SetDItem(win, diSplashPicture, itemtype, (Handle) pic, &itemrect);
		}
	}
	/* (should enable/disable connect button depending on networking ability) */
	if (can_connect()) {
	} else {
	}
	ShowWindow(win);
	SelectWindow(win);
	ModalDialog((ModalFilterProcPtr) filter_splash, &ditem);
	/* We don't loop around here, just return the ditem and let caller decide what to do. */
	DisposDialog(win);
	return ditem;
}

/* Dialog app-defined item callback that displays the list of possible games. */

pascal void
draw_game_list(win, ditem)
WindowPtr win;
short ditem;
{
	short itemtype;  Handle itemhandle;  Rect itemrect;

	GetDItem(newgamewin, diNewGameList, &itemtype, &itemhandle, &itemrect);
	/* Draw the list of available games. */
	LUpdate(newgamewin->visRgn, newgamelist);
	/* Frame it nicely. */
	--itemrect.left;
	FrameRect(&itemrect);
}

/* Dialog app-defined item callback to display the game description. */

pascal void
draw_game_blurb(win, ditem)
WindowPtr win;
short ditem;
{
	GrafPtr oldport;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	GetDItem(newgamewin, diNewGameBlurb, &itemtype, &itemhandle, &itemrect);
	/* Make sure the text is up-to-date. */
	GetPort(&oldport);
	SetPort(newgamewin);
	TEUpdate(&itemrect, newgametext);	
	SetPort(oldport);
	/* Frame it. */
	InsetRect(&itemrect, -1, -1);
	FrameRect(&itemrect);
}

/* Dialog app-defined item callback to display the game picture, if available. */

pascal void
draw_game_picture(win, ditem)
WindowPtr win;
short ditem;
{
	RgnHandle tmprgn;
	Rect cliprect;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	GetDItem(newgamewin, diNewGamePicture, &itemtype, &itemhandle, &itemrect);
	EraseRect(&itemrect);
	if (newgamepicture != nil) {
		tmprgn = NewRgn();
		GetClip(tmprgn);
		cliprect = itemrect;
		ClipRect(&cliprect);
		center_pict(newgamepicture, &itemrect);
		DrawPicture(newgamepicture, &itemrect);
		SetClip(tmprgn);
		DisposeRgn(tmprgn);
	}
#ifdef DESIGNERS
	/* A low-impact feedback that this game will launch directly into designing. */
	if (allbedesigners) {
		PenSize(3, 3);
		itemrect.right -= 3;  itemrect.bottom -= 3;
		FrameRect(&itemrect);
		PenNormal();
	}
#endif /* DESIGNERS */
}

/* Utility that figures out how to center a picture in a rectangle. */

center_pict(picture, rect)
PicHandle picture;
Rect *rect;
{
	int picth, pictv;
	Rect initrect, pictrect;

	initrect = *rect;
	pictrect = (*picture)->picFrame;
	picth = pictrect.right - pictrect.left;  pictv = pictrect.bottom - pictrect.top;
	rect->left += (initrect.right - initrect.left) / 2 - picth / 2;
	rect->top += (initrect.bottom - initrect.top) / 2 - pictv / 2;
	rect->right = rect->left + picth;
	rect->bottom = rect->top + pictv;
}

Module *
module_from_cell(cell)
Cell cell;
{
	return (cell.v < numgames ? possiblegames[cell.v] : NULL);
}

/* This filter for the new game dialog just handles the game list, updating the
   other items to reflect the currently selected game. */

int lastnewgameclick;
Point lastnewgamemouse;

pascal Boolean
filter_new_game(dialog, evt, itemhit)
DialogPtr dialog;
EventRecord *evt;
short *itemhit;
{
	GrafPtr oldport;
	Point pt, origpt;
	short ditem;
	char ch;

	/* Look for the right kind of event. */
	switch (evt->what) {
		case mouseDown:
			GetPort(&oldport);
			SetPort(newgamewin);
			pt = origpt = evt->where;
			GlobalToLocal(&pt);
			ditem = FindDItem(newgamewin, pt) + 1;
			if (ditem == diNewGameList) {
				/* Loop around in list handling. */
				LClick(pt, evt->modifiers, newgamelist);
				/* We're finished clicking, show the results. */
				select_game();
				display_selected_game();
				SetPort(oldport);
				if (TickCount() - lastnewgameclick < GetDblTime()
					&& between(-3, origpt.h - lastnewgamemouse.h, 3)
					&& between(-3, origpt.v - lastnewgamemouse.v, 3)
					) {
					*itemhit = diNewGameOK;
				} else {
					lastnewgameclick = TickCount();
					lastnewgamemouse = evt->where;
					*itemhit = diNewGameList;
				}
				return TRUE;
			} else {
				SetPort(oldport);
				return FALSE;
			}
			break;
		case keyDown:
			ch = evt->message & charCodeMask;
			if (ch == 3 || ch == 13) {
				if (selectedgame) {
					*itemhit = diNewGameOK;
					return TRUE;
				}
			}
#ifdef DESIGNERS
			/* A secret way to get into designing at startup. */
			if (ch == 'd') {
				allbedesigners = !allbedesigners;
				DrawDialog(newgamewin);
			}
#endif
#ifdef DEBUGGING
			/* A secret way to get debugging at startup. */
			if (ch == 'D') {
				toggle_debugging(&Debug);
				DrawDialog(newgamewin);
			}
			if (ch == 'M') {
				toggle_debugging(&DebugM);
				DrawDialog(newgamewin);
			}
			if (ch == 'G') {
				toggle_debugging(&DebugG);
				DrawDialog(newgamewin);
			}
#endif
			break;
	}
	return FALSE;
}

/* Given a selected game, display assorted info about it - basically a preview so that
   prospective players can see what they're getting into.  If no game has been selected,
   then this routine clears the displays. */

display_selected_game()
{
	short itemtype;  Handle itemhandle;  Rect itemrect;
	char *desc, buf[BUFSIZE];
	Str255 tmpstr;
	PicHandle oldnewgamepicture;

	DGprintf("display selected game %s\n", module_desig(selectedgame));
	/* Set the blurb for the selected game. */
	GetDItem(newgamewin, diNewGameBlurb, &itemtype, &itemhandle, &itemrect);
	TESetSelect(0, 100000, newgametext);
	TECut(newgametext);
	if (selectedgame /* && selectedgame->complete */) {
		if (selectedgame->blurb) {
			desc = selectedgame->blurb;
		} else {
			desc = "??? experimental ???";
		}
	} else {
		desc = "";
	}
	TESetText(desc, strlen(desc), newgametext);
	TEUpdate(&itemrect, newgametext);
	/* Load a picture if one can be found. */
	/* (should look for picture explicitly named in a game-module slot first) */
	oldnewgamepicture = newgamepicture;
	newgamepicture = nil;
	if (selectedgame) {
		sprintf(spbuf, "%s game", selectedgame->name);
		c2p(spbuf, tmpstr);
		newgamepicture = (PicHandle) GetNamedResource('PICT', tmpstr);
		if (newgamepicture == nil) {
			sprintf(spbuf, "%s", selectedgame->name);
			c2p(spbuf, tmpstr);
			newgamepicture = (PicHandle) GetNamedResource('PICT', tmpstr);
		}
	} else {
		/* Nothing to display - how boring! */
	}
	/* Gray out the OK button if no game selected. */
	GetDItem(newgamewin, diNewGameOK, &itemtype, &itemhandle, &itemrect);
	HiliteControl((ControlHandle) itemhandle, (selectedgame ? 0 : 255));
	/* We have to force redraw to get the picture item updated. */
	if (oldnewgamepicture != newgamepicture) DrawDialog(newgamewin);
}

/* This is a modal dialog from which the user selects a game and sets options. */

new_game_dialog()
{
	int done = FALSE;
	short ditem, i;
	Point pnt, cellsize;
	Cell tmpcell;
	Rect listrect, destrect, viewrect;
	Module *module;
	char *gamename;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	collect_possible_games();
	if (newgamewin == nil) {
		newgamewin = GetNewDialog(dNewGame, NULL, (DialogPtr) -1);
		SetPort(newgamewin);
		TextFont(newYork);
/*		TextSize(10); */
		/* Set up the app-defined item that lists games. */
		GetDItem(newgamewin, diNewGameList, &itemtype, &itemhandle, &itemrect);
		SetDItem(newgamewin, diNewGameList, itemtype, (Handle) draw_game_list, &itemrect);
		SetPt(&cellsize, 0, 0);
		listrect.top = 0;  listrect.left = 0;
		listrect.bottom = 0;  listrect.right = 1;
		itemrect.top += 1;
		itemrect.bottom -= 1;  itemrect.right -= sbarwid + 1;
		/* Create the list of games itself and fill it in. */
		newgamelist = LNew(&itemrect, &listrect, cellsize, 0, newgamewin, 0,0,0,TRUE);
		SetPt(&tmpcell, 0, 0);
		for (i = 0; i < numgames; ++i) {
			module = possiblegames[i];
			gamename = module->title;
			if (gamename == NULL) gamename = module->name;
			if (gamename == NULL) gamename = "???";
			LAddRow(1, tmpcell.v, newgamelist);
			LSetCell(gamename, strlen(gamename), tmpcell, newgamelist);
			++tmpcell.v;
		}
		GetDItem(newgamewin, diNewGameBlurb, &itemtype, &itemhandle, &itemrect);
		SetDItem(newgamewin, diNewGameBlurb, itemtype, (Handle) draw_game_blurb, &itemrect);
		destrect = itemrect;
		viewrect = itemrect;
/*		TextSize(10);  */
		newgametext = TENew(&destrect, &viewrect);
		GetDItem(newgamewin, diNewGamePicture, &itemtype, &itemhandle, &itemrect);
		SetDItem(newgamewin, diNewGamePicture, itemtype, (Handle) draw_game_picture, &itemrect);
		newgamepicture = nil;
	}
	update_new_game_list();
	SetPt(&tmpcell, 0, 0);
	LSetSelect(TRUE, tmpcell, newgamelist);  /* (do this only for new/changed list?) */
	select_game();
	display_selected_game();
	SetCursor(&QD(arrow));
	ShowWindow(newgamewin);
	LDoDraw(1, newgamelist);
	/* Loop around here until the player picks a game to play, or cancels. */
	while (!done) {
		draw_default_button(newgamewin, diNewGameOK);
		ModalDialog((ModalFilterProcPtr) filter_new_game, &ditem);
		switch (ditem) {
			case diNewGameOK:
				/* Make this dialog disappear now, startup may take a long time. */
				HideWindow(newgamewin);
				if (start_new_game()) {
					done = TRUE;
				} else {
					/* If we failed to start the new game, just go around again. */
					ShowWindow(newgamewin);
					SelectWindow(newgamewin);
				}
				break;
			case diNewGameCancel:
				done = TRUE;
				break;
			default:
				break;
		}
	}
	/* We're done, OK to throw away the dialog. */
	DisposDialog(newgamewin);
	newgamewin = nil;
	/* (should release TEs and lists also) */
}

/* The comparison function for the game list puts un-formally-named modules at the end,
   plus the default sorting puts initial-lowercased names after uppercased ones. */

module_name_compare(mp1, mp2)
Module **mp1, **mp2;
{
	if ((*mp1)->title == NULL) {
		if ((*mp2)->title == NULL) {
			/* (this assumes we have no entirely unnamed modules) */
			return strcmp((*mp1)->name, (*mp2)->name);
		} else {
			return 1;
		}
	} else {
		if ((*mp2)->title == NULL) {
			return (-1);
		} else {
			return strcmp((*mp1)->title, (*mp2)->title);
		}
	}
}

/* This routine finds and lists all the games that should be listed as choices for
   the user. */

collect_possible_games()
{
	int numresources, i;
	Handle modulehandle;
	short moduleid;
	char *modulename = NULL, *modulecontents = NULL;
	ResType restype;
	Str255 resname;
	Module *module;

	if (numgames == 0 && !game_already_loaded()) {
		numresources = CountResources('XCgm');
		/* Make enough room to record all the possible games. */
		possiblegames = (Module **) xmalloc((numresources + 1) * sizeof(Module *));
		/* Collect the intro game module and put at head of list. */
		module = get_game_module(firstgamename);
		add_to_possible_games(module);
		/* Pick up game modules that are wired in as resources. */
		for (i = 0; i < numresources; ++i) {
			modulehandle = GetIndResource('XCgm', i + 1);
			/* (should test for resource validity?) */
			if (0 /* size > 0 */) {
				/* set modulecontents from resource */
				modulecontents = NULL;
			}
			/* Try to pick up module name from its resource name, otherwise
			   assume its name in its content. */
			GetResInfo(modulehandle, &moduleid, &restype, &resname);
			if (resname[0] > 0) {
				resname[resname[0]+1] = '\0';
				modulename = copy_string(resname+1);
			} else {
				modulename = NULL;
			}
			module = get_game_module(modulename);
			module->contents = modulecontents;
			add_to_possible_games(module);
		}
#if 0
		/* (eventually add random library files) */
		/* (will have to count them first) */
		printf("Lib is %s\n", xconqlib);
		sprintf(spbuf, "%s:", xconqlib);
		pascalify(spbuf);
		for (i = 0; i < 10; ++i) {
			FInfo info;
			OSErr rslt = GetFInfo(tmpbuf, i, &info);
			
			printf("%d: rslt is %d\n", i, rslt);
			if (rslt == noErr) {
				printf("  %c %c %d %d %d\n",
						info.fdType, info.fdCreator, info.fdFlags,
						info.fdLocation, info.fdFldr);
			}
		}
#endif
		if (numgames > 2) {
			/* Sort all but the first game into alphabetical order by displayed name. */
			qsort(&(possiblegames[1]), numgames - 1, sizeof(Module *), module_name_compare);
		}
	}
}

/* Load a game's description and add it to the list of games. */

add_to_possible_games(module)
Module *module;
{
	int i;

	if (module != NULL) {
		if (load_game_description(module)) {
			/* It might be that the module description supplies the real name,
		  	 and that the module already exists. (work on this) */
		  	/* Don't add duplicate modules. */
			for (i = 0; i < numgames; ++i) {
				if (possiblegames[i] == module) return;
			}
		  	if (numgames < 100) {
				possiblegames[numgames++] = module;
			}
		}
	}
}

/* Select a game from the list of possible games. */

select_game()
{
	Cell tmpcell;

	SetPt(&tmpcell, 0, 0);
	if (game_already_loaded()) {
		/* selectedgame is already set correctly. */
	} else if (LGetSelect(TRUE, &tmpcell, newgamelist)) {
		selectedgame = module_from_cell(tmpcell);
	} else {
		selectedgame = NULL;
	}
}

/* When the new game OK button is hit, this will do the work of getting things started,
   possibly pausing to ask about player/side setup. */

start_new_game()
{
	if (selectedgame == NULL) { /* Just in case... */
		SysBeep(20);
		return FALSE;
	}
	if (!game_already_loaded()) {
		/* Suck in the selected module.  This cannot be undone (yet). */
		mainmodule = selectedgame;
		load_game_module(selectedgame);
		/* Change cursor back, in case it was different during loading. */
		SetCursor(&QD(arrow));
		/* If the loaded game is not valid, we will get an alert somewhere in here,
		   and possibly bomb out if the player chooses not to continue. */
		check_game_validity();
	}
	return launch_game();
}

/* This routine is for when we end up back in the new game dialog,
   after something has already been loaded. */

/* (could also include any game that has this one as a base module) */

update_new_game_list()
{
	char *gamename;
	Point tmpcell;
	
	if (newgamelist != nil && game_already_loaded()) {
		SetPt(&tmpcell, 0, 0);
		/* Clear out the entire list. */
		LDelRow(0, 0, newgamelist);
		/* Add the loaded game back. */ 
		LAddRow(1, tmpcell.v, newgamelist);
		/* Find a name to paste into the list. */
		gamename = selectedgame->title;
		if (gamename == NULL) gamename = selectedgame->name;
		if (gamename == NULL) gamename = "???";
		/* Set and select the one cell. */
		LSetCell(gamename, strlen(gamename), tmpcell, newgamelist);
		LSetSelect(TRUE, tmpcell, newgamelist);
	}
}

pascal void
draw_variant_slider(win, ditem)
WindowPtr win;
short ditem;
{
	Str255 tmpstr;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	GetDItem(win, ditem, &itemtype, &itemhandle, &itemrect);
	/* Always frame. */
	FrameRect(&itemrect);
	if (between(diVariantsFirstSlider, ditem, diVariantsFirstSlider + numsliders - 1)) {
		MoveTo(itemrect.left + 2, itemrect.top + 10);
		c2p(slidernames[ditem - diVariantsFirstSlider], tmpstr);
		DrawString(tmpstr);
	} else {
		gray_out_rect(&itemrect);
	}
}

pascal void
draw_variant_help(win, ditem)
WindowPtr win;
short ditem;
{
	short itemtype;  Handle itemhandle;  Rect itemrect;

	GetDItem(win, ditem, &itemtype, &itemhandle, &itemrect);
	/* Always frame. */
	FrameRect(&itemrect);
}

/* The variants dialog basically handles all the player-settable options defined
   by a game. */

variants_dialog()
{
	int i, done = FALSE, changed = FALSE;
	char *gamename;
	Str255 tmpstr;
	short ditem;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	interpret_variants();
	if (!anyvariants) return TRUE;
	if (variantswin == nil) {
		variantswin = GetNewDialog(dVariants, NULL, (DialogPtr) -1L);
		/* Set the title of this dialog appropriately. */
		if (selectedgame != NULL) {
			gamename = selectedgame->title;
			if (gamename == NULL) gamename = selectedgame->name;
			if (gamename == NULL) gamename = "???";
		} else {
			gamename = "?";
		}
		sprintf(spbuf, "Variants for \"%s\"", gamename);
		c2p(spbuf, tmpstr);
		GetDItem(variantswin, diVariantsText, &itemtype, &itemhandle, &itemrect);
		SetIText(itemhandle, tmpstr);
		/* Give all the slider app items the same proc. */
		for (i = diVariantsFirstSlider; i < (diVariantsFirstSlider + MAXSLIDERS); ++i) {
			GetDItem(variantswin, i, &itemtype, &itemhandle, &itemrect);
			SetDItem(variantswin, i, itemtype, (Handle) draw_variant_slider, &itemrect);
		}
		GetDItem(variantswin, diVariantsHelp, &itemtype, &itemhandle, &itemrect);
		SetDItem(variantswin, diVariantsHelp, itemtype, (Handle) draw_variant_help, &itemrect);
	}
	/* Display/default the standard variants appropriately. */
	set_variant_item(diVariantsWorldSeen, varyseen, TRUE, newseen);
	set_variant_item(diVariantsSeeAll, varyallseen, TRUE, newseeall);
	set_variant_item(diVariantsWorldSize, varyworld, FALSE, 0);
	/* For each random checkbox used, give it a title and make it displayable,
	   otherwise gray it. */
	for (i = 0; i < MAXCHECKBOXES; ++i) {
		GetDItem(variantswin, diVariantsFirstCheckBox + i, &itemtype, &itemhandle, &itemrect);
		if (i < numcheckboxes) {
			c2p(checkboxnames[i], tmpstr);
			SetCTitle((ControlHandle) itemhandle, tmpstr);
			ShowControl((ControlHandle) itemhandle);
			/* should set its initial state based on variant's default */
		} else {
			sprintf(spbuf, "Unused", i);
			c2p(spbuf, tmpstr);
			SetCTitle((ControlHandle) itemhandle, tmpstr);
			ShowControl((ControlHandle) itemhandle);
			HiliteControl((ControlHandle) itemhandle, 255);
		}
	}
	ShowWindow(variantswin);
	while (!done) {
		draw_default_button(variantswin, diVariantsOK);
		ModalDialog(NULL, &ditem);
		switch (ditem) {
			case diVariantsOK:
				implement_variants();
				changed = TRUE;
				/* Fall through to next case. */
			case diVariantsCancel:
				done = TRUE;
				break;
			case diVariantsWorldSeen:
			case diVariantsSeeAll:
				/* Toggle check boxes. */
				GetDItem(variantswin, ditem, &itemtype, &itemhandle, &itemrect);
				toggle_checkbox(itemhandle);
				break;
			case diVariantsWorldSize:
				/* Fire up a separate dialog for world geometry. */
				world_shape_dialog();
				break;
			default:
				/* Handle all the checkboxes similarly. */
				if (between(diVariantsFirstCheckBox, ditem, diVariantsFirstCheckBox+MAXCHECKBOXES-1)) {
					GetDItem(variantswin, ditem, &itemtype, &itemhandle, &itemrect);
					toggle_checkbox(itemhandle);
				}
				break;
		}
	}
	DisposDialog(variantswin);
	variantswin = nil;
	return changed;
}

set_variant_item(di, flag, flag2, val)
int di;
int flag, flag2, val;
{
	short itemtype;  Handle itemhandle;  Rect itemrect;

	GetDItem(variantswin, di, &itemtype, &itemhandle, &itemrect);
	ShowControl((ControlHandle) itemhandle);
	HiliteControl((ControlHandle) itemhandle, (flag ? 0 : 255));
	if (flag && flag2) {
		SetCtlValue((ControlHandle) itemhandle, val);
	}
}

/* Go through all the variants and set up appropriate flags. */

interpret_variants()
{
	Obj *vars, *var, *vartype, *vardata, *vartmp;
	char *varname, *vartypename;

	varyseen = varyallseen = varyworld = anyvariants = FALSE;
	if (!selectedgame) return;
	numcheckboxes = 0;
	numsliders = 0;
	for (vars = selectedgame->variants; vars != lispnil; vars = cdr(vars)) {
		anyvariants = TRUE;
		var = car(vars);
		if (symbolp(var)) {
			var = cons(var, lispnil);
		} else if (!consp(var)) {
			/* already warned about */
			continue;
		}
		varname = c_string(car(var));
		if (stringp(car(var))) {
			var = cdr(var);
		}
		vartype = car(var);
		vardata = cdr(var);
		vartypename = c_string(vartype);
		switch (keyword_code(vartypename)) {
			case K_WORLD_SEEN:
				varyseen = TRUE;
				newseen = FALSE;
				if (vardata != lispnil) {
					vartmp = eval(car(vardata));
					if (numberp(vartmp)) {
						newseen = c_number(vartmp);
					}
				}
				break;
			case K_SEE_ALL:
				varyallseen = TRUE;
				newseeall = FALSE;
				if (vardata != lispnil) {
					vartmp = eval(car(vardata));
					if (numberp(vartmp)) {
						newseeall = c_number(vartmp);
					}
				}
				break;
			case K_WORLD_SIZE:
				varyworld = TRUE;
				/* Start with some defaults. */
				newcircumference = DEFAULTCIRCUMFERENCE;
				newwidth = DEFAULTWIDTH;  newheight = DEFAULTHEIGHT;
				newlatitude = 0;  newlongitude = 0;
				break;
			default:
				if (1 /* boolean variant */) {
					if (numcheckboxes >= MAXCHECKBOXES) {
						init_warning("too many variants, can't set all of them");
						break;
					}
					checkboxnames[numcheckboxes] = varname;
					checkboxes[numcheckboxes] = var;
					++numcheckboxes;
				} else {
					/* (should set up slider) */
				}
				break;
		}
	}
}

/* This is where we actually change the state of the game according to the variants. */

/* (should share more with other interfaces) */

implement_variants()
{
	int i;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	variants = lispnil;
	if (varyworld) {
		/* It is critically important that users not be able to reshape already-alloced
		   areas, but do let them know that their request had to be overridden. */
		if (((area.width > 0 && area.width != newwidth)
			|| (area.height > 0 && area.height != newheight)
			|| (world.circumference > 0 && world.circumference != newcircumference))
			&& (1 /* some layers (probably) allocated already */)) {
			/* (this is misleading, is an "expected" alert) */
			init_warning("Area dimensions must remain %d x %d, %d around world",
						 area.width, area.height, world.circumference);
			newwidth = area.width;  newheight = area.height;
			newcircumference = world.circumference;
		}
		variants = cons(cons(intern_symbol("world-size"),
							 cons(new_number(newwidth),
							 	  cons(new_number(newheight),
									   cons(new_number(newcircumference),
									        cons(new_number(newlongitude),
									             cons(new_number(newlatitude),
								  	        lispnil)))))),
					    variants);
	}
	if (varyseen) {
		GetDItem(variantswin, diVariantsWorldSeen, &itemtype, &itemhandle, &itemrect);
		variants = cons(cons(intern_symbol("world-seen"),
							 cons(intern_symbol(GetCtlValue((ControlHandle) itemhandle) ?
							 				    "true" : "false"),
								  lispnil)),
					    variants);
	}
	if (varyallseen) {
		GetDItem(variantswin, diVariantsSeeAll, &itemtype, &itemhandle, &itemrect);
		variants = cons(cons(intern_symbol("see-all"),
							 cons(intern_symbol(GetCtlValue((ControlHandle) itemhandle) ?
							 				    "true" : "false"),
								  lispnil)),
					    variants);
	}
	/* Implement the random checkbox variants. */
	for (i = 0; i < numcheckboxes; ++i) {
		GetDItem(variantswin, diVariantsFirstCheckBox + i, &itemtype, &itemhandle, &itemrect);
		if (GetCtlValue((ControlHandle) itemhandle)) {
			variants = cons(cons(car(checkboxes[i]),
								 cons(intern_symbol("true"),
								      lispnil)),
						    variants);
		}
	}
	/* (should implement the random slider variants) */
	do_module_variants(selectedgame, variants);
	/* Recheck everything, the variants might have broken something. */
	if (anyvariants) {
		check_game_validity();
	}
}

void
special_xform(x, y, sxp, syp)
int x, y, *sxp, *syp;
{
	int hw = 1, hh = 1, hch = 1, sx = newheight / 4, sy = 0, totsh = newheight;

	*sxp = x * hw + (y * hw) / 2 - sx;
	*syp = totsh - y * hch - sy;
}


/* This is a callback that draws the world and its areas. */

/* (Would be really cool to draw accurate curvature over globe here) */

pascal void
draw_world_picture(win, ditem)
WindowPtr win;
short ditem;
{
	int llx, lly, lrx, lry, rx, ry, urx, ury, ulx, uly, lx, ly;
	Point dims, center;
	Rect worldrect, arearect;
	PolyHandle poly;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	GetDItem(worldshapewin, diWorldShapePicture, &itemtype, &itemhandle, &itemrect);
	/* Make a framed gray background for the world picture. */
	FillRect(&itemrect, QD(gray));
	FrameRect(&itemrect);
	InsetRect(&itemrect, 10, 10);
	dims.h = itemrect.right - itemrect.left;  dims.v = itemrect.bottom - itemrect.top;
	center.h = itemrect.left + dims.h / 2;  center.v = itemrect.top + dims.v / 2;
	worldrect = itemrect;
	/* Draw the whole world as a circle if it's not too big. */
	if (newcircumference < newwidth) {
		/* If world circumference smaller than area, world is not being used at all. */
	} else if (newcircumference / 2 <= dims.h) {
		InsetRect(&worldrect, (dims.h - newcircumference / 2) / 2, (dims.v - newcircumference / 2) / 2);
		EraseOval(&worldrect);
		FrameOval(&worldrect);
	} else {
		EraseRect(&worldrect);
	}
	/* Draw some grid lines. */
	MoveTo(worldrect.left, center.v);
	LineTo(worldrect.right, center.v);
	/* (should draw 30 and 60 lat also, if globe displayed) */
	MoveTo(center.h, worldrect.top);
	LineTo(center.h, worldrect.bottom);
	if (newwidth == newcircumference) {
		/* should show better globe-girdling area */
		arearect.top = center.v - newheight / 2;
		arearect.left = center.h - newwidth / 2;
		arearect.bottom = center.v + newheight / 2;
		arearect.right = center.h + newwidth / 2;
		OffsetRect(&arearect, newlatitude, newlongitude);
		FillRect(&arearect, QD(dkGray));
	} else if (newwidth < newcircumference / 2 && newheight < newcircumference / 2) {
		if (1 /* area is a hexagon */) {
			poly = OpenPoly();		
			special_xform(0 + newheight/2, 0, &llx, &lly);
			MoveTo(llx, lly);
			special_xform(newwidth-1, 0, &lrx, &lry);
		 	LineTo(lrx, lry);
			special_xform(newwidth-1, newheight/2, &rx, &ry);
			LineTo(rx, ry);
	 		special_xform(newwidth-1 - newheight/2, newheight-1, &urx, &ury);
			LineTo(urx, ury);
	 		special_xform(0, newheight-1, &ulx, &uly);
			LineTo(ulx, uly);
	 		special_xform(0, newheight/2, &lx, &ly);
			LineTo(lx, ly);
			LineTo(llx, lly);
			ClosePoly();
			OffsetPoly(poly, center.h - newwidth / 2, center.v - newheight / 2);
			OffsetPoly(poly, newlatitude, newlongitude);
			FillPoly(poly, QD(dkGray));
		} else {
			arearect.top = center.v - newheight / 2;
			arearect.left = center.h - newwidth / 2;
			arearect.bottom = center.v + newheight / 2;
			arearect.right = center.h + newwidth / 2;
			OffsetRect(&arearect, newlatitude, newlongitude);
			FillRect(&arearect, QD(dkGray));
		}
	} else {
		/* (should show a partially wrapped area) */
	}
}

#define put_number_into_ditem(di,num)  \
  GetDItem(worldshapewin, (di), &itemtype, &itemhandle, &itemrect);  \
  NumToString((num), tmpstr);  \
  SetIText(itemhandle, tmpstr);

#define get_number_from_ditem(di,place)  \
  GetDItem(worldshapewin, (di), &itemtype, &itemhandle, &itemrect);  \
  GetIText(itemhandle, tmpstr);  \
  StringToNum(tmpstr, &(place));

pascal Boolean
filter_world_setup(dialog, evt, itemhit)
DialogPtr dialog;
EventRecord *evt;
short *itemhit;
{
	GrafPtr oldport;
	Point pt, dims, center;
	short ditem;
	int wid, hgt;
	char ch;
	Str255 tmpstr;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	switch (evt->what) {
		case mouseDown:
			GetPort(&oldport);
			SetPort(worldshapewin);
			pt = evt->where;
			GlobalToLocal(&pt);
			ditem = FindDItem(worldshapewin, pt) + 1;
			if (ditem == diWorldShapePicture) {
				GetDItem(worldshapewin, diWorldShapePicture, &itemtype, &itemhandle, &itemrect);
				dims.h = itemrect.right - itemrect.left;  dims.v = itemrect.bottom - itemrect.top;
				center.h = itemrect.left + dims.h / 2;  center.v = itemrect.top + dims.v / 2;
				wid = 2 * abs(pt.h - center.h);  hgt = 2 * abs(pt.v - center.v);
				/* (should adjust the cursor to indicate valid places) */
				if (valid_area_shape(wid, hgt, FALSE)) {
					put_number_into_ditem(diWorldShapeWidth, wid);
					put_number_into_ditem(diWorldShapeHeight, hgt);
					/* The outer dialog loop will do the redraw. */
				}
				SetPort(oldport);
				*itemhit = diWorldShapePicture;
				return TRUE;
			} else {
				SetPort(oldport);
			}
			break;
		case keyDown:
			ch = evt->message & charCodeMask;
			if (ch == 3 || ch == 13) {
				*itemhit = diWorldShapeOK;
				return TRUE;
			}
			break;
	}
	return FALSE;
}

/* This dialog asks for the shape and size of a world to generate randomly. */

world_shape_dialog()
{
	int done = FALSE, maybechanged = TRUE, changed = FALSE;
	Str255 dlgtext, tmpstr;
	short ditem;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	if (worldshapewin == nil) {
		worldshapewin = GetNewDialog(dWorldShape, NULL, (DialogPtr) -1L);
		GetDItem(worldshapewin, diWorldShapePicture, &itemtype, &itemhandle, &itemrect);
		SetDItem(worldshapewin, diWorldShapePicture, itemtype, (Handle) draw_world_picture, &itemrect);
		/* Plug all the starting values into dialog items. */
		put_number_into_ditem(diWorldShapeCircumference, newcircumference);
		put_number_into_ditem(diWorldShapeWidth, newwidth);
		put_number_into_ditem(diWorldShapeHeight, newheight);
		put_number_into_ditem(diWorldShapeLatitude, newlatitude);
		put_number_into_ditem(diWorldShapeLongitude, newlongitude);
	}
	while (!done) {
		/* adjust items to reflect current status */
		if (maybechanged) {
			/* Dig the values out of the dialog boxes. */
			get_number_from_ditem(diWorldShapeCircumference, newcircumference);
			get_number_from_ditem(diWorldShapeWidth, newwidth);
			get_number_from_ditem(diWorldShapeHeight, newheight);
			get_number_from_ditem(diWorldShapeLatitude, newlatitude);
			get_number_from_ditem(diWorldShapeLongitude, newlongitude);
			DrawDialog(worldshapewin); /* just to do picture item */
			maybechanged = FALSE;
		}
		/* If the proposed shape is valid, enable the OK button. */
		/* (still need to disable returns etc) */
		GetDItem(worldshapewin, diWorldShapeOK, &itemtype, &itemhandle, &itemrect);
		HiliteControl((ControlHandle) itemhandle,
					  (valid_area_shape(newwidth, newheight, FALSE) ? 0 : 255));
		draw_default_button(worldshapewin, diWorldShapeOK);
		ModalDialog((ModalFilterProcPtr) filter_world_setup, &ditem);
		switch (ditem) {
			case diWorldShapeOK:
				changed = TRUE;
				/* Fall through to next case. */
			case diWorldShapeCancel:
				done = TRUE;
				break;
			case diWorldShapePicture:
			case diWorldShapeCircumference:
			case diWorldShapeWidth:
			case diWorldShapeHeight:
			case diWorldShapeLatitude:
			case diWorldShapeLongitude:
				maybechanged = TRUE;
				break;
			default:
				break;
		}
	}
	DisposDialog(worldshapewin);
	worldshapewin = nil;
	return changed;
}

/* This is a callback that displays the list of sides and their assigned players. */

pascal void
draw_player_setup_list(win, ditem)
WindowPtr win;
short ditem;
{
	int i, advantage = -1;
	Rect entryrect, tmprect;
	char *sidetitle, playertitle[BUFSIZE];
	Player *player;
	Side *side;
	Str255 tmpstr;
	RgnHandle tmprgn;
	GrafPtr oldport;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	GetDItem(playersetupwin, diPlayerSetupList, &itemtype, &itemhandle, &itemrect);
	tmprgn = NewRgn();
	GetClip(tmprgn);
	ClipRect(&itemrect);
	entryrect = itemrect;
	entryrect.bottom = entryrect.top + playerh;
	InsetRect(&entryrect, 1, 1);
	for (i = 0; i < numsides; ++i) {
		/* Clear this entry. */
		tmprect = entryrect;
		InsetRect(&tmprect, -1, -1);
		EraseRect(&tmprect);
		/* Describe the side. */
		side = assignments[i].side;
		draw_side_emblem(playersetupwin, entryrect.left + 2, entryrect.top + 2,
						 16, 16, side_number(side));
		sidetitle = (char *) short_side_title(side);
		if (sidetitle == NULL || strlen(sidetitle) == 0) sidetitle = "(unnamed)";
		MoveTo(entryrect.left + 2 + 16 + 2, entryrect.top + playerbaseline);
		TextFont(newYork);
		TextFace(0);
		DrawText(sidetitle, 0, strlen(sidetitle));
		/* Describe the intended player for the side. */
		player = assignments[i].player;
		long_player_title(playertitle, player, "mac");
		MoveTo(entryrect.left + 2 + 130, entryrect.top + playerbaseline);
		TextFace(italic);
		DrawText(playertitle, 0, strlen(playertitle));
		/* Indicate the player's initial advantage. */
		sprintf(spbuf, "%d", player->advantage);
		MoveTo(entryrect.left + 2 + 260, entryrect.top + playerbaseline);
		TextFace(0);
		DrawText(spbuf, 0, strlen(spbuf));
		/* (should be able to draw/update selection separately) */
		if (i == selectedplayer) {
			tmprect = entryrect;
			InsetRect(&tmprect, -1, -1);
			InvertRect(&tmprect);
			InsetRect(&tmprect, 3, 3);
			InvertRect(&tmprect);
		} else {
			/* Sides may be present, but not in the game; gray them out. */
			if (!side->ingame) gray_out_rect(&entryrect);
			FrameRect(&entryrect);
		}
		OffsetRect(&entryrect, 0, playerh);
	}
	/* Draw the remaining available positions as gray outlines. */
	PenPat(QD(gray));
	for (i = numsides + 1; i < g_sides_max(); ++i) {
		FrameRect(&entryrect);
		OffsetRect(&entryrect, 0, playerh);
	}
	PenNormal();
	SetClip(tmprgn);
	DisposeRgn(tmprgn);
}

pascal void
draw_player_setup_advantage(win, ditem)
WindowPtr win;
short ditem;
{
	int advantage, halfhgt;
	Side *side;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	GetDItem(playersetupwin, diPlayerSetupAdvantage, &itemtype, &itemhandle, &itemrect);
	EraseRect(&itemrect);
	if (updownpicture != nil) {
		DrawPicture(updownpicture, &itemrect);
		if (!selected_a_player()) {
			gray_out_rect(&itemrect);
		} else {
			advantage = selected_player()->advantage;
			side = selected_side();
			halfhgt = (itemrect.bottom - itemrect.top) / 2;
			if (advantage <= side->minadvantage) {
				/* Gray out the down arrow. */
				itemrect.top += halfhgt;
				gray_out_rect(&itemrect);
				itemrect.top -= halfhgt;
			}
			if (side->maxadvantage > 0 && advantage >= side->maxadvantage) {
				/* Gray out the up arrow. */
				itemrect.bottom -= halfhgt;
				gray_out_rect(&itemrect);
				itemrect.bottom += halfhgt;
			}
		}
	}
}

pascal Boolean
filter_player_setup(dialog, evt, itemhit)
DialogPtr dialog;
EventRecord *evt;
short *itemhit;
{
	Player *tmpplayer, *curselected;
	GrafPtr oldport;
	Point pt;
	short ditem;
	char ch;
	int newadvantage;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	switch (evt->what) {
		case mouseDown:
			GetPort(&oldport);
			SetPort(playersetupwin);
			pt = evt->where;
			GlobalToLocal(&pt);
			ditem = FindDItem(playersetupwin, pt) + 1;
			if (ditem == diPlayerSetupList) {
				curselected = selected_player();
				GetDItem(playersetupwin, diPlayerSetupList, &itemtype, &itemhandle, &itemrect);
				select_player((pt.v - itemrect.top) / playerh);
				if (curselected != selected_player()) {
					set_player_setup_button_states();
					DrawDialog(playersetupwin);
				}
				SetPort(oldport);
				return TRUE;
			} else if (ditem == diPlayerSetupAdvantage) {
				GetDItem(playersetupwin, diPlayerSetupAdvantage, &itemtype, &itemhandle, &itemrect);
				if ((tmpplayer = selected_player()) != NULL) {
					if (pt.v - itemrect.top < ((itemrect.bottom - itemrect.top) / 2)) {
						adjust_advantage(tmpplayer, selected_side(), 1);
					} else {
						adjust_advantage(tmpplayer, selected_side(), -1);
					}
				}
			} else {
				SetPort(oldport);
			}
			break;
		case keyDown:
			ch = evt->message & charCodeMask;
			if (ch == 3 || ch == 13) {
				*itemhit = diPlayerSetupOK;
				return TRUE;
			}
			/* Shortcuts to tweak the advantage. */
			if (ch == '+') {
				adjust_advantage(selected_player(), selected_side(), 1);
			} else if (ch == '-') {
				adjust_advantage(selected_player(), selected_side(), -1);
			}
			break;
	}
	return FALSE;
}

adjust_advantage(player, side, amt)
Player *player;
Side *side;
int amt;
{
	int newadvantage;

	if (player == NULL) return;
	newadvantage = player->advantage + amt;
	if (amt > 0 && side->maxadvantage > 0 && newadvantage > side->maxadvantage) {
		SysBeep(20);
		return;
	}
	if (amt < 0 && newadvantage < side->minadvantage) {
		SysBeep(20);
		return;
	}
	if (newadvantage != player->advantage) {
		player->advantage = newadvantage;
		DrawDialog(playersetupwin);
	}
}

select_player(n)
int n;
{
	if ((!between(0, n, numsides-1))
	    || (assignments[n].side && !(assignments[n].side->ingame))) {
	    n = -1;
	}
	selectedplayer = n;
	/* (update ditem directly?) */
}

/* Bring up a dialog that allows rearrangement of the side/player assignments.
   Returns true if accepted, false if cancelled. */

player_setup_dialog()
{
	int done = FALSE, playersok = FALSE, changedselected, changedall;
	short ditem;
	Player *tmpplayer;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	if (playersetupwin == nil) {
		playersetupwin = GetNewDialog(dPlayerSetup, NULL, (DialogPtr) -1L);
		GetDItem(playersetupwin, diPlayerSetupList, &itemtype, &itemhandle, &itemrect);
		SetDItem(playersetupwin, diPlayerSetupList, itemtype, (Handle) draw_player_setup_list, &itemrect);
		GetDItem(playersetupwin, diPlayerSetupAdvantage, &itemtype, &itemhandle, &itemrect);
		SetDItem(playersetupwin, diPlayerSetupAdvantage, itemtype, (Handle) draw_player_setup_advantage, &itemrect);
		updownpicture = (PicHandle) GetResource('PICT', pUpDownPicture);
	}
	/* Always start out with the first player selected. */
	selectedplayer = 0;
	ShowWindow(playersetupwin);
	/* Stay in here until the player accepts the setup or cancels. */
	while (!done) {
		set_player_setup_button_states();
		draw_default_button(playersetupwin, diPlayerSetupOK);
		changedselected = changedall = FALSE;
		/* Sit in the modal dialog until something happens. */
		ModalDialog((ModalFilterProcPtr) filter_player_setup, &ditem);
		switch (ditem) {
			case diPlayerSetupOK:
				playersok = TRUE;
				/* Fall through. */
			case diPlayerSetupCancel:
				done = TRUE;
				break;
			case diPlayerSetupAdd:
				add_side_and_player();
				/* Select the newly-created side/player pair. */
				select_player(numsides - 1);
				init_emblem_images(selected_side());
				changedall = TRUE;
				break;
			case diPlayerSetupRemove:
				remove_side_and_player();
				break;
			case diPlayerSetupRename:
				selected_side()->name = NULL;
				selected_side()->noun = NULL;
				/* always need to clear this cache before renaming... */
				selected_side()->pluralnoun = NULL;
				selected_side()->adjective = NULL;
				make_up_side_name(selected_side());
				init_emblem_images(selected_side());
				changedselected = TRUE;
				break;
			case diPlayerSetupAI:
				tmpplayer = selected_player();
				/* Click between mplayer and not, for now */
				if (tmpplayer->aitypename) {
					tmpplayer->aitypename = NULL;
				} else {
					tmpplayer->aitypename = "mplayer";
				}
				changedselected = TRUE;
				break;
			case diPlayerSetupExchange:
				/* Make the next player in the exchange be the selected one,
				   then multiple clicks on the exchange button will move the
				   selected player down through the list. */
				selectedplayer = exchange_players(selectedplayer, -1);
				changedall = TRUE;
				break;
			default:
				break;
		}
		if (changedall || changedselected) {
			/* (should minimize redrawing) */
			DrawDialog(playersetupwin);
		}
	}
	/* Get rid of this dialog, unlikely to need it again. */
	DisposDialog(playersetupwin);
	playersetupwin = nil;
	return playersok;
}

/* Set the state of the various buttons and controls.  In general, a side/player
   pair must be selected before anything can be done. */	

set_player_setup_button_states()
{
	int locked = g_player_sides_locked(), player = (selected_a_player() && !locked);
	int i, numingame = 0;
	Side *side;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	for (i = 0; i < numsides; ++i) {
		if (assignments[i].side && (assignments[i].side)->ingame) ++numingame;
	}

#define set_player_button(di, val)  \
	GetDItem(playersetupwin, (di), &itemtype, &itemhandle, &itemrect);  \
	HiliteControl((ControlHandle) itemhandle, ((val) ? 0 : 255));

	set_player_button(diPlayerSetupAdd, !locked && numsides < g_sides_max());
	set_player_button(diPlayerSetupRemove, player && !locked && FALSE /* (numsides > g_sides_min()) */);
	set_player_button(diPlayerSetupRename, player && !(selected_side()->nameslocked));
	set_player_button(diPlayerSetupAI, player);
	set_player_button(diPlayerSetupRemote, player && FALSE);
	set_player_button(diPlayerSetupExchange, player && numingame > 1);
}

/* This dialog opens a player-specified file as a game. */

open_game_dialog()
{
	Point pnt;
	/* SFTypelist typelist; */
	long typelist[4];
	SFReply reply;
	char modulename[256];
	Module *module;

	/* Gotta be somewhere... */
	SetPt(&pnt, 100, 100);
	/* Game module are text files. */
	typelist[0] = 'TEXT';
#ifdef THINK_C
	SFGetFile(pnt, "\p", NULL, 1, &typelist, NULL, &reply);
#else
/*	SFGetFile(pnt, "\p", NULL, 1, (SFTypeList) &typelist, NULL, &reply); */
#endif
	if (reply.good) {
		return open_game_from_name_and_volume(reply.fName, reply.vRefNum);
	}
	return FALSE;
}

open_game_from_fsspec(fsspec)
FSSpec *fsspec;
{
	return open_game_from_name_and_volume(fsspec->name, fsspec->vRefNum);
}

open_game_from_name_and_volume(name, vrefnum)
Str255 name;
int vrefnum;
{
	char modulename[256];
	Module *module;

	toggle_debugging(&Debug);
	Dprintf("Name is %30s, vrefnum is %d\n", name+1, vrefnum);
	/* Set the current volume to be where the file is, so path-less
	   fopen() in module loading below will find the file.  Note that this
	   won't work for loading multiple modules from multiple non-library
	   locations, but this dialog can only load one file at a time anyway. */
	SetVol(nil, vrefnum);
	clear_game_modules();
	/* Build a module with the given filename. */
	module = get_game_module(NULL);
	p2c(name, modulename);
	module->filename = copy_string(modulename);
	module->hook = NULL;
	/* This module is effectively the "selected game", whether or not
	   it's loadable. */
	selectedgame = module;
	/* Load the module. */
	mainmodule = module;
	load_game_module(module);
	/* Change cursor back, in case it was different during loading. */
	SetCursor(&QD(arrow));
	if (!module->loaded) {
		init_warning("Could not load a module from %s", module->filename);
		return FALSE;
	}
	/* If the loaded module has bugs, we will get alerts here. */
	check_game_validity();
	/* We can now merge with the "new game" startup sequence. */
	return launch_game();
}

/* This is just the collection of init steps shared by both new and open dialogs. */

launch_game()
{
	/* Bring up the pre-player-setup variants. */
	if (!variants_dialog()) {
		/* Cancelling from variants dialog goes back to menu bar (?) */
		return FALSE;
	}
	/* Load any colors first, we can use them to substitute for patterns. */
	init_terrain_colors();
	/* Set up the images we'll be needing. */
	init_terrain_images();
	init_unit_images();
	/* Note that trial side/player assignment will create the default player if nec. */
	make_trial_assignments();
	init_all_emblem_images();
	/* Ask for modifications to the trial assignment. */
	if (!player_setup_dialog()) {
		/* Cancelled out of player setup, need to go around again. */
		update_new_game_list();
		return FALSE;
	}
	/* Complain if any images were missing.  Will also note if image resource files
	   were never found - might be one of the explanations. */
	check_for_missing_images();
	/* A last-minute patch to ensure reasonable area dimensions. */
	if (area.width == 0) area.width = DEFAULTWIDTH;
	if (area.height == 0) area.height = DEFAULTHEIGHT;
	set_area_shape(area.width, area.height, TRUE);
	open_progress_dialog();
	/* Synthesize the remainder of the initial game setup. */
	run_synth_methods();
	final_init();
	assign_players_to_sides();
	/* No more interminable init calcs (we hope), so close the progress bar. */
	close_progress_dialog();
	/* Init the display now. */
	init_display();
	/* Do first set of turn calcs. */
	run_game(0);
	return TRUE;
}

/* This counts up all the images that had to be manufactured, and maybe puts up an
   alert asking if the player wants to go with the substitutes. */

check_for_missing_images()
{
	int u, t, e;
	Side *side;
	Str255 tmpstr;

	for_all_terrain_types(t) {
		if (timages[t].ersatz) record_missing_image(TTYP, t_type_name(t));
	}
	for_all_unit_types(u) {
		if (uimages[u].ersatz) record_missing_image(UTYP, u_type_name(u));
	}
	for_all_sides(side) {
		e = side_number(side);
		if (eimages[e].ersatz) record_missing_image(3, side_desig(side)); 
	}
	if (missing_images(tmpbuf)) {
		c2p(tmpbuf, tmpstr);
		ParamText(tmpstr, (foundimagesfile ? "\p" : "\p & no image resource files found"), "\p", "\p");
		switch (CautionAlert(aImagesMissing, nil)) {
			case 1:
				/* We decided to live with low-quality images. */
				break;
			case 2:
				/* Ugliness is unbearable, get out now. */
				/* (should do a more controlled exit...) */
				ExitToShell();
				break;
		}
	}
}

/* Rotate around "marching parentheses" cursors to indicate reading is happening. */

announce_read_progress()
{
	if (readprogressors[curcurs] != nil) SetCursor(*(readprogressors[curcurs]));
	curcurs = (curcurs + 1) % NUMcParens;
}

/* Display progress in lengthy initialization processes. */

/* This is the app-defined item that actually draws the progress bar. */

pascal void
draw_progress(win, ditem)
WindowPtr win;
short ditem;
{
	short itemtype;  Handle itemhandle;  Rect itemrect;

	GetDItem(progresswin, diProgressBar, &itemtype, &itemhandle, &itemrect);
	if (progress >= 0) {
		/* Only draw the gray bg initially or when "progress" decreases (would be a
		   bug in synth method, but useful to get visual evidence if it happens). */
		if (lastprogress < 0 || lastprogress > progress) {
			FillRect(&itemrect, QD(gray));
			FrameRect(&itemrect);
		}
		itemrect.right =
			itemrect.left + (progress * (itemrect.right - itemrect.left)) / 100;
		FillRect(&itemrect, QD(black));
	} else {
		EraseRect(&itemrect);
	}
}

/* This starts off the lengthy process. */

open_progress_dialog()
{
	short itemtype;  Handle itemhandle;  Rect itemrect;

	progresswin = GetNewDialog(dProgress, NULL, (DialogPtr) -1L);
	GetDItem(progresswin, diProgressBar, &itemtype, &itemhandle, &itemrect);
	SetDItem(progresswin, diProgressBar, itemtype, (Handle) draw_progress, &itemrect);
	/* Always disable the Stop button for now. */
	/* (should figure out what to do with this) */
	GetDItem(progresswin, diProgressCancel, &itemtype, &itemhandle, &itemrect);
	HiliteControl((ControlHandle) itemhandle, 255);
	curcurs = 0;
	if (progressors[curcurs] != nil) SetCursor(*(progressors[curcurs]));
}

/* This is called to announce the beginning of a lengthy process. */

announce_lengthy_process(msg)
char *msg;
{
	Str255 tmpstr;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	if (progresswin == nil) return;  /* be safe */
	/* Paste in the description of what is going to take so long. */
	GetDItem(progresswin, diProgressText, &itemtype, &itemhandle, &itemrect);
	c2p(msg, tmpstr);
	SetIText(itemhandle, tmpstr);
	/* Initialize the progress bar itself. */
	progress = -1;
	lastprogress = 0;
	ShowWindow(progresswin);
	SelectWindow(progresswin);
	Dprintf("%s;", msg);
#ifdef USE_CONSOLE
	if (Debug) fflush(stdout);
#endif
	curcurs = 0;
	if (progressors[curcurs] != nil) SetCursor(*(progressors[curcurs]));
}

/* This is called periodically to say how much progress has been made. */

announce_progress(pctdone)
int pctdone;
{
	if (progresswin == nil) return;  /* be safe */
	SelectWindow(progresswin);
	lastprogress = progress;
	progress = min(max(0, pctdone), 100);
	/* Redraw the progress bar app-defined item. */
	DrawDialog(progresswin);
	/* Switch to the next cursor in the animation sequence. */
	if (progressors[curcurs] != nil) SetCursor(*(progressors[curcurs]));
	curcurs = (curcurs + 1) % NUMcSynth;
	Dprintf(" %d%%", pctdone);
#ifdef USE_CONSOLE
	if (Debug) fflush(stdout);
#endif
}

/* This is called to signal the end of the lengthy process. */

finish_lengthy_process()
{
	short itemtype;  Handle itemhandle;  Rect itemrect;

	if (progresswin == nil) return;  /* be safe */
	SelectWindow(progresswin);
	/* Replace the text with a "reassuring" but uninformative message. */
	GetDItem(progresswin, diProgressText, &itemtype, &itemhandle, &itemrect);
	SetIText(itemhandle, "\pKeeping busy...");
	progress = -1;
	/* Force a redraw, which will cause the progress bar to be erased. */
	DrawDialog(progresswin);
	Dprintf(" done.\n");
	curcurs = 0;
	if (progressors[curcurs] != nil) SetCursor(*(progressors[curcurs]));
}

/* When we're completely done with all lengthy init processes, discard the dialog. */

close_progress_dialog()
{
	/* Restore the normal cursor. */
	SetCursor(&QD(arrow));
	DisposDialog(progresswin);
	progresswin = nil;
}

/* Kernel callback to put in a default player, assumed to be on
   "this" screen (which we call "mac", for lack of inspiration). */

Player *
add_default_player()
{
	Player *player = add_player();
	
	player->displayname = "mac";
	return player;
}

/* This is a kernel callback that sets the given side to have a user
   interface.  On the Mac, we just record the side as `dside', will
   finish opening later. */

init_ui(side)
Side *side;
{
	if (side != NULL) {
		side->ui = (UI *) xmalloc(sizeof(UI));
		side->ui->active = FALSE;
		dside = side;
		/* Make sure this is allocated.  Note that xmalloc clears, so this
		   will look like it has an empty Pascal string. */
		if (curdatestr == NULL) curdatestr = (char *) xmalloc(50);
	}
}

/* Flag the screen as officially "open", init random globals, and open the
   initial set of windows. */

init_display()
{
	int p;
	Handle handle;

	/* If dside never got assigned, try some hasty repairs. */
	if (dside == NULL) {
		init_warning("Nobody wanted a display!");
		/* Make an arbitrary choice - pick the first side. */
		init_ui(side_n(1));
		if (dside == NULL) {
			/* Geez, nothing is going right. */
			init_error("Can't set up the display!");
		}
	}
	if (bordbitmaps == NULL) {
		bordbitmaps = (BitMap *) xmalloc(NUMPOWERS * sizeof(BitMap));
	}
	if (connbitmaps == NULL) {
		connbitmaps = (BitMap *) xmalloc(NUMPOWERS * sizeof(BitMap));
	}
	/* Init polygon-caching machinery. */
	for (p = 0; p < NUMPOWERS; ++p) {
		polygons[p] = nil;
		lastpolyx[p] = lastpolyy[p] = 0;
		cellrgns[p] = gridcellrgns[p] = nil;
		lastcellrgnx[p] = lastcellrgny[p] = 0;
		lastgridcellrgnx[p] = lastgridcellrgny[p] = 0;
		bordbitmaps[p].rowBytes = 0;
		connbitmaps[p].rowBytes = 0; 
	}
	if ((handle = GetResource('ICN#', 140)) != nil) {
		HLock(handle);
		bordbitmaps[4].baseAddr = *handle;
		bordbitmaps[4].rowBytes = 4;
		SetRect(&(bordbitmaps[4].bounds), 0, 0, 32, 32);
	}
	if ((handle = GetResource('ICN#', 141)) != nil) {
		HLock(handle);
		connbitmaps[4].baseAddr = *handle;
		connbitmaps[4].rowBytes = 4;
		SetRect(&(connbitmaps[4].bounds), 0, 0, 32, 32);
	}
	/* Init assorted list heads and window pointers. */
	maplist = NULL;
	listlist = NULL;
	unitcloseuplist = NULL;
	sidecloseuplist = NULL;
	historywin = nil;
	/* Flag the interface as ready to display stuff.  After this point,
	   any events causing display activity will actually be rendered
	   on the screen. */
	dside->ui->active = TRUE;
	/* Now bring up the initial collection of windows. */
	if (0 /* restore preferred windows */) {
	} else {
		/* The game window is useful orientation material. */
		create_game_window();
		ShowWindow(gamewin);
		/* This needs to come up so that players unfamiliar with the game
		   will have some guidance. */
		instructions_dialog();
		/* This will be the main default display, so it should be on top. */
		create_map(5);
	}
#ifdef DESIGNERS
	/* If we're designing, bring up the palette automatically. */
	if (dside->designer) {
		create_design_window();
	}
#endif /* DESIGNERS */
	Dprintf("Opened Mac display!\n");
}

/* Set up the collection of terrain images/patterns, making a default pattern
   if necessary. */

init_terrain_images()
{
	int i, t;
	Handle pathandle;
	PicHandle pichandle;
	Image *img;

	timages = (ImageFamily *) xmalloc(numttypes * sizeof(ImageFamily));

	for_all_terrain_types(t) {
		init_image_family(&(timages[t]));
		/* (should be in kernel) */
		if (t_image_name(t) != NULL && strlen(t_image_name(t)) > 0) {
			timages[t].name = t_image_name(t);
		} else {
			timages[t].name = t_type_name(t);
		}
		load_image_family(&(timages[t]));
		/* Make a default pattern of box with ttype number in binary inside. */
		if (timages[t].numsizes == 0) {
			/* Count this type as not having a pattern, but only if we can't
			   render the terrain with a solid color. */
			if (!(hasColorQD && t_color_defined(t))) {
				timages[t].ersatz = TRUE;
			}
			/* Build a substitute pattern. */
			img = &(timages[t].images[(timages[t].numsizes)++]);
			img->w = img->h = 8;
			img->minw = img->minh = 1;
			img->maxw = img->maxh = 9999;
			img->pat[0] = 0x00;
			img->pat[1] = 0x7f;
			img->pat[2] = 0x41;
			img->pat[3] = 0x40 | (t << 1) | 0x01;
			img->pat[4] = 0x40 | (t << 1) | 0x01;
			img->pat[5] = 0x40 | (t << 1) | 0x01;
			img->pat[6] = 0x41;
			img->pat[7] = 0x7f;
			img->patdefined = TRUE;
		}
	}
}

interp_named_color(name, grayvar, colorvar)
char *name;
enum grays *grayvar;
RGBColor *colorvar;
{
	ImageColor tmpcolor;

	if (strcmp(name, "black") == 0) {
		*grayvar = blackgray;
		colorvar->red = colorvar->green = colorvar->blue = 0;
	} else if (strcmp(name, "dark gray") == 0) {
		*grayvar = mediumgray;
		colorvar->red = colorvar->green = colorvar->blue = 16384;
	} else if (strcmp(name, "gray") == 0) {
		*grayvar = mediumgray;
		colorvar->red = colorvar->green = colorvar->blue = 32768;
	} else if (strcmp(name, "light gray") == 0) {
		*grayvar = mediumgray;
		colorvar->red = colorvar->green = colorvar->blue = 49000;
	} else if (strcmp(name, "white") == 0) {
		*grayvar = whitegray;
		colorvar->red = colorvar->green = colorvar->blue = 65535;
	} else if (hasColorQD) {
		tmpcolor.name = name;
		load_image_color(&tmpcolor);
		colorvar->red = tmpcolor.r;
		colorvar->green =tmpcolor.g;
		colorvar->blue = tmpcolor.b;
	}
}

/* Allocate space and load a named color for each terrain type
   that defines one. */

init_terrain_colors()
{
	int t;

	if (hasColorQD) {
		tcolors = (ImageColor **) xmalloc(numttypes * sizeof(ImageColor *));
		for_all_terrain_types(t) {
			if (t_color(t) != NULL && strlen(t_color(t)) > 0) {
				tcolors[t] = (ImageColor *) xmalloc(sizeof(ImageColor));
				tcolors[t]->name = t_color(t);
				load_image_color(tcolors[t]);
			}
		}
	}
	interp_named_color(g_grid_color(), &gridgray, &gridcolor);
	interp_named_color(g_unseen_color(), &unseengray, &unseencolor);
	if (strcmp(g_grid_color(), g_unseen_color()) == 0) gridmatchesunseen = TRUE;
	blackcolor.red = blackcolor.green = blackcolor.blue = 0;
}

/* Allocate and fill in images for all the unit types. */

init_unit_images()
{
	int u, i;
	Rect tmprect;
	Str255 tmpstr;
	Image *img;
	Handle sicnhandle, maskhandle;
	PicHandle pichandle;
	RgnHandle tmprgn;

	uimages = (ImageFamily *) xmalloc(numutypes * sizeof(ImageFamily));

	for_all_unit_types(u) {
		init_image_family(&(uimages[u]));
		/* (should be in kernel) */
		if (u_image_name(u) && strlen(u_image_name(u)) > 0) {
			uimages[u].name = u_image_name(u);
		} else {
			uimages[u].name = u_internal_name(u);
		}
		load_image_family(&(uimages[u]));
		/* Default unit image puts its name inside a box shape. */
		if (uimages[u].numsizes == 0) {
			uimages[u].ersatz = TRUE;
			img = &(uimages[u].images[(uimages[u].numsizes)++]);
			tmprgn = NewRgn();
			GetClip(tmprgn);
			SetRect(&tmprect, 0, 0, 32, 32);
			ClipRect(&tmprect);
			pichandle = OpenPicture(&tmprect);
			InsetRect(&tmprect, 2, 2);
			FillRect(&tmprect, QD(white));
			InsetRect(&tmprect, 1, 1);
			FrameRect(&tmprect);
			MoveTo(2, 16);
			DrawText(u_type_name(u), 0, strlen(u_type_name(u)));
			ClosePicture();
			SetClip(tmprgn);
			DisposeRgn(tmprgn);
			img->monopict = pichandle;
			img->istile = FALSE;
		}
	}
}

/* Load/create all the images of each side's emblem. */

init_all_emblem_images()
{
	Side *side2;

	eimages = (ImageFamily *) xmalloc((MAXSIDES + 1) * sizeof(ImageFamily));

	for_all_sides_plus_indep(side2) {
		init_emblem_images(side2);
	}
}

init_emblem_images(side2)
Side *side2;
{
	int e = side_number(side2), i;
	Rect tmprect;
	Str255 tmpstr;
	Image *img;
	Handle sicnhandle, maskhandle;
	PicHandle pichandle;
	RgnHandle tmprgn;
	
	init_image_family(&(eimages[e]));
	if (side2 != NULL && side2->emblemname && strlen(side2->emblemname) > 0) {
		eimages[e].name = side2->emblemname;
	}
	load_image_family(&(eimages[e]));
	/* Default image is SICN-sized number.  Use only if no images have been found
	   and the emblem name is not "none". */
	if (eimages[e].numsizes == 0
		&& !(eimages[e].name && strcmp(eimages[e].name, "none") == 0)) {
		img = &(eimages[e].images[(eimages[e].numsizes)++]);
		img->w = img->h = 16;
		img->minw = img->minh = 8;
		img->maxw = img->maxh = 64;
		sprintf(tmpbuf, "s%d", e);
		c2p(tmpbuf, tmpstr);
		if ((sicnhandle = GetNamedResource('SICN', tmpstr)) != nil) {
			/* Image itself is the first 32 bytes, mask is second 32 if present. */
			img->monosicn = sicnhandle;
			maskhandle = (Handle) NewHandle(32);
			if (SizeResource(sicnhandle) >= 64) {
				for (i = 0; i < 32; ++i) {
					(*maskhandle)[i] = (*sicnhandle)[i+32];
				}
			} else {
				/* Set up an all-white background. */
				for (i = 0; i < 32; ++i) {
					(*maskhandle)[i] = 0xff;
				}
			}
			img->masksicn = maskhandle;
		} else {
			/* If we couldn't even find a number sicn, build a crude substitute pict. */
			eimages[e].ersatz = TRUE;
			tmprgn = NewRgn();
			GetClip(tmprgn);
			SetRect(&tmprect, 0, 0, 16, 16);
			ClipRect(&tmprect);
			pichandle = OpenPicture(&tmprect);
			FillRect(&tmprect, QD(white));
			MoveTo(2, 12);
			DrawText(tmpbuf+1, 0, strlen(tmpbuf+1));
			ClosePicture();
			SetClip(tmprgn);
			DisposeRgn(tmprgn);
			img->monopict = pichandle;
		}
		/* Emblems never tile. */
		img->istile = FALSE;
	}
}
