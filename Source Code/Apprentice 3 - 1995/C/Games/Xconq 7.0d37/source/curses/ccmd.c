/* Commands for the curses interface to Xconq.
   Copyright (C) 1986, 1987, 1988, 1989, 1991, 1992, 1993, 1994, 1995
   Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "cconq.h"
int ask_direction PROTO ((char *prompt, int *dirp));

/* Command table. */

typedef struct cmdtab {
    char fchar;                 /* character to match against */
    char *name;                 /* Full name of command */
    char *argtypes;
    void (*fn) PROTO ((void));  /* pointer to command's function */
    char *help;                 /* short documentation string */
} CmdTab;

static void resize_map PROTO ((int n));
static void execute_named_command PROTO ((char *cmdstr));
static int execute_command_from_table PROTO ((CmdTab *cmdtab));
static int execute_named_command_from_table PROTO ((char *cmdstr,
						    CmdTab *cmdtab));
static void describe_commands PROTO ((int arg, char *key, char *buf));
static void describe_command_table PROTO ((int arg, char *key, char *buf, CmdTab *cmdtab));
static void describe_help PROTO ((int arg, char *key, char *buf));
static void cmd_error PROTO ((char *fmt, ...));

/* Declarations of all the command functions. */

#undef DEF_CMD
#define DEF_CMD(letter,name,args,FN,help) void FN PROTO ((void));

#include "cmd.def"

#include "ccmd.def"

/* Define a table of generic commands. */

#define C(c) ((c)-0x40)

CmdTab commands[] = {

#undef DEF_CMD
#define DEF_CMD(LETTER,NAME,ARGS,FN,HELP) { LETTER, NAME, ARGS, FN, HELP },

#include "cmd.def"

  { 0, NULL, NULL, NULL, NULL }
};

/* Define a table of curses-specific commands. */

CmdTab ccommands[] = {

#include "ccmd.def"

  { 0, NULL, NULL, NULL, NULL }
};

/* Use this macro in any command if it requires a current unit. */

#define REQUIRE_UNIT()  \
  if (!in_play(curunit)) {  \
    curunit = NULL;  \
    cmd_error("No current unit");  \
    return;  \
  }

Unit *lastactor = NULL;

Unit *
find_next_and_look()
{
    Unit *nextunit, *unit;

    nextunit = find_next_actor(dside, curunit);
    if (nextunit != NULL) {
	make_current(nextunit);
	show_cursor();
    }
    return nextunit;
}

void
do_add_player()
{
    cmd_error("command not implemented");
}

void
do_add_terrain()
{
    int u, t, dir;

    REQUIRE_UNIT();
    u = curunit->type;
    if (ask_direction("Add terrain to where?", &dir)) {
	for_all_terrain_types(t) {
	    if (ut_acp_to_add_terrain(u, t) > 0
		&& curunit->act
		&& curunit->act->acp >= ut_acp_to_add_terrain(u, t)) {
		if (0 <= ut_alter_range(curunit->type, t)) {
		    if (prep_add_terrain_action(curunit, curunit,
						curunit->x, curunit->y,
						dir, t))
		      ;
		    else
		      xbeep();
		}
	    }
	}
    }
}

/* Set which AI is to run the side's play. */

void
do_ai_side()
{
    if (side_has_ai(dside)) {
	set_side_ai(dside, NULL);
    } else {
	set_side_ai(dside, "mplayer");
    }
}

void
do_attack()
{
    int x, y, rslt;
    Unit *other;

    REQUIRE_UNIT();
    if (ask_position("Attack where?", &x, &y)) {
	for_all_stack(x, y, other) {
	    if (!unit_trusts_unit(curunit, other)) {
		if (valid(check_attack_action(curunit, curunit, other, 100))) {
		    prep_attack_action(curunit, curunit, other, 100);
		    return;
		}
		/* (should try other types of actions?) */
	    }
	}
	cmd_error("Nothing for %s to attack at %d,%d!",
		  unit_handle(dside, curunit), x, y);
    }
}

/* Set the unit to automatic control.  */

void
do_auto()
{
    REQUIRE_UNIT();
    if (curunit->plan) {
	curunit->plan->autotask = !curunit->plan->autotask;
	/* finish move under automatic control */
    }
}

void
do_build()
{
    int u2;

    REQUIRE_UNIT();
    if (!can_build(curunit)) {
	cmd_error("%s can't build anything!", unit_handle(dside, curunit));
#if 0
    } else if (!u_occproduce(curunit->type) && curunit->transport != NULL) {
	cmd_error("%s can't build anything while inside another unit!",
		  unit_handle(dside, curunit));
#endif
    } else {
	u2 = ask_unit_type("Type to create:", NULL);
	if (u2 != NONUTYPE) {
	    notify(dside, "%s will build %d %s",
		   unit_handle(dside, curunit), 99, u_type_name(u2));
	    push_build_task(curunit, u2, 99);
	} else {
	    /* should clear toplines */
	}
    }
}

void
do_clear_plan()
{
    REQUIRE_UNIT();
    if (curunit->plan) {
	set_unit_plan_type(dside, curunit, PLAN_NONE);
    }
}

void
do_copying()
{
    notify(dside, "You may copy freely.  See the file COPYING.");
}

void
do_delay()
{
    Unit *oldcurunit = curunit, *nextmover;

    nextmover = find_next_and_look();

    if (nextmover == NULL) {
	cmd_error("no units???");
    } else if (curunit == oldcurunit) {
	notify(dside, "No other units to move!");
    }
}

void
do_detach()
{
    cmd_error("command not implemented");
}

void
do_detonate()
{
    Unit *unit = curunit;

    REQUIRE_UNIT();
    prep_detonate_action(unit, unit, unit->x, unit->y, unit->z);
}

/* Supposedly you could only get to these by typing the full command names. */

void
do_dir()
{
    cmd_error("use the single-character commands instead");
}

void
do_dir_multiple()
{
    cmd_error("use the single-character commands instead");
}

/* Get rid of a unit. */

void
do_disband()
{
    int u;

    REQUIRE_UNIT();
    u = curunit->type;
#ifdef DESIGNERS
    if (dside->designer) {
	kill_unit(curunit, -1);
	return;
    }
#endif /* DESIGNERS */
    if (u_hp_per_disband(u) > 0) {
	if (prep_disband_action(curunit, curunit)) {
	    /* (text should come from game design) */
	    notify(dside, "%s will go home.", unit_handle(dside, curunit));
	} else {
	    /* failed, should say something */
	}
    } else {
	cmd_error("You can't just get rid of %s!",
		  unit_handle(dside, curunit));
    }
}

/* Determine how far away another point is.  */

void
do_distance()
{
    int x, y;

    if (ask_position("Distance to where?", &x, &y)) {
	notify(dside, "Distance is %d cells.", distance(curx, cury, x, y));
    }
}

/* What about trying to embark a unit on itself or on its previous transp? */

void
do_embark()
{
    Unit *transport;

    REQUIRE_UNIT();
    if (ask_unit("Which unit to board?", &transport)) {
	if (!in_play(transport)) {
	    cmd_error("This transport is nonsensical");
	} else if (!can_occupy(curunit, transport)) {
	    cmd_error("Can't occupy");
	} else {
	    prep_enter_action(curunit, curunit, transport);
	    /* (should be able to set up task if can't do action immedly) */
	}
    }
}

/* Command to end our activity for this turn. */

void
do_end_turn()
{
    finish_turn(dside);
}

/* Command to fire at a specified unit or location. */

void
do_fire()
{
    int x, y;
    Unit *unit2;

    REQUIRE_UNIT();
    sprintf(spbuf, "Fire %s at where?", unit_handle(dside, curunit));
    /* (should have some sort of range feedback) */
    if (ask_position(spbuf, &x, &y)) {
	for_all_stack(x, y, unit2) {
	    if (g_see_all() /* or under observation in some way */
		&& unit2->side != curunit->side) {
		prep_fire_at_action(curunit, curunit, unit2, -1);
		return;
	    }
	}
	/* (should say that nothing is visible and verify firing) */
	if (1) {
	    prep_fire_into_action(curunit, curunit, x, y, 0, -1);
	} else {
	    xbeep();
	}
    }
}

void
do_fire_into()
{
}

/* Give supplies to a transport.  The argument tells how many to give. */

void
do_give()
{
    int n = cmdarg, something = FALSE, u, m, r, gift, actual;
    Unit *main;

    REQUIRE_UNIT();
    u = curunit->type;
    main = curunit->transport;
    if (main != NULL) {
	sprintf(spbuf, "");
	m = main->type;
	for_all_material_types(r) {
	    gift = (n < 0 ? (um_storage_x(m, r) - main->supply[r]) : n);
	    if (gift > 0) {
		something = TRUE;
		/* Be stingy if we're low */
		if (2 * curunit->supply[r] < um_storage_x(u, r))
		    gift = max(1, gift/2);
		actual = transfer_supply(curunit, main, r, gift);
		sprintf(tmpbuf, " %d %s", actual, m_type_name(r));
		strcat(spbuf, tmpbuf);
	    }
	}
	if (something) {
	    notify(dside, "%s gave%s.", unit_handle(dside, curunit), spbuf);
	} else {
	    notify(dside, "%s gave nothing.", unit_handle(dside, curunit));
	}
    } else {
	cmd_error("Can't transfer supplies here!");
    }
}

/* Give a unit to another side or "to" independence. */

void
do_give_unit()
{
    int u;

    REQUIRE_UNIT();
    u = curunit->type;
#ifdef DESIGNERS
    if (dside->designer) {
	unit_changes_side(curunit, side_n(cmdarg), -1, -1);
	/* (should update_unit_display also?) */
	all_see_cell(curunit->x, curunit->y);
	return;
    }
#endif /* DESIGNERS */
    if (1) { /* (should test both temporary and permanent invalidity) */
	prep_change_side_action(curunit, curunit, side_n(cmdarg));
    } else {
	cmd_error("You can't just give away the %s!", u_type_name(u));
    }
}

/* Bring up help info. */

static void
describe_help(arg, key, buf)
int arg;
char *key, *buf;
{
    sprintf(buf, "duh");
}

void
do_help()
{
    /* Switch to help mode. */
    prevmode = mode;
    mode = HELP;
    /* Get a help node and display its content. */
    if (cur_help_node == NULL) {
	help_help_node =
	  add_help_node("help", describe_help, 0, firsthelpnode);
	add_help_node("commands", describe_commands, 0, firsthelpnode);
	topics_help_node =
	  add_help_node("topics", describe_topics, 0, firsthelpnode);
	cur_help_node = topics_help_node;
    }
    helpstring = get_help_text(cur_help_node);
    show_help();
    refresh();
}

/* Send a short message to another side. */

void
do_message()
{
    char *msg;
    Side *side3 = NULL;

    if (cmdarg == 0) {
	/* (should ask who to send to) */
    }
    if (ask_string("Message: ", "", &msg)) {
	if (strlen(msg) == 0) {
	    notify(dside, "You keep your mouth shut.");
	} else if (1 /* broadcast? */) {
	    if (strlen(msg) > 0) {
		notify(dside, "You announce: %s", msg);
	    }
	} else {
	    notify(dside, "You send your message (?)");
	}
    }
}

/* Set unit to move to a given location.  Designers do a teleport. */

void
do_move_to()
{
    int x, y;

    REQUIRE_UNIT();
    sprintf(spbuf, "Move %s to where?", unit_handle(dside, curunit));
    if (ask_position(spbuf, &x, &y)) {
#ifdef DESIGNERS
	if (dside->designer) {
	    designer_teleport(curunit, x, y);
	    make_current(curunit);
	    return;
	}
#endif /* DESIGNERS */
	order_moveto(curunit, x, y);
    }
}

/* Command to name or rename the current unit or a given side. */

void
do_name()
{
    char *newname;

    REQUIRE_UNIT();
    if (ask_string("New name for unit:", curunit->name, &newname)) {
      if (strlen(newname) == 0)
	newname = NULL;
      set_unit_name(dside, curunit, newname);
    }
}

void
do_other()
{
    char *cmd;

    if (ask_string("ext cmd # ", NULL, &cmd)) {
	if (empty_string(cmd)) {
	    cmd_error("No command");
	} else if (strcmp(cmd, "?") == 0) {
	    do_help();
	    /* (should go to command list specifically) */
	} else {
	    execute_named_command(cmd);
	}
    }
}

void
do_print_view()
{
    dump_text_view(dside);
}

void
do_produce()
{
    cmd_error("command not implemented");
}

/* Command to get out of a game, one way or another. */

void
do_quit()
{
    Side *side2 = NULL;

    if (!dside->ingame) {
	exit_cconq(dside);
    } else if (0 /* other players think they can win */) {
	if (ask_bool("Do you really want to give up?", FALSE)) {
	    if (numsides > 2) {
/*		ask_side("Who do you want to surrender to?", NULL); */
		resign_game(dside, side2);
	    } else {
		resign_game(dside, NULL);
	    }
	}
    } else if (ask_bool("Do you really want to quit now?", FALSE)) {
	/* should force a loss first if necessary. */
	exit_cconq(dside);
    }
}

/* Move the current location as close to the center of the display as
   possible, and redraw everything. */

void
do_recenter()
{
    set_view_focus(mvp, curx, cury);
    center_on_focus(mvp);
    set_map_viewport();
    show_map();
    refresh();
}

/* Redraw everything using the same code as when windows need a redraw. */

void
do_refresh()
{
    redraw();
}

void
do_remove_terrain()
{
    int t, dir;

    REQUIRE_UNIT();
    if (ask_direction("Remove terrain from where?", &dir)) {
      for_all_terrain_types(t) {
	if (ut_acp_to_remove_terrain(curunit->type, t) > 0
	    && curunit->act
	    && curunit->act->acp >= ut_acp_to_remove_terrain(curunit->type, t)) {
	    if (0 <= ut_alter_range(curunit->type, t)) {
		if (prep_remove_terrain_action(curunit, curunit, curunit->x, curunit->y, dir, t))
		  ;
		else
		  xbeep();
	    }
	}
      }
    }
}

void
do_reserve()
{
    REQUIRE_UNIT();
    set_unit_asleep(dside, curunit, TRUE, TRUE);
}

/* Set unit to return to a good resupply place. */

void
do_return()
{
    REQUIRE_UNIT();
    set_resupply_task(curunit);
}

/* Stuff game state into a file.  By default, it goes into the current
   directory.  If building a scenario, we can specify just which parts
   of the game state are to be written. */

void
do_save()
{
    char *rawcontents;
    Module *module;
    Obj *contents;
    Side *side2;

#ifdef DESIGNERS
    if (dside->designer) {
	if (ask_string("Data to write?", "everything", NULL)) {
	    /* (should be in a designer_create_module?) */
	    /* need to be able to get this name from somewhere */
	    module = create_game_module("random.scn");
	    /* need something better to turn contents into a Lisp object */
	    contents = intern_symbol(rawcontents);
	    /*	interpret_content_spec(module, contents);  */
	    notify(dside, "Module will be written to \"%s\" ...", module->filename);
	    if (write_game_module(module)) {
		notify(dside, "Done writing to \"%s\".", module->filename);
	    } else {
		cmd_error("Can't open file \"%s\"!", module->filename);
	    }
	    return;
	} else {
	    return;
	}
    }
#endif /* DESIGNERS */
    if (0 /* checkpointing not allowed */) {
	if (ask_bool("You really want to save and exit?", FALSE)) {
	    notify(dside, "Game will be saved to \"%s\" ...", saved_game_filename());
	    if (write_entire_game_state(saved_game_filename())) {
		close_displays();
		/* this should be conditional? */
		exit(0);
	    } else {
		cmd_error("Can't open file \"%s\"!", saved_game_filename());
	    }
	}
    } else {
	notify(dside, "Saving...");
	if (write_entire_game_state(saved_game_filename())) {
	    notify(dside, "Game saved.");
	} else {
	    cmd_error("Couldn't save to \"%s\"!", saved_game_filename());
	}	    
    }
}

void
do_set_formation()
{
    Unit *leader;

    REQUIRE_UNIT();
    sprintf(spbuf, "Which unit to follow?");
    if (ask_unit(spbuf, &leader)) {
	if (!in_play(leader)) {
	    cmd_error("No unit to follow!");
	} else if (leader == curunit) {
	    cmd_error("Unit can't follow itself!");
	} else if (leader->side != dside /* or "trusted side"? */) {
	    cmd_error("Can't follow somebody else's unit!");
	} else {
	    set_formation(curunit, leader, curunit->x - leader->x, curunit->y - leader->y, 1, 1);
	}
    }
}

void
do_sleep()
{
    REQUIRE_UNIT();
    set_unit_asleep(dside, curunit, TRUE, TRUE);
}

/* Command to toggle between interaction modes. */

void
do_survey()
{
    if (mode == MOVE) {
	lastactor = curunit;
	mode = SURVEY;
    } else {
	mode = MOVE;
	/* If we weren't looking at a unit when we switched modes,
	   go back to the last unit that was being moved. */
	if (curunit == NULL && in_play(lastactor)) {
	    make_current(lastactor);
	}
    }
    show_map();
    refresh();
}

/* Take supplies from transport. */

void
do_take()
{
    int m, rslt, amts[MAXMTYPES];

    REQUIRE_UNIT();
    rslt = take_supplies(curunit, NULL, amts);
    if (rslt) {
	spbuf[0] = '\0';
	for_all_material_types(m) {
	    if (amts[m] > 0) {
		sprintf(tmpbuf, " %d %s", amts[m], m_type_name(m));
		strcat(spbuf, tmpbuf);
	    }
	}
	notify(dside, "%s got%s.", unit_handle(dside, curunit), spbuf);
    } else {
	notify(dside, "%s got nothing.", unit_handle(dside, curunit));
    }
}

void
do_take_unit()
{
    cmd_error("command not implemented");
}

#if 0
void
do_undelay()
{
    Unit *nextunit;

    if ((nextunit = find_prev_actor(dside, curunit)) != NULL) {
	make_current(nextunit);
    } else {
	xbeep();
    }
}
#endif

/* Wake *everything* (that's ours) within the given radius.  Two commands
   actually; "top-level" units (not in a transport) vs all units. */

/* Wake top-level units. */

void
do_wake()
{
    wake_area(dside, curx, cury, cmdarg, FALSE);
}

/* Wake all units found. */

void
do_wake_all()
{
    wake_area(dside, curx, cury, cmdarg, TRUE);
}

/* Display the program version. */

void
do_version()
{
    notify(dside, "Curses Xconq version %s", version_string());
    notify(dside, "(c) %s", copyright_string());
}

void
do_warranty()
{
    notify(dside, "There is no warranty.");
}

/* Curses-specific commands. */

void
do_c_change_list_type()
{
    cycle_list_type();
    show_list();
    refresh();
}


void
do_c_change_list_filter()
{
    cycle_list_filter();
    show_list();
    refresh();
}

void
do_c_change_list_order()
{
    cycle_list_order();
    show_list();
    refresh();
}

/* Toggle the action following flag. */

void
do_c_follow_action()
{
    follow_action = !follow_action;
    if (follow_action) {
	notify(dside, "Following the action.");
    } else {
	notify(dside, "Not following the action.");
    }
}

/* Commands to change the dividing line between the right-hand and left-hand
   windows of the screen. */

void
do_c_grow_map()
{
    if (cmdarg < 0)
      cmdarg = 5;
    if (lw - cmdarg < 5) {
	cmd_error("list side must be at least 5");
	return;
    }
    resize_map(cmdarg);
}

/* This is a clever (if I do say so myself) command to examine all occupants
   and suboccupants, in an inorder fashion. */

/* Should have an option to open up a list window that shows everything
   all at once. */

void
do_c_occupant()
{
    Unit *nextup;

    REQUIRE_UNIT();
    if (curunit->occupant != NULL) {
	make_current(curunit->occupant);
    } else if (curunit->nexthere != NULL) {
	make_current(curunit->nexthere);
    } else {
	nextup = curunit->transport;
	if (nextup != NULL) {
	    while (nextup->transport != NULL && nextup->nexthere == NULL) {
		nextup = nextup->transport;
	    }
	    if (nextup->nexthere != NULL)
	      make_current(nextup->nexthere);
	    if (nextup->transport == NULL)
	      make_current(nextup);
	} else {
	    /* This is a no-op if there is no stacking within a cell. */
	    make_current(unit_at(curunit->x, curunit->y));
	}
    }
}

void
do_c_shrink_map()
{
    if (cmdarg < 0)
      cmdarg = 5;
    if (mw + cmdarg < 10) {
	cmd_error("map side must be at least 10");
	return;
    }
    resize_map(0 - cmdarg);
}

static void
resize_map(n)
int n;
{
    /* Resize the left-hand-side windows. */
    mw += n;
    closeupwin->w += n;
    mapwin->w += n;
    /* Move and resize the right-hand-side windows. */
    lw -= n;
    sideswin->x += n;
    sideswin->w -= n;
    listwin->x += n;
    listwin->w -= n;
    /* Update the screen to reflect the changes. */
    set_scroll();
    redraw();
}

void
do_c_use_both_chars()
{
    use_both_chars = !use_both_chars;
    show_map();
    refresh();
}

#ifdef DESIGNERS

static int check_designer_status PROTO ((char *str));

/* The following commands are only available to designers. */

int curradius = 0;
int curttype = 0;
int curutype = 0;
int curfeature = 0;
int cursidenumber = 1;

static int
check_designer_status(str)
char *str;
{
    if (dside->designer) {
	return TRUE;
    } else {
	cmd_error("You're not a designer, can't %s!", str);
	return FALSE;
    }
}

void
do_design()
{
    if (!dside->designer) {
	become_designer(dside);
    } else {
	become_nondesigner(dside);
    }
}

void
do_c_set_unit_type()
{
    int u;

    if (!check_designer_status("set unit types to create"))
      return;
    u = ask_unit_type("Type of unit to create: ", NULL);
    if (u != NONUTYPE) {
	curutype = u;
	if (cmdarg >= 0)
	  cursidenumber = cmdarg;
	notify(dside, "will now be creating side %d %s units.",
	       cursidenumber, u_type_name(u));
    }
}

void
do_c_add_unit()
{
    Unit *unit;

    if (!check_designer_status("create units"))
      return;
    unit = designer_create_unit(dside, curutype, cursidenumber, curx, cury);
    if (unit != NULL) {
	make_current(unit);
    } else {
	cmd_error("Unit creation failed!");
    }
}

void
do_c_set_terrain_type()
{
    int t;

    if (!check_designer_status("set ttypes to paint"))
      return;
    t = ask_terrain_type("Type of terrain: ", NULL);
    if (t != NONTTYPE) {
	curttype = t;
	if (cmdarg >= 0)
	  curradius = cmdarg;
	notify(dside, "will now be painting %d-radius %s.",
	       curradius, t_type_name(t));
    }
}

/* Terrain painting command. */

void
do_c_paint_terrain()
{
    int t;

    /* (should ask for dir for linear types?) */
    if (!check_designer_status("paint terrain"))
      return;
    /* If command's arg is nonegative, interpret as temporary
       change of terrain type. */
    t = (cmdarg >= 0 ? cmdarg : curttype);
    paint_cell(dside, curx, cury, curradius, t);
}

/* (should add painting for all other layers here) */

#endif /* DESIGNERS */

#ifdef DEBUGGING

void
do_debug()
{
#ifndef Debug
    Debug = !Debug;
#endif
}

void
do_debugg()
{
#ifndef DebugG
    DebugG = !DebugG;
#endif
}

void
do_debugm()
{
#ifndef DebugM
    DebugM = !DebugM;
#endif
}

#endif /* DEBUGGING */

/* Search in command table and execute function if found, complaining if
   the command is not recognized.  Many commands operate on the "current
   unit", and all uniformly error out if there is no current unit, so put
   that test here.  Also fix up the arg if the value passed is one of the
   specially recognized ones. */

void
execute_command()
{
    if (execute_command_from_table(ccommands))
      return;
    if (execute_command_from_table(commands))
      return;
    cmd_error("unknown command character '%c'", inpch);
}

char tmpkey;

static int
execute_command_from_table(cmdtab)
CmdTab *cmdtab;
{
    CmdTab *cmd;
    char ch = inpch;
    int sx = -1, sy = -1;
    void (*fn) PROTO ((void));
    
    for (cmd = cmdtab; cmd->name != NULL; ++cmd) {
	if (ch == cmd->fchar) {
	    fn = cmd->fn;
	    if (fn == NULL) {
		run_warning("no command function for %s (0x%x)?",
			    cmd->name, ch);
		return TRUE;
	    }
	    tmpkey = ch;
	    (*fn)();
	    /* Whatever might have happened, we *did* find the command. */
	    return TRUE;
	}
    }
    return FALSE;
}

static void
execute_named_command(cmdstr)
char *cmdstr;
{
    /* Look for the command name in the curses-specific table. */
    if (execute_named_command_from_table(cmdstr, ccommands))
      return;
    /* Try the generic table. */
    if (execute_named_command_from_table(cmdstr, commands))
      return;
    cmd_error("unknown command name \"%s\"", cmdstr);
}

static int
execute_named_command_from_table(cmdstr, cmdtab)
char *cmdstr;
CmdTab *cmdtab;
{
    CmdTab *cmd;
    void (*fn) PROTO ((void));

    for (cmd = cmdtab; cmd->name != NULL; ++cmd) {
	if (strcmp(cmdstr, cmd->name) == 0) {
	    if ((fn = cmd->fn) == NULL) {
		run_warning("no command function for %s?", cmd->name);
		return TRUE;
	    }
	    tmpkey = cmd->fchar;
	    (*fn)();
	    /* Whatever might have happened, we *did* find the command. */
	    return TRUE;
	}
    }
    return FALSE;
}

/* Paste one-line descriptions of commands into the supplied buffer. */

static void
describe_commands(arg, key, buf)
int arg;
char *key, *buf;
{
    sprintf(spbuf, "To move a unit, use [hlyubn]\n");
    append_to_buffer(buf, spbuf);
    sprintf(spbuf, "[HLYUBN] moves unit repeatedly in that direction\n");
    append_to_buffer(buf, spbuf);
    sprintf(spbuf, "To look at another unit, use survey mode ('z')\n");
    append_to_buffer(buf, spbuf);
    sprintf(spbuf, "and use [hlyubnHLYUBN] to move the cursor\n");
    append_to_buffer(buf, spbuf);
    sprintf(spbuf, "\n");
    append_to_buffer(buf, spbuf);
    sprintf(spbuf, "Generic commands:\n");
    append_to_buffer(buf, spbuf);
    describe_command_table(arg, key, buf, commands);
    sprintf(spbuf, "Cconq-specific commands:\n");
    describe_command_table(arg, key, buf, ccommands);
}

static void
describe_command_table(arg, key, buf, cmdtab)
int arg;
char *key, *buf;
CmdTab *cmdtab;
{
    CmdTab *cmd;

    strcat(buf, "Single-key commands:\n\n");
    for (cmd = cmdtab; cmd->name != NULL; ++cmd) {
	describe_command (cmd->fchar, cmd->name, cmd->help, TRUE, buf);
    }
    strcat(buf, "\nLong name commands:\n\n");
    for (cmd = cmdtab; cmd->name != NULL; ++cmd) {
	describe_command (cmd->fchar, cmd->name, cmd->help, FALSE, buf);
    }
}

/* Generic command error routine just does a notify. */

static void
#ifdef __STDC__
cmd_error(char *fmt, ...)
#else
cmd_error(fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9)
char *fmt;
long a1, a2, a3, a4, a5, a6, a7, a8, a9;
#endif
{
    char tmpnbuf[BUFSIZE];
#ifdef __STDC__
    {
	va_list ap;

	va_start(ap, fmt);
	vsprintf(tmpnbuf, fmt, ap);
	va_end(ap);
    }
#else
    sprintf(tmpnbuf, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9);
#endif
    low_notify(tmpnbuf);
    xbeep();
}
