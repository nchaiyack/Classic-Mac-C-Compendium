/* Copyright (c) 1992, 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Map interaction for the Mac interface to Xconq. */

#include "conq.h"
#include "mac.h"

/* This is a temporary used by procs. */

Map *curmap = NULL;

/* This scroll proc is shared by both the horizontal and vertical scrollbars. */

pascal void
map_scroll_proc(control, code)
ControlHandle control;
short code;
{
	int curvalue, pagesize, jump;

	if (curmap == NULL) return;
	/* The page jump should be most but not all of a screenful. */
	if (control == curmap->hscrollbar) {
		pagesize = (2 * curmap->pxw) / 3;
	} else {
		pagesize = (2 * curmap->pxh) / 3;
	}
	switch (code) {
		case inPageDown:
			jump = pagesize;
			break;
		case inDownButton:
			jump = 4;
			break;
		case inPageUp:
			jump = 0 - pagesize;
			break;
		case inUpButton:
			jump = -4;
			break;
		default:
			jump = 0;
			break;
	}
	/* Letting control's code do the max/min hacking. */
	SetCtlValue(control, GetCtlValue(control) + jump);
	curvalue = GetCtlValue(control);
	/* Tweak the map's own variables to match. */
	if (control == curmap->hscrollbar) {
		curmap->sx = curvalue;
	} else {
		curmap->sy = curvalue;
	}
	curmap = NULL;
}

/* Handle a mouse down in the map window. */

do_mouse_down_map(map, mouse, mods)
Map *map;
Point mouse;
int mods;
{
	ControlHandle control;
	short part, value;
	long oldsx, oldsy;
	WindowPtr window = map->window;

	part = FindControl(mouse, window, &control);
	if (control == map->hscrollbar) {
		/* Handle the horizontal scrollbar. */
		oldsx = map->sx;
		part = FindControl(mouse, window, &control);
		switch (part) {
			case inThumb:
				/* (should add tracking ability) */
				part = TrackControl(control, mouse, NULL);
				map->sx = GetCtlValue(control);
				break;
			default:
				curmap = map;
				part = TrackControl(control, mouse, (ProcPtr) map_scroll_proc);
				break;
		}
		if (oldsx != map->sx) {
			focus_on_center(map);
			force_map_update(map);
			draw_related_maps(map);
		}
	} else if (control == map->vscrollbar) {
		/* Handle the vertical scrollbar. */
		oldsy = map->sy;
		part = FindControl(mouse, window, &control);
		switch (part) {
			case inThumb:
				part = TrackControl(control, mouse, NULL);
				map->sy = GetCtlValue(control);
				break;
			default:
				curmap = map;
				part = TrackControl(control, mouse, (ProcPtr) map_scroll_proc);
				break;
		}
		if (oldsy != map->sy) {
			focus_on_center(map);
			force_map_update(map);
			draw_related_maps(map);
		}
	} else if (mouse.h <= conwid) {
		/* Interpret as a control panel hit. */
		do_mouse_down_map_control_panel(map, mouse.h, mouse.v, mods);
	} else {
		do_mouse_down_map_content(map, mouse.h, mouse.v, mods);
	}
}

do_mouse_down_map_control_panel(map, h, v, mods)
Map *map;
int h, v, mods;
{
	int i, winh = map->window->portRect.bottom - map->window->portRect.top;
	Unit *unit;

	/* (should better organize tests here) */
	if (between(winh - 2 * sbarwid, v, winh)) {
		switch ((winh - v) / sbarwid) {
			case 0:
				magnify_map(map, ((h < conwid / 2) ? -1 : 1));
				break;
			case 1:
				modaltool = 1;
				break;
		}
	} else if (v < 32) {
		toggle_survey(map);
	} else if ((v - 32) < 5 * 15) {
		switch ((v - 32) / 15) {
			case 0:
				if (h < conwid / 2) {
					select_previous_awake_mover(map);
				} else {
					select_next_awake_mover(map);
				}
				break;
			case 1:
				if (h < conwid / 2) {
					select_previous_mover(map);
				} else {
					select_next_mover(map);
				}
				break;
			case 2:
				if (h < conwid / 2) {
					select_previous_actor(map);
				} else {
					select_next_actor(map);
				}
				break;
			case 3:
				if (h < conwid / 2) {
					select_previous_unit(map);
				} else {
					select_next_unit(map);
				}
				break;
			case 4:
				SysBeep(20);
				break;
		}
	} else if (v - 32 - 5*15 - 2 - 5/*why?*/ < 5 * 11) {
		switch ((v - 32 - 5*15 - 2 - 5/*why?*/) / 11) {
			case 0:
				toggle_map_grid(map);
				break;
			case 1:
				toggle_map_names(map);
				break;
			case 2:
				if (people_sides_defined()) {
					toggle_map_people(map);
				}
				break;
			case 3:
				toggle_map_plans(map);
				break;
			case 4:
				toggle_map_ai(map);
				break;
		}
	} else if (mayseeall) {
		weseeall = !weseeall;
		force_map_update(map);
	}
}

toggle_survey(map)
Map *map;
{
	int i;
	Unit *unit;

	map->moveonclick = !map->moveonclick;
	map->autoselect = !map->autoselect;
	draw_control_panel(map);
	if (map->autoselect) {
		if (map->numselections > 0) {
			for (i = 0; i < map->numselections; ++i) {
				if ((unit = map->selections[i]) != NULL) {
					map->curunit = autonext_unit(dside, unit);
					select_exactly_one_unit(map, map->curunit);
				}
			}
		}
	}
}

magnify_map(map, inout)
Map *map;
int inout;
{
	set_map_mag(map, map->power + inout);
}

/* This sets the map's magnification directly and updates it. */

set_map_mag(map, newpower)
Map *map;
int newpower;
{
	newpower = clip_to_limits(0, newpower, NUMPOWERS-1);
	if (map->power != newpower) {
		set_map_power(map, newpower);
		center_on_focus(map);
		/* Doubling these calcs is really crude... */
		set_map_scrollbars(map);
		set_map_viewport(map);
		set_map_scrollbars(map);
		set_map_viewport(map);
		force_map_update(map);
		draw_related_maps(map);
	}
}

toggle_map_grid(map)
Map *map;
{
	map->drawgrid = !map->drawgrid;
	/* (should not do a total redraw?) */
	force_map_update(map);
}

toggle_map_topline(map)
Map *map;
{
	Rect tmprect;
	GrafPtr oldport;
	RgnHandle tmprgn;

	map->toph = (map->toph ? 0 : tophgt);
	set_content_rect(map);
	if (map->toph) {
 		GetPort(&oldport);
		SetPort(map->window);
#if 0
		tmprect = map->window->portRect;
		tmprect.left += conwid;
		tmprect.right -= sbarwid;  tmprect.bottom -= sbarwid;
/*		ScrollRect(&tmprect, 0, map->toph, tmprgn); */
		/* (should draw the topline here now) */
#endif
		SetPort(oldport);
		force_map_update(map);
	} else {
		force_map_update(map);
	}
}

toggle_map_other_maps(map)
Map *map;
{
	map->drawothermaps = !map->drawothermaps;
	/* (should not do a total redraw) */
	force_map_update(map);
}

toggle_map_lighting(map)
Map *map;
{
	map->drawlighting = !map->drawlighting;
	/* We have to do a total redraw. */
	force_map_update(map);
}

toggle_map_names(map)
Map *map;
{
	map->drawnames = !map->drawnames;
	/* (if now on, should draw names on top of everything, don't redraw everything) */
	if (map->hh > 5) {
		force_map_update(map);
	} else {
		/* (should be a force update on control panel alone) */
		draw_control_panel(map);
	}
}

toggle_map_people(map)
Map *map;
{
	map->drawpeople = !map->drawpeople;
	if (bwid2[map->power] > 0) {
		force_map_update(map);
	} else {
		/* (should be a force update on control panel alone) */
		draw_control_panel(map);
	}
}

toggle_map_elevations(map)
Map *map;
{
	map->drawelevations = !map->drawelevations;
	force_map_update(map);
}

toggle_map_materials(map, m)
Map *map;
int m;
{
	map->drawmaterials[m] = !map->drawmaterials[m];
	map->nummaterialstodraw += (map->drawmaterials[m] ? 1 : -1);
	force_map_update(map);
}

toggle_map_temperature(map, m)
Map *map;
int m;
{
	map->drawtemperature = !map->drawtemperature;
	force_map_update(map);
}

toggle_map_winds(map, m)
Map *map;
int m;
{
	map->drawwinds = !map->drawwinds;
	force_map_update(map);
}

toggle_map_clouds(map, m)
Map *map;
int m;
{
	map->drawclouds = !map->drawclouds;
	force_map_update(map);
}

toggle_map_storms(map, m)
Map *map;
int m;
{
	map->drawstorms = !map->drawstorms;
	force_map_update(map);
}

toggle_map_plans(map)
Map *map;
{
	map->drawplans = !map->drawplans;
	if (map->numselections > 0) {
		force_map_update(map);
	} else {
		/* (should be a force update on control panel alone) */
		draw_control_panel(map);
	}
}

toggle_map_ai(map)
Map *map;
{
	if (!side_has_ai(dside)) return;
	map->drawai = !map->drawai;
	force_map_update(map);
}

static int selrect;
static int downx, downy, downdir;

do_mouse_down_map_content(map, h, v, mods)
Map *map;
int h, v, mods;
{
	int i;
	Unit *unit;
	
	/* Remember this hex. */
	nearest_cell(map, h, v, &downx, &downy);
	/* Assume that last place clicked is a reasonable focus. */
	if (inside_area(downx, downy)) {
		map->vcx = downx;  map->vcy = downy;
	}
	if (modaltool > 0) {
		switch (modaltool) {
			case 1:
				select_area_and_zoom(map, h, v, mods);
				break;
			case 2:
				if (do_fire_command()) {
					modaltool = 0;
				}
				break;
			default:
				/* (should error out) */
				break;
		}
	} else if (mods & cmdKey) {
		if (map->moveonclick && map->autoselect) {
			unselect_all(map);
			nearest_unit(map, h, v, &unit);
			if (unit != NULL && side_controls_unit(dside, unit)) {
				/* The nearest unit will become the "current unit". */
				map->curunit = unit;
				select_unit_on_map(map, unit);
				draw_selections(map);
				move_on_drag(map, unit, mods);
			} else {
				select_all_dragged_over(map, h, v, mods);
				/* Pick the first of the multiple selection as the "current unit". */
				if (map->numselections > 0) {
					map->curunit = map->selections[0];
				}
			}
		} else {
			for (i = 0; i < map->numselections; ++i) {
				if ((unit = map->selections[i]) != NULL) {
					move_the_selected_unit(map, unit, h, v);
				}
			}
		}
	} else if (mods & optionKey) {
		for (i = 0; i < map->numselections; ++i) {
			if ((unit = map->selections[i]) != NULL) {
				fire_the_selected_unit(map, unit, h, v);
			}
		}
	} else if (mods & shiftKey) {
		nearest_unit(map, h, v, &unit);
		if (unit && side_sees_unit(dside, unit)) {
			/* Invert the selection status of the unit. */
			if (unit_is_selected(map, unit)) {
				unselect_unit_on_map(map, unit);
				erase_selection(map, unit);
			} else {
				select_unit_on_map(map, unit);
				draw_selections_at(map, unit->x, unit->y);
			}
		} else {
			select_all_dragged_over(map, h, v, mods);
		}
	} else {
		/* Interpret an unmodified mouse down. */
#ifdef DESIGNERS
		if (dside->designer && tooltype != notool) {
			apply_designer_tool(map, h, v, mods);
		} else
#endif /* DESIGNERS */
		if (map->moveonclick && !dside->designer) {
			/* Usually will only be one to move, but be general anyway. */
			for (i = 0; i < map->numselections; ++i) {
				if ((unit = map->selections[i]) != NULL) {
					move_the_selected_unit(map, unit, h, v);
				}
			}
		} else {
			unselect_all(map);
			nearest_unit(map, h, v, &unit);
			if (unit != NULL && side_controls_unit(dside, unit)) {
				select_unit_on_map(map, unit);
				draw_selections(map);
				move_on_drag(map, unit, mods);
			} else {
				select_all_dragged_over(map, h, v, mods);
			}
		}
	}
}

select_all_dragged_over(map, h0, v0, mods)
Map *map;
int h0, v0, mods;
{
	Point pt0, pt1, newmouse;
	int h1, v1, drawn = FALSE, x, y;
	Rect tmprect;

	SetPt(&pt0, h0, v0);
	SetPt(&pt1, h0, v0);
	SetRect(&tmprect, h0, v0, h0, v0);
	/* (should be a generic subr?) */
	PenMode(patXor);
	PenPat(qd.gray);
	while (WaitMouseUp()) {
		GetMouse(&newmouse);
		if (!EqualPt(pt1, newmouse) /* && PtInRect(newmouse, &(map->window->portRect)) */) {
			if (drawn) {
				tmprect.left = min(pt0.h, pt1.h);  tmprect.top = min(pt0.v, pt1.v);
				tmprect.right = max(pt0.h, pt1.h);  tmprect.bottom = max(pt0.v, pt1.v);
				FrameRect(&tmprect);
			}
			pt1 = newmouse;
			tmprect.left = min(pt0.h, pt1.h);  tmprect.top = min(pt0.v, pt1.v);
			tmprect.right = max(pt0.h, pt1.h);  tmprect.bottom = max(pt0.v, pt1.v);
			FrameRect(&tmprect);
			drawn = TRUE;
		}
	}
	if (drawn) {
		tmprect.left = min(pt0.h, pt1.h);  tmprect.top = min(pt0.v, pt1.v);
		tmprect.right = max(pt0.h, pt1.h);  tmprect.bottom = max(pt0.v, pt1.v);
		FrameRect(&tmprect);
	}
	PenNormal();
	select_all_units_in_rect(map, &tmprect);
}

select_area_and_zoom(map, h0, v0, mods)
Map *map;
int h0, v0, mods;
{
	Point pt0, pt1, newmouse;
	int drawn = FALSE, x, y;
	Rect tmprect;

	SetPt(&pt0, h0, v0);
	SetPt(&pt1, h0, v0);
	/* (should be a generic subr) */
	PenMode(patXor);
	PenPat(qd.gray);
	while (WaitMouseUp()) {
		GetMouse(&newmouse);
		if (!EqualPt(pt1, newmouse) /* && PtInRect(newmouse, &(map->window->portRect)) */) {
			if (drawn) {
				tmprect.left = min(pt0.h, pt1.h);  tmprect.top = min(pt0.v, pt1.v);
				tmprect.right = max(pt0.h, pt1.h);  tmprect.bottom = max(pt0.v, pt1.v);
				FrameRect(&tmprect);
			}
			pt1 = newmouse;
			tmprect.left = min(pt0.h, pt1.h);  tmprect.top = min(pt0.v, pt1.v);
			tmprect.right = max(pt0.h, pt1.h);  tmprect.bottom = max(pt0.v, pt1.v);
			FrameRect(&tmprect);
			drawn = TRUE;
		}
	}
	if (drawn) {
		tmprect.left = min(pt0.h, pt1.h);  tmprect.top = min(pt0.v, pt1.v);
		tmprect.right = max(pt0.h, pt1.h);  tmprect.bottom = max(pt0.v, pt1.v);
		FrameRect(&tmprect);
	}
	PenNormal();
	/* Done being modal. */
	modaltool = 0;
	nearest_cell(map, pt1.h, pt1.v, &x, &y);
	if (x != downx && y != downy) {
		magnify_to_fit(map, downx, downy, x, y);
	}
}

move_on_drag(map, unit, mods)
Map *map;
Unit *unit;
int mods;
{
	int sx, sy, sw, sh, h0, v0, drawn = FALSE, x, y;
	Point pt0, pt1, newmouse;

	xform_unit_self(map, unit, &sx, &sy, &sw, &sh);
	h0 = sx + sw / 2;  v0 = sy + sh / 2;
	SetPt(&pt0, h0, v0);
	SetPt(&pt1, h0, v0);
	/* (should be a generic subr?) */
	PenMode(patXor);
	while (WaitMouseUp()) {
		GetMouse(&newmouse);
		/* should scroll, then abort if we drag outside the window */
		if (0 /* PtInRect(newmouse, &(map->window->portRect)) */) {
		}
		if (!EqualPt(pt1, newmouse)) {
			if (drawn) {
				MoveTo(h0, v0);  LineTo(pt1.h, pt1.v);
			}
			pt1 = newmouse;
			MoveTo(h0, v0);  LineTo(pt1.h, pt1.v);
			drawn = TRUE;
		}
	}
	/* Erase the last drawn line. */
	if (drawn) {
		MoveTo(h0, v0);  LineTo(pt1.h, pt1.v);
	}
	PenNormal();
	nearest_cell(map, pt1.h, pt1.v, &x, &y);
	if (x != downx || y != downy) {
		move_the_selected_unit(map, unit, pt1.h, pt1.v);
	} else {
		/* (should try to enter another unit in this cell) */
	}
}

unselect_all(map)
Map *map;
{
	int num = map->numselections;

	if (map->numselections > 0) {
		erase_selections(map);
		map->numselections = 0;
	}
}

/* Add the given unit to the array of units selected in the given map.  If we need
   more space, then grow the array by 50%. */

select_unit_on_map(map, unit)
Map *map;
Unit *unit;
{
	if (map->numselections >= map->maxselections) {
		int newsize = map->maxselections + map->maxselections / 2;
		Unit **newarray = (Unit **) realloc(map->selections, newsize * sizeof(Unit *));

		if (newarray == NULL) {
			run_warning("couldn't realloc map selection array");
			return;
		}
		map->maxselections = newsize;
		map->selections = newarray;
	}
	map->selections[map->numselections++] = unit;
}

unit_is_selected(map, unit)
Map *map;
Unit *unit;
{
	int i;

	for (i = 0; i < map->numselections; ++i) {
		if (map->selections[i] == unit) return TRUE;
	}
	return FALSE;
}

unselect_unit_on_map(map, unit)
Map *map;
Unit *unit;
{
	int i, j;

	for (i = 0; i < map->numselections; ++i) {
		if (map->selections[i] == unit) {
			/* Keep selection list contiguous, move other units down. */
			for (j = i + 1; j < map->numselections; ++j) {
				map->selections[j - 1] = map->selections[j];
			}
			--map->numselections;
			return;
		}
	}
}

erase_map_selection(map)
Map *map;
{
}

/* Given a map and a rectangle in it, select all the units whose images touch on
   that rectangle. */

select_all_units_in_rect(map, rectptr)
Map *map;
Rect *rectptr;
{
	int rectissmall = FALSE;
	int sx, sy, sw, sh;
	Side *side;
	Unit *unit;
	Rect unitrect, tmprect;
	
	/* First see if we're selecting over a large area or within a single cell. */
	if (rectptr->right - rectptr->left < map->hw
		&& rectptr->bottom - rectptr->top < map->hh) rectissmall = TRUE;
	/* Now look at all the plausible units and see if any's image intersects the rect. */
	for_all_units(side, unit) {
		if (in_play(unit)
			&& side_controls_unit(dside, unit)
			&& (rectissmall || unit->transport == NULL)) {
			xform_unit_self(map, unit, &sx, &sy, &sw, &sh);
			SetRect(&unitrect, sx, sy, sx + sw, sy + sh);
			if (SectRect(&unitrect, rectptr, &tmprect)) {
				select_unit_on_map(map, unit);
				/* (could do setport etc once...) */
				draw_selected_unit_setport(map, unit);
			}
		}
	}
}

/* This translates the user's "go to here" into appropriate tasks and/or actions. */

move_the_selected_unit(map, unit, h, v)
Map *map;
Unit *unit;
int h, v;
{
	int x, y;
	Unit *other;

	nearest_cell(map, h, v, &x, &y);
#ifdef DESIGNERS
	/* Designers use this function to push units around, bound only by the
	   limits on occupancy. */
	if (dside->designer) {
		nearest_unit(map, h, v, &other);
		if (other != NULL && can_occupy(unit, other)) {
			/* Teleport into a transport. */
			leave_hex(unit);
			enter_transport(unit, other);
		} else if (can_occupy_cell(unit, x, y)) {
			/* Teleport the unit into the designated cell. */
			leave_hex(unit);
			enter_hex(unit, x, y);
		} else {
			SysBeep(20);
		}
		return;
	}
#endif /* DESIGNERS */
	if (x != unit->x || y != unit->y) {
		if (unit->act && unit->plan) { /* (should be more sophisticated test?) */
			if (distance(unit->x, unit->y, x, y) == 1) {
				if (unit_at(x, y) != NULL) {
					/* There are units at our desired destination. */
					nearest_unit(map, h, v, &other);
					if (other == NULL) {
						if (can_occupy_cell(unit, x, y)
						    && valid(check_move_action(unit, unit, x, y, unit->z))) {
							prep_move_action(unit, unit, x, y, unit->z);
						} else {
							SysBeep(20);
						}
					}
					if (other->side == unit->side /* (should be "trusted side") */) {
						/* One of ours, maybe get on it. */
						if (can_occupy(unit, other)) {
							if (valid(check_enter_action(unit, unit, other))) {
								prep_enter_action(unit, unit, other);
							} else {
								/* (should schedule for next turn?) */
							}
						} else if (can_occupy(other, unit)) {
							/* Have other unit do an enter action, then move. */
							/* (not quite right, move should happen after other unit
								is actually inside, in case it fills dest) */
							prep_enter_action(other, other, unit);
							order_moveto(unit, x, y);
						} else if (other->transport != NULL
								   && can_occupy(unit, other->transport)) {
							if (valid(check_enter_action(unit, unit, other->transport))) {
								prep_enter_action(unit, unit, other->transport);
							} else {
								/* (should schedule for next turn?) */
							}
						} else if (other->transport != NULL
								   && other->transport->transport != NULL
								   && can_occupy(unit, other->transport->transport)) {
							/* two levels up should be sufficient */
							if (valid(check_enter_action(unit, unit, other->transport->transport))) {
								prep_enter_action(unit, unit, other->transport->transport);
							} else {
								/* (should schedule for next turn?) */
							}
						} else if (can_occupy_cell(unit, x, y)
						    && valid(check_move_action(unit, unit, x, y, unit->z))) {
							prep_move_action(unit, unit, x, y, unit->z);
						} else {
							SysBeep(20);
						}
					} else {
						/* Somebody else's unit, try to victimize it in various ways,
						   trying coexistence only as a last resort :-) */
						if (valid(check_capture_action(unit, unit, other))) {
							prep_capture_action(unit, unit, other);
						} else if (valid(check_attack_action(unit, unit, other, 100))) {
							prep_attack_action(unit, unit, other, 100);
						} else if (valid(check_overrun_action(unit, unit, x, y, unit->z, 100))) {
							prep_overrun_action(unit, unit, x, y, unit->z, 100);
						} else if (valid(check_fire_at_action(unit, unit, other, -1))) {
							prep_fire_at_action(unit, unit, other, -1);
						} else if (valid(check_detonate_action(unit, unit, x, y, unit->z))) {
							prep_detonate_action(unit, unit, x, y, unit->z);
						} else if (valid(check_move_action(unit, unit, x, y, unit->z))) {
							prep_move_action(unit, unit, x, y, unit->z);
						} else {
							SysBeep(20);
						}
					}
				} else {
					/* Cell is empty, try to move into it directly. */
					if (can_occupy_cell(unit, x, y)
						&& valid(check_move_action(unit, unit, x, y, unit->z))) {
						prep_move_action(unit, unit, x, y, unit->z);
					} else {
						SysBeep(20);
					}
				}
			} else {
				/* We're not adjacent to the destination, set up a move task. */
				DGprintf("Ordering %s to move to %d,%d\n", unit_desig(unit), x, y);
				order_moveto(unit, x, y);
			}
		} else {
			/* ??? can't act ??? */
		}
	} else {
		/* Destination is in this cell, try to do an enter action. */
		nearest_unit(map, h, v, &other);
		if (other != NULL) {
		    if (valid(check_enter_action(unit, unit, other))) {
				prep_enter_action(unit, unit, other);
			} else {
				SysBeep(20);
			}
		} else {
			/* This is a no-op, don't say anything. */
		}
	}
}

fire_the_selected_unit(map, unit, h, v)
Map *map;
Unit *unit;
int h, v;
{
	int x, y;
	Unit *other;

	nearest_cell(map, h, v, &x, &y);
	if (x != unit->x || y != unit->y) {
		if (unit->act && unit->plan) { /* (should be more sophisticated test?) */
			if ((other = unit_at(x, y)) != NULL) {
				/* There's a unit to fire at. */
				if (other->side == unit->side) {
					SysBeep(20);
				} else {
					prep_fire_at_action(unit, unit, other, -1);
				}
			} else {
				SysBeep(20);
			}
		}
	}
}

select_exactly_one_unit(map, unit)
Map *map;
Unit *unit;
{
    Unit *thisunit;

	if (map->numselections > 0) {		
		thisunit = map->selections[0];
		if (thisunit == unit) return;
	}
	unselect_all(map);
	select_unit_on_map(map, unit);
	scroll_to_unit(map, unit);
	draw_selections(map);
}

select_next_unit(map)
Map *map;
{
	select_another(map, find_next_unit);
}

select_previous_unit(map)
Map *map;
{
	select_another(map, find_prev_unit);
}

select_next_actor(map)
Map *map;
{
	select_another(map, find_next_actor);
}

select_previous_actor(map)
Map *map;
{
	select_another(map, find_prev_actor);
}

Unit *
find_next_mover(side, prevunit)
Side *side;
Unit *prevunit;
{
	Unit *unit = NULL;

	if (side != NULL) {
		if (prevunit == NULL) prevunit = side->unithead;
		for (unit = prevunit->next; unit != prevunit; unit = unit->next) {
			if (is_unit(unit) && unit->id > 0
				&& alive(unit)
				&& inside_area(unit->x, unit->y)
				&& (unit->act && unit->act->acp > 0)) {
				return unit;
			}
		} 
	}
	return NULL;
}

Unit *
find_prev_mover(side, nextunit)
Side *side;
Unit *nextunit;
{
    Unit *unit = NULL;

    if (side != NULL) {
	if (nextunit == NULL) nextunit = side->unithead;
	for (unit = nextunit->prev; unit != nextunit; unit = unit->prev) {
	    if (is_unit(unit) && unit->id > 0
		&& alive(unit)
		&& inside_area(unit->x, unit->y)
		&& (unit->act && unit->act->acp > 0)) {
		return unit;
	    }
	} 
    }
    return NULL;
}

select_next_mover(map)
Map *map;
{
	select_another(map, find_next_mover);
}

select_previous_mover(map)
Map *map;
{
	select_another(map, find_prev_mover);
}

Unit *
find_next_awake_mover(side, prevunit)
Side *side;
Unit *prevunit;
{
	Unit *unit = NULL;

	if (side != NULL) {
		if (prevunit == NULL) prevunit = side->unithead;
		for (unit = prevunit->next; unit != prevunit; unit = unit->next) {
			if (is_unit(unit) && unit->id > 0
				&& alive(unit)
				&& inside_area(unit->x, unit->y)
				&& (unit->act && unit->act->acp > 0)
				&& (unit->plan && !unit->plan->asleep && !unit->plan->reserve)) {
				return unit;
			}
		} 
	}
	return NULL;
}

Unit *
find_prev_awake_mover(side, nextunit)
Side *side;
Unit *nextunit;
{
    Unit *unit = NULL;

    if (side != NULL) {
	if (nextunit == NULL) nextunit = side->unithead;
	for (unit = nextunit->prev; unit != nextunit; unit = unit->prev) {
	    if (is_unit(unit) && unit->id > 0
		&& alive(unit)
		&& inside_area(unit->x, unit->y)
				&& (unit->act && unit->act->acp > 0)
				&& (unit->plan && !unit->plan->asleep && !unit->plan->reserve)) {
		return unit;
	    }
	} 
    }
    return NULL;
}

select_next_awake_mover(map)
Map *map;
{
	select_another(map, find_next_awake_mover);
}

select_previous_awake_mover(map)
Map *map;
{
	select_another(map, find_prev_awake_mover);
}

/* Given a map and a searching function, go find the "next" matching unit and select it. */

select_another(map, fn)
Map *map;
Unit *(*fn)();
{
    Unit *thisunit, *nextunit;

	if (fn == NULL) {
		SysBeep(20);
		return;
	}
	if (map->numselections > 0) {
		thisunit = map->selections[0];
	} else {
		thisunit = NULL;
	}
	nextunit = (*fn)(dside, thisunit);
	if (nextunit != NULL) {
		unselect_all(map);
		select_unit_on_map(map, nextunit);
		scroll_to_unit(map, nextunit);
		draw_selections(map);
		if (map->autoselect) {
			map->curunit = nextunit;
		}
	} else if (thisunit != NULL) {
		scroll_to_unit(map, thisunit);
		/* (should not be done this way, but how else?) */
		if (map->autoselect
			&& (thisunit->act && thisunit->act->acp > 0)
			&& (thisunit->plan && !thisunit->plan->asleep && !thisunit->plan->reserve)) {
			map->curunit = thisunit;
		}
	}
}

/* Scroll the given map over to display the given unit. */

scroll_to_unit(map, unit)
Map *map;
Unit *unit;
{
	if (inside_area(unit->x, unit->y)) {
		if (!in_middle(map, unit->x, unit->y)) {
			map->vcx = unit->x;  map->vcy = unit->y;
			center_on_focus(map);
			set_map_scrollbars(map);
			force_map_update(map);
		}
	}
}

/* This routine changes a map's viewport and magnification to fit the given rectangle. */

magnify_to_fit(map, x1, y1, x2, y2)
Map *map;
int x1, y1, x2, y2;
{
	int wid, hgt, wanted, power;

	DGprintf("Magnifying map to fit in area %d,%d - %d,%d\n", x1, y1, x2, y2);
	/* (still need to do y/2 correction) */
	wid = abs(x2 - x1) + 1;  hgt = abs(y2 - y1) + 1;
	map->vcx = min(x1, x2) + wid / 2;  map->vcy = min(y1, y2) + hgt / 2;
	/* Compute the "ideal" size of a displayed cell. */
	wanted = min(map->pxw / wid, map->pxh / hgt);
	/* Search for the best approximation. */
	for (power = NUMPOWERS-1; power > 0; --power) {
		if (hws[power] < wanted) break;
	}
	set_map_mag(map, power);
}
