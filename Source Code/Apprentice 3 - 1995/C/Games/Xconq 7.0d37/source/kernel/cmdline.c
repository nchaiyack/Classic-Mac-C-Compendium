/* Command line parsing for Xconq.
   Copyright (C) 1987, 1988, 1989, 1991, 1992, 1993, 1994, 1995
   Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Command lines get parsed in several stages, since for instance
   the choice of game module will decide which variants are available. */

/* Command-line-interpreting programs always have a console. */

#ifndef USE_CONSOLE
#define USE_CONSOLE
#endif

#include "conq.h"
#include "cmdline.h"

int popup_dialog;  /* probably to be moved elsewhere.  Massimo */

extern int checkpointinterval;
extern int allbedesigners;

static void parse_world_option PROTO ((char *str));
static void parse_realtime_option PROTO ((char *subopt, char *arg));
static void parse_variant PROTO ((char *str));

static void add_a_raw_player_spec PROTO ((char *specstr));

/* The startup-settable options. */

static int option_alltimeout;
static int option_totalgametime;
static int option_perturntime;
static int option_persidetime;
static int option_add_default_player;
static int option_automate_default_player;
static int option_warnings;

static char *default_ai_type = ",mplayer";

/* This is a command-line parser that may be used in implementations that
   get a command line from somewhere. */

/* This says whether the command line arguments actually chose a
   specific game to play (as opposed to setting random flags). */

int gamechosen = FALSE;

/* The list of asked-for players. */

struct raw_spec {
  char *spec;
  struct raw_spec *next;
} *raw_player_specs, *last_raw_player_spec;

static char *raw_default_player_spec;

/* The list of modules to loaded in addition to the main one. */

struct module_spec {
  Module *module;
  struct module_spec *next;
} *extra_modules, *last_extra_module;

/* The list of accumulated variant choices. */

Obj *variant_settings;

char *programname = "";

int helpwanted = FALSE;

int haderror = FALSE;

/* Set the most basic of defaults on the dynamically-settable options. */

/* (need flags to indicate which options were actually used, so variant
   handling can warn about improper use) */

void
init_options()
{
    allbedesigners = FALSE;
    option_alltimeout = 0;
    option_totalgametime = 0;
    option_add_default_player = TRUE;
    option_automate_default_player = FALSE;
    variant_settings = lispnil;
}

/* This macro just checks that a required next argument is actually there. */

#define REQUIRE_ONE_ARG  \
  if (i + 1 >= argc) {  \
    fprintf(stderr, "Error: `%s' requires an argument\n", argv[i]);  \
    exit(1);  \
  }  \
  numused = 2;

/* Each of these causes argument parsing to skip over the option if it's
   not the right time to look at it. */

#define GENERAL_OPTION if (spec != general_options) continue;
#define VARIANT_OPTION if (spec != variant_options) continue;
#define PLAYER_OPTION  if (spec != player_options)  continue;

/* Generic command line parsing.  This is used by several different programs,
   so it just collects info, doesn't process it much.  This is called
   several times, because the validity of some args depends on which
   game modules are loaded and which players are to be in the game,
   and interfaces may need to do some of their own processing in between. */

void
parse_command_line(argc, argv, spec)
int argc, spec;
char *argv[];
{
    char *arg, *aispec, tmpspec[100];
    int i, n, numused;

    programname = argv[0];

    if (spec == general_options) init_options();

    for (i = 1; i < argc; ++i) {
	if (argv[i] == NULL || (argv[i])[0] == '\0') {
	    /* Already munched, nothing to do. */
	} else if ((argv[i])[0] == '-') {
	    arg = argv[i];
	    Dprintf("%s\n", arg);
	    numused = 1;
	    if (strcmp(arg, "-A") == 0) {
		PLAYER_OPTION;
		option_automate_default_player = TRUE;
	    } else if (strcmp(arg, "-ai") == 0) {
		REQUIRE_ONE_ARG;
		fprintf(stderr, "  %s not implemented yet, sorry\n", arg);
		haderror = TRUE;
	    } else if (strcmp(arg, "-design") == 0) {
		GENERAL_OPTION;
		allbedesigners = TRUE;
	    } else if (strcmp(arg, "-c") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		checkpointinterval = atoi(argv[i+1]);
	    } else if (strcmp(arg, "-x") == 0) {
		GENERAL_OPTION;
		popup_dialog = TRUE;
#ifdef DEBUGGING
	    } else if (strncmp(arg, "-D", 2) == 0) {
		GENERAL_OPTION;
		Debug = TRUE;
		if (strchr(arg+2, '-'))
		  Debug = FALSE;
		if (strchr(arg+2, 'M'))
		  DebugM = TRUE;
		if (strchr(arg+2, 'G'))
		  DebugG = TRUE;
#endif /* DEBUGGING */
	    } else if (strncmp(arg, "-e", 2) == 0) {
		REQUIRE_ONE_ARG;
		PLAYER_OPTION;
		n = atoi(argv[i+1]);
		/* A comma indicates that the name of a particular desired
		   AI type follows. */
		if (strlen(arg) > 2) {
		    aispec = arg + 2;
		    if (*aispec != ',') {
			sprintf(tmpspec, ",mplayer%s", aispec);
			aispec = tmpspec;
		    }
		} else {
		    aispec = default_ai_type;
		}
		while (n-- > 0)
		  add_a_raw_player_spec(aispec);
	    } else if (strcmp(arg, "-f") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		add_a_module(NULL, argv[i+1]); 
	    } else if (strcmp(arg, "-g") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		add_a_module(copy_string(argv[i+1]), NULL);
	    } else if (strcmp(arg, "-h") == 0) {
		REQUIRE_ONE_ARG;
		PLAYER_OPTION;
		fprintf(stderr, "  %s not implemented yet, sorry\n", arg);
		haderror = TRUE;
	    } else if (strcmp(arg, "-help") == 0) {
		GENERAL_OPTION;
		helpwanted = TRUE;
		/* Will display help info later. */
	    } else if (strcmp(arg, "-join") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		fprintf(stderr, "  %s not implemented yet, sorry\n", arg);
		haderror = TRUE;
	    } else if (strncmp(arg, "-L", 2) == 0) {
		GENERAL_OPTION;
		/* (should be adding to a search list) */
		xconqlib = copy_string(arg + 2);
	    } else if (strcmp(arg, "-M") == 0) {
		REQUIRE_ONE_ARG;
		VARIANT_OPTION;
		parse_world_option(argv[i+1]);
	    } else if (strcmp(arg, "-mail") == 0) {
		GENERAL_OPTION;
		fprintf(stderr, "  %s not implemented yet, sorry\n", arg);
		haderror = TRUE;
	    } else if (strcmp(arg, "-n") == 0) {
		PLAYER_OPTION;
		option_add_default_player = FALSE;
#ifdef DEBUGGING
	    } else if (strcmp(arg, "-R") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		init_xrandom(atoi(argv[i+1]));
#endif
	    } else if (strncmp(arg, "-t", 2) == 0) {
		REQUIRE_ONE_ARG;
		VARIANT_OPTION;
		parse_realtime_option(arg+2, argv[i+1]);
	    } else if (strncmp(arg, "-v", 2) == 0) {
		VARIANT_OPTION;
		parse_variant(arg+2);
	    } else if (strcmp(arg, "-w") == 0) {
		GENERAL_OPTION;
		option_warnings = FALSE;
	    } else if (strcmp(arg, "-wait") == 0) {
		REQUIRE_ONE_ARG;
		PLAYER_OPTION;
		fprintf(stderr, "  %s not implemented yet, sorry\n", arg);
		haderror = TRUE;
	    } else if (strcmp(arg, "--help") == 0) {
		GENERAL_OPTION;
	    } else if (strcmp(arg, "--version") == 0) {
		GENERAL_OPTION;
	    } else {
		numused = 0;
		/* Anything unrecognized during the last parse is an error. */
		if (spec >= leftover_options) {
		    fprintf(stderr, "Unrecognized option `%s'\n", arg);
		    haderror = TRUE;
		}
	    }
	    if (numused >= 1)
	      argv[i] = "";
	    if (numused >= 2)
	      argv[i+1] = "";
	    if (numused >= 1)
	      i += (numused - 1);
	} else {
	    if (spec == player_options) {
		if (*(argv[i]) == '+' || *(argv[i]) == '@') {
		    raw_default_player_spec = argv[i];
		} else {
		    add_a_raw_player_spec(argv[i]);
		}
		argv[i] = NULL;
	    }
	}
    }
    if (haderror || helpwanted) {
	if (helpwanted && mainmodule != NULL) {
	    load_all_modules();
	    /* (should display other random info about the game here) */
	}
	general_usage_info();
	exit(haderror);
    }
}

/* Given a module name and/or filename, add it to the list of modules
   to load. */

void
add_a_module(name, filename)
char *name, *filename;
{
    Module *module;

    module = get_game_module(name);
    if (module == NULL)
      exit(1);  /* bad error if happens */
    if (filename)
      module->filename = copy_string(filename);
    if (mainmodule == NULL) {
	mainmodule = module;
    } else {
	struct module_spec *extra = (struct module_spec *) xmalloc(sizeof(struct module_spec));

	extra->module = module;
	if (extra_modules == NULL)
	  extra_modules = extra;
	else
	  last_extra_module->next = extra;
	last_extra_module = extra;
    }
    gamechosen = TRUE;
}

static void
add_a_raw_player_spec(specstr)
char *specstr;
{
    struct raw_spec *spec =
	(struct raw_spec *) xmalloc (sizeof(struct raw_spec));

    spec->spec = copy_string(specstr);
    if (raw_player_specs == NULL)
      raw_player_specs = spec;
    else
      last_raw_player_spec->next = spec;
    last_raw_player_spec = spec;
}

/* This routine prints out help info about all the possible arguments. */

void
general_usage_info()
{
    printf("Usage:\n\t%s [ -options ... ]\n\n", programname);
    printf("General options:\n\n");
    printf("    -design\t\tmake every player a designer\n");
    printf("    -c n\t\tcheckpoint every <n> turns\n");
    printf("    -f filename\t\trun <filename> as a game\n");
    printf("    -g gamename\t\tfind <gamename> in library and run it\n");
    printf("    -help\t\tdisplay this help info\n");
    printf("    -join <game@host>\tconnect to the given game\n");
    printf("    -Lpathname\t\tset <pathname> to be library location\n");
    printf("    -mail\t\tset up game as play-by-mail\n");
    printf("    -t mins\t\tlimit each player to <mins> of play time total\n");
    printf("    -tside mins\t\tlimit each player to <mins> of time each turn\n");
    printf("    -tturn mins\t\tlimit each turn to <mins> minutes\n");
    printf("    -w\t\t\tsuppress warnings\n");
    printf("Long options:\n");
    printf("    --help\t\t\tdisplay this help info\n");
    printf("    --version\t\t\tdisplay version info\n");
    game_usage_info();
    printf("\nPlayer setup options:\n\n");
    player_usage_info();
    printf("\n");
}

/* Describe the available variants for a game. */

void
game_usage_info()
{
    int i;
    char *varname = "?", *vartypename = NULL;
    char buf[BUFSIZE];
    Variant *var;

    printf("\nGame variant options");
    if (mainmodule == NULL) {
	printf(":\n\n    No game loaded, no information available.\n\n");
	return;
    }
    printf(" for \"%s\":\n\n", mainmodule->name);
    if (mainmodule->variants == NULL) {
	printf("    No variants defined.\n\n");
	return;
    }
    for (i = 0; mainmodule->variants[i].id != lispnil; ++i) {
	var = &(mainmodule->variants[i]);
	varname = var->name;
	vartypename = c_string(var->id);
	switch (keyword_code(vartypename)) {
	  case K_WORLD_SEEN:
	    printf("    -v\t\t\tmake the world be seen already (default %s)\n",
		   (var->dflt == lispnil ? "true" :
		    (c_number(eval(var->dflt)) ? "true" : "false")));
	    break;
	  case K_SEE_ALL:
	    printf("    -vv\t\t\tmake everything be always seen (default %s)\n",
		   (var->dflt == lispnil ? "true" :
		    (c_number(eval(var->dflt)) ? "true" : "false")));
	    break;
	  case K_SEQUENTIAL:
	    printf("    -vseq\t\t\tmove sequentially (default %s)\n",
		   (var->dflt == lispnil ? "true" :
		    (c_number(eval(var->dflt)) ? "true" : "false")));
	    break;
	  case K_WORLD_SIZE:
	    /* (is this accurate?) */
	    printf("    -M width[xheight][Wcircum][+lat][+lon]\tset world size (default ?)\n");
	    break;
	  default:
	    unixify_variant_name(buf, varname);
	    printf("    -v%s[=value] (default ", buf);
	    sprintlisp(buf, var->dflt);
	    printf(")\n");
	    break;
	}
    }
}

/* Replace blanks in a variant's name with hyphens, and put the whole
   name in lowercase. */

void
unixify_variant_name(buf, varname)
char *buf, *varname;
{
    int i, slen;

    strcpy(buf, varname);
    slen = (int) strlen(buf);
    for (i = 0; i < slen; ++i) {
	if (buf[i] == ' ')
	  buf[i] = '-';
	if (isupper(buf[i]))
	  buf[i] = tolower(buf[i]);
    }
}

void
player_usage_info()
{
    printf("    name[,ai][/config][@host][+advantage]\tadd player\n");
    printf("        ai\t\t= name of AI type\n");
    printf("        config\t\t= name of config file\n");
    printf("        host\t\t= name of player's host machine or display\n");
    printf("        advantage\t= numerical initial advantage (default 1)\n");
    printf("    -A\t\t\tuse AI with default player\n");
    printf("    -e number[,ai]\tadd <number> computer players\n");
    printf("    -h number[,ai]\tadd <number> human players\n");
    printf("    -n\t\t\tno default player on local display\n");
    printf("    -wait minutes\t\twait time for players to join\n");
}

/* Given a string representing world dimensions, extract various components
   and compose a variant setting. */

static void
parse_world_option(str)
char *str;
{
    int width, height, circumference = 0;
    char *str2;

    width = atoi(str);
    if ((str2 = strchr(str, 'x')) != NULL) {
	height = atoi(str2 + 1);
    } else {
	height = width;
    }
    if ((str2 = strchr(str, 'W')) != NULL) {
	circumference = atoi(str2 + 1);
    } else {
	/* (should get variant's default?) */
    }
    /* Glue onto the list of variant_settings. */
    push_key_cdr_binding(&variant_settings, K_WORLD_SIZE,
			 cons(new_number(width),
			      cons(new_number(height),
				   cons(new_number(circumference),
					lispnil))));
}

static void
parse_realtime_option(subopt, arg)
char *subopt, *arg;
{
    if (strcmp(subopt, "-timeout") == 0) {
	option_alltimeout = 60 * atoi(arg);
    } else if (strcmp(subopt, "-tgame") == 0) {
	option_totalgametime = 60 * atoi(arg);
    } else if (strcmp(subopt, "-tside") == 0) {
	option_persidetime = 60 * atoi(arg);
    } else if (strcmp(subopt, "-tturn") == 0) {
	option_perturntime = 60 * atoi(arg);
    } else {
    	/* usage? */
    }
}

/* Given a variant, turn it into a list "(name val)". */

static void
parse_variant(str)
char *str;
{
    char *varname = NULL, *str2;
    Obj *varval = lispnil;

    if (strcmp(str, "") == 0) {
	push_key_int_binding(&variant_settings, K_WORLD_SEEN, 1);
    } else if (strcmp(str, "v") == 0) {
	push_key_int_binding(&variant_settings, K_SEE_ALL, 1);
    } else if (strcmp(str, "all") == 0) {
	push_key_int_binding(&variant_settings, K_SEE_ALL, 1);
    } else if (strcmp(str, "seq") == 0) {
	push_key_int_binding(&variant_settings, K_SEQUENTIAL, 1);
    } else if (strcmp(str, "simul") == 0) {
	push_key_int_binding(&variant_settings, K_SEQUENTIAL, 0);
    } else {
	str2 = strchr(str, '=');
	if (str2 != NULL && str2 != str) {
	    /* (should interp val as string or number) */
	    varval = new_number(atoi(str2 + 1));
	    varname = copy_string(str);
	    varname[str2 - str] = '\0';
	} else {
	    varname = str;
	    varval = new_number(1);
	}
	if (varname)
	  push_binding(&variant_settings, intern_symbol(varname), varval);
    }
}

/* Load all the game modules that were asked for on cmd line. */

void
load_all_modules()
{
    struct module_spec *extra;

    if (mainmodule != NULL) {
	load_game_module(mainmodule, TRUE);
	for (extra = extra_modules; extra != NULL; extra = extra->next) {
	    load_game_module(extra->module, TRUE);
	}
    } else {
	/* If we've got absolutely no files to load, the standard game is
	   the one to go for.  It will direct the remainder of random gen. */
	load_default_game();
    }
}

/* Set module variants from command line options. */

void
set_variants_from_options()
{
    do_module_variants(mainmodule, variant_settings);
}

/* Set player characteristics from command-line options. */

void
set_players_from_options()
{
    Player *player;
    struct raw_spec *spec;

    /* Assume that if players exist already, then this is a restored
       game, and don't allow the command line to mess with the restored
       players.  This is not ideal, because it means there is no way
       to edit the player list, perhaps because one of the players
       wants to be displayed on a different screen. */
    if (numplayers > 0)
      return;
    /* Add the default player. */
    if (raw_player_specs == NULL || option_add_default_player) {
	player = add_default_player();
	if (option_automate_default_player)
	  player->aitypename = "mplayer";
	parse_player_spec(player, raw_default_player_spec);
	canonicalize_player(player);
    }
    /* Add the explicitly listed players. */
    for (spec = raw_player_specs; spec != NULL; spec = spec->next) {
	player = add_player();
	parse_player_spec(player, spec->spec);
	canonicalize_player(player);
    }
}

/* Parse the syntax "[username][,ai][/config][@display][+advantage]". */

void
parse_player_spec(player, spec)
Player *player;
char *spec;
{
    int commapos, slashpos, atpos, pluspos;

    if (spec != NULL && strcmp(spec, "*") != 0) {
	/* Extract (destructively) a trailing advantage specification. */
	pluspos = iindex('+', spec);
	if (pluspos >= 0) {
	    player->advantage = max(1, atoi(&(spec[pluspos + 1])));
	    spec[pluspos] = '\0';
	}
	/* Extract a displayname if given. */
	atpos = iindex('@', spec);
	if (atpos >= 0) {
	    player->displayname = copy_string(spec + atpos + 1);
	    spec[atpos] = '\0';
	}
	/* Extract a configuration name if given. */
	slashpos = iindex('/', spec);
	if (slashpos >= 0) {
	    player->configname = copy_string(spec + slashpos + 1);
	    spec[slashpos] = '\0';
	}
	/* Extract an AI type if given. */
	commapos = iindex(',', spec);
	if (commapos >= 0) {
	    player->aitypename = copy_string(spec + commapos + 1);
	    spec[commapos] = '\0';
	}
	/* Just a plain old string left. */
	if (strlen(spec) > 0) {
	    if (atpos >= 0) {
		/* Display given separately, so this is a name. */
		player->name = copy_string(spec);
	    } else {
		player->displayname = copy_string(spec);
	    }
	}
    }
    canonicalize_player(player);
}

/* This is not, strictly speaking, part of command line processing,
   but command-line programs also have stdio usually. */

void
print_instructions()
{
    Obj *instructions = mainmodule->instructions, *rest;

    printf("\n");
    if (instructions != lispnil) {
	if (stringp(instructions)) {
	    printf("%s\n", c_string(instructions));
	} else if (consp(instructions)) {
	    for (rest = instructions; rest != lispnil; rest = cdr(rest)) {
		if (stringp(car(rest))) {
		    printf("%s\n", c_string(car(rest)));
		} else {
		    /* (should probably warn about this case too) */
		}
	    }
	} else {
	    run_warning("Instructions are of wrong type");
	}
    } else {
	printf("(no instructions supplied)\n");
    }
}
