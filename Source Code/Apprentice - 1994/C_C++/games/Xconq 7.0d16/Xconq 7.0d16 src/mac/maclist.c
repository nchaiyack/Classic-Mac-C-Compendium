/* Copyright (c) 1992, 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Unit lists for the Mac interface. */

#include "conq.h"
#include "mac.h"

/* (should be adjusted for expected range of values sometimes) */

int curactcolw = 10;
int imagecolw = 20;
int namecolw = 100;
int typecolw = 70;
int sidecolw = 70;
int hpcolw = 40;
int acpcolw = 60;
int poscolw = 60;
int supcolw = 40;
int notecolw = 50;

int maxlistwidth;

int listtoph = 20;
int entryspacing;
int smallentryspacing = 18;
int largeentryspacing = 34;

int fixedfieldwidth;

int listnum = 1;

int lastlisth = -1, lastlistv = -1;

create_list()
{
	int i, numvisunits, h, v;
	List *list = (List *) xmalloc(sizeof(List));
	Rect hscrollrect, vscrollrect;

	maxlistwidth = 0;
	maxlistwidth += curactcolw;
	maxlistwidth += imagecolw;
	maxlistwidth += namecolw;
	fixedfieldwidth = maxlistwidth;
	maxlistwidth += typecolw;
	maxlistwidth += sidecolw;
	maxlistwidth += hpcolw;
	maxlistwidth += acpcolw;
	maxlistwidth += poscolw;
	maxlistwidth += nummtypes * supcolw;
	maxlistwidth += notecolw;

	DGprintf("Creating a list\n");
	list->sides == -1L;  /* lists every side and indeps too */
	for (i = 0; i < MAXSORTKEYS; ++i) {
		list->sortkeys[i] = bynothing;
	}
	list->sortkeys[0] = byside;
	list->mainsortmi = miViewBySide;
	list->listglimpsed = FALSE;
	init_list_contents(list);
	organize_list_contents(list);
	list->next = listlist;
	listlist = list;
	/* Make a window for the list, give it scrollbars. */
	list->window = GetNewWindow(wList, nil, (WindowPtr) -1L);
	stagger_window(list->window, &lastlisth, &lastlistv);
	ShowWindow(list->window);
	SetPort(list->window);
	hscrollrect = list->window->portRect;
	hscrollrect.top = hscrollrect.bottom - sbarwid;
	hscrollrect.bottom += 1;
	hscrollrect.left += (curactcolw + imagecolw + namecolw) + 1;
	hscrollrect.right -= sbarwid - 1;
	list->hscrollbar = NewControl(list->window, &hscrollrect, "\p", TRUE,
			 					  0, 0, 300, scrollBarProc, 0L);
	vscrollrect = list->window->portRect;
	vscrollrect.top -= 1;
	vscrollrect.bottom -= sbarwid - 1;
	vscrollrect.left = vscrollrect.right - sbarwid;
	vscrollrect.right += 1;
	numvisunits = (vscrollrect.bottom - vscrollrect.top) / smallentryspacing;
	list->vscrollbar = NewControl(list->window, &vscrollrect, "\p", TRUE,
			 					  0, 0, max(0, list->numunits - numvisunits), scrollBarProc, 0L);
	set_list_scrollbar(list);
	list->firstvisible = 0;
	sprintf(spbuf, "List %d", listnum++);
	add_window_menu_item(spbuf, list->window);
}

/* Make the list be empty. */

init_list_contents(list)
List *list;
{
	int i;

	list->contents = make_unit_vector(500);
	list->numunits = 0;
}

/* This takes the list and fills in the items it is to display. */

organize_list_contents(list)
List *list;
{
	Side *side2;
	Unit *unit;

	/* Build up the array of units for this list. */
	list->numunits = 0;
	clear_unit_vector(list->contents);
	/* We always see our own units. */
	for_all_side_units(dside, unit) {
		add_unit_to_list(list, unit);
	}
	for_all_sides(side2) {
		if (dside != side2) {
			for_all_side_units(side2, unit) {
				if (side_sees_image(dside, unit)) {
					add_unit_to_list(list, unit);
				}
			}
		}
	}
	for_all_side_units(indepside, unit) {
		if (side_sees_image(dside, unit)) {
			add_unit_to_list(list, unit);
		}
	}
	/* Now sort the list according to its keys. */
	sort_list_contents(list);
}

sort_list_contents(list)
List *list;
{
	int i;

	for (i = 0; i < MAXSORTKEYS; ++i) {
		tmpsortkeys[i] = list->sortkeys[i];
	}
	sort_unit_vector(list->contents);
}

add_unit_to_list(list, unit)
List *list;
Unit *unit;
{
	if (alive(unit)) {
		list->contents = add_unit_to_vector(list->contents, unit, FALSE);
		/* (should apply other inclusion criteria too?) */
		++list->numunits;
	}
}

set_list_scrollbar(list)
List *list;
{
	int numvisunits = (list->window->portRect.bottom - list->window->portRect.top - sbarwid) / smallentryspacing;
	/* Adjust scrollbars to fit the list size. */
	SetCtlMax(list->hscrollbar, maxlistwidth);
	SetCtlMax(list->vscrollbar, max(0, list->numunits - numvisunits));
}

List *
list_from_window(window)
WindowPtr window;
{
	List *list;
	
	if (dside == NULL) return NULL;
	for_all_lists(list) {
		if (list->window == window) return list;
	}
	return NULL;
}

draw_list(list)
List *list;
{
	WindowPtr listwin = list->window;
	Rect tmprect, cliprect;
	RgnHandle tmprgn;

	tmprgn = NewRgn();
	GetClip(tmprgn);
	tmprect = listwin->portRect;
	tmprect.right -= sbarwid;
	BackPat(QD(white));
	EraseRect(&tmprect);
	/* Set up clipping for the contents of the list. */
	cliprect = listwin->portRect;
	cliprect.right -= sbarwid;
	ClipRect(&cliprect);
	TextSize(9);
	draw_list_contents(list);
	SetClip(tmprgn);
	DisposeRgn(tmprgn);
}
	
draw_list_contents(list)
List *list;
{
	int line, numvisunits;
	Rect tmprect;

	tmprect = list->window->portRect;
	EraseRect(&tmprect);
	entryspacing = (list->largeicons ? largeentryspacing : smallentryspacing);
	/* Image is basically square, but add a bit of extra space on each side. */
	imagecolw = entryspacing + 2;
	/* Draw the selection and sorting as a sort of header. */
	draw_list_headings(list);
	/* Compute how many list elements are actually visible. */
	numvisunits = (tmprect.bottom - tmprect.top - listtoph - sbarwid) / entryspacing;
	numvisunits = min(numvisunits, list->numunits);
	list->lastvisible = list->firstvisible + numvisunits - 1;
	for (line = list->firstvisible; line <= list->lastvisible; ++line) {
		draw_unit_list_entry(list, line, FALSE);
	}
	HiliteControl(list->vscrollbar, ((numvisunits < list->numunits) ? 0 : 255));
	HiliteControl(list->hscrollbar, (((list->window->portRect.right - list->window->portRect.left - sbarwid) < maxlistwidth) ? 0 : 255));
	/* (should do for hscroll also) */
}

draw_list_headings(list)
List *list;
{
	int x = 0, m;
	Str255 tmpstr;
	Rect cliprect;

	cliprect = list->window->portRect;
	cliprect.right -= sbarwid;
	ClipRect(&cliprect);
	/* Draw a dividing line that crosses both fixed and scrolling fields. */
	MoveTo(0, listtoph);
	Line(list->window->portRect.right, 0);
	/* (should underline sort keys with varying line heaviness) */
	/* We have to do MoveTo everywhere because DrawString moves the pen. */
	x += curactcolw;
	MoveTo(x, 15);
	DrawString("\ps/L");
	x += imagecolw;
	MoveTo(x, 15);
	DrawString("\pName/Number");
	/* Shift left by horiz scroll. */
	x -= list->firstvisfield;
	cliprect = list->window->portRect;
	cliprect.right -= sbarwid;
	cliprect.left = fixedfieldwidth;
	ClipRect(&cliprect);
	x += namecolw;
	MoveTo(x, 15);
	DrawString("\pType");
	x += typecolw;
	MoveTo(x, 15);
	DrawString("\pSide");
	x += sidecolw;
	MoveTo(x, 15);
	DrawString("\pHp");
	x += hpcolw;
	MoveTo(x, 15);
	DrawString("\pAcp");
	x += acpcolw;
	MoveTo(x, 15);
	DrawString("\pLoc");
	x += poscolw;
	for_all_material_types(m) {
		MoveTo(x, 15);
		c2p(m_type_name(m), tmpstr);
		DrawString(tmpstr);
		x += supcolw;
	}
}

/* This draws a one-line entry for the given unit. */

draw_unit_list_entry(list, n, clearfirst)
List *list;
int n, clearfirst;
{
	int u, m, x, y = (n - list->firstvisible) * entryspacing + listtoph;
	int texty = y + 15;
	Side *side2;
	Rect entryrect, tmprect;
	Unit *unit = list->contents->units[n].unit;
	Rect cliprect;

	cliprect = list->window->portRect;
	cliprect.right -= sbarwid;
	ClipRect(&cliprect);

	SetRect(&entryrect, 0, y, list->window->portRect.right, y + entryspacing);
	if (clearfirst) {
		EraseRect(&entryrect);
	}
	if (unit == NULL || !alive(unit)) {
		/* We need to recalculate the list contents. */
		list->shouldreorg = TRUE;
		return;
	}
	u = unit->type;
	/* Draw whether the unit is awake or asleep. */
	if (unit->plan && completed(unit)) {
		SetRect(&tmprect, 0, y + entryspacing/2 - curactcolw/2,
				curactcolw, y + entryspacing/2 + curactcolw/2);
		InsetRect(&tmprect, 2, 2);
		/* (should draw the following analogously to map display) */
		if (unit->plan->asleep) {
			/* Leave rectangle blank. */
		} else if (unit->plan->reserve) {
			FillRect(&tmprect, QD(gray));
		} else {
			FillRect(&tmprect, QD(black));
		}
		FrameRect(&tmprect);
	}
	/* Draw an icon with side emblem for this unit. */
	draw_unit_image(list->window, curactcolw + 2, y + 1,
					(list->largeicons ? 32 : 16), (list->largeicons ? 32 : 16),
					u, side_number(unit->side), !completed(unit));
	/* Write the name or ordinal number. */
	name_or_number(unit, spbuf);
	spbuf[15] = '\0';  /* (should be clipping by pixels) */
	x = curactcolw + imagecolw;
	MoveTo(x, texty);
	DrawText(spbuf, 0, strlen(spbuf));
	/* Adjust according to the horizontal scroll. */
	x -= list->firstvisfield;
	cliprect = list->window->portRect;
	cliprect.right -= sbarwid;
	cliprect.left = fixedfieldwidth;
	ClipRect(&cliprect);
	/* Write the name of the unit's type. */
	x += namecolw;
	MoveTo(x, texty);
	sprintf(spbuf, "%s", u_type_name(u));
	spbuf[10] = '\0';
	DrawText(spbuf, 0, strlen(spbuf));
	/* Write the side of the unit. */
	x += typecolw;
	MoveTo(x, texty);
	side2 = unit->side;
	shortest_side_title(side2, spbuf);
	spbuf[15] = '\0'; /* truncate long side names */
	DrawText(spbuf, 0, strlen(spbuf));
	/* Draw the current hit points of the unit. */
	x += sidecolw;
	if (side_sees_unit(dside, unit)) {
		MoveTo(x, texty);
		hp_desc(spbuf, unit, FALSE);
		DrawText(spbuf, 0, strlen(spbuf));
	}
	x += hpcolw;
	if (side_sees_unit(dside, unit)) {
		MoveTo(x, texty);
		acp_desc(spbuf, unit, FALSE);
		if (strlen(spbuf) == 0) strcpy(spbuf, "-");
		DrawText(spbuf, 0, strlen(spbuf));
	}
	/* Draw the location. */
	x += acpcolw;
	MoveTo(x, texty);
	sprintf(spbuf, "%d,%d", unit->x, unit->y);
	if (unit->z != 0) {
		tprintf(spbuf, ",%d", unit->z);
	}
	DrawText(spbuf, 0, strlen(spbuf));
	/* Draw the state of all the supplies. */
	x += poscolw;
	if (side_sees_unit(dside, unit)) {
		for_all_material_types(m) {
			MoveTo(x, texty);
			sprintf(spbuf, "%d", unit->supply[m]);
			DrawText(spbuf, 0, strlen(spbuf));
			x += supcolw;
		}
	} else {
		x += nummtypes * supcolw; /* actually not necessary since no more cols */
	}
	/* Fix the clipping. */
	cliprect = list->window->portRect;
	cliprect.right -= sbarwid;
	ClipRect(&cliprect);
	/* Highlight this entry if it was selected. */
	if (list->contents->units[n].flag) {
		InvertRect(&entryrect);
	}
}

grow_list(list, w, h)
List *list;
int w, h;
{
	WindowPtr listwin = list->window;

	EraseRect(&listwin->portRect);
	SizeWindow(listwin, w, h, 1);
	adjust_list_decor(list);
	InvalRect(&listwin->portRect);
}

/* Zooming works like a list view in the Finder - it calculates a "perfect" size,
   showing as much as possible but with no wasted blank areas. */

zoom_list(list, part)
List *list;
short part;
{
	int maxh;
	WindowPtr listwin = list->window;

	if (part == inZoomOut) {
		entryspacing = (list->largeicons ? largeentryspacing : smallentryspacing);
		maxh = listtoph + list->numunits * entryspacing + sbarwid;
		set_standard_state(listwin, maxlistwidth, maxh);
	}
	EraseRect(&listwin->portRect);
	ZoomWindow(listwin, part, true);
	adjust_list_decor(list);
}

adjust_list_decor(list)
List *list;
{
	int w, h, spacing, lines;

	w = list->window->portRect.right - list->window->portRect.left;
	h = list->window->portRect.bottom - list->window->portRect.top;
	MoveControl(list->hscrollbar, (curactcolw+imagecolw+namecolw), h - sbarwid);
	SizeControl(list->hscrollbar, w - (curactcolw+imagecolw+namecolw) - sbarwid + 1, sbarwid + 1);
	MoveControl(list->vscrollbar, w - sbarwid, listtoph);
	SizeControl(list->vscrollbar, sbarwid + 1, h - listtoph - sbarwid + 1);
	spacing = (list->largeicons ? largeentryspacing : smallentryspacing);
	lines = (h - listtoph - sbarwid) / spacing;
	if (lines >= list->numunits) {
		list->firstvisible = 0;
	} else if ((list->firstvisible + lines) > list->numunits) {
		list->firstvisible = list->numunits - 1 - lines;
	}
	SetCtlValue(list->vscrollbar, list->firstvisible);
}

List *curlist = NULL;

pascal void
list_vscroll_proc(control, code)
ControlHandle control;
short code;
{
	int curvalue, minvalue, maxvalue, pagesize, dir, jump;

	if (curlist == NULL) return;

	curvalue = GetCtlValue(control);
	minvalue = GetCtlMin(control);
	maxvalue = GetCtlMax(control);
	pagesize = curlist->lastvisible - curlist->firstvisible + 1;
	switch (code) {
		case inPageDown:
			jump = max(1, pagesize - 1);
			break;
		case inDownButton:
			jump = 1;
			break;
		case inPageUp:
			jump = min(-1, - (pagesize - 1));
			break;
		case inUpButton:
			jump = -1;
			break;
		default:
			jump = 0;
			break;
	}
	curvalue = max(min(curvalue + jump, maxvalue), minvalue);
	curlist->firstvisible = curvalue;
	curlist->lastvisible = min(curlist->numunits, curlist->firstvisible + pagesize) - 1;
	SetCtlValue(control, curvalue);
	curlist = NULL;
}

pascal void
list_hscroll_proc(control, code)
ControlHandle control;
short code;
{
	int curvalue, minvalue, maxvalue, pagesize, dir, jump;

	if (curlist == NULL) return;

	curvalue = GetCtlValue(control);
	minvalue = GetCtlMin(control);
	maxvalue = GetCtlMax(control);
	pagesize = curlist->lastvisfield - curlist->firstvisfield;
	switch (code) {
		case inPageDown:
			jump = pagesize - 10;
			break;
		case inDownButton:
			jump = 10;
			break;
		case inPageUp:
			jump = - (pagesize - 10);
			break;
		case inUpButton:
			jump = -10;
			break;
		default:
			jump = 0;
			break;
	}
	curvalue = max(min(curvalue + jump, maxvalue), minvalue);
	curlist->firstvisfield = curvalue;
	curlist->lastvisfield = curlist->firstvisfield + pagesize;
	SetCtlValue(control, curvalue);
	curlist = NULL;
}

/* Handle a mouse down in the list.  Grafport already set, mouse coords are local. */

/* (mouse downs should select/deselect list elements) */

do_mouse_down_list(list, mouse, mods)
List *list;
Point mouse;
int mods;
{
	ControlHandle control;
	short part, value;
	int n, tmp;
	WindowPtr window = list->window;

	part = FindControl(mouse, window, &control);
	if (control == list->vscrollbar) {
		switch (part) {
			case inThumb:
				part = TrackControl(control, mouse, NULL);
				list->firstvisible = GetCtlValue(control);
				force_update(window);
				break;
			default:
				curlist = list;
				part = TrackControl(control, mouse, (ProcPtr) list_vscroll_proc);
				force_update(window);
				break;
		}
	} else if (control == list->hscrollbar) {
		switch (part) {
			case inThumb:
				part = TrackControl(control, mouse, NULL);
				list->firstvisfield = GetCtlValue(control);
				force_update(window);
				break;
			default:
				curlist = list;
				part = TrackControl(control, mouse, (ProcPtr) list_hscroll_proc);
				force_update(window);
				break;
		}
	} else {
		if (mouse.v < listtoph) {
			if (between(curactcolw, mouse.h, curactcolw + imagecolw)) {
				toggle_list_large_icons(list);
			}
			/* do others eventually */
		} else {
			/* Figure out the selected unit. */
			n = (mouse.v - listtoph) / entryspacing + list->firstvisible;
			tmp = list->contents->units[n].flag;
			clear_selections(list);
			list->contents->units[n].flag = !tmp;
			redraw_unit_list_entry(list, n);
		}
	}
}

set_list_sorting(list, newkey, mi)
List *list;
enum sortkeys newkey;
int mi;
{
	int i;
	
	if (newkey != list->sortkeys[0]) {
		/* Push all the existing sortkeys back - this way they'll can be
		   used as tiebreakers for the new sort key. */
		for (i = MAXSORTKEYS - 1; i > 0; --i) {
			list->sortkeys[i] = list->sortkeys[i - 1];
		}
		/* Add the new one onto the front. */
		list->sortkeys[0] = newkey;
		sort_list_contents(list);
		force_update(list->window);
		/* Record the menu item so it can get a checkmark during menu adjust. */
		list->mainsortmi = mi;
	}
}

toggle_list_large_icons(list)
List *list;
{
	list->largeicons = !list->largeicons;
	force_update(list->window);
}

update_unit_in_lists(unit)
Unit *unit;
{
	int line;
	List *list;

	for_all_lists(list) {
		if ((line = unit_position_in_list(list, unit)) >= 0) {
			if (between(list->firstvisible, line, list->lastvisible)) {
				redraw_unit_list_entry(list, line);
			}
		} else {
/*			add_unit_to_list(list, unit); */
			organize_list_contents(list);
			force_update(list->window);
		}
	}
}

unit_position_in_list(list, unit)
List *list;
Unit *unit;
{
	int i;
	
	for (i = 0; i < list->numunits; ++i) {
		if (unit == list->contents->units[i].unit) return i;
	}
	return (-1);
}

redraw_unit_list_entry(list, n)
List *list;
int n;
{
	WindowPtr listwin;
	Rect cliprect;
	GrafPtr oldport;
	RgnHandle tmprgn;

	if (!active_display(dside) || list == NULL) return;

	listwin = list->window;
 	GetPort(&oldport);
	SetPort(listwin);
	tmprgn = NewRgn();
	GetClip(tmprgn);
	/* Set up clipping for the contents of the list. */
	cliprect = listwin->portRect;
	cliprect.right -= sbarwid;
	ClipRect(&cliprect);
	TextSize(9);
	draw_unit_list_entry(list, n, TRUE);
	SetClip(tmprgn);
	DisposeRgn(tmprgn);
	SetPort(oldport);
}

clear_selections(list)
List *list;
{
	int i;
	
	for (i = 0; i < list->numunits; ++i) {
		if (list->contents->units[i].flag) {
			list->contents->units[i].flag = FALSE;
			redraw_unit_list_entry(list, i);
		}
	}
}

Unit *
selected_unit_in_list(list)
List *list;
{
	int i;
	
	for (i = 0; i < list->numunits; ++i) {
		if (list->contents->units[i].flag) return list->contents->units[i].unit;
	}
	return NULL;
}

/* This finds a good map to scroll over to look at a unit mentioned in the list. */

scroll_to_selected_unit_in_list(list)
List *list;
{
	Unit *unit;
	Map *map, *bestmap;
	
	if ((unit = selected_unit_in_list(list)) != NULL
	    && inside_area(unit->x, unit->y)) {
		/* Find the best map to scroll over to the unit. */
		bestmap = maplist;
		for_all_maps(map) {
			if (map->power > bestmap->power
			    || (map->power == bestmap->power
			        && in_middle(map, unit->x, unit->y)
				 	&& !in_middle(bestmap, unit->x, unit->y))) {
				bestmap = map;
			}
		}
		/* We found a map, now make it show the unit. */
		if (bestmap != NULL) {
			if (!in_middle(bestmap, unit->x, unit->y)) {
				scroll_to_unit(bestmap, unit);
			}
			SelectWindow(bestmap->window);
			adjust_menus(); 
		}
	}
}

activate_list(list, activate)
List *list;
int activate;
{
	Rect growRect;

	if (activate) {
		HiliteControl(list->vscrollbar, 0);
		HiliteControl(list->hscrollbar, 0);
#if 0
		/* the controls must be redrawn on activation: */
		(*(list->vscrollbar))->contrlVis = 255;
		(*(list->hscrollbar))->contrlVis = 255;
		InvalRect(&(*(list->vscrollbar))->contrlRect);
		InvalRect(&(*(list->hscrollbar))->contrlRect);
#endif
		/* The growbox needs to be redrawn on activation. */
		growRect = list->window->portRect;
		/* adjust for the scrollbars */
		growRect.top = growRect.bottom - sbarwid;
		growRect.left = growRect.right - sbarwid;
		InvalRect(&growRect);
	} else {
		/* The scrollbars must be hidden on deactivation. */
		HiliteControl(list->vscrollbar, 255);
		HiliteControl(list->hscrollbar, 255);
/*		HideControl(list->vscrollbar);
		HideControl(list->hscrollbar); */
		/* The growbox should be changed immediately on deactivation. */
		DrawGrowIcon(list->window);
	}
}

print_list(list)
List *list;
{
/*	TPPrPort printport;
	extern THPrint printrecordhandle;

	printport = PrOpenDoc(printrecordhandle, nil, nil);
	PrCloseDoc(printport); */
}

/* Remove and destroy the list object. */

destroy_list(list)
List *list;
{
	List *list2;
	
	if (listlist == list) {
		listlist = list->next;
	} else {
		for_all_lists(list2) {
			if (list2->next == list) {
				list2->next = list->next;
			}
		}
	}
	/* (should destroy substructs) */
	free(list);
}
