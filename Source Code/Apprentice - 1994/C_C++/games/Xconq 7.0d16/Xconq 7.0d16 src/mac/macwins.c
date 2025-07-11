/* Copyright (c) 1992, 1993 Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Handling of assorted minor windows for the Mac interface. */

#include "conq.h"
#include "mac.h"

/* #include <time.h> */

HelpNode *find_help_node();

Unit *get_selected_construction_unit();

extern time_t realtimeturnplaystart;
extern time_t realtimegamestart;

char *unit_namer();
char *propose_unit_name();
UnitVector *add_unit_to_vector();

/* Globals for the instructions window. */

WindowPtr instructionswin = nil;

TEHandle instructionstext = nil;

/* Globals for the game window. */

WindowPtr gamewin = nil;

int gamewinw = 200;
int gametoph = 32;
int gametoptoph = 18;
int gamesidehgt = 24;

Handle aisicnhandle = nil;
Handle facesicnhandle[3];

char *gameprogressstr = "";

time_t lastnow;

/* Globals for the construction window. */

WindowPtr constructionwin = nil;

ListHandle constructionunitlist = nil;
ListHandle constructiontypelist = nil;

int constructmargin = 5;
int constructtop = 32;

/* This is the vector of units that can do construction or research. */

UnitVector *possibleunits = NULL;

int numposstypes;
int *possibletypes = NULL;

int currunlength = 99;

ControlHandle constructbutton;
ControlHandle researchbutton;

TEHandle runlengthtext = nil;

Rect unitlistrect, typelistrect;

/* Globals for unit closeup windows. */

int lastunitcloseuph = -1, lastunitcloseupv = -1;

/* Globals for the history window. */

WindowPtr historywin = nil;

HistEvent **histcontents = NULL;

int numhistcontents = 0;

ControlHandle histvscrollbar;

int histlinespacing = 15; /* (should derive from font) */

HistEvent *firstvisevt = NULL;

int numvisevents;

int numvishistlines;

int totalhistlines = 0;

/* Globals for the help window. */

DialogPtr helpwin = nil;

TEHandle helptopic = nil;

TEHandle helptext = nil;

ControlHandle helpvscrollbar;
ControlHandle helphscrollbar;

ControlHandle topicsbutton;
ControlHandle helpbutton;
ControlHandle prevbutton;
ControlHandle nextbutton;
ControlHandle backbutton;

struct a_helpnode *curhelpnode = NULL;
char *helpstring = NULL;
int helpscreentop = 0;

/* The help node that tells about how to use the help system. */

HelpNode *helphelpnode = NULL;

#define NODESTACKSIZE 50

HelpNode **nodestack;

int nodestackpos;

/* The instructions window. */

pascal void
draw_instructions_text(win, ditem)
WindowPtr win;
short ditem;
{
	GrafPtr oldport;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	GetDItem(instructionswin, diInstructionsText, &itemtype, &itemhandle, &itemrect);
	GetPort(&oldport);
	SetPort(instructionswin);
	TextSize(14);
	TEUpdate(&itemrect, instructionstext);
	/* This makes the title item draw big. */
	TextSize(18);
	SetPort(oldport);
}

/* Bring up the dialog with instructions on how to play. */

instructions_dialog()
{
	if (instructionswin == nil) {
		create_instructions_dialog();
	}
	ShowWindow(instructionswin);
	SelectWindow(instructionswin);
}

create_instructions_dialog()
{
	Obj *instructions, *rest;
	char *str;
	Str255 tmpstr;
	Rect destrect, viewrect;
	short itemtype;  Handle itemhandle;  Rect itemrect;
	extern Module *selectedgame;

	instructionswin = GetNewDialog(dInstructions, nil, (DialogPtr) -1L);
	GetDItem(instructionswin, diInstructionsTitle, &itemtype, &itemhandle, &itemrect);
	c2p(((selectedgame && selectedgame->title) ? selectedgame->title : ""), tmpstr);
	SetIText(itemhandle, tmpstr);
	GetDItem(instructionswin, diInstructionsText, &itemtype, &itemhandle, &itemrect);
	SetDItem(instructionswin, diInstructionsText, itemtype, (Handle) draw_instructions_text, &itemrect);
	destrect = itemrect;
	viewrect = itemrect;
	SetPort(instructionswin);
	/* All text will be in Times. */
	/* (should put sizes in a resource so can be edited) */
	TextFont(times);
	TextSize(14);
	instructionstext = TENew(&destrect, &viewrect);
	TESetSelect(0, 32767, instructionstext);
	TEDelete(instructionstext);
	if (selectedgame && (instructions = selectedgame->instructions) != lispnil) {
		if (stringp(instructions)) {
			str = c_string(instructions);
			TEInsert(str, strlen(str), instructionstext);
		} else {
		    for (rest = instructions; rest != lispnil; rest = cdr(rest)) {
		    	if (stringp(car(rest))) {
		    		str = c_string(car(rest));
		    		/* An empty line is a paragraph break. */
		    		if (strlen(str) == 0) {
		    			str = "\r\r";
		    		}
		    		TEInsert(str, strlen(str), instructionstext);
		    		TESetSelect(100000, 100000, instructionstext);
		    		/* Always insert a blank between strings, since they are
		    		   usually word breaks. */
		    		TEInsert(" ", 1, instructionstext);
		    		TESetSelect(100000, 100000, instructionstext);
		    	}
	    	}
	    }
	} else {
		str = "(no instructions available)";
		TESetText(str, strlen(str), instructionstext);
	}
	/* This makes the title item draw big. */
	TextSize(18);
	add_window_menu_item("Instructions", instructionswin);
}

hit_instructions_dialog(dialog, itemhit, evt)
DialogPtr dialog;
short itemhit;
EventRecord *evt;
{
	short itemtype;  Handle itemhandle;  Rect itemrect;
	
	switch (itemhit) {
		case diInstructionsHelp:
			/* Just jump to the help dialog. */
			help_dialog();
			break;
	}
	return TRUE;
}

/* The game progress window. */

/* Create the game progress window. */

create_game_window()
{
	extern int numscores;

	/* Create the window, color if possible, since emblems may be in color. */
	if (hasColorQD) {	
		gamewin = GetNewCWindow(wGame, NULL, (WindowPtr) -1L);
	} else {
		gamewin = GetNewWindow(wGame, NULL, (WindowPtr) -1L);
	}
	/* Add an additional row of space for each two scorekeepers. */
	if (keeping_score()) {
		gamesidehgt += 15 * ((numscorekeepers + 1) / 2);
	}
	/* (should allocate extra display width if game is realtime per side) */
	/* This is not growable, so we have to ensure it's big enough to start with. */
	SizeWindow(gamewin, gamewinw, numsides * gamesidehgt + gametoph, 1);
	/* Get handles to useful sicns. */
	aisicnhandle = GetNamedResource('SICN', "\pmplayer");
	facesicnhandle[0] = GetNamedResource('SICN', "\phostile");
	facesicnhandle[1] = GetNamedResource('SICN', "\pneutral");
	facesicnhandle[2] = GetNamedResource('SICN', "\pfriendly");
}

draw_game()
{
	Side *side2;
	GrafPtr oldport;

	if (gamewin == nil) return;
	GetPort(&oldport);
	SetPort(gamewin);
	EraseRect(&gamewin->portRect);
	draw_game_date();
	draw_game_progress();
	/* Draw a solid separating line between date info and side list. */
	MoveTo(0, gametoph);
	Line(gamewin->portRect.right, 0);
	for_all_sides(side2) {
		draw_game_side(side2);
	}
	SetPort(oldport);
}

/* Display the current time and date and any realtime countdowns. */

draw_game_date()
{
	Rect tmprect;
	extern int endofgame;

	SetRect(&tmprect, 0, 0, gamewin->portRect.right, gametoptoph - 1);
	EraseRect(&tmprect);
	MoveTo(10, 12);
	TextFace(bold);
	DrawString(curdatestr);
	TextFace(0);
	/* (should draw season name here somewhere?) */
	draw_game_clocks();
	if (endofgame) {
		gray_out_rect(&tmprect);
	}
#ifdef DEBUGGING
    /* Indicate the state of all the debug flags. */
	if (Debug || DebugM || DebugG) {
		sprintf(spbuf, "%c%c%c",
				(Debug ? 'D' : ' '), (DebugM ? 'M' : ' '), (DebugG ? 'G' : ' '));
		MoveTo(tmprect.right - 30, 12);
		DrawText(spbuf, 0, strlen(spbuf));
	}
#endif /* DEBUGGING */
}

draw_game_clocks()
{
	int elapsed;
    time_t now;
	Str255 tmpstr;
	Rect tmprect;

	if (g_rt_for_game() > 0) {
		time(&now);
    	elapsed = (int) difftime(now, realtimegamestart);
		time_desc(spbuf, g_rt_for_game() - elapsed, g_rt_for_game());
		SetRect(&tmprect, 100, 0, 100 + 100 /*TextWidth(spbuf, 0, strlen(spbuf))*/, 10);
		EraseRect(&tmprect);
		MoveTo(100, 10);
		DrawText(spbuf, 0, strlen(spbuf));
		lastnow = now;
	}
	if (g_rt_per_turn() > 0) {
		time(&now);
    	elapsed = (int) difftime(now, realtimeturnplaystart);
		time_desc(spbuf, g_rt_per_turn() - elapsed, g_rt_per_turn());
		SetRect(&tmprect, 100, 10, 100 + 100 /*TextWidth(spbuf, 0, strlen(spbuf))*/, 20);
		EraseRect(&tmprect);
		MoveTo(100, 20);
		DrawText(spbuf, 0, strlen(spbuf));
		lastnow = now;
	}
	/* (should draw per-side clocks) */
}

draw_game_progress()
{
	Rect tmprect;

	SetRect(&tmprect, 0, gametoptoph, gamewin->portRect.right, gametoph - 1);
	EraseRect(&tmprect);
	MoveTo(1, gametoptoph + 12);
	DrawText(gameprogressstr, 0, strlen(gameprogressstr));
}

/* Draw info about a given side. */

draw_game_side(side2)
Side *side2;
{
	int s2 = side_number(side2);
	int sx = 20, sy = (s2 - 1) * gamesidehgt + gametoph;
	Str255 tmpstr;
	Rect tmprect;
	RGBColor tmpcolor;

#if 0
	/* (this should be added to emblem drawing as "shadow" option, and set up to use
	the emblem's mask properly) */
	/* Fill offset rect with gray, for contrast with white parts of emblem. */
	SetRect(&tmprect, 2 + 1, sy + 4 + 1, 2 + 16 + 1, sy + 4 + 16 + 1);
	if (hasColorQD) {
		tmpcolor.red = tmpcolor.green = tmpcolor.blue = 49000;
		RGBForeColor(&tmpcolor);
		PaintRect(&tmprect);
		/* Restore the previous color. */
		tmpcolor.red = tmpcolor.green = tmpcolor.blue = 0;
		RGBForeColor(&tmpcolor);
	} else {
		FillRect(&tmprect, QD(gray));
	}
#endif
	draw_side_emblem(gamewin, 2, sy + 4, 16, 16, s2);
	sprintf(spbuf, "%s", short_side_title(side2));
	MoveTo(sx, sy + 12);
	/* Put the name of our side in boldface. */
	TextFace((side2 == dside ? bold : 0));
	DrawText(spbuf, 0, strlen(spbuf));
	TextFace(0);
	if (side_has_ai(side2) && side2->ingame) {
		/* Show that the side is run by an AI. */
		plot_sicn(gamewin, 182, sy + 2, aisicnhandle, 0, TRUE);
	}
	if (side2 != dside && side2->ingame && side_has_ai(side2) /* doesn't account for other humans */) {
		/* Indicate attitude of other side. */
		plot_sicn(gamewin, 164, sy + 2,
			facesicnhandle[feeling_towards(side2, dside)], 0, TRUE);
	}
	draw_side_status(side2);
	/* Draw a separating line. */
	PenPat(QD(gray));
	MoveTo(0, sy + gamesidehgt);
	Line(gamewin->portRect.right, 0);
	PenNormal();
}

feeling_towards(side, side2)
Side *side, *side2;
{
	if (side_has_ai(side) && should_try_to_win(side)) {
		return 0;
	} else {
		return 1;
	}
}

/* Draw the current details about a side. */

draw_side_status(side2)
Side *side2;
{
	int sx, sy = (side_number(side2) - 1) * gamesidehgt + gametoph, i, h, v;
	int totacp, resvacp, acpleft, percentleft, percentresv;
	Rect siderect, tmprect, progressrect;
	extern int endofgame;
    Scorekeeper *sk;

	SetRect(&siderect, 0, sy + 1, gamewin->portRect.right, sy + gamesidehgt);
	if (!side2->ingame || endofgame) {
		gray_out_rect(&siderect);
		if (side_won(side2)) {
			/* (should) Indicate that this side has won. */
			/* draw like a trophy or flourishes or some such?) */
		} else if (side_lost(side2)) {
			/* Draw a line crossing out the loser.  Simple and obvious. */
			MoveTo(1, sy + 8);  Line(gamewin->portRect.right - 3, 0);
		}
	} else {
		/* Set up and clear the area where we show progress. */
		SetRect(&progressrect, 20, sy + 12 + 4, 20 + 100, sy + 12 + 4 + 7);
		EraseRect(&progressrect);
		/* Show the current acp totals/progress of the side. */
		/* This is not quite the security hole it might seem,
		   you don't get much value out of seeing how far along each side is. */
		totacp = side_initacp(side2);
		if (totacp > 0) {
			FrameRect(&progressrect);
			acpleft = side_acp(side2);
			resvacp = side_acp_reserved(side2);
			if (totacp > 0) {
				percentleft = (100 * acpleft) / totacp;
				percentleft = max(0, min(99, percentleft));
				percentresv = (100 * resvacp) / totacp;
				percentresv = max(0, min(99, percentresv));
			} else {
				percentleft = percentresv = 0;
			}
			if (percentleft > 0) {
				tmprect = progressrect;
				InsetRect(&tmprect, 1, 1);
				tmprect.right = tmprect.left + percentleft;
				FillRect(&tmprect, QD(black));
			}
			if (percentresv > 0) {
				PenPat(QD(gray));
				MoveTo(progressrect.left + 1 + percentresv, progressrect.top);
				Line(0, 6);
				PenNormal();
				/* (or could draw a grayish area??) */
			}
		} else {
			/* Draw a gray frame to indicate that this side has no units
			   that can actually do anything. */
			PenPat(QD(gray));
			FrameRect(&progressrect);
			PenNormal();
		}
		/* (should this be a generic kernel test?) */
		if (side2->finishedturn || !(side_has_ai(side2) || side_has_display(side2))) {
			tmprect = progressrect;
			InsetRect(&tmprect, 1, 1);
			gray_out_rect(&tmprect);
		}
	}
	if (keeping_score()) {
		int i = 0;

		siderect.top += 24;
		EraseRect(&siderect);
		for_all_scorekeepers(sk) {
			if (sk->title != NULL) {
				sprintf(spbuf, "%s", sk->title);
			} else {
				sprintf(spbuf, "SK #%d", sk->id);
			}
			if (sk->scorenum >= 0) {
				tprintf(spbuf, ": %d", side2->scores[sk->scorenum]);
			}
			MoveTo(10 + (((i & 1) > 0) ? gamewinw/2 : 0) , siderect.top + 10 + 15 * (i/2));
			DrawText(spbuf, 0, strlen(spbuf));
			++i;
		}
	}
}

do_mouse_down_game()
{
}

/* The construction planning window. */

create_construction_window()
{
	int done = FALSE, u, num, wid, hgt;
	short ditem, i;
	Unit *unit;
	Point pnt, cellsize;
	Cell tmpcell;
	Rect listrect, destrect, viewrect, tmprect;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	if (hasColorQD) {
		constructionwin = GetNewCWindow(wConstruction, NULL, (WindowPtr) -1L);
	} else {
		constructionwin = GetNewWindow(wConstruction, NULL, (WindowPtr) -1L);
	}
	constructbutton = GetNewControl(cConstructButton, constructionwin);
	researchbutton = GetNewControl(cResearchButton, constructionwin);
	SetPort(constructionwin);
	TextFont(monaco);
	TextSize(9);
	/* Set up the list of all constructing units. */
	calc_construction_rects();
	tmprect = unitlistrect;
	tmprect.right -= sbarwid;
	SetRect(&listrect, 0, 0, 1, 0);
	SetPt(&cellsize, 300, 12);
	/* Create the list of units itself. */
	constructionunitlist =
		LNew(&tmprect, &listrect, cellsize, 128, constructionwin,
			 FALSE, FALSE, FALSE, TRUE);
	/* Now set up the list of types. */
	tmprect = typelistrect;
	tmprect.right -= sbarwid;
	SetRect(&listrect, 0, 0, 1, 0);
	/* (should calc this from the desired font) */
	SetPt(&cellsize, 300, 12);
	constructiontypelist =
		LNew(&tmprect, &listrect, cellsize, 128, constructionwin,
			 FALSE, FALSE, FALSE, TRUE);
	init_construction_lists();
	/* (should try to keep the window out of the way of the frontmost map?) */
	ShowWindow(constructionwin);
}

init_construction_lists()
{
	int u, tm, num, i;
	Unit *unit;
	Point cellsize;
	Cell tmpcell, tmpcell2;
	Rect listrect, destrect, viewrect;

	/* Update the list of units. */
	LDoDraw(0, constructionunitlist);
	LDelRow(0, 0, constructionunitlist);
	SetPt(&tmpcell, 0, 0);
	/* Create the vector of constructing units, at a reasonable initial size. */
	if (possibleunits == NULL) {
		possibleunits = make_unit_vector(max(50, numunits));
	}
	clear_unit_vector(possibleunits);
	for_all_side_units(dside, unit) {
		maybe_add_unit_to_construction_list(unit);
	}
	LDoDraw(1, constructionunitlist);
	/* Update the list of types. */
	LDoDraw(0, constructiontypelist);
	LDelRow(0, 0, constructiontypelist);
	SetPt(&tmpcell, 0, 0);
	if (possibletypes == NULL) {
		possibletypes = (int *) xmalloc(numutypes * sizeof(int));
	}
	numposstypes = 0;
	for_all_unit_types(u) {
		if (1 /* could be built by some unit that could be on the side */) {
			LAddRow(1, tmpcell.v, constructiontypelist);
			constructible_desc(spbuf, dside, u, NULL);
			LSetCell(spbuf, strlen(spbuf), tmpcell, constructiontypelist);
			++tmpcell.v;
			possibletypes[numposstypes++] = u;
		}
	}
	LDoDraw(1, constructiontypelist);
	adjust_construction_controls();
}

reinit_construction_lists()
{
	init_construction_lists();
}

/* Draw the construction window by updating the lists and framing them. */

draw_construction()
{
	int u;
	Point tmpcell;
	Rect tmprect;

	calc_construction_rects();
	LUpdate(constructionwin->visRgn, constructionunitlist);
	tmprect = unitlistrect;
	InsetRect(&tmprect, -1, -1);
	FrameRect(&tmprect);
#if 0
	for_all_unit_types(u) {
		if (1 /* could be built by some unit that could be on the side */) {
			constructible_desc(spbuf, dside, u, possibleunits);
			SetPt(&tmpcell, 0, u);
			LSetCell(spbuf, strlen(spbuf), tmpcell, constructiontypelist);
		}
	}
#endif
	LUpdate(constructionwin->visRgn, constructiontypelist);
	tmprect = typelistrect;
	InsetRect(&tmprect, -1, -1);
	FrameRect(&tmprect);
	/* Maybe show the construct button as the default. */
	draw_construction_default();
}

/* Draw a heavy outline around the construction button. */

draw_construction_default()
{
	Rect tmprect;
	GrafPtr oldport;

	GetPort(&oldport);
	SetPort(constructionwin);
	tmprect = (*constructbutton)->contrlRect;
	PenSize(3, 3);
	InsetRect(&tmprect, -4, -4);
	if ((*constructbutton)->contrlHilite != 0) {
		PenMode(patBic);
	}
	FrameRoundRect(&tmprect, 16, 16);
	PenNormal();
	SetPort(oldport);
}

/* Figure out how to subdivide the construction window for the two lists. */

calc_construction_rects()
{
	int wid, hgt, divide;
	Rect tmprect;

	tmprect = constructionwin->portRect;
	wid = tmprect.right - tmprect.left - sbarwid;
	hgt = tmprect.bottom - tmprect.top - sbarwid;
	if (wid / 2 > 220 /* maxtypewid */) {
		divide = wid - 220;
	} else {
		divide = wid / 2;
	}
	SetRect(&unitlistrect, 0, constructtop, divide, hgt);
	InsetRect(&unitlistrect, constructmargin, constructmargin);
	SetRect(&typelistrect, divide, constructtop, wid, hgt);
	InsetRect(&typelistrect, constructmargin, constructmargin);
}

activate_construction(activate)
int activate;
{
	LActivate(activate, constructionunitlist);
	LActivate(activate, constructiontypelist);
}

Unit *
get_selected_construction_unit()
{
	Point tmpcell;
	Unit *unit;

	SetPt(&tmpcell, 0, 0);
	if (LGetSelect(TRUE, &tmpcell, constructionunitlist)) {				
		if (tmpcell.v < possibleunits->numunits) {
			unit = (possibleunits->units)[tmpcell.v].unit;
			if (is_acting(unit)) return unit;
		}
	}
	return NULL;
}

get_selected_construction_type()
{
	int u;
	Point tmpcell;

	SetPt(&tmpcell, 0, 0);
	if (LGetSelect(TRUE, &tmpcell, constructiontypelist)) {				
		if (tmpcell.v < numposstypes) {
			return possibletypes[tmpcell.v];
		}
	}
	return NONUTYPE;
}

do_mouse_down_construction(mouse, mods)
Point mouse;
int mods;
{
	ControlHandle control;
	short part, value;
	int u;
	Point pt;
	Unit *unit;

	part = FindControl(mouse, constructionwin, &control);
	if (control == constructbutton) {
		if ((unit = get_selected_construction_unit()) != NULL) {
			if ((u = get_selected_construction_type()) != NONUTYPE) {
				push_build_task(unit, u, currunlength);
				update_construction_unit_list(unit);
				return;
			}
		}
	} else if (control == researchbutton) {
		if ((unit = get_selected_construction_unit()) != NULL) {
			if ((u = get_selected_construction_type()) != NONUTYPE) {
				push_research_task(unit, u, u_tech_to_build(u));
				update_construction_unit_list(unit);
				return;
			}
		}
	} else if (PtInRect(mouse, &unitlistrect)) {
		LClick(mouse, mods, constructionunitlist);
		/* Update the type list to show what could be built and in how long. */
		update_type_list_for_unit(get_selected_construction_unit());
	} else if (PtInRect(mouse, &typelistrect)) {
		LClick(mouse, mods, constructiontypelist);
		/* Update the unit list to show what could build the type */
		update_unit_list_for_type(get_selected_construction_type());
	} else {
		/* Click was not in any useful part of the window. */ 
	}
}

/* Highlight exactly one specific unit in the construction window, and unhighlight
   any others. */

select_unit_in_construction_window(unit)
Unit *unit;
{
	int i;
	Point tmpcell;

	for (i = 0; i < possibleunits->numunits; ++i) {
		SetPt(&tmpcell, 0, i);
		LSetSelect((unit == (possibleunits->units)[i].unit), tmpcell, constructionunitlist);
		LAutoScroll(constructionunitlist);
	}
	update_type_list_for_unit(get_selected_construction_unit());
}

select_type_in_construction_window(u)
int u;
{
	int i;
	Point tmpcell;

	for (i = 0; i < numutypes; ++i) {
		SetPt(&tmpcell, 0, i);
		LSetSelect((u == i), tmpcell, constructiontypelist);
		LAutoScroll(constructiontypelist);
	}
	update_unit_list_for_type(get_selected_construction_type());
}

/* Given a unit (which may be any unit), update the list of constructing units. */

update_construction_unit_list(unit)
Unit *unit;
{
	int i, u;
	Point tmpcell;

	if (constructionwin == nil) return;
	u = get_selected_construction_type();
	/* We need to look for it even if it might not be ours, since it might
	   have been captured or otherwise lost, and needs to be removed. */
	for (i = 0; i < possibleunits->numunits; ++i) {
		if (unit == (possibleunits->units)[i].unit) {
			SetPt(&tmpcell, 0, i);
			if (is_active(unit)
				&& can_build(unit)
				&& side_controls_unit(dside, unit)) {
				construction_desc(spbuf, unit, u);
				LSetCell(spbuf, strlen(spbuf), tmpcell, constructionunitlist);
			} else {
				remove_unit_from_vector(possibleunits, unit, i);
				LDelRow(1, tmpcell.v, constructionunitlist);
			}
			return;
		}
	}
	/* Unit was not found, try to add it to the list. */
	maybe_add_unit_to_construction_list(unit);
}

maybe_add_unit_to_construction_list(unit)
Unit *unit;
{
	Point tmpcell;

	if (is_acting(unit)
		&& can_build(unit)
		&& side_controls_unit(dside, unit)) {
		/* Add this unit to the vector of constructing units. */
		possibleunits = add_unit_to_vector(possibleunits, unit, 0);
		/* (should sort and maybe rearrange list here) */
		/* Add a row at the end of the list. */
		SetPt(&tmpcell, 0, possibleunits->numunits - 1);
		LAddRow(1, possibleunits->numunits - 1, constructionunitlist);
		construction_desc(spbuf, unit, get_selected_construction_type());
		LSetCell(spbuf, strlen(spbuf), tmpcell, constructionunitlist);
	}
}

update_unit_list_for_type(u)
int u;
{
	int i;
	Point tmpcell;
	Unit *unit;

	for (i = 0; i < possibleunits->numunits; ++i) {
		if ((unit = (possibleunits->units)[i].unit) != NULL) {
			SetPt(&tmpcell, 0, i);
			if (is_acting(unit) && unit->side == dside) {
				construction_desc(spbuf, unit, u);
				LSetCell(spbuf, strlen(spbuf), tmpcell, constructionunitlist);
			} else {
/*				LDelRow(1, tmpcell.v, constructionunitlist); */
				LSetCell("", 0, tmpcell, constructionunitlist);
			}
		}
	}
	adjust_construction_controls();
}

update_construction_type_list()
{
	int i, u;
	Point tmpcell;

	if (constructionwin == nil) return;
	u = get_selected_construction_type();
	update_type_list_for_unit(get_selected_construction_unit());
}

update_type_list_for_unit(unit)
Unit *unit;
{
	int i, u;
	Point tmpcell;

	for_all_unit_types(u) {
		if (1 /* could be built by some unit that could be on the side */) {
			SetPt(&tmpcell, 0, u);
			constructible_desc(spbuf, dside, u, get_selected_construction_unit());
			LSetCell(spbuf, strlen(spbuf), tmpcell, constructiontypelist);
		}
	}
	adjust_construction_controls();
}

/* Enable/disable controls according to whether the selected list elements can
   do construction activities. */

adjust_construction_controls()
{
	int u, canconstruct = FALSE, canresearch = FALSE;
	Unit *unit;

	if ((unit = get_selected_construction_unit()) != NULL) {
		if ((u = get_selected_construction_type()) != NONUTYPE) {
			if (uu_acp_to_create(unit->type, u) > 0) canconstruct = TRUE;
			if (uu_acp_to_research(unit->type, u) > 0) canresearch = TRUE;
		}
	}
	HiliteControl(constructbutton, (canconstruct ? 0 : 255));
	HiliteControl(researchbutton, (canresearch ? 0 : 255));
	draw_construction_default();
}

/* Resize the construction window to the given size. */

grow_construction(h, v)
short h, v;
{
	int wid, hgt, leftwid;
	Rect tmprect;

	EraseRect(&constructionwin->portRect);
	SizeWindow(constructionwin, h, v, 1);
	/* Recalculate size and position of the unit and type lists. */
	calc_construction_rects();
	LSize(unitlistrect.right - unitlistrect.left - sbarwid,
		  unitlistrect.bottom - unitlistrect.top,
		  constructionunitlist);
	/* Move the type list (is this the approved way to do it?) */
	(*constructiontypelist)->rView.left = typelistrect.left;
	LSize(typelistrect.right - typelistrect.left - sbarwid,
		  typelistrect.bottom - typelistrect.top,
		  constructiontypelist);
	/* This will force a full redraw at the next update. */
	InvalRect(&constructionwin->portRect);
}					

/* Zooming "rightsizes" the window. */

zoom_construction()
{
}

/* The side closeup window. */

create_side_closeup(side2)
Side *side2;
{
	SideCloseup *sidecloseup = (SideCloseup *) xmalloc(sizeof(SideCloseup));
	Rect vscrollrect, hscrollrect;

	DGprintf("Creating a side closeup\n");
	sidecloseup->side = side2;
	sidecloseup->dialog = GetNewDialog(dSideCloseup, nil, (DialogPtr) -1L);
	sidecloseup->next = sidecloseuplist;
	sidecloseuplist = sidecloseup;
	ShowWindow(sidecloseup->dialog);
	add_window_menu_item("Side Closeup", sidecloseup->dialog);
}


SideCloseup *
side_closeup_from_window(window)
WindowPtr window;
{
	SideCloseup *sidecloseup;
	
	for_all_side_closeups(sidecloseup) {
		if (sidecloseup->dialog == window) return sidecloseup;
	}
	return NULL;
}

draw_side_closeup(sidecloseup)
SideCloseup *sidecloseup;
{
	if (!active_display(dside) || sidecloseup == NULL) return;
	DrawDialog(sidecloseup->dialog);
}

hit_in_side_closeup(dialog, itemhit, evt)
DialogPtr dialog;
short itemhit;
EventRecord *evt;
{
	int closewhendone = FALSE;
	SideCloseup *sidecloseup = side_closeup_from_window(dialog);
	short itemtype;  Handle itemhandle;  Rect itemrect;
	
	if (sidecloseup == NULL) return FALSE;

	switch (itemhit) {
		case diSideCloseupMachineStrategy:
			GetDItem(dialog, diSideCloseupMachineStrategy, &itemtype, &itemhandle, &itemrect);
			SetCtlValue((ControlHandle) itemhandle, !GetCtlValue((ControlHandle) itemhandle));
			break;
		case diSideCloseupAutofinish:
			GetDItem(dialog, diSideCloseupAutofinish, &itemtype, &itemhandle, &itemrect);
			SetCtlValue((ControlHandle) itemhandle, !GetCtlValue((ControlHandle) itemhandle));
			break;
		case diSideCloseupOK:
			closewhendone = TRUE;
		case diSideCloseupApply:
			/* apply all the bits to the side. */
			GetDItem(dialog, diSideCloseupMachineStrategy, &itemtype, &itemhandle, &itemrect);
			dside->usemachinestrategy = GetCtlValue((ControlHandle) itemhandle);
			GetDItem(dialog, diSideCloseupAutofinish, &itemtype, &itemhandle, &itemrect);
			dside->autofinish = GetCtlValue((ControlHandle) itemhandle);
			break;
		case diSideCloseupCancel:
			closewhendone = TRUE;
			break;
	}
	if (closewhendone) {
		HideWindow(sidecloseup->dialog);
	}
	return TRUE;
}

char *
get_string_from_item(itemhandle)
Handle itemhandle;
{
	char tmpbuf[BUFSIZE];
	Str255 tmpstr;
	
	GetIText(itemhandle, tmpstr);
	/* This is basically p2c. */
 	strncpy(tmpbuf, tmpstr+1, tmpstr[0]);
	/* Make sure the string is terminated properly. */
	tmpbuf[tmpstr[0]] = '\0';
	return copy_string(tmpbuf);
}

side_rename_dialog(side)
Side *side;
{
	short done = FALSE, changed = TRUE, ditem;
	Str255 tmpstr;
	DialogPtr win;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	win = GetNewDialog(dSideRename, NULL, (DialogPtr) -1L);
	while (!done) {
		if (changed) {
			/* Seed the items with the current side names. */
			GetDItem(win, diSideRenameName, &itemtype, &itemhandle, &itemrect);
			c2p((side->name ? side->name : ""), tmpstr);
			SetIText(itemhandle, tmpstr);
			GetDItem(win, diSideRenameFullName, &itemtype, &itemhandle, &itemrect);
			c2p((side->longname ? side->longname : ""), tmpstr);
			SetIText(itemhandle, tmpstr);
			GetDItem(win, diSideRenameAcronym, &itemtype, &itemhandle, &itemrect);
			c2p((side->shortname ? side->shortname : ""), tmpstr);
			SetIText(itemhandle, tmpstr);
			GetDItem(win, diSideRenameNoun, &itemtype, &itemhandle, &itemrect);
			c2p((side->noun ? side->noun : ""), tmpstr);
			SetIText(itemhandle, tmpstr);
			GetDItem(win, diSideRenamePluralNoun, &itemtype, &itemhandle, &itemrect);
			c2p((side->pluralnoun ? side->pluralnoun : ""), tmpstr);
			SetIText(itemhandle, tmpstr);
			GetDItem(win, diSideRenameAdjective, &itemtype, &itemhandle, &itemrect);
			c2p((side->adjective ? side->adjective : ""), tmpstr);
			SetIText(itemhandle, tmpstr);
			GetDItem(win, diSideRenameEmblemName, &itemtype, &itemhandle, &itemrect);
			c2p((side->emblemname ? side->emblemname : ""), tmpstr);
			SetIText(itemhandle, tmpstr);
			GetDItem(win, diSideRenameColorScheme, &itemtype, &itemhandle, &itemrect);
			c2p((side->colorscheme ? side->colorscheme : ""), tmpstr);
			SetIText(itemhandle, tmpstr);
			ShowWindow(win);
			changed = FALSE;
		}
		draw_default_button(win, diSideRenameOK);
		ModalDialog(NULL, &ditem);
		switch (ditem) {
			case diSideRenameOK:
				/* Actually change the side's slots. */
				GetDItem(win, diSideRenameName, &itemtype, &itemhandle, &itemrect);
				set_side_name(dside, dside, get_string_from_item(itemhandle));
				GetDItem(win, diSideRenameFullName, &itemtype, &itemhandle, &itemrect);
				side->longname = get_string_from_item(itemhandle);
				GetDItem(win, diSideRenameAcronym, &itemtype, &itemhandle, &itemrect);
				side->shortname = get_string_from_item(itemhandle);
				GetDItem(win, diSideRenameNoun, &itemtype, &itemhandle, &itemrect);
				side->noun = get_string_from_item(itemhandle);
				GetDItem(win, diSideRenamePluralNoun, &itemtype, &itemhandle, &itemrect);
				side->pluralnoun = get_string_from_item(itemhandle);
				GetDItem(win, diSideRenameAdjective, &itemtype, &itemhandle, &itemrect);
				side->adjective = get_string_from_item(itemhandle);
				GetDItem(win, diSideRenameEmblemName, &itemtype, &itemhandle, &itemrect);
				side->emblemname = get_string_from_item(itemhandle);
				GetDItem(win, diSideRenameColorScheme, &itemtype, &itemhandle, &itemrect);
				side->colorscheme = get_string_from_item(itemhandle);
				/* Tweak the side menu. */
				update_side_menu(dside);
				/* Force redisplay of everything that might use any side names. */
				force_overall_update();
				/* Fall into next case. */
			case diSideRenameCancel:
				done = TRUE;
				break;
			case diSideRenameRandom:
				side->name = NULL;
				side->noun = NULL;
				/* always need to clear this cache before renaming... */
				side->pluralnoun = NULL;
				side->adjective = NULL;
				make_up_side_name(side);
				init_emblem_images(side);
				changed = TRUE;
				break;
		}
	}
	DisposDialog(win);
}

/* Generic renaming, applies to both units and features. */

unit_rename_dialog(unit)
Unit *unit;
{
	short done = FALSE, ditem;
	char *newname;
	char *namer = unit_namer(unit);
	Str255 tmpstr;
	DialogPtr win;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	if (unit == NULL) return;
	win = GetNewDialog(dRename, NULL, (DialogPtr) -1L);
	/* Seed the text item with the original name. */
	newname = unit->name;
	if (newname == NULL) newname = "";
	GetDItem(win, diRenameName, &itemtype, &itemhandle, &itemrect);
	c2p(newname, tmpstr);
	SetIText(itemhandle, tmpstr);
	/* Gray out the random renaming button if no namers available. */
	GetDItem(win, diRenameRandom, &itemtype, &itemhandle, &itemrect);
	HiliteControl((ControlHandle) itemhandle, ((namer != NULL) ? 0 : 255));
	ShowWindow(win);
	while (!done) {
		draw_default_button(win, diRenameOK);
		ModalDialog(NULL, &ditem);
		switch (ditem) {
			case diRenameOK:
				GetDItem(win, diRenameName, &itemtype, &itemhandle, &itemrect);
				set_unit_name(dside, unit, get_string_from_item(itemhandle));
				/* Fall into next case. */
			case diRenameCancel:
				done = TRUE;
				break;
			case diRenameRandom:
				newname = propose_unit_name(unit);
				GetDItem(win, diRenameName, &itemtype, &itemhandle, &itemrect);
				c2p(newname, tmpstr);
				SetIText(itemhandle, tmpstr);
				break;
		}
	}
	DisposDialog(win);
}

/* Unit closeups. */

UnitCloseup *
find_unit_closeup(unit)
Unit *unit;
{
	UnitCloseup *unitcloseup;

	for_all_unit_closeups(unitcloseup) {
		if (unitcloseup->unit == unit
			&& unitcloseup->window
			&& ((WindowPeek) unitcloseup->window)->visible)
		  return unitcloseup;
	}
	return NULL;
}

create_unit_closeup(unit)
Unit *unit;
{
	int u, w, h, v;
	Str255 tmpstr;
	WindowPtr win;
	UnitCloseup *unitcloseup = (UnitCloseup *) xmalloc(sizeof(UnitCloseup));
	short itemtype;  Handle itemhandle;  Rect itemrect;

	if (!active_display(dside) || unit == NULL) return;
	DGprintf("Creating a closeup of %s\n", unit_desig(unit));
	u = unit->type;
	unitcloseup->unit = unit;
	if (hasColorQD) {
		win = GetNewCWindow(wUnitCloseup, nil, (WindowPtr) -1L);
	} else {
		win = GetNewWindow(wUnitCloseup, nil, (WindowPtr) -1L);
	}
	unitcloseup->window = win;
	stagger_window(unitcloseup->window, &lastunitcloseuph, &lastunitcloseupv);
	preferred_closeup_size(u, &w, &h);
	SizeWindow(win, w, h, 1);
	sprintf(spbuf, "%s #%d Closeup", u_type_name(u), unit->id);
	add_window_menu_item(spbuf, win);
	unitcloseup->next = unitcloseuplist;
	unitcloseuplist = unitcloseup;
	/* We're now ready to show this closeup to the world. */
	ShowWindow(win);
}

preferred_closeup_size(u, widp, hgtp)
int u, *widp, *hgtp;
{
	int wid = 200, hgt = 100;

	/* (should base on only the mtypes that the unit can hold) */
	if (nummtypes > 0) hgt += nummtypes * 20;
	if (u_acp(u) > 0) hgt += 120;
	*widp = wid;  *hgtp = hgt;
}

draw_unit_closeup(unitcloseup)
UnitCloseup *unitcloseup;
{
	int u, m, sx = 4, sy;
	char tmpbuf[BUFSIZE];
	Str255 tmpstr;
	Rect tmprect;
	GrafPtr oldport;
	WindowPtr win = unitcloseup->window;
	Unit *unit = unitcloseup->unit;

	if (!active_display(dside)) return;
	if (!in_play(unit) || !side_controls_unit(dside, unit)) {
		/* If the unit is no longer alive and ours, shut down the window. */
		remove_window_menu_item(win);
		destroy_unit_closeup(unitcloseup);
		HideWindow(win);
		return;
	}
	GetPort(&oldport);
	SetPort(win);
	EraseRect(&win->portRect);
	u = unit->type;
	/* Draw the unit's image. */
	SetRect(&tmprect, sx, sx, sx + 32, sx + 32); 
	EraseRect(&tmprect);
	draw_unit_image(win, tmprect.left, tmprect.top,
					tmprect.right - tmprect.left, tmprect.bottom - tmprect.top,
					u, side_number(unit->side), !completed(unit));
	/* Draw the unit's name. */
	name_or_number(unit, tmpbuf);
	MoveTo(45, 21);
	DrawText(tmpbuf, 0, strlen(tmpbuf));
	if (Debug || DebugG || DebugM) {
		sprintf(tmpbuf, " %d ", unit->id);
		MoveTo(170, 15);
		DrawText(tmpbuf, 0, strlen(tmpbuf));
	}
	side_and_type_name(tmpbuf, dside, u, unit->side);
	MoveTo(45, 41);
	DrawText(tmpbuf, 0, strlen(tmpbuf));
	if (unit->z != 0) {
		sprintf(tmpbuf, "; alt %d", unit->z);
	} else {
		strcpy(tmpbuf, "");
	}
	sprintf(spbuf, "at %d,%d%s", unit->x, unit->y, tmpbuf);
	if (unit->transport != NULL) {
		sprintf(tmpbuf, "In %s (%s)", short_unit_handle(unit->transport), spbuf);
	} else {
		strcpy(tmpbuf, spbuf);
	}
	sy = 61;
	MoveTo(sx, sy);
	DrawText(tmpbuf, 0, strlen(tmpbuf));
	hp_desc(tmpbuf, unit);
	sy += 20;
	MoveTo(sx, sy);
	DrawText(tmpbuf, 0, strlen(tmpbuf));
	acp_desc(tmpbuf, unit);
	sy += 20;
	MoveTo(sx, sy);
	DrawText(tmpbuf, 0, strlen(tmpbuf));
	/* Display the unit's supplies. */
	for_all_material_types(m) {
		strcpy(tmpbuf, m_type_name(m));
		sy += 20;
		MoveTo(sx, sy);
		DrawText(tmpbuf, 0, strlen(tmpbuf));
		sprintf(tmpbuf, "%d", unit->supply[m]);
		MoveTo(sx + 80, sy);
		DrawText(tmpbuf, 0, strlen(tmpbuf));
	}
	if (unit->plan) {
		Task *task;
		extern char *plantypenames[];
		Plan *plan = unit->plan;
		
		sprintf(tmpbuf, "Plan: %s", plantypenames[plan->type]);
    	if (plan->waitingfortasks) strcat(tmpbuf, " [waiting]");
    	if (plan->reserve) strcat(tmpbuf, " [reserve]");
    	if (plan->asleep) strcat(tmpbuf, " [asleep]");
    	if (plan->aicontrol) strcat(tmpbuf, " [delegated]");
		sy += 20;
		MoveTo(sx, sy);
		DrawText(tmpbuf, 0, strlen(tmpbuf));
    	if (plan->maingoal) {
    		/* (should have a "goal_desc" routine) */
    		strcpy(tmpbuf, goal_desig(plan->maingoal));
			sy += 20;			
			MoveTo(sx, sy);
			DrawText(tmpbuf, 0, strlen(tmpbuf));
    	}
		if (plan->tasks) {
			for (task = plan->tasks; task != NULL; task = task->next) {
				task_desc(tmpbuf, task);
				sy += 20;			
				MoveTo(sx, sy);
				DrawText(tmpbuf, 0, strlen(tmpbuf));
			}
		}
	}
	SetPort(oldport);
}


UnitCloseup *
unit_closeup_from_window(win)
WindowPtr win;
{
	UnitCloseup *unitcloseup;
	
	for_all_unit_closeups(unitcloseup) {
		if (unitcloseup->window == win) return unitcloseup;
	}
	return NULL;
}

do_mouse_down_unit_closeup(unitcloseup, mouse, mods)
UnitCloseup *unitcloseup;
Point mouse;
int mods;
{
	ControlHandle control;
	short part, value;
	WindowPtr window = unitcloseup->window;

	part = FindControl(mouse, window, &control);
	if (0 /* some control */) {
	} else {
		/* This just forces a redraw of the window - kind of crude. */
		update_unit_display(dside, unitcloseup->unit, TRUE);
		return TRUE;
	}
}

destroy_unit_closeup(unitcloseup)
UnitCloseup *unitcloseup;
{
	UnitCloseup *unitcloseup2;
	
	if (unitcloseuplist == unitcloseup) {
		unitcloseuplist = unitcloseup->next;
	} else {
		for_all_unit_closeups(unitcloseup2) {
			if (unitcloseup2->next == unitcloseup) {
				unitcloseup2->next = unitcloseup->next;
			}
		}
	}
	/* (should destroy substructs) */
	free(unitcloseup);
}

/* History window. */

HistEvent *
get_nth_history_line(n)
int n;
{
	int i = 1;
	HistEvent *hevt;
	
	for (hevt = history->next; hevt != history; hevt = hevt->next) {
		if (1 /* passes filter */) {
			if (i == n || (i - 1) == n) return hevt;
			if (hevt->startdate != hevt->prev->startdate) ++i;
			++i;
		}
	}
	/* Return the last event. */
	return history->prev;
}

create_history_window()
{
	Rect vscrollrect;

	historywin = GetNewWindow(wHistory, NULL, (WindowPtr) -1L);
	histcontents = (HistEvent **) xmalloc(100 * sizeof(HistEvent *));
	vscrollrect = historywin->portRect;
	vscrollrect.top -= 1;
	vscrollrect.bottom -= sbarwid - 1;
	vscrollrect.left = vscrollrect.right - sbarwid;
	vscrollrect.right += 1;
	histvscrollbar = NewControl(historywin, &vscrollrect, "\p", TRUE,
			 					0, 0, 100, scrollBarProc, 0L);
	/* Start off with the most recent event.  This will be adjusted by scrollbar
	   setting if the window is > 1 line long. */
	firstvisevt = history->prev;
	update_total_hist_lines();
	set_history_scrollbar();
}

calc_history_layout()
{
	update_total_hist_lines();
	set_history_scrollbar();
}

update_total_hist_lines()
{
	int i = 1;
	HistEvent *hevt;
	
	totalhistlines = 0;
	for (hevt = history->next; hevt != history; hevt = hevt->next) {
		if (1 /* passes filter */) {
			if (hevt->startdate != hevt->prev->startdate) ++totalhistlines;
			++totalhistlines;
		}
	}
}

set_history_scrollbar()
{
	int hgt, i, oldmax;

	hgt = historywin->portRect.bottom - historywin->portRect.top;
	numvishistlines = (hgt - sbarwid) / histlinespacing;
	oldmax = GetCtlMax(histvscrollbar);
	SetCtlMax(histvscrollbar, max(0, totalhistlines - numvishistlines));
	HiliteControl(histvscrollbar, (numvishistlines < totalhistlines ? 0 : 255));
	if (GetCtlValue(histvscrollbar) == oldmax) {
		SetCtlValue(histvscrollbar, GetCtlMax(histvscrollbar));
		if (GetCtlValue(histvscrollbar) == 0) {
			firstvisevt = history->next;
		} else {
			firstvisevt = get_nth_history_line(GetCtlValue(histvscrollbar));
		}
	}
}

draw_history()
{
	int i;
    HistEvent *hevt;

	if (!active_display(dside) || historywin == nil) return;

	numhistcontents = 0;
	/* Always show a date on the first line of the window. */
	histcontents[numhistcontents++] = NULL;
	histcontents[numhistcontents++] = firstvisevt;
	for (hevt = firstvisevt->next; hevt != history; hevt = hevt->next) {
		if (1 /* passes filter */) {
			if (hevt->startdate != hevt->prev->startdate) {
				histcontents[numhistcontents++] = NULL;
			}
			histcontents[numhistcontents++] = hevt;
			if (numhistcontents >= numvishistlines) break;
		}
	}
	for (i = 0; i < numvishistlines; ++i) {
		if (histcontents[i] != NULL) {
			draw_historical_event(histcontents[i], i);
		} else {
			draw_historical_date(histcontents[i+1], i);
		}
	}
}

draw_historical_event(hevt, y)
HistEvent *hevt;
int y;
{
	int numchars;
	char hevtbuf[500];

	if (hevt == NULL) return;
	MoveTo(20, histlinespacing * y + 10);
	historical_event_desc(hevt, hevtbuf);
	/* (should clip to drawing only visible chars) */
	numchars = strlen(hevtbuf);
	DrawText(hevtbuf, 0, numchars);
}

draw_historical_date(hevt, y)
HistEvent *hevt;
int y;
{
	int numchars;
	char *datestr, hdatebuf[100];

	if (hevt == NULL) return;
	MoveTo(2, histlinespacing * y + 10);
	/* (should be relative) */
	datestr = absolute_date_string(hevt->startdate);
	if (hevt->startdate == hevt->prev->startdate) {
		sprintf(hdatebuf, "(%s)", datestr);
	} else {
		strcpy(hdatebuf, datestr);
	}
	/* (should clip to drawing only visible chars) */
	numchars = strlen(hdatebuf);
	DrawText(hdatebuf, 0, numchars);
}

pascal void
history_scroll_proc(control, code)
ControlHandle control;
short code;
{
	int curvalue, maxvalue, pagesize, jump;

	curvalue = GetCtlValue(control);
	maxvalue = GetCtlMax(control);
	pagesize = numvishistlines;
	switch (code) {
		case inPageDown:
			jump = pagesize - 1;
			break;
		case inDownButton:
			jump = 1;
			break;
		case inPageUp:
			jump = - (pagesize - 1);
			break;
		case inUpButton:
			jump = -1;
			break;
		default:
			jump = 0;
			break;
	}
	curvalue = max(min(curvalue + jump, maxvalue), 0);
	SetCtlValue(control, curvalue);
}

do_mouse_down_history(mouse, mods)
Point mouse;
int mods;
{
	ControlHandle control;
	short part, value;

	part = FindControl(mouse, historywin, &control);
	if (control == histvscrollbar) {
		switch (part) {
			case inThumb:
				part = TrackControl(control, mouse, NULL);
				firstvisevt = get_nth_history_line(GetCtlValue(control));
				force_update(historywin);
				break;
			default:
				part = TrackControl(control, mouse, (ProcPtr) history_scroll_proc);
				firstvisevt = get_nth_history_line(GetCtlValue(control));
				force_update(historywin);
				break;
		}
	} else {
		/* anything to do here? */
	}
}

/* Grow/shrink the history window to the given size. */

grow_history(h, v)
short h, v;
{
	Rect tmprect;

	EraseRect(&historywin->portRect);
	SizeWindow(historywin, h, v, 1);
	MoveControl(histvscrollbar, h - sbarwid, 0);
	SizeControl(histvscrollbar, sbarwid + 1, v - sbarwid + 1);
	set_history_scrollbar();
	/* This will force a full redraw at the next update. */
	InvalRect(&historywin->portRect);
}					

zoom_history(part)
int part;
{
	int titleh, h, v, vislinesavail;
	Rect winrect, zoomrect, gdrect;
	GDHandle gd, zoomgd;

	EraseRect(&historywin->portRect);
	if (part == inZoomOut) {
		if (!hasColorQD) {
			/* If no Color QD, then there is only one screen. */
			zoomrect = screenBits.bounds;
			InsetRect(&zoomrect, 4, 4);
		} else {
			for (gd = GetDeviceList(); gd != nil; gd = GetNextDevice(gd)) {
				if (TestDeviceAttribute(gd, screenDevice)) {
					if (TestDeviceAttribute(gd, screenActive)) {
						zoomgd = gd;
						break;
					}
				}
			}
			zoomrect = (*zoomgd)->gdRect;
			if (zoomgd == GetMainDevice()) {
				zoomrect.top += GetMBarHeight();
			}
			titleh = 20; /* (should calc) */
			zoomrect.top += titleh;
			InsetRect(&zoomrect, 3, 3);
			vislinesavail = (zoomrect.bottom - zoomrect.top - sbarwid) / histlinespacing;
			if (vislinesavail > numhistcontents) {
				zoomrect.bottom = zoomrect.top + numhistcontents * histlinespacing + sbarwid;
			}
		}
		(*((WStateDataHandle) ((WindowPeek) historywin)->dataHandle))->stdState = zoomrect;
	}
	ZoomWindow(historywin, part, (historywin == FrontWindow()));
	h = window_width(historywin);  v = window_height(historywin);
	MoveControl(histvscrollbar, h - sbarwid, 0);
	SizeControl(histvscrollbar, sbarwid + 1, v - sbarwid + 1);
	set_history_scrollbar();
	/* This will force a full redraw at the next update. */
	InvalRect(&historywin->portRect);
}

/* Help window. */

/* This is the top-level access to bring up the help window, can be called
   anywhere, anytime. */

help_dialog()
{
	if (helpwin == nil) {
		create_help_window();
	}
	ShowWindow(helpwin);
	SelectWindow(helpwin);
}

describe_menus(arg, key, buf)
int arg;
char *key, *buf;
{
	strcat(buf, "File\n");
	strcat(buf, "  The usual file operations.\n");
	strcat(buf, "\n");
	strcat(buf, "Edit\n");
	strcat(buf, "  Select All selects all of your units at once.\n");
	strcat(buf, "\n");
	strcat(buf, "Find\n");
	strcat(buf, "  Find Selected goes to the next map back and looks at the unit selected or viewed in the front window.\n");
	strcat(buf, "\n");
	strcat(buf, "Play\n");
	strcat(buf, "  This is the main menu for unit actions.");
	strcat(buf, "  If an action is grayed out, then none of the selected units can do it.\n");
	strcat(buf, "  Closeup shows details of the selected units.\n");
	strcat(buf, "  Build brings up the construction dialog.\n");
	strcat(buf, "Side\n");
	strcat(buf, "  This menu is for things that affect your whole side.\n");
	strcat(buf, "Windows\n");
	strcat(buf, "  This menu does window control.\n");
	strcat(buf, "View (Map)\n");
	strcat(buf, "  Items toggle various display elements in the front map window.\n");
	strcat(buf, "\n");
	strcat(buf, "View (List)\n");
	strcat(buf, "\n");
}

describe_mouse(arg, key, buf)
int arg;
char *key, *buf;
{
	strcat(buf, "In move-on-click mode:\n");
	strcat(buf, "  The next unit that can do anything will be selected automatically.\n");
	strcat(buf, "  Click once on a destination to move the selected unit there.\n");
	strcat(buf, "In normal mode:\n");
	strcat(buf, "  Click once on a unit to select it.  ");
	strcat(buf, "  Drag to destination to move it.  ");
	strcat(buf, "  Shift-click to select additional units, ");
	strcat(buf, "  Drag out rectangle to select all units inside.\n");
}

describe_shortcuts(arg, key, buf)
int arg;
char *key, *buf;
{
	strcat(buf, "Command-click moves all selected units to the clicked-on location.");
}

describe_help(arg, key, buf)
int arg;
char *key, *buf;
{
	strcat(buf, "Click on the obvious buttons.");
	strcat(buf, "\n");
	strcat(buf, "To select a topic in the topics list, click on it.");
}

create_help_window()
{
	int h, v;
	Rect helptopicrect, destrect, viewrect, vscrollrect;

	/* Create the window, color if possible, since images may be in color. */
	if (hasColorQD) {	
		helpwin = GetNewCWindow(wHelp, NULL, (WindowPtr) -1L);
	} else {
		helpwin = GetNewWindow(wHelp, NULL, (WindowPtr) -1L);
	}
	topicsbutton = GetNewControl(cTopicsButton, helpwin);
	helpbutton = GetNewControl(cHelpButton, helpwin);
	prevbutton = GetNewControl(cPrevButton, helpwin);
	nextbutton = GetNewControl(cNextButton, helpwin);
	backbutton = GetNewControl(cBackButton, helpwin);
	SetPort(helpwin);
	/* All text will be in Times. */
	/* (should these be choosable?) */
	TextFont(times);
	/* Set up the topic title. */
	TextSize(18);
	SetRect(&helptopicrect, 45, 45, 305, 75); 
	helptopic = TENew(&helptopicrect, &helptopicrect);
	/* Set up the help text proper. */
	TextSize(14);
	h = window_width(helpwin);  v = window_height(helpwin);
	SetRect(&viewrect, 5, 75, h - sbarwid, v - sbarwid); 
	destrect = viewrect;
	helptext = TENew(&destrect, &destrect);
	/* Set up a vertical scrollbar. */
	vscrollrect = helpwin->portRect;
	vscrollrect.top = 75;
	vscrollrect.bottom -= sbarwid - 1;
	vscrollrect.left = vscrollrect.right - sbarwid;
	vscrollrect.right += 1;
	helpvscrollbar =
		NewControl(helpwin, &vscrollrect, "\p", TRUE, 0, 0, 0, scrollBarProc, 0L);
	HiliteControl(helpvscrollbar, 0);
	/* Add the Mac-specific help nodes. */
	add_help_node("menus", describe_menus, 0, firsthelpnode);
	add_help_node("shortcuts", describe_shortcuts, 0, firsthelpnode);
	add_help_node("mouse input", describe_mouse, 0, firsthelpnode);
	helphelpnode = add_help_node("help", describe_help, 0, firsthelpnode);
	curhelpnode = firsthelpnode;
	set_help_content(curhelpnode);
	if (nodestack == NULL) {
		nodestack = (HelpNode **) xmalloc(NODESTACKSIZE * sizeof(HelpNode *));
	}
	nodestackpos = 0;
	add_window_menu_item("Help", helpwin);
}

set_help_content(curnode)
HelpNode *curnode;
{
	int i;
	char *str;
	Str255 tmpstr;

	get_help_text(curnode);
	/* Set the displayed topic title. */
	TESetSelect(0, 32767, helptopic);
	TECut(helptopic);
	/* Copy in the new help topic text. */
	TESetText(curnode->key, strlen(curnode->key), helptopic);
	/* Set the displayed text. */
	str = curnode->text;
	/* Hack up newlines so that TextEdit recognizes them. */
	for (i = 0; i < strlen(str); ++i) {
		if (str[i] == '\n') str[i] = '\r';
	}
	helpstring = str;
	/* Remove all the existing text. */
	TESetSelect(0, 32767, helptext);
	TECut(helptext);
	/* Copy in the new help text. */
	TESetText(helpstring, strlen(helpstring), helptext);
	(*helptext)->destRect = (*helptext)->viewRect;
	/* Update on the screen. */
	draw_help();
	adjust_help_scrollbar();
}

draw_help()
{
	Rect tmprect;
	GrafPtr oldport;

	GetPort(&oldport);
	SetPort(helpwin);
	SetRect(&tmprect, 5, 40, 5 + 32, 40 + 32);
	EraseRect(&tmprect);
	if (curhelpnode->nclass == utypenode && is_unit_type(curhelpnode->arg)) {
		draw_unit_image(helpwin, tmprect.left, tmprect.top, 32, 32,
						curhelpnode->arg, -1, 0);
	} else if (curhelpnode->nclass == ttypenode && is_terrain_type(curhelpnode->arg)) {
		draw_terrain_sample(tmprect, curhelpnode->arg); 
	}
	TextSize(18);
	TEUpdate(&(helpwin->portRect), helptopic);
	TextSize(14);
	TEUpdate(&(helpwin->portRect), helptext);
	SetPort(oldport);
	adjust_help_scrollbar();
}

adjust_help_scrollbar()
{
	int lines, newmax, value;

	lines = (*helptext)->nLines;
	newmax = lines - (((*helptext)->viewRect.bottom - (*helptext)->viewRect.top)
					 / (*helptext)->lineHeight);
	if (newmax < 0) newmax = 0;
	SetCtlMax(helpvscrollbar, newmax);
	value = ((*helptext)->viewRect.top - (*helptext)->destRect.top)
			/ (*helptext)->lineHeight;
	SetCtlValue(helpvscrollbar, value);
}

activate_help(activate)
int activate;
{
	HiliteControl(helpvscrollbar, (activate ? 0 : 255));
}

pascal void
help_vscroll_proc(control, code)
ControlHandle control;
short code;
{
	int curvalue, minvalue, maxvalue, pagesize, dir, jump;

	curvalue = GetCtlValue(control);
	minvalue = GetCtlMin(control);
	maxvalue = GetCtlMax(control);
	pagesize = ((*helptext)->viewRect.bottom - (*helptext)->viewRect.top) /
				(*helptext)->lineHeight;
	switch (code) {
		case inPageDown:
			jump = pagesize;
			break;
		case inDownButton:
			jump = 1;
			break;
		case inPageUp:
			jump = - pagesize;
			break;
		case inUpButton:
			jump = -1;
			break;
		default:
			jump = 0;
			break;
	}
	if (curvalue + jump > maxvalue) jump = 0;
	if (curvalue + jump < minvalue) jump = 0;
	curvalue = max(min(curvalue + jump, maxvalue), minvalue);
	SetCtlValue(control, curvalue);
	if (jump != 0) TEScroll(0, - jump * (*helptext)->lineHeight, helptext);
}

/* Respond to an event occurring in the help window. */

do_mouse_down_help(mouse, mods)
Point mouse;
int mods;
{
	ControlHandle control;
	short part, value;
	int u, i;
	Point pt;
	Unit *unit;
	HelpNode *prevhelpnode = curhelpnode, *helpnode;

	part = FindControl(mouse, helpwin, &control);
	if (control == topicsbutton) {
		curhelpnode = firsthelpnode;
	} else if (control == helpbutton) {
		curhelpnode = helphelpnode;
	} else if (control == prevbutton) {
		curhelpnode = curhelpnode->prev;
	} else if (control == nextbutton) {
		curhelpnode = curhelpnode->next;
	} else if (control == backbutton) {
		if (nodestackpos > 0) {
			curhelpnode = nodestack[--nodestackpos];
		}
	} else if (control == helpvscrollbar) {
		if (part != 0) {
			switch (part) {
				case inPageDown:
				case inDownButton:
				case inPageUp:
				case inUpButton:
					value = TrackControl(control, mouse, (ProcPtr) help_vscroll_proc);
					break;
				case inThumb:
					value = GetCtlValue(control);
					if ((part = TrackControl(control, mouse, nil)) != 0) {
						value -= GetCtlValue(control);
						if (value != 0) {
							TEScroll(0, value * (*helptext)->lineHeight, helptext);
						}
					}
					break;
			}
		}
	} else if (PtInRect(mouse, &((*helptext)->viewRect))) {
		TEClick(mouse, 0, helptext);
		if (curhelpnode == firsthelpnode) {
			char strbuf[100], *cr1, *cr2;
			int selstart = (*helptext)->selStart, selend = (*helptext)->selEnd;
			CharsHandle chars = TEGetText(helptext);

			if (selstart == selend) {
				/* Manufacture a "selection" of the line clicked in. */
				for (cr1 = (*chars)+selstart; cr1 != (*chars); --cr1) if (*cr1 == '\r') break;
				cr2 = strchr((*chars)+selstart, '\r');
				selstart = (cr1 != NULL ? cr1 - (*chars) + 1 : 0);
				selend = (cr2 != NULL ? cr2 - (*chars) : 0);
				if (selstart > selend) selstart = selend;
			}
			if (selstart != selend) {
				strncpy(strbuf, (*chars)+selstart, selend - selstart);
				strbuf[selend - selstart] = '\0';
				helpnode = find_help_node(curhelpnode, strbuf);
				if (helpnode != NULL) {
					curhelpnode = helpnode;
				} else {
					SysBeep(20);
				}
			} else {
				SysBeep(20);
			}
		}
	}
	/* If we changed help nodes, get its contents and record on the node stack. */
	if (prevhelpnode != curhelpnode) {
		set_help_content(curhelpnode);
		if (control != backbutton) {
			if (nodestackpos >= NODESTACKSIZE) {
				for (i = 1; i < NODESTACKSIZE; ++i) nodestack[i - 1] = nodestack[i];
				nodestackpos = NODESTACKSIZE - 1;
			}
			nodestack[nodestackpos++] = prevhelpnode;
		}
	}
	return TRUE;
}

grow_help(h, v)
short h, v;
{
	EraseRect(&helpwin->portRect);
	SizeWindow(helpwin, h, v, 1);
	MoveControl(helpvscrollbar, h - sbarwid, 75);
	SizeControl(helpvscrollbar, sbarwid + 1, v - 75 - sbarwid + 1);
	(*helptext)->viewRect.right = h - sbarwid;
	(*helptext)->viewRect.bottom = v - sbarwid;
	(*helptext)->destRect.right = h - sbarwid;
	TECalText(helptext);
	InvalRect(&helpwin->portRect);
}					
