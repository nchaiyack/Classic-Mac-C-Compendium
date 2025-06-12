/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Help support for Xconq.  This is basically support code for interfaces,
   which handle the actual help interaction themselves. */

#include "conq.h"

tprintf(buf, str, a1, a2, a3, a4, a5, a6, a7, a8, a9)
char *buf, *str;
long a1, a2, a3, a4, a5, a6, a7, a8, a9;
{
    char line[300];

    sprintf(line, str, a1, a2, a3, a4, a5, a6, a7, a8, a9);
    strcat(buf, line);
}

/* The first help node in the chain, usually the list of topics. */

HelpNode *firsthelpnode = NULL;

/* The last help node. */

HelpNode *lasthelpnode = NULL;

/* Create an empty help node. */

HelpNode *
create_help_node()
{
    HelpNode *node = (HelpNode *) xmalloc(sizeof(HelpNode));

    node->key = NULL;
    node->fn = NULL;
    node->nclass = miscnode;
    node->arg = 0;
    node->text = NULL;
    node->prev = node->next = NULL;
    return node;
}

/* Add a help node after the given node. */

HelpNode *
add_help_node(key, fn, arg, prevnode)
char *key;
int (*fn)(), arg;
HelpNode *prevnode;
{
    HelpNode *node = create_help_node(), *nextnode;

    node->key = key;
    node->fn = fn;
    node->arg = arg;
    if (prevnode != NULL) {
	nextnode = prevnode->next;
    } else {
	prevnode = lasthelpnode;
	nextnode = firsthelpnode;
    }
    node->prev = prevnode;
    node->next = nextnode;
    prevnode->next = node;
    nextnode->prev = node;
    /* Might need to fix last help node. */
    lasthelpnode = firsthelpnode->prev;
    return node;
}

/* Given a string and node, find the next node whose key matches. */

HelpNode *
find_help_node(node, str)
HelpNode *node;
char *str;
{
    HelpNode *tmp;

    for (tmp = node->next; tmp != firsthelpnode /* or node? */; tmp = tmp->next) {
    	if (strcmp(tmp->key, str) == 0) return tmp;
    	if (strstr(tmp->key, str) != NULL) return tmp;
    }
    return NULL;
}

/* Create a raw list of help topics by just iterating through all the nodes. */

describe_topics(arg, key, buf)
int arg;
char *key, *buf;
{
    HelpNode *tmp;

    for (tmp = firsthelpnode->next; tmp != firsthelpnode; tmp = tmp->next) {
	tprintf(buf, "%s", tmp->key);
	tprintf(buf, "\n");
    }
}

/* Get the news file and put it into text buffer. */

describe_news(arg, key, buf)
int arg;
char *key, *buf;
{
    FILE *fp;

    if ((fp = fopen(newsfile_name(), "r")) != NULL) {
	tprintf(buf, "XCONQ NEWS\n\n");
	while (fgets(spbuf, BUFSIZE-1, fp) != NULL) {
	    tprintf(buf, "%s", spbuf);
	}
	fclose(fp);
    } else {
	tprintf(buf, "(no news)");
    }
}

/* Create the initial help node, which is the index of topics, and link
   it to itself.  Subsequent nodes will be inserted later, after a game
   has been loaded. */

init_help()
{
    firsthelpnode = create_help_node();
    firsthelpnode->key = "topics";
    firsthelpnode->fn = describe_topics;
    firsthelpnode->prev = firsthelpnode->next = firsthelpnode;
    lasthelpnode = firsthelpnode;
    add_help_node("news", describe_news, 0, NULL);
}

/* This function creates the actual set of help nodes for the kernel. */

void
create_game_help_nodes()
{
    int u, m, t;
    char *name, *longname;
    HelpNode *node;

    add_help_node("game design", describe_game_design, 0, NULL);
    add_help_node("modules", describe_game_modules, 0, NULL);
    add_help_node("scoring", describe_scorekeepers, 0, NULL);
    for_all_unit_types(u) {
	longname = u_long_name(u);
	if (longname != NULL && strlen(longname) > 0) {
	    sprintf(spbuf, "%s (%s)", longname, u_type_name(u));
	    name = copy_string(spbuf);
	} else {
	    name = u_type_name(u);
	}
	node = add_help_node(name, describe_utype, u, NULL);
	node->nclass = utypenode;
    }
    for_all_material_types(m) {
	node = add_help_node(m_type_name(m), describe_mtype, m, NULL);
	node->nclass = mtypenode;
    }
    for_all_terrain_types(t) {
	node = add_help_node(t_type_name(t), describe_ttype, t, NULL);
	node->nclass = ttypenode;
    }
    add_help_node("general concepts", describe_concepts, 0, NULL);
    /* Invalidate any existing topics node. */
    firsthelpnode->text = NULL;
}

/* Return the string containing the text of the help node, possibly
   computing it first. */

char *
get_help_text(node)
HelpNode *node;
{
    if (node != NULL) {
	/* Maybe calculate the text to display. */
	if (node->text == NULL) {
	    if (node->fn != NULL) {
		if ((node->text = xmalloc(5000)) != NULL) {
		    /* Make buffer into an empty string. */
		    (node->text)[0] = '\0';
		    node->textend = 0;
		    node->textsize = 5000;
		    (*(node->fn))(node->arg, node->key, node->text);
		    node->textend = strlen(node->text);
		} else {
		}
	    } else {
		sprintf(spbuf, "%s: No info available.", node->key);
		node->text = copy_string(spbuf);
		node->textend = strlen(node->text);
	    }
	}
	return node->text;
    } else {
	return NULL;
    }
}

describe_concepts(arg, key, buf)
int arg;
char *key, *buf;
{
    tprintf(buf, "Action points (Acp) are what units need to do ");
    tprintf(buf, "anything at all.");
    tprintf(buf, "\n");
}

/* Spit out all the general game_design parameters in a readable fashion.
   Some interfaces might have a better way of doing this, but that's up
   to the individual interface. */

describe_game_design(arg, key, buf)
int arg;
char *key, *buf;
{
    int u, m, t, i;

    /* Replicate title and blurb? */
    tprintf(buf, "This game includes %d unit types and %d terrain types",
	    numutypes, numttypes);
    if (nummtypes > 0) {
	tprintf(buf, ", along with %d material types", nummtypes);
    }
    tprintf(buf, ".\n");
    if (g_sides_min() == g_sides_max()) {
    	tprintf(buf, "Exactly %d sides may play.\n", g_sides_min());
    } else {
    	tprintf(buf, "From %d up to %d sides may play.\n", g_sides_min(), g_sides_max());
    }
    tprintf(buf, "\n");
    if (0 /* will/did do country placing */) {
        tprintf(buf, "Countries are %d cells across, between %d and %d cells apart.\n",
	    2 * g_min_radius() + 1,
	    g_min_separation(), g_max_separation());
    }
    tprintf(buf, "\n");
    if (g_see_all()) {
	tprintf(buf, "Everything is always seen by all sides.\n");
    } else {
    	if (g_see_terrain_always()) {
	    tprintf(buf, "Terrain view is always accurate once seen.\n");
    	}
    	if (g_see_weather_always()) {
	    tprintf(buf, "Weather view is always accurate once terrain seen.\n");
    	}
    	if (g_terrain_seen()) {
	    tprintf(buf, "World terrain is already seen by all sides.\n");
    	}
    }
    tprintf(buf, "\n");
    if (g_last_turn() < 9999) {
	tprintf(buf, "Game can go for up to %d turns", g_last_turn());
	if (g_extra_turn() > 0) {
	    tprintf(buf, ", with %d%% chance of additional turn thereafter.", g_extra_turn());
	}
	tprintf(buf, ".\n");
    }
    if (g_rt_for_game() > 0) {
	tprintf(buf, "Entire game can last up to %d minutes.\n",
		g_rt_for_game() / 60);
    }
    if (g_rt_per_turn() > 0) {
	printf(buf, "Each turn can last up to %d minutes.\n",
		g_rt_per_turn() / 60);
    }
    if (g_rt_per_side() > 0) {
	tprintf(buf, "Each side gets a total %d minutes to act.\n",
		g_rt_per_side() / 60);
    }
    if (g_units_in_game_max() >= 0) {
	tprintf(buf, "Limited to no more than %d units in all.\n", g_units_in_game_max());
    }
    if (g_units_per_side() >= 0) {
	tprintf(buf, "Limited to no more than %d units per side.\n", g_units_per_side());
    }
    tprintf(buf, "\nUnit Types:\n");
    for_all_unit_types(u) {
	tprintf(buf, "  %s", u_type_name(u));
	if (!empty_string(u_help(u))) tprintf(buf, " (%s)", u_help(u));
	tprintf(buf, "\n");
    }
    tprintf(buf, "\nTerrain Types:\n");
    for_all_terrain_types(t) {
	tprintf(buf, "  %s", t_type_name(t));
	if (!empty_string(t_help(t))) tprintf(buf, " (%s)", t_help(t));
	tprintf(buf, "\n");
    }
    if (nummtypes > 0) {
	tprintf(buf, "\nMaterial Types:\n");
	for_all_material_types(m) {
	    tprintf(buf, "  %s", m_type_name(m));
	    if (!empty_string(m_help(m))) tprintf(buf, " (%s)", m_help(m));
	    tprintf(buf, "\n");
	}
    }
}

/* Full details on the given type of unit. */

/* (The defaults should come from the *.def defaults!!) */

describe_utype(u, key, buf)
int u;
char *key, *buf;
{
    append_help_phrase(buf, u_help(u));
    if (u_point_value(u) > 0) {
    	tprintf(buf, "     (point value %d)\n", u_point_value(u));
    }
    if (u_can_be_self(u)) {
    	tprintf(buf, "Can be self-unit.\n");
    }
    if (u_acp(u) > 0) {
	    tprintf(buf, "Gets %d action points each turn", u_acp(u));
	    if (u_acp_min(u) != 0) {
		tprintf(buf, ", can go down to %d acp", u_acp_min(u));
	    }
	    if (u_acp_max(u) != -1) {
		tprintf(buf, ", can go up to %d acp", u_acp_max(u));
	    }
	    if (u_free_acp(u) != 0) {
		tprintf(buf, ", %d free", u_free_acp(u));
	    }
	    tprintf(buf, ".\n");
    } else {
	tprintf(buf, "Does not act.\n");
    }
    if (u_speed(u) > 0) {
	tprintf(buf, "Speed (mp/acp ratio) is %d.%d cells/acp.\n",
		u_speed(u) / 100, u_speed(u) % 100);
	if (u_mp_to_leave_world(u) >= 0) {
	    tprintf(buf, "%d mp to leave the world entirely.\n", u_mp_to_leave_world(u));
	}
    } else {
	tprintf(buf, "Does not move.\n");
    }
    tprintf(buf, "Hit Points: %d.", u_hp_max(u));
    if (u_parts(u) > 1) {
	tprintf(buf, "  Parts: %d.", u_parts(u));
    }
    if (u_hp_recovery(u) != 0) {
	tprintf(buf, "  Recovers by %d.%d HP each turn.",
		u_hp_recovery(u) / 100, u_hp_recovery(u) % 100);
    }
    tprintf(buf, "\n");
    if (u_capacity(u) != 0) {
	tprintf(buf, "Generic capacity for units is %d.\n",
		u_capacity(u));
    }
    if (u_cp(u) != 1) {
	tprintf(buf, "Construction points: %d.\n", u_cp(u));
    }
    if (u_tech_to_see(u) != 0) {
	tprintf(buf, "Tech to see: %d.\n", u_tech_to_see(u));
    }
    if (u_tech_to_use(u) != 0) {
	tprintf(buf, "Tech to use: %d.\n", u_tech_to_use(u));
    }
    if (u_tech_to_dissect(u) != 0) {
	tprintf(buf, "Tech to dissect: %d.\n", u_tech_to_dissect(u));
    }
    if (u_tech_to_build(u) != 0) {
	tprintf(buf, "Tech to build: %d.\n", u_tech_to_build(u));
    }
    if (u_tech_max(u) != 0) {
	tprintf(buf, "Tech max: %d.\n", u_tech_max(u));
    }
    if (u_acp_to_fire(u) > 0) {
	tprintf(buf, "\nCombat:\n");
   	if (u_acp_to_fire(u) > 0) {
		tprintf(buf, "Can fire (%d ACP), at ranges", u_acp_to_fire(u));
		if (u_range_min(u) > 0) {
	    		tprintf(buf, " from %d", u_range_min(u));
		}
		tprintf(buf, " up to %d", u_range(u));
		tprintf(buf, ".\n");
	}
	if (u_acp_to_detonate(u) > 0) {
		tprintf(buf, "Can detonate self (%d ACP)", u_acp_to_detonate(u));
		if (u_hp_per_detonation(u) < u_hp_max(u)) {
		    tprintf(buf, ", losing %d HP per detonation", u_hp_per_detonation(u));
		}
		tprintf(buf, ".\n"); 
	}
    }
    if (u_wrecked_type(u) != NONUTYPE) {
    	tprintf(buf, "Becomes a %s when destroyed.\n", u_type_name(u_wrecked_type(u)));
    }
    if (u_acp(u) > 0
        && (u_acp_to_change_side(u) > 0
            || u_acp_to_disband(u) > 0
            || u_acp_to_transfer_part(u) > 0
            )) {
	tprintf(buf, "\nOther Actions:\n");
	if (u_acp_to_change_side(u) > 0) {
	    tprintf(buf, "Can be given to another side (%d ACP).\n", u_acp_to_change_side(u));
	}
	if (u_acp_to_disband(u) > 0) {
	    tprintf(buf, "Can be disbanded (%d ACP)", u_acp_to_disband(u));
	    if (u_hp_per_disband(u) < u_hp_max(u)) {
	    	tprintf(buf, ", losing %d HP per action", u_hp_per_disband(u));
	    }
	    tprintf(buf, ".\n"); 
	}
	if (u_acp_to_transfer_part(u) > 0) {
	    tprintf(buf, "Can transfer parts (%d ACP).\n", u_acp_to_transfer_part(u));
	}
    }
    if (!g_see_all()) {
    	tprintf(buf, "\nVision:\n");
	tprintf(buf, "%d%% chance to be seen at outset of game.\n",
		u_already_seen(u));
	tprintf(buf, "%d%% chance to be seen at outset of game if independent.\n",
		u_already_seen_indep(u));
	switch (u_vision_range(u)) {
		case -1:
			tprintf(buf, "Can never see other units.\n");
			break;
		case 0:
			tprintf(buf, "Can see other units at own location.\n");
			break;
		case 1:
			/* Default range, no need to say anything. */
			break;
		default:
			tprintf(buf, "Can see units up to %d away.\n", u_vision_range(u));
			break;
	}
    }
    if (u_spy_chance(u) > 0 /* and random event in use */) {
	tprintf(buf, "%d%% chance to spy, on units up to %d away.",
		u_spy_chance(u), u_spy_range(u));
    }
    if (u_revolt(u) > 0 /* and random event in use */) {
	tprintf(buf, "%d.%d%% chance of revolt.\n", u_revolt(u) / 100, u_revolt(u) % 100);
    }
    /* Display the designer's notes for this type. */
    if (u_notes(u) != lispnil) {
	tprintf(buf, "\nNotes:\n");
	append_notes(buf, u_notes(u));
    }
}

append_help_phrase(buf, phrase)
char *buf, *phrase;
{
    if (phrase == NULL || strlen(phrase) == 0) return;
    tprintf(buf, "-- ");
    tprintf(buf, "%s", phrase);
    tprintf(buf, " --\n");
}

append_notes(buf, notes)
char *buf;
Obj *notes;
{
    char *notestr;
    Obj *rest;

    if (stringp(notes)) {
	notestr = c_string(notes);
	if (strlen(notestr) > 0) { 
	    tprintf(buf, "%s", notestr);
	    tprintf(buf, " ");
	} else {
	    tprintf(buf, "\n");
	}
    } else if (consp(notes)) {
	for (rest = notes; rest != lispnil; rest = cdr(rest)) {
	    append_notes(buf, car(rest));
	}
    } else {
	/* error? */
    }
}

/* A simple table-printing utility. Blanks out default values so they don't
   clutter the table. */

append_number(buf, value, dflt)
char *buf;
int value, dflt;
{
    if (value != dflt) {
	sprintf(tmpbuf, "%5d ", value);
	tprintf(buf, "%s", tmpbuf);
    } else {
	tprintf(buf, "      ");
    }
}

describe_mtype(m, key, buf)
int m;
char *key, *buf;
{
    append_help_phrase(buf, m_help(m));
    if (m_people(m) > 0) {
	tprintf(buf, "1 of this represents %d individuals.", m_people(m));
    }
    /* Display the designer's notes for this type. */
    if (m_notes(m) != lispnil) {
	tprintf(buf, "\nNotes:\n");
	append_notes(buf, m_notes(m));
    }
}

describe_ttype(t, key, buf)
int t;
char *key, *buf;
{
    extern int tempscanvary;
    extern int windscanvary;

    append_help_phrase(buf, t_help(t));
    switch (t_subtype(t)) {
      case cellsubtype:
	break;
      case bordersubtype:
	tprintf(buf, " (a border type)\n");
	break;
      case connectionsubtype:
	tprintf(buf, " (a connection type)\n");
	break;
      case coatingsubtype:
	tprintf(buf, " (a coating type)\n");
	break;
    }
    tprintf(buf, "Generic unit capacity is %d.\n", t_capacity(t));
    if (minelev != maxelev /* should be "elevscanvary" */) {
	if (t_elev_min(t) == t_elev_max(t)) {
	    tprintf(buf, "Elevation is always %d.\n",
		    t_elev_min(t));
	} else {
	    tprintf(buf, "Elevations fall between %d and %d.\n",
		    t_elev_min(t), t_elev_max(t));
	}
    }
    if (tempscanvary) {
	if (t_temp_min(t) == t_temp_max(t)) {
	    tprintf(buf, "Temperature is always %d.\n",
		    t_temp_min(t));
	} else {
	    tprintf(buf, "Temperatures fall between %d and %d, averaging %d.\n",
		    t_temp_min(t), t_temp_max(t), t_temp_avg(t));
	}
    }
    if (windscanvary) {
	if (t_wind_force_min(t) == t_wind_force_max(t)) {
	    tprintf(buf, "Wind force is always %d.\n",
		    t_wind_force_min(t));
	} else {
	    tprintf(buf, "Wind forces fall between %d and %d, averaging %d.\n",
		    t_wind_force_min(t), t_wind_force_max(t), t_wind_force_avg(t));
	}
	/* (plus need wind variability) */
    }
    if (0 /* cloudscanvary */) {
	if (t_clouds_min(t) == t_clouds_max(t)) {
	    tprintf(buf, "Cloud cover is always %d.\n",
		    t_clouds_min(t));
	} else {
	    tprintf(buf, "Cloud cover falls between %d and %d\n",
		    t_clouds_min(t), t_clouds_max(t));
	}
    }
    /* Display the designer's notes for this type. */
    if (t_notes(t) != lispnil) {
	tprintf(buf, "\nNotes:\n");
	append_notes(buf, t_notes(t));
    }
}

describe_scorekeepers(arg, key, buf)
int arg;
char *key, *buf;
{
    int i = 1, u, r, num;
    Scorekeeper *sk;

    if (scorekeepers == NULL) {
	tprintf(buf, "No scores are being kept.");
    } else {
	for_all_scorekeepers(sk) {
	    if (numscorekeepers > 1) {
		tprintf(buf, "%d.  ", i++);
	    }
	    if (symbolp(sk->body)
		&& match_keyword(sk->body, K_LAST_SIDE_WINS)) {
		tprintf(buf, "The last side left in the game wins.");
		/* (should mention point values also) */
	    } else {
		tprintf(buf, "(an indescribably complicated scorekeeper)");
	    }
	    tprintf(buf, "\n");
	}
    }
}

#ifdef USE_CONSOLE

/* Print the news file onto the console if there is anything to print. */

print_any_news()
{
    FILE *fp;

    if ((fp = fopen(newsfile_name(), "r")) != NULL) {
	printf("                              XCONQ NEWS\n\n");
	while (fgets(spbuf, BUFSIZE-1, fp) != NULL) {
	    fputs(spbuf, stdout);
	}
	/* Add another blank line, to separate from init printouts. */
	printf("\n");
	fclose(fp);
    }
}

#endif /* USE_CONSOLE */

/* Generate a readable description of the game (design) being played. */
/* This works by writing out appropriate help nodes, along with some
   indexing material.  This does *not* do interface-specific help,
   such as commands. */

print_game_description_to_file(fp)
FILE *fp;
{
    HelpNode *node;

    /* (need to work on which nodes to dump out) */
    for (node = firsthelpnode; node != firsthelpnode; node = node->next) {
	get_help_text(node);
	if (node->text != NULL) {
	    fprintf(fp, "\014\n%s\n", node->key);
	    fprintf(fp, "%s\n", node->text);
	}
    }
}
