/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Initialization is complicated, because xconq needs lots of setup for
   maps, units, sides, and the like.  The data must also be able to come
   from saved games, scenarios, bare maps in files, type definitions,
   or be synthesized if necessary. */

/* The general theory is that everything gets set to a known empty state,
   then all files are read, then all the synthesis methods get run.
   Files/readers and synth methods are each responsible for preventing
   fatal conflicts. */

#include "conq.h"

/* This is true when the set of types has been defined.  Subsequently only
   scenarios based on those types can be loaded. */

int typesdefined = FALSE;

/* This is true after the game is totally synthesized, but players may
   not all be in the game yet. */

int gamedefined = FALSE;

/* These are handy tmp vars, usually used with function args. */

short tmputype;
short tmpmtype;
short tmpttype;

/* The array of player/side matchups. */

Assign *assignments = NULL;

/* The pathname to the library directory. */

char *xconqlib;

/* The name of the default game module. */

char *standardgamename;

/* This is the main printing buffer. */

char spbuf[BUFSIZE];

/* This is an auxiliary printing buffer. */

char tmpbuf[BUFSIZE];

#ifdef DESIGNERS

/* This is true if all sides with displays should become designers
   automatically. */

int allbedesigners = FALSE;

/* This password is for designers to get access to their tools.  It is
   not particularly secret, but knowing it doesn't help one to cheat,
   because displays inform everybody who the designers are. */

char *designerpassword = "hackhack";

/* This is the current count of how many sides are designers. */

int numdesigners = 0;

#endif /* DESIGNERS */

FILE *errorfp = NULL;

int maxzocrange = 0;

/* The table of all available synthesis methods. */

int make_fractal_terrain();
int make_random_terrain();
int make_earthlike_terrain();
int make_maze_terrain();
int make_rivers();
int make_countries();
int make_independent_units();
int make_initial_materials();
int name_units_randomly();
int name_geographical_features();
int make_roads();
int make_random_date();

struct synthtype {
    int key;
    int (*fn)();
    int dflt;
} synthmethods[] = {
    { K_MAKE_FRACTAL_PTILE_TERRAIN, make_fractal_terrain, TRUE },
    { K_MAKE_RANDOM_TERRAIN, make_random_terrain, FALSE },
    { K_MAKE_EARTHLIKE_TERRAIN, make_earthlike_terrain, FALSE },
    { K_MAKE_MAZE_TERRAIN, make_maze_terrain, FALSE },
    { K_MAKE_RIVERS, make_rivers, TRUE },
    { K_MAKE_COUNTRIES, make_countries, TRUE },
    { K_MAKE_INDEPENDENT_UNITS, make_independent_units, TRUE },
    { K_MAKE_INITIAL_MATERIALS, make_initial_materials, TRUE },
    { K_NAME_UNITS_RANDOMLY, name_units_randomly, TRUE },
    { K_NAME_GEOGRAPHICAL_FEATURES, name_geographical_features, TRUE },
    { K_MAKE_ROADS, make_roads, TRUE },
    { K_MAKE_RANDOM_DATE, make_random_date, FALSE },
    { -1, NULL, 0 }
};

/* Allow environment vars to override compiled-in library and game. */

init_library_path()
{
    extern char *standardgamename;

    if ((xconqlib = getenv("XCONQLIB")) == NULL
    	|| strlen(xconqlib) == 0)
	xconqlib = XCONQLIB;
    if ((standardgamename = getenv("XCONQSTANDARDGAME")) == NULL
    	|| strlen(standardgamename) == 0)
	standardgamename = "standard";  /* define in config.h? */
}

/* This is a general init that prepares data structures to be filled in
   by a game definition.  It should run *before* command line parsing. */

init_data_structures()
{
    int i;

#ifdef DEBUGGING
#ifdef USE_CONSOLE
    dfp  = stdout;
    dgfp = stdout;
    dmfp = stdout;
#else
    /* need to open files or something */
#endif
#endif

    init_xrandom(-1);
    init_lisp();
    init_types();
    init_globals();
    init_namers();
    init_world();
    init_history();
    init_sides();
    init_agreements();
    init_units();
    init_help();
    init_scorekeepers();
}

/* Build the default list of synthesis methods. */

set_g_synth_methods_default()
{
    int i;
    Obj *synthlist = lispnil, *synthlistend = lispnil, *tmp;

    for (i = 0; synthmethods[i].key >= 0; ++i) {
	if (synthmethods[i].dflt) {
	    tmp = cons(intern_symbol(keyword_name(synthmethods[i].key)),
		       lispnil);
	    if (synthlist == lispnil) {
		synthlist = synthlistend = tmp;
	    } else {
		synthlistend->v.cons.cdr = tmp;
		synthlistend = tmp;
	    }
	}
    }
    /* Now alter the global variable to contain this list. */
    set_g_synth_methods(synthlist);
}

/* Run a doublecheck on plausibility of game parameters.  Additional
   checks are performed elsewhere as needed, for instance during random
   generation.  Serious mistakes exit now, since they can cause all sorts
   of strange behavior and core dumps.  It's a little more friendly to only
   exit at the end of the tests, so all the mistakes can be found at once. */

/* In theory, if a game passes these tests, then Xconq will never crash. */

/* (This should always be run, might find errors in compiled game.) */

check_game_validity()
{
    int failed = FALSE, movers = FALSE, actors = FALSE;
    int u1, u2, t1, t2;
    extern int numcelltypes;

    /* We must have at least one kind of unit (really? check this). */
    if (numutypes < 1) {
	init_error("no unit types have been defined");
	failed = TRUE;
    }
    /* OK not to have any types of materials. */
    /* We must have at least one kind of terrain. */
    if (numttypes < 1) {
	init_error("no terrain types have been defined");
	failed = TRUE;
    }
    /* Make sure inter-country distances relate correctly. */
    if (g_min_separation() >= 0
	&& g_max_separation() >= 0
	&& !(g_min_separation() <= g_max_separation())) {
	init_warning("country separations %d to %d screwed up",
		     g_min_separation(), g_max_separation());
    }

    /* (Need more general game checks.) */

    maxzocrange = -1;

    /* Check that all the unit names and chars are distinct. */
    for_all_unit_types(u1) {
	for_all_unit_types(u2) {
	    /* Only do "upper triangle" of utype x utype matrix. */
	    if (u1 < u2) {
		if (strcmp(u_type_name(u1), u_type_name(u2)) == 0) {
		    init_warning(
		     "unit types %d and %d are both named \"%s\"",
				 u1, u2, u_type_name(u1));
		    /* This is bad but not disastrous, so don't fail. */
		}
	    }
	    if (uu_zoc_range(u1, u2) > maxzocrange) {
		maxzocrange = uu_zoc_range(u1, u2);
	    }
	}
    }
    /* Large ZOC ranges are not implemented efficiently. */
    if (maxzocrange > 3) {
	init_warning("ZOC range goes up to %d, may be very inefficient", maxzocrange);
    }
    /* (Eventually check material types also.) */
    /* Check that all terrain names and chars are distinct. */
    for_all_terrain_types(t1) {
	for_all_terrain_types(t2) {
	    /* Only do "upper triangle" of ttype x ttype matrix. */
	    if (t1 < t2) {
		if (strcmp(t_type_name(t1), t_type_name(t2)) == 0) {
		    init_warning(
		     "terrain types %d and %d are both named \"%s\"",
		     t1, t2, t_type_name(t1));
		    /* This is bad but not disastrous, so don't fail. */
		}
		/* Should check that colors are different from each other
		   and from builtin colors? */
	    }
	}
    }
    /* Check various unit type properties. */
    for_all_unit_types(u1) {
	/* should be part of general bounds check */
	if (u_cp(u1) <= 0) {
	    init_error("unit type %d has nonpositive cp", u1);
	    failed = TRUE;
	}
	if (u_hp(u1) <= 0) {
	    init_error("unit type %d has nonpositive hp", u1);
	    failed = TRUE;
	}
	if (u_parts(u1) <= 0 || u_hp(u1) % u_parts(u1) != 0) {
	    init_error("unit type %d hp not a multiple of its parts.", u1);
	    failed = TRUE;
	}
	if (u_speed(u1) > 0) {
	    movers = TRUE;
	}
	if (u_acp(u1) > 0) {
	    actors = TRUE;
	}
    }
    /* If nothing can move and nothing can build, this will probably be
       a really dull game, but there may be such games, do don't say
       anything normally. */
    if (numutypes > 0 && !actors) {
	Dprintf("No actors have been defined.\n");
    }
    if (numutypes > 0 && !movers) {
	Dprintf("No movers have been defined.\n");
    }
    if (numttypes > 0 && numcelltypes == 0) {
	init_error("no terrain type has been allowed for cells");
	failed = TRUE;
    }
    /* This is a really bad game definition, leave before we crash.  This would
       only be executed on systems where init_error doesn't exit immediately. */
    if (failed) {
	exit(0);
    }
    Dprintf("Finished checking game design.\n");
    Dprintf("It defines %d unit types, %d material types, %d terrain types.\n",
	    numutypes, nummtypes, numttypes);
}

/* Calculate the values of global variables that are used everywhere. */

calculate_globals()
{
    calculate_world_globals();
}

/* Clean up all the objects and cross-references. */

patch_object_references()
{
    Unit *unit, *transport;
    Side *side, *side2;
    Player *player;

    /* Use read-in ids to fill in side slots that point to other objects. */
    for_all_sides(side) {
	if (side->playerid > 0) {
	    side->player = find_player(side->playerid);
	}
	if (side->controlledbyid > 0) {
	    side->controlledby = side_n(side->controlledbyid);
	}
	if (side->selfunitid > 0) {
	    side->selfunit = find_unit(side->selfunitid);
	}
    }
    reverse_unit_order(); /* why is this important? */
    for_all_units(side, unit) {
	/* It's possible that dead units got read in, so check. */
	if (alive(unit)) {
	    if (((int) unit->transport) > 0) {
		transport = find_unit((int) unit->transport);
		/* it is important to make sure that unit->x, and unit->y 
		   are negative at this point.  Otherwise, the coverage will
		   be messed up for units put into transports that have not yet
		   been placed.  They will be covered for entering the hex,
		   and again when the transport enters the hex. */
		if (transport != NULL) {
		    /* (also check that this is a valid transport type?) */
		    enter_transport(unit, transport);
		} else {
		    init_warning("could not find transport (#%d) for %d",
				 (int) unit->transport, unit_desig(unit));
		    /* (could let the unit enter the cell, or could
		       make it infinitely postponed) */
		}
	    } else {
		unit->transport = NULL;
		/* Check that the unit's location is meaningful. */
		if (!inside_area(unit->x, unit->y)) {
		    if (inside_area(unit->prevx, unit->prevy)) {
			if (can_occupy_cell(unit, unit->prevx, unit->prevy)) {
			    enter_hex(unit, unit->prevx, unit->prevy);
			} else {
			    /* Search this cell for units to enter. */
			    for_all_stack(unit->prevx, unit->prevy, transport) {
				if (unit->side == transport->side
				    && can_occupy(unit, transport)) {
				    enter_transport(unit, transport);
				    break;
				}
			    }
			    if (!inside_area(unit->x, unit->y)) {
				init_warning("No room for %s at %d,%d",
					     unit_desig(unit),
					     unit->prevx, unit->prevy);
			    }
			}
			/* This prevents attempts to do a second enter_hex
			   during initialization. */
			unit->prevx = unit->prevy = -1;
		    } else if (unit->cp >= 0) {
			/* Warn, but not if we're shrinking the area. */
			if (area.fullwidth == 0) {
			    init_warning("%d,%d offworld, %s left there",
					 unit->prevx, unit->prevy,
					 unit_desig(unit));
			}
			/* This will make it be a reinforcement. */
			unit->cp = -1;
		    }
		}
	    }
	    /* Make sure that side numbering will use only new numbers. */
	    if (unit->side) {
		if (unit->number > 0) {
		    (side->counts)[unit->type] =
		      max((side->counts)[unit->type], 1 + unit->number);
		}
	    } else {
		/* Trash the numbers on indep units. */
		unit->number = 0;
	    }
	    if (completed(unit)) {
		init_unit_actorstate(unit);
		/* Might already have a plan, leave alone if so. */
		if (unit->plan == NULL) {
		    init_unit_plan(unit);
		}
	    }
	} else {
	    /* actually, this is OK */
#if 0
	    init_warning("dead unit %s was read in", unit_desig(unit));
#endif
	    if (inside_area(unit->x, unit->y)) {
	    	unit->transport = NULL; /* kind of bogus */
		leave_hex(unit);
	    }
	}
    }
}

/* Make up a proposed side/player assignment, creating sides and players
   as necessary.  Lock down any assignments that should not be changed,
   but leave everything to be changed as desired. */

/* (when does locking get done?) */

make_trial_assignments()
{
    int i = 0;
    Side *side;
    Player *player;

    /* Fill in the side's predefined default and range of initial advantage. */
    for_all_sides(side) {
    	init_side_advantage(side);
    }
    /* Ensure we have as many sides as will be required. */
    while (numsides < g_sides_min()) {
	make_up_a_side();
    }
    while (numsides < min(numplayers, g_sides_max())) {
	make_up_a_side();
    }
    if (numsides < numplayers) {
	/* We have too many players. */
	init_warning("too many players (%d)", numplayers);
	numplayers = numsides; /* probably bogus */
	/* Ideally, would ask whether to continue */
    }
    /* Put in all the sides. */
    for_all_sides(side) {
	assignments[i++].side = side;
    }
    /* If no players have been created, make one that is human-run,
       presumably by the person who started up this program. */
    if (numplayers == 0) {
	add_default_player();
    }
    /* Add in enough default players for all the sides. */
    while (numplayers < numsides) {
	player = add_player();
	/* Default players are always AIs (at least for now). */
	player->aitypename = "mplayer";
    }
    /* (make any prespecified assignments) */
    /* Assign any remaining players. */
    player = playerlist;
    for (i = 0; i < numsides; ++i) {
	if (assignments[i].player == NULL) {
	    for (player = playerlist ; player != NULL; player = player->next) {
		if (player->side == NULL) {
		    assignments[i].player = player;
		    player->side = assignments[i].side;
		    (assignments[i].side)->player = player;
		    break;
		} else {
		}
	    }
	}
	/* Set the player's advantage to be the side's advantage, if defined. */
	if (assignments[i].player != NULL
	    && assignments[i].player->advantage == 0) {
	    assignments[i].player->advantage = assignments[i].side->advantage;
	}
	Dprintf("Tentatively assigned %s to %s%s\n",
		side_desig(assignments[i].side),
		player_desig(assignments[i].player),
		(assignments[i].locked ? " (locked)" : ""));
    }
    /* At this point, we have matching sides and players, ready to be
       rearranged if desired. */
}

/* Create a random side with default characteristics. */

make_up_a_side()
{
    int i;
    Side *side = create_side();

    if (side == NULL) {
	run_error("a side couldn't be created!");
	return;
    }
    make_up_side_name(side);
    init_side_advantage(side);
    /* A newly-created side starts out in the game, can drop out later. */
    side->ingame = TRUE;
    Dprintf("Made up a side %s\n", side_desig(side));
}

init_side_advantage(side)
Side *side;
{
	/* Set up the default and range of initial advantages. */
	if (side->advantage == 0) {
	    side->advantage = g_advantage_default();
	}
	if (side->minadvantage == 0) {
	    side->minadvantage = g_advantage_min();
	}
	if (side->maxadvantage == 0) {
	    side->maxadvantage = g_advantage_max();
	}
}

/* Add a side and a player to go with it (used by interfaces). */

add_side_and_player()
{
    int n;
    Side *side;
    Player *player;

    make_up_a_side();
    side = side_n(numsides);
    if (side == NULL) return FALSE;
    n = numsides - 1;
    assignments[n].side = side;
    player = add_player();
    assignments[n].player = player;
    player->side = assignments[n].side;
    (assignments[n].side)->player = player;
    /* Set the player's advantage to be the side's advantage, if not already set. */
    if (player->advantage == 0) {
	player->advantage = side->advantage;
    }
    return TRUE;
}

/* This can be used by interfaces to exchange players between one side and
   another. */

exchange_players(n, n2)
int n, n2;
{
	int i;
    Player *tmpplayer = assignments[n].player;

    if (n < 0) n = 0;
    if (n2 < 0) {
	for (i = n + 1; i <= numsides + n; ++i) {
	    n2 = i % numsides;
	    if (assignments[n2].side && (assignments[n2].side)->ingame) break;
	}
	/* No sides to exchange with, return. */
    	if (i == numsides + n) return -1;
    }
    assignments[n].player = assignments[n2].player;
    assignments[n2].player = tmpplayer;
    /* Doesn't seem like these should be needed, but they are. */
    assignments[n].player->side = assignments[n].side;
    assignments[n].side->player = assignments[n].player;
    assignments[n2].player->side = assignments[n2].side;
    assignments[n2].side->player = assignments[n2].player;
    return n2;
}

remove_side_and_player()
{
    /* (how to do this?) */
    /* (would need to renumber sides etc) */
}

/* Synthesis methods fill in whatever is not fixed by game modules or by
   the player(s). */

run_synth_methods()
{
    int i, methkey, found = FALSE;
    Obj *synthlist, *methods, *method;

    gamedefined = 1; /* this isn't really the right place */
    calculate_globals();
    synthlist = g_synth_methods();
    Dprintf("Will run syntheses ");  Dprintlisp(synthlist);  Dprintf("\n");
    for (methods = synthlist; methods != lispnil; methods = cdr(methods)) {
	method = car(methods);
	if (symbolp(method) || stringp(method)) {
	    methkey = keyword_code(c_string(method));
	    for (i = 0; synthmethods[i].key >= 0; ++i) {
		if (methkey == synthmethods[i].key) {
		    (*synthmethods[i].fn)();
		    found = TRUE;
		    break;
		}
	    }
	}
	if (!found) {
	    sprintlisp(spbuf, method);
	    init_warning("bad synthesis method %s, ignoring", spbuf);
	}
    }
    /* should go elsewhere eventually */
    {
	int x, y;
	extern int maxwindforce;	

	if (maxwindforce > 0) {
	    if (!winds_defined()) {
		allocate_area_winds();
		for_all_hexes(x, y) {
		    set_wind_at(x, y, random_dir(),
				t_wind_force_avg(terrain_at(x, y)));
		}
	    }
	}
    }
}

/* Set the starting date/time to a random value. */

make_random_date()
{
    extern int baseyear;

    /* (this should tweak a "date offset" rather than curturn,
       which should be actual turn number) */
    baseyear = 1900 + xrandom(10);
}

/* The final init cleans up various stuff. */

final_init()
{
    /* At this point we should be ready to roll.  Any inconsistencies
       hereafter will be fatal. */
    check_consistency();
    /* If any sides' balance sheets were not restored, count the units now. */
    init_side_balance();
    /* Fix up garbled view data. */
    init_all_views();
    configure_sides();
    /* Check again, just to be sure. */
    check_consistency();
    create_game_help_nodes();
    /* Set up the scores to be attached to each side. */
    init_scores();
    /* Calculate which random event methods will be run. */
    init_random_events();
    /* Report on memory consumption. */
    Dprintf("One side is %d bytes.\n", sizeof(Side));
    Dprintf("One unit is %d bytes, one plan is %d bytes.\n",
	     sizeof(Unit), sizeof(Plan));
    if (Debug) report_malloc();
}

/* Load up any player-specified configuration data. */

configure_sides()
{
    Side *side;

    for_all_sides(side) {
	if (0 /* side->username || side->configname */) {
	    load_side_config(side);
	}
    }
}

/* Allocate the terrain (prevents crashes), but don't touch it otherwise. */

ensure_terrain_defined()
{
    if (!terrain_defined()) {
	allocate_area_terrain();
    }
}

/* Transfer a unit to the given side. */

give_initial_unit(side, unit)
Side *side;
Unit *unit;
{
    set_unit_side(unit, side);
}

/* Calculate what each side knows about the world. */

init_all_views()
{
    int x, y, i = 0, todo = max(1, numsides * area_cells());
    Unit *unit;
    Side *side;

    /* No view init needed if everything always visible. */
    if (g_see_all()) return;
    /* Set up the basic view structures for all sides first. */
    for_all_sides(side) {
	init_view(side);
	/* Can't think of any other place to put this... */
	calc_start_xy(side);
    }
    announce_lengthy_process("Computing current view at each location");
    /* (coverage is also done at this point) */
    for_all_sides(side) {
	for_all_hexes(x, y) {
	    ++i;
    	    if (i % 100 == 0) announce_progress((100 * i) / todo);
	    see_hex(side, x, y);
	}
    }
    finish_lengthy_process();
    init_area_views();
}

/* This is a helper for the following routine.  The helper is applied to
   each hex, decides what is visible in that hex. */

/* This only works from already_seen, does not account for coverage. */

init_view_hex(x, y)
int x, y;
{
    int u, chance;
    Unit *unit;

    /* Guaranteed to see the terrain accurately. */
    set_terrain_view(tmpside, x, y, buildtview(terrain_at(x, y)));
    /* If this cell is under observation, don't need to do anything special. */
    if (tmpside->coverage && cover(tmpside, x, y) > 0) return;
    /* Scan all the units here to see if any are visible. */
    for_all_stack(x, y, unit) {
    	if (in_play(unit)) {
    	    u = unit->type;
	    if (u_see_always(u)) {
	    	see_exact(tmpside, x, y);
	    	/* or flag unit as spotted? */
		if (tmpside->coverage) set_cover(tmpside, x, y, 1);
	    	return;
	    }
	    chance = (indep(unit) ? u_already_seen_indep(u)
		      : u_already_seen(u));
	    if (probability(chance)) {
	    	see_exact(tmpside, x, y);
	    	/* (should not return always??) */
	   	return;
	    }
	}
    }
}

/* Do ranged initial views from units. */

init_area_views()
{
    int rad, x, y, pop, dir, x1, y1;
    Unit *unit;
    Side *side, *side2;

    /* Don't run if nothing exists to look at. */
    if (!terrain_defined()) return;
    /* Skip if everything already known, side creation got these cases. */
    if (g_see_all() || g_terrain_seen()) return;
    /* Compute the view from each unit of each side. */ 
    for_all_sides(side) {
	/* Set this so the helper fn has a side to use. */
	tmpside = side;
	for_all_side_units(side, unit) {
	    /* The unit always sees itself. */
	    see_exact(unit->side, unit->x, unit->y);
	    /* It may also see things nearby. */
	    rad = u_seen_radius(unit->type);
	    if (rad >= area.maxdim) {
		/* Special optimization - view the whole area. */
		for_all_hexes(x, y) {
		    init_view_hex(x, y);
		}
		/* Nothing more to do now, so go on to next side. */
		break;
	    } else if (rad >= 0) {
		apply_to_area(unit->x, unit->y, rad, init_view_hex);
	    }
	}
    }
    /* The people see everything in the cells that they are in, plus the
       normally visible things in adjacent cells. */
    if (people_sides_defined()) {
    	for_all_interior_hexes(x, y) {
	    pop = people_side_at(x, y);
	    side2 = side_n(pop);
	    if (pop != NOBODY && side2 != NULL) {
		see_exact(side2, x, y);
		for_all_directions(dir) {
		    point_in_dir(x, y, dir, &x1, &y1);
		    tmpside = side2;
		    init_view_hex(x1, y1);
		}
	    }
    	}
    }
}

/* Method to give all units and terrain a basic stockpile of supply. */

make_initial_materials()
{
    int m, t, amts[MAXTTYPES], doany, x, y;
    Unit *unit;
    Side *loopside;

    /* This should not be run if any unit supplies were read in */
    for_all_units(loopside, unit) {
	init_supply(unit);
    }
    /* Go over each material and terrain type, looking for nonzero
       material in terrain possibilities, then alloc and fill in layers
       as needed. */
    for_all_material_types(m) {
	    doany = FALSE;
	    for_all_terrain_types(t) {
		amts[t] = min(tm_storage_x(t, m), tm_initial(t, m));
		if (amts[t] > 0) doany = TRUE;
	    }
	    if (doany) {
		allocate_area_material(m);
		for_all_hexes(x, y) {
		    set_material_at(x, y, m, amts[terrain_at(x, y)]);
		}
	    }
    }
}

/* Give the unit what it is declared to have stockpiled
   at the start of a game. */

init_supply(unit)
Unit *unit;
{
    int m, u = unit->type;

    for_all_material_types(m) {
	/* (should try to fill non-specific storage also somehow) */
	unit->supply[m] = min(um_storage_x(u, m), um_initial(u, m));
    }
}

/* Quicky test needed in a couple places. */

saved_game()
{
    FILE *fp;

    if ((fp = fopen(savefile_name(), "r")) != NULL) {
	fclose(fp);
	return TRUE;
    } else {
	return FALSE;
    }
}

/* Count all the initial units in each side's balance sheet, but only
   if the balance sheet is blank. */

/* (this is like restoring a history log) */

/* (if no log was found, we're starting over - else re-create log and caches)*/

init_side_balance()
{
    int u, num = 0;
    Unit *unit;
    Side *side;

    /* Should decide whether the balance has already been set from a file. */
    for_all_sides(side) {
/*	for_all_unit_types(u) num += side_balance(side, u, FIRSTUNIT); */
	if (num == 0) {
	    for_all_side_units(side, unit) {
/*		side_balance(side, unit->type, FIRSTUNIT)++;  */
		/* record_event(H_UNIT_STARTED_WITH); */
	    }
	}
    }
}

/* This routine does a set of checks to make sure that Xconq's data
   is in a valid state.  This is particularly important after init,
   since the combination of files and synthesis methods may have
   caused some sort of disaster. */

check_consistency()
{
    int x, y;

    /* If no terrain, make a flat area of all ttype 0. */
    if (!terrain_defined()) {
	init_warning("No terrain defined, substituting type 0");
	allocate_area_terrain();
	for_all_hexes(x, y) {
	    if (inside_area(x, y)) {
		set_terrain_at(x, y, 0);
	    }
	}
	add_edge_terrain();
    }
    if (numsides <= 0) {
	init_error("There are no player sides at all in this game");
    } else if (numsides < numplayers) {
	init_warning("Only made %d of the %d sides requested",
		     numsides, numplayers);
    }
    /* Need any kind of unit checks? */
}

/* This does the actual assignment of players to sides, and initializes the
   side structures appropriately. */

assign_players_to_sides()
{
    int i, numdisplays = 0, numdisplayswanted = 0, numais = 0, n = 0;
    Side *side;
    Player *player;

    announce_lengthy_process("Assigning players to sides");
    for (i = 0; i < numsides; ++i) {
	announce_progress((100 * i) / numsides);
	side = assignments[i].side;
	player = assignments[i].player;
	canonicalize_player(player);
	/* Call the interface code to initialize the side's display, if
	   it wants to use one (the interface has to decide). */
	if (side_wants_display(side)) {
	    ++numdisplayswanted;
	    init_ui(side);
	    if (side_has_display(side)) {
		++numdisplays;
	    }
	}
	/* Count the desired AIs, for setup below. */
	if (side_wants_ai(side)) {
	    ++numais;
	}
	Dprintf("Assigned %s to %s\n",
		 side_desig(side), player_desig(player));
	record_event(H_SIDE_JOINED, -1, side_number(side));
	/* plus record all the units as initial units? */
    }
    finish_lengthy_process();
    if (numdisplays < numdisplayswanted) {
	if (numdisplays < 1) {
	    init_warning("None of the %d requested displays opened",
			 numdisplayswanted);
	} else {
	    init_warning("Only %d of %d requested displays opened",
			 numdisplays, numdisplayswanted);
	}
    } else if (numdisplays == 0) {
	init_warning("Need at least one display to run!");
    }
#ifdef DESIGNERS
    /* Make each displayed side into a designer if it was requested. */
    if (allbedesigners) {
    	for_all_sides(side) {
    		if (side_has_display(side)) {
    			become_designer(side);
    		}
    	}
    }
#endif /* DESIGNERS */
  if (numais > 0) {
    announce_lengthy_process("Setting up AIs");
    for (i = 0; i < numsides; ++i) {
	if (numais > 1) announce_progress((100 * n++) / numais);
	side = assignments[i].side;
	if (side_wants_ai(side)) {
	    init_ai(side);
	}
    }
    finish_lengthy_process();
  }
}

/* Return the program version. */

char *
version_string()
{
    return version;
}

/* Return the copyright notice. */

char *
copyright_string()
{
    return "Copyright 1987-1993  Stanley T. Shebs";
}
