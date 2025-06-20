/* Game modules for Xconq.
   Copyright (C) 1991, 1992, 1993, 1994 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"

extern int canaddutype, canaddmtype, canaddttype;

/* List of all known modules. Their descriptions can co-exist in memory,
   even if their contents cannot. */

Module *modulelist = NULL;

/* The main module defining the game in effect. */

Module *mainmodule = NULL;

char *moduledesigbuf = NULL;

/* Empty out the list of modules. */

void
clear_game_modules()
{
    modulelist = mainmodule = NULL;
}

/* Create a brand-new game module. */

Module *
create_game_module(name)
char *name;
{
    Module *module = (Module *) xmalloc(sizeof(Module));

    /* Nullify/zeroify most module slots. */
    memset(module, 0, sizeof(Module));
    /* Fill in nonzero slots. */
    /* The module's name must never be NULL. */
    if (name == NULL) name = "";
    module->name = name;
    module->instructions = lispnil;
    module->notes = lispnil;
    module->designnotes = lispnil;
    module->startlineno = 1;
    module->endlineno = 1;
    /* Add to front of module list. */
    module->next = modulelist;
    modulelist = module;
    return module;
}

Module *
find_game_module(name)
char *name;
{
    Module *module;

    if (name != NULL) {
	for_all_modules(module) {
	    if (module->name && strcmp(name, module->name) == 0) return module;
	}
    }
    return NULL;
}


/* Produce a module of the given name, either by finding it or creating it. */

Module *
get_game_module(name)
char *name;
{
    Module *module = find_game_module(name);

    if (module != NULL) return module;
    return create_game_module(name);
}

/* Make a game module for the given name and maybe bolt it into the include
   list of another module. */

Module *
add_game_module(name, includer)
char *name;
Module *includer;
{
    Module *module = get_game_module(name), *other;

    if (includer) {
	/* Add to the end of the list of include files. */
	if (includer->include == NULL) {
	    includer->include = includer->lastinclude = module;
	} else {
	    for (other = includer->include; other != NULL; other = other->nextinclude) {
		/* already here, just return it. */
		if (module == other) return module;
	    }
	    includer->lastinclude->nextinclude = module;
	    includer->lastinclude = module;
	}
    } else {
	/* an error? */
    }
    return module;
}

/* Display game module info to a side. */

void
describe_game_modules(arg, key, buf)
int arg;
char *key, *buf;
{
    if (mainmodule != NULL) {
	/* First put out basic module info. */
	describe_game_module_aux(buf, mainmodule, 0);
	/* Now do the lengthy module notes (with no indentation). */
	describe_module_notes(buf, mainmodule);
    } else {
	sprintf(buf, "(No game module information is available.)");
    }
}

/* Recurse down through included modules to display docs on each.
   Indents each file by inclusion level.  Note that modules cannot
   be loaded more than once, so each will be described only once here. */

void   
describe_game_module_aux(buf, module, level)
char *buf;
Module *module;
int level;
{
    int i;
    char indentbuf[100];
    Module *submodule;

    indentbuf[0] = '\0';
    for (i = 0; i < level; ++i) {
	strcat(indentbuf, "  ");
    }
    tprintf(buf, "%s\"%s\"", indentbuf,
	    (module->title ? module->title : module->name));
    if (module->title == NULL) {
	tprintf(buf, " (\"%s\")", module->name);
    }
    if (module->version != NULL) {
	tprintf(buf, " (version \"%s\")", module->version);
    }
    tprintf(buf, "\n");
    tprintf(buf, "%s          %s\n",
	    indentbuf,
	    (module->blurb ? module->blurb : "(no description)"));
    if (module->notes != lispnil) {
	tprintf(buf, "%s          (See notes below)\n", indentbuf);
    }
    /* Now describe any included modules. */
    for_all_includes(module, submodule) {
	describe_game_module_aux(buf, submodule, level + 1);
    }
}

/* Dump the module player's and designer's notes into the given buffer. */

void
describe_module_notes(buf, module)
char *buf;
Module *module;
{
    Module *submodule;

    if (module->notes != lispnil) {
	tprintf(buf, "\nNotes to \"%s\":\n", module->name);
	append_notes(buf, module->notes);
    }
    /* Only show design notes if any designers around. */
    if (numdesigners > 0 && module->designnotes != lispnil) {
	tprintf(buf, "\nDesign Notes to \"%s\":\n", module->name);
	append_notes(buf, module->designnotes);
    }
    for_all_includes(module, submodule) {
	describe_module_notes(buf, submodule);
    }
}

/* Sometimes we find ourselves lacking a game to provide meaning and
   context for interpretation; this routine loads the standard game
   (or a specified alternative default) immediately, but only makes
   it the main module if none defined. */

void
load_default_game()
{
    extern char *standard_game_name;
    char *defaultname = standard_game_name;
    Module *module;

    /* If we have a different default module, use it instead. */
    if (mainmodule != NULL && mainmodule->defaultbasemodulename != NULL) {
    	defaultname = mainmodule->defaultbasemodulename;
    }
    module = get_game_module(defaultname);
    if (mainmodule == NULL) mainmodule = module;
    load_game_module(module, TRUE);
}

/* Attempt to read just the first form in a module and use it as a
   description of the module.  Return true if this worked, false
   otherwise. */

int
load_game_description(module)
Module *module;
{
    Obj *form, *thecar;
    char *name;

    if (open_module(module, FALSE)) {
	if ((form = read_form(module->fp,
			      &(module->startlineno),
			      &(module->endlineno)))
	    != lispeof) {
	    if (consp(form) && symbolp(thecar = car(form))) {
		name = c_string(thecar);
		if (keyword_code(name) == K_GAME_MODULE) {
		    interp_game_module(form, module);
		    close_module(module);
		    /* Note that module is still not considered "loaded". */
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}

/* Game files can live in library directories or somewhere else.  This
   function tries to find a file, open it, and load the contents. */

void
load_game_module(module, dowarn)
Module *module;
int dowarn;
{
    char ch;

    if (open_module(module, dowarn)) {
	if (module->fp) {
	    /* Peek at the first character - was 'X' in old format files. */
	    ch = getc(module->fp);
	    ungetc(ch, module->fp);
	    if (ch == 'X') {
		init_error("\"%s\" is probably an obsolete Xconq file; in any case, it cannot be used.",
			   module->filename);
	    } else {
		/* New format, read it all. */
		read_forms(module);
	    }
	} else {
	    /* (should be able to read from contents string) */
	}
	/* We're done, can close. */
	close_module(module);
	/* Mark the module as having been loaded - note that this will happen
	   even if there were horrible errors. */
	module->loaded = TRUE;
	/* If the turn number has been set explicitly to a positive value,
	   assume that a saved game is being restored into the middle of the turn. */
	if (g_turn() > 0)
	  midturnrestore = TRUE;
	/* If the random state has been set explicitly to a nonnegative value,
	   use it to reseed the generator. */
	if (g_random_state() >= 0)
	  init_xrandom(g_random_state());
	/* Make all the cross-references right. */
	patch_object_references();
	/* Are all the types staked down now? */
	if (!canaddutype && !canaddmtype && !canaddttype) {
	    typesdefined = TRUE;
	    /* (should also record this as a sort of base module?) */
	}
    }
}

void
load_base_module(module)
Module *module;
{
    char *basename = module->basemodulename;
    Module *basemodule;

    if (!empty_string(basename)) {
	basemodule = find_game_module(basename);
	if (basemodule == NULL)
	  basemodule = add_game_module(basename, module);
	if (basemodule->loaded) {    
	    Dprintf("Base module `%s' already loaded.\n", basename);
	} else {
	    Dprintf("Loading base module `%s' ...\n", basename);
	    load_game_module(basemodule, FALSE);
	    Dprintf("... Done loading `%s'.\n", basename);
	}
    }
}

/* Given a module, attempt to open it. */

int
open_module(module, dowarn)
Module *module;
int dowarn;
{
    FILE *fp = NULL;

    /* Don't open more than once. */
    if (module->open && dowarn) {
	init_warning("Module \"%s\" is already open, ignoring attempt to reopen",
		     module->name);
	return FALSE;
    }
    /* Don't open if already loaded. */
    if (module->loaded && dowarn) {
	init_warning("Module \"%s\" is already loaded, ignoring attempt to reload",
		     module->name);
	return FALSE;
    }
    if (module->contents) {
	/* Uninterpreted contents already available, init the ptr. */
	module->sp = module->contents;
	Dprintf("Reading module \"%s\" from string ...\n", module->name);
    } else if ((fp = open_module_library_file(module)) != NULL) {
	/* Found the module in a library directory. */
	Dprintf("Reading module \"%s\" from library file \"%s\" ...\n",
		module->name, module->filename);
	module->fp = fp;
    } else if ((fp = open_module_explicit_file(module)) != NULL) {
	Dprintf("Reading module \"%s\" from file \"%s\" ...\n",
		module->name, module->filename);
	module->fp = fp;
    } else {
	if (dowarn) {
	    if (module->name) {
	    	init_warning("Can't find module \"%s\" anywhere",
			     module->name);
	    } else {
	    	init_warning("Can't find unnamed module anywhere");
	    }
	}
	return FALSE;
    }
    /* It worked, mark this module as open. */
    module->open = TRUE;
    return TRUE;
}

/* Read info about a side's preferences and setup. */

/* This assumes one form only, probably too restrictive. */
/* should read all the forms, use the relevant ones. */
/* (how does this interact with other defaults?) */
/* (should be delayed until player can confirm it...) */

/* (update to work like other module stuff? then can use resources etc) */
/* (fix so that correct name can be found reliably) */

int
load_side_config(side)
Side *side;
{
#if 0
    FILE *fp;
    Obj *config;
    Module *module;

    /* (should incorp config name somehow, also be sys-dependent) */
    module = create_game_module(side->player->name);

    if ((module->fp = fopen(module->filename, "r")) != NULL) {
	if ((config = read_form(module->fp,
				&(module->startlineno),
				&(module->endlineno)))
	    != lispeof) {
	    /* interpret the config */
	    Dprintf("Interpreting %s config form", side_desig(side));
	    Dprintlisp(config);
	    Dprintf("\n");
	    fill_in_side(side, config, TRUE);
	} else {
	    /* no config form in the file */
	}
    } else {
	init_warning("Module \"%s\" could not be opened", module->name);
	/* Not a disaster, keep going */
    }
#endif
    return FALSE;
}

/* Read an entire file, attempting to pick up objects in it. */

/* (does this interp game-module form twice if description previously
   loaded?) */

void
read_forms(module)
Module *module;
{
    Obj *form;

    Dprintf("Trying to read a new format file \"%s\"...\n", module->name);
    while ((form = read_form(module->fp,
			     &(module->startlineno),
			     &(module->endlineno)))
	   != lispeof) {
	interp_form(module, form);
    }
    Dprintf("... Done reading \"%s\".\n", module->name);
}

void
init_module_reshape(module)
Module *module;
{
    /* Seed all the reshaping parameters with reasonable values. */
    module->maybereshape = TRUE;
    module->subareawidth = area.width;
    module->subareaheight = area.height;
    module->subareax = module->subareay = 0;
    module->finalsubareawidth = area.width;
    module->finalsubareaheight = area.height;
    module->finalsubareax = module->finalsubareay = 0;
    module->finalwidth = area.width;  module->finalheight = area.height;
    module->finalcircumference = world.circumference;
    module->filltype = 0;
}

/* This is true if any actual reshaping is required. */

int
reshape_the_output(module)
Module *module;
{
    return (module->maybereshape
	    && (module->subareawidth != area.width
		|| module->subareaheight != area.height
		|| module->subareax != 0
		|| module->subareay != 0
		|| module->finalsubareawidth != area.width
		|| module->finalsubareaheight != area.height
		|| module->finalsubareax != 0
		|| module->finalsubareay != 0
		|| module->finalwidth != area.width
		|| module->finalheight != area.height
		|| module->finalcircumference != world.circumference));
}

/* Check if the proposed reshape will actually work. */

int
valid_reshape(module)
Module *module;
{
    /* (should check hexagon shaping) */
    if (module->subareawidth > area.width
	|| module->subareaheight > area.height) return FALSE;
    /* (should check other offsets) */
    if (module->finalwidth < 3 || module->finalheight < 3) return FALSE;
    return TRUE;
}

/* Close the module. */

void
close_module(module)
Module *module;
{
    if (module->sp) {
	module->sp = NULL;
    }
    if (module->fp) {
	fclose(module->fp);
	module->fp = NULL;
    }
    module->open = FALSE;
}

/* Return a description of the module. */

char *
module_desig(module)
Module *module;
{
    if (moduledesigbuf == NULL) moduledesigbuf = xmalloc(BUFSIZE);
    sprintf(moduledesigbuf, "module %s (%s)",
	    module->name, (module->title ? module->title : "no title"));
    return moduledesigbuf;
}

/* (random check, should be sent to right places) */

#ifdef DEBUGGING
/* Use this to ensure that everything is cool. */

void
doublecheck_state(side)
Side *side;
{
    Unit *unit;

    for_all_units(unit) {
	if (unit->x < 0 || unit->x >= area.width ||
	    unit->y <= 0 || unit->y >= (area.height - 1) ||
	    unit->hp <= 0) {
	    Dprintf("%s off map hp %d", unit_desig(unit), unit->hp);
	}
    }
}
#endif /* DEBUGGING */

#if 0 /* unused */
/* Return the value of an extension, or the default if it wasn't found. */

long
extension_value(extensions, key, dflt)
Obj *extensions;
char *key;
long dflt;
{
    Obj *bdg, *val;

    for ( ; extensions != lispnil; extensions = cdr(extensions)) {
	bdg = car(extensions);
	if (consp(bdg)
	    && symbolp(car(bdg))
	    && strcmp(c_string(car(bdg)), key) == 0) {
	    val = cadr(bdg);
	    /* Return number or string directly, Lisp value otherwise. */
	    if (numberp(val)) {
		return c_number(val);
	    } else if (stringp(val)) {
		return ((long) c_string(val));
	    } else {
		return ((long) val);
	    }
	}
    }
    return dflt;
}
#endif

void set_u_internal_name(u, s) int u; char *s; { utypes[u].iname = s; }
void set_u_type_name(u, s) int u; char *s; { utypes[u].name = s; }
void set_m_type_name(m, s) int m; char *s; { mtypes[m].name = s; }
void set_t_type_name(t, s) int t; char *s; { ttypes[t].name = s; }

/* If a special symbol, we might not have to fail. */

int
lazy_bind(sym)
Obj *sym;
{
    int u, m, t;
    Obj *value;

    switch (keyword_code(c_string(sym))) {
      case K_USTAR:
	value = lispnil;
	/* Since consing glues onto the front, iterate backwards
	   through the types. */
	for (u = numutypes - 1; u >= 0; --u) {
	    value = cons(new_utype(u), value);
	}
	break;
      case K_MSTAR:
	value = lispnil;
	for (m = nummtypes - 1; m >= 0; --m) {
	    value = cons(new_mtype(m), value);
	}
	break;
      case K_TSTAR:
	value = lispnil;
	for (t = numttypes - 1; t >= 0; --t) {
	    value = cons(new_ttype(t), value);
	}
	break;
      default:
	return FALSE;
    }
    setq(sym, value);
    return TRUE;
}

/* (should be in lisp.c, need to change _code to _value lookup on symbols) */

int
eval_boolean_expression(expr, fn, dflt)
Obj *expr;
int (*fn) PROTO ((Obj *)), dflt;
{
    char *opname;

    if (expr == lispnil) {
	return dflt;
    } else if (consp(expr) && symbolp(car(expr))) {
	opname = c_string(car(expr));
	switch (keyword_code(opname)) {
		case K_AND:
	    return (eval_boolean_expression(cadr(expr), fn, dflt)
		    && eval_boolean_expression(car(cddr(expr)), fn, dflt));
		case K_OR:
	    return (eval_boolean_expression(cadr(expr), fn, dflt)
		    || eval_boolean_expression(car(cddr(expr)), fn, dflt));
		case K_NOT:
	    return !eval_boolean_expression(cadr(expr), fn, dflt);
		default:
	    return (*fn)(expr);
	}
    } else {
	return (*fn)(expr);
    }
}

int
coerce_to_side_id(x)
Obj *x;
{
    if (numberp(x)) {
	return c_number(x);
    }
    return 0;
}

Side *
coerce_to_side(x)
Obj *x;
{
    return side_n(coerce_to_side_id(x));
}

int
coerce_to_unit_id(x)
Obj *x;
{
    if (numberp(x)) {
	return c_number(x);
    }
    return 0;
}

Unit *
coerce_to_unit(x)
Obj *x;
{
    return find_unit(coerce_to_unit_id(x));
}
