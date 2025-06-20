/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* This file is primarily mplayer calculations that concern the static aspects
   of a game, such as which unit types are better for which kinds of tasks. */

#include "conq.h"
#include "mplay.h"

#define DICE(N,NUMDICE,SPOTS,OFFSET)  \
  (((N) >> 14 == 0 || (N) >> 14 == 3) ?  \
   (NUMDICE = 0, SPOTS = 0, OFFSET = (N)) :  \
   (NUMDICE = ((N) >> 11) & 0x07, SPOTS = ((N) >> 7) & 0x0f, OFFSET = (N) & 0x7f))

int unit_count;

/* General collections of numbers used by all machine players. */

int can_move_in_dir[NUMDIRS];  /* directions it make sence to try to move in */

long bestworth = -10000, bestx, besty;

Unit *munit;                    /* Unit being decided about */

Side *mside;                    /* Side whose unit is being decided about */

/* General collections of numbers used by all machine players. */

int *localworth;                /* for evaluation of nearby hexes */

int base_building = FALSE;     /* can some unit build a base */

int worths_known = FALSE;  /* does the period file have portions info */

/* Init used by all machine players.  Precompute useful information
   relating to unit types in general, and that usually gets referenced
   in inner loops. */

mplayer_init_shared()
{
    int u, u1, u2, t, m1, numbuilders;
    Side *side;
    int tmp;
    
    /* Need 3 scratch layers for routefinding. */
    allocate_area_scratch(3);

    if (!g_see_all() && 1 /* terrain not all known */) {
	    /* Compute approx percentage occurrence of each terrain type */
	    /* in the area not yet seen */
	    /* (not useful if fractal gen not used) */
	    for_all_terrain_types(t) {
		set_t_fraction(t, ((t_alt_max(t) - t_alt_min(t)) *
			       (t_wet_max(t) - t_wet_min(t))) / 100);
	    }
    }
    /* Recognize unit types that are bases */
    for_all_unit_types(u1) {
	set_u_is_base(u1, FALSE);
	tmp = FALSE;
	for_all_material_types(m1) {
	    if (um_produce(u1, m1) > 0) {
		tmp = TRUE;
		break;
	    }
	}
	if (tmp) {
	    for_all_unit_types(u2) {
		if ((u1 != u2) && could_carry(u1,u2)) {
		    set_u_is_base(u1, TRUE);
		    continue;
		}
	    }
	}
    }
    /* Note that is_base_builder is set to the type of base that can */
    /* be built.  That means that unit zero can not be a base which */
    /* can be built. */
    for_all_unit_types(u1) {
	set_u_is_transport(u1, FALSE);
	set_u_is_carrier(u1, FALSE);
	set_u_is_base_builder(u1, FALSE);
	set_u_can_make(u1, FALSE);
	set_u_can_capture(u1, FALSE);
	numbuilders = 0;
/*	ave_build_time[u1] = 0;  */
	for_all_unit_types(u2) {
	    if (u_is_base(u2) &&
		could_make(u1, u2) &&
		1 /* can be made quickly? */) {
		set_u_is_base_builder(u1, u2);
		base_building = TRUE;
	    }
	    if (u_speed(u1) > 0 && could_carry(u1, u2)) {
		set_u_is_transport(u1, TRUE);
	    }
	    if (could_make(u2, u1)) {
		numbuilders++;
/*		ave_build_time[u1] += uu_make(u2,u1);  */
		set_u_can_make(u2, TRUE);
	    }
	    if (could_capture(u1, u2)) {
		set_u_can_capture(u1, TRUE);
	    }
	}
/*	if (numbuilders > 0)
	  ave_build_time[u1] /= numbuilders;  */
    }
    /* a carrier is a unit that is a mobile base, but that cannot
       move a passenger anywhere the passenger could not go itself. */
    for_all_unit_types(u1) {
	if (u_is_transport(u1)) {
	    set_u_is_carrier(u1, TRUE);
	    for_all_unit_types(u2) {
		if (could_carry(u1, u2)) {
		    for_all_terrain_types(t) {
			if (could_move(u1, t) && !could_move(u2, t))
			  set_u_is_carrier(u1, FALSE);
		    }
		}
	    }
	}
    }
    for_all_unit_types(u) {
	set_u_bw(u, basic_worth(u));
    }
    for_all_unit_types(u) {
	for_all_unit_types(u2) {
	    set_uu_bhw(u, u2, basic_hit_worth(u, u2));
	    set_uu_bcw(u, u2, basic_capture_worth(u, u2));
	}
    }
    /* Tell how things rated. */
    if (DebugM) display_assessment();
}

/* A crude estimate of the worth of having one type of unit. */

int basic_worth(u)
int u;
{
    int worth = 0, u2, r, range;
  
    worth += u_hp(u) * 10;
    for_all_unit_types(u2) {
	if (could_make(u, u2))
	  worth += (u_bw(u2) * (50)) / 1 /* uu_make(u, u2) */;
	if (could_carry(u, u2))
	  worth += (1 + u_speed(u)) * uu_capacity_x(u, u2) *
	    (u_is_base(u) ? 10 : 1) * u_bw(u2) / 30;
    }
    range = 12345;
    for_all_material_types(r) {
	worth += um_produce(u, r) * (u_is_base(u) ? 4 : 1);
	if (um_tomove(u, r) > 0)
	  range = min(range, um_storage(u, r) / max(1, um_tomove(u, r)));
	if (um_consume(u, r) > 0) 
	  range =
	    min(range, u_speed(u) * um_storage(u, r) / max(1, um_consume(u, r)));
    }
    worth += u_speed(u) * u_hp(u);
    worth += (range == 12345 ? area.maxdim : range)
      * u_hp(u) / max(1, 10 - u_speed(u));
    for_all_unit_types(u2) {
	worth += (worth * uu_capture(u, u2)) / 150;
    }
    worth = isqrt(worth);
    DMprintf("unit type %s worth %d \n ", u_type_name(u), worth);
    return worth;
}

/* A crude estimate of the worth of having a type of unit for offense. */

int
offensive_worth(u)
int u;
{
    int worth = 0, u2, r, range;

    if (mobile(u)) {
    	for_all_unit_types(u2) {
    		if (basic_hit_worth(u, u2) > 0) worth += basic_hit_worth(u, u2);
    	}
    }
#if 0  
    worth += u_hp(u) * 10;
    for_all_unit_types(u2) {
	if (could_carry(u, u2))
	  worth += (1 + u_speed(u)) * uu_capacity_x(u, u2) *
	    (u_is_base(u) ? 10 : 1) * u_bw(u2) / 30;
    }
    worth += u_speed(u) * u_hp(u);
    worth += (range == 12345 ? area.maxdim : range)
      * u_hp(u) / max(1, 10 - u_speed(u));
    for_all_unit_types(u2) {
	worth += (worth * uu_capture(u, u2)) / 150;
    }
    worth = isqrt(worth);
#endif
    DMprintf("unit type %s worth %d \n ", u_type_name(u), worth);
    return worth;
}

/* A basic estimate of the payoff of one unit type attacking another type
   directly.  This is "context-free", does not account for overall goals etc. */

/* (should account for number of attacks possible in one turn) */

basic_hit_worth(u, e)
int u, e;
{
    int dam, numdice, numspots, offset, avgdamage, worth = 0, anti = 0;

    dam = uu_damage(u, e);
    DICE(dam, numdice, numspots, offset);
    avgdamage = offset + (numdice * numspots) / 2;
    if (avgdamage > u_hp(e)) avgdamage = u_hp(e);
    worth = (uu_hit(u, e) * avgdamage) / u_hp(e);
    if (1 /* strength of counterattack */) {
	dam = uu_damage(e, u);
	DICE(dam, numdice, numspots, offset);
	avgdamage = offset + (numdice * numspots) / 2;
	if (avgdamage > u_hp(u)) avgdamage = u_hp(u);
	anti = (uu_hit(e, u) * avgdamage) / u_hp(u);
    }
    return worth - (anti * 9) / 10;
}

/* A crude estimate of the payoff of one unit type trying to capture. */

basic_capture_worth(u, e)
int u, e;
{
    int worth = 0;

    if (could_capture(u, e)) {
	worth = uu_capture(u, e) * u_acp(u) /* divided by acp/attack */;
    }
    return worth;
}


/* This is the maximum distance from "home" that a unit can expect to get, travelling
   on its most hostile terrain type. */

operating_range_worst(u)
int u;
{
    int m, t, prod, range, worstrange = area.maxdim;

    for_all_material_types(m) {
    	if (um_consume(u, m) > 0) {
	    for_all_terrain_types(t) {
	    	if (!terrain_always_impassable(u, t)) {
	    	    prod = (um_produce(u, m) * ut_productivity(u, t)) / 100;
	    	    if (prod < um_consume(u, m)) {
			range = um_storage(u, m) / (um_consume(u, m) - prod);
			if (range < worstrange) worstrange = range;
		    }
		}
	    }
	}
    }
    return worstrange;
}

/* Same, but for best terrain. */

operating_range_best(u)
int u;
{
    int m, t, prod, range, tbestrange, tbest = 0, bestrange = 0;

    for_all_terrain_types(t) {
	if (!terrain_always_impassable(u, t)) {
	    tbestrange = area.maxdim;
	    for_all_material_types(m) {
		if (um_consume(u, m) > 0) {
	    	    prod = (um_produce(u, m) * ut_productivity(u, t)) / 100;
	    	    if (prod < um_consume(u, m)) {
			range = um_storage(u, m) / (um_consume(u, m) - prod);
			if (range < tbestrange) tbestrange = range;
		    }
		}
	    }
	    if (tbestrange > bestrange) {
		bestrange = tbestrange;
		tbest = t;
	    }
	}
    }
    return bestrange;
}

/* Some notion of the unit's "strength"? */

unit_strength(u)
int u;
{
    return 1;
}

/* Display the results of our calculations. */

display_assessment()
{
    int t, u, u2;

    DMprintf("\n\nEstimated Terrain Percentages:\n");
    for_all_terrain_types(t) {
	DMprintf(" %3d%%", t_fraction(t));
    }
    DMprintf("\nUnit Attributes:\n");
    for_all_unit_types(u) {
	DMprintf(" %s : base %d, transport %d, carrier %d, worth %d\n",
	       utype_name_n(u, 3), u_is_base(u),
	       u_is_transport(u), u_is_carrier(u), u_bw(u));
	DMprintf("    Operate between ranges %d and %d\n", operating_range_worst(u), operating_range_best(u));
    }
    DMprintf("\nUnit vs Unit Combat:\n");
    for_all_unit_types(u) {
	for_all_unit_types(u2) DMprintf("%5d", uu_bhw(u, u2));
	DMprintf("\n");
    }
    DMprintf("\nUnit vs Unit Capture:\n");
    for_all_unit_types(u) {
	for_all_unit_types(u2) DMprintf(" %4d", uu_bcw(u, u2));
	DMprintf("\n");
    }
    DMprintf("\n");
}

mplayer_react_to_unit_loss(side, unit)
Side *side;
Unit *unit;
{
    int x = unit->x, y = unit->y;
    Theater *th;

    if (!inside_area(x, y)) {
	x = unit->prevx;  y = unit->prevy;
    }
    if (!inside_area(x, y)) return;
    if (mplayer(side) && (th = theater_at(side, x, y)) != NULL) {
    	++(th->units_lost);
    }
}

is_base_for(u1, u2)
int u1, u2;
{
    return (u_speed(u1) == 0
	    && (uu_capacity_x(u2, u1) > 0
		|| (uu_size(u2, u1) <= u_capacity(u1))));
}

is_carrier_for(u1, u2)
int u1, u2;
{
    return (u_speed(u1) > 0
	    && (uu_capacity_x(u2, u1) > 0
		|| (uu_size(u2, u1) <= u_capacity(u1))));
}

/* Since *.def parameters don't have setters usually, we have to supply
   some here.  These are very sensitive to how the parameters are organized,
   and they don't do any checking, so you have to careful about using them. */

set_u_is_base(u, n) int u, n; {  utypes[u].is_base = n;  }
set_u_is_transport(u, n) int u, n; {  utypes[u].is_transport = n;  }
set_u_is_carrier(u, n) int u, n; {  utypes[u].is_carrier = n;  }
set_u_is_base_builder(u, n) int u, n; {  utypes[u].is_base_builder = n;  }
set_u_can_make(u, n) int u, n; {  utypes[u].can_make = n;  }
set_u_can_capture(u, n) int u, n; {  utypes[u].can_capture = n;  }
set_u_bw(u, n) int u, n; {  utypes[u].bw = n;  }
set_t_fraction(t, n) int t, n; {  ttypes[t].fraction = n;  }

int bhwtab = -1;
int bcwtab = -1;

set_uu_bhw(u1, u2, v)
int u1, u2, v;
{
    if (bhwtab < 0) {
	    for (bhwtab = 0; tabledefns[bhwtab].name != NULL; ++bhwtab) {
		if (strcmp("zzz-bhw", tabledefns[bhwtab].name) == 0) {
		    allocate_table(bhwtab, FALSE);
		    break;
		}
	    }
    }
    (*(tabledefns[bhwtab].table))[numutypes * u1 + u2] = v;
}

set_uu_bcw(u1, u2, v)
int u1, u2, v;
{
  if (bcwtab < 0) {
   for (bcwtab = 0; tabledefns[bcwtab].name != NULL; ++bcwtab) {
	if (strcmp("zzz-bcw", tabledefns[bcwtab].name) == 0) {
	    allocate_table(bcwtab, FALSE);
	    break;
	}
    }
  }
  (*(tabledefns[bcwtab].table))[numutypes * u1 + u2] = v;
}

