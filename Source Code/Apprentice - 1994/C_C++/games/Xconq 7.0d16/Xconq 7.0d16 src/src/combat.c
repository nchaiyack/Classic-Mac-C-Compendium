/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* This file manages combat actions specifically. */

/* Rules of combat: the attacker hits the defender ("other") unit and its
   occupants, but the damage does not take effect right away.  If counter
   attacks are possible in this period, the defender always does so, with
   the same odds.  If the defender dies, then the attacker moves into the
   hex.  If the attacker dies, nothing happens.  If both survive, then the
   attacker may attempt to capture the defender. */

#include "conq.h"

extern int *occdeath;

char *summarize_units();

int maxdetonaterange = 1;

/* Remember what the main units involved are, so display is handled relative */
/* to them and not to any occupants. */

Unit *amain, *omain;

/* Hits on main units saved up, hits on occupants happen immediately. */

int ahit, ohit;

/* ... but the data is saved anyway, for message generation. */

int *occhits = NULL, *occkills = NULL;

int numsoundplays;

prep_attack_action(unit, unit2, defender, n)
Unit *unit, *unit2, *defender;
int n;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_ATTACK;
    unit->act->nextaction.args[0] = defender->id;
    unit->act->nextaction.args[1] = n;
    return TRUE;
}

/* This is an attack on a given unit at a given level of commitment. */

do_attack_action(unit, unit2, defender, n)
Unit *unit, *unit2, *defender;
int n;
{
    int u, u2, u3, withdrawchance, surrenderchance;
    int rslt = check_attack_action(unit, unit2, defender, n);

    if (!valid(rslt)) return rslt;
    u = unit->type;  u2 = unit2->type;  u3 = defender->type;
    /* Defender might be a type that can sneak away to avoid attack. */
    if ((withdrawchance = uu_withdraw_per_attack(u2, u3)) > 0) {
	if (probability(withdrawchance)) {
	    if (retreat_unit(defender, unit2)) {
    		if (alive(unit)) use_up_acp(unit, uu_acp_to_attack(u2, u3));
		return A_ANY_DONE;
	    }
	}
    }
    /* (should also impl chance to surrender immediately) */
    one_attack(unit, defender);
    if (alive(unit)) use_up_acp(unit, uu_acp_to_attack(u2, u3));
    /* The defender in an attack has to take time to defend itself. */
    if (alive(defender)) use_up_acp(defender, uu_acp_to_defend(u2, u3));
    return A_ANY_DONE;
}

check_attack_action(unit, unit2, defender, n)
Unit *unit, *unit2, *defender;
int n;
{
    int u, u2, u3, u2x, u2y, dfx, dfy;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!in_play(defender)) return A_ANY_ERROR;
    /* We can't attack ourselves. */
    if (unit2 == defender) return A_ANY_ERROR;
    /* (should allow some self-attacks sometimes?) */
    if (unit2->side != NULL && unit2->side == defender->side) return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;  u3 = defender->type;
    if (uu_acp_to_attack(u2, u3) < 1) return A_ANY_CANNOT_DO;
    if (!has_enough_acp(unit, uu_acp_to_attack(u2, u3))) return A_ANY_NO_ACP;
    u2x = unit2->x;  u2y = unit2->y;
    dfx = defender->x;  dfy = defender->y;
    if (!between(0, distance(u2x, u2y, dfx, dfy), uu_attack_range(u2, u3))) {
	return A_ANY_ERROR;
    }
    if (uu_hit(u2, u3) <= 0) return A_ANY_ERROR;
    /* (should prorate ammo needs by intensity of attack) */
    if (!enough_ammo(unit2, defender)) return A_ANY_ERROR;
    /* Allow attacks if zero damage, this amounts to "harassment". */
    return A_ANY_OK;
}

prep_overrun_action(unit, unit2, x, y, z, n)
Unit *unit, *unit2;
int x, y, z, n;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_OVERRUN;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = z;
    unit->act->nextaction.args[3] = n;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

/* This is an attack on a given unit at a given level of commitment. */

do_overrun_action(unit, unit2, x, y, z, n)
Unit *unit, *unit2;
int x, y, z, n;
{
    int u, u2, u3, cleared = TRUE, cost;
    Unit *defender;
    int rslt = check_overrun_action(unit, unit2, x, y, z, n);

    if (!valid(rslt)) return rslt;
    u = unit->type;  u2 = unit2->type;
    for_all_stack(x, y, defender) {
	u3 = defender->type;
	/* Don't attack any of our buddies. */
	if (unit2->side == defender->side) continue;
	one_attack(unit, defender);
	if (alive(unit)) use_up_acp(unit, uu_acp_to_attack(u2, u3));
	/* The target of an attack has to take time to defend itself. */
	if (alive(defender)) {
	    use_up_acp(defender, uu_acp_to_defend(u, u3));
	    cleared = FALSE;
	}
    }
    if (cleared) {
	/* Try to enter the cleared cell now - might still have
	   friendlies filling it up already, so check first. */
	if (can_occupy_cell(unit, x, y)) {
	    cost = move_unit(unit, x, y);
	    /* Note that we'll say the action succeeded even if
	       the cell did not have enough room for us to actually
	       be in it, which is a little weird. */
	}
	rslt = A_OVERRUN_SUCCEEDED;
    } else {
	rslt = A_OVERRUN_FAILED;
    }
    return rslt;
}

check_overrun_action(unit, unit2, x, y, z, n)
Unit *unit, *unit2;
int x, y, z, n;
{
    int u, u2, u2x, u2y;
    Unit *defender;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!inside_area(x, y)) return A_ANY_ERROR;
    if (n == 0) return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    if (!has_enough_acp(unit, 1)) return A_ANY_NO_ACP;
    u2x = unit2->x;  u2y = unit2->y;
    if (!between(0, distance(u2x, u2y, x, y), 1)) return A_ANY_TOO_FAR;
    for_all_stack(x, y, defender) {
	/* (should test if units here can be attacked en masse) */
    	/* (should prorate ammo needs by intensity of overrun) */
    	if (!enough_ammo(unit2, defender)) return A_ANY_NO_MATERIAL;
    }
    return A_ANY_OK;
}

/* Return true if the attacker defeated the defender, and can therefore
   try to move into the defender's old position. */

one_attack(atker, defender)
Unit *atker, *defender;
{
    int u, ax = atker->x, ay = atker->y, ox = defender->x, oy = defender->y;
    Side *as = atker->side, *os = defender->side;

    amain = atker;  omain = defender;
    ahit = ohit = 0;
    if (occhits  == NULL) occhits  = (int *) xmalloc(numutypes * sizeof(int));
    if (occkills == NULL) occkills = (int *) xmalloc(numutypes * sizeof(int));
    for_all_unit_types(u) occhits[u] = occkills[u] = 0;
    numsoundplays = 0;
    attack_unit(atker, defender);
    /* (this test should account for counterattacking occupants too,
       handle them in here somehow) */
    if (1 /* uu_counter_strength(defender->type, atker->type) > 0 */) {
	attack_unit(defender, atker);
    }
    reckon_damage();
    see_exact(as, ax, ay);
    see_exact(as, ox, oy);
    see_exact(os, ax, ay);
    see_exact(os, ox, oy);
    update_cell_display(as, ax, ay, TRUE);
    update_cell_display(as, ox, oy, TRUE);
    update_cell_display(os, ax, ay, TRUE);
    update_cell_display(os, ox, oy, TRUE);
    all_see_hex(ax, ay);
    all_see_hex(ox, oy);
    attempt_to_capture_unit(atker, defender);
    return (alive(atker) && unit_at(ox, oy) == NULL);
}

/* Shooting at a given unit. */

prep_fire_at_action(unit, unit2, defender, m)
Unit *unit, *unit2, *defender;
int m;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_FIRE_AT;
    unit->act->nextaction.args[0] = defender->id;
    unit->act->nextaction.args[1] = m;
    return TRUE;
}

do_fire_at_action(unit, unit2, defender, m)
Unit *unit, *unit2, *defender;
int m;
{
    int ux = unit->x, uy = unit->y, uz = unit->z, ox, oy, oz, u;
    Unit *other;
    int rslt = check_fire_at_action(unit, unit2, defender, m);

    if (!valid(rslt)) return rslt;
    update_fire_at_display(unit->side, unit, defender, m, TRUE);
    update_fire_at_display(defender->side, unit, defender, m, TRUE);
    ox = defender->x;  oy = defender->y;  oz = defender->z;
    amain = unit;  omain = defender;
    ahit = ohit = 0;
    if (occhits  == NULL) occhits  = (int *) xmalloc(numutypes * sizeof(int));
    if (occkills == NULL) occkills = (int *) xmalloc(numutypes * sizeof(int));
    for_all_unit_types(u) occhits[u] = occkills[u] = 0;
    fire_on_unit(unit, defender);
    reckon_damage();
    if (alive(unit)) use_up_acp(unit, u_acp_to_fire(unit->type));
    /*	if (alive(defender)) use_up_acp(defender, 1); */
    /* Each side sees what happened to its own unit. */
    update_unit_display(unit->side, unit);
    update_unit_display(defender->side, defender);
    /* The attacking side also sees the remote cell. */
    update_cell_display(unit->side, ox, oy, TRUE);
    update_cell_display(defender->side, ox, oy, TRUE);
    /* Victim might see something in attacker's cell. */
    update_cell_display(defender->side, ux, uy, TRUE);
    /* Actually, everybody might be seeing the combat. */
    all_see_hex(ux, uy);
    all_see_hex(ox, oy);
    /* Always expend the ammo (but only if m is a valid mtype). */
    return A_ANY_DONE;
}

/* Test a fire action for plausibility. */

check_fire_at_action(unit, unit2, unit3, m)
Unit *unit, *unit2, *unit3;
int m;
{
    int u, u2, u3, ux, uy, uz, acp, dist;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!in_play(unit3)) return A_ANY_ERROR;
    /* We can't attack ourselves. */
    if (unit2 == unit3) return A_ANY_ERROR;
    u = unit->type; u2 = unit2->type;  u3 = unit3->type;
    ux = unit->x;  uy = unit->y;  uz = unit->z;
    acp = u_acp_to_fire(u2);
    if (acp < 1) return A_ANY_CANNOT_DO;
    if (!has_enough_acp(unit, acp)) return A_ANY_NO_ACP;
    /* Check that target is in range. */
    dist = distance(ux, uy, unit3->x, unit3->y);
    if (dist > u_range(u2)) return A_FIRE_AT_TOO_FAR;
    if (dist < u_range_min(u2)) return A_ANY_TOO_NEAR;
    /* (should allow some self-attacks sometimes?) */
    if (unit2->side != NULL && unit2->side == unit3->side) return A_ANY_ERROR;
    /* Check intervening elevations. */
    if (found_blocking_elevation(u2, ux, uy, uz, u3, unit3->x, unit3->y, unit3->z))
      return A_ANY_ERROR;
    /* Check for enough of right kind of ammo. */
    if (is_material_type(m)) {
    	if (unit->supply[m] == 0) return A_ANY_NO_MATERIAL;
    } else {
    	if (!enough_ammo(unit2, unit3)) return A_ANY_NO_MATERIAL;
    }
    return A_ANY_OK;
}

/* Shooting at a given location. */

prep_fire_into_action(unit, unit2, x, y, z, m)
Unit *unit, *unit2;
int x, y, z, m;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_FIRE_INTO;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = z;
    unit->act->nextaction.args[3] = m;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

/* One can always shoot, if the hex is visible, but there might not
   not be anything to hit!  No counterattacks when shooting, and the
   results might not be visible to the shooter. */

do_fire_into_action(unit, unit2, x, y, z, m)
Unit *unit, *unit2;
int x, y, z, m;
{
    int ux = unit->x, uy = unit->y, uz = unit->z, ox, oy, oz, u;
    Unit *other;
    int rslt = check_fire_into_action(unit, unit2, x, y, z, m);

    if (!valid(rslt)) return rslt;
    /* If any units at target, hit them. */
    for_all_stack(x, y, other) {
	ox = other->x;  oy = other->y;  oz = other->z;
	amain = unit;  omain = other;
	ahit = ohit = 0;
	if (occhits  == NULL) occhits  = (int *) xmalloc(numutypes * sizeof(int));
	if (occkills == NULL) occkills = (int *) xmalloc(numutypes * sizeof(int));
	for_all_unit_types(u) occhits[u] = occkills[u] = 0;
	fire_on_unit(unit, other);
	reckon_damage();
	/* Each side sees what happened to its own unit. */
	update_unit_display(unit->side, unit);
	update_unit_display(other->side, other);
	/* The attacking side also sees the remote cell. */
	update_cell_display(unit->side, ox, oy, TRUE);
	update_cell_display(other->side, ox, oy, TRUE);
	/* Victim might see something in attacker's cell. */
	update_cell_display(other->side, ux, uy, TRUE);
	/* Actually, everybody might be seeing the combat. */
	all_see_hex(ux, uy);
	all_see_hex(ox, oy);
	/* don't take moves though! */
    }
    /* Always expend the ammo (but only if r is a valid rtype). */
    /* We're always successful, even though the bombardment may have
       had little or no actual effect. */
    return A_ANY_DONE;
}

/* Test a shoot action for plausibility. */

check_fire_into_action(unit, unit2, x, y, z, m)
Unit *unit, *unit2;
int x, y, z, m;
{
    int u, u2, ux, uy, uz, acp, dist;

    if (!in_play(unit)) return A_ANY_ERROR;
    ux = unit->x;  uy = unit->y;  uz = unit->z;
    /* Check that target location is meaningful. */
    if (!inside_area(x, y)) return A_FIRE_INTO_OUTSIDE_WORLD;
    u = unit->type;  u2 = unit2->type;
    acp = u_acp_to_fire(u2);
    if (acp < 1) return A_ANY_CANNOT_DO;
    if (!has_enough_acp(unit, acp)) return A_ANY_NO_ACP;
    /* Check that target is in range. */
    dist = distance(ux, uy, x, y);
    if (dist > u_range(u2)) return A_FIRE_INTO_TOO_FAR;
    if (dist < u_range_min(u2)) return A_FIRE_INTO_TOO_NEAR;
    /* Check intervening elevations. */
    if (found_blocking_elevation(u2, ux, uy, uz, NONUTYPE, x, y, z))
      return A_ANY_ERROR;
    /* Check for enough of right kind of ammo. */
    if (is_material_type(m)) {
    	if (unit->supply[m] == 0) return A_ANY_NO_MATERIAL;
    } else {
    	/* should just assume amount is appropriate? */
    }
    return A_ANY_OK;
}

found_blocking_elevation(u, ux, uy, uz, u2, u2x, u2y, u2z)
int u, ux, uy, uz, u2, u2x, u2y, u2z;
{
    int maxrise = u_elev_at_max_range(u);

    if (world_is_flat()) return FALSE;
    /* Adjacent cells can't be screened by elevation. */
    /* (should accommodate possibility that target is at top of
       cliff in adj and back away from its edge, thus screened) */
    if (distance(ux, uy, u2x, u2y) <= 1) return FALSE;
    /* (should add scan of path) */
    return FALSE;
}
 
fire_on_unit(atker, other)
Unit *atker, *other;
{
    int m, effect;
    int dist = distance(atker->x, atker->y, other->x, other->y);

    /* this should be an event needing a response */
/*    wake_unit(other, TRUE, WAKEENEMY, atker);  */
    if (alive(atker) && alive(other)) {
	if (enough_ammo(atker, other)) {
	    maybe_hit_unit(atker, other, (dist > u_hit_falloff_range(atker->type)));
	    for_all_material_types(m) {
		if (um_hitby(other->type, m) > 0) {
		    atker->supply[m] -= um_hitswith(atker->type, m);
		}
	    }
	}
    }
}

/* Test to see if enough ammo is available to make the attack.
   Need enough of *all* types - semi-bogus but too complicated otherwise? */

enough_ammo(unit, other)
Unit *unit, *other;
{
    int m;

    for_all_material_types(m) {
	if (um_hitby(other->type, m) > 0 &&
	    unit->supply[m] < um_hitswith(unit->type, m))
	  return FALSE;
    }
    return TRUE;
}

/* Single attack, no counterattack.  Check and use ammo - usage independent
   of outcome, but types used depend on unit types involved. */

attack_unit(atker, other)
Unit *atker, *other;
{
    int m;

    /* this should be an event needing a response */
/*    wake_unit(other, TRUE, WAKEENEMY, atker);  */
    if (alive(atker) && alive(other)) {
	if (enough_ammo(atker, other)) {
	    maybe_hit_unit(atker, other, FALSE);
	    for_all_material_types(m) {
		if (um_hitby(other->type, m) > 0) {
		    atker->supply[m] -= um_hitswith(atker->type, m);
		}
	    }
	}
    }
}

/* Make a single hit and maybe hit some passengers also.  Power of hit
   is constant, but chance is affected by terrain, quality,
   and occupants' protective abilities.  If a hit is successful, it may
   have consequences on the defender's occupants, but limited by the
   protection that the transport provides. */

maybe_hit_unit(atker, other, fallsoff)
int fallsoff;
Unit *atker, *other;
{
    int chance, t, hit = 0, a = atker->type, o = other->type;
    int dist, disthit, rangedelta, hitdelta, rangeamt;
    Unit *occ;
    Side *as = atker->side, *side3;

    chance = uu_hit(a, o);
    t = terrain_at(atker->x, atker->y);
    chance = (chance * ut_attack_terrain_effect(a, t)) / 100;
    t = terrain_at(other->x, other->y);
    chance = (chance * ut_defend_terrain_effect(o, t)) / 100;
    /* (should modify due to cxp of attacker and defender) */
#if 0 /* make sure protection effect defaults to 100% */
    for_all_occupants(other, occ) {
	chance = (chance * uu_protect(occ->type, o)) / 100;
    }
#endif
    if (fallsoff) {
	dist = distance(atker->x, atker->y, other->x, other->y);
    	disthit = uu_hit_max_range_effect(a, o);
    	rangedelta = u_range(a) - u_hit_falloff_range(a);
    	rangeamt = dist - u_hit_falloff_range(a);
    	hitdelta = uu_hit(a, o) - disthit;
    	chance = uu_hit(a, o) - ((uu_hit(a, o) - disthit) * rangeamt) / rangedelta;
    }
    /* Compute the hit itself. */
    if (probability(chance)) hit = roll_dice(uu_damage(a, o));
    Dprintf("Probability of hit was %d%%, ", chance);
    if (hit > 0) {
    	Dprintf("damage will be %d hp", hit);
    } else {
    	Dprintf("missed");
    }
    Dprintf("\n");
    /* (should record a raw statistic) */
    if (hit > 0) {
	chance = uu_retreat_chance(a, o);
	/* (should adjust chance by morale etc) */
	if (probability(chance)) {
	    if (retreat_unit(other, atker)) {
#if 0
		notify(as, "%s runs away!", unit_handle(as, other));
		notify(os, "%s runs away!", unit_handle(os, other));
#endif
		hit = 0; /* should only be reduced hit, may still be > 0 */
	    }
	}
    }
    hit_unit(other, hit, atker);
    for_all_sides(side3) {
    	if (active_display(side3)) {
    	    if (g_see_all()
    	    	|| (side3 == atker->side || side3 == other->side)) {
    	    	draw_blast(other, side3, hit);
    	    }
    	}
    }
    for_all_occupants(other, occ) {
	if (probability(100 - uu_protection(o, occ->type))) {
	    maybe_hit_unit(atker, occ, fallsoff);
	}
    }
    /* We get combat experience only if there could have been some damage. */
    if (chance > 0) {
    	if (atker->cxp < u_cxp_max(a)) atker->cxp += uu_cxp_per_combat(a, o);
    	if (other->cxp < u_cxp_max(o)) other->cxp += uu_cxp_per_combat(o, a);
    	/* (should occupants get cxp also?) */
    }
}

/* Do the hit itself. */

hit_unit(unit, hit, atker)
Unit *unit, *atker;
int hit;
{
    int u = unit->type;

    /* Some units might detonate automatically upon being hit. */
    if (hit > 0
        && atker != NULL
        && probability(uu_detonation_on_hit(u, atker->type))) {
    	detonate_unit(unit, unit->x, unit->y, unit->z);
    }
    /* Record the loss of hp. */
    unit->hp2 -= hit;
    /* Some units detonate automatically just before dying. */
    if (hit > 0
        && unit->hp2 <= 0
        && probability(u_detonate_on_death(u))) {
	detonate_unit(unit, unit->x, unit->y, unit->z);
    }
}

/* Hits on the main units have to be done later, so that mutual
   destruction works properly.  This function also does all the notifying. */

/* (Only the main units of a hex rate messages, occupants' fates are */
/* summarized briefly.) */

/* (What if occupants change type when killed, but transport vanishes?) */

reckon_damage()
{
    int o = omain->type, a = amain->type, i;
    Side *as = amain->side, *os = omain->side;
	char hitbuf[BUFSIZE], killbuf[BUFSIZE];

#if 0
    strcpy(aabuf, unit_handle(as, amain));
    strcpy(aobuf, unit_handle(as, omain));
    strcpy(oabuf, unit_handle(os, amain));
    strcpy(oobuf, unit_handle(os, omain));
#endif
/*    draw_blast(amain, omain->side, ahit);
    draw_blast(omain, amain->side, ohit); */
    damage_unit(omain);
#if 0
    if (ohit >= omain->hp) {
/*	notify(as, "%s %s %s!", aabuf, u_destroymsg(o), aobuf);
	notify(os, "%s %s %s!", oabuf, u_destroymsg(o), oobuf);  */
	for_all_unit_types(i) occkills[i] += (occdeath ? occdeath[i] : 0);
    } else if (ohit > 0) {
/*	notify(as, "%s hits %s!", aabuf, aobuf);
	notify(os, "%s hits %s!", oabuf, oobuf); */
    } else {
	/* messages about missing not too useful */
    }
    summarize_units(hitbuf, occhits);
    summarize_units(killbuf, occkills);
    if (strlen(hitbuf) > 0) {
	if (strlen(killbuf) > 0) {
/*	    notify(as, "   (Also hit%s, killed%s)", hitbuf, killbuf);
	    notify(os, "   (%s hurt, %s killed)", hitbuf, killbuf); */
	} else {
/*	    notify(as, "   (Also hit%s)", hitbuf);
	    notify(os, "   (%s hurt)", hitbuf); */
	}
    } else {
	if (strlen(killbuf) > 0) {
/*	    notify(as, "   (Also killed%s)", killbuf);
	    notify(os, "   (%s killed)", killbuf); */
	}
    }
#endif
    damage_unit(amain);
#if 0
    if (ahit >= amain->hp) {
/*	notify(as, "%s %s %s!", aobuf, u_wreckmsg(a), aabuf);
	notify(os, "%s %s %s!", oobuf, u_wreckmsg(a), oabuf); */
    } else if (ahit > 0) {
/*	notify(as, "%s hits %s!", aobuf, aabuf);
	notify(os, "%s hits %s!", oobuf, oabuf); */
	amain->hp -= ahit;
    } else {
	/* messages about missing not too useful */
    }
#endif
}

reckon_damage_here(x, y)
int x, y;
{
    Unit *unit;

    for_all_stack(x, y, unit) {
	damage_unit(unit);
    }
}

reckon_damage_around(x, y, r)
int x, y, r;
{
    if (r > 0) {
	apply_to_area(x, y, r, reckon_damage_here);
    } else {
	reckon_damage_here(x, y);
    }
}

damage_unit(unit)
Unit *unit;
{
    Unit *occ;

    /* Take care of all the occupants first. */
    for_all_occupants(unit, occ) {
	damage_unit(occ);
    }
    /* If no damage was recorded, just return. */
    if (unit->hp2 == unit->hp) return;
    /* If unit is to die, do the consequences. */
    if (unit->hp2 <= 0) {
	if (u_wrecked_type(unit->type) == NONUTYPE) {
	    /* (should let occupants escape now?) */
	    kill_unit(unit, H_UNIT_KILLED);
	} else {
	    change_unit_type(unit, u_wrecked_type(unit->type));
	    /* Restore to default hp for the new type. */
	    unit->hp = unit->hp2 = u_hp(unit->type);
	    /* Get rid of occupants if overfull. */
	    eject_excess_occupants(unit);
	    /* (should report that unit was wrecked) */
	}
    } else {
	unit->hp = unit->hp2;
	/* (should record that unit was damaged) */
	/* record_event(H_UNIT_DAMAGED); */
    }
    /* Let the unit's owner know about all this. */
    update_unit_display(unit->side, unit, TRUE);
}

/* Retreat is a special kind of movement that a unit uses to avoid
   damage during combat. It bypasses some of the normal move rules. */

retreat_unit(unit, atker)
Unit *unit, *atker;
{
    int dir;

    if (unit->x == atker->x && unit->y == atker->y) {
    	dir = random_dir();
    } else {
    	dir = approx_dir(unit->x - atker->x, unit->y - atker->y);
    }
    if (retreat_in_dir(unit, dir)) return TRUE;
    if (flip_coin()) {
    	if (retreat_in_dir(unit, left_dir(dir))) return TRUE;
    	if (retreat_in_dir(unit, right_dir(dir))) return TRUE;
    } else {
    	if (retreat_in_dir(unit, right_dir(dir))) return TRUE;
    	if (retreat_in_dir(unit, left_dir(dir))) return TRUE;
    }
    return FALSE;
}

retreat_in_dir(unit, dir)
Unit *unit;
int dir;
{
    int nx, ny, rslt;

    if (!interior_point_in_dir(unit->x, unit->y, dir, &nx, &ny)) return FALSE;
    rslt = check_move_action(unit, unit, nx, ny, unit->z);
    if (!valid(rslt)) return FALSE;
    do_move_action(unit, unit, nx, ny, unit->z);
    return TRUE;
}

/* Prepare a capture action to be executed later. */

prep_capture_action(unit, unit2, defender)
Unit *unit, *unit2, *defender;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_CAPTURE;
    unit->act->nextaction.args[0] = defender->id;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

do_capture_action(unit, unit2, defender)
Unit *unit, *unit2, *defender;
{
    int rslt = check_capture_action(unit, unit2, defender);

    if (!valid(rslt)) return rslt;
    attempt_to_capture_unit(unit2, defender);
    use_up_acp(unit, uu_acp_to_capture(unit2->type, defender->type));
    return A_ANY_DONE;
}

check_capture_action(unit, unit2, unit3)
Unit *unit, *unit2, *unit3;
{
    int u, u2, u3, acp;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!in_play(unit3)) return A_ANY_ERROR;
    /* We can't capture ourselves. */
    if (unit2 == unit3) return A_ANY_ERROR;
    /* We can't capture units on our side. */
    if (unit2->side == unit3->side) return A_ANY_ERROR;    
    u = unit->type;  u2 = unit2->type;  u3 = unit2->type;
    acp = uu_acp_to_capture(u2, u3);
    if (acp < 1) return A_ANY_CANNOT_DO;
    if (uu_capture(u2, u3) == 0) return A_ANY_CANNOT_DO;
    if (distance(unit2->x, unit2->y, unit3->x, unit3->y) > 1) return A_ANY_ERROR;
    if (!has_enough_acp(unit, acp)) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* Handle capture possibility and repulse/slaughter. */

/* The chance to capture an enemy is modified by several factors. */
/* Neutrals have a different chance to be captured, and presence of */
/* occupants should also has an effect.  Can't capture anything that is */
/* on a kind of terrain that the capturer can't go on, unless victim has */
/* "bridge effect". */

/* (Need a little better treatment of committed assaults, where lack of
   success == death.) */

attempt_to_capture_unit(atker, other)
Unit *atker, *other;
{
    int a = atker->type, o = other->type, chance, prot;
    int ox = other->x, oy = other->y;
    Unit *occ;
    Side *as = atker->side, *os = other->side;

    if (alive(atker) && alive(other) && could_capture(a, o)) {
	if (impassable(atker, ox, oy) && !uu_bridge(o, a)) return;
	/* Can possibly detonate on *any* attempt to capture! */
	if (probability(uu_detonation_on_capture(o, a))) {
    		detonate_unit(other, other->x, other->y, other->z);
    		/* Might not be possible to capture anything anymore. */
    		if (!alive(atker) || !alive(other)) return;
    		/* Types of units might have changed, recalc things. */
    		a = atker->type;  o = other->type;
		as = atker->side;  os = other->side;
	}
	chance = uu_capture(a, o);
#if 0
	if (u_maxquality(a) > 0) {
	    chance += ((chance * atker->quality * u_skillf(a)) /
		       u_maxquality(a)) / 100;
	}
#endif
	/* Occupants can protect the transport. */
	for_all_occupants(other, occ) {
	    if (is_active(occ)) {
	    	prot = uu_protection(occ->type, o);
		chance = (chance * prot) / 100;
	    }
	}
	/* Test whether the capture actually happens. */
	if (probability(chance)) {
	    capture_unit(atker, other);
#if 0
	} else if (atker->transport != NULL && 
		   (impassable(atker, ox, oy) ||
		    impassable(atker, atker->x, atker->y))) {
	    /* was the failed assault a commitment? */
	    notify(as, "Resistance... %s was slaughtered!",
		   unit_handle(as, atker));
	    notify(os, "Resistance... %s was slaughtered!",
		   unit_handle(os, atker));
	    kill_unit(atker, COMBAT);
#endif
	} else {
	    /* (should record failed attempt to capture) */
#if 0
	    strcpy(aabuf, unit_handle(as, atker));
	    notify(as, "%s repulses %s!", unit_handle(as, other), aabuf);
	    strcpy(oabuf, unit_handle(os, atker));
	    notify(os, "%s repulses %s!", unit_handle(os, other), oabuf);
#endif
	}
    }
}

/* There are many consequences of a unit being captured.
   If the capturer is needed as a garrison, unload its occupants first. */

capture_unit(unit, pris)
Unit *unit, *pris;
{
    int u = unit->type, u2, px = pris->x, py = pris->y, i;
    Unit *occ;
    Side *ps = pris->side, *us = unit->side;

    if (probability(uu_scuttle(pris->type, unit->type))) {
	/* (should add terrain effect on success too) */
	/* (should characterize as a scuttle) */
	kill_unit(pris, H_UNIT_DISBANDED);
    }
    if (alive(pris)) {
	/* Decide the fate of each occupant of our prisoner. */
	for_all_occupants(pris, occ) {
	    capture_occupant(unit, pris, occ);
	}
	/* The change of side itself.  This happens recursively to any
	   remaining occupants as well. */
	unit_changes_side(pris, us, H_UNIT_ACQUIRED, H_UNIT_CAPTURED);
	/* Possibly garrison the newly-captured unit. */
	/* (should be shared with unit completion code) */
	if (uu_hp_to_garrison(u, pris->type) > 0) {
	    if (uu_hp_to_garrison(u, pris->type) >= unit->hp) {
		/* Garrisoning unit will be vanishing, move its occupants out. */
		for_all_occupants(unit, occ) {
		    if (can_occupy(occ, pris)) {
			leave_hex(occ);
			enter_transport(occ, pris);
		    }
		    if (can_occupy_cell(occ, px, py)) {
			leave_hex(occ);
			enter_hex(occ, px, py);
		    }
		}
		/* This may still kill some of the garrison's occupants, but */
		/* only under some pretty exotic conditions. */
		kill_unit(unit, H_UNIT_GARRISONED);
	    } else {
		unit->hp -= uu_hp_to_garrison(u, pris->type);
	    }
	}
	/* Clear any actions and plans. */
	init_unit_actorstate(pris);
	init_unit_plan(pris);
	/* Record for posterity. */
	record_event(H_UNIT_CAPTURED, -1, u, pris->type);
    }
    /* Update everybody's view of the situation. */
    see_exact(ps, px, py);
    update_cell_display(ps, px, py, TRUE);
    all_see_hex(px, py);
}

/* Given that the main unit is going to be captured, decide what each occupant
   will do. */

capture_occupant(unit, pris, occ)
Unit *unit, *pris, *occ;
{
    int u = unit->type;
    Unit *subocc;

    if (probability(uu_occ_escape(u, occ->type))) {
	/* The occupant escapes, along with all its suboccupants. */
	/* (should impl by moving to nearby cells?) */
    } else if (0 /* (should allow for occ scuttling also) */) {
    } else if (/* u_change_side(occ->type) || */ could_capture(u, occ->type)) {
	/* Side change will actually happen later. */
	for_all_occupants(occ, subocc) {
	    capture_occupant(unit, occ, subocc);
	}
    } else {
	/* Occupant can't live as a prisoner, but suboccs might. */
	for_all_occupants(occ, subocc) {
	    capture_occupant(unit, occ, subocc);
	}
	/* Any suboccupants that didn't escape will die. */
	/* (what if subocc captured tho??) */
	kill_unit(occ, H_UNIT_KILLED);
    }
}

/* A detonate action just blasts the vicinity. */

prep_detonate_action(unit, unit2, x, y, z)
Unit *unit, *unit2;
int x, y, z;
{
    if (unit == NULL || unit->act == NULL) return FALSE;
    unit->act->nextaction.type = A_DETONATE;
    unit->act->nextaction.args[0] = x;
    unit->act->nextaction.args[1] = y;
    unit->act->nextaction.args[2] = z;
    unit->act->nextaction.actee = unit2->id;
    return TRUE;
}

do_detonate_action(unit, unit2, x, y, z)
Unit *unit, *unit2;
int x, y, z;
{
    int u2;
    int rslt = check_detonate_action(unit, unit2, x, y, z);

    if (!valid(rslt)) return rslt;
    u2 = unit2->type;
    detonate_unit(unit2, x, y, z);
    /* Note that if the maxrange is further than the actual range of this
       detonation, only just-damaged units will be looked at. */
    reckon_damage_around(x, y, maxdetonaterange);
    use_up_acp(unit, u_acp_to_detonate(u2));
    return A_ANY_DONE;
}

check_detonate_action(unit, unit2, x, y, z)
Unit *unit, *unit2;
int x, y, z;
{
    int u, u2, acp;

    if (!in_play(unit)) return A_ANY_ERROR;
    if (!in_play(unit2)) return A_ANY_ERROR;
    if (!inside_area(x, y)) return A_ANY_ERROR;
    u = unit->type;  u2 = unit2->type;
    /* The unit must actually be able to detonate. */
    acp = u_acp_to_detonate(u2);
    if (acp < 1) return A_ANY_CANNOT_DO;
    /* Can only detonate in our own or an adjacent cell. */
    if (distance(unit2->x, unit2->y, x, y) > 1) return A_ANY_ERROR;
    if (!has_enough_acp(unit, acp)) return A_ANY_NO_ACP;
    return A_ANY_OK;
}

/* Actual detonation may occur by explicit action or automatically; this
   routine makes the detonation effects happen, pyrotechnics and all. */

/* (need to accumulate a damage report a la regular combat) */
/* (detonation effect at large distances should be patchy) */
/* (let elevations screen effects?) */
/* (interleave visual effects here - hook to display routines) */
/* (game design should be able to ask for a screen flash as extra) */

detonate_unit(unit, x, y, z)
Unit *unit;
int x, y, z;
{
    int u = unit->type, dir, x1, y1, dmg;
    Unit *unit2;

    /* Hit the detonating unit first. */
    hit_unit(unit, u_hp_per_detonation(u), NULL);
    /* Hit units at ground zero. */
    for_all_stack(x, y, unit2) {
    	if (unit2 != unit) {
	    hit_unit(unit2, uu_detonation_damage_at(u, unit2->type), unit);
	}
    }
    /* Hit units in adjacent cells if this is defined. */
    if (maxdetonaterange >= 1) {
        for_all_directions(dir) {
	    if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
		for_all_stack(x1, y1, unit2) {
		    dmg = uu_detonation_damage_adj(u, unit2->type);
		    hit_unit(unit2, dmg, unit);
		}
	    }
	}
    }
    /* (should) Hit units at a distance, possibly throughout the world. */
    if (maxdetonaterange >= 2) {
    	/* apply_to_area */
    }
    /* (Alter terrain, terrain materials?) */
    return TRUE;
}
