/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* This file contains all code that manages units. */

#include "conq.h"

void leave_hex_aux();

/* This is not a limit, just sets initial allocation of unit objects
   and how many additional to get if more are needed.  Should be tuned
   for the system. */

#ifndef INITMAXUNITS
#define INITMAXUNITS 200
#endif

/* The list of available units. */

Unit *freeunits;

/* A scratch global. */

Unit *tmpunit;

/* Buffer for descriptions of units. */

#define NUMSHORTBUFS 3

int curshortbuf = 0;

char *shortbufs[NUMSHORTBUFS] = { NULL, NULL, NULL };

char utypenamen[100];

char *actorstatebuf = NULL;

/* Total number of units in existence. */

int numunits = 0;

/* The next number to use for a unit id. */

int nextid = 1;            /* next number to be used for ids */

/* buffer for remembering occupant death */

int *occdeath = NULL;

/* Have units died since dead unit lists made */

int recent_dead_flushed = TRUE;

int totunits;
int totutypes[MAXUTYPES];

/* Init gets a first block of units, and sets up the "independent side" list,
   since newly created units will appear on it. */

void
init_units()
{
    extern Task *freetasks;

    allocate_unit_block();
    if (freetasks == NULL) init_tasks();  /* hack */
    init_side_unithead(indepside);
}

/* Grab another block of unit objects to work with. */

allocate_unit_block()
{
    int i;
    Unit *unitblock = (Unit *) xmalloc(INITMAXUNITS * sizeof(Unit));

    for (i = 0; i < INITMAXUNITS; ++i) {
	unitblock[i].id = -1;
	unitblock[i].next = &unitblock[i+1];
    }
    unitblock[INITMAXUNITS-1].next = NULL;
    freeunits = unitblock;
    Dprintf("Allocated space for %d units.\n", INITMAXUNITS);
    return TRUE;
}

/* The primitive unit creator, called by regular creator and also used to
   make the dummy units that sit at the heads of the per-side unit lists. */

Unit *
create_bare_unit(type)
int type;
{
    Unit *newunit;

    /* If our free list is empty, go and get some more units. */
    if (freeunits == NULL) {
	allocate_unit_block();
    }
    /* Take the first unit off the free list. */
    newunit = freeunits;
    freeunits = freeunits->next;
    /* Give it a valid type... */
    newunit->type = type;
    /* ...but an invalid id. */
    newunit->id = -1;
    return newunit;
}

/* The regular unit creation routine.  All the slots should have something
   reasonable in them, since individual units objects see a lot of reuse. */

Unit *
create_unit(type, makebrains)
int type, makebrains;
{
    int i, r;
    Unit *newunit;

    /* Test whether we've hit any designer-specified limits. */
    if ((u_type_in_game_max(type) >= 0
	 && totutypes[type] >= u_type_in_game_max(type))
	|| (g_units_in_game_max() >= 0
	    && totunits >= g_units_in_game_max())) {
	return NULL;
    }
    /* Allocate the unit object.  Xconq will fail instead of returning null. */
    newunit = create_bare_unit(type);
    /* Init all the slots to distinguishable values.  The unit is not
       necessarily newly allocated, so we have to hit all of its slots. */
    newunit->id = nextid++;
    newunit->name = NULL;
    /* Number == 0 means unit is unnumbered. */
    newunit->number = 0;
    newunit->x = newunit->y = -1;
    newunit->z = 0;  /* on the ground */
    /* Units default to being independent. */
    newunit->side = NULL;
    /* Create at max hp (caller should reduce if necessary). */
    newunit->hp = newunit->hp2 = u_hp(type);
    /* Create fully functional, let other routines set incompleteness. */
    newunit->cp = u_cp(type);
    /* Not in a transport. */
    newunit->transport = NULL;
#if (MAXMTYPES > 2)
    /* Note that the space never needs to be freed. */
    if (newunit->supply == NULL && nummtypes > 0) {
	newunit->supply = (short *) xmalloc(nummtypes * sizeof(short));
    }
#endif
    /* In any case, zero out all the supply values. */
    for_all_material_types(r) newunit->supply[r] = 0;
    /* Will allocate tooling state when actually needed. */
    newunit->tooling = NULL;
    /* Will allocate attitudes when actually needed. (?) */
    newunit->attitudes = NULL;
    if (makebrains) {
	init_unit_actorstate(newunit);
	init_unit_plan(newunit);
    } else {
	newunit->act = NULL;
	newunit->plan = NULL;
    }
    newunit->occupant = NULL;
    newunit->nexthere = NULL;
    /* Glue this unit into the list of independent units. */
    newunit->next = newunit;
    newunit->prev = newunit;
    insert_unit(indepside->unithead, newunit);
    /* Init more random slots. */
    newunit->prevx = newunit->prevy = -1;
    newunit->hook = lispnil;
    newunit->aihook = NULL;
    /* Add to the global unit count (is this really necessary?). */
    ++numunits;
    return newunit;
}

/* Alter the actorstate object to be correct for this unit. */

init_unit_actorstate(unit)
Unit *unit;
{
    if (u_acp(unit->type) > 0 && unit->cp > 0) {
	/* Might already have an actorstate, don't realloc if so. */
	if (unit->act == NULL) {
	    unit->act = (ActorState *) xmalloc(sizeof(ActorState));
	}
	/* Flag the action as undefined. */
	unit->act->nextaction.type = A_NONE;
    } else {
	if (unit->act != NULL) free(unit->act);
	unit->act = NULL;
    }
}

/* Every unit that can act needs a plan object, types that can't act
   should have it cleared out. */

init_unit_plan(unit)
Unit *unit;
{
    if (u_acp(unit->type) > 0 && unit->cp > 0) {
	/* Might already have a plan, so don't realloc. */
	if (unit->plan == NULL) {
	    unit->plan = (Plan *) xmalloc(sizeof(Plan));
	}
	/* Put the plan into a default state, side will work it up later. */
	unit->plan->type = PLAN_NONE;
	unit->plan->creationtime = curturn;
	unit->plan->asleep = unit->plan->reserve = FALSE;
	clear_task_agenda(unit->plan);
	/* Allow AIs to make this unit do things. */
	unit->plan->aicontrol = TRUE;
    } else {
	/* Brainless units don't need anything, can free up plan. */
	if (unit->plan != NULL) free(unit->plan);
	unit->plan = NULL;
    }
}

/* A designer can call this to create an arbitrary unit during the game. */

Unit *
designer_create_unit(side, u, s, x, y)
Side *side;
int u, s, x, y;
{
    Unit *newunit;
    Side *side2;

    if ((newunit = create_unit(u, TRUE)) != NULL) {
	if (s != 0) {
	    side2 = side_n(s);
	    if (unit_allowed_on_side(newunit, side2)) {
	    	set_unit_side(newunit, side2);
	    	/* (should ensure that any changed counts are set correctly also) */
	    }
	}
	init_supply(newunit);
	enter_hex(newunit, x, y);
	see_exact(side, x, y);
	update_cell_display(side, x, y, TRUE);
	update_unit_display(side, newunit, TRUE);
	return newunit;
    } else {
	return NULL;
    }
}

/* Changing a unit's type has many effects. */

change_unit_type(unit, newtype)
Unit *unit;
int newtype;
{
    int oldtype = unit->type, oldhp = unit->hp;

    /* Don't do anything if we're "changing" to the same type. */
    if (oldtype == newtype) return;
    /* Do the actual change. */
    unit->type = newtype;
    /* Set the new hp to the same ratio of max as the unit had before.
       Caller can tweak to something else if necessary. */
    unit->hp = (oldhp * u_hp_max(newtype)) / u_hp_max(oldtype);
    /* Need to guarantee a positive value though. */
    if (unit->hp < 1) unit->hp = 1;
    /* (should modify side counts) */
    if (!type_allowed_on_side(newtype, unit->side)) {
    	if (type_allowed_on_side(newtype, NULL)) {
	    unit_changes_side(unit, NULL, -1, -1);
	} else {
	    run_warning("Leaving unit on disallowed side");
	}
    } else {
    	/* If unit didn't change sides, it will need a new number. */
    	assign_unit_number(unit);
    }
    init_unit_actorstate(unit);
    init_unit_plan(unit);
    unit->aihook = NULL;
}

max_builds(u)
int u;
{
    int u2;

    for_all_unit_types(u2) {
	if (could_make(u, u2)) return 1;
    }
    return 0;
}

/* A unit occupies a hex by adding itself to the list of occupants.
   It will not occupy a transport even if one is at this position
   (other code should have taken care of this case already)
   If something goes wrong, return false.  This routine is heavily used. */

enter_hex(unit, x, y)
Unit *unit;
int x, y;
{
    register int u = unit->type, ustack, u2stack;
    Unit *unit2, *topunit = unit_at(x, y), *prevunit = NULL, *nextunit = NULL;

#ifdef DEBUGGING
    /* Not necessarily an error, but indicative of bugs elsewhere. */
    if (unit->x >= 0 || unit->y >= 0) {
	run_warning("unit %d occupying hex (%d, %d), was at (%d %d)",
		    unit->id, x, y, unit->x, unit->y);
    }
#endif /* DEBUGGING */
    /* Always check this one, but not necessarily fatal. */
    if (!inside_area(x, y)) {
	run_warning("No cell at %d,%d, %s can't enter it",
		    x, y, unit_desig(unit));
	/* Let the unit remain off-world. */
	return FALSE;
    }
    if (!can_occupy_cell(unit, x, y)) {
	run_warning("Cell at %d,%d is too full for %s", x, y, unit_desig(unit));
	/* Let the unit remain off-world. */
	return FALSE;
    }
    if (topunit) {
    	/* Insert the entering unit into the stack at its correct position. */
    	ustack = u_stack_order(u);
    	for_all_stack(x, y, unit2) {
	    u2stack = u_stack_order(unit2->type);
	    if (ustack > u2stack
		|| (ustack == u2stack && unit->id < unit2->id)) {
		nextunit = unit2;
		if (unit2 == topunit) topunit = unit;
		break;
	    }
	    prevunit = unit2;
    	}
    	if (prevunit != NULL) prevunit->nexthere = unit;
    } else {
    	topunit = unit;
    }
    unit->nexthere = nextunit;
    set_unit_at(x, y, topunit);
    /* Set the location slots now. */
    enter_hex_aux(unit, x, y);
    /* Inevitable side-effect of appearing in the new location. */
    all_see_occupy(unit, x, y, TRUE);
    return TRUE;
}

/* Return true if the given unit can fit onto the given hex. */

/* (should eventually account for variable-size units) */

can_occupy_cell(unit, x, y)
Unit *unit;
int x, y;
{
    int u = unit->type, t = terrain_at(x, y), numthistype = 0, fullness = 0;
    int tcap, utcap;
    Unit *unit2;

    if (unit == NULL) run_error("null unit?");  /* should never happen */
    tcap = t_capacity(t);
    utcap = ut_capacity_x(u, t);
    if (tcap <= 0 && utcap <= 0) return FALSE;
    for_all_stack(x, y, unit2) {
	if (unit2->type == u) ++numthistype;
	fullness += ut_size(unit2->type, t);
    }
    /* Unit can be in this cell if there is dedicated space. */
    if (numthistype + 1 <= utcap) return TRUE;
    /* Otherwise decide on the basis of fullness. */
    return (fullness + ut_size(u, t) <= tcap);
}

type_can_occupy_cell(u, x, y)
int u, x, y;
{
    int t = terrain_at(x, y), numthistype = 0, fullness = 0;
    int tcap, utcap;
    Unit *unit2;

    tcap = t_capacity(t);
    utcap = ut_capacity_x(u, t);
    if (tcap <= 0 && utcap <= 0) return FALSE;
    for_all_stack(x, y, unit2) {
	if (unit2->type == u) ++numthistype;
	fullness += ut_size(unit2->type, t);
    }
    /* Unit can be in this cell if there is dedicated space. */
    if (numthistype + 1 <= utcap) return TRUE;
    /* Otherwise decide on the basis of fullness. */
    return (fullness + ut_size(u, t) <= tcap);
}

/* Recursive helper to update everybody's position.  This should be one of
   two routines that modify actual unit positions (leaving is the other). */

enter_hex_aux(unit, x, y)
Unit *unit;
int x, y;
{
    int u = unit->type, dir, x1, y1, tview;
    Unit *occ;
    Side *side = unit->side;

#ifdef DEBUGGING
    /* Not necessarily an error, but indicative of bugs elsewhere. */
    if (unit->x >= 0 || unit->y >= 0) {
	run_warning("unit %d occupying hex (%d, %d), was at (%d %d)",
		    unit->id, x, y, unit->x, unit->y);
    }
#endif /* DEBUGGING */
    /* Actually set the unit position. */
    unit->x = x;  unit->y = y;
    /* Increment viewing coverage, if unit is complete and can see out
       of its transport. */
    if (completed(unit)
    	&& (unit->transport == NULL
	    || uu_occ_can_see(u, unit->transport->type))) {
	cover_area(unit, x, y, 1);
	/* If vision range is 0, allow glimpses of adjacent cell terrain.
	   This applies to terrain only, adjacent units cannot be seen. */
	if (u_vision_range(u) == 0
	    && unit->transport == NULL
	    && !g_see_all()
	    && !g_terrain_seen()
	    && side != NULL) {
	    for_all_directions(dir) {
		if (point_in_dir(x, y, dir, &x1, &y1)) {
		    if (terrain_view(side, x1, y1) == UNSEEN) {
			set_terrain_view(side, x1, y1,
					 buildtview(terrain_at(x1, y1)));
			update_cell_display(side, x1, y1, TRUE);
		    }
		}
	    }
	}
    }
#if 0
    /* Increment special viewing coverage. */
    if (unit->transport == NULL
	|| uu_sp_occ_can_see(unit->type, unit->transport->type)) {
	sp_cover_area(unit, x, y, 1);
    }
#endif
    /* Do for all the occupants too, recursively. */
    for_all_occupants(unit, occ) {
	enter_hex_aux(occ, x, y);
    }
}

/* Decide whether the given unit can actually be in the given transport. */

/* (this still needs to account for multipart units) */

can_occupy(unit, transport)
Unit *transport, *unit;
{
    return can_carry(transport, unit);
}

can_carry(transport, unit)
Unit *transport, *unit;
{
    int u = unit->type, u2 = transport->type;
    int numthistype = 0, numalltypes = 0, occvolume = 0;
    int ucap, uucap;
    Unit *occ;

    /* Intercept nonsensical arguments. */
    if (transport == unit) return FALSE;
/*    if (!completed(transport)) return FALSE;  */
    if (unit->occupant != NULL && !uu_occ_can_have_occs(u, u2)) return FALSE;
    ucap = u_capacity(u2);
    uucap = uu_capacity_x(u2, u);
    if (ucap <= 0 && uucap <= 0) return FALSE;
    /* Compute the transport's fullness. */
    for_all_occupants(transport, occ) {
	if (occ->type == u) ++numthistype;
	++numalltypes;
	occvolume += uu_size(occ->type, u2);
    }
    /* Can carry if dedicated space available. */
    if (numthistype + 1 <= uucap) return TRUE;
    /* Check upper limit on count of occupants. */
    if (uu_occ_max(u2, u) >= 0
        && numthistype + 1 - uucap > uu_occ_max(u2, u)) return FALSE;
    if (u_occ_total_max(u2) >= 0
        && numalltypes + 1 > u_occ_total_max(u2)) return FALSE;
    /* Can carry if general unit hold has room. */
    return (occvolume + uu_size(u, u2) <= ucap);
}

type_can_occupy(u, transport)
int u;
Unit *transport;
{
    int u2 = transport->type;
    int numthistype = 0, numalltypes = 0, occvolume = 0;
    int ucap, uucap;
    Unit *occ;

    if (!completed(transport)) return FALSE;
    ucap = u_capacity(u2);
    uucap = uu_capacity_x(u2, u);
    if (ucap <= 0 && uucap <= 0) return FALSE;
    /* Compute the transport's fullness. */
    for_all_occupants(transport, occ) {
	if (occ->type == u) ++numthistype;
	++numalltypes;
	occvolume += uu_size(occ->type, u2);
    }
    /* Can carry if dedicated space available. */
    if (numthistype + 1 <= uucap) return TRUE;
    if (uu_occ_max(u2, u) >= 0
        && numthistype + 1 - uucap > uu_occ_max(u2, u))
      return FALSE;
    if (u_occ_total_max(u2) >= 0
        && numalltypes + 1 > u_occ_total_max(u2)) return FALSE;
    /* Can carry if general unit hold has room. */
    return (occvolume + uu_size(u, u2) <= ucap);
}

can_occupy_type(unit, u2)
Unit *unit;
int u2;
{
    int u = unit->type;

    /* Can occupy if nonzero reserved capacity for this type. */
    if (uu_capacity_x(u2, u) > 0) return TRUE;
    /* Can occupy if general unit hold has room for at least one unit. */
    return (uu_size(u, u2) <= u_capacity(u2));
}

can_carry_type(transport, u)
Unit *transport;
int u;
{
    return type_can_occupy(u, transport);
}

/* Units become occupants by linking into the transport's occupant list. */

void
enter_transport(unit, transport)
Unit *unit, *transport;
{
    int u = unit->type, ustack, u2stack;
    Unit *unit2, *topunit = transport->occupant;
    Unit *prevunit = NULL, *nextunit = NULL;

    if (unit == transport) {
    	run_error("Unit is trying to enter itself");
    }
    if (topunit) {
    	/* Insert the entering unit into the occupant list at
	   its correct position. */
    	ustack = u_stack_order(u);
    	for_all_occupants(transport, unit2) {
	    u2stack = u_stack_order(unit2->type);
	    if (ustack > u2stack
		|| (ustack == u2stack && unit->id < unit2->id)) {
		nextunit = unit2;
		if (unit2 == topunit) topunit = unit;
		break;
	    }
	    prevunit = unit2;
    	}
    	if (prevunit != NULL) prevunit->nexthere = unit;
    } else {
    	topunit = unit;
    }
    unit->nexthere = nextunit;
    transport->occupant = topunit;
    /* Point from the unit back to its transport. */
    unit->transport = transport;
    /* Set the passenger's coords to match the transport's. */
    enter_hex_aux(unit, transport->x, transport->y);
    /* Others might be watching. */
    all_see_occupy(unit, transport->x, transport->y, FALSE);
}

/* Unit departs from a hex by zeroing out pointer if in hex or by being
   removed from the list of transport occupants. */

/* Dead units (hp = 0) may be run through here, so don't error out. */

void
leave_hex(unit)
Unit *unit;
{
    int ux = unit->x, uy = unit->y;
    Unit *transport = unit->transport, *other;

    if (ux < 0 || uy < 0) {
	/* Sometimes called twice */
    } else if (transport != NULL) {
	leave_transport(unit);
	leave_hex_aux(unit);
	all_see_leave(unit, ux, uy, FALSE);
	/* not all_see_hex here because can't see inside transports */
	update_unit_display(transport->side, transport, TRUE);
    } else {
	/* Unsplice ourselves from the list of units in this hex. */
	if (unit == unit_at(ux, uy)) {
	    set_unit_at(ux, uy, unit->nexthere);
	} else {
	    for_all_stack(ux, uy, other) {
		if (unit == other->nexthere) {
		    other->nexthere = other->nexthere->nexthere;
		    break;
		}
	   } 
	}
	/* Bash this now-spurious link. */
	unit->nexthere = NULL;
	/* Now bash the coords. */
	leave_hex_aux(unit);
	/* Now let everybody observe that the unit is gone. */
	all_see_leave(unit, ux, uy, TRUE);
    }
}

/* When leaving, remove view coverage, record old position, and then
   trash the old coordinates just in case.  Catches many bugs.  Do
   this for all the occupants as well. */

void
leave_hex_aux(unit)
Unit *unit;
{
    Unit *occ;

    if (unit->x < 0 && unit->y < 0) {
	run_warning("unit has already left the cell");
    }
    /* Decrement viewing coverage around our about-to-be-old location. */
    if (completed(unit)
        && (unit->transport == NULL
	   || uu_occ_can_see(unit->type, unit->transport->type))) {
	cover_area(unit, unit->x, unit->y, -1);
    }
    /* Stash the old coords. */
    unit->prevx = unit->x; unit->prevy = unit->y;
    /* Set to a recognizable value. */
    unit->x = -1;  unit->y = -1;
    /* Make any occupants leave too. */
    for_all_occupants(unit, occ) {
	leave_hex_aux(occ);
    }
}

/* Disembarking unlinks from the list of passengers only, leaves the unit
   hanging in limbo, so should have it occupy something immediately. */

void
leave_transport(unit)
Unit *unit;
{
    Unit *transport = unit->transport, *occ;

    if (unit == transport) {
    	run_error("Unit is trying to leave itself");
    }
    if (unit == transport->occupant) {
	transport->occupant = unit->nexthere;
    } else {
	for_all_occupants(transport, occ) {
	    if (unit == occ->nexthere) {
		occ->nexthere = occ->nexthere->nexthere;
		break;
	    }
	}
    }
    /* Bash the now-spurious link. */
    unit->transport = NULL;
}

/* Given an overfull unit, spew out occupants until back within limits. */

eject_excess_occupants(unit)
Unit *unit;
{
    int u, u2 = unit->type, overfull = TRUE, count;
    int numalltypes = 0, occvolume = 0;
    int numeachtype[MAXUTYPES], sharedeachtype[MAXUTYPES];
    Unit *occ;

    for_all_unit_types(u) numeachtype[u] = sharedeachtype[u] = 0;
    /* Eject occupants overflowing counts in shared space. */
    for_all_occupants(unit, occ) ++numeachtype[occ->type];
    for_all_unit_types(u) {
    	if (numeachtype[u] > uu_capacity_x(u2, u)) {
	    sharedeachtype[u] = numeachtype[u] - uu_capacity_x(u2, u);
	    if (uu_occ_max(u2, u) >= 0
	    	&& sharedeachtype[u] > uu_occ_max(u2, u)) {
	    		count = sharedeachtype[u] - uu_occ_max(u2, u);
	    		while (count > 0) {
			    	for_all_occupants(unit, occ) {
		    			if (occ->type == u) {
		    				eject_occupant(unit, occ);
		    				--count;
		    				break;
		    			}
		    		}
		    	}
	    }
	}
    }
    /* Eject occupants over the total max count allowed. */
    for_all_occupants(unit, occ) ++numalltypes;
    if (u_occ_total_max(u2) >= 0 && numalltypes > u_occ_total_max(u2)) {
    	count = numalltypes - u_occ_total_max(u2);
    	while (unit->occupant != NULL) {
		eject_occupant(unit, unit->occupant);
		if (--count <= 0) break;
    	}
    }
    /* Eject occupants overflowing volume of shared space. */
    while (overfull) {
	for_all_unit_types(u) numeachtype[u] = 0;
	occvolume = 0;
	for_all_occupants(unit, occ) ++numeachtype[occ->type];
	for_all_unit_types(u) {
	    occvolume +=
	    	 max(0, numeachtype[u] - uu_capacity_x(u2, u)) * uu_size(u, u2);
	}
	if (occvolume > u_capacity(u2)) {
	    overfull = TRUE;
	    eject_occupant(unit, unit->occupant);
	} else {
	    overfull = FALSE;
	}
    }
}

/* Given that an occupant must leave its transport, decide what happens; either
   move out into the open, into another unit, or vanish. */

/* (should be generic test) */
#define ut_dies_on(u, t) (ut_vanishes_on(u,t) || ut_wrecks_on(u, t))

eject_occupant(unit, occ)
Unit *unit, *occ;
{
    if (!in_play(unit) || !in_play(occ)) return;
    /* If the occupant is mobile and the current cell has room, let it escape
       but be stacked in the transport's cell. */
    if (mobile(occ->type)
        && !ut_dies_on(occ->type, terrain_at(unit->x, unit->y))
        && can_occupy_cell(occ, unit->x, unit->y)) {
        leave_hex(occ);
        enter_hex(occ, unit->x, unit->y);
        return;
    }
    /* (should let occupants escape into other units in cell) */
    /* (should let occupants with acp escape into adj cells) */
    /* Evaporating the occupant is our last option. */
    kill_unit(occ, -1);
}

/* Handle the general situation of a unit changing allegiance from one side
   to another.  This is a common internal routine, so no messages here. */

unit_changes_side(unit, newside, reason1, reason2)
Unit *unit;
Side *newside;
int reason1, reason2;
{
    int ux = unit->x, uy = unit->y;
    Side *oldside = unit->side;
    Unit *occ;

    if (oldside == newside) return;
    if (!unit_allowed_on_side(unit, newside)) {
	if (reason1 == H_UNIT_CAPTURED) kill_unit(unit, -1);
	return;
    }
    /* Give captured units some additional moves maybe. */
#if 0
    if (reason1 == CAPTURE)
      unit->movesleft = (period_capturemoves() ? compute_move(unit) : 0);
#endif
    /* Remove unit from former side's inventory and counts. */
    if (oldside != NULL) {
/*	oldside->units[unit->type]--;  */
/*	if (reason2 >= 0) side_balance(oldside, unit->type, reason2)++; */
	update_unit_display(oldside, unit, TRUE);
	/* for machine players */
/*	if (reason2 == PRISONER) register_loss_to_area(oldside, unit, 2);  */
    }
    /* Add unit to new side's inventory and counts. */
    if (newside != NULL) {
/*	newside->units[unit->type]++;  */
/*	    if (reason1 >= 0) side_balance(newside, unit->type, reason1)++; */
    }
    /* (Should this be switchable maybe?) */
    for_all_occupants(unit, occ) {
	unit_changes_side(occ, newside, reason1, reason2);
    }
    /* Adjust view coverage.  The sequencing here is to make sure that no
       viewing coverage gets left on or off inadvertantly. */
    if (alive(unit) && inside_area(ux, uy)) {
	/* Uncover the current viewed area. */
	cover_area(unit, ux, uy, -1);
	/* Actually set the side slot of the unit here. */
	set_unit_side(unit, newside);
	/* Cover it for the new side now. */
	cover_area(unit, ux, uy, 1);
    }
    /* Reflect the changeover in any appropriate displays. */
    if (oldside != NULL) {
	update_unit_display(oldside, unit, TRUE); /* a second update? */
    }
    if (newside != NULL) {
	update_unit_display(newside, unit, TRUE);
    }
}

/* This is a general test as to whether the given unit can be
   on the given side. */

unit_allowed_on_side(unit, side)
Unit *unit;
Side *side;
{
    if (unit == NULL) return FALSE;
    /* (allow for tests on individual units also?) */
    return type_allowed_on_side(unit->type, side);
}

test_class_membership(leaf)
Obj *leaf;
{
    char *sclass;

    if (stringp(leaf)) {
	sclass = c_string(leaf);
	if (tmpside != NULL) {
	    if (empty_string(tmpside->sideclass)) return FALSE;
	    return (strcmp(sclass, tmpside->sideclass) == 0);
	} else {
	    return (strcmp(sclass, "independent") == 0);
	}
    } else {
	init_warning("testing side against garbled class expression");
	/* Be permissive if continued. */
	return TRUE;
    }
}

type_allowed_on_side(u, side)
int u;
Side *side;
{
    Obj *sclass = u_possible_sides(u);

    /* (could optimize by precomputing bit vectors) */
    tmputype = u;
    tmpside = side;
    return eval_boolean_expression(sclass, test_class_membership, TRUE);
}

/* Put the given unit on the given side, without all the fancy effects.
   Important to handle independents, because this gets called during init.
   This is the only way that a unit's side may be altered. */

/* Note that this may be run on dead units, as part of clearing out a
   side's units, in which case we just want to relink, don't care about
   testing whether the type is allowed or not. */
 
set_unit_side(unit, side)
Unit *unit;
Side *side;
{
    if (unit->side != side) {
	if (alive(unit) && !unit_allowed_on_side(unit, side)) return FALSE;
	unit->side = side;
	/* Make sure this unit is off anybody else's list. */
	delete_unit(unit);
	insert_unit((side ? side->unithead : indepside->unithead), unit);
    }
    return TRUE;
}

set_unit_plan_type(side, unit, type)
Side *side;
Unit *unit;
int type;
{
    int oldtype;

    if (unit->plan) {
	oldtype = unit->plan->type;
	if (type != oldtype) {
	    unit->plan->type = type;
	    update_unit_display(side, unit, TRUE);
	}
    }
}

set_unit_asleep(side, unit, flag, recurse)
Side *side;
Unit *unit;
int flag, recurse;
{
    int oldflag;
    Unit *occ;

    if (unit->plan) {
	oldflag = unit->plan->asleep;
	if (flag != oldflag) {
	    unit->plan->asleep = flag;
	    if (side != NULL) update_unit_display(side, unit, TRUE);
	}
    }
    if (recurse) {
    	for_all_occupants(unit, occ) {
	    set_unit_asleep(side, occ, flag, recurse);
    	}
    }
}

set_unit_reserve(side, unit, flag, recurse)
Side *side;
Unit *unit;
int flag, recurse;
{
    int oldflag;
    Unit *occ;

    if (unit->plan) {
	oldflag = unit->plan->reserve;
	if (flag != oldflag) {
	    unit->plan->reserve = flag;
	    if (side != NULL) update_unit_display(side, unit, TRUE);
	}
    }
    if (recurse) {
    	for_all_occupants(unit, occ) {
	    set_unit_reserve(side, occ, flag, recurse);
    	}
    }
}

set_unit_ai_control(side, unit, flag, recurse)
Side *side;
Unit *unit;
int flag, recurse;
{
    int oldflag;
    Unit *occ;

    if (unit->plan) {
	oldflag = unit->plan->aicontrol;
	if (flag != oldflag) {
	    unit->plan->aicontrol = flag;
	    if (side != NULL) update_unit_display(side, unit, TRUE);
	}
    }
    if (recurse) {
    	for_all_occupants(unit, occ) {
	    set_unit_ai_control(side, occ, flag, recurse);
    	}
    }
}

set_unit_name(side, unit, newname)
Side *side;
Unit *unit;
char *newname;
{
    /* Always turn 0-length names into NULL. */
    if (newname != NULL && strlen(newname) == 0) newname = NULL;
    unit->name = newname;
    update_unit_display(side, unit, TRUE);
    update_unit_display(unit->side, unit, TRUE);
    /* (should also send to any other side directly viewing this unit!) */
}

/* Remove a unit from play.  This is different from making it available for
   reallocation - only the unit flusher can do that.  We remove all the
   passengers too, recursively.  Sometimes units are "killed twice", so
   be sure not to run all this twice.  Also count up occupant deaths, being
   sure not to count the unit itself as an occupant. */

kill_unit(unit, reason)
Unit *unit;
int reason;
{
    int u = unit->type, u2, selfdied = FALSE;

    if (alive(unit)) {
	if (unit->side && unit->side->selfunit == unit) selfdied = TRUE;
	if (unit->occupant != NULL) {
	    if (occdeath == NULL) {
		occdeath = (int *) xmalloc(numutypes * sizeof(int));
	    }
	    for_all_unit_types(u2) occdeath[u2] = 0;
	    /* The recursive routine will count this unit's death, so start it
	       out down by one. */
	    occdeath[u] = -1;
	}
	leave_hex(unit);
	kill_unit_aux(unit, reason);
	if (selfdied) {
	    if (u_self_resurrects(u)) {
		/* should find and designate a new self unit */
		return;
	    } else {
		/* Make sure this doesn't have serious consequences? */
		/* (actually, side might have won??  should just take out
		   of game permanently) */
		side_loses(unit->side, NULL);
		/* (can't do all consequences just yet?) */
	    }
	}
	recent_dead_flushed = FALSE;
    }
}

/* Trash it now - occupant doesn't need to leave_hex.  Also record the
   event, and update the apropriate display.  The unit here should
   be known to be alive. */

kill_unit_aux(unit, reason)
Unit *unit;
int reason;
{
    int u = unit->type;
    Unit *occ;
    Side *side = unit->side;
    
    unit->hp = 0;
    if (occdeath != NULL) occdeath[u]++;
    /* dispose_of_plan(unit);  for now... */
    record_unit_loss(unit, reason);
    if (side != NULL) {
	/* --(side->units[u]);  */
	if (reason >= 0) {
	    /* side_balance(side, u, reason)++; */
  	}
    	react_to_unit_loss(side, unit, reason);
	update_unit_display(side, unit, TRUE);
    }
    /* Kill all the occupants in turn. */
    for_all_occupants(unit, occ) {
	if (alive(occ)) kill_unit_aux(occ, reason);
    }
}

/* Give strategists a chance to react. */

react_to_unit_loss(side, unit, reason)
Side *side;
Unit *unit;
int reason;
{
    if (side_has_ai(side)) {
	ai_react_to_unit_loss(side, unit);
    }
}

/* Do something about the plan object, possibly passing to a deputy unit,
   or maybe just destroying it entirely. */

dispose_of_plan(unit)
Unit *unit;
{
    free_plan(unit->plan);
    unit->plan = NULL;
}

/* Get rid of all dead units at once.  It's important to get rid of all
   of the dead units, so they don't come back and haunt us.
   (This routine is basically a garbage collector, and should not be called
   during a unit list traversal.) The process starts by finding the first
   live unit, making it the head, then linking around all in the middle.
   Dead units stay on the dead unit list for each side until that side has had
   a chance to move.  Then they are finally flushed in a permanent fashion. */ 

void
flush_dead_units()
{
#ifdef REUSE_DEAD
    Unit *unit, *prevunit;
    Side *side;

    /* This never ends up flushing independent units, no big loss. */
    if (!recent_dead_flushed) {
	recent_dead_flushed = TRUE;
	for_all_units(side, unit) {
	    if (!alive(unit)) {
		prevunit = unit->prev;
		delete_unit(unit);
		put_unit_on_dead_list(unit);
		unit = prevunit;
	    }
	}
    }
#endif
}

/* Put dead units on sides dead unit lists. */

put_unit_on_dead_list(unit)
Unit *unit;
{
#ifdef REUSE_DEAD
    if (unit->side == NULL) {
	flush_one_unit(unit);
    } else {
	unit->next = unit->side->deadunits;
	unit->side->deadunits = unit;
    }
    --numunits;
#endif
}

/* Clean up dead units and put them back on free list. */

flush_side_dead(side)
Side *side;
{
#ifdef REUSE_DEAD
    Unit *unit, *next;
 
    next = side->deadunits;
    while (next != NULL) {
	unit = next;
	next = unit->next;
	flush_one_unit(unit);
    }
    side->deadunits = NULL;
#endif
}

/* Keep it clean - hit all links to other places.  Some might not be
   strictly necessary, but this is not an area to take chances with. */

flush_one_unit(unit)
Unit *unit;
{
    unit->id = -1;
    unit->occupant = NULL;
    unit->transport = NULL;
    unit->nexthere = NULL;
    /* Add it on the list of available units. */
    unit->next = freeunits;
    freeunits = unit;
}

/* Do multiple passes of bubble sort.  This is intended to improve locality
   among unit positions and reduce the amount of scrolling around.  It is
   not required for correct behavior anywhere (I think).
   Data is generally coherent, so bubble sort not too bad if we allow
   early termination when everything is already in order.  */

/* If slowness objectionable, replace with something clever, but be
   sure that average performance in real games is what's being improved. */

void
sort_units()
{
    int flips;
    int passes = 0;
    register Unit *unit, *nextunit;
    Side *side;

    for_all_sides(side) {
	passes = 0;
	flips = TRUE;
	while (flips) {
	    flips = FALSE;
	    for_all_side_units(side,unit) {
		if (unit->next != side->unithead
		    && !in_order(unit, unit->next)) {
		    flips = TRUE;
		    /* Reorder the units by fiddling with their links. */
		    nextunit = unit->next;
		    unit->prev->next = nextunit;
		    nextunit->next->prev = unit;
		    nextunit->prev = unit->prev;
		    unit->next = nextunit->next;
		    nextunit->next = unit;
		    unit->prev = nextunit;
		}
		++passes;
	    }
	}
    }
    Dprintf("Sorting passes = %d\n", passes);
}

in_order(unit1, unit2)
Unit *unit1, *unit2;
{
    int d1, d2;

    if (unit1->type < unit2->type) return TRUE;
    if (unit1->type > unit2->type) return FALSE;
    if (unit1->name && unit2->name == NULL) return TRUE;
    if (unit1->name == NULL && unit2->name) return FALSE;
    if (unit1->name && unit2->name) return strcmp(unit1->name, unit2->name) <= 0;
    if (unit1->number != unit2->number) return (unit1->number < unit2->number);
    if (unit1->y != unit2->y) return (unit1->y >= unit2->y);
    if (unit1->x != unit2->x) return (unit1->x <= unit2->x);
    /* Last gasp - ids impose a total ordering. */
    return (unit1->id < unit2->id);
}

/* Useful for the machine player to know how long it can move this
   piece before it should go home.  Assumes can't replenish from
   terrain.  Result may be negative, in which case it's time to go! */

moves_till_low_supplies(unit)
Unit *unit;
{
    int u = unit->type, r, moves = 12345, tmp;

    for_all_material_types(r) {
	if ((um_tomove(u, r) > 0)) {
	    tmp = (unit->supply[r] - um_storage(u, r) / 2) / um_tomove(u, r);
	    moves = min(moves, tmp);
	}
    }
    return moves;
}

/* Short, unreadable, but greppable listing of unit.  Primarily useful
   for debugging and warnings.  We use several buffers and rotate between
   them so we can call this more than once in a single printf. */

char *
unit_desig(unit)
Unit *unit;
{
    char *shortbuf;

    if (unit != NULL) {
	/* Allocate if not yet done so. */
    	if (shortbufs[curshortbuf] == NULL) {
	    shortbufs[curshortbuf] = xmalloc(BUFSIZE);
	}
	shortbuf = shortbufs[curshortbuf];
	curshortbuf = (curshortbuf + 1) % NUMSHORTBUFS;
	if (unit->id == -1) {
	    sprintf(shortbuf, "s%d head", side_number(unit->side));
	    return shortbuf;
	} else if (is_unit_type(unit->type)) {
	    sprintf(shortbuf, "s%d %-3.3s %d (%d,%d",
		    side_number(unit->side), u_type_name(unit->type),
		    unit->id, unit->x, unit->y);
	    if (unit->z != 0) {
		sprintf(shortbuf+strlen(shortbuf), ",%d", unit->z);
	    }
	    /* Yuck, should find a better way to do this */
	    if (((int) unit->transport) > 1000 /* != NULL */) {
		sprintf(shortbuf+strlen(shortbuf), ",in%d",
			unit->transport->id);
	    }
	    strcat(shortbuf, ")");  /* close out the unit location */
	    return shortbuf;
	} else {
	    return "!garbage unit!";
	}
    } else {
	return "no unit";
    }
}

/* Come up with a unit type name that fits in the given space. */

char *
utype_name_n(u, n)
int u, n;
{
    char *utypename, *shortname, *rawname;

    utypename = u_type_name(u);
    if (n <= 0 || strlen(utypename) <= n) {
	return utypename;
    } else if (!empty_string(u_short_name(u))) {
    	shortname = u_short_name(u);
	if (strlen(shortname) <= n) {
	    return shortname;
	} else {
	    rawname = shortname;
	}
    } else {
    	rawname = utypename;
    }
    /* Copy what will fit. */
    strncpy(utypenamen, rawname, n);
    utypenamen[n] = '\0';
    return utypenamen;
}

/* This formats an actorstate readably. */

char *
actorstate_desig(as)
ActorState *as;
{
	if (actorstatebuf == NULL) actorstatebuf = xmalloc(BUFSIZE);
    if (as != NULL) {
	sprintf(actorstatebuf, "acp %d/%d %s",
		as->acp, as->initacp, action_desig(&(as->nextaction)));
	return actorstatebuf;
    } else {
	return "no act";
    }
}

/* Search for a unit with the given id number. */

/* This is used a lot, it should be sped up. */

Unit *
find_unit(n)
int n;
{
    Unit *unit;
    Side *loopside;

    for_all_units(loopside, unit) {
	if (alive(unit) && unit->id == n) return unit;
    }
    return NULL;
}

Unit *
find_unit_by_name(nm)
char *nm;
{
    Unit *unit;
    Side *loopside;

	if (nm == NULL) return NULL;
    for_all_units(loopside, unit) {
	if (alive(unit) && unit->name != NULL && strcmp(unit->name, nm) == 0) return unit;
    }
    return NULL;
}

/* Given a name, find the type. */

find_unit_name(str)
char *str;
{
    int u;

    for_all_unit_types(u)
      if (strcmp(str, u_short_name(u)) == 0
	  || strcmp(str, u_type_name(u)) == 0) 
	return u;
    return NONUTYPE;
}

/* Reverse the order of the unit list.  This is needed since the
   loaded units from game modules are put into the list in reverse order. */

reverse_unit_order()
{
    Unit *next, *unit;
    Side *side;

    for_all_sides(side) {
	next = side->unithead->next;
	do {
	    unit = next;
	    next = unit->next;
	    unit->next = unit->prev;
	    unit->prev = next;
	} while (unit != side->unithead);
    }
}    

/* Return the first unit on the list. */

Unit *
first_unit (side)
Side *side;
{
    return side->unithead;
}

/* Insert the given unit after the other given unit. */

void
insert_unit(unithead, unit)
Unit *unithead, *unit;
{
    unit->next = unithead->next;
    unit->prev = unithead;
    unithead->next->prev = unit;
    unithead->next = unit;
}

/* Delete the unit from its list. */

void
delete_unit(unit)
Unit *unit;
{
    unit->next->prev = unit->prev;
    unit->prev->next = unit->next;
}

num_occupants(unit)
Unit *unit;
{
    int num = 0;
    Unit *occ;

    for_all_occupants(unit, occ) {
	num += 1;
    }
    return num;
}

num_units_at(x, y)
int x, y;
{
    int num = 0;
    Unit *unit;

    x = wrapx(x);
    if (!in_area(x, y)) {
	run_warning("num_units_at %d,%d??", x, y);
	return 0;
    }
    for_all_stack(x, y, unit) {
	num += 1;
    }
    return num;
}

/* Call this to doublecheck invariants on units. */

check_all_units()
{
    Unit *unit;
    Side *loopside;

    for_all_units(loopside, unit) {
	check_unit(unit);
    }
}

check_unit(unit)
Unit *unit;
{
    if (alive(unit) && unit->transport && !alive(unit->transport)) {
    	run_warning("%s is inside a dead transport", unit_desig(unit));
    }
    /* etc */
}

#ifdef DESIGNERS

/* Move a unit to a given location instantly, with all sides observing.
   This is for use by designers only! */

designer_teleport(unit, x, y)
Unit *unit;
int x, y;
{
    leave_hex(unit);
    enter_hex(unit, x, y);
    all_see_hex(x, y);
}

#endif /* DESIGNERS */

UnitVector *
make_unit_vector(initsize)
int initsize;
{
    UnitVector *vec;
	
    vec = (UnitVector *)
      xmalloc(sizeof(UnitVector) + initsize * sizeof(UnitVectorEntry));
    vec->size = initsize;
    vec->numunits = 0;
    return vec;
}

clear_unit_vector(vec)
UnitVector *vec;
{
    vec->numunits = 0;
}

UnitVector *
add_unit_to_vector(vec, unit, flag)
UnitVector *vec;
Unit *unit;
int flag;
{
    int i;
    UnitVector *newvec;

    /* (should search to see if already present) */
    if (vec->numunits >= vec->size) {
	newvec = make_unit_vector((3 * vec->size) / 2);
	newvec->numunits = vec->numunits;
	for (i = 0; i < vec->numunits; ++i) {
	    newvec->units[i] = vec->units[i];
	}
	vec = newvec;
    }
    ((vec->units)[vec->numunits]).unit = unit;
    ((vec->units)[vec->numunits]).flag = flag;
    ++(vec->numunits);
    return vec;
}

remove_unit_from_vector(vec, unit, pos)
UnitVector *vec;
Unit *unit;
int pos;
{
	int j;

	if (pos <= 0) {
		/* should search for unit in vector */
	}
	for (j = pos + 1; j < vec->numunits; ++j) {
		vec->units[j-1] = vec->units[j];
	}
	--(vec->numunits);
}

enum sortkeys tmpsortkeys[MAXSORTKEYS];

compare_units_by_keys(e1, e2)
UnitVectorEntry *e1, *e2;
{
    int i;
    Unit *unit1 = e1->unit, *unit2 = e2->unit;
    
    if (unit1 == unit2) return 0;
    if (unit1 == NULL) return 1;
    if (unit2 == NULL) return -1;
    for (i = 0; i < MAXSORTKEYS; ++i) {
	switch (tmpsortkeys[i]) {
	  case byside:
	    if (unit1->side != unit2->side) {
		int s1 = side_number(unit1->side);
		int s2 = side_number(unit2->side);
		
		/* Put independents at the end of any list. */
		if (s1 == 0) s1 = numsides + 1;
		if (s2 == 0) s2 = numsides + 1;
		return (s1 - s2);
	    }
	    break;
	  case bytype:
	    if (unit1->type != unit2->type) {
		return (unit1->type - unit2->type);
	    }
	    break;
	  case byname:
	    if (unit1->name) {
		if (unit2->name) {
		    return strcmp(unit1->name, unit2->name);
		} else {
		    return -1;
		}
	    } else if (unit1->number > 0) {
		if (unit2->name) {
		    return 1;
		} else if (unit2->number > 0) {
		    return (unit1->number - unit2->number);
		} else {
		    return -1;
		}
	    } else if (unit2->name) {
		return 1;
	    } else if (unit2->number > 0) {
		return 1;
	    }
	    break;
	  case byactorder:
	    /* (should sort by action priority?) */
	    break;
	  case bylocation:
	    if (unit1->y != unit2->y) {
		return (unit2->y - unit1->y);
	    } else if (unit1->x != unit2->x) {
		return (unit1->x - unit2->x);
	    } else {
		/* Both units are at the same location. Sort by transport. */
		if (unit1->transport) {
		    if (unit2->transport) {
		    } else {
			return 1;
		    }
		} else {
		    if (unit2->transport) {
			return -1;
		    } else {
		    }
		}
	    }
	    break;
	  case bynothing:
	    return (unit1->id - unit2->id);
	  default:
	    break;
	}
    }
    /* Unit ids are all unique, so this is a reliable default sort key. */
    return (unit1->id - unit2->id);
}

sort_unit_vector(vec)
UnitVector *vec;
{
    qsort(vec->units, vec->numunits, sizeof(UnitVectorEntry),
	  compare_units_by_keys);
}

