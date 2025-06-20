/* Copyright (c) 1992, 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Map graphics for the Mac interface. */

#include "conq.h"
#include "mac.h"

extern int daynight;
extern int gridmatchesunseen;

#define lighting(x,y,snx,sny)  \
  ((distance(wrapx(x), y, snx, sny) < (3 * world.circumference) / 5) ? 2 : 0)

/* True if the given x,y is dark. */

#define night_at(x,y) (daynight && lighting((x), (y), sunx, suny) == 0)

int tmpdrawlighting;

#define hex_overlay(x, y) (tmpdrawlighting ? (night_at(x, y) ? -1 : 0) : 0)

#define hex_terrain(x, y, power) (terrain_visible(wrapx(x), y) ? terrain_at(wrapx(x), y) : NONTTYPE)

#define hex_style(x, y, power)  \
  (terrain_visible(wrapx(x), y)  ? (power >= 4 ? usepolygons : useblocks) : dontdraw);

int seeall = FALSE;

int seeallterrain = FALSE;

int terrainseen = FALSE;

int mayseeall = FALSE;

int weseeall = FALSE;

/* The width of the left-side control panel. */

int conwid = 32;

/* The height of the top line. */

int tophgt = 16;

int mapnum = 1;

int nummaps = 0;

int lastmaph = -1, lastmapv = -1;

/* Handles of the pictures that display all the map control buttons. */

PicHandle tlcontrols = nil;
PicHandle blcontrols = nil;

char *mouseover = "something";

/* This tests whether the given cell might possibly be visible on the given map.
   Not as precise as a real cliprect calculation. */

at_all_visible(map, x, y)
Map *map;
int x, y;
{
	int sx, sy;
	
	xform(map, x, y, &sx, &sy);
	sx -= conwid;  sy -= map->toph;
	return ((sx <= map->pxw && sx + map->hw >= 0)
			&& (sy <= map->pxh && sy + map->hh >= 0));
}

/* Decide whether given location is away from the edge of the map's window. */

in_middle(map, x, y)
Map *map;
int x, y;
{
	int sx, sy, insetx1, insety1, insetx2, insety2;
	Point pt;
	Rect tmprect;
	
	xform(map, x, y, &sx, &sy);
	/* Adjust to be the center of the cell, more reasonable if large. */
	sx += map->hw / 2;  sy += map->hh / 2;
	insetx1 = min(map->pxw / 4, 1 * map->hw);  insety1 = min(map->pxh / 4, 1 * map->hch);
	insetx2 = min(map->pxw / 4, 2 * map->hw);  insety2 = min(map->pxh / 4, 2 * map->hch);
	if (sx < conwid + insetx2) return FALSE;
	if (sx > conwid + map->pxw - insetx2) return FALSE;
	if (sy < map->toph + (between(2, y, area.height-3) ? insety2 : insety1)) return FALSE;
	if (sy > map->toph + map->pxh - (between(2, y, area.height-3) ? insety2 : insety1)) return FALSE;
	return TRUE;
}

/* Draw an individual detailed hex, as a row of one, on all maps. */
  
update_cell_display(side, x, y, rightnow)
Side *side;
int x, y, rightnow;
{
	int i, sx, sy;
	Unit *unit;
	GrafPtr oldport;
	Map *map;
	RgnHandle tmprgn;
	
	if (active_display(side)) {
		GetPort(&oldport);
		for_all_maps(map) {
			/* If update was only to report temperature, maybe don't bother. */
			if (rightnow == 34 && !map->drawtemperature) continue;
			if (at_all_visible(map, x, y)) {
				/* Set up the drawing context. */
				SetPort(map->window);
				tmprgn = NewRgn();
				GetClip(tmprgn);
				/* Clip to the content area of the map's window. */
				ClipRect(&(map->contentrect));
				/* Draw the cell. */
				draw_row(map, x, y, 1, TRUE);
				/* Draw any selections that are here. */
#if 1 /* (should eventually go away, when basic drawing does this right) */				
				for (i = 0; i < map->numselections; ++i) {
					unit = map->selections[i];
					if (unit && unit->x == x && unit->y == y) {
						draw_selected_unit(map, unit);
					}
				}
#endif
				SetClip(tmprgn);
				DisposeRgn(tmprgn);
			}
		}
		SetPort(oldport);
	}
}

/* Create a new map window and all its paraphernalia. */

Map *
create_map(power)
int power;
{
	int m, sx, sy, i, x, y, h, v;
	Unit *unit;
	Rect tmprect, vscrollrect, hscrollrect;
	Map *map = (Map *) xmalloc(sizeof(Map));
	WindowPtr win;

	DGprintf("Creating map, mag power %d\n", power);
	calc_vision();
	set_map_power(map, power);
	/* Pick an appropriate focus of the view. */
	pick_a_focus(dside, &x, &y);
	map->vcx = x;  map->vcy = y;
	/* Set default values for the display controls. */
	map->toph = tophgt;
	map->drawterrain = TRUE;
	map->drawhexpats = TRUE;  /* should get from prefs */
	map->drawunits = TRUE;
	map->drawnames = defaultdrawnames;
	map->drawpeople = FALSE;
	map->drawelevations = FALSE;
	for_all_material_types(m) {
		map->drawmaterials[m] = FALSE;
	}
	map->nummaterialstodraw = 0;
	map->drawlighting = TRUE;
	map->drawtemperature = FALSE;
	map->drawwinds = FALSE;
	map->drawclouds = FALSE;
	map->drawstorms = TRUE;
	map->drawgrid = defaultdrawgrid;
	/* Display AI info by default if there is an AI present. */
	map->drawai = side_has_ai(dside);
	/* Don't indicate other maps by default - too confusing initially. */
	map->drawothermaps = FALSE;
	map->autoselect = defaultautoselect;
	map->moveonclick = defaultmoveonclick;
	map->numselections = 0;
	map->maxselections = max(100, numunits + numunits / 2);
	map->selections = (Unit **) xmalloc(map->maxselections * sizeof(Unit *));
	/* Newest map goes on the front of the list. */
	map->next = maplist;
	maplist = map;
	if (hasColorQD) {
		win = GetNewCWindow(wMap, nil, (WindowPtr) -1L);
	} else {
		win = GetNewWindow(wMap, nil, (WindowPtr) -1L);
	}
	map->window = win;
	stagger_window(win, &lastmaph, &lastmapv);
	ShowWindow(win);
	SetPort(win);
	sprintf(spbuf, "Map %d", mapnum++);
	add_window_menu_item(spbuf, win);
	set_content_rect(map);
	center_on_focus(map);
	set_map_viewport(map);
	/* Make the scrollbars. */
	vscrollrect = map->window->portRect;
	vscrollrect.top -= 1;
	vscrollrect.bottom -= sbarwid - 1;
	vscrollrect.left = vscrollrect.right - sbarwid;
	vscrollrect.right += 1;
	map->vscrollbar =
		NewControl(win, &vscrollrect, "\p", 1,
			 map->sy, 0, max(0, map->totsh - map->pxh), scrollBarProc, 0L);
	hscrollrect = win->portRect;
	hscrollrect.top = hscrollrect.bottom - sbarwid;
	hscrollrect.bottom += 1;
	hscrollrect.left += conwid + 1;
	hscrollrect.right -= sbarwid - 1;
	map->hscrollbar =
		NewControl(win, &hscrollrect, "\p", 1,
			 map->sx, 0, max(0, map->totsw - map->pxw), scrollBarProc, 0L);
	set_map_scrollbars(map);
	++nummaps;
	return map;
}

calc_vision()
{
	/* (should be in kernel) */
	seeall = g_see_all();
	terrainseen = g_terrain_seen();
	seeallterrain = all_terrain_visible();   /* (should tweak when vars change) */
}

/* Compute the content part of the map window. */

set_content_rect(map)
Map *map;
{
	map->contentrect = map->window->portRect;
	map->contentrect.left += conwid;  map->contentrect.top += map->toph;
	map->contentrect.right -= sbarwid;  map->contentrect.bottom -= sbarwid;
	map->pxw = (map->contentrect.right - map->contentrect.left);
	map->pxh = (map->contentrect.bottom - map->contentrect.top);
}

/* Set vcx/vcy to point to the center of the displayed map. */

focus_on_center(map)
Map *map;
{
	map->vcy = (map->totsh - (map->sy + map->pxh / 2)) / map->hch;
	map->vcx = map->sx / map->hw - (map->vcy / 2) + map->vh / 2;
	/* Restrict the focus to be *inside* the area. */
	map->vcy = max(1, min(map->vcy, area.height - 2));
	if (area.xwrap) {
		map->vcx = wrapx(map->vcx);
	} else {
		map->vcx = max(1, min(map->vcx, area.width - 2));
		if (map->vcx + map->vcy < area.height / 2 + 1)
			map->vcx = area.height / 2 + 1;
		if (map->vcx + map->vcy > area.width + area.height / 2 - 1)
			map->vcx = area.width + area.height / 2 - 1;
	}
}

/* Put vcx,vcy in the middle of the map. */

center_on_focus(map)
Map *map;
{
	/* Scale, add hex offset adjustment, translate to get left edge. */
	map->sx = map->vcx * map->hw + (map->vcy * map->hw) / 2 - map->pxw / 2 + map->hw / 2;
	/* Scale, translate to top edge, flip. */
	map->sy = map->totsh - (map->vcy * map->hch + map->pxh / 2 + map->hh / 2);
	/* Weird vcx,vcy might make sx,sy nonsensical, so clip to rational limits. */
	map->sx = max(0, min(map->sx, map->totsw - map->pxw));
	map->sy = max(0, min(map->sy, map->totsh - map->pxh));
	DGprintf("Mapwin at %d,%d, focused at %d,%d\n", map->sx, map->sy, map->vcx, map->vcy);
}

/* Adjust the appearance and thumb of the scroll bars to reflect the map.  This is
   needed whenever the map is scrolled under program control, such as when magnifying
   or scrolling to a specified location. */

set_map_scrollbars(map)
Map *map;
{
	int hilite;

	if (map->pxw < (map->totsw - hexagon_adjust(map))) {
		/* Set horiz min so that leftmost hex in area is at edge of window. */
		SetCtlMin(map->hscrollbar, hexagon_adjust(map));
		/* Wrapped-around areas need extra room to look at cells on the seam. */
		SetCtlMax(map->hscrollbar, map->totsw - (area.xwrap ? 0 : map->pxw));
		SetCtlValue(map->hscrollbar, map->sx);
		hilite = TRUE;
	} else {
		/* Force sx to a default value and disable the scrollbar. */
		map->sx = hexagon_adjust(map);
		hilite = FALSE;
	}
	/* Adjust the hiliting of the scrollbar, but only if the window is in front,
	   otherwise the scrollbar should remain unhilited. */
	if (map->window == FrontWindow()) {
		HiliteControl(map->hscrollbar, (hilite ? 0 : 255));
	}
	DGprintf("Hscroll (%shilite) is %d -- %d -- %d\n",
			 (hilite ? "" : "no "), GetCtlMin(map->hscrollbar),
	         GetCtlValue(map->hscrollbar), GetCtlMax(map->hscrollbar));
	if (map->pxh < map->totsh) {
		/* Vertical scrollbar min is always zero. */
		SetCtlMax(map->vscrollbar, map->totsh - map->pxh);
		/* Constrain the scaled y position of the map.  This keeps the computed
		   y from exceeding what the scroll bar will allow (which happens because
		   the sy calcs don't take scroll bar limits into account, should be fixed). */
		if (map->sy > map->totsh - map->pxh) map->sy = map->totsh - map->pxh;
		SetCtlValue(map->vscrollbar, map->sy);
		hilite = TRUE;
	} else {
		/* Force sy to the top of the map and disable the scrollbar. */
		map->sy = 0;
		hilite = FALSE;
	}
	if (map->window == FrontWindow()) {
		HiliteControl(map->vscrollbar, (hilite ? 0 : 255));
	}
	DGprintf("Vscroll (%shilite) is %d -- %d -- %d\n",
			 (hilite ? "" : "no "), GetCtlMin(map->vscrollbar),
	         GetCtlValue(map->vscrollbar), GetCtlMax(map->vscrollbar));
}

/* Given a magnification power, look up and/or calculate the sizes of everything,
   in pixels. */

set_map_power(map, power)
Map *map;
int power;
{
	map->power = power;
	map->mag = mags[power]; /* is this used?? */
	map->hw = hws[power];  map->hh = hhs[power];  map->hch = hcs[power];
	map->uw = uws[power];  map->uh = uhs[power];
	/* Calculate and cache the width in pixels of the whole area, adding an
	   an adjustment to account for the "bulge" of hexagon-shaped areas. */
	map->totsw = area.width * map->hw + hexagon_adjust(map);
	/* Total scaled height is based on center-to-center height, plus an adjustment
	   to include the bottom parts of the bottom row. */
	map->totsh = area.height * map->hch + (map->hh - map->hch);
	if (power >= 4 && cellrgns[power] == nil) make_cell_clip(power);
	DGprintf("Power is now %d, total scaled area is %d x %d\n",
			 map->power, map->totsw, map->totsh);
}

make_cell_clip(power)
int power;
{
	int hw = hws[power], hh = hhs[power], delt = (hhs[power] - hcs[power]);
	PolyHandle poly;
	RgnHandle tmprgn;

	poly = OpenPoly();
	MoveTo(hw / 2, 0);
	LineTo(hw, delt);
	LineTo(hw, hh - delt);
	LineTo(hw / 2, hh);
	LineTo(0, hh - delt);
	LineTo(0, delt);
	LineTo(hw / 2, 0);
	ClosePoly();
	cellrgns[power] = NewRgn();
	OpenRgn();
	FramePoly(poly);
	CloseRgn(cellrgns[power]);
	gridcellrgns[power] = NewRgn();
	CopyRgn(cellrgns[power], gridcellrgns[power]);
	tmprgn = NewRgn();
	SetRectRgn(tmprgn, hw - 1, 0, hw + 1, hh); 
	DiffRgn(gridcellrgns[power], tmprgn, gridcellrgns[power]);
	tmprgn = NewRgn();
	CopyRgn(cellrgns[power], tmprgn);
	OffsetRgn(tmprgn, 0, -1);
	SectRgn(gridcellrgns[power], tmprgn, gridcellrgns[power]);
}

/* Given a map with already-set pixel size and position, compute
   reasonable values for its size and position in area coordinates.
   This is really just caching, can be run at any time. */

set_map_viewport(map)
Map *map;
{
	/* Compute the size of the viewport.  Make sure it will extend past the edges
	   of the window, so that partial hexes around the edges will be filled in. */
	map->vw = min(area.width, map->pxw / map->hw + 2);
	map->vh = min(area.height, map->pxh / map->hch + 2);
	/* Compute the bottom visible row. */
	map->vy = ((map->totsh - map->sy) / map->hch) - map->vh;
	/* Now adjust the bottom row so it doesn't go outside the area. */
	if (map->vy < 0) map->vy = 0;
	if (map->vy > area.height - map->vh) map->vy = area.height - map->vh;
	/* Compute the leftmost "column". */
	map->vx = map->sx / map->hw - map->vy / 2 - 1;
	DGprintf("Set %dx%d viewport at %d,%d\n", map->vw, map->vh, map->vx, map->vy);
}

/* Given a window, find the map that it belongs to. */

Map *
map_from_window(window)
WindowPtr window;
{
	Map *map;
	
	if (dside == NULL) return NULL;
	for_all_maps(map) {
		if (map->window == window) return map;
	}
	return NULL;
}

/* Set the size of the map window and position its scrollbars correctly. */

grow_map(map, w, h)
Map *map;
int w, h;
{
/*	EraseRect(&map->window->portRect); */
	SizeWindow(map->window, w, h, 1);
	adjust_map_decor(map);
	draw_related_maps(map);
}

/* Map zooming actually does "rightsizing" if possible. */

zoom_map(map, part)
Map *map;
short part;
{
	WindowPtr mapwin = map->window;

	if (part == inZoomOut) {
		set_standard_state(mapwin,
						   area.width * map->hw + conwid + sbarwid + 1,
						   area.height * map->hch + (map->hh - map->hch) + sbarwid + map->toph + 1);
	}
	EraseRect(&mapwin->portRect);
	ZoomWindow(mapwin, part, true);
	adjust_map_decor(map);
}

GDHandle
best_zoom_screen(rectptr)
Rect *rectptr;
{
	int greatestarea = 0, sectarea;
	Rect srect;
	GDHandle screen = GetDeviceList(), bestscreen = nil;

	while (screen != nil) {
		if (TestDeviceAttribute(screen, screenDevice)
			&& TestDeviceAttribute(screen, screenActive)) {
			SectRect(rectptr, &((*screen)->gdRect), &srect);
			sectarea = (srect.right - srect.left) * (srect.bottom - srect.top);
			if (sectarea > greatestarea) {
				greatestarea = sectarea;
				bestscreen = screen;
			}
		}
		screen = GetNextDevice(screen);
	}
	return bestscreen;
}

set_standard_state(win, fullw, fullh)
WindowPtr win;
int fullw, fullh;
{
	int screenw, screenh, wintitlehgt, mbaradj = 0;
	Rect winrect, gdrect, zoomrect;
	GDHandle bestscreen;

	if (!hasColorQD) {
		zoomrect = screenBits.bounds;
		InsetRect(&zoomrect, 4, 4);
	} else {
		winrect = win->portRect;
		LocalToGlobal((Point *) &(winrect.top));
		LocalToGlobal((Point *) &(winrect.bottom));
		wintitlehgt = winrect.top - 1 - (*(((WindowPeek) win)->strucRgn))->rgnBBox.top;
		/* Get the best screen to zoom on. */
		bestscreen = best_zoom_screen(&winrect);
		gdrect = (*bestscreen)->gdRect;
		/* Adjust to the actual subarea that we can use. */
		if (bestscreen == GetMainDevice()) {
			gdrect.top += GetMBarHeight();
		}
		InsetRect(&gdrect, 3, 3);
		gdrect.top += wintitlehgt;
		screenw = gdrect.right - gdrect.left;  screenh = gdrect.bottom - gdrect.top;
		if (winrect.left + fullw <= gdrect.right
			&& winrect.top + fullh <= gdrect.bottom) {
			SetRect(&zoomrect, winrect.left, winrect.top, winrect.left + fullw, winrect.top + fullh);
		} else if (fullw <= screenw || fullh <= screenh) {
			SetRect(&zoomrect, gdrect.left, gdrect.top, gdrect.left + fullw, gdrect.top + fullh);
			if (fullw > screenw) zoomrect.right = gdrect.right;
			if (fullh > screenh) zoomrect.bottom = gdrect.bottom;
		} else {
			zoomrect = gdrect;
		}
	}
	((WStateDataPtr) *(((WindowPeek) win)->dataHandle))->stdState = zoomrect;
}

/* Move and size the controls to be correct for the map. */

adjust_map_decor(map)
Map *map;
{				
	int w, h;

	w = map->window->portRect.right - map->window->portRect.left;
	h = map->window->portRect.bottom - map->window->portRect.top;
/*	HideControl(map->hscrollbar); */
	MoveControl(map->hscrollbar, conwid, h - sbarwid);
	SizeControl(map->hscrollbar, w - conwid - sbarwid + 1, sbarwid + 1);
/*	HideControl(map->vscrollbar); */
	MoveControl(map->vscrollbar, w - sbarwid, -1);
	SizeControl(map->vscrollbar, sbarwid + 1, h - sbarwid + 1 + 1);
	set_content_rect(map);
	InvalRect(&map->window->portRect);
}

/* Given a map and a cell, compute the pixel coords of the cell's UL corner.
   This is the core routine that relates cells and pixels. */

void
xform(map, x, y, sxp, syp)
Map *map;
int x, y, *sxp, *syp;
{
	if (in_area(x, y)) {
		*sxp = x * map->hw + (y * map->hw) / 2 - map->sx;
		*syp = (map->totsh - (map->hh + y * map->hch)) - map->sy;
	} else {
		/* Always complain about this, indicates bugs. */
		run_warning("attempting to xform %d,%d", x, y);
		/* But it's not fatal, so return a semi-plausible position. */
		*sxp = *syp = 1;
	}
	/* Adjust for the control panel and topline displays. */
	*sxp += conwid;  *syp += map->toph;
}

void
xform_unit(map, unit, sxp, syp, swp, shp)
Map *map;
Unit *unit;
int *sxp, *syp, *swp, *shp;
{
	int num = 0, n = -1, sq, sx, sy, sx1, sy1, sw1, sh1, x = unit->x, y = unit->y;
	Unit *unit2;

	if (unit->transport == NULL) {
		xform(map, x, y, &sx, &sy);
		/* Adjust to the unit box within the cell. */
		sx += (map->hw - map->uw) / 2;  sy += (map->hh - map->uh) / 2;
		/* Figure out our position in this cell's stack. */
		for_all_stack(x, y, unit2) {
			if (unit == unit2) n = num;
			++num;
		}
		if (n < 0) run_error("xform_unit weirdness");
		if (num <= 1) {
			sq = 1;
		} else if (num <= 4) {
			sq = 2;
		} else if (num <= 16) {
			sq = 4;
		} else if (num <= 256) {
			sq = 8;
		} else {
			/* This is room for 65,536 units in a stack. */
			sq = 16;
		}
		*swp = map->uw / sq;  *shp = map->uh / sq;
		*sxp = sx + *swp * (n / sq);  *syp = sy + *shp * (n % sq);
	} else {
		/* Go up the transport chain to get the bounds for this unit. */
		xform_unit(map, unit->transport, &sx1, &sy1, &sw1, &sh1);
		xform_occupant(map, unit->transport, unit, sx1, sy1, sw1, sh1, sxp, syp, swp, shp);
	}
}

void
xform_unit_self(map, unit, sxp, syp, swp, shp)
Map *map;
Unit *unit;
int *sxp, *syp, *swp, *shp;
{
	int num, n, sq, sx1, sy1, sw1, sh1, x = unit->x, y = unit->y;
	Unit *unit2;

	if (unit->transport == NULL) {
		if (unit->occupant == NULL) {
			xform_unit(map, unit, sxp, syp, swp, shp);
		} else {
			xform_unit(map, unit, &sx1, &sy1, &sw1, &sh1);
			xform_occupant(map, unit, unit, sx1, sy1, sw1, sh1, sxp, syp, swp, shp);
		}
	} else {
		xform_unit(map, unit->transport, &sx1, &sy1, &sw1, &sh1);
		xform_occupant(map, unit->transport, unit, sx1, sy1, sw1, sh1, sxp, syp, swp, shp);
	}
}

void
xform_occupant(map, transport, unit, sx, sy, sw, sh, sxp, syp, swp, shp)
Map *map;
Unit *transport, *unit;
int sx, sy, sw, sh, *sxp, *syp, *swp, *shp;
{
	int num = 0, n = -1, nmx, nmy;
	Unit *unit2;

	/* Figure out the position of this unit amongst all the occupants. */
	for_all_occupants(transport, unit2) {
		if (unit2 == unit) n = num;
		++num;
	}
	if (unit == transport) {
		if (num > 0) {
			/* Transport image shrinks by half in each dimension. */
			*swp = sw / 2;  *shp = sh / 2;
		}
		/* Transport is always in the UL corner. */
		*sxp = sx;  *syp = sy;
	} else {
		if (n < 0) run_error("xform_occupant weirdness");
		/* Compute how the half-box will be subdivided.  Only use powers of two,
		   so image scaling works better. */
		if (num <= 2) {
			nmx = 2;
		} else if (num <= 8) {
			nmx = 4;
		} else if (num <= 128) {
			nmx = 8;
		} else {
			/* This is room for 32,768 units in a stack. */
			nmy = 16;
		}
		nmy = nmx / 2;
		*swp = sw / nmx;  *shp = (sh / 2) / nmy;
		*sxp = sx + *swp * (n / nmy);  *syp = sy + sh / 2 + *shp * (n % nmy);
	}
}

/* Un-transform screen coordinates (as supplied by mouse perhaps) into
   map coordinates.  This routine doesn't yet
   pay attention to hexagon boundaries, using only box-shaped areas,
   but in practice (for small hexes only) this seems to be OK. */

nearest_cell(map, sx, sy, xp, yp)
Map *map;
int sx, sy, *xp, *yp;
{
	/* Subtract offset due to map decor. */
	sx -= conwid;
	sy -= map->toph;
	/* Flip the raw y and then scale to hex coords. */
	*yp = (map->totsh - (map->sy + sy)) / map->hch;
	/* Scale adjusted x to hex coord. */
	*xp = (sx + map->sx - (*yp * map->hw) / 2) / map->hw;
	/* If the magnification of the map is large enough that the top and bottom
	   edges of a hex are visibly sloping, then we have to take those edges
	   int account, and accurately. */
	if ((map->hh - map->hch) / 2 > 1) {
		/* (should adjust according to hex boundaries correctly here) */
	}
	/* Wrap coords as usual. */
	if (area.xwrap) *xp = wrapx(*xp);
	DGprintf("Pixel %d,%d -> hex %d,%d\n", sx, sy, *xp, *yp);
	return (in_area(*xp, *yp));
}

/* Find the closest direction of the closest boundary. */

nearest_boundary(map, sx, sy, xp, yp, dirp)
Map *map;
int sx, sy, *xp, *yp, *dirp;
{
	int sx2, sy2, ydelta, hexslope;

	/* Get the nearest cell... */
	nearest_cell(map, sx, sy, xp, yp);
	/* ... and xform it back to get the pixel coords. */ 
	xform(map, *xp, *yp, &sx2, &sy2);
	ydelta = sy - sy2;
	hexslope = (map->hh - map->hch) / 2;
	if (sx - sx2 > map->hw / 2) {
		*dirp = ((ydelta < hexslope) ? NORTHEAST : (ydelta > map->hch ? SOUTHEAST : EAST));
	} else {
		*dirp = ((ydelta < hexslope) ? NORTHWEST : (ydelta > map->hch ? SOUTHWEST : WEST));
	}
	DGprintf("Pixel %d,%d -> hex %d,%d dir %d\n", sx, sy, *xp, *yp, *dirp); 
}

Unit *
find_unit_or_occ(map, unit, usx, usy, usw, ush, sx, sy)
Map *map;
Unit *unit;
int usx, usy, usw, ush, sx, sy;
{
	int usx1, usy1, usw1, ush1;
	Unit *occ, *rslt;

	/* See if the point might be over an occupant. */
	if (unit->occupant != NULL) {
		for_all_occupants(unit, occ) {
			xform_unit(map, occ, &usx1, &usy1, &usw1, &ush1);
			if ((rslt = find_unit_or_occ(map, occ, usx1, usy1, usw1, ush1, sx, sy))) {
				return rslt;
			}
		}
	}
	/* Otherwise see if it could be the unit itself.  This has the effect of
	   "giving" the transport everything in its box that is not in an occ. */
	xform_unit(map, unit, &usx1, &usy1, &usw1, &ush1);
	if (between(usx1, sx, usx1 + usw1) && between(usy1, sy, usy1 + ush1)) {
		return unit;
	}
	return NULL;
}

Unit *
find_unit_at(map, x, y, sx, sy)
Map *map;
int x, y, sx, sy;
{
	int usx, usy, usw, ush;
	Unit *unit, *rslt;
	
	for_all_stack(x, y, unit) {
		xform_unit(map, unit, &usx, &usy, &usw, &ush);
		if ((rslt = find_unit_or_occ(map, unit, usx, usy, usw, ush, sx, sy))) {
			return rslt;
		}
	}
	return NULL;
}

nearest_unit(map, sx, sy, unitp)
Map *map;
int sx, sy;
Unit **unitp;
{
	int x, y;

	if (!nearest_cell(map, sx, sy, &x, &y)) {
		*unitp = NULL;
	} else if (map->power > 4) {
		*unitp = find_unit_at(map, x, y, sx, sy);
	} else {
		*unitp = unit_at(x, y);
	}
	DGprintf("Pixel %d,%d -> unit %s\n", sx, sy, unit_desig(*unitp)); 
}

erase_map(map)
Map *map;
{
	BackPat(QD(gray));
	EraseRect(&(map->window->portRect));
}

/* Display a map and all of its paraphernalia. */

draw_map(map)
Map *map;
{
	Rect tmprect;
	WindowPtr mapwin = map->window;
	RgnHandle tmprgn;

	/* Draw control panel and topline before clipping to inner part of window. */
	draw_control_panel(map);
	if (map->toph > 0) draw_top_line(map);
	tmprgn = NewRgn();
	GetClip(tmprgn);
	ClipRect(&(map->contentrect));
	/* Calculate shapes and sizes. */
	/* If part of the window is entirely outside the world, we draw its shape on
	   top of gray, otherwise window starts out all white. */
	if (area.width * map->hw < 32000) {
		switch (bggray) {
			case blackgray:
				FillRect(&(map->contentrect), QD(black));  break;
			case darkgray:
				FillRect(&(map->contentrect), QD(dkGray));  break;
			case mediumgray:
				FillRect(&(map->contentrect), QD(gray));  break;
			case lightgray:
				FillRect(&(map->contentrect), QD(ltGray));  break;
			case whitegray:
				FillRect(&(map->contentrect), QD(white));  break;
		}
	} else {
		if (hasColorQD) {
			RGBForeColor((gridmatchesunseen ? &gridcolor : &unseencolor));
			PaintRect(&(map->contentrect));
			RGBForeColor(&blackcolor);
		} else {
			switch ((gridmatchesunseen ? gridgray : unseengray)) {
				case blackgray:
					FillRect(&(map->contentrect), QD(black));  break;
				case darkgray:
					FillRect(&(map->contentrect), QD(dkGray));  break;
				case mediumgray:
					FillRect(&(map->contentrect), QD(gray));  break;
				case lightgray:
					FillRect(&(map->contentrect), QD(ltGray));  break;
				case whitegray:
					FillRect(&(map->contentrect), QD(white));  break;
			}
		}
	}
	set_map_scrollbars(map);
	set_map_viewport(map);
	draw_area_background(map);
	draw_map_content(map);
	if (map->drawothermaps) draw_other_maps(map);
	draw_selections(map);
#ifdef DEBUGGING
	/* Indicate where the focus is. */
	if (DebugG) {
		int sx, sy;

		xform(map, map->vcx, map->vcy, &sx, &sy);
		SetRect(&tmprect, sx, sy, sx + map->hw, sy + map->hh);
		InsetRect(&tmprect, -4, -4);
		InvertOval(&tmprect);
		InsetRect(&tmprect, 2, 2);
		InvertOval(&tmprect);
	}
#endif /* DEBUGGING */
	SetClip(tmprgn);
	DisposeRgn(tmprgn);
}

/* Draw the actual map data. */

draw_map_content(map)
Map *map;
{
	int y1, y2, y, x1, x2, xx1, yy1, xx2, yy2;
	int halfheight = area.height / 2;
	int limitleft = FALSE, limitrite = FALSE;
	Rect bbox = (*(map->window->visRgn))->rgnBBox;
	Rect tmprect = map->contentrect;
	extern int endofgame;

	ClipRect(&(map->contentrect));
	if (DebugG) {
		FillRgn(map->window->visRgn, QD(white));
	}
	/* Compute top and bottom rows to be displayed. */
	y1 = min(map->vy + map->vh, area.height - 1);
	y2 = map->vy;
	/* Find the top and bottom rows that are in the visRgn. */
	if (nearest_cell(map, bbox.left, bbox.top, &xx1, &yy1))
	  limitleft = TRUE;
	if (nearest_cell(map, bbox.right + map->hw, bbox.bottom + map->hh, &xx2, &yy2))
	  limitrite = TRUE;
	DGprintf("Map rows are %d - %d, update area rows are %d - %d\n", y2, y1, yy2, yy1);
	if (between(y2, yy1, y1)) y1 = yy1; 
	if (between(y2, yy2, y1)) y2 = yy2;
	for (y = y1; y >= y2; --y) {
		/* Adjust the right and left bounds to fill the viewport as
		   much as possible, without going too far (the drawing code
		   will clip, but clipped drawing is still expensive). */
		/* could test by drawing viewport rect as lines... */
		x1 = map->vx - (y - map->vy) / 2;
		x2 = x1 + map->vw + 2 /* bleah, shouldn't be necessary */;
		/* If the area doesn't wrap, then we might have to stop
		   drawing before we reach the edge of the viewport. */
		if (!area.xwrap) {
			/* Truncate x's to stay within the area. */
			x1 = max(0, min(x1, area.width-1));
			x2 = max(0, min(x2, area.width));
			/* If this row is entirely in the NE corner, don't draw anything. */
			if (x1 + y > area.width + halfheight)
			  continue;
			/* If this row is entirely in the SW corner, don't draw anything. */
			if (x2 + y < halfheight)
			  continue;
			/* If the row ends up in the NE corner, shorten it. */
			if (x2 + y > area.width + halfheight)
			  x2 = area.width + halfheight - y;
			/* If the row starts out in the SW corner, shorten it. */
			if (x1 + y < halfheight)
			  x1 = halfheight - y;
		}
		/* Clip the ends of the row to the visRgn. */
		if (limitleft && between(x1, xx1, x2)) x1 = xx1; 
		if (limitrite && between(x1, xx2, x2)) x2 = xx2;
		draw_row(map, x1, y, x2 - x1, FALSE);
	}
	/* If the game is over, draw a large X over the map. */
	if (endofgame) {
		PenSize(2, 2);
		PenPat(QD(dkGray));
		MoveTo(tmprect.left,  tmprect.top);  LineTo(tmprect.right, tmprect.bottom);
		MoveTo(tmprect.right, tmprect.top);  LineTo(tmprect.left,  tmprect.bottom);
		PenNormal();
	}
}

/* This draws a hexagon or rectangle that covers the total area of the area, whether
   seen or not. */
 
draw_area_background(map)
Map *map;
{
	int sx, sy, x, y;
	int llx, lly, lrx, lry, rx, ry, urx, ury, ulx, uly, lx, ly;
	PolyHandle poly;
	Rect arearect;

	if (area.width * map->hw < 32000) {
		if (area.xwrap) {
			xform(map, 0, 0, &sx, &sy);
			arearect.left = 0;  arearect.top = 0;
			arearect.right = area.width * map->hw;  arearect.bottom = sy;
			if (hasColorQD) {
				RGBForeColor(&gridcolor);
				PaintRect(&arearect);
				RGBForeColor(&blackcolor);
			} else {
				switch (gridgray) {
					case blackgray:
						FillRect(&arearect, QD(black));   break;
					case darkgray:
						FillRect(&arearect, QD(dkGray));  break;
					case mediumgray:
						FillRect(&arearect, QD(gray));    break;
					case lightgray:
						FillRect(&arearect, QD(ltGray));  break;
					case whitegray:
						FillRect(&arearect, QD(white));   break;
				}
			}
		} else {
			/* (should make once and save?) */
			poly = OpenPoly();		
			xform(map, 0 + area.height/2, 0, &llx, &lly);
			MoveTo(llx, lly);
			xform(map, area.width-1, 0, &lrx, &lry);
		 	LineTo(lrx, lry);
			xform(map, area.width-1, area.height/2, &rx, &ry);
			LineTo(rx, ry);
 			xform(map, area.width-1 - area.height/2, area.height-1, &urx, &ury);
			LineTo(urx, ury);
 			xform(map, 0, area.height-1, &ulx, &uly);
			LineTo(ulx, uly);
 			xform(map, 0, area.height/2, &lx, &ly);
			LineTo(lx, ly);
			LineTo(llx, lly);
			ClosePoly();
			OffsetPoly(poly, map->hw/2, map->hh/2);
			if (hasColorQD) {
				RGBForeColor(&gridcolor);
				PaintPoly(poly);
				RGBForeColor(&blackcolor);
			} else {
				switch (gridgray) {
					case blackgray:
						FillPoly(poly, QD(black));   break;
					case darkgray:
						FillPoly(poly, QD(dkGray));  break;
					case mediumgray:
						FillPoly(poly, QD(gray));    break;
					case lightgray:
						FillPoly(poly, QD(ltGray));  break;
					case whitegray:
						FillPoly(poly, QD(white));   break;
				}
			}
		}
#if 0
		/* This doesn't look very good in practice. */
		for (x = 0; x < area.width; ++x) {
			xform(map, x, 0, &sx, &sy);
			draw_border_line(sx, sy, SW, map->power, -1);
			draw_border_line(sx, sy, SE, map->power, -2);
		}
		PenNormal();
#endif
	}
}

/* Draw the map control panel as a pair of PICTs. */

draw_control_panel(map)
Map *map;
{
	int winhgt, baseh, basev;
	Rect tmprect;

	winhgt = (map->window->portRect).bottom - (map->window->portRect).top;
	SetRect(&tmprect, 0, 0, conwid, winhgt);
	FillRect(&tmprect, QD(white));
	MoveTo(conwid - 1, 0);  Line(0, winhgt);
	if (tlcontrols == nil) {
		tlcontrols = (PicHandle) GetResource('PICT', pMapControlsTL);
	}
	if (tlcontrols != nil) {
		SetRect(&tmprect, 0, 0,
				picture_width(tlcontrols), picture_height(tlcontrols));
		DrawPicture(tlcontrols, &tmprect);
	}
	if (blcontrols == nil) {
		blcontrols = (PicHandle) GetResource('PICT', pMapControlsBL);
	}
	if (blcontrols != nil) {
		SetRect(&tmprect, 0, winhgt - picture_height(blcontrols),
				picture_width(blcontrols), winhgt);
		DrawPicture(blcontrols, &tmprect);
	}
	if (map->moveonclick && map->autoselect) {
		SetRect(&tmprect, 4, 5, 26, 26);
		InvertRect(&tmprect);
	}
	/* (should modify appearance of top left arrow buttons to reflect abilities) */
	basev = 32 + 5*15 + 2 + 5/*why?*/ + 1;
	SetRect(&tmprect, 0, basev, 30, basev + 10);
	if (map->drawgrid) {
		InvertRect(&tmprect);
	}
	OffsetRect(&tmprect, 0, 11);
	if (map->drawnames) {
		InvertRect(&tmprect);
	}
	OffsetRect(&tmprect, 0, 11);
	if (map->drawpeople) {
		InvertRect(&tmprect);
	} else if (!people_sides_defined()) {
		gray_out_rect(&tmprect);
	}
	OffsetRect(&tmprect, 0, 11);
	if (map->drawplans) {
		InvertRect(&tmprect);
	}
	OffsetRect(&tmprect, 0, 11);
	if (map->drawai) {
		InvertRect(&tmprect);
	} else if (!side_has_ai(dside)) {
		/* (should ensure that this is updated when side gets an AI) */
		gray_out_rect(&tmprect);
	}
	/* Draw state of bottom left control buttons. */
	if (map->power <= 0) {
		SetRect(&tmprect, 0, winhgt - 15, 15, winhgt);
		gray_out_rect(&tmprect);
	}
	if (map->power >= NUMPOWERS - 1) {
		SetRect(&tmprect, 16, winhgt - 15, 30, winhgt);
		gray_out_rect(&tmprect);
	}
}

draw_top_line(map)
Map *map;
{
	int numchars;
	Rect tmprect;

	/* Clear the whole topline area. */
	SetRect(&tmprect, conwid, 0, conwid + map->pxw, tophgt);
	FillRect(&tmprect, QD(white));
	/* Draw a line dividing this from the map content. */
	MoveTo(conwid, tmprect.bottom - 1);  Line(map->pxw, 0);
	/* Draw the current date. */
	TextSize(10);
	MoveTo(tmprect.right - 60, tmprect.top + 11);
	DrawString(curdatestr);
	if (mouseover != NULL) {
		/* Draw description of what the mouse is over. */
		/* (should clip to avail space) */
		numchars = strlen(mouseover);
		MoveTo(conwid + 10, tmprect.top + 11);
		DrawText(mouseover, 0, numchars);
	}
}

/* Draw an indication of the position of other maps relative to this one. */

draw_other_maps(map)
Map *map;
{
	Map *map2;

	for_all_maps(map2) {
		if (map != map2 /* && reasonable to show? */) {
			draw_other_map(map, map2);
		}
	}
}

draw_related_maps(map)
Map *map;
{
	Map *map2;
	GrafPtr oldport;

	for_all_maps(map2) {
		if (map != map2 && map2->drawothermaps /* && reasonable to show? */) {
			GetPort(&oldport);
			SetPort(map2->window);
			/* (also clipping?) */
			draw_other_map(map2, map);
			SetPort(oldport);
		}
	}
}

draw_other_map(map, map2)
Map *map, *map2;
{
	int sx, sy, sw, sh;
	Rect tmprect;

	sx = (map2->sx * map->hw) / map2->hw - map->sx;
	sy = (map2->sy * map->hch) / map2->hch - map->sy;
	sw = (map2->pxw * map->hw) / map2->hw;
	sh = (map2->pxh * map->hch) / map2->hch;
	SetRect(&tmprect, sx, sy, sx + sw, sy + sh);
	OffsetRect(&tmprect, conwid, map->toph);
	if (map->hw < 8) PenSize(2, 2);
	PenMode(patXor);
	FrameRect(&tmprect);
	PenNormal();
}

/* x0 may be negative here. */

draw_row(map, x0, y0, len, clearit)
Map *map;
int x0, y0, len, clearit;
{
	int empty = FALSE;
	char ch;
	int i = 0, x, sx, sy, t;

	if (!between(0, y0, area.height - 1)) return;

	if (partial_views() && !map->drawai) {
		empty = TRUE;
		/* Examine row to see if we can skip it entirely. */
		for (x = x0; x < x0 + len; ++x) {
			if (terrain_visible(wrapx(x), y0)) { empty = FALSE;  break; }
			/* (only need following test if border types possible) */
			if (numbordtypes > 0 && terrain_visible(wrapx(x), y0 + 1)) { empty = FALSE;  break; }
		}
	}
	if (empty) return;
	/* The terrain always comes first. */
	if (map->drawterrain) {
		draw_terrain_row(map, x0, y0, len);
	}
	if (map->drawterrain && any_aux_terrain_defined()) {
		/* The relative ordering of these is quite important - connections
		   should always be drawn on top of borders.  Note that
		   each should be prepared to run independently also, since the
		   other displays might have been turned off. (?) */
		if (bords_to_draw(map)) {
			for_all_terrain_types(t) {
				if (t_is_border(t) && aux_terrain_defined(t)) {
					for (x = x0; x < x0 + len; ++x) {
						draw_borders(map, x, y0, t);
					}
				}
			}
		}
		if (conns_to_draw(map)) {
			for_all_terrain_types(t) {
				if (t_is_connection(t) && aux_terrain_defined(t)) {
					for (x = x0; x < x0 + len; ++x) {
						draw_connections(map, x, y0, t);
					}
				}
			}
		}
	}
	/* Although we had to draw the terrain on the edge, we can skip everything else. */
	/* Skip the top and bottom edge rows. */
	if (!between(1, y0, area.height - 2)) return;
	/* Shorten the row. */
	if (!inside_area(x0+len-1, y0)) --len;
	if (len <= 0) return;
	if (!inside_area(x0, y0)) ++x0;
	if (any_cell_materials_defined() && map->nummaterialstodraw > 0 && map->hh > 20) {
		for (x = x0; x < x0 + len; ++x) {
			draw_materials(map, x, y0);
		}
	}
	/* (should be global to entire map drawing somehow?) */
	if (map->drawnames && map->hh > 5) {
		for (x = x0; x < x0 + len; ++x) {
			if (!inside_area(x, y0)) continue;
			draw_legend(map, x, y0);
		}
	}
	if (people_sides_defined() && bwid2[map->power] > 0) {
		draw_people_row(map, x0, y0, len);
	}
	if (map->drawunits && map->hw > 2) {
		for (x = x0; x < x0 + len; ++x) {
			draw_units(map, x, y0);
		}
	}
	if (map->drawai) {
		for (x = x0; x < x0 + len; ++x) {
			draw_theater(map, x, y0);
		}
	}
	/* If debugging, draw coverage on top of everything else. */
	if (DebugG && !g_see_all()) {
		for (x = x0; x < x0 + len; ++x) {
			xform(map, x, y0, &sx, &sy);
			draw_coverage(sx, sy, map->power, cover(dside, x, y0));
		}
	}
}

hex_update(map, x, y)
Map *map;
int x, y;
{
	int sx, sy;
	Rect tmprect;

	xform(map, x, y, &sx, &sy);
	SetRect(&tmprect, sx, sy, sx + map->hw, sy + map->hh);
	return (RectInRgn(&tmprect, map->window->visRgn));
}

/* Draw an entire row of terrain, possibly with a single rectangle fill. */
 
/* x0 may be negative. */

extern int sunx, suny;

draw_terrain_row(map, x0, y0, len)
Map *map;
int x0, y0, len;
{
	int empty;
	int x, x1, t, sx, sy, w, h, p = map->power;
	int dogrid = map->drawgrid, dofill = map->drawhexpats;
	int i = 0;
	int inarea, seginarea;
	int style, segstyle;
	int terr, segterr;
	int over, segover;
	int update, segupdate;
	Rect tmprect, rect;
	Image *timg;

	tmpdrawlighting = map->drawlighting;
	x1 = x0;
	seginarea = in_area(x0, y0);
	segstyle = hex_style(x0, y0, p);
	segterr = hex_terrain(x0, y0, p);
	segover = hex_overlay(x0, y0);
	segupdate = (seginarea ? TRUE /* hex_update(map, x0, y0) */ : FALSE);
	for (x = x0; x < x0 + len + 1; ++x) {
		inarea = in_area(x, y0);
		style = hex_style(x, y0, p);
		terr = hex_terrain(x, y0, p);
		over = hex_overlay(x, y0);
		update = (inarea ? TRUE /* hex_update(map, x, y0) */ : FALSE);
		/* Decide if the run is over and we need to dump some output. */
		if (x == x0 + len
			|| inarea != seginarea
			|| style != segstyle
			|| terr != segterr
			|| over != segover
			|| update != segupdate
			|| segstyle == usepictures
			|| segstyle == usepolygons) {
			/* don't draw anything that would match the window's bg */
			if (seginarea && segupdate && segstyle != dontdraw) {
				xform(map, x1, y0, &sx, &sy);
				switch (segstyle) {
					case useblocks:
						draw_hex_block(sx, sy, i, map->power, segterr, segover);
						break;
					case usepictures:
						break;
					case usepolygons:
						draw_hex_region(sx, sy, map->power, map->drawgrid, segterr, segover);
						/* Assume that only polygon scale can fit numbers. */
						if (elevations_defined()
							&& map->drawelevations
							&& 1 /* draw_elevation_here(x1, y0) */) {
							draw_elevation(sx, sy, map->power, elev_at(x1, y0));
						}
						if (clouds_defined()
						    && map->drawclouds
						    && draw_clouds_here(x1, y0)) {
							draw_clouds(sx, sy, map->power, raw_cloud_at(x1, y0), 0, 0);
						}
						if (winds_defined()
						    && map->drawwinds
						    && draw_winds_here(x1, y0)) {
							draw_winds(sx, sy, map->power, wind_dir_at(x1, y0), wind_force_at(x1, y0));
						}
						if (temperatures_defined()
						    && map->drawtemperature
						    && draw_temperature_here(x1, y0)) {
							draw_temperature(sx, sy, map->power, temperature_at(x1, y0));
						}
				}
			}
			i = 0;
			x1 = x;
			seginarea = inarea;
			segstyle = style;
			segterr = terr;
			segover = over;
			segupdate = update;
		}
		++i;
	}
}

draw_borders(map, x, y, b)
Map *map;
int x, y, b;
{
	int dir, sx, sy, bitmask = 0;
	
 	if (!terrain_visible(wrapx(x), y) || !any_borders_at(x, y, b)) return;
	for_all_directions(dir) {
		if (border_at(x, y, dir, b) && borders_visible(x, y, dir)) {
			bitmask |= 1 << dir;
		}
	}
	if (bitmask != 0) {
		xform(map, x, y, &sx, &sy);
		draw_border_line_multiple(map->window, sx, sy, bitmask, map->power, b);
	}
}

/* Draw all the connections of the given hex. */

draw_connections(map, x, y, c)
Map *map;
int x, y, c;
{
	int sx, sy, dir, bitmask = 0, t = terrain_at(x, y);
	
	if (!terrain_visible(wrapx(x), y) || !any_connections_at(x, y, c)) return;
	for_all_directions(dir) {
		if (connection_at(x, y, dir, c)) {
			bitmask |= 1 << dir;
		}
	}
	if (bitmask != 0) {
		xform(map, x, y, &sx, &sy);
		draw_connection_line_multiple(map->window, sx, sy, bitmask, map->power, c);
	}
}

/* Draw all the units visible in the given cell. */

draw_units(map, x, y)
Map *map;
int x, y;
{
	short uview;
	int sx, sy, sw, sh, u, s;
	int terr = terrain_at(wrapx(x), y);
	Side *side2;
	Unit *unit;
	Rect tmprect;
	extern PicHandle dotdotdotpicture;

	if (units_visible(x, y) && unit_at(x, y) != NULL) {
		xform(map, x, y, &sx, &sy);
		if (map->uw <= 16) {
			unit = unit_at(x, y);
			/* Adjust to unit part of cell. */
			sx += (map->hw - map->uw) / 2;  sy += (map->hh - map->uh) / 2;
			draw_unit_image(map->window, sx, sy, map->uw, map->uh,
							unit->type, side_number(unit->side), !completed(unit));
			/* Indicate if more than one stacked here. */
			if (unit->nexthere != NULL && map->uw > 8) {
				SetRect(&tmprect, sx + map->uw/2 - 6, sy + map->uh - 2,
								  sx + map->uw/2 + 6, sy + map->uh + 2);
				/* (should clip to fit in hex) */
				DrawPicture(dotdotdotpicture, &tmprect);
			}
			if (map->drawnames) draw_unit_name(unit, sx, sy, map->uw, map->uh);
		} else {
			for_all_stack(x, y, unit) {
				xform_unit(map, unit, &sx, &sy, &sw, &sh);
				draw_unit_and_occs(map, unit, sx, sy, sw, sh);
			}
		}
	} else {
		if ((uview = unit_view(dside, x, y)) != EMPTY) {
			u = vtype(uview);  s = vside(uview);
			xform(map, x, y, &sx, &sy);
			/* Adjust to unit part of cell. */
			sx += (map->hw - map->uw) / 2;  sy += (map->hh - map->uh) / 2;
			draw_unit_image(map->window, sx, sy, map->uw, map->uh, u, s, 0);
		}
	}
}

draw_unit_and_occs(map, unit, sx, sy, sw, sh)
Map *map;
Unit *unit;
int sx, sy, sw, sh;
{
	int u = unit->type, s = side_number(unit->side), sx2, sy2, sw2, sh2;
	Unit *occ;
	Rect tmprect;

	/* If an occupant's side is the same as its transport's, then there's
	   really no need to draw its side emblem, since the transport's emblem
	   will also be visible. */
	if (unit->transport && unit->side == unit->transport->side) s = -1;
	if (unit->occupant == NULL) {
		draw_unit_image(map->window, sx, sy, sw, sh, u, s, !completed(unit));
		if (map->drawnames) draw_unit_name(unit, sx, sy, sw, sh); 
	} else {
		/* Draw a sort of "grouping box", in white. */
		SetRect(&tmprect, sx, sy, sx + sw, sy + sh);
		FillRect(&tmprect, QD(white));
		FrameRect(&tmprect);
		/* Draw the transport in the UL quarter of the box. */
		xform_occupant(map, unit, unit, sx, sy, sw, sh, &sx2, &sy2, &sw2, &sh2);
		draw_unit_image(map->window, sx2, sy2, sw2, sh2, u, s, !completed(unit));
		if (map->drawnames) draw_unit_name(unit, sx2, sy2, sw2, sh2);
		/* Draw all the occupants, in the bottom half of the box. */
		for_all_occupants(unit, occ) {
			xform_occupant(map, unit, occ, sx, sy, sw, sh, &sx2, &sy2, &sw2, &sh2);
			draw_unit_and_occs(map, occ, sx2, sy2, sw2, sh2);
		}
	}
}

/* Indicate what kind of people are living in the given hex. */

draw_people_row(map, x0, y, len)
Map *map;
int x0, y, len;
{
	int pop, xx, x, sx, sy, sw, sh, ex, ey, ew, eh, dir, x1, y1, pop1;
	int bitmask1, bitmask2, drawemblemhere;

	for (xx = x0; xx < x0 + len; ++xx) {
		x = wrapx(xx);
		if (!terrain_visible(x, y)) continue;
		pop = people_side_at(x, y);
		bitmask1 = bitmask2 = 0;
		drawemblemhere = FALSE;
		/* Decide which edges are borders of the country. */
		for_all_directions(dir) {
			/* Don't do anything about edge cells. */
			if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
				if (terrain_visible(x1, y1)) {
					pop1 = people_side_at(x1, y1);
					if (pop != pop1) {
						/* Borders with uninhabitated regions are drawn differently. */
						if (pop == NOBODY || pop1 == NOBODY) {
							bitmask2 |= 1 << dir;
						} else {
							bitmask1 |= 1 << dir;
						}
					}
				} else {
					/* Draw just people in the cells right at the edge of the known world. */
					drawemblemhere = TRUE;
				}
			}
		}
		/* Now draw both the edges and an emblem for the cell. */
		if ((bitmask1 | bitmask2) != 0 || (map->drawpeople && drawemblemhere)) {
			xform(map, x, y, &sx, &sy);
			if (bitmask1 != 0) {
				draw_country_borders(map->window, sx, sy, bitmask1, map->power, 0);
			}
			if (bitmask2 != 0) {
				draw_country_borders(map->window, sx, sy, bitmask2, map->power, 2);
			}
			/* Draw an emblem for the people in the cell. */
			if (map->drawpeople && pop != NOBODY) {
				sw = map->uw;  sh = map->uh;
				ew = min(sw, max(8, sw / 2));  eh = min(sh, max(8, sh / 2));
				ex = sx + (map->hw - map->uw) / 2 + sw / 2 - ew / 2;  ey = sy + (map->hh - map->uh) / 2 + sh / 2 - eh / 2;
				draw_side_emblem(map->window, ex, ey, ew, eh, pop);
			}
		}
	}
}

#if 0
draw_people(map, x, y)
Map *map;
int x, y;
{
	int pop, sx, sy, sw, sh, ex, ey, ew, eh, dir, x1, y1, pop1;
	int bitmask1 = 0, bitmask2 = 0, drawemblemhere = FALSE;

	if (!terrain_visible(x, y)) return;
	pop = people_side_at(wrapx(x), y);
	/* Decide which edges are borders of the country. */
	for_all_directions(dir) {
		/* Don't do anything about edge cells. */
		if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
			if (terrain_visible(x1, y1)) {
				pop1 = people_side_at(x1, y1);
				if (pop != pop1) {
					/* Borders with uninhabitated regions are drawn differently. */
					if (pop == NOBODY || pop1 == NOBODY) {
						bitmask2 |= 1 << dir;
					} else {
						bitmask1 |= 1 << dir;
					}
				}
			} else {
				/* Draw just people in the cells right at the edge of the known world. */
				drawemblemhere = TRUE;
			}
		}
	}
	/* Now draw both the edges and an emblem for the cell. */
	if ((bitmask1 | bitmask2) != 0 || (map->drawpeople && drawemblemhere)) {
		xform(map, x, y, &sx, &sy);
		if (bitmask1 != 0) {
			draw_country_borders(map->window, sx, sy, bitmask1, map->power, 0);
		}
		if (bitmask2 != 0) {
			draw_country_borders(map->window, sx, sy, bitmask2, map->power, 2);
		}
		/* Draw an emblem for the people in the cell. */
		if (map->drawpeople && pop != NOBODY) {
			sw = map->uw;  sh = map->uh;
			ew = min(sw, max(8, sw / 2));  eh = min(sh, max(8, sh / 2));
			ex = sx + (map->hw - map->uw) / 2 + sw / 2 - ew / 2;  ey = sy + (map->hh - map->uh) / 2 + sh / 2 - eh / 2;
			draw_side_emblem(map->window, ex, ey, ew, eh, pop);
		}
	}
}
#endif

/* This draws a small set of bar charts, one for each material type. */

draw_materials(map, x, y)
Map *map;
int x, y;
{
	int m, t, sx, sy, mx, my, mw, mh, amt, maxamt, h;
	Rect graphrect;
	
	if (nummtypes == 0) return;
	mw = map->uw / nummtypes /* should be count of displayable materials... */;  mh = map->uh;
	if (mw <= 2 || mh <= 2) return;
	t = terrain_at(x, y);
	xform(map, x, y, &sx, &sy);
	mx = sx + (map->hw - map->uw) / 2;  my = sy + (map->hh - map->uh) / 2;
	for_all_material_types(m) {
		if (map->drawmaterials[m] && (maxamt = tm_storage_x(t, m)) > 0) {
			SetRect(&graphrect, mx + m * mw, my, mx + (m + 1) * mw, my + map->uh);
			FrameRect(&graphrect);
			amt = material_at(x, y, m);
			h = (amt * mh) / maxamt;
			graphrect.top -= (mh - h);
			FillRect(&graphrect, QD(black));
		}
	}
}

draw_theater(map, x, y)
Map *map;
int x, y;
{
	int thid, sx, sy, sw, sh, ex, ey, ew, eh, dir, x1, y1, thid1;
	int bitmask = 0;

	thid = ai_theater_at(dside, wrapx(x), y);
	/* Decide which edges are borders of the theater. */
	for_all_directions(dir) {
		/* Don't do anything about edge cells. */
		if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
			thid1 = ai_theater_at(dside, x1, y1);
			if (thid != thid1) {
				bitmask |= 1 << dir;
			}
		}
	}
	if (bitmask != 0) {
		xform(map, x, y, &sx, &sy);
		if (bitmask != 0) {
			draw_theater_borders(map->window, sx, sy, bitmask, map->power);
		}
	}
}

/* Draw any text that should be associated with this hex. */

/* (could precompute what the string will lap over and move or truncate str),
   should be deterministic for each mag, so redraw doesn't scramble */

char *feature_desc();

draw_legend(map, x, y)
Map *map;
int x, y;
{
	int xx, numhexes = 1, color, sx, sy, pixlen;
	char *str, buf[BUFSIZE];
	Unit *unit;
	Feature *feature;

	/* Draw the name of a terrain feature. */
	/* (should limit to one cell of feature, preferably centering on quasi-centroid) */	
	if (terrain_visible(wrapx(x), y)) {
		if ((feature = feature_at(x, y)) != NULL) {
			if (1 /* feature->size > 0  make sure this is calced! */) {
				if (1 /* center of feature, or center far away */) {
					if ((str = feature_desc(feature, buf)) != NULL) {
						xform(map, x, y, &sx, &sy);
						draw_legend_text(sx + map->hw/2, sy + map->hh/2, map->uh, str, 0);
					}
				}
			}
		}
	}
}

draw_blast(unit, side3, hit)
Unit *unit;
Side *side3;
int hit;
{
	int ux = unit->x, uy = unit->y, sx, sy, sw, sh, i, btype;
	int startticks, innerticks;
	Side *us = unit->side, *side;
	Map *map;
	GrafPtr oldport, curport = NULL;

	btype = ((hit >= unit->hp) ? 2 : ((hit > 0) ? 1 : 0));
	if (active_display(side3) && units_visible(ux, uy)) {
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
				xform_unit(map, unit, &sx, &sy, &sw, &sh);
				draw_blast_image(map->window, sx, sy, sw, sh, btype);
			}
			while (TickCount() < innerticks + 32);
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

/* Draw all the selections of all the units. */

draw_selections(map)
Map *map;
{
	int sx, sy, sx2, sy2, i;
	GrafPtr oldport;
	Rect tmprect;
	RgnHandle tmprgn;
	Unit *unit;

 	GetPort(&oldport);
	SetPort(map->window);
	tmprgn = NewRgn();
	GetClip(tmprgn);
	ClipRect(&(map->contentrect));
	for (i = 0; i < map->numselections; ++i) {
		unit = map->selections[i];
		draw_selected_unit(map, unit);
	}
	SetClip(tmprgn);
	DisposeRgn(tmprgn);
	SetPort(oldport);
}

/* Draw all the selected units in the given cell. */

draw_selections_at(map, x, y)
Map *map;
int x, y;
{
	int i;
	GrafPtr oldport;
	RgnHandle tmprgn;
	Unit *unit;

 	GetPort(&oldport);
	SetPort(map->window);
	tmprgn = NewRgn();
	GetClip(tmprgn);
	ClipRect(&(map->contentrect));
	for (i = 0; i < map->numselections; ++i) {
		unit = map->selections[i];
		if (unit && unit->x == x && unit->y == y) {
			draw_selected_unit(map, unit);
		}
	}
	SetClip(tmprgn);
	DisposeRgn(tmprgn);
	SetPort(oldport);
}

draw_selected_unit_setport(map, unit)
Map *map;
Unit *unit;
{
	int i;
	GrafPtr oldport;
	RgnHandle tmprgn;

 	GetPort(&oldport);
	SetPort(map->window);
	tmprgn = NewRgn();
	GetClip(tmprgn);
	ClipRect(&(map->contentrect));
	draw_selected_unit(map, unit);
	SetClip(tmprgn);
	DisposeRgn(tmprgn);
	SetPort(oldport);
}

/* Draw a single selected unit on the given map.  Assumes that grafport already set. */

draw_selected_unit(map, unit)
Map *map;
Unit *unit;
{
	int sx, sy, sw, sh, size, wholecell = FALSE, drawmag = FALSE;
	int sx1, sy1, sw1, sh1;
	Rect tmprect;

	if (!in_play(unit)) return; /* unselect it too? */
	if (map->uw >= 32) {
		xform_unit_self(map, unit, &sx, &sy, &sw, &sh);
		if (map->numselections == 1
			&& sw < 16
			&& unit->transport != NULL) {
			wholecell = TRUE;
			drawmag = TRUE;
			sx1 = sx;  sy1 = sy;  sw1 = sw;  sh1 = sh;
		}
	} else {
		wholecell = TRUE;
	}
	if (wholecell) {
		xform(map, unit->x, unit->y, &sx, &sy);
		/* Adjust to unit part of cell. */
		sx += (map->hw - map->uw) / 2;  sy += (map->hh - map->uh) / 2;
		sw = map->uw;  sh = map->uh;
	}
	if (0 /* not actually within visible area */) return;
	if (map->drawplans
		&& unit->plan
		&& unit->plan->tasks) {
		int sx2, sy2;
		Task *task = unit->plan->tasks, *nexttask;

		if (task != NULL) {
			if ((nexttask = task->next) != NULL) {
				switch (nexttask->type) {
					case MOVETO_TASK:
					case HIT_UNIT_TASK:
						xform(map, nexttask->args[0], nexttask->args[1], &sx2, &sy2);
						PenPat(QD(dkGray));
						MoveTo(sx + sw/2, sy + sh/2);
						LineTo(sx2 + map->hw/2, sy2 + map->hh/2);
						PenNormal();
				}
			}
			switch (task->type) {
				case MOVETO_TASK:
				case HIT_UNIT_TASK:
					xform(map, task->args[0], task->args[1], &sx2, &sy2);
					PenPat(QD(dkGray));
					MoveTo(sx + sw/2, sy + sh/2);
					LineTo(sx2 + map->hw/2, sy2 + map->hh/2);
					PenNormal();
			}
		}
	}
	/* Draw magnification lines pointing to the true location of the unit. */
	if (drawmag) {
		/* PenPat should already be black. */
		MoveTo(sx,      sy);       LineTo(sx1,       sy1);
		MoveTo(sx + sw, sy);       LineTo(sx1 + sw1, sy1);
		MoveTo(sx,      sy + sh);  LineTo(sx1,       sy1 + sh1);
		MoveTo(sx + sw, sy + sh);  LineTo(sx1 + sw1, sy1 + sh1);
	}
	/* Be sure the selected unit is drawn. */
	draw_unit_image(map->window, sx, sy, sw, sh,
					unit->type, side_number(unit->side), !completed(unit));
	/* Draw a highlighting rectangle. */
	SetRect(&tmprect, sx, sy, sx + sw, sy + sh);
	/* A hack to prevent leakage into the grid. */
	if (map->drawgrid && map->power == 5) --tmprect.bottom;
	/* First, draw an outer white frame, for contrast. */
	PenPat(QD(white));
	FrameRect(&tmprect);
	InsetRect(&tmprect, 1, 1);
	/* Black is for units that can still act, dark gray for actors, gray if the
	   unit can't do anything. */
	PenPat((unit->act && unit->act->initacp > 0) ?
			((unit->act->acp > 0) ? QD(black) : QD(dkGray)) : QD(gray));
	/* Wide border if awake, narrow if asleep or napping. */
	size = ((unit->plan && (unit->plan->asleep || unit->plan->reserve)) ? 1 : 2);
	PenSize(size, size);
	FrameRect(&tmprect);
	PenNormal();
	DGprintf("draw selection of %s at %d,%d\n", unit_desig(unit));
}

/* (should only redraw any given cell once) */

erase_selections(map)
Map *map;
{
	int sx, sy, sx2, sy2, i;
	GrafPtr oldport;
	RgnHandle tmprgn;
	Unit *unit;

 	GetPort(&oldport);
	SetPort(map->window);
	tmprgn = NewRgn();
	GetClip(tmprgn);
	ClipRect(&(map->contentrect));
	for (i = 0; i < map->numselections; ++i) {
		unit = map->selections[i];
		draw_unselected_unit(map, unit);
	}
	SetClip(tmprgn);
	DisposeRgn(tmprgn);
	SetPort(oldport);
}

erase_selection(map, unit)
Map *map;
Unit *unit;
{
	GrafPtr oldport;
	RgnHandle tmprgn;

 	GetPort(&oldport);
	SetPort(map->window);
	tmprgn = NewRgn();
	GetClip(tmprgn);
	ClipRect(&(map->contentrect));
	draw_unselected_unit(map, unit);
	SetClip(tmprgn);
	DisposeRgn(tmprgn);
	SetPort(oldport);
}

draw_unselected_unit(map, unit)
Map *map;
Unit *unit;
{
	if (!in_play(unit)) return;
	draw_row(map, unit->x, unit->y, 1, TRUE);
	DGprintf("erase selection of %s at %d,%d\n", unit_desig(unit));
}

force_map_update(map)
Map *map;
{
	force_update(map->window);
}

/* Remove and destroy the map object. */

destroy_map(map)
Map *map;
{
	Map *map2;
	
	if (maplist == map) {
		maplist = map->next;
	} else {
		for_all_maps(map2) {
			if (map2->next == map) {
				map2->next = map->next;
			}
		}
	}
	/* (should destroy substructs) */
	free(map);
}

/* (should be able to mention borders and conns also - share code with curses?) */

oneliner(map, sx, sy)
Map *map;
int sx, sy;
{
	int x, y, t2, uview, u, s, ps = NOBODY, dep, sayin = FALSE;
	char *peopdesc = NULL, *sidedesc, *namedesc;
	char descbuf[80];
	Unit *unit;
	Side *side;
	char *mplayer_at_desig();

	if (!nearest_cell(map, sx, sy, &x, &y)) {
		strcpy(tmpbuf, "(nothing)");
		return;
	} else if (terrain_visible(x, y)) {
		strcpy(tmpbuf, " ");
		/* Describe the side of the people here. */
		if (people_sides_defined()) {
			ps = people_side_at(x, y);
			if (ps != NOBODY) {
				side = side_n(ps);
				if (side == NULL) {
					peopdesc = "indep";
				} else if (side == dside) {
					peopdesc = "your";
				} else {
					peopdesc = side_adjective(side);
					if (peopdesc[0] == '\0') {
						sprintf(descbuf, "s%d", side->id);
						peopdesc = descbuf;
					}
				}
			}
		}
		if (units_visible(x, y)) {
			nearest_unit(map, sx, sy, &unit);
			if (unit != NULL) {
				if (unit->side != dside) {
					sidedesc = side_adjective(unit->side);
					if (ps != NOBODY && ps == side_number(unit->side)) {
						peopdesc = "own";
					}
				} else {
					sidedesc = "your";
				}
				strcat(tmpbuf, sidedesc);
				if (unit->name) {
					strcat(tmpbuf, " ");
					strcat(tmpbuf, u_type_name(unit->type));
					strcat(tmpbuf, " ");
					strcat(tmpbuf, unit->name);
				} else if (unit->number > 0) {
					tprintf(tmpbuf, " %d%s %s",
							unit->number, ordinal_suffix(unit->number), u_type_name(unit->type));
				} else {
					strcat(tmpbuf, " ");
					strcat(tmpbuf, u_type_name(unit->type));
				}
				if (Debug || DebugG || DebugM) {
					tprintf(tmpbuf, " #%d", unit->id);
				}
				sayin = TRUE;
			}
		} else {
			if ((uview = unit_view(dside, x, y)) != EMPTY) {
				u = vtype(uview);  s = vside(uview);
				if (ps != NOBODY && ps == s) {
					peopdesc = "own";
				}
				strcat(tmpbuf, side_adjective(side_n(s)));
				strcat(tmpbuf, " ");
				strcat(tmpbuf, u_type_name(u));
				sayin = TRUE;
			}
		}
		if (sayin) {
			strcat(tmpbuf, " (in ");
		}
		if (peopdesc != NULL) {
			strcat(tmpbuf, peopdesc);
			strcat(tmpbuf, " ");
		}
		strcat(tmpbuf, t_type_name(terrain_at(x, y)));
		if (sayin) {
			strcat(tmpbuf, ")");
		}
		if (elevations_defined()) {
			tprintf(tmpbuf, " Elev %d", elev_at(x, y));
		}
		if (temperatures_defined()) {
			tprintf(tmpbuf, " T %d�", temperature_at(x, y));
		}
		if (numcoattypes > 0) {
			for_all_terrain_types(t2) {
				if (t_is_coating(t2)
					&& aux_terrain_defined(t2)
					&& ((dep = aux_terrain_at(x, y, t2)) > 0)) {
					tprintf(tmpbuf, " %s %d", t_type_name(t2), dep);
				}
			}
		}
	} else {
		sprintf(tmpbuf, "(unknown)");
	}
	tprintf(tmpbuf, " @%d,%d", x, y);
	if (map->drawai && side_has_ai(dside)) {
		strcat(tmpbuf, " ");
		strcat(tmpbuf, mplayer_at_desig(dside, x, y));
	}
}

activate_map(map, activate)
Map *map;
int activate;
{
	Rect growRect;

	if (activate) {
		HiliteControl(map->vscrollbar, 0);
		HiliteControl(map->hscrollbar, 0);
#if 0
		/* the controls must be redrawn on activation: */
		(*(map->vscrollbar))->contrlVis = 255;
		(*(map->hscrollbar))->contrlVis = 255;
		InvalRect(&(*(map->vscrollbar))->contrlRect);
		InvalRect(&(*(map->hscrollbar))->contrlRect);
#endif
		/* The growbox needs to be redrawn on activation. */
		growRect = map->window->portRect;
		/* adjust for the scrollbars */
		growRect.top = growRect.bottom - sbarwid;
		growRect.left = growRect.right - sbarwid;
		InvalRect(&growRect);
	} else {
		/* The scrollbars must be hidden on deactivation. */
		HiliteControl(map->vscrollbar, 255);
		HiliteControl(map->hscrollbar, 255);
/*		HideControl(map->vscrollbar);
		HideControl(map->hscrollbar); */
		/* The growbox should be changed immediately on deactivation. */
		DrawGrowIcon(map->window);
	}
}

print_map(map)
Map *map;
{
/*	TPPrPort printport;
	extern THPrint printrecordhandle;

	printport = PrOpenDoc(printrecordhandle, nil, nil);
	PrCloseDoc(printport); */
}
