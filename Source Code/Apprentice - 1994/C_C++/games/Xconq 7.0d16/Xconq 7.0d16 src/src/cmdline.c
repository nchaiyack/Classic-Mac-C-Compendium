/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Command-line-interpreting programs always have a console. */

#ifndef USE_CONSOLE
#define USE_CONSOLE
#endif

#include "conq.h"

char *strchr();

extern int checkpointinterval;
extern int allbedesigners;

/* The startup-settable options. */

int options_seeall;
int options_alldesigners;
int options_alltimeout;
int options_totalgametime;
int options_perturntime;
int options_adddefaultplayer;
int options_automatedefaultplayer;
int options_warnings;

char *defaultaitype = "mplayer";

/* This is a command-line parser that may be used in implementations that
   get a command line from somewhere. */

/* This says whether the command line arguments actually chose a
   specific game to play (as opposed to setting random flags). */

int gamechosen = FALSE;

/* This is the array of asked-for players. */

char *rawplayers[100];
int numrawplayers = 0;

int numextramodules = 0;
Module *extramodules[100];

Obj *variants;

char *rawvariants[100];
int numrawvariants = 0;

char *programname = "";

int helpwanted = FALSE;

int haderror = FALSE;

/* Set the most basic of defaults on the dynamically-settable options. */

/* (need flags to indicate which options were actually used, so variant
   handling can warn about improper use) */

init_options()
{
    allbedesigners = FALSE;
    options_alltimeout = 0;
    options_totalgametime = 0;
    options_adddefaultplayer = TRUE;
    options_automatedefaultplayer = FALSE;
    variants = lispnil;
}

/* This macro just checks that a required next argument is actually there. */

#define REQUIRE_ONE_ARG  \
  if (i+1 >= argc) {  \
    fprintf(stderr, "Error: `%s' requires an argument\n", argv[i]);  \
    exit(1);  \
  }  \
  numused = 2;

/* Each of these causes argument parsing to skip over the option if it is
   not the right time to look at it. */

#define GENERAL_OPTION if (spec != general_options) continue;
#define VARIANT_OPTION if (spec != variant_options) continue;
#define PLAYER_OPTION  if (spec != player_options)  continue;

/* Generic command line parsing.  This is used by several different programs,
   so it just collects info, doesn't process it much.  This is called
   several times, because the validity of some args depends on which
   game modules are loaded and which players are to be in the game. */

parse_command_line(argc, argv, spec)
int argc, spec;
char *argv[];
{
    char ch, *arg, *aitype;
    int i, n, numused;
    Module *module;

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
		options_automatedefaultplayer = TRUE;
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
#ifdef DEBUGGING
	    } else if (strncmp(arg, "-D", 2) == 0) {
		GENERAL_OPTION;
		Debug = TRUE;
		if (strchr(arg+2, '-')) Debug = FALSE;
		if (strchr(arg+2, 'M')) DebugM = TRUE;
		if (strchr(arg+2, 'G')) DebugG = TRUE;
#endif /* DEBUGGING */
	    } else if (strcmp(arg, "-e") == 0) {
		REQUIRE_ONE_ARG;
		PLAYER_OPTION;
		n = atoi(argv[i+1]);
		/* A comma indicates that the name of a particular desired
		   AI type follows. */
		if (strchr(argv[i+1], ',')) {
		    aitype = copy_string(strchr(argv[i+1], ',') + 1);
		} else {
		    aitype = defaultaitype;
		}
		while (n-- > 0) rawplayers[numrawplayers++] = aitype;
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
		options_adddefaultplayer = FALSE;
#ifdef DEBUGGING
	    } else if (strcmp(arg, "-R") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		init_xrandom(atoi(argv[i+1]));
#endif
	    } else if (strcmp(arg, "-t") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		options_alltimeout = 60 * atoi(argv[i+1]);
	    } else if (strcmp(arg, "-T") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		options_totalgametime = 60 * atoi(argv[i+1]);
	    } else if (strcmp(arg, "-TT") == 0) {
		REQUIRE_ONE_ARG;
		GENERAL_OPTION;
		options_perturntime = 60 * atoi(argv[i+1]);
	    } else if (strncmp(arg, "-v", 2) == 0) {
		VARIANT_OPTION;
		parse_view_variant(arg+2);
	    } else if (strncmp(arg, "-V", 2) == 0) {
		VARIANT_OPTION;
		parse_general_variant(arg+2);
	    } else if (strcmp(arg, "-w") == 0) {
		GENERAL_OPTION;
		options_warnings = FALSE;
	    } else if (strcmp(arg, "-wait") == 0) {
		REQUIRE_ONE_ARG;
		PLAYER_OPTION;
		fprintf(stderr, "  %s not implemented yet, sorry\n", arg);
		haderror = TRUE;
	    } else {
		numused = 0;
		/* Anything unrecognized during the last parse is an error. */
		if (spec >= leftover_options) {
		    fprintf(stderr, "Unrecognized option `%s'\n", arg);
		    haderror = TRUE;
		}
	    }
	    if (numused >= 1) argv[i] = "";
	    if (numused >= 2) argv[i+1] = "";
	    if (numused >= 1) i += (numused - 1);
	} else {
	    if (spec == player_options) {
		rawplayers[numrawplayers++] = argv[i];
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

add_a_module(name, filename)
char *name, *filename;
{
    Module *module;

    module = get_game_module(name);
    if (module == NULL) exit(1);  /* bad error if happens */
    if (filename) module->filename = copy_string(filename);
    if (mainmodule == NULL) {
	mainmodule = module;
    } else {
	extramodules[numextramodules++] = module;
    }
    gamechosen = TRUE;
}
    
/* This routine prints out help info about all the possible arguments. */

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
    printf("    -T mins\t\tlimit each player to <mins> of time each turn\n");
    printf("    -TT mins\t\tlimit each turn to <mins> minutes\n");
    printf("    -w\t\t\tsuppress warnings\n");
    game_usage_info();
    printf("\nPlayer setup options:\n\n");
    player_usage_info();
    printf("\n");
}

game_usage_info()
{
    int dflt, i;
    char *varname = "?", *vartypename = NULL;
    char buf[BUFSIZE];
    Obj *vars, *var, *vartype, *vardata;

    printf("\nGame variant options");
    if (mainmodule == NULL) {
	printf(":\n\n    No game loaded, no information available.\n\n");
	return;
    }
    printf(" for \"%s\":\n\n", mainmodule->name);
    if (mainmodule->variants == lispnil) {
	printf("    No variants defined.\n\n");
	return;
    }
    for (vars = mainmodule->variants; vars != lispnil; vars = cdr(vars)) {
	var = car(vars);
	if (stringp(car(var))) {
	    varname = c_string(car(var));
	    var = cdr(var);
	}
	vartype = car(var);
	vardata = cdr(var);
	if (symbolp(vartype)) vartypename = c_string(vartype);
	switch (keyword_code(vartypename)) {
	  case K_WORLD_SEEN:
	    printf("    -v\t\t\tmake the world be seen already (default %s)\n",
		   (c_number(eval(car(vardata))) ? "true" : "false"));
	    break;
	  case K_SEE_ALL:
	    printf("    -vv\t\t\tmake everything be always seen (default %s)\n",
		   (c_number(eval(car(vardata))) ? "true" : "false"));
	    break;
	  case K_WORLD_SIZE:
	    printf("    -M width[xheight][Ocircum][+lat][+lon]\tset world size\n");
	    break;
	  default:
	    unixify_variant_name(buf, varname);
	    printf("    -V%s[=value]\n", buf);
/*	    sprintlisp(buf, car(vars));  */
	    break;
	}
    }
}

/* Replace blanks in a variant's name with hyphens, and put the whole
   name in lowercase. */

unixify_variant_name(buf, varname)
char *buf, *varname;
{
    int i;

    strcpy(buf, varname);
    for (i = 0; i < strlen(buf); ++i) {
	if (buf[i] == ' ') buf[i] = '-';
	if (isupper(buf[i])) buf[i] = tolower(buf[i]);
    }
}

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

parse_world_option(str)
char *str;
{
    int width, height;
    char *str2;

    width = atoi(str);
    if ((str2 = strchr(str, 'x')) != NULL) {
	height = atoi(str2 + 1);
    } else {
	height = width;
    }
    /* Glue onto the list of variants. */
    variants = cons(cons(intern_symbol("world-size"),
			 cons(new_number(width),
			      cons(new_number(height),
				   lispnil))),
		    variants);
}

parse_view_variant(str)
char *str;
{
    if (strcmp(str, "") == 0) {
	variants = cons(cons(intern_symbol("world-seen"),
			     cons(intern_symbol("true"),
				  lispnil)),
			variants);
    } else if (strcmp(str, "v") == 0) {
	variants = cons(cons(intern_symbol("see-all"),
			     cons(intern_symbol("true"),
				  lispnil)),
			variants);
    } else {
    }
}

parse_general_variant(str)
char *str;
{
    if (strcmp(str, "") != 0) {
	variants = cons(cons(intern_symbol(str),
			     cons(intern_symbol("true"),
				  lispnil)),
			variants);
    }
}

/* Load all the game modules that were asked for on cmd line. */

load_all_modules()
{
    int i;

    if (mainmodule != NULL) {
	load_game_module(mainmodule, TRUE);
	for (i = 0; i < numextramodules; ++i) {
	    load_game_module(extramodules[i], TRUE);
	}
    } else {
	/* If we've got absolutely no files to load, the standard game is
	   the one to go for.  It will direct the remainder of random gen. */
	load_default_game();
    }
}

/* Set module variants from command line options. */

set_variants_from_options()
{
    do_module_variants(mainmodule, variants);
}

find_raw_variant(varname)
{
    int i;
    char buf[BUFSIZE];

    unixify_variant_name(buf, varname);

    for (i = 0; i < numrawvariants; ++i) {
	if (strcmp(buf, rawvariants[i]) == 0) {
	    return TRUE;
	}
    }
    return FALSE;
}

/* Set player characteristics from command-line options. */

set_players_from_options()
{
    int i;
    Player *player;

    if (numrawplayers == 0 || options_adddefaultplayer) {
	player = add_default_player();
	if (options_automatedefaultplayer) {
	    player->aitypename = "mplayer";
	}
	canonicalize_player(player);
    }
    for (i = 0; i < numrawplayers; ++i) {
	player = add_player();
	if (strcmp(rawplayers[i], "mplayer") == 0) { /* not really needed? */
	    player->aitypename = "mplayer";
	} else {
	    parse_player_spec(player, rawplayers[i]);
	    player->displayname = rawplayers[i];
	}
	canonicalize_player(player);
    }
}

/* Parse the syntax "username,ai/config@display+advantage". */

parse_player_spec(player, spec)
Player *player;
char *spec;
{
    if (spec != NULL && strcmp(spec, "*") != 0) {
	/* Extract a trailing advantage specification. */
	if (iindex('+', spec) >= 0) {
	    player->displayname = copy_string(spec + iindex('@', spec) + 1);
	    player->advantage = max(1, atoi(&(spec[iindex('+', spec)+ 1])));
	    spec[iindex('+', spec)] = '\0';
	}
	/* Extract a displayname. */
	if (iindex('@', spec) >= 0) {
	    player->displayname = copy_string(spec + iindex('@', spec) + 1);
	    if (strlen(player->displayname) == 0) player->displayname = NULL;
	    spec[iindex('@', spec)] = '\0';
	    player->name = spec;
	} else {
	    if (strcmp(spec, "mplayer") == 0) {
		player->aitypename = copy_string(spec);
	    } else {
		player->displayname = copy_string(spec);
	    }
	}
	/* Separate user and configuration name if both given. */
	if (iindex('/', player->name) >= 0) {
	    player->configname =
	      copy_string(player->name + iindex('/', spec) + 1);
	    (player->name)[iindex('/', spec)] = '\0';
	}
    }
    canonicalize_player(player);
}
