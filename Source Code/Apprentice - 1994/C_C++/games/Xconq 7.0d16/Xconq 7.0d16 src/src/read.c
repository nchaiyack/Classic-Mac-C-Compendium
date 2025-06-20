/* Copyright (c) 1989-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Interpret the forms found in a game module. */

/* Syntax is el cheapo Lisp. */

#include "conq.h"

Unit *find_unit_by_name();

struct a_key {
    char *name;
    enum keywords key;
    short value;
} keywordtable[] = {

#undef  DEF_KWD
#define DEF_KWD(NAME,CODE,VALUE)  { NAME, CODE, VALUE },

#include "keyword.def"

    { NULL, 0 }
};

keyword_code(str)
char *str;
{
    int i;

    /* (should do a binary search first, then switch to exhaustive) */
    for (i = 0; keywordtable[i].name != NULL; ++i) {
	if (strcmp(str, keywordtable[i].name) == 0) return keywordtable[i].key;
    }
    return (-1);
}

char *
keyword_name(k)
enum keywords k;
{
    return keywordtable[k].name;
}

keyword_value(k)
enum keywords k;
{
    return keywordtable[k].value;
}

#define TYPEPROP(TYPES, N, DEFNS, I, TYPE)  \
  ((TYPE *) &(((char *) (&(TYPES[N])))[DEFNS[I].offset]))[0]

/* This is a generic syntax check and escape. */

#define SYNTAX(X,TEST,MSG)  \
  if (!(TEST)) { syntax_error((X), (MSG));  return; }
  
syntax_error(x, msg)
Obj *x;
char *msg;
{
    char foobuf[BUFSIZE];

    sprintlisp(foobuf, x);
    init_warning("syntax error in %s - %s", foobuf, msg);
}

#define TYPECHECK(PRED,X,MSG)  \
  if (!PRED(X)) { type_error((X), (MSG));  return; }

type_error(x, msg)
Obj *x;
char *msg;
{
    char foobuf[BUFSIZE];

    sprintlisp(foobuf, x);
    init_warning("type error in %s - %s", foobuf, msg);
}

#define PARSE_PROPERTY(BDG,NAME,VAL)  \
  SYNTAX(bdg, (consp(bdg) && symbolp(car(bdg))), "property binding");  \
  (NAME) = c_string(car(BDG));  \
  (VAL) = cadr(BDG);

/* This is the list of side defaults that will be applied
   to all sides read subsequently. */

Obj *sidedefaults;

/* These variables indicate whether new types can still be defined.
   Once a table or list of types is manipulated, these are turned off. */

int canaddutype = TRUE;
int canaddmtype = TRUE;
int canaddttype = TRUE;

/* True if game will start up in the middle of a turn. */

int midturnrestore = FALSE;

/* The count of cells that did not have valid terrain data. */

int numbadterrain = 0;

init_constant(key)
int key;
{
    Obj *sym = intern_symbol(keyword_name(key));

    setq(sym, new_number(keyword_value(key)));
    flag_as_constant(sym);
}

init_self_eval(key)
int key;
{
    Obj *sym = intern_symbol(keyword_name(key));

    setq(sym, sym);
    flag_as_constant(sym);
}

init_predefined_symbols()
{
    /* Predefined constants. */
    init_constant(K_FALSE);
    init_constant(K_TRUE);
    init_constant(K_NON_UNIT);
    init_constant(K_NON_MATERIAL);
    init_constant(K_NON_TERRAIN);
    init_constant(K_CELL);
    init_constant(K_BORDER);
    init_constant(K_CONNECTION);
    init_constant(K_COATING);
    init_constant(K_RIVER_X);
    init_constant(K_VALLEY_X);
    init_constant(K_ROAD_X);
    init_constant(K_OVER_NOTHING);
    init_constant(K_OVER_OWN);
    init_constant(K_OVER_BORDER);
    init_constant(K_OVER_ALL);
    /* Random self-evaluating symbols. */
    init_self_eval(K_AND);
    init_self_eval(K_OR);
    init_self_eval(K_NOT);
    init_self_eval(K_REJECT);
    init_self_eval(K_RESET);
    init_self_eval(K_USUAL);
    init_self_eval(K_APPEAR);
    /* Leave these unbound so that first ref computes correct list. */
    intern_symbol(keyword_name(K_USTAR));
    intern_symbol(keyword_name(K_MSTAR));
    intern_symbol(keyword_name(K_TSTAR));
    /* This just needs to be inited somewhere. */
    sidedefaults = lispnil;
}

/* This is the basic interpreter of a form appearing in a module. */

interp_form(module, form)
Module *module;
Obj *form;
{
    Obj *thecar, *rest;
    char *name, *str;
    Module *basemodule;

    if (consp(form) && symbolp(thecar = car(form))) {
	name = c_string(thecar);
	if (Debug) {
	    /* If in a module, report the line number(s) of a form. */
	    if (module != NULL) {
		Dprintf("Line %d", module->startlineno);
		if (module->endlineno != module->startlineno) {
		    Dprintf("-%d", module->endlineno);
		}
	    }
	    Dprintf(": (%s ", name);
	    Dprintlisp(cadr(form));
	    if (cddr(form) != lispnil) {
		Dprintf(" ");
		Dprintlisp(caddr(form));
		if (cdr(cddr(form)) != lispnil) {
		    Dprintf(" ...");
		}
	    }
	    Dprintf(")\n");
	}
	switch (keyword_code(name)) {
	  case K_GAME_MODULE:
	    interp_game_module(form, module);
	    load_base_module(module);
	    break;
#ifndef SPECIAL
	  case K_UNIT_TYPE:
	    interp_utype(form);  break;
	  case K_MATERIAL_TYPE:
	    interp_mtype(form);  break;
	  case K_TERRAIN_TYPE:
	    interp_ttype(form);  break;
	  case K_TABLE:
	    interp_table(form);  break;
          case K_DEFINE:
	    interp_variable(form, TRUE);  break;
	  case K_SET:
	    interp_variable(form, FALSE);  break;
	  case K_UNDEFINE:
	    undefine_variable(form);  break;
	  case K_ADD:
	    add_properties(form);  break;
#endif /* n SPECIAL */
	  case K_WORLD:
	    interp_world(form);  break;
	  case K_AREA:
	    interp_area(form);  break;
	  case K_SIDE:
	    interp_side(form, NULL);  break;
	  case K_SIDE_DEFAULTS:
	    sidedefaults = cdr(form);  break;
	  case K_INDEPENDENT_UNITS:
	    interp_side(form, indepside);  break;
	  case K_PLAYER:
	    interp_player(form);  break;
	  case K_AGREEMENT:
	    interp_agreement(form);  break;
	  case K_SCOREKEEPER:
	    interp_scorekeeper(form);  break;
	  case K_EVT:
	    interp_history(form);  break;
	  case K_BATTLE:
	    init_warning("battle objects not yet supported");  break;
	  case K_UNIT:
	    /* This is for when the unit type name matches a keyword */
		interp_unit(cdr(form));  break;
	  case K_UNIT_DEFAULTS:
	    interp_unit_defaults(cdr(form));  break;
	  case K_NAMER:
	    interp_namer(form);  break;
	  case K_TEXT:
	    interp_text_generator(form);  break;
	  case K_INCLUDE:
	    include_module(form, module);  break;
	  case K_IF:
	    start_conditional(form, module);  break;
	  case K_END_IF:
	    end_conditional(form, module);  break;
	  case K_PRINT:
#ifdef USE_CONSOLE
	    printlisp(cadr(form));
	    if (symbolp(cadr(form))) {
		if (boundp(cadr(form))) {
		    printf(" -> ");
		    printlisp(symbol_value(cadr(form)));
		} else {
		    printf(" <unbound>");
		}
	    }
	    printf("\n");
#else
	    /* should send to the interface to handle */
#endif /* USE_CONSOLE */
	    break;
	  default:
	    if (numutypes == 0) load_default_game();
	    if (utype_from_name(name) != NONUTYPE) {
		interp_unit(form);
	    } else {
		useless_form_warning(module, form);
	    }
	}
    } else {
	useless_form_warning(module, form);
    }
}

useless_form_warning(module, form)
Module *module;
Obj *form;
{
    char buf[BUFSIZE];

    sprintlisp(buf, form);
    if (module) {
	init_warning("%d-%d: A useless form: %s",
		     module->startlineno, module->endlineno, buf);
    } else {
	init_warning("Useless input form: %s", buf);
    }
}

/* Inclusion is half-module-like, not strictly textual. */

include_module(form, module)
Obj *form;
Module *module;
{
    char *name;
    Obj *mname = cadr(form);
    Module *submodule;

    SYNTAX(mname, (symbolp(mname) || stringp(mname)),
	   "Included module name not a string or symbol");
    name = c_string(mname);
    Dprintf("Including \"%s\" ...\n", name);
    submodule = add_game_module(name, module);
    load_game_module(submodule, TRUE);
    if (submodule->loaded) {
    	do_module_variants(submodule, cddr(form));
    } 
    Dprintf("... Done including \"%s\".\n", name);
}

/* Interpret the given list of variants. */

do_module_variants(module, lis)
Module *module;
Obj *lis;
{
    int found;
    char *varname, *vartypename;
    Obj *restv, *var, *vartype, *vardata;
    Obj *restset, *varset, *varsettype, *varsetdata;
    
    for (restset = lis; restset != lispnil; restset = cdr(restset)) {
	varset = car(restset);
	found = FALSE;
	for (restv = module->variants; restv != lispnil; restv = cdr(restv)) {
	    var = car(restv);
	    if (!consp(var)) {
		var = cons(var, lispnil);
	    }
	    if (car(var) == lispnil) {
		var = cdr(var);
	    }
	    if (stringp(car(var))) {
		varname = c_string(car(var));
		var = cdr(var);
	    } else {
		varname = "Variant xxx";
	    }
	    vartype = car(var);
	    vardata = cdr(var);
	    if (equal(varset, vartype)) {
		do_one_variant(module, vartype, vardata, TRUE);
		/* (should define a set_car for this) */
		restv->v.cons.car = cons(lispnil, var);
		found = TRUE;
	    } else if (consp(varset)) {
		varsettype = car(varset);
		varsetdata = cdr(varset);
		if (equal(varsettype, vartype)) {
		    do_one_variant(module, vartype, vardata, varsetdata);
		    restv->v.cons.car = cons(lispnil, var);
		    found = TRUE;
		}
	    }
	}
	if (!found) {
	    init_warning("Mystifying variant");
	}
    }
    /* Now implement all the defaults. */
    for (restv = module->variants; restv != lispnil; restv = cdr(restv)) {
	var = car(restv);
	if (car(var) != lispnil) {
	    if (stringp(car(var))) {
		var = cdr(var);
	    }
	    do_one_variant(module, car(var), cdr(var), cdr(var));
	}
    }
}

do_one_variant(module, vartype, vardata, varsetdata)
Module *module;
Obj *vartype, *vardata, *varsetdata;
{
    int width, height, val, actuallyset = FALSE;
    char *vartypename = c_string(vartype), buf[BUFSIZE];
    Obj *rest;

#if 0
    sprintlisp(buf, varsetdata);
    Dprintf("Module %s variant %s setting to `%s'\n",
	    module_desig(module), vartypename, buf);
#endif
    switch (keyword_code(vartypename)) {
      case K_WORLD_SEEN:
	set_g_terrain_seen(c_number(eval(car(varsetdata))));
	break;
      case K_SEE_ALL:
	set_g_see_all(c_number(eval(car(varsetdata))));
	break;
      case K_WORLD_SIZE:
	if (varsetdata != lispnil) {
	    width = c_number(eval(car(varsetdata)));
	    if (cdr(varsetdata)) {
		height = c_number(eval(cadr(varsetdata)));
	    } else {
		height = width;
	    }
	    actuallyset = TRUE;
	} else {
	}
	if (actuallyset) {
	    set_area_shape(width, height, TRUE);
	}
	break;
      default:
	/* This is the general case. */
	if (consp(vardata)) {
	    if (consp(car(vardata))) {
		/* Apparently no default supplied, assume it's false,
		   that setting is t/f, and just eval the rest of the
		   variant. */
		if (symbolp(car(varsetdata))
		    && c_number(eval(car(varsetdata)))) {
		    for (rest = vardata; rest != lispnil; rest = cdr(rest)) {
			interp_form(NULL, car(rest));
		    }
		}
	    } else {
		/* newer form, should impl */
	    }
	} else {
	    /* syntax error? */
	}
	break;
    }
}

start_conditional(form, module)
Obj *form;
Module *module;
{
    init_warning("Conditionals not supported yet");
}

end_conditional(form, module)
Obj *form;
Module *module;
{
    init_warning("Conditionals not supported yet");
}

/* Digest the form defining the module as a whole. */

interp_game_module(form, module)
Obj *form;
Module *module;
{
    char *name = NULL, *propname, *strval = NULL;
    Obj *props = cdr(form), *bdg, *tmp, *propval;
    Module *submodule;

    if (module == NULL) return;  /* why is this here? */

    if (stringp(car(props))) {
	name = c_string(car(props));
	props = cdr(props);
    }
    if (name != NULL) {
	if (module->name == NULL || strlen(module->name) == 0) {
	    module->name = name;
	} else {
	    if (strcmp(name, module->name) != 0) {
		init_warning(
   "Module's name `%s' does not match declared name `%s', ignoring declared name",
			     module->name, name);
	    }
	}
    }
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	if (stringp(propval)) strval = c_string(propval);
	switch (keyword_code(propname)) {
	  case K_TITLE:
	    module->title = strval;  break;
	  case K_BLURB:
	    module->blurb = strval;  break;
	  case K_PICTURE_NAME:
	    module->picturename = strval;  break;
	  case K_BASE_MODULE:
	    module->basemodulename = strval;  break;
	  case K_DEFAULT_BASE_MODULE:
	    module->defaultbasemodulename = strval;  break;
	  case K_BASE_GAME:
	    module->basegame= strval;  break;
	  case K_INSTRUCTIONS:
	    /* The instructions are a list of strings. */
	    module->instructions = propval;  break;
	  case K_VARIANTS:
	    /* The variants are a list of lists. */
	    module->variants = cdr(bdg);  break;
	  case K_NOTES:
	    /* The player notes are a list of strings. */
	    module->notes = propval;  break;
	  case K_DESIGN_NOTES:
	    /* The design notes are a list of strings. */
	    module->designnotes = propval;  break;
	  case K_VERSION:
	    module->version = strval;  break;
	  case K_PROGRAM_VERSION:
	    module->programversion = strval;  break;
	  default:
	    unknown_property("game module", module->name, propname);
	}
    }
    /* Should be smarter about earlier vs later versions. */
    if (module->programversion != NULL
	&& strcmp(module->programversion, version) != 0) {
	/* This should become some sort of alert on some systems. */
	init_warning(
	      "The module `%s' is claimed to be for Xconq version `%s', but you are actually running version `%s'",
		     module->name, module->programversion, version);
    }
}

/* The following code is unneeded if all the types have been compiled in. */

#ifndef SPECIAL

/* Create a new type of unit and fill in info about it. */

interp_utype(form)
Obj *form;
{
    int u;
    Obj *name = cadr(form), *utype;

    TYPECHECK(symbolp, name, "unit-type name not a symbol");
    if (!canaddutype)
      init_warning("Should not be defining more unit types");
    if (numutypes < MAXUTYPES) {
	u = numutypes++;
	utype = new_utype(u);
	/* Set default values for the unit type's props first. */
	/* Any default type name shouldn't confuse the code below. */
	default_unit_type(u);
	setq(name, utype);
	/* Set the values of random props. */
	fill_in_utype(u, cddr(form));
	/* If no internal type name string given, use the defined name. */
	if (empty_string(u_internal_name(u))) {
	    set_u_internal_name(u, c_string(name));
	}
	if (empty_string(u_type_name(u))) {
	    set_u_type_name(u, u_internal_name(u));
	}
	/* If the official type name is different from the internal name,
	   make it a variable bound to the type. */ 
	if (strcmp(u_type_name(u), u_internal_name(u)) != 0) {
	    setq(intern_symbol(u_type_name(u)), utype);
	}
    } else {
	too_many_types("unit", MAXUTYPES, name);
    }
    /* Blast any cached list of types. */
    makunbound(intern_symbol(keyword_name(K_USTAR)));
    eval_symbol(intern_symbol(keyword_name(K_USTAR)));
}

/* Trudge through assorted properties, filling them in. */

fill_in_utype(u, list)
int u;
Obj *list;
{
    char *propname;
    Obj *bdg, *val;

    for ( ; list != lispnil; list = cdr(list)) {
	bdg = car(list);
	PARSE_PROPERTY(bdg, propname, val);
	set_utype_property(u, propname, val);
    }
}

/* Given a unit type, property name, and a value, find the
   definition of the property and set its value. */

set_utype_property(u, propname, val)
int u;
char *propname;
Obj *val;
{
    int i, found = FALSE;

    for (i = 0; utypedefns[i].name != NULL; ++i) {
	if (strcmp(propname, utypedefns[i].name) == 0) {
	    if (utypedefns[i].intgetter) {
		TYPEPROP(utypes, u, utypedefns, i, short) =
		  c_number(eval(val));
	    } else if (utypedefns[i].strgetter) {
		TYPEPROP(utypes, u, utypedefns, i, char *) =
		  c_string(eval(val));
	    } else {
		TYPEPROP(utypes, u, utypedefns, i, Obj *) = val;
	    }
	    found = TRUE;
	    break;
	}
    }
    if (!found) unknown_property("unit type", u_type_name(u), propname);
    return found;
}

/* Declare a new type of material and fill in info about it. */

interp_mtype(form)
Obj *form;
{
    int m;
    Obj *name = cadr(form), *mtype;
    
    TYPECHECK(symbolp, name, "material-type name not a symbol");
    if (!canaddmtype)
      init_warning("Should not be defining more material types");
    if (nummtypes < MAXMTYPES) {
	m = nummtypes++;
	mtype = new_mtype(m);
	/* Set default values for the material type's properties first. */
	default_material_type(m);
	setq(name, mtype);
	/* Set the values of random props. */
	fill_in_mtype(m, cddr(form));
	/* If no type name string given, use the defined name. */
	if (empty_string(m_type_name(m))) {
	    set_m_type_name(m, c_string(name));
	}
    } else {
	too_many_types("material", MAXMTYPES, name);
    }
    /* Blast and remake any cached list of types. */
    makunbound(intern_symbol(keyword_name(K_MSTAR)));
    eval_symbol(intern_symbol(keyword_name(K_MSTAR)));
}

/* Go through a list of prop name/value pairs and fill in the
   material type description from them. */

fill_in_mtype(m, list)
int m;
Obj *list;
{
    int i, found;
    Obj *bdg, *val;
    char *propname;

    for ( ; list != lispnil; list = cdr(list)) {
	bdg = car(list);
	PARSE_PROPERTY(bdg, propname, val);
	found = FALSE;
	for (i = 0; mtypedefns[i].name != NULL; ++i) {
	    if (strcmp(propname, mtypedefns[i].name) == 0) {
		if (mtypedefns[i].intgetter) {
		    TYPEPROP(mtypes, m, mtypedefns, i, short) =
		      c_number(eval(val));
		} else if (mtypedefns[i].strgetter) {
		    TYPEPROP(mtypes, m, mtypedefns, i, char *) =
		      c_string(eval(val));
		} else {
		    TYPEPROP(mtypes, m, mtypedefns, i, Obj *) = val;
		}
		found = TRUE;
		break;
	    }
	}
	if (!found) unknown_property("material type", m_type_name(m), propname);
    }
}

/* Declare a new type of terrain and fill in info about it. */

interp_ttype(form)
Obj *form;
{
    int t;
    Obj *name = cadr(form), *ttype;

    TYPECHECK(symbolp, name, "terrain-type name not a symbol");
    if (!canaddttype)
      init_warning("Should not be defining more terrain types");
    if (numttypes < MAXTTYPES) {
	t = numttypes++;
	ttype = new_ttype(t);
	/* Set default values for the terrain type's props first. */
	default_terrain_type(t);
	setq(name, ttype);
	/* Set the values of random properties. */
	fill_in_ttype(t, cddr(form));
	/* If no type name string given, use the defined name. */
	if (empty_string(t_type_name(t))) {
	    set_t_type_name(t, c_string(name));
	}
    } else {
	too_many_types("terrain", MAXTTYPES, name);
    }
    /* Blast and remake any cached list of all types. */
    makunbound(intern_symbol(keyword_name(K_TSTAR)));
    eval_symbol(intern_symbol(keyword_name(K_TSTAR)));
}

/* Go through a list of prop name/value pairs and fill in the
   terrain type description from them. */

fill_in_ttype(t, list)
int t;
Obj *list;
{
    int i, found, allfound = FALSE;
    char *propname;
    Obj *bdg, *val;

    for ( ; list != lispnil; list = cdr(list)) {
	bdg = car(list);
	PARSE_PROPERTY(bdg, propname, val);
	found = FALSE;
	for (i = 0; ttypedefns[i].name != NULL; ++i) {
	    if (strcmp(propname, ttypedefns[i].name) == 0) {
		if (ttypedefns[i].intgetter) {
		    TYPEPROP(ttypes, t, ttypedefns, i, short) =
		      c_number(eval(val));
		} else if (ttypedefns[i].strgetter) {
		    TYPEPROP(ttypes, t, ttypedefns, i, char *) =
		      c_string(eval(val));
		} else {
		    TYPEPROP(ttypes, t, ttypedefns, i, Obj *) = val;
		}
		found = TRUE;
		break;
	    }
	}
	if (!found) unknown_property("terrain type", t_type_name(t), propname);
    }
    /* Recalculate the count of subtypes. */
    count_terrain_subtypes();
}

/* Fill in a table. */

interp_table(form)
Obj *form;
{
    int i, found, lim1, lim2, reset = TRUE;
    Obj *formsym = cadr(form), *body = cddr(form);
    char *tablename;

    TYPECHECK(symbolp, formsym, "table name not a symbol");
    tablename = c_string(formsym);
    found = FALSE;
    for (i = 0; tabledefns[i].name != NULL; ++i) {
	if (strcmp(tablename, tabledefns[i].name) == 0) {
	    if (symbolp(car(body))
		&& keyword_code(c_string(car(body))) == K_ADD) {
		body = cdr(body);
		reset = FALSE;
	    }
	    allocate_table(i, reset);
	    add_to_table(formsym, i, body, FALSE);
	    found = TRUE;
	    break;
	}
    }
    if (!found) init_warning( "Undefined table `%s'", tablename);
}

/* Given a table and a list of value-setting clauses, fill in the table. */

#define INDEXP(typ, x) \
  ((typ == UTYP) ? utypep(x) : ((typ == MTYP) ? mtypep(x) : ttypep(x)))

#define nonlist(x) (!consp(x) && x != lispnil)

#define CHECK_INDEX_1(tbl, x)  \
  if (!INDEXP(tabledefns[tbl].index1, (x))) {  \
      init_warning("index 1 to table %s has wrong type",  \
		   tabledefns[tbl].name);  \
      return;  \
  }

#define CHECK_INDEX_2(tbl, x)  \
  if (!INDEXP(tabledefns[tbl].index2, (x))) {  \
      init_warning("index 2 to table %s has wrong type",  \
		   tabledefns[tbl].name);  \
      return;  \
  }

#define CHECK_VALUE(tbl, x)  \
  if (!numberp(x)) {  \
      init_warning("value for table %s is not a number",  \
		   tabledefns[tbl].name);  \
      return;  \
  }

#define CHECK_LISTS(tablename, lis1, lis2)  \
  if (consp(lis2)  \
      && !list_lengths_match(lis1, lis2, "table", tablename))  {  \
      return;  \
  }


add_to_table(tablename, tbl, clauses, init)
int tbl, init;
Obj *tablename, *clauses;
{
    int i, j, num, lim1, lim2;
    Obj *clause, *indexes1, *indexes2, *values;

    lim1 = numtypes_from_index_type(tabledefns[tbl].index1);
    lim2 = numtypes_from_index_type(tabledefns[tbl].index2);
    for ( ; clauses != lispnil; clauses = cdr(clauses)) {
	clause = car(clauses);
	switch (clause->type) {
	  case SYMBOL:
	    clause = eval_symbol(clause);
	    TYPECHECK(numberp, clause, "table clause does not eval to number");
	    /* Now treat it as a number. */
	  case NUMBER:
	    /* a constant value - blast over everything */
	    num = c_number(clause);
	    for (i = 0; i < lim1 * lim2; ++i) {
		(*(tabledefns[tbl].table))[i] = num;
	    }
	    break;
	  case CONS:
	    /* Evaluate the three parts of a clause. */
	    indexes1 = eval(car(clause));
	    indexes2 = eval(cadr(clause));
	    values = eval(caddr(clause));
	    interp_one_clause(tablename, tbl, lim1, lim2,
			      indexes1, indexes2, values);
	    break;
	  case STRING:
	    break; /* error? */
	  default:
	    /* who knows? */
	    break;
	}
    }
}

interp_one_clause(tablename, tbl, lim1, lim2, indexes1, indexes2, values)
Obj *tablename, *indexes1, *indexes2, *values;
int tbl, lim1, lim2;
{
    int i, j, num;
    Obj *tmp1, *tmp2, *value, *subvalue;

    if (nonlist(indexes1)) {
	CHECK_INDEX_1(tbl, indexes1);
	i = c_number(indexes1);
	if (nonlist(indexes2) ) {
	    CHECK_INDEX_2(tbl, indexes2);
	    j = c_number(indexes2);
	    value = values;
	    CHECK_VALUE(tbl, value);
	    num = c_number(value);
	    (*(tabledefns[tbl].table))[lim2 * i + j] = num;
	} else {
	    CHECK_LISTS(tablename, indexes2, values);
	    for (tmp2 = indexes2; tmp2 != lispnil; tmp2 = cdr(tmp2)) {
		CHECK_INDEX_2(tbl, car(tmp2));
		j = c_number(car(tmp2));
		value = (consp(values) ? car(values) : values);
		CHECK_VALUE(tbl, value);
		num = c_number(value);
		(*(tabledefns[tbl].table))[lim2 * i + j] = num;
		if (consp(values)) values = cdr(values);
	    }
	}
    } else {
	CHECK_LISTS(tablename, indexes1, values);
	for (tmp1 = indexes1; tmp1 != lispnil; tmp1 = cdr(tmp1)) {
	    CHECK_INDEX_1(tbl, car(tmp1));
	    i = c_number(car(tmp1));
	    value = (consp(values) ? car(values) : values);
	    if (nonlist(indexes2)) {
		CHECK_INDEX_2(tbl, indexes2);
		j = c_number(indexes2);
		CHECK_VALUE(tbl, value);
		num = c_number(value);
		(*(tabledefns[tbl].table))[lim2 * i + j] = num;
	    } else {
		CHECK_LISTS(tablename, indexes2, value);
		for (tmp2 = indexes2; tmp2 != lispnil; tmp2 = cdr(tmp2)) {
		    CHECK_INDEX_2(tbl, car(tmp2));
		    j = c_number(car(tmp2));
		    subvalue = (consp(value) ? car(value) : value);
		    CHECK_VALUE(tbl, subvalue);
		    num = c_number(subvalue);
		    (*(tabledefns[tbl].table))[lim2 * i + j] = num;
		    if (consp(value)) value = cdr(value);
		}
	    }
	    if (consp(values)) values = cdr(values);
	}
    }
}

/* Set the binding of an existing known variable. */

interp_variable(form, isnew)
Obj *form;
int isnew;
{
    Obj *var = cadr(form);
    Obj *val = eval(caddr(form));
    char *name;

    if (!symbolp(var)) {
	init_warning("Can't set a non-symbol!");
	return;
    }
    name = c_string(var);

  if (isnew) {
    if (boundp(var)) {
	init_warning("Symbol `%s' has been bound already, overwriting", name);
    }
    setq(var, val);
  } else {

#undef  DEF_VAR_I
#define DEF_VAR_I(STR,fname,SETFNAME,doc,var,lo,dflt,hi)  \
    if (strcmp(name, STR) == 0)  \
      { SETFNAME(c_number(val));  return; }
#undef  DEF_VAR_S
#define DEF_VAR_S(STR,fname,SETFNAME,doc,var,dflt)  \
    if (strcmp(name, STR) == 0)  \
      { SETFNAME(c_string(val));  return; }
#undef  DEF_VAR_L
#define DEF_VAR_L(STR,fname,SETFNAME,doc,var,dflt)  \
    if (strcmp(name, STR) == 0)  \
      { SETFNAME(val);  return; }

#include "gvar.def"

    /* Try as a random symbol. */
    if (boundp(var)) {
	setq(var, val);
	return;
    }
    /* Out of luck. */
    init_warning("Can't set unknown global named `%s'", name);
  }
}

undefine_variable(form)
Obj *form;
{
    Obj *var = cadr(form);

    if (!symbolp(var)) {
	init_warning("Can't undefine a non-symbol!");
	return;
    }
    makunbound(var);
}

/* General function to augment types. */

add_properties(form)
Obj *form;
{
   char buf[BUFSIZE];
   Obj *types = eval(cadr(form));
   Obj *prop = caddr(form);
   Obj *values = eval(cadr(cddr(form)));

   if (utypep(types) || (consp(types) && utypep(car(types)))) {
       add_to_utypes(types, prop, values);
   } else if (mtypep(types) || (consp(types) && mtypep(car(types)))) {
       add_to_mtypes(types, prop, values);
   } else if (ttypep(types) || (consp(types) && ttypep(car(types)))) {
       add_to_ttypes(types, prop, values);
   } else {
       sprintlisp(buf, form);
       init_warning("No types to add to in `%s'", buf);
   }
}

/* Compare a list of types with a list of values, complain if
   they don't match up. */

list_lengths_match(types, values, formtype, form)
Obj *types, *values, *form;
char *formtype;
{
    if (length(types) != length(values)) {
	sprintlisp(spbuf, form);
	init_warning("Lists of differing lengths (%d vs %d) in %s `%s'",
		     length(types), length(values), formtype, spbuf);
	return FALSE;
    }
    return TRUE;
}

add_to_utypes(types, prop, values)
Obj *types, *prop, *values;
{
    char *propname = c_string(prop);
    Obj *lis1, *lis2;

    if (utypep(types)) {
	set_utype_property(types->v.num, propname, values);
    } else if (consp(types)) {
	if (consp(values)) {
	    if (!list_lengths_match(types, values, "utype property", prop)) return;
	    for (lis1 = types, lis2 = values;
		 lis1 != lispnil && lis2 != lispnil;
		 lis1 = cdr(lis1), lis2 = cdr(lis2)) {
		TYPECHECK(utypep, car(lis1), "not a unit type");
		if (!set_utype_property(car(lis1)->v.num, propname, car(lis2))) break;
	    }
	} else {
	    for (lis1 = types; lis1 != lispnil; lis1 = cdr(lis1)) {
		TYPECHECK(utypep, car(lis1), "not a unit type");
		if (!set_utype_property(car(lis1)->v.num, propname, values)) break;
	    }
	}
    }
}

add_to_mtypes(types, prop, values)
Obj *types, *prop, *values;
{
    Obj *lis1, *lis2;

    if (mtypep(types)) {
	fill_in_mtype(types->v.num,
		      cons(cons(prop, cons(values, lispnil)), lispnil));
    } else if (consp(types)) {
	if (consp(values)) {
	    if (!list_lengths_match(types, values, "mtype property", prop)) return;
	    for (lis1 = types, lis2 = values;
		 lis1 != lispnil && lis2 != lispnil;
		 lis1 = cdr(lis1), lis2 = cdr(lis2)) {
		TYPECHECK(mtypep, car(lis1), "not a unit type");
		fill_in_mtype(car(lis1)->v.num,
			      cons(cons(prop, cons(car(lis2), lispnil)),
				   lispnil));
	    }
	} else {
	    for (lis1 = types; lis1 != lispnil; lis1 = cdr(lis1)) {
		TYPECHECK(mtypep, car(lis1), "not a unit type");
		fill_in_mtype(car(lis1)->v.num,
			      cons(cons(prop, cons(values, lispnil)),
				   lispnil));
	    }
	}
    }
}

add_to_ttypes(types, prop, values)
Obj *types, *prop, *values;
{
    Obj *lis1, *lis2;

    if (ttypep(types)) {
	fill_in_ttype(types->v.num,
		      cons(cons(prop, cons(values, lispnil)), lispnil));
    } else if (consp(types)) {
	if (consp(values)) {
	    if (!list_lengths_match(types, values, "ttype property", prop)) return;
	    for (lis1 = types, lis2 = values;
		 lis1 != lispnil && lis2 != lispnil;
		 lis1 = cdr(lis1), lis2 = cdr(lis2)) {
		TYPECHECK(ttypep, car(lis1), "not a terrain type");
		fill_in_ttype(car(lis1)->v.num,
			      cons(cons(prop, cons(car(lis2), lispnil)),
				   lispnil));
	    }
	} else {
	    for (lis1 = types; lis1 != lispnil; lis1 = cdr(lis1)) {
		TYPECHECK(ttypep, car(lis1), "not a terrain type");
		fill_in_ttype(car(lis1)->v.num,
			      cons(cons(prop, cons(values, lispnil)),
				   lispnil));
	    }
	}
    }
}

#endif /* n SPECIAL */

/* Interpret a world-specifying form. */

interp_world(form)
Obj *form;
{
    int newworld = FALSE;
    Obj *props, *bdg, *propval;
    char *propname;

    props = cdr(form);
    if (symbolp(car(props))) {
	/* This is the id of the world (eventually). */
	props = cdr(props);
    }
    if (numberp(car(props))) {
    	set_world_circumference(c_number(car(props)), TRUE);
	props = cdr(props);
    }
    for ( ; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	switch (keyword_code(propname)) {
	  case K_CIRCUMFERENCE:
	    world.circumference = c_number(propval);  break;
	  case K_DAY_LENGTH:
	    world.daylength = c_number(propval);  break;
	  case K_YEAR_LENGTH:
	    world.yearlength = c_number(propval);  break;
	  case K_AXIAL_TILT:
	    world.axialtilt = c_number(propval);  break;
	  default:
	    unknown_property("world", "", propname);
	}
    }
}

/* Only one area, of fixed size.  Created anew if shape/size is supplied, else
   just modified. */

interp_area(form)
Obj *form;
{
    int newarea = FALSE, newwidth = 0, newheight = 0;
    Obj *props, *subprop, *bdg, *propval, *rest;
    char *propname;

    props = cdr(form);
    /* (eventually this will be an id or name) */
    if (symbolp(car(props))) {
	props = cdr(props);
	newarea = TRUE;
    }
    if (numberp(car(props))) {
	newwidth = newheight = c_number(car(props));
    	if (area.fullwidth == 0) newarea = TRUE;
	if (area.fullwidth > 0 && area.fullwidth != newwidth)
		init_warning("weird areas - %d vs %d", area.fullwidth, newwidth);
	props = cdr(props);
    }
    if (numberp(car(props))) {
	newheight = c_number(car(props));
    	if (area.fullheight == 0) newarea = TRUE;
	if (area.fullheight > 0 && area.fullheight != newheight)
		init_warning("weird areas - %d vs %d", area.fullheight, newheight);
	props = cdr(props);
    }
    if (consp(car(props))
        && symbolp(car(car(props)))
        && keyword_code(c_string(car(car(props)))) == K_RESTRICT) {
        /* (should check all these values before using) */
        subprop = cdr(car(props));
        area.fullwidth = c_number(car(subprop));
        subprop = cdr(subprop);
        area.fullheight = c_number(car(subprop));
        subprop = cdr(subprop);
        area.fullx = c_number(car(subprop));
        subprop = cdr(subprop);
        area.fully = c_number(car(subprop));
	props = cdr(props);        
    }
    /* If this is setting the area's shape for the first time, actually do it. */
    if (newarea) set_area_shape(newwidth, newheight, TRUE);
    for ( ; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	rest = cdr(bdg);
	switch (keyword_code(propname)) {
	  case K_WIDTH:
	    area.width = c_number(propval);  break;
	  case K_HEIGHT:
	    area.height = c_number(propval);  break;
	  case K_LATITUDE:
	    area.latitude = c_number(propval);  break;
	  case K_LONGITUDE:
	    area.longitude = c_number(propval);  break;
	  case K_CELL_WIDTH:
	    area.cellwidth = c_number(propval);  break;
	  case K_TERRAIN:
	    fill_in_terrain(rest);  break;
	  case K_AUX_TERRAIN:
	    fill_in_aux_terrain(rest);  break;
	  case K_PEOPLE_SIDES:
	    fill_in_people_sides(rest);  break;
	  case K_FEATURES:
	    fill_in_features(rest);  break;
	  case K_ELEVATIONS:
	    fill_in_elevations(rest);  break;
	  case K_MATERIAL:
	    fill_in_cell_material(rest);  break;
	  case K_TEMPERATURES:
	    fill_in_temperatures(rest);  break;
	  case K_WINDS:
	    fill_in_winds(rest);  break;
	  case K_CLOUDS:
	    fill_in_clouds(rest);  break;
	  case K_CLOUD_BOTTOMS:
	    fill_in_cloud_bottoms(rest);  break;
	  case K_CLOUD_HEIGHTS:
	    fill_in_cloud_heights(rest);  break;
	  default:
	    unknown_property("area", "", propname);
	}
    }
    /* This rechecks the width and height if they were set via properties. */
    set_area_shape(area.width, area.height, TRUE);
}

/* The general plan of reading is similar for all layers - create a blank
   layer if none allocated, then call read_layer and pass a function that will
   actually put a value into a cell of the layer.  We need to define those
   functions because most of the setters are macros, and because we can do
   some extra error checking. */

/* Read the area terrain. */

fn_set_terrain_at(x, y, val)
int x, y, val;
{
    /* It's important not to put bad values into the terrain layer. */
    if (!is_terrain_type(val)) {
    	/* Only warn the first few times, just count thereafter. */
    	if (numbadterrain < 10) {
	    init_warning("Unknown terrain type (%d) at %d,%d, substituting %s",
			 val, x, y, t_type_name(0));
	}
	val = 0;
	++numbadterrain;
    }
    set_terrain_at(x, y, val);
}

fill_in_terrain(contents)
Obj *contents;
{
    /* We must have some terrain types or we're screwed. */
    if (numttypes == 0) load_default_game();
    numbadterrain = 0;
    /* Make sure the terrain layer exists. */
    if (!terrain_defined()) allocate_area_terrain();
    read_layer(contents, fn_set_terrain_at);
    if (numbadterrain > 0) {
    	init_warning("%d occurrences of unknown terrain in all.", numbadterrain);
    }
}

/* Read a layer of auxiliary terrain. */

fn_set_aux_terrain_at(x, y, val)
int x, y, val;
{
    /* (should check values here too?) */
    set_aux_terrain_at(x, y, tmpttype, val);
}

fill_in_aux_terrain(contents)
Obj *contents;
{
    int t;
    Obj *typesym = car(contents), *typeval;

    if (symbolp(typesym) && ttypep(typeval = eval(typesym))) {
	t = c_number(typeval);
	contents = cdr(contents);
	/* Make sure aux terrain space exists, but don't overwrite. */
	allocate_area_aux_terrain(t);
	tmpttype = t;
	read_layer(contents, fn_set_aux_terrain_at);
	/* Ensure that borders and connections have all their bits
	   correctly set. */
	patch_linear_terrain(t);
    } else {
	/* not a valid aux terrain type */
    }
}

fn_set_people_side_at(x, y, val)
int x, y, val;
{
    set_people_side_at(x, y, val);
}

fill_in_people_sides(contents)
Obj *contents;
{
    /* Make sure the people sides layer exists. */
    if (!people_sides_defined()) allocate_area_people_sides();
    read_layer(contents, fn_set_people_side_at);
}

/* This should recompute size etc of all these features too. */

fn_set_raw_feature_at(x, y, val)
int x, y, val;
{
    set_raw_feature_at(x, y, val);
}

fill_in_features(contents)
Obj *contents;
{
    int fid, fidnext = 1;
    Obj *featspec, *flist;
    Feature *feat;

    init_features();
    for (flist = car(contents); flist != lispnil; flist = cdr(flist)) {
	featspec = car(flist);
	fid = 0;
	if (numberp(car(featspec))) {
	    fid = c_number(car(featspec));
	    fidnext = max(fid + 1, fidnext);
	    featspec = cdr(featspec);
	}
	feat = create_feature(c_string(car(featspec)),
			      c_string(cadr(featspec)));
	if (fid == 0) fid = fidnext++;
	feat->id = fid;
    }
    read_layer(cdr(contents), fn_set_raw_feature_at);
}

fn_set_elevation_at(x, y, val)
int x, y, val;
{
    set_elev_at(x, y, val);
}

fill_in_elevations(contents)
Obj *contents;
{
    /* Make sure the elevation layer exists. */
    if (!elevations_defined()) allocate_area_elevations();
    read_layer(contents, fn_set_elevation_at);
}

fn_set_material_at(x, y, val)
int x, y, val;
{
    set_material_at(x, y, tmpmtype, val);
}

fill_in_cell_material(contents)
Obj *contents;
{
    int m;
    Obj *typesym = car(contents), *typeval;

    if (symbolp(typesym) && mtypep(typeval = eval(typesym))) {
	m = c_number(typeval);
	contents = cdr(contents);
	/* Make sure this material layer exists. */
	allocate_area_material(m);
	tmpmtype = m;
	read_layer(contents, fn_set_material_at);
    } else {
	/* not a valid material type spec, should warn */
    }
}

fn_set_temperature_at(x, y, val)
int x, y, val;
{
    set_temperature_at(x, y, val);
}

fill_in_temperatures(contents)
Obj *contents;
{
    /* Make sure the temperature layer exists. */
    if (!temperatures_defined()) allocate_area_temperatures();
    read_layer(contents, fn_set_temperature_at);
}

fn_set_raw_wind_at(x, y, val)
int x, y, val;
{
    set_raw_wind_at(x, y, val);
}

fill_in_winds(contents)
Obj *contents;
{
    /* Make sure the winds layer exists. */
    if (!winds_defined()) allocate_area_winds();
    read_layer(contents, fn_set_raw_wind_at);
}

fn_set_raw_cloud_at(x, y, val)
int x, y, val;
{
    set_raw_cloud_at(x, y, val);
}

fill_in_clouds(contents)
Obj *contents;
{
    /* Make sure the winds layer exists. */
    if (!clouds_defined()) allocate_area_clouds();
    read_layer(contents, fn_set_raw_cloud_at);
}

fn_set_raw_cloud_bottom_at(x, y, val)
int x, y, val;
{
    set_raw_cloud_bottom_at(x, y, val);
}

fill_in_cloud_bottoms(contents)
Obj *contents;
{
    /* Make sure the cloud bottoms layer exists. */
    if (!cloud_bottoms_defined()) allocate_area_cloud_bottoms();
    read_layer(contents, fn_set_raw_cloud_bottom_at);
}

fn_set_raw_cloud_height_at(x, y, val)
int x, y, val;
{
    set_raw_cloud_height_at(x, y, val);
}

fill_in_cloud_heights(contents)
Obj *contents;
{
    /* Make sure the cloud heights layer exists. */
    if (!cloud_heights_defined()) allocate_area_cloud_heights();
    read_layer(contents, fn_set_raw_cloud_height_at);
}

/* Interpret a side spec. */

interp_side(form, side)
Obj *form;
Side *side;
{
    int id = -1;
    Obj *ident = lispnil, *props = cdr(form);

    /* See if there's an optional side identifier and pick it off. */
    if (props != lispnil && !consp(car(props))) {
	ident = car(props);
	props = cdr(props);
    }
    if (numberp(ident)) {
	id = c_number(ident);
	side = side_n(id);
    } else {
	/* We want to create a new side. */
    }
    if (side == NULL) {
	side = create_side();
    }
    if (id >= 0) {
    	/* (should worry about id conflicts) */
    	side->id = id;
    }
    /* Apply the current side defaults first. */
    fill_in_side(side, sidedefaults, FALSE);
    /* Now fill in from the explicitly specified properties. */
    fill_in_side(side, props, FALSE);
    Dprintf("  Got side %s\n", side_desig(side));
}

/* Given a side, fill in some of its props. */

fill_in_side(side, props, userdata)
Side *side;
Obj *props;
int userdata;
{
    int s, u, numval = 0;
    char *propname, *strval = NULL;
    Obj *bdg, *tmp, *propval;

    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	if (symbolp(propval)) propval = eval(propval);
	if (numberp(propval)) numval = c_number(propval);
	if (stringp(propval)) strval = c_string(propval);
	/* (should check if user-settable property by looking at keyword flag) */
	switch (keyword_code(propname)) {
	  case K_NAME:
	    check_name_uniqueness(side, strval, "name");
	    side->name = strval;  break;
	  case K_LONG_NAME:
	    check_name_uniqueness(side, strval, "long name");
	    side->longname = strval;  break;
	  case K_SHORT_NAME:
	    check_name_uniqueness(side, strval, "short name");
	    side->shortname = strval;  break;
	  case K_NOUN:
	    check_name_uniqueness(side, strval, "noun");
	    side->noun = strval;  break;
	  case K_PLURAL_NOUN:
	    check_name_uniqueness(side, strval, "plural noun");
	    side->pluralnoun = strval;  break;
	  case K_ADJECTIVE:
	    check_name_uniqueness(side, strval, "adjective");
	    side->adjective = strval;  break;
          /* Several synonyms are allowed for specifying colors. */
	  case K_COLOR:
	  case K_COLORS:
	  case K_COLOR_SCHEME:
	    side->colorscheme = strval;  break;
	  case K_EMBLEM_NAME:
	    side->emblemname = strval;  break;
	  case K_UNIT_NAMERS:
	    /* Allocate space if not already done so. */
	    if (side->unitnamers == NULL) {
		side->unitnamers = (char **) xmalloc(numutypes * sizeof(char *));
	    }
	    merge_unit_namers(side, cdr(bdg));
	    break;
	  case K_FEATURE_NAMERS:
	  	/* (should merge instead of bashing) */
	  	side->featurenamers = cdr(bdg);  break;
	  case K_TASK_LIMIT:
	    side->tasklimit = numval;  break;
	  case K_RESPECT_NEUTRALITY:
	    side->respectneutrality = numval;  break;
	  case K_REAL_TIMEOUT:
	    side->realtimeout = numval;  break;
	  case K_WILLING_TO_DRAW:
	    side->willingtodraw = numval;  break;
	  case K_TRUSTS:
	    s = 0;
	    for (tmp = cdr(bdg); tmp != lispnil; tmp = cdr(tmp)) {
		if (s > g_sides_max()) break;
		side->trusts[s++] = c_number(car(tmp));
	    }
	    break;
	  case K_TRADES:
	    s = 0;
	    for (tmp = cdr(bdg); tmp != lispnil; tmp = cdr(tmp)) {
		if (s > g_sides_max()) break;
		side->trades[s++] = c_number(car(tmp));
	    }
	    break;
	  case K_DOCTRINES:
	    read_utype_doctrine(side, cadr(bdg), cddr(bdg));  break;
	  case K_UI_PREFS:
	    /* should pass to interface routine */
	    break;

	  case K_NAMES_LOCKED:
	    side->nameslocked = numval;  break;
	  case K_CLASS:
/*	    if (userdata) { init_warning("No permission to set property");  break; } */
	    side->sideclass = strval;  break;
	  case K_ACTIVE:
	    side->ingame = numval;  break;
	  case K_STATUS:
	    side->status = numval;  break;
	  case K_PLAYER:
	    side->playerid = numval;  break;
	  case K_CONTROLLED_BY:
	    side->controlledbyid = numval;  break;
	  case K_SELF_UNIT:
	    side->selfunitid = numval;  break;
	  case K_TURN_TIME_USED:
	    side->turntimeused = numval;  break;
	  case K_TOTAL_TIME_USED:
	    side->totaltimeused = numval;  break;
	  case K_TIMEOUTS:
	    side->timeouts = numval;  break;
	  case K_TIMEOUTS_USED:
	    side->timeoutsused = numval;  break;
	  case K_FINISHED_TURN:
	    side->finishedturn = numval;  break;
	  case K_START_WITH:
	    if (side->startwith == NULL) {
		side->startwith = (short *) xmalloc(numutypes * sizeof(short));
	    }
	    u = 0;
	    for (tmp = cdr(bdg); tmp != lispnil; tmp = cdr(tmp)) {
		if (u >= numutypes) break;
		side->startwith[u++] = c_number(car(tmp));
	    }
	    break;
	  case K_NEXT_NUMBERS:
	    if (side->counts == NULL) {
		side->counts = (short *) xmalloc(numutypes * sizeof(short));
	    }
	    u = 0;
	    for (tmp = cdr(bdg); tmp != lispnil; tmp = cdr(tmp)) {
		if (u >= numutypes) break;
		side->counts[u++] = c_number(car(tmp));
	    }
	    break;
	  case K_TECH:
	    if (side->tech == NULL) {
		side->tech = (short *) xmalloc(numutypes * sizeof(short));
	    }
	    u = 0;
	    for (tmp = cdr(bdg); tmp != lispnil; tmp = cdr(tmp)) {
		if (u >= numutypes) break;
		side->tech[u++] = c_number(car(tmp));
	    }
	    break;
	  case K_INIT_TECH:
	    if (side->inittech == NULL) {
		side->inittech = (short *) xmalloc(numutypes * sizeof(short));
	    }
	    u = 0;
	    for (tmp = cdr(bdg); tmp != lispnil; tmp = cdr(tmp)) {
		if (u >= numutypes) break;
		side->inittech[u++] = c_number(car(tmp));
	    }
	    break;
	  case K_SCORES:
	    /* This will be patched up later, after scorekeepers exist. */
	    side->scores = (short *) cdr(bdg);  break;
	  case K_TERRAIN_VIEW:
	    read_terrain_view(side, cdr(bdg));  break;
	  case K_UNIT_VIEW:
	    read_unit_view(side, cdr(bdg));  break;
	  case K_UNIT_VIEW_DATES:
	    read_unit_view_dates(side, cdr(bdg));  break;
	  case K_AI:
	    ai_read_state(side, cdr(bdg));  break;
	  default:
	    unknown_property("side", side_desig(side), propname);
	}
    }
    if (side->noun != NULL && side->pluralnoun == NULL) {
	side->pluralnoun = copy_string(plural_form(side->noun));
    }
}

check_name_uniqueness(side, str, kind)
Side *side;
char *str, *kind;
{
    if (name_in_use(side, str)) {
	init_warning("Side %s `%s' is already in use", kind, str);
    }
}

/* Given a list of (utype str) pairs, set unit namers appropriately. */

merge_unit_namers(side, lis)
Side *side;
Obj *lis;
{
	int u;
	Obj *rest, *elt, *types, *namer;

	for (rest = lis; rest != lispnil; rest = cdr(rest)) {
		elt = car(rest);
		if (consp(elt)) {
			types = eval(car(elt));
			namer = cadr(elt);
			if (utypep(types) && stringp(namer)) {
				u = c_number(types);
				side->unitnamers[u] = c_string(namer);
			} else {
				init_warning("garbled unit namer");
			}
		} else {
			/* (should assign to "next" utype?) */
			init_warning("by-position unit namer not handled");
		}
	}
}

/* Helper function to init side view from rle encoding. */

fn_set_terrain_view(x, y, val)
int x, y, val;
{
    set_terrain_view(tmpside, x, y, val);
}

read_terrain_view(side, contents)
Side *side;
Obj *contents;
{
    init_view(side);
    tmpside = side;
    read_layer(contents, fn_set_terrain_view);
}

fn_set_unit_view(x, y, val)
int x, y, val;
{
    set_unit_view(tmpside, x, y, val);
}

read_unit_view(side, contents)
Side *side;
Obj *contents;
{
    init_view(side);
    tmpside = side;
    read_layer(contents, fn_set_unit_view);
}

fn_set_unit_view_date(x, y, val)
int x, y, val;
{
    set_unit_view_date(tmpside, x, y, val);
}

read_unit_view_dates(side, contents)
Side *side;
Obj *contents;
{
    /* (should ensure presence of view date layer) */
    tmpside = side;
    read_layer(contents, fn_set_unit_view_date);
}

/* Read doctrine info pertaining to a particular unit type. */

read_utype_doctrine(side, ulist, props)
Side *side;
Obj *ulist, *props;
{
    int u = 0;
    char *propname;
    Obj *bdg, *val;

    if (!consp(ulist)) ulist = cons(ulist, lispnil);

    ulist = eval(ulist);
    if (numberp(car(ulist))) u = c_number(car(ulist));

    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, val);
	switch (keyword_code(propname)) {
	  case K_EVER_ASK_SIDE:
	    u_doctrine(side, u, everaskside) = c_number(val);  break;
	  case K_AVOID_BAD_TERRAIN:
	    u_doctrine(side, u, avoidbadterrain) = c_number(val);  break;
#if 0
	  case K_EXPLORE_INPORTANCE:
	    u_doctrine(side, u, exploreimportance) = c_number(val);  break;
#endif
	  default:
	    unknown_property("utype doctrine", "", propname);
	}
    }
}

interp_player(form)
Obj *form;
{
    int id = -1;
    Obj *ident = lispnil, *props = cdr(form);
    Player *player = NULL;

    if (props != lispnil) {
	if (!consp(car(props))) {
	    ident = car(props);
	    props = cdr(props);
	}
    }
    if (numberp(ident)) {
	id = c_number(ident);
	player = find_player(id);
    }
    if (player == NULL) {
	player = add_player();
    }
    if (id > 0) player->id = id;
    fill_in_player(player, props);
    Dprintf("  Got player %s\n", player_desig(player));
}

fill_in_player(player, props)
Player *player;
Obj *props;
{
    char *propname, *strval;
    Obj *bdg, *tmp, *propval;

    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, propval);
	if (stringp(propval)) strval = c_string(propval);
	switch (keyword_code(propname)) {
	  case K_NAME:
	    player->name = strval;  break;
	  case K_CONFIG_NAME:
	    player->configname = strval;  break;
	  case K_DISPLAY_NAME:
	    player->displayname = strval;  break;
	  case K_AI_TYPE_NAME:
	    player->aitypename = strval;  break;
	  case K_INITIAL_ADVANTAGE:
	    player->advantage = c_number(propval);  break;
	  case K_PASSWORD:
	    player->password = strval;  break;
	  default:
	    unknown_property("player", player_desig(player), propname);
	}
    }
    canonicalize_player(player);
}

interp_agreement(form)
Obj *form;
{
    int id;
    char *propname;
    Obj *props = cdr(form), *agid, *bdg, *val;
    Agreement *ag;

    agid = car(props);
    if (numberp(agid)) {
	/* use the number eventually */
	props = cdr(props);
    }
    if (1 /* must create a new agreement object */) {
	ag = create_agreement();
	ag->active = TRUE;  /* default for now */
/*	ag->signers = 0;
	ag->willing = 0;
	ag->knownto = 0; */
    }
    /* Interpret the properties. */
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, val);
	switch (keyword_code(propname)) {
	  case K_TYPE_NAME:
	    ag->typename = c_string(val);  break;
	  case K_NAME:
	    ag->name = c_string(val);  break;
	  case K_ACTIVE:
	    ag->active = c_number(val);  break;
	  case K_TERMS:
	    ag->terms = val;  break;
	  case K_PROPOSER:
	    break;
	  case K_SIGNERS:
	    break;
	  case K_WILLING_TO_SIGN:
	    break;
	  case K_KNOWN_TO:
	    break;
	  case K_ENFORCEMENT:
	    break;
	  default:
	    unknown_property("agreement", "", propname);
	}
    }
}

int uxoffset = 0, uyoffset = 0;

interp_unit_defaults(form)
Obj *form;
{
    int numval = 0, nusn = 0, nuid = 0, nuidorig, nutsn, nut = -1, m;
    int varying;
    Obj *props = form, *bdg, *tmp, *val;
    char *propname;
    Unit *unit;
    extern int nextid;

    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, val);
	if (numberp(val)) numval = c_number(val);
	varying = FALSE;
	switch (keyword_code(propname)) {
	  case K_AT:
	    uxoffset = numval;
	    uyoffset = c_number(caddr(bdg));
	    varying = TRUE;
	    break;
	  default:
	    unknown_property("unit-defaults", "", propname);
	}
	if (!varying && cddr(bdg) != lispnil)
	  init_warning("Extra junk in a %s property, ignoring", propname);
    }
}

ttype_from_name(str)
char *str;
{
    int t;

    for_all_terrain_types(t) {
	if (strcmp(str, t_type_name(t)) == 0)
	  return t;
    }
    return NONTTYPE;
}

/* Try to find a unit type named by the string. */

utype_from_name(str)
char *str;
{
    int u;

    for_all_unit_types(u) {
	if (strcmp(str, u_type_name(u)) == 0
	    || (u_short_name(u) && strcmp(str, u_short_name(u)) == 0)
	    || (u_long_name(u) && strcmp(str, u_long_name(u)) == 0))
	  return u;
    }
    /* Try evaluating the symbol too. */
    if (boundp(intern_symbol(str))
	&& symbol_value(intern_symbol(str))->type == UTYPE) {
	return symbol_value(intern_symbol(str))->v.num;
    }
    return NONUTYPE;
}

/* This creates an individual unit and fills in data about it. */

interp_unit(form)
Obj *form;
{
    int u, numval = 0, nusn = 0, nuid = 0, nuidorig, nutsn, nut = -1, m;
    int varying;
    Obj *head = car(form), *props = cdr(form), *bdg, *tmp, *val;
    char *propname;
    Unit *unit;
    extern int nextid;

    /* We must have some unit types or we're screwed. */
    if (numutypes == 0) load_default_game();
    Dprintf("Reading a unit from ");
    Dprintlisp(form);
    Dprintf("\n");
    if (symbolp(head) && (u = utype_from_name(c_string(head))) != NONUTYPE) {
	unit = create_unit(u, FALSE);
    } else if (stringp(head)) {
    	if ((unit = find_unit_by_name(c_string(head))) == NULL) {
	    init_warning("Couldn't find a unit named \"%s\", ignoring",
			 c_string(head));
	    return;
    	}
    }
    init_supply(unit);  /* doesn't seem right? */
    if (unit == NULL) {
	init_warning("Failed to create a unit!");
    }
    /* Peel off fixed-position properties, if they're supplied. */
    if (numberp(car(props))) {
	unit->prevx = c_number(car(props)) + uxoffset - area.fullx;
	props = cdr(props);
    }
    if (numberp(car(props))) {
	unit->prevy = c_number(car(props)) + uyoffset - area.fully;
	props = cdr(props);
    }
    if (!consp(car(props))) {
	nusn = c_number(eval(car(props)));
	props = cdr(props);
    }
    /* Now crunch through optional stuff.  The unit's properties must *already*
       be correct. */
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, val);
	if (numberp(val)) numval = c_number(val);
	varying = FALSE;
	switch (keyword_code(propname)) {
	  case K_N:
	    unit->name = c_string(val);  break;
	  case K_SHARP:
	    nuid = numval;  break;
	  case K_S:
	    nusn = numval;  break;
	  case K_AT:
	    unit->prevx = numval + uxoffset;
	    unit->prevy = c_number(caddr(bdg)) + uyoffset;
	    varying = TRUE;
	    break;
	  case K_NM:
	    unit->number = numval;  break;
	  case K_CP:
	    unit->cp = numval;  break;
	  case K_HP:
	    unit->hp = numval;  break;
	  case K_CXP:
	    unit->cxp = numval;  break;
	  case K_M:
	    /* Will get weird if nummtypes doesn't match, but won't fail. */
	    tmp = cdr(bdg);
	    for_all_material_types(m) {
		if (tmp != lispnil) {
		    unit->supply[m] = c_number(car(tmp));
		    tmp = cdr(tmp);
		}
	    }
	    varying = TRUE;
	    break;
	  case K_IN:
	    nut = numval;  break;
	  case K_ACT:
	    interp_unit_act(unit, cdr(bdg));  varying = TRUE;  break;
	  case K_PLAN:
	    interp_unit_plan(unit, cdr(bdg));  varying = TRUE;  break;
	  case K_Z:
	    /* (should be prevz?) */
	    unit->z = numval;  break;
	  case K_X:
	    unit->hook = cdr(bdg); varying = TRUE;  break;
	  default:
	    unknown_property("unit", unit_desig(unit), propname);
	}
	if (!varying && cddr(bdg) != lispnil)
	  init_warning("Extra junk in a %s property, ignoring", propname);
    }
    /* If the unit id was given, assign it to the unit, avoiding
       duplication. */
    if (nuid > 0) {
    	nuidorig = nuid;
    	/* If this id is already in use, try the next one. */
    	while (find_unit(nuid) != NULL) ++nuid;
    	/* Guaranteed distinct, safe to use. */
	unit->id = nuid;
	/* (should stash nuidorig in a slot, use to recalc references) */
	/* Ensure that future random ids won't step on this one. */
	nextid = max(nextid, nuid + 1);
    }
    /* Sleazy trick - will fill in actual transport ptr when all units in. */
    unit->transport = (Unit *) nut;
    /* Set the side of this unit if it is not independent. */
    if (nusn > 0) {
	set_unit_side(unit, side_n(nusn));
    }
    Dprintf("  Got %s\n", unit_desig(unit));
}

/* Interpret a unit's action state. */

interp_unit_act(unit, props)
Unit *unit;
Obj *props;
{
    Obj *bdg, *val;
    char *propname;

    if (unit->act == NULL) {
	unit->act = (ActorState *) xmalloc(sizeof(ActorState));
	/* Flag the action as undefined. */
	unit->act->nextaction.type = A_NONE;
    }
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, val);
	switch (keyword_code(propname)) {
	  case K_ACP:
	    unit->act->acp = c_number(val);  break;
	  case K_ACP0:
	    unit->act->initacp = c_number(val);  break;
	  case K_AA:
	    unit->act->actualactions = c_number(val);  break;
	  case K_AM:
	    unit->act->actualmoves = c_number(val);  break;
	  case K_A:
	    /* (should interp a spec for the next action) */
	    break;
	  default:
	    unknown_property("unit actionstate", unit_desig(unit), propname);
	}
    }
}

interp_unit_plan(unit, props)
Unit *unit;
Obj *props;
{
    Obj *bdg, *val, *plantypesym, *trest;
    char *propname;
    Goal *goal;
    Goal *interp_goal();
    Task *task;
    Task *interp_task();

    if (unit->plan == NULL) {
	/* Create the plan explicitly, even if unit type doesn't allow (?). */
	unit->plan = (Plan *) xmalloc(sizeof(Plan));
    }
    plantypesym = car(props);
    SYNTAX(symbolp(plantypesym), props, "plan type must be a symbol");
    unit->plan->type = lookup_plan_type(plantypesym);
    props = cdr(props);
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, val);
	switch (keyword_code(propname)) {
	  case K_ASLEEP:
	    unit->plan->asleep = c_number(val);  break;
	  case K_RESERVE:
	    unit->plan->reserve = c_number(val);  break;
	  case K_WAIT:
	    unit->plan->waitingfortasks = c_number(val);  break;
	  case K_GOAL:
	    goal = interp_goal(cdr(bdg));
	    unit->plan->maingoal = goal;
	    break;
	  case K_TASKS:
	    for (trest = cdr(bdg); trest != lispnil; trest = cdr(trest)) {
	    	task = interp_task(car(trest));
	        /* (should add tasks in reverse order) */
    		task->next = unit->plan->tasks;
    		unit->plan->tasks = task;
	    }
	    break;
	  default:
	    unknown_property("unit plan", unit_desig(unit), propname);
	}
    }
}

lookup_plan_type(sym)
Obj *sym;
{
    int i;
    char *name = c_string(sym);
    extern char *plantypenames[];

    for (i = 0; plantypenames[i] != NULL; ++i) {
	if (strcmp(name, plantypenames[i]) == 0) return i; /* should get real enum */
    }
    return PLAN_NONE;
}

Task *
interp_task(form)
Obj *form;
{
    int tasktype, i;
    Obj *tasktypesym;
    Task *task;

    tasktypesym = car(form);
    SYNTAX(symbolp(tasktypesym), form, "task type must be a symbol");
    tasktype = lookup_task_type(tasktypesym);
    task = create_task(tasktype);
    form = cdr(form);
    task->execnum = c_number(car(form));
    form = cdr(form);
    task->retrynum = c_number(car(form));
    for (i = 0; i < MAXTASKARGS; ++i) {
	form = cdr(form);
	if (form == lispnil) break;
	SYNTAX(symbolp(car(form)), form, "task arg must be a number");
	task->args[i] = c_number(car(form));
    }
    return task;
}

lookup_task_type(sym)
Obj *sym;
{
    int i;
    char *name = c_string(sym);

    for (i = 0; taskdefns[i].name != NULL; ++i) {
	if (strcmp(name, taskdefns[i].name) == 0) return i; /* should get real enum */
    }
    return NO_TASK;
}

Goal *
interp_goal(form)
Obj *form;
{
    return NULL;
}

/* Make a namer from the form. */

interp_namer(form)
Obj *form;
{
    Obj *id = cadr(form), *meth = car(cddr(form));

    if (symbolp(id)) {
	setq(id, make_namer(meth));
    }
}

interp_text_generator(form)
Obj *form;
{
    Obj *id = cadr(form), *meth = car(cddr(form));

    if (symbolp(id)) {
	setq(id, lispnil);
    }
}

/* Make a scorekeeper from the given form. */

interp_scorekeeper(form)
Obj *form;
{
    int id = 0;
    char *propname;
    Obj *props = cdr(form), *bdg, *val;
    Scorekeeper *sk = NULL;

    if (numberp(car(props))) {
	id = c_number(car(props));
	props = cdr(props);
    }
    if (id > 0) {
	/* (should attempt to find scorekeeper) */
    }
    /* Create a new scorekeeper object if necessary. */
    if (sk == NULL) {
	sk = create_scorekeeper();
	if (id > 0) {
	    sk->id = id;
	}
    }
    /* Interpret the properties. */
    for (; props != lispnil; props = cdr(props)) {
	bdg = car(props);
	PARSE_PROPERTY(bdg, propname, val);
	switch (keyword_code(propname)) {
	  case K_TITLE:
	    sk->title = c_string(val);  break;
	  case K_WHEN:
	    sk->when = val;  break;
	  case K_APPLIES_TO:
	    sk->who = val;  break;
	  case K_KNOWN_TO:
	    sk->knownto = val;  break;
	  case K_TRIGGER:
	    sk->trigger = val;  break;
	  case K_DO:
	    sk->body = val;  break;
	  case K_MESSAGES:
	    sk->messages = val;  break;
	  case K_SPECIAL_EFFECTS:
	    sk->specialeffects = val;  break;
	  case K_TRIGGERED:
	    sk->triggered = c_number(val);  break;
	  case K_INITIAL:
	    sk->initial = c_number(val);  break;
	  case K_NOTES:
	    sk->notes = val;  break;
	  default:
	    unknown_property("scorekeeper", "??", propname);
	}
    }
}

/* Make a historical event from the form. */

interp_history(form)
Obj *form;
{
}

/* isn't this redundant? */

r_in_area(x, y)
int x, y;
{
    return (between(0, x, area.width-1) && between(0, y, area.height-1));
}

/* Designer is trying to define too many different types. */

too_many_types(typename, maxnum, name)
char *typename;
int maxnum;
Obj *name;
{
    init_warning("Limited to %d types of %s", maxnum, typename);
    sprintlisp(spbuf, name);
    init_warning("(Failed to create type with name `%s')", spbuf);
}

/* Property name is unknown, either misspelled or misapplied. */

unknown_property(type, inst, name)
char *type, *inst, *name;
{
    init_warning("The %s form %s has no property named %s", type, inst, name);
}

int layerusedefault;
int layerdefault;
int layermultiplier;
int layeradder;
int layerareax, layerareay;
int layerareawidth, layerareaheight;

int ignorespecials;

read_layer(contents, setter)
Obj *contents;
int (*setter)();
{
    int i, t, n, ix, usechartable = FALSE;
    char *str;
    short chartable[256];
    char buf[BUFSIZE];
    Obj *rest, *desc, *rest2, *subdesc, *sym, *num;

    layerusedefault = FALSE;
    layerdefault = 0;
    layermultiplier = 1;
    layeradder = 0;
    layerareax = layerareay = 0;
    layerareawidth = area.width;  layerareaheight = area.height;
    ignorespecials = FALSE;
    for (rest = contents; rest != lispnil; rest = cdr(rest)) {
	desc = car(rest);
	if (stringp(desc)) {
	    /* Read from here to the end of the list, interpreting as contents. */
	    read_rle(rest, setter, (usechartable ? chartable : NULL));
	    return;
	} else if (consp(desc) && symbolp(car(desc))) {
	    switch (keyword_code(c_string(car(desc)))) {
	      case K_CONSTANT:
		/* should set to a constant value taken from cadr */
		init_warning("Constant layers not supported yet");
		return;
	      case K_SUBAREA:
	        /* should apply data to a subarea */
		init_warning("Layer subareas not supported yet");
		break;
	      case K_XFORM:
		layermultiplier = c_number(cadr(desc));
		layeradder = c_number(caddr(desc));
		break;
	      case K_BY_BITS:
		break;
	      case K_BY_CHAR:
		/* Assign each char to its corresponding index. */
		for (i = 0; i < 255; ++i) chartable[i] = 0;
		str = c_string(cadr(desc));
		for (i = 0; i < strlen(str); ++i) {
		    chartable[str[i]] = i;
		    /* If special chars in by-char string, flag it. */
		    if (str[i] == '*' || str[i] == ',') ignorespecials = TRUE;
		}
		usechartable = TRUE;
		break;
	      case K_BY_NAME:
		/* Work through list and match names to numbers. */
		for (i = 0; i < 255; ++i) chartable[i] = 0;
		desc = cdr(desc);
		/* Support optional explicit string a la by-char. */
		if (stringp(car(desc))) {
		    str = c_string(car(desc));
		    for (i = 0; i < strlen(str); ++i) chartable[str[i]] = i;
		    desc = cdr(desc);
		} else {
		    str = NULL;
		}
		i = 0;
		for (rest2 = desc; rest2 != lispnil; rest2 = cdr(rest2)) {
		    subdesc = car(rest2);
		    if (symbolp(subdesc)) {
		    	sym = subdesc;
		    	ix = i++;
		    } else if (consp(subdesc)) {
		    	sym = car(subdesc);
		    	num = cadr(subdesc);
		    	SYNTAX(num, numberp(num), "by-name explicit value is not a number");
		    	ix = c_number(num);
		    } else {
		    	init_warning("garbage by-name subdesc, ignoring");
		    	continue;
		    }
		    /* Eval the symbol into something resembling a value. */
		    sym = eval(sym);
		    SYNTAX(sym, numberishp(sym), "by-name index is not numberish");
		    n = c_number(sym);
		    chartable[(str ? str[ix] : 'a' + ix)] = n;
		}
		usechartable = TRUE;
		break;
	      default:
		sprintlisp(buf, desc);
		init_warning("Ignoring garbage terrain description %s", buf);
	    }
	}
    }
}

/* General RLE reader.  This basically parses the run lengths and calls
   the function that records what was read. */

read_rle(contents, setter, chartable)
Obj *contents;
int (*setter)();
short *chartable;
{
    char ch, *rowstr;
    int i, x, y, run, val, numbadchars = 0;
    int basex = layerareax, basey = layerareay + layerareaheight - 1;
    Obj *rest, *tmp;

    rest = contents;
    y = basey;
    while (rest != lispnil && y >= 0) {
	/* should error check ... */
	rowstr = c_string(car(rest));
	i = 0;
	x = basex;  /* depends on shape of saved data... */
	while ((ch = rowstr[i++]) != '\0' && x < layerareawidth) {
	    if (isdigit(ch)) {
		run = ch - '0';
		/* will foul up if end of string here! */
		/* should check... */
		while ((ch = rowstr[i++]) != 0 && isdigit(ch)) {
		    run = run * 10 + ch - '0';
		}
		/* if we've got garbled data, skip to the next line */
		if (ch == '\0') goto recovery;
	    } else {
		run = 1;
	    }
	    if (ch == '*' && !ignorespecials) {
		/* A large number run separator. */
	    } else if (ch == ',' && !ignorespecials) {
		/* This was a value instead of a run length. */
		val = run;
		run = 1;
	    } else if (chartable != NULL) {
		val = chartable[ch];
	    } else if (between('a', ch, '~')) {
		val = ch - 'a';
	    } else if (between(':', ch, '[')) {
		val = ch - ':' + 30;
	    } else {
	    	/* Warn about strange characters. */
		if (++numbadchars < 5) {
		    init_warning("Bad char '%c' (0x%x) in layer, using NUL instead", ch, ch);
		}
		val = 0;
	    }
	    val = val * layermultiplier + layeradder;
	    while (run-- > 0 && r_in_area(x, y)) { /* i.e. is in array */
		(*setter)(x, y, val);
		++x;
	    }
	}
      recovery:
	/* Fill-in string may be too short, should subst default for rest */
	rest = cdr(rest);
	y--;
    }
    /* Report the sum of garbage, in case there were a great many. */
    if (numbadchars > 0) {
        init_warning("A total of %d bad chars were present", numbadchars);
    }
}

#ifdef DEBUGGING
/* Use this to ensure that everything is cool. */

doublecheck_state(side)
{
    Unit *unit;
    Side *loop_side;

    for_all_units(loop_side, unit) {
	if (unit->x < 0 || unit->x >= area.width ||
	    unit->y <= 0 || unit->y >= (area.height - 1) ||
	    unit->hp <= 0) {
	    Dprintf("%s off map hp %d", unit_desig(unit), unit->hp);
	}
    }
}
#endif /* DEBUGGING */

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

set_u_internal_name(u, s) int u; char *s; { utypes[u].iname = s; }
set_u_type_name(u, s) int u; char *s; { utypes[u].name = s; }
set_m_type_name(m, s) int m; char *s; { mtypes[m].name = s; }
set_t_type_name(t, s) int t; char *s; { ttypes[t].name = s; }

/* If a special symbol, we might not have to fail. */

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

eval_boolean_expression(expr, fn, dflt)
Obj *expr;
int (*fn)(), dflt;
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

coerce_to_side_id(x)
Obj *x;
{
    if (numberp(x)) {
	return c_number(x);
    }
    return 0;
}

Side *coerce_to_side(x)
Obj *x;
{
    return side_n(coerce_to_side_id(x));
}

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
