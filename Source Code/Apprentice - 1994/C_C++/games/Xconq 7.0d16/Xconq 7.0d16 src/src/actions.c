/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Implementations of the actions. */

/* The general theory of actions is that the interface or AI code calls, for
   an action foo, the routine prep_foo_action, which just records the action
   for later execution.  The action loop in run_game eventually calls
   do_foo_action, which first calls check_foo_action to confirm that the
   action will succeed.  If check_foo_action does not find any errors, then
   the action cannot fail.  The main body of do_foo_action then implements
   the effects of the action.  Interfaces may call check_foo_action freely,
   but should never call do_foo_action directly. */

#include "conq.h"

extern int anypostactionscores;

UnitVector *add_unit_to_vector();

#define angle_with(d1, d2) min((d1-d2+NUMDIRS)%NUMDIRS, (d2-d1+NUMDIRS)%NUMDIRS)

/* The table of all the types of actions. */

ActionDefn actiondefns[] = {

#undef  DEF_ACTION
#define DEF_ACTION(NAME,CODE,ARGS,FN,doc) { CODE, NAME, ARGS, FN },

#include "action.def"

    { -1, NULL, NULL, NULL }
};

char *actiondesigbuf = NULL;

/* Eventually this should do initial alloc of actions. */

init_actions()
{
}

/* Just a placeholder action, so not much to do here. */

do_none_action(unit, unit2)
Unit *unit, *unit2;
{
    return A_ANY_DONE;
}

prep_move_action(unit, unit2, x, y, z)
Unit *unit, *unit2;
int x, y, z;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_MOVE;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = z;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

/* The basic act of moving.  This attempts to go and maybe fails, but
   takes no corrective action.  Note that this requires space in the
   destination hex, will not board, attack, etc - all that is task- and
   plan-level behavior. */

do_move_action(unit, unit2, nx, ny, nz)
Unit *unit, *unit2;
int nx, ny, nz;
{
    int u, u2, t, speed, mpcost = 0, acpcost;
    int rslt = check_move_action(unit, unit2, nx, ny, nz);

    if (!valid(rslt)) return rslt;
    u = unit->type;  u2 = unit2->type;
    t = terrain_at(nx, ny);
    if (!inside_area(nx, ny)) {
	kill_unit(unit2, H_UNIT_LEFT_WORLD);
	rslt = A_ANY_DONE;
    } else if (ut_vanishes_on(u2, t) && !can_move_via_conn(unit2, nx, ny)) {
	kill_unit(unit2, H_UNIT_VANISHED);
	rslt = A_ANY_DONE; /* should return something else :-) */
    } else if (ut_wrecks_on(u2, t) && !can_move_via_conn(unit2, nx, ny)) {
	if (u_wrecked_type(u2) == NONUTYPE) {
	    /* Occupants always die if the wrecked unit disappears. */
	    kill_unit(unit, H_UNIT_WRECKED);
	} else if (ut_vanishes_on(u_wrecked_type(u2), t)) {
	    /* Change the unit's type at its *current* location. */
	    change_unit_type(unit2, u_wrecked_type(u2));
	    /* Restore to default hp for the new type. */
	    unit2->hp = unit2->hp2 = u_hp(u2);
	    /* Get rid of occupants if now overfull. */
	    eject_excess_occupants(unit2);
	} else {
	    move_unit(unit2, nx, ny);
	    /* Change the unit's type at its new location. */
	    change_unit_type(unit2, u_wrecked_type(u2));
	    /* Restore to default hp for the new type. */
	    unit2->hp = unit2->hp2 = u_hp(u2);
	    /* Get rid of occupants if now overfull. */
	    eject_excess_occupants(unit2);
	}
	rslt = A_ANY_DONE;
    } else {
	mpcost = move_unit(unit2, nx, ny);
	/* ZOC move cost is added after action is done. */
    	mpcost += zoc_move_cost(unit2, nx, ny, nz);
	rslt = A_ANY_DONE;
    }
    if (alive(unit)) {
/*	    acpcost = u_acp_to_move(u2);  where does this fit in? */
	    speed = u_speed(u2);
	    if (speed > 0) {
		acpcost = (mpcost * 100) / speed;
		if (acpcost < 1) acpcost = 1;
	    }
	    use_up_acp(unit, acpcost);
    }
    /* Count the unit as having actually moved. */
    if (alive(unit2) && unit2->act) ++(unit2->act->actualmoves);
    return rslt;
}

check_move_action(unit, unit2, nx, ny, nz)
Unit *unit, *unit2;
int nx, ny, nz;
{
    int u, u2, u3, ox, oy, oz, mpavail, totcost, freemp, m, speed;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    /* Note that edge cell dests (used to leave the world) are allowed. */
    if (!in_area(nx, ny)) return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    ox = unit2->x;  oy = unit2->y;  oz = unit2->z;
    if (u_acp_to_move(u2) < 1) return A_ANY_CANNOT_DO;
    if (!has_enough_acp(unit, u_acp_to_move(u2))) return A_ANY_NO_ACP;
    if (!has_supply_to_act(unit2)) return A_ANY_NO_MATERIAL;
    /* Destination is outside the world and we're not allowed to leave. */
    if (!inside_area(nx, ny) && u_mp_to_leave_world(u2) < 0)
	return A_MOVE_CANNOT_LEAVE_WORLD;
    /* Check if the destination is within our move range. */
    /* (but check for border slides here?) */
    if (distance(ox, oy, nx, ny) > u_move_range(u2)) return A_MOVE_TOO_FAR;
    if (nz > 0) return A_MOVE_TOO_FAR;
    /* Check if the destination is in a blocking ZOC. */
    if (in_blocking_zoc(unit, nx, ny, nz)) return A_ANY_ERROR;
    /* Now start looking at the move costs. */
    u3 = (unit2->transport ? unit2->transport->type : NONUTYPE);
    totcost = total_move_cost(u2, u3, ox, oy, oz, nx, ny, nz);
    speed = u_speed(u2);
    /* (should generalize!) */
    if (winds_defined() && u_wind_speed_effect(u2) != lispnil) {
	speed *= wind_force_at(ox, oy);
    }
    if (speed > 0 && unit->act) {
	mpavail = (unit->act->acp * speed) / 100;
    } else {
	mpavail = 0;
    }
    /* Zero mp always disallows movement. */
    if (mpavail <= 0) return A_MOVE_NO_MP;
    /* The free mp might get us enough moves, so add it before comparing. */
    if (mpavail + u_free_mp(u2) < totcost) return A_MOVE_NO_MP;
    /* If cell terrain is too small, we can't move into it. */
    if (!can_occupy_cell(unit2, nx, ny)) return A_MOVE_DEST_FULL;
    /* We have to have a minimum level of supply to be able to move. */
    for_all_material_types(m) {
	if (unit->supply[m] < um_to_move(u2, m)) return A_ANY_NO_MATERIAL;
    }
    return A_ANY_OK;
}

can_move_via_conn(unit, nx, ny)
Unit *unit;
int nx, ny;
{
	int c;

	if (numconntypes == 0) return FALSE;
	for_all_terrain_types(c) {
	    if (t_is_connection(c)
		&& aux_terrain_defined(c)
		&& connection_at(unit->x, unit->y, closest_dir(nx - unit->x, ny - unit->y), c)
		&& !ut_vanishes_on(unit->type, c)
		&& !ut_wrecks_on(unit->type, c)) {
		return TRUE;
	    }
	}
	return FALSE;
}

/* Conduct the actual move (used in both normal moves and some combat). */

move_unit(unit, nx, ny)
Unit *unit;
int nx, ny;
{
    int u = unit->type, ox = unit->x, oy = unit->y, mpcost = 0;
    extern int anypeoplesidechanges;

    /* Disappear from the old location ... */
    leave_hex(unit);
    /* ... and appear in the new one! */
    enter_hex(unit, nx, ny);
    /* Movement may set off other people's alarms. */
    maybe_react_to_move(unit, ox, oy);
    /* The people at the new location may change sides immediately. */
    if (people_sides_defined()
	&& anypeoplesidechanges
	&& probability(ut_people_surrender(u, terrain_at(nx, ny)))) {
	change_people_side_around(nx, ny, u, unit->side);
    }
    /* Use up supplies as directed. */
    consume_move_supplies(unit);
    /* a hack */
    update_cell_display(unit->side, ox, oy, TRUE);
    /* Always return the mp cost, even if the mover died. */
    return total_move_cost(u, NONUTYPE, ox, oy, 0, nx, ny, 0);
}

can_move_at_all(unit)
Unit *unit;
{
    return u_speed(unit->type) > 0;
}

/* This is true if the given location is in a blocking zoc for the unit. */

extern int maxzocrange;

in_blocking_zoc(unit, x, y, z)
Unit *unit;
int x, y, z;
{
    int t = terrain_at(x, y), dir, x1, y1;
    Unit *unit2;

    switch (maxzocrange) {
      case -1:
	break;
      case 0:
	for_all_stack(x, y, unit2) {
	    if (unit_blockable_by(unit, unit2)
		&& ut_zoc_into(unit2->type, t)) return TRUE;
	}
	break;
      case 1:
	for_all_directions(dir) {
	    if (point_in_dir(x, y, dir, &x1, &y1)) {
		for_all_stack(x, y, unit2) {
		    if (unit_blockable_by(unit, unit2)
			&& uu_zoc_range(unit2->type, unit->type) > 0
			&& ut_zoc_into(unit2->type, t)) return TRUE;
		}
	    }
	}
	break;
      default:
	/* (should write this - bleah, complicated) */
	break;
    }
    return FALSE;
}

/* This is true if unit2 wants to block unit from moving. */

unit_blockable_by(unit, unit2)
Unit *unit, *unit2;
{
    return (unit->side != unit2->side
	    && uu_mp_to_enter_zoc(unit->type, unit2->type) < 0);
}

/* Compute the number of move points that will be needed to do the given
   move. */

total_move_cost(u, u2, x1, y1, z1, x2, y2, z2)
int u, u2, x1, y1, z1, x2, y2, z2;
{
    int cost, b, c, conncost, angle;

    if (z1 != 0 || z2 != 0) {
    	/* should write these calcs eventually */
    }
    /* are the two points adjacent? */
    if (u2 != NONUTYPE) {
    	cost = uu_mp_to_leave(u, u2);
    	/* (should also add ferrying effect for departure) */
    } else {
    	cost = ut_mp_to_leave(u, terrain_at(x1, y1));
    }
    if (numbordtypes > 0) {
	    /* Add any necessary border crossing costs. */
	    for_all_terrain_types(b) {
		if (t_is_border(b)
		    && aux_terrain_defined(b)
		    && border_at(x1, y1, closest_dir(x2 - x1, y2 - y1), b)) {
		    cost += ut_mp_to_enter(u, b);
		}
	    }
    }
    cost += ut_mp_to_enter(u, terrain_at(x2, y2));
    /* Use a connection traversal if it would be cheaper.  This is
       only automatic if the connection on/off costs are small enough,
       otherwise the unit has to do explicit actions to get on the
       connection and off again. */
    if (numconntypes > 0) {
	/* Try each connection type to see if it's better. */
	for_all_terrain_types(c) {
	    if (t_is_connection(c)
		&& aux_terrain_defined(c)
		&& connection_at(x1, y1, closest_dir(x2 - x1, y2 - y1), c)) {
		conncost = ut_mp_to_enter(u, c)
		  + ut_mp_to_traverse(u, c)
		    + ut_mp_to_leave(u, c);
		cost = min(cost, conncost);
	    }
	}
    }
    /* The cost of leaving the world is always an addon. */
    if (!inside_area(x2, y2)) {
    	cost += u_mp_to_leave_world(u);
    }
    if (winds_defined() && u_wind_speed_effect(u) != lispnil) {
	angle = angle_with(closest_dir(x2 - x1, y2 - y1), wind_dir_at(x1, y1));
	cost += angle * wind_force_at(x1, y1);
    }
    /* Any movement must always cost at least 1 mp. */
    if (cost < 1) cost = 1;
    return cost;
}

zoc_move_cost(unit, x, y, z)
Unit *unit;
int x, y, z;
{
    int t = terrain_at(x, y), mpcost = 0, dir, x1, y1;
    Unit *unit2;

    switch (maxzocrange) {
      case -1:
	break;
      case 0:
	for_all_stack(x, y, unit2) {
	    if (in_play(unit2) /* should be is_active? */
		&& uu_zoc_range(unit2->type, unit->type) >= 0
		&& ut_zoc_into(unit2->type, t))
	      mpcost = max(mpcost, unit_slowdown_by(unit, unit2));
	}
	break;
      case 1:
	for_all_directions(dir) {
	    if (point_in_dir(x, y, dir, &x1, &y1)) {
		for_all_stack(x1, y1, unit2) {
		    if (in_play(unit2) /* should be is_active? */
			&& uu_zoc_range(unit2->type, unit->type) >= 1
			&& ut_zoc_into(unit2->type, t))
		      mpcost = max(mpcost, unit_slowdown_by(unit, unit2));
		}
	    }
	}
	break;
      default:
	/* (should write this - bleah, complicated) */
	break;
    }
    return mpcost;
}

unit_slowdown_by(unit, unit2)
Unit *unit, *unit2;
{
    return (unit->side != unit2->side ?
	    uu_mp_to_enter_zoc(unit->type, unit2->type) : 0);
}

#if 0
    int u = unit->type, m, moves, maxacp;
    Unit *occ;

    /* Start with the maximum speed. */
    moves = u_speed(u);
    /* Compute any effect of damage on the unit's speed. */
    if (moves > 0) {
	if (unit->hp < u_hp_at_max_speed(u)) {
	    if (unit->hp <= u_hp_at_min_speed(u)) {
		moves = u_speed_min(u);
	    } else if (u_hp_at_min_speed(u) == u_hp_at_max_speed(u)) {
		/* ? */
		moves = -1;
	    } else {
		/* Interpolate to get speed, rounding down. */
		moves =
		  u_speed_min(u) + (u_speed(u) - u_speed_min(u)) /
		    (u_hp_at_max_speed(u) - u_hp_at_min_speed(u));
	    }
	}
	/* Compute any effect of occupants on a transport's speed. */
	for_all_occupants(unit, occ) {
	    if (uu_mobility(u, occ->type) != 100) {
		moves = (moves * uu_mobility(u, occ->type)) / 100;
		break;  /* or make conditional somehow? */
	    }
	}
	/* The previous slowdowns should never reduce below 1. */
	moves = max(1, moves);
	/* Missing movement materials can immobilize us however. */
	/* (need to warn players when this happens) */
	for_all_material_types(m) {
	    if (um_tomove(u, m) > 0 && unit->supply[m] <= 0) {
		moves = 0;
		break;
	    }
	}
    }
#endif

closest_dir(x, y)
int x, y;
{
    int dir;

    for_all_directions(dir) {
	if (dirx[dir] == x && diry[dir] == y) return dir;
    }
    return 0;
}

/* This is a hook to handle any reactions to the unit's successful move. */

maybe_react_to_move(unit, ox, oy)
Unit *unit;
int ox, oy;
{
}

/* Use up the supply consumed by a successful move.  Also, the move might
   have used up essentials and left the unit without its survival needs,
   so check for this case and maybe hit/kill the unit. */
  
consume_move_supplies(unit)
Unit *unit;
{
    int u = unit->type, m;
    
    for_all_material_types(m) {
	if (alive(unit) && um_consume_per_move(u, m) > 0) {
	    unit->supply[m] -= um_consume_per_move(u, m);
	    if (unit->supply[m] <= 0
	    	&& unit->transport == NULL
		&& um_consume(u, m) > 0
		&& um_hp_per_starve(u, m) > 0) {
		    exhaust_supply(unit, m, FALSE);
	    }
	}
    }
    /* Trigger any supply alarms. */
    if (alive(unit)
    	&& unit->plan
    	&& (unit->plan->alarmmask & 0x01)
    	&& !(unit->plan->alarms & 0x01)
    	&& past_halfway_point(unit)
    	) {
    	unit->plan->alarms &= 0x01;
    }
}

/* Movement into another unit. */

prep_enter_action(unit, unit2, dest)
Unit *unit, *unit2, *dest;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_ENTER;
    unit->act->nextaction.args[0] = dest->id;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

do_enter_action(unit, unit2, dest)
Unit *unit, *unit2, *dest;
{
    int rslt = check_enter_action(unit, unit2, dest);

    if (!valid(rslt)) return rslt;
    leave_hex(unit2);
    enter_transport(unit2, dest);
    /* this is doctrine? */
    /*	if (n > 0) order_sentry(unit, n); */
    use_up_acp(unit, uu_acp_to_enter(unit2->type, dest->type));
    return A_ANY_DONE;
}

check_enter_action(unit, unit2, dest)
Unit *unit, *unit2, *dest;
{
    int u, u2, u3, u2x, u2y, dfx, dfy;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!in_play(dest)) return A_ANY_ERROR;
    u = unit->type;
    u2 = unit2->type;
    u3 = dest->type;
    if (uu_acp_to_enter(u2, u3) < 1) return A_ANY_CANNOT_DO;
    /* Can't enter self. */
    if (unit2 == dest) return A_ANY_ERROR;
    u2x = unit2->x;  u2y = unit2->y;
    dfx = dest->x;  dfy = dest->y;
    if (!between(0, distance(u2x, u2y, dfx, dfy), 1)) return A_ANY_ERROR;
    if (!sides_allow_entry(unit2, dest)) return A_ANY_ERROR;
    if (!can_occupy(unit2, dest)) return A_ANY_ERROR;
    if (!has_enough_acp(unit, uu_acp_to_enter(u2, u3))) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* This tests whether the relationship between the sides of a unit
   and a prospective transport allows for entry of the unit. */

sides_allow_entry(unit, transport)
Unit *unit, *transport;
{
    if (unit->side == NULL) {
    	if (transport->side == NULL) {
    	    return TRUE;
    	} else {
    	    return uu_can_enter_indep(unit->type, transport->type);
    	}
    } else {
    	if (transport->side == NULL) {
    	    return uu_can_enter_indep(unit->type, transport->type);
    	} else {
    	    return (unit->side == transport->side);  /* should test side relations */
    	}
    }
}

/* This computes the total mp cost of entering a transport. */

total_entry_cost(u1, x1, y1, z1, u2, x2, y2, z2)
int u1, x1, y1, z1, u2, x2, y2, z2;
{
    int cost = 0, ferry = uu_ferry_on_enter(u1, u2);
    int t1 = terrain_at(x1, y1), t2 = terrain_at(x2, y2);

    /* (should add in possibility of using conn to enter) */
    /* Maybe costs to leave terrain of own cell. */
    if (ferry < 3) {
    	cost += ut_mp_to_leave(u1, t1);
    }
    /* Maybe costs to cross a border. */
    if (ferry < 2) {
    }
    /* Maybe even have to pay cost of crossing destination's terrain. */
    if (ferry < 1) {
    	cost += ut_mp_to_enter(u1, t2);
    }
    /* Add the actual cost of entry. */
    cost += uu_mp_to_enter(u1, u2);
    /* Movement must always cost at least 1 mp. */
    if (cost < 1) cost = 1;
    return cost;
}

/* Explicit material production. */

prep_produce_action(unit, unit2, m, n)
Unit *unit, *unit2;
int m, n;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_PRODUCE;
    unit->act->nextaction.args[0] = m;
    unit->act->nextaction.args[1] = n;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

do_produce_action(unit, unit2, m, n)
Unit *unit, *unit2;
int m, n;
{
    int rslt = check_produce_action(unit, unit2, m, n);

    if (!valid(rslt)) return rslt;
    use_up_acp(unit, um_acp_to_produce(unit2->type, m));
    return A_ANY_DONE;
}

check_produce_action(unit, unit2, m, n)
Unit *unit, *unit2;
int m, n;
{
    int acp;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!is_material_type(m)) return A_ANY_ERROR;
    acp = um_acp_to_produce(unit2->type, m);
    if (acp < 1) return A_ANY_CANNOT_DO;
    if (!has_enough_acp(unit, acp)) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* Transfer of material. */

prep_transfer_action(unit, unit2, m, n, unit3)
Unit *unit, *unit2, *unit3;
int m, n;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_TRANSFER;
    unit->act->nextaction.args[0] = m;
    unit->act->nextaction.args[1] = n;
    unit->act->nextaction.args[2] = unit3->id;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

do_transfer_action(unit, unit2, m, n, unit3)
Unit *unit, *unit2, *unit3;
int m, n;
{
    int actual;
    int rslt = check_transfer_action(unit, unit2, m, n, unit3);

    if (!valid(rslt)) return rslt;
    if (n > 0) {
    	actual = transfer_supply(unit2, unit3, m, n);
    } else {
    	actual = transfer_supply(unit3, unit2, m, -n);
    }
    use_up_acp(unit, 1);
    if (actual == n) {
	return A_ANY_DONE;
    } else {
    	/* (should be able to say that action did not do all that was requested) */
	return A_ANY_DONE;
    }
}

check_transfer_action(unit, unit2, m, n, unit3)
Unit *unit, *unit2, *unit3;
int m, n;
{
    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!is_material_type(m)) return A_ANY_ERROR;
    if (n == 0) return A_ANY_ERROR;
    if (!in_play(unit3)) return A_ANY_ERROR;
    if (n > 0) {
	if (unit2->supply[m] <= 0) return A_ANY_ERROR;
	/* (should check for generic capacity also) */
	if (um_storage(unit3->type, m) == 0) return A_ANY_ERROR;
    } else {
	if (unit3->supply[m] <= 0) return A_ANY_ERROR;
	/* (should check for generic capacity also) */
	if (um_storage(unit2->type, m) == 0) return A_ANY_ERROR;
    }
    if (!has_enough_acp(unit, 1)) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* Move supply from one unit to another.  Don't move more than is possible;
   check both from and to amounts and capacities. */

transfer_supply(from, to, m, amount)
Unit *from, *to;
int m, amount;
{
    int origfrom = from->supply[m], origto = to->supply[m];

    amount = min(amount, origfrom);
    amount = min(amount, um_storage(to->type, m) - origto);
    from->supply[m] -= amount;
    to->supply[m] += amount;
    /* Make sure any displays of supply levels see the transfer. */
    update_unit_display(from->side, from, TRUE); 
    update_unit_display(to->side, to, TRUE); 
    Dprintf("%s (had %d) transfers %d %s to %s (had %d)\n",
	    unit_desig(from), origfrom, amount, m_type_name(m),
	    unit_desig(to), origto);
    return amount;
}

/* If a side's tech level is under its max, research can increase it. */

prep_research_action(unit, unit2, u3)
Unit *unit, *unit2;
int u3;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_RESEARCH;
    unit->act->nextaction.args[0] = u3;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

do_research_action(unit, unit2, u3)
Unit *unit, *unit2;
int u3;
{
    int u = unit->type, u2 = unit2->type, lim;
    Side *side = unit2->side;
    int rslt = check_research_action(unit, unit2, u3);

    if (!valid(rslt)) return rslt;
    side->tech[u3] += prob_fraction(uu_tech_per_research(u2, u3));
    /* Silently apply the per-side-per-turn limit on tech gains. */
    lim =  side->inittech[u3] + u_tech_per_turn_max(u3);
    if (side->tech[u3] > lim) side->tech[u3] = lim;
    /* Adjust the tech levels of any related unit types to match. */
    adjust_tech_crossover(side, u3);
    /* (should notify side about changes and/or thresholds reached?) */
    use_up_acp(unit, uu_acp_to_research(u2, u3));
    return A_ANY_DONE;
}

check_research_action(unit, unit2, u3)
Unit *unit, *unit2;
int u3;
{
    int u;
    Side *side;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!is_unit_type(u3)) return A_ANY_ERROR;
    u = unit->type;
    side = unit->side;
    /* Independent units don't do research. */
    if (side == NULL) return A_ANY_ERROR;
    /* This unit must be of a type that can research the given type. */
    if (uu_acp_to_research(u, u3) < 1) return A_ANY_CANNOT_DO;
    /* Max tech level means there's nothing more to learn. */
    if (side->tech[u3] >= u_tech_max(u3)) return A_ANY_ERROR;
    if (!has_enough_acp(unit, uu_acp_to_research(u, u3))) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* For all unit types, bring their tech level up to match the crossovers
   from the given unit type. */

adjust_tech_crossover(side, u)
Side *side;
int u;
{
    int u2, cross;

    for_all_unit_types(u2) {
	if (u2 != u) {
	    /* (should be "as ratio of max levels for each type") */
	    cross = (uu_tech_crossover(u, u2) * side->tech[u2]) / 100;
	    if (cross > side->tech[u2]) side->tech[u2] = cross;
	}
    }
}

/* Before a unit can build another, it must take some time to prepare by
   "tooling up". */

prep_toolup_action(unit, unit2, u3)
Unit *unit, *unit2;
int u3;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_TOOL_UP;
    unit->act->nextaction.args[0] = u3;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

do_toolup_action(unit, unit2, u3)
Unit *unit, *unit2;
int u3;
{
    int rslt = check_toolup_action(unit, unit2, u3);

    if (!valid(rslt)) return rslt;
    /* Increase the tooling. */
    unit2->tooling[u3] += 1;
    use_up_acp(unit, uu_acp_to_toolup(unit2->type, u3));
    return A_ANY_DONE;
}

check_toolup_action(unit, unit2, u3)
Unit *unit, *unit2;
int u3;
{
    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!is_unit_type(u3)) return A_ANY_ERROR;
    if (uu_acp_to_toolup(unit->type, u3) < 1) return A_ANY_CANNOT_DO;
    if (!has_enough_acp(unit, uu_acp_to_toolup(unit2->type, u3)))
      return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* For all unit types, bring their tooling level up to match the crossovers
   from the given unit type. */

adjust_tooling_crossover(unit, u2)
Unit *unit;
int u2;
{
    int u3, uucross, cross;

    for_all_unit_types(u3) {
	if (u3 != u2) {
	    if ((uucross = uu_tp_crossover(u2, u3)) > 0) {
		/* (should be "as ratio of max levels for each type") */
		cross = (uucross * unit->tooling[u2]) / 100;
		if (cross > unit->tooling[u3]) unit->tooling[u3] = cross;
	    }
	}
    }
}

prep_create_in_action(unit, unit2, u3, dest)
Unit *unit, *unit2, *dest;
int u3;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_CREATE_IN;
    unit->act->nextaction.args[0] = u3;
    unit->act->nextaction.args[1] = dest->id;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

/* This action creates the (incomplete) unit. */

do_create_in_action(unit, unit2, u3, dest)
Unit *unit, *unit2, *dest;
int u3;
{
    int u, u2, m;
    Unit *newunit;
    int rslt = check_create_in_action(unit, unit2, u3, dest);

    if (!valid(rslt)) return rslt;
    u = unit->type;  u2 = unit2->type;
    /* Make the new unit. */
    if ((newunit = create_unit(u3, FALSE)) != NULL) {
	newunit->hp = 1;
	newunit->cp = uu_creation_cp(u2, u3);
	set_unit_side(newunit, unit->side);
	/* Always number the unit when first created. */
	assign_unit_number(newunit);
    	for_all_material_types(m) {
	    if (newunit->supply[m] < um_storage_x(u3, m)) {
		newunit->supply[m] = max(um_created_supply(u3, m),
					 um_storage_x(u3, m));
		/* (should account for shared storage also) */
	    }
    	}
	enter_transport(newunit, dest);
	/* Unit might have started out complete. */
	if (completed(newunit)) {
    	    garrison_unit(newunit, unit2);
	    make_unit_complete(newunit);
	} else {
	    record_event(H_UNIT_CREATED, -1, u3);
	}
    for_all_material_types(m) {
        unit2->supply[m] -= um_consume_per_build(u3, m);
    }
	use_up_acp(unit, uu_acp_to_create(u2, u3));
	return A_ANY_DONE;
    } else {
	/* We've hit a max number of units, nothing to be done. */
	return A_ANY_ERROR;
    }
}

check_create_in_action(unit, unit2, u3, dest)
Unit *unit, *unit2, *dest;
int u3;
{
    int u, u2, m;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!is_unit_type(u3)) return A_ANY_ERROR;
    if (!in_play(dest)) return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    if (uu_acp_to_create(u2, u3) < 1) return A_ANY_CANNOT_DO;
    /* Check the tech level of the side. */
    if (u_tech_to_build(u3) > 0) {
	if (unit->side == NULL) return A_ANY_ERROR;
	if (unit->side->tech[u3] < u_tech_to_build(u3)) return A_ANY_ERROR;
    }
    if (distance(unit2->x, unit2->y, dest->x, dest->y) > uu_sep_to_create_max(u2, u3))
      return A_ANY_TOO_FAR;
    /* (should check tooling also) */
    if (unit2->transport != NULL
        && !uu_occ_can_build(unit2->transport->type, u2))
      return A_ANY_ERROR;
    if (!type_can_occupy(u3, dest)) return A_ANY_ERROR;
    for_all_material_types(m) {
    	if (unit2->supply[m] < um_to_create(u3, m))
    	  return A_ANY_NO_MATERIAL;
    	if (unit2->supply[m] < um_consume_per_build(u3, m))
    	  return A_ANY_NO_MATERIAL;
    }
    if (!has_enough_acp(unit, uu_acp_to_create(u2, u3))) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

prep_create_at_action(unit, unit2, u3, x, y, z)
Unit *unit, *unit2;
int u3, x, y, z;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_CREATE_AT;
    unit->act->nextaction.args[0] = u3;
    unit->act->nextaction.args[1] = x;
    unit->act->nextaction.args[2] = y;
    unit->act->nextaction.args[3] = z;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

do_create_at_action(unit, unit2, u3, x, y, z)
Unit *unit, *unit2;
int u3, x, y, z;
{
    int u, u2 = unit2->type, m;
    Unit *newunit;
    int rslt = check_create_at_action(unit, unit2, u3, x, y, z);

    if (!valid(rslt)) return rslt;
    u = unit->type;  u2 = unit2->type;
    /* Make the new unit. */
    if ((newunit = create_unit(u3, FALSE)) != NULL) {
	newunit->hp = 1;
	newunit->cp = uu_creation_cp(u2, u3);
	set_unit_side(newunit, unit->side);
    	for_all_material_types(m) {
    	    newunit->supply[m] = um_created_supply(u3, m);
    	}
	/* Always number the unit when first created. */
	assign_unit_number(newunit);
	/* Put it at a correct location. */
	if (can_occupy_cell(newunit, x, y)) {
	    enter_hex(newunit, x, y);
	} else {
	    /* Always try to let builder occupy its incomplete work. */
	    leave_hex(unit2);
	    if (can_occupy_cell(newunit, x, y) && can_occupy(unit2, newunit)) {
	    	enter_hex(newunit, x, y);
	    	enter_transport(unit2, newunit);
	    } else {
		/* Put the builder back. */
		enter_hex(unit2, x, y);
	    	run_warning("something is garbled in construction");
	    }
	}
	/* and set its altitude? */
	/* Unit might be complete right away. */
	if (completed(newunit)) {
    	    garrison_unit(newunit, unit2);
	    make_unit_complete(newunit);
	} else {
	    record_event(H_UNIT_CREATED, -1, u3);
	}
    	for_all_material_types(m) {
    	    unit2->supply[m] -= um_consume_per_build(u3, m);
    	}
	use_up_acp(unit, uu_acp_to_create(u2, u3));
	return A_ANY_DONE;
    } else {
	/* We've hit a max number of units, nothing to be done. */
	return A_ANY_ERROR;
    }
}

check_create_at_action(unit, unit2, u3, x, y, z)
Unit *unit, *unit2;
int u3, x, y, z;
{
    int u, u2, m;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!is_unit_type(u3)) return A_ANY_ERROR;
    if (!inside_area(x, y)) return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    if (uu_acp_to_create(u2, u3) < 1) return A_ANY_CANNOT_DO;
    if (u_tech_to_build(u3) > 0) {
	if (unit->side == NULL) return A_ANY_ERROR;
	if (unit->side->tech[u3] < u_tech_to_build(u3)) return A_ANY_ERROR;
    }
    if (distance(unit2->x, unit2->y, x, y) > uu_sep_to_create_max(u2, u3))
      return A_ANY_TOO_FAR;
    /* check tooling */
    if (unit2->transport != NULL
        && !uu_occ_can_build(unit2->transport->type, u2))
      return A_ANY_ERROR;
    /* (should check for room and safety of terrain) */
    if (!type_can_occupy_cell(u3, x, y)
        && !can_occupy_type(unit2, u3)) return A_ANY_ERROR;
    /* (should check that unit limit not hit yet) */
    for_all_material_types(m) {
    	if (unit2->supply[m] < um_to_create(u3, m))
    	  return A_ANY_NO_MATERIAL;
    	if (unit2->supply[m] < um_consume_per_build(u3, m))
    	  return A_ANY_NO_MATERIAL;
    }
    if (!has_enough_acp(unit, uu_acp_to_create(u2, u3))) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

prep_build_action(unit, unit2, newunit)
Unit *unit, *unit2, *newunit;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_BUILD;
    unit->act->nextaction.args[0] = newunit->id;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

/* This action makes progress on a construction effort, possibly completing
   the new unit and making it available. */

do_build_action(unit, unit2, newunit)
Unit *unit, *unit2, *newunit;
{
    int u, u2, u3, m, mk, mmk, use, x, y;
    Side *us;
    Unit *transp;
    int rslt = check_build_action(unit, unit2, newunit);

    if (!valid(rslt)) return rslt;
    u = unit->type;  u2 = unit2->type;  u3 = newunit->type;
    x = unit2->x;  y = unit2->y;
    transp = unit2->transport;
    for_all_material_types(m) {
    	unit2->supply[m] -= um_to_build(u3, m);
    }
    newunit->cp += uu_cp_per_build(u2, u3);
    if (completed(newunit)) {
    	garrison_unit(newunit, unit2);
	make_unit_complete(newunit);
    }
    update_unit_display(newunit->side, newunit, TRUE);
    use_up_acp(unit, uu_acp_to_build(u2, u3));
    return A_ANY_DONE;
}

garrison_unit(newunit, unit2)
Unit *newunit, *unit2;
{
    int u2 = unit2->type, u3 = newunit->type, x = unit2->x, y = unit2->y;
    Unit *transp, *occ;

    /* Maybe get rid of the building unit if it is to be the garrison. */
    if (uu_hp_to_garrison(u2, u3) >= unit2->hp) {
	/* But first get the about-to-be-killed garrisoning unit
	   disconnected from everything. */
	leave_hex(unit2);
	/* Put new unit in place of the builder, if it was an occupant. */
	if (newunit->transport == unit2) {
	    leave_transport(newunit);
	    if (transp != NULL) {
		enter_transport(newunit, transp);
	    } else {
		enter_hex(newunit, x, y);
	    }
	}
	/* Move the other occupants anywhere we can find. */
	for_all_occupants(unit2, occ) {
	    if (can_occupy(occ, newunit)) {
		enter_transport(occ, newunit);
	    } else if (transp != NULL && can_occupy(occ, transp)) {
		enter_transport(occ, transp);
	    } else if (can_occupy_cell(occ, x, y)) {
		enter_hex(occ, x, y);
	    }
	    /* Otherwise the occupant has to die along with the garrison. */
	    /* (should also do something with sub-occs of doomed occs?) */
	}
	/* Now we can get rid of the garrisoning unit without scrambling
	   anything else. */
	kill_unit(unit2, H_UNIT_GARRISONED);
    } else {
	/* Builder just loses hp, doesn't vanish. */
	unit2->hp -= uu_hp_to_garrison(u2, u3);
    }
}

check_build_action(unit, unit2, newunit)
Unit *unit, *unit2, *newunit;
{
    int u, u2, u3, acpcost, m;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!in_play(newunit)) return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;  u3 = newunit->type;
    acpcost = uu_acp_to_build(u2, u3);
    if (acpcost < 1) return A_ANY_CANNOT_DO;
    /* Can't finish building a unit until we have the technology. */
    if (u_tech_to_build(u3) > 0) {
	if (unit->side == NULL) return A_ANY_ERROR;
	if (unit->side->tech[u3] < u_tech_to_build(u3)) return A_ANY_ERROR;
    }
    if (distance(unit->x, unit->y, newunit->x, newunit->y)
	> uu_sep_to_build_max(u, u3))
      return A_ANY_ERROR;
    /* Note that we should be able to build when inside the incomplete
       unit we're building. */
    if (unit2->transport != NULL
	&& completed(unit2->transport)
        && !uu_occ_can_build(unit2->transport->type, u2))
      return A_ANY_ERROR;
    if (!has_enough_acp(unit, acpcost)) return A_ANY_NO_ACP;
    for_all_material_types(m) {
    	if (unit2->supply[m] < um_to_build(u2, m))
    	  return A_ANY_NO_MATERIAL;
    	if (unit2->supply[m] < um_consume_per_build(u3, m))
    	  return A_ANY_NO_MATERIAL;
    }
    return A_ANY_OK;
}

make_unit_complete(unit)
Unit *unit;
{
    int u = unit->type, m;
    extern UnitVector *actionvector;

    /* Make this a "complete" but not a "fullsized" unit. */
    unit->cp = max(unit->cp, u_cp(u) / u_parts(u));
    unit->hp = u_hp(u) / u_parts(u);
    /* Christen our new unit. Its serial number (if it is a type that has
       one) was assigned just after its creation. */
    make_up_unit_name(unit);
    /* It also effectively starts viewing its surroundings. */
    if (unit->transport == NULL
	|| uu_occ_can_see(unit->type, unit->transport->type)) {
	cover_area(unit, unit->x, unit->y, 1);
    }
    /* Set all the supplies up to their unit-just-created levels. */
    for_all_material_types(m) {
	unit->supply[m] = max(unit->supply[m], um_created_supply(u, m));
	unit->supply[m] = min(unit->supply[m], um_storage(u, m));
    }
    init_unit_actorstate(unit);
    init_unit_plan(unit);
    /* Put this unit into action immediately, at full acp. */
    if (unit->act) {
	compute_acp(unit);
	if (unit->act->initacp > 0) {
	    actionvector = add_unit_to_vector(actionvector, unit, 0);
	}
    }
    record_event(H_UNIT_COMPLETED, -1, unit->id);
    /* (should add to any per-side tallies) */
    Dprintf("%s is completed\n", unit_desig(unit));
}

/* Repair action. */

prep_repair_action(unit, unit2, unit3)
Unit *unit, *unit2, *unit3;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_REPAIR;
    unit->act->nextaction.args[0] = unit3->id;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

do_repair_action(unit, unit2, unit3)
Unit *unit, *unit2, *unit3;
{
    int u, u2, u3, rep, m;
    int rslt = check_repair_action(unit, unit2, unit3);

    if (!valid(rslt)) return rslt;
    u = unit->type;  u2 = unit2->type;  u3 = unit3->type;
    rep = uu_repair(u2, u3);
    /* Add to the repairee's hit points. */
    unit3->hp += (rep / 100) + probability(rep % 100);
    /* Eat supplies used up by repair. */
    for_all_material_types(m) {
	unit2->supply[m] -= um_consumed_by_repair(u3, m);
    }
    use_up_acp(unit, uu_acp_to_repair(u2, u3));
    return A_ANY_DONE;
}

check_repair_action(unit, unit2, unit3)
Unit *unit, *unit2, *unit3;
{
    int u, u2, u3, acp, m;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!in_play(unit3)) return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;  u3 = unit3->type;
    acp = uu_acp_to_repair(u2, u3);
    if (acp < 1) return A_ANY_CANNOT_DO;
    if (uu_repair(u2, u3) <= 0) return A_ANY_ERROR;
    if (unit3->hp >= u_hp(u3)) return A_ANY_ERROR;
    if (unit2->hp < uu_hp_to_repair(u2, u3)) return A_ANY_ERROR;
    for_all_material_types(m) {
	if (unit2->supply[m] < um_to_repair(u2, m))
	  return A_ANY_NO_MATERIAL;
	if (unit2->supply[m] < um_consumed_by_repair(u3, m))
	  return A_ANY_NO_MATERIAL;
    }
    if (!has_enough_acp(unit, acp)) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* The disband action destroys a unit in an "orderly" fashion, and can be
   undertaken voluntarily. */

prep_disband_action(unit, unit2)
Unit *unit, *unit2;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_DISBAND;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

/* Basic disbanding action. */

do_disband_action(unit, unit2)
Unit *unit, *unit2;
{
    int u, u2, m, amt, disb;
    int rslt = check_disband_action(unit, unit2);

    if (!valid(rslt)) return rslt;
    u2 = unit2->type;
    /* Recover some percentage of the unit's supply. */
    for_all_material_types(m) {
    	if (um_supply_per_disband(u2, m) > 0 && unit2->supply[m] > 0) {
    	    amt = (unit2->supply[m] * um_supply_per_disband(u2, m)) / 100;
    	    /* Unit always loses the amount, whether or not distributed. */
    	    unit2->supply[m] -= amt;
    	    distribute_material(unit2, m, amt);
    	}
    }
    /* Remove hit points or kill the unit directly. */
    disb = u_hp_per_disband(u2);
    if (disb < unit2->hp) {
	unit2->hp -= disb;
    } else {
    	/* Pass around whatever we can get out of the unit itself. */
    	for_all_material_types(m) {
    	    if (um_recycleable(u2, m) > 0) {
    	    	distribute_material(unit2, m, um_recycleable(u2, m));
    	    }
    	}
    	/* should ensure vanish */
	kill_unit(unit2, H_UNIT_DISBANDED);
    }
    use_up_acp(unit, u_acp_to_disband(u2));
    return A_ANY_DONE;
}

/* Given a unit and a quantity of material, pass it out to nearby units. */

distribute_material(unit, m, amt)
Unit *unit;
int m, amt;
{
    Unit *unit2;

    /* Distribute to transport first. */
    if (amt > 0 && unit->transport != NULL) {
    	/* (should clip to capacity etc) */
    	unit->transport->supply[m] += amt;
    	amt = 0;
    }
    /* Then to any unit in the cell. */
    if (amt > 0) {
    }
    /* Then to any unit in an adjacent cell. */
    if (amt > 0) {
    }
    /* (should note anything that went to waste?) */
}

check_disband_action(unit, unit2)
Unit *unit, *unit2;
{
    int u, u2, acp;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    u = unit->type;  u2 = unit->type;
    acp = u_acp_to_disband(u2);
    if (acp < 1) return A_ANY_CANNOT_DO;
    if (u_hp_per_disband(unit2->type) <= 0)
      return A_ANY_ERROR; /* should warn instead */
    if (!has_enough_acp(unit, acp)) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* Create a new unit that is similar to the original one, and give it
   some of the parts of the original unit. */
/* (New unit in same hex if possible or else in random adjacent hex.) */

prep_transfer_part_action(unit, unit2, parts, unit3)
Unit *unit, *unit2, *unit3;
int parts;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_TRANSFER_PART;
    unit->act->nextaction.args[0] = parts;
    unit->act->nextaction.args[1] = unit3->id;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

do_transfer_part_action(unit, unit2, parts, unit3)
Unit *unit, *unit2, *unit3;
int parts;
{
    int u, u2, u3, acp;
    Unit *newunit;
    int rslt = check_transfer_part_action(unit, unit2, parts, unit3);

    if (!valid(rslt)) return rslt;
    u = unit->type;  u2 = unit2->type;
    if (unit3 == NULL) {
	/* Create a new unit with parts from unit2. */
	if ((unit3 = create_unit(u, TRUE)) != NULL) {
	    set_unit_side(newunit, unit->side);
	    /* Always number the unit when first created. */
	    assign_unit_number(newunit);
	    /* (fill in more slots of new unit) */
	} else {
	    /* we have a problem... */
	}
    } else {
	/* Increase the unit3's hp by what's in this unit, and cap it. */
	unit3->hp += unit->hp;
	unit3->hp = max(unit3->hp, u_hp(u));
	/* Should affect morale etc according to proportions of mixing */
    }
    /* Need to tweak parts in unit2 also */
    if (0 /* transferred all parts in unit2 */) {
	kill_unit(unit2, -1);  /* need a merge kill-reason? */
    }
    acp = u_acp_to_transfer_part(u2);
    use_up_acp(unit, acp);
    return A_ANY_DONE;
}

check_transfer_part_action(unit, unit2, parts, unit3)
Unit *unit, *unit2, *unit3;
int parts;
{
    int u, u2, u3, acp;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    /* unit3 can be null? */
    u = unit->type;  u2 = unit2->type;
    acp = u_acp_to_transfer_part(u2);
    if (acp < 1) return A_ANY_CANNOT_DO;
    if (u_parts(u2) <= 1) return A_ANY_ERROR;
    if (!has_enough_acp(unit, acp)) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* Prepare to change the type of a unit. */

prep_change_type_action(unit, unit2, u3)
Unit *unit, *unit2;
int u3;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_CHANGE_TYPE;
    unit->act->nextaction.args[0] = u3;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

/* Actually change the type of a unit. */

do_change_type_action(unit, unit2, u3)
Unit *unit, *unit2;
int u3;
{
    int u, u2;
    int rslt = check_change_type_action(unit, unit2, u3);

    if (!valid(rslt)) return rslt;
    u = unit->type;  u2 = unit2->type;
    change_unit_type(unit2, u3);
    update_unit_display(unit2->side, unit2, TRUE);
    use_up_acp(unit, uu_acp_to_change_type(u2, u3));
    return A_ANY_DONE;
}

check_change_type_action(unit, unit2, u3)
Unit *unit, *unit2;
int u3;
{
    int u, u2, acp;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!is_unit_type(u3)) return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    acp = uu_acp_to_change_type(u2, u3);
    if (acp < 1) return A_ANY_CANNOT_DO;
    /* should check allowable side */
    if (!has_enough_acp(unit, acp)) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* Force a unit to change to a given side. */

/* (what about occs, garrisons, plans?) */

prep_change_side_action(unit, unit2, side)
Unit *unit, *unit2;
Side *side;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_CHANGE_SIDE;
    unit->act->nextaction.args[0] = side_number(side);
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

do_change_side_action(unit, unit2, side)
Unit *unit, *unit2;
Side *side;
{
    int rslt = check_change_side_action(unit, unit2, side);

    if (!valid(rslt)) return rslt;
    if (side_controls_unit(unit->side, unit2)) {
	/* If we own it, we can just change it. */
	unit_changes_side(unit2, side, -1, -1);
	rslt = A_ANY_DONE;
    } else {
	rslt = A_ANY_ERROR;
    }
    use_up_acp(unit, u_acp_to_change_side(unit2->type));
    return rslt;
}

check_change_side_action(unit, unit2, side)
Unit *unit, *unit2;
Side *side;
{
    int u, u2, acp;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!side_in_play(side)) return A_ANY_ERROR;
    if (!unit_allowed_on_side(unit2, side)) return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    acp = u_acp_to_change_side(u2);
    if (acp < 1) return A_ANY_CANNOT_DO;
    if (!has_enough_acp(unit, acp)) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* Change the terrain in the hex to something else. */

/* We don't need to ensure that the unit can exist on the new terrain
   type, because the designer is presumed to have set things up sensibly,
   because the unit might be in an appropriate transport, or because
   there is some actual use in such a bizarre shtick. */

/* What if engineers dig hole underneath enemy unit?  Should this be
   possible, or should there be a "can-dig-under-enemy" parm?  */

prep_alter_cell_action(unit, unit2, x, y, t)
Unit *unit, *unit2;
int x, y, t;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_ALTER_TERRAIN;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = t;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

do_alter_cell_action(unit, unit2, x, y, t)
Unit *unit, *unit2;
int x, y, t;
{
    int u, u2, oldt, acpr, acpa;
    Side *side;
    int rslt = check_alter_cell_action(unit, unit2, x, y, t);

    if (!valid(rslt)) return rslt;
    u = unit->type;  u2 = unit2->type;
    oldt = terrain_at(x, y);
    /* Change the terrain to the new type. */
    set_terrain_at(x, y, t);
    /* Let everybody see what has happened. */
    for_all_sides(side) {
    	if (g_see_all() || g_see_terrain_always() || side == unit->side) {
    	    update_cell_display(side, x, y, TRUE);
    	}
    }
    acpr = ut_acp_to_remove_terrain(u2, oldt);
    acpa = ut_acp_to_add_terrain(u2, t);
    use_up_acp(unit, acpr + acpa);
    return A_ANY_DONE;
}

check_alter_cell_action(unit, unit2, x, y, t)
Unit *unit, *unit2;
int x, y, t;
{
    int u, u2, oldt, acpr, acpa;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!in_area(x, y)) return A_ANY_ERROR;
    if (!is_terrain_type(t) || !t_is_cell(t)) return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    oldt = terrain_at(x, y);
    acpr = ut_acp_to_remove_terrain(u2, oldt);
    acpa = ut_acp_to_add_terrain(u2, t);
    if (acpr < 1 || acpa < 1) return A_ANY_CANNOT_DO;
    if (distance(unit2->x, unit2->y, x, y) > ut_alter_range(u2, t))
      return A_ANY_ERROR;
    if (!has_enough_acp(unit, acpr + acpa)) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* Add a border, connection, or coating. */

prep_add_terrain_action(unit, unit2, x, y, dir, t)
Unit *unit, *unit2;
int x, y, dir, t;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_ADD_TERRAIN;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = dir;
    unit->act->nextaction.args[3] = t;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

do_add_terrain_action(unit, unit2, x, y, dir, t)
Unit *unit, *unit2;
int x, y, dir, t;
{
    int u, oldval, newval;
    Side *side;
    int rslt = check_add_terrain_action(unit, unit2, x, y, dir, t);

    if (!valid(rslt)) return rslt;
    u = unit->type;
    switch (t_subtype(t)) {
      case cellsubtype:
      	/* Will never happen. */
      	break;
      case bordersubtype:
      	oldval = border_at(x, y, dir, t);
      	newval = TRUE;
	set_border_at(x, y, dir, t, newval);
      	break;
      case connectionsubtype:
      	oldval = connection_at(x, y, dir, t);
      	newval = TRUE;
	set_connection_at(x, y, dir, t, newval);
      	break;
      case coatingsubtype:
    	oldval = aux_terrain_at(x, y, t);
      	/* Interpret "dir" as depth of coating to add. */
    	newval = min(oldval + dir, tt_coat_max(terrain_at(x, y), t));
    	set_aux_terrain_at(x, y, t, newval);
      	break;
    }
    /* Let everybody see what has happened. */
    for_all_sides(side) {
    	if (g_see_all() || g_see_terrain_always() || side == unit->side) {
    	    update_cell_display(side, x, y, TRUE);
    	}
    }
    use_up_acp(unit, (newval != oldval ? ut_acp_to_add_terrain(u, t) : 1));
    return A_ANY_DONE;
}

check_add_terrain_action(unit, unit2, x, y, dir, t)
Unit *unit, *unit2;
int x, y, dir, t;
{
    int u, u2, acp;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!inside_area(x, y)) return A_ANY_ERROR;
    if (!is_terrain_type(t) || t_is_cell(t)) return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    acp = ut_acp_to_add_terrain(u2, t);
    if (acp < 1) return A_ANY_CANNOT_DO;
    if (distance(unit->x, unit->y, x, y) > ut_alter_range(u, t))
      return A_ANY_ERROR;
    if (!has_enough_acp(unit, acp)) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* Remove a border, connection, or coating. */

prep_remove_terrain_action(unit, unit2, x, y, dir, t)
Unit *unit, *unit2;
int x, y, dir, t;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_REMOVE_TERRAIN;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = dir;
    unit->act->nextaction.args[3] = t;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

do_remove_terrain_action(unit, unit2, x, y, dir, t)
Unit *unit, *unit2;
int x, y, dir, t;
{
    int u, oldval, newval;
    Side *side;
    int rslt = check_remove_terrain_action(unit, unit2, x, y, dir, t);

    if (!valid(rslt)) return rslt;
    u = unit->type;
    switch (t_subtype(t)) {
      case cellsubtype:
      	/* Will never happen. */
      	break;
      case bordersubtype:
	oldval = border_at(x, y, dir, t);
	newval = FALSE;
	set_border_at(x, y, dir, t, newval);
      	break;
      case connectionsubtype:
	oldval = connection_at(x, y, dir, t);
	newval = FALSE;
	set_connection_at(x, y, dir, t, newval);
      	break;
      case coatingsubtype:
    	oldval = aux_terrain_at(x, y, t);
       	/* Interpret "dir" as depth of coating to remove. */
   	newval = max(oldval - dir, 0);
   	/* If newval drops below the min coating depth, coating will vanish. */
    	if (newval < tt_coat_min(terrain_at(x, y), t)) newval = 0;
    	set_aux_terrain_at(x, y, t, newval);
      	break;
    }
    /* Let everybody see what has happened. */
    for_all_sides(side) {
    	if (g_see_all() || g_see_terrain_always() || side == unit->side) {
    	    update_cell_display(side, x, y, TRUE);
    	}
    }
    use_up_acp(unit, ut_acp_to_remove_terrain(u, t));
    return A_ANY_DONE;
}

check_remove_terrain_action(unit, unit2, x, y, dir, t)
Unit *unit, *unit2;
int x, y, dir, t;
{
    int u, u2, acp;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!inside_area(x, y)) return A_ANY_ERROR;
    /* should check dir also */
    if (!is_terrain_type(t) || t_is_cell(t)) return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    acp = ut_acp_to_remove_terrain(u2, t);
    if (acp < 1) return A_ANY_CANNOT_DO;
    if (distance(unit->x, unit->y, x, y) > ut_alter_range(u, t))
      return A_ANY_ERROR;
    if (!has_enough_acp(unit, acp)) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* Execute a given action on a given unit. */

/* (assumes unit can act in the first place - valid?) */

execute_action(unit, action)
Unit *unit;
Action *action;
{
    char *argtypestr;
    int rslt = A_ANY_ERROR, n, (*fn)(), i;
    long args[4];
    Unit *unit2, *argunit;
    Side *argside, *side2;

    Dprintf("%s doing %s with %d acp left\n",
	    unit_desig(unit), action_desig(action), unit->act->acp);

    if (!alive(unit) || !unit->act || unit->act->acp <= 0) return A_ANY_ERROR;

    argtypestr = actiondefns[(int) action->type].argtypes;
    n = strlen(argtypestr);
    for (i = 0; i < n; ++i) {
	switch (argtypestr[i]) {
	  case 'n':
	  case 'u':
	  case 'm':
	  case 't':
	  case 'x':
	  case 'y':
	  case 'z':
	  case 'd':
	    args[i] = action->args[i];
	    break;
	  case 'U':
	    argunit = find_unit(action->args[i]);
	    if (argunit == NULL) {
		/* an error, should do run_warning */
	    }
	    args[i] = (long) argunit;
	    break;
	  case 'S':
	    argside = side_n(action->args[i]);
	    args[i] = (long) argside;
	    break;
	  default:
	    /* should warn */
	    break;
	}
    }
    fn = actiondefns[(int) action->type].dofn;
    if (action->actee == 0) {
	unit2 = unit;
    } else {
	unit2 = find_unit(action->actee);
    }
    if (unit2 == NULL) {
	return A_ANY_ERROR;
    }
    switch (n) {
      case 0:
	rslt = (*fn)(unit, unit2);
	break;
      case 1:
	rslt = (*fn)(unit, unit2, args[0]);
	break;
      case 2:
	rslt = (*fn)(unit, unit2, args[0], args[1]);
	break;
      case 3:
	rslt = (*fn)(unit, unit2, args[0], args[1], args[2]);
	break;
      case 4:
	rslt = (*fn)(unit, unit2, args[0], args[1], args[2], args[3]);
	break;
      default:
	case_panic("action arg count", n);
    }
    Dprintf("%s action %s result is %s, %d acp left\n",
	    unit_desig(unit), action_desig(action), hevtdefns[rslt].name,
	    (unit->act ? unit->act->acp : -9999));
    if (unit->plan) {
    	unit->plan->lastaction = *action;
    	unit->plan->lastresult = rslt;
    }
    if (unit->side && side_has_ai(unit->side)) {
	ai_react_to_action_result(unit->side, unit, rslt);
    }
    if (unit->side && side_has_display(unit->side)) {
	update_action_result_display(unit->side, unit, rslt);
    }
    /* Show other sides that some action has occurred. */
    for_all_sides(side2) {
	if (side_has_display(side2)) {
	    update_side_display(side2, unit->side, TRUE);
	}
    }
    /* Check any scorekeepers that run after each action. */
    if (anypostactionscores) {
	check_post_action_scores(unit, action, rslt);
    }
    /* Return success/failure so caller can use. */
    return rslt;
}

/* Basic check that unit has sufficient acp to do an action. */

has_enough_acp(unit, acp)
Unit *unit;
int acp;
{
    return ((unit->act->acp - acp) >= u_acp_min(unit->type));
}

/* This is true iff the unit has enough of each sort of supply to act. */

has_supply_to_act(unit)
Unit *unit;
{
    int m;

    for_all_material_types(m) {
	if (unit->supply[m] < um_to_act(unit->type, m)) return FALSE;
    }
    return TRUE;
}

/* Make the consumed acp disappear, but not go below the minimum possible. */

use_up_acp(unit, acp)
Unit *unit;
int acp;
{
    int oldacp, newacp, acpmin;

    /* This can sometimes be called on dead or non-acting units,
       so check first. */
    if (alive(unit) && unit->act) {
    	oldacp = unit->act->acp;
	newacp = oldacp - acp;
	acpmin = u_acp_min(unit->type);
	unit->act->acp = max(newacp, acpmin);
	/* Maybe modify the unit's display. */
	if (oldacp != unit->act->acp) {
		update_unit_display(unit->side, unit);
	}
    }
}

/* Functions returning general abilities of a unit. */

can_create(unit)
Unit *unit;
{
    int u = unit->type, u2;
	
    for_all_unit_types(u2) {
	if (uu_acp_to_create(u, u2) > 0) return TRUE;
    }
    return FALSE;
}

can_complete(unit)
Unit *unit;
{
    int u = unit->type, u2;
	
    for_all_unit_types(u2) {
	if (uu_acp_to_build(u, u2) > 0) return TRUE;
    }
    return FALSE;
}

/* This tests whether the given unit is capable of doing repair. */

can_repair(unit)
Unit *unit;
{
    int u = unit->type, u2;
	
    for_all_unit_types(u2) {
	if (uu_acp_to_repair(u, u2) > 0) return TRUE;
    }
    return FALSE;
}

can_attack(unit)
Unit *unit;
{
    return type_can_attack(unit->type);
}

type_can_attack(u)
int u;
{
    int u2;
	
    for_all_unit_types(u2) {
	if (uu_hit(u, u2) > 0 && uu_damage(u, u2) > 0) return TRUE;
    }
    return FALSE;
}

can_fire(unit)
Unit *unit;
{
	return type_can_fire(unit->type);
}

type_can_fire(u)
int u;
{
    int u2;
	
	if (u_acp_to_fire(u) == 0) return FALSE;
    for_all_unit_types(u2) {
	if (uu_hit(u, u2) > 0 && uu_damage(u, u2) > 0) return TRUE;
    }
    return FALSE;
}

can_detonate(unit)
Unit *unit;
{
    return (u_acp_to_detonate(unit->type) > 0);
}

can_disband_at_all(side, unit)
Side *side;
Unit *unit;
{
    return (side != NULL
    	    && alive(unit)
	    && side_controls_unit(side, unit)
	    && (u_acp_to_disband(unit->type) > 0
	        || !completed(unit)
	        || side->designer));
	    	
}

/* Compose a legible description of a given action. */

char *
action_desig(act)
Action *act;
{
    int i;
    char ch, *str;

    if (act == NULL) return "?null action?";
    if (act->type == A_NONE) return "[]";
    if (actiondesigbuf == NULL) actiondesigbuf = xmalloc(BUFSIZE);
    str = actiondesigbuf;
    sprintf(str, "[%s", actiondefns[act->type].name);
    for (i = 0; i < strlen(actiondefns[act->type].argtypes); ++i) {
	ch = (actiondefns[act->type].argtypes)[i];
	switch (ch) {
	  case 'U':
	    tprintf(str, " \"%s\"",
		    unit_desig(find_unit(act->args[i])));
	    break;
	  default:
	    tprintf(str, " %d", act->args[i]);
	}
    }
    if (act->actee != 0) {
	tprintf(str, " (#%d)", act->actee);
    }
    strcat(str, "]");
    return actiondesigbuf;
}
