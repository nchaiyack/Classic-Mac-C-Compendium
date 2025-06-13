/* Xconq game module writing.
   Copyright (C) 1987, 1988, 1989, 1991, 1992, 1993, 1994, 1995
   Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
extern int gamestatesafe;

#define key(x) (keyword_name(x))

extern char *escaped_symbol PROTO ((char *str));
extern char *escaped_string PROTO ((char *str));

static void start_form PROTO ((char *hd));
static void add_to_form PROTO ((char *x));
static void add_num_to_form PROTO ((int x));
static void end_form PROTO ((void));
static void newline_form PROTO ((void));
static void space_form PROTO ((void));
static void write_bool_prop PROTO ((char *name, int value, int dflt, int nodefaulting, int addnewline));
static void write_num_prop PROTO ((char *name, int value, int dflt, int nodefaulting, int addnewline));
static void write_str_prop PROTO ((char *name, char *value, char *dflt, int nodefaulting, int addnewline));
static int string_not_default PROTO ((char *str, char *dflt));
static void write_lisp_prop PROTO ((char *name, struct a_obj *value, struct a_obj *dflt, int nodefaulting, int ascdr, int addnewline));
static void write_utype_value_list PROTO ((char *name, short *arr, int dflt, int addnewline));
static void write_mtype_value_list PROTO ((char *name, short *arr, int dflt, int addnewline));
static void write_side_value_list PROTO ((char *name, short *arr, int dflt, int addnewline));
static void write_utype_string_list PROTO ((char *name, char **arr , char *dflt, int addnewline));
extern char *shortest_escaped_name PROTO ((int u));
static void write_types PROTO ((void));
static void write_tables PROTO ((void));
static void write_type_name_list PROTO ((int typ, int *flags, int dim));
#if 0
static void write_type_value_list PROTO ((int typ, int *flags, int dim, int (*getter)(int, int), int i));
#endif
static void write_table PROTO ((char *name, int (*getter)(int, int), int dflt, int typ1, int typ2));
static void write_world PROTO ((void));
static void write_areas PROTO ((Module *module));
static void write_area_terrain PROTO ((int compress));
static void write_area_aux_terrain PROTO ((int compress));
static void write_area_features PROTO ((int compress));
static void write_area_elevations PROTO ((int compress));
static void write_area_people_sides PROTO ((int compress));
static void write_area_materials PROTO ((int compress));
static void write_area_temperatures PROTO ((int compress));
static void write_area_clouds PROTO ((int compress));
static void write_area_winds PROTO ((int compress));
static void write_globals PROTO ((void));
static void write_scorekeepers PROTO ((void));
static void write_sides PROTO ((Module *module));
static void write_side_properties PROTO ((Side *side));
static int fn_terrain_view PROTO ((int x, int y));
static int fn_unit_view PROTO ((int x, int y));
static int fn_unit_view_date PROTO ((int x, int y));
static void write_side_view PROTO ((Side *side, int compress));
static void write_players PROTO ((void));
static void write_player PROTO ((struct a_player *player));
static void write_units PROTO ((Module *module));
static void write_unit_properties PROTO ((Unit *unit));
static void write_unit_act PROTO ((Unit *unit));
static void write_unit_plan PROTO ((Unit *unit));
static void write_task PROTO ((Task *task));
static void write_goal PROTO ((Goal *goal, int keyword));
static void write_rle PROTO ((int (*datafn)(int, int), int lo, int hi, int (*translator)(int), int compress));
static void write_run PROTO ((int run, int val));
static void write_history PROTO ((void));
static void write_past_unit PROTO ((PastUnit *pastunit));
static void write_historical_event PROTO ((HistEvent *hevt));
static int reshaped_point PROTO ((int x1, int y1, int *x2p, int *y2p));
static int original_point PROTO ((int x1, int y1, int *x2p, int *y2p));

/* The pointer to the file being written to. */

static FILE *fp;

/* True if the area is to be saved to a different size than it is now. */

static int doreshape = FALSE;

static Module *reshaper = NULL;

static char *shortestbuf = NULL;

static char *escapedthingbuf = NULL;

/* Little routines to do low-level syntax.  While these look excessive, calling fprintf
   directly would result in an object code size increase of as much as 25%. */

static void
start_form(hd)
char *hd;
{
    fprintf(fp, "(%s", hd);
}

static void
add_to_form(x)
char *x;
{
    fprintf(fp, " %s", x);
}

static void
add_num_to_form(x)
int x;
{
    fprintf(fp, " %d", x);
}

static void
end_form()
{
    fprintf(fp, ")");
}

static void
newline_form()
{
    fprintf(fp, "\n");
}

static void
space_form()
{
    fprintf(fp, " ");
}

/* These two routines make sure that any symbols and strings can
   be read in again. */

char *
escaped_symbol(str)
char *str;
{
    char *tmp = str;

    if (str[0] == '|' && str[strlen(str)-1] == '|')
      return str;
    while (*tmp != '\0') {
	if (((char *) strchr(" ()#\";|", *tmp)) != NULL || isdigit(str[0])) {
	    sprintf(escapedthingbuf, "|%s|", str);
	    return escapedthingbuf;
	}
	++tmp;
    }
    return str;
}

/* Note that this works correctly on NULL strings, turning them into
   strings of length 0. */

char *
escaped_string(str)
char *str;
{
    char *tmp = str, *rslt = escapedthingbuf;

    *rslt++ = '"';
    if (str != NULL) {
	while (*tmp != 0) {
	    if (*tmp == '"') *rslt++ = '\\';
	    *rslt++ = *tmp++;
	}
    }
    *rslt++ = '"';
    *rslt = '\0';
    return escapedthingbuf;
}

static void
write_bool_prop(name, value, dflt, nodefaulting, addnewline)
char *name;
int value, dflt, nodefaulting, addnewline;
{
    if (nodefaulting || value != dflt) {
	fprintf(fp, " (%s %s)", name, (value ? "true" : "false"));
	if (addnewline) 
	  newline_form();
    }
}

static void
write_num_prop(name, value, dflt, nodefaulting, addnewline)
char *name;
int value, dflt, nodefaulting, addnewline;
{
    if (nodefaulting || value != dflt) {
    	/* Write a normal value or a dice spec, as appropriate. */
	if (value >> 14 == 1) {
	    fprintf(fp, " (%s %dd%d+%d)", name,
		    (value >> 11) & 0x07, (value >> 7) & 0x0f, value & 0x7f);
	} else {
	    fprintf(fp, " (%s %d)", name, value);
	}
	if (addnewline)
	  newline_form();
    }
}

/* Handle the writing of a single string-valued property. */

static void
write_str_prop(name, value, dflt, nodefaulting, addnewline)
char *name, *value, *dflt;
int nodefaulting, addnewline;
{
    if (nodefaulting || string_not_default(value, dflt)) {
	fprintf(fp, " (%s %s)", name, escaped_string(value));
	if (addnewline)
	  newline_form();
    }
}

static int
string_not_default(str, dflt)
char *str, *dflt;
{
    if (empty_string(dflt)) {
	if (empty_string(str)) {
	    return FALSE;
	} else {
	    return TRUE;
	}
    } else {
	if (empty_string(str)) {
	    return TRUE;
	} else {
	    return (strcmp(str, dflt) != 0);
	}
    }
}

static void
write_lisp_prop(name, value, dflt, nodefaulting, ascdr, addnewline)
char *name;
Obj *value, *dflt;
int nodefaulting, ascdr, addnewline;
{
    Obj *rest;

    if (nodefaulting || !equal(value, dflt)) {
	space_form();
	start_form(name);
	if (ascdr && consp(value)) {
	    for (rest = value; rest != lispnil; rest = cdr(rest)) {
	    	space_form();
	    	fprintlisp(fp, car(rest));
	    }
	} else {
	    space_form();
	    fprintlisp(fp, value);
	}
	end_form();
	if (addnewline)
	  newline_form();
    }
}

static void
write_utype_value_list(name, arr, dflt, addnewline)
char *name;
short *arr;
int dflt, addnewline;
{
    int u;

    if (arr == NULL)
      return;
    space_form();
    start_form(name);
    for_all_unit_types(u) {
	fprintf(fp, " %hd", arr[u]);
    }
    end_form();
    if (addnewline)
      newline_form();
}

static void
write_mtype_value_list(name, arr, dflt, addnewline)
char *name;
short *arr;
int dflt, addnewline;
{
    int m;

    if (nummtypes == 0 || arr == NULL)
	  return;
    space_form();
    start_form(name);
    for_all_material_types(m) {
	fprintf(fp, " %hd", arr[m]);
    }
    end_form();
    if (addnewline)
      newline_form();
}

static void
write_side_value_list(name, arr, dflt, addnewline)
char *name;
short *arr;
int dflt, addnewline;
{
    int s;

    if (arr == NULL)
	  return;
    space_form();
    start_form(name);
    for (s = 0; s <= numsides; ++s) {
	fprintf(fp, " %hd", arr[s]);
    }
    end_form();
    if (addnewline)
      newline_form();
}

static void
write_utype_string_list(name, arr, dflt, addnewline)
char *name;
char **arr, *dflt;
int addnewline;
{
    int u;

    if (arr == NULL)
	  return;
    space_form();
    start_form(name);
    for_all_unit_types(u) {
	fprintf(fp, " %s", escaped_string(arr[u]));
    }
    end_form();
    if (addnewline)
      newline_form();
}

/* Return the shortest properly escaped name that can be used to identify
   unit type. */

char *
shortest_escaped_name(u)
int u;
{
    char *typename = u_type_name(u);

    if (shortestbuf == NULL)
      shortestbuf = xmalloc(BUFSIZE);
    if (strlen(u_internal_name(u)) < strlen(typename)) {
	typename = u_internal_name(u);
    }
    sprintf(shortestbuf, "%s", escaped_symbol(typename));
    return shortestbuf;
}

/* A saved game should include everything necessary to recreate a game
   exactly.  It is important that this routine not attempt to use graphics,
   since it may be called when graphics code fails.  Returns TRUE if
   that save was successful. */

int
write_entire_game_state(fname)
char *fname;
{
    Module *module = create_game_module(fname);
    int rslt;

    module->filename = fname;
    module->compresslayers = TRUE;
    module->defall = TRUE;
    rslt = write_game_module(module);
    /* Record that the game's state is accurately saved away. */
    if (rslt)
      gamestatesafe = TRUE;
    /* Note in the history that a copy was made. */
    record_event(H_GAME_SAVED, ALLSIDES);
    return rslt;
}

/* Given a game module telling what is in the module, write out a file
   containing the requested content.  Return true if everything went OK. */

int
write_game_module(module)
Module *module;
{
    if (escapedthingbuf == NULL)
      escapedthingbuf = xmalloc(BUFSIZE);
    if (module->filename == NULL) {
	/* (should be an error?) */
	return FALSE;
    }
    fp = fopen(module->filename, "w");
    if (fp != NULL) {
	/* Write the definition of this game module. */
	start_form(key(K_GAME_MODULE));
	add_to_form(escaped_string(module->name));  newline_form();
	if (module->defall) {
	    write_str_prop(key(K_TITLE), module->title,
			   "", 1, 0);
	    write_str_prop(key(K_VERSION), module->version,
			   "", 1, 0);
	    write_str_prop(key(K_BLURB), module->blurb,
			   "", 1, 0);
	    write_str_prop(key(K_BASE_MODULE), module->basemodulename,
			   "", 1, 0);
	    write_str_prop(key(K_PROGRAM_VERSION), version_string(),
			   "", 1, 0);
	    /* etc */
	}
	newline_form();
	end_form();  newline_form();  newline_form();
	if (module->defall || module->deftypes)
	  write_types();
	if (module->defall || module->deftables)
	  write_tables();
	if (module->defall || module->defglobals)
	  write_globals();
	if (1 /* need to suppress synthesis after reload */) {
	  start_form(key(K_SET));
	  add_to_form("synthesis-methods");
	  add_to_form("nil");
	  end_form();
	  newline_form();
	}
	if (module->defall || module->defscoring)
	  write_scorekeepers();
	doreshape = reshape_the_output(module);
	reshaper = module;
	if (module->defall || module->defworld)
	  write_world();
	if (module->defall || module->defareas)
	  write_areas(module);
	if (module->defall || module->defsides)
	  write_sides(module);
	if (module->defall || module->defplayers)
	  write_players();
	if (module->defall || module->defunits)
	  write_units(module);
	if (module->defall || module->defhistory)
	  write_history();
	/* Write the game notes here (seems reasonable, no deeper reason). */
	if (module->instructions != lispnil) {
		start_form(key(K_GAME_MODULE));  space_form();
	    write_lisp_prop(key(K_INSTRUCTIONS), module->instructions,
			    lispnil, 0, FALSE, 1);
		newline_form();  end_form();  newline_form();  newline_form();
	}
	if (module->notes != lispnil) {
		start_form(key(K_GAME_MODULE));  space_form();
	    write_lisp_prop(key(K_NOTES), module->notes,
			    lispnil, 0, FALSE, 1);
		newline_form();  end_form();  newline_form();  newline_form();
	}
	if (module->designnotes != lispnil) {
		start_form(key(K_GAME_MODULE));  space_form();
	    write_lisp_prop(key(K_DESIGN_NOTES), module->designnotes,
			    lispnil, 0, FALSE, 1);
		newline_form();  end_form();  newline_form();  newline_form();
	}
	fclose(fp);
	return TRUE;
    } else {
	return FALSE;
    }
}

/* Write definitions of all the types. */

static void
write_types()
{
    int u, m, t, i, ival;
    char *typename, *sval;
    Obj *obj;

    /* (or write out all the default values first for doc, then
       only write changed values) */

    for_all_unit_types(u) {
	start_form(key(K_UNIT_TYPE));
	typename = shortest_escaped_name(u);
	add_to_form(typename);
	newline_form();
	for (i = 0; utypedefns[i].name != NULL; ++i) {
	    if (utypedefns[i].intgetter) {
		ival = (*(utypedefns[i].intgetter))(u);
		write_num_prop(utypedefns[i].name, ival,
			       utypedefns[i].dflt, 0, 1);
	    } else if (utypedefns[i].strgetter) {
		sval = (*(utypedefns[i].strgetter))(u);
		/* Special-case a couple possibly-redundant slots. */
		if (utypedefns[i].strgetter == u_type_name
		    && strcmp(typename, sval) == 0)
		  continue;
		if (utypedefns[i].strgetter == u_internal_name
		    && strcmp(typename, sval) == 0)
		  continue;
		write_str_prop(utypedefns[i].name, sval,
			       utypedefns[i].dfltstr, 0, 1);
	    } else {
		obj = (*(utypedefns[i].objgetter))(u);
		write_lisp_prop(utypedefns[i].name, obj,
				lispnil, FALSE, FALSE, TRUE);
	    }
	}
	end_form();  newline_form();
    }
    for_all_material_types(m) {
	start_form(key(K_MATERIAL_TYPE));
	add_to_form(escaped_symbol(m_type_name(m)));  newline_form();
	for (i = 0; mtypedefns[i].name != NULL; ++i) {
	    if (mtypedefns[i].intgetter) {
		ival = (*(mtypedefns[i].intgetter))(m);
		write_num_prop(mtypedefns[i].name, ival,
			       mtypedefns[i].dflt, 0, 1);
	    } else if (mtypedefns[i].strgetter) {
		sval = (*(mtypedefns[i].strgetter))(m);
		/* Special-case a a possibly-redundant slot. */
		if (mtypedefns[i].strgetter == m_type_name
		    && strcmp(typename, sval) == 0)
		  continue;
		write_str_prop(mtypedefns[i].name, sval,
			       mtypedefns[i].dfltstr, 0, 1);
	    } else {
		obj = (*(mtypedefns[i].objgetter))(m);
		write_lisp_prop(mtypedefns[i].name, obj,
				lispnil, FALSE, FALSE, TRUE);
	    }
	}
	end_form();  newline_form();
    }
    for_all_terrain_types(t) {
	start_form(key(K_TERRAIN_TYPE));
	add_to_form(escaped_symbol(t_type_name(t)));  newline_form();
	for (i = 0; ttypedefns[i].name != NULL; ++i) {
	    if (ttypedefns[i].intgetter) {
		ival = (*(ttypedefns[i].intgetter))(t);
		write_num_prop(ttypedefns[i].name, ival,
			       ttypedefns[i].dflt, 0, 1);
	    } else if (ttypedefns[i].strgetter) {
		sval = (*(ttypedefns[i].strgetter))(t);
		/* Special-case a a possibly-redundant slot. */
		if (ttypedefns[i].strgetter == t_type_name
		    && strcmp(typename, sval) == 0)
		  continue;
		write_str_prop(ttypedefns[i].name, sval,
			       ttypedefns[i].dfltstr, 0, 1);
	    } else {
		obj = (*(ttypedefns[i].objgetter))(t);
		write_lisp_prop(ttypedefns[i].name, obj,
				lispnil, FALSE, FALSE, TRUE);
	    }
	}
	end_form();  newline_form();
    }
    newline_form();
}

/* Write definitions of all the tables. */

static void
write_tables()
{
    int tbl;

    newline_form();
    for (tbl = 0; tabledefns[tbl].name != 0; ++tbl) {
	if (*(tabledefns[tbl].table) != NULL) {
	    write_table(tabledefns[tbl].name,
			tabledefns[tbl].getter, tabledefns[tbl].dflt,
			tabledefns[tbl].index1, tabledefns[tbl].index2);
	}
    }
}

#define star_from_typ(typ)  \
  ((typ) == UTYP ? "u*" : ((typ) == MTYP ? "m*" :"t*"))

#define name_from_typ(typ, i)  \
  ((typ) == UTYP ? shortest_escaped_name(i) : ((typ) == MTYP ? m_type_name(i) : t_type_name(i)))

static void
write_type_name_list(typ, flags, dim)
int typ, *flags, dim;
{
    int j, first = TRUE, listlen = 0;

    if (flags == NULL)
      return;
    for (j = 0; j < dim; ++j)
      if (flags[j])
        ++listlen;
    if (listlen > 1)
      fprintf(fp, "(");
    for (j = 0; j < dim; ++j) {
	if (flags[j]) {
	    if (first)
	      first = FALSE;
	    else
	      fprintf(fp, " ");
	    fprintf(fp, "%s", escaped_symbol(name_from_typ(typ, j)));
	}
    }
    if (listlen > 1)
      end_form();
}

#if 0
/* Write out a list of values in a table. */

static void
write_type_value_list(typ, flags, dim, getter, i)
int typ, *flags, dim, (*getter) PROTO ((int, int)), i;
{
    int j, first = TRUE, listlen = 0;

    for (j = 0; j < dim; ++j)
      if (flags == NULL || flags[j])
        ++listlen;
    if (listlen > 1)
      fprintf(fp, "(");
    for (j = 0; j < dim; ++j) {
	if (flags == NULL || flags[j]) {
	    if (first)
	      first = FALSE;
	    else
	      fprintf(fp, " ");
	    fprintf(fp, "%d", (*getter)(i, j));
	}
    }
    if (listlen > 1)
      end_form();
}
#endif

/* A simple histogram struct - count and value, that's all. */

struct histo { int count, val; };

/* Sort into *descending* order by count. */

static int
histo_compare(x, y)
CONST void *x, *y;
{
    return ((struct histo *) y)->count - ((struct histo *) x)->count;
}

/* Write out a single table.  Only write it if it contains non-default
   values, and try to find runs of constant value, since tables can be
   really large, but often have constant areas within them. */

static void
write_table(name, getter, dflt, typ1, typ2)
char *name;
int (*getter) PROTO ((int, int)), dflt, typ1, typ2;
{
    int i, j, k, colvalue, constcol, next;
    int numrandoms, nextrowdiffers, writeconst;
    int sawfirst, constrands, constval;
    int dim1 = numtypes_from_index_type(typ1);
    int dim2 = numtypes_from_index_type(typ2);
    struct histo mostcommon[200]; /* needs to be more than MAXxTYPES */
    int indexes1[200], randoms[200];
    int firstclause = TRUE;

    start_form(key(K_TABLE));
    add_to_form(name);
    fprintf(fp, "  ; %d\n", dflt);
    if (dim1 <= dim2) {
        /* Analyze the table by rows. */
	for (k = 0; k < dim1; ++k)
	  indexes1[k] = FALSE;
	for (i = 0; i < dim1; ++i) {
	    /* First see if this row has all the same values as the next. */
	    indexes1[i] = TRUE;
	    nextrowdiffers = FALSE;
	    if (i < dim1 - 1) {
	    	for (j = 0; j < dim2; ++j) {
	    	    if ((*getter)(i, j) != (*getter)(i + 1, j)) {
	    	    	nextrowdiffers = TRUE;
	    	    	break;
	    	    }
	    	}
	    } else {
	    	/* The last row is *always* "different". */
	    	nextrowdiffers = TRUE;
	    }
	    /* (should look at *all* rows to find matching rows before
	       dumping one) */
	    if (nextrowdiffers) {
		/* Make a histogram of all the values in this row. */
		mostcommon[0].count = 1;
		mostcommon[0].val = (*getter)(i, 0);
		next = 1;
		for (j = 0; j < dim2; ++j) {
		    for (k = 0; k < next; ++k) {
			if (mostcommon[k].val == (*getter)(i, j)) {
			    ++(mostcommon[k].count);
			    break;
			}
		    }
		    if (k == next) {
			mostcommon[next].count = 1;
			mostcommon[next].val = (*getter)(i, j);
			++next;
		    }
		}
		if (next == 1 && mostcommon[0].val == dflt) {
		    /* Entire row(s) is/are just the default table value. */
		} else {
		    writeconst = FALSE;
		    numrandoms = 0;
		    if (next == 1) {
			/* Only one value in the row(s). */
			writeconst = TRUE;
		    } else {
			qsort(mostcommon, next, sizeof(struct histo),
			      histo_compare);
			if (mostcommon[0].count >= (3 * dim2) / 4) {
			    /* The most common value in this row(s) is not the only value,
			       but it is worth writing into a separate clause. */
			    writeconst = TRUE;
			    for (j = 0; j < dim2; ++j) {
				/* Flag the other values as needing to be
				   written separately. */
				randoms[j] =
				  (mostcommon[0].val != (*getter)(i, j));
				if (randoms[j])
				  ++numrandoms;
			    }
			} else {
			    /* Flag all in the row as randoms. */
			    for (j = 0; j < dim2; ++j) {
				randoms[j] = TRUE;
				++numrandoms;
			    }
			}
		    }
		    /* Write out the most common value (if non-default) in the row(s),
		       expressing it with a clause that applies the value
		       to the entire row(s). */
		    if (writeconst && mostcommon[0].val != dflt) {
			if (firstclause)
			  firstclause = FALSE;
			else
			  newline_form();
			fprintf(fp, "  (");
			write_type_name_list(typ1, indexes1, dim1);
			fprintf(fp, " %s %d", star_from_typ(typ2), mostcommon[0].val);
			end_form();
		    }
		    /* Now override the most common value with any
		       exceptions. */
		    if (numrandoms > 0) {
			constrands = TRUE;
			sawfirst = FALSE;
			for (j = 0; j < dim2; ++j) {
			    if (randoms[j]) {
			        if (!sawfirst) {
				    constval = (*getter)(i, j);
				    sawfirst = TRUE;
			        }
			        if (sawfirst && constval != (*getter)(i, j)) {
				    constrands = FALSE;
				    break;
			        }
			    }
			}
			if (constrands) {
			    if (firstclause)
			      firstclause = FALSE;
			    else
			      newline_form();
			    fprintf(fp, "  (");
			    write_type_name_list(typ1, indexes1, dim1);
			    fprintf(fp, " ");
			    write_type_name_list(typ2, randoms, dim2);
			    add_num_to_form(constval);
			    end_form();
			} else {
			    /* We have a group of rows with varying data
			       in the columns; write a separate row. */
			    for (j = 0; j < dim2; ++j) {
				if (randoms[j]) {
				    if (firstclause)
				      firstclause = FALSE;
				    else
				      newline_form();
				    fprintf(fp, "  (");
				    write_type_name_list(typ1, indexes1, dim1);
				    fprintf(fp, " %s %d",
				    	    escaped_symbol(name_from_typ(typ2, j)),
				    	    (*getter)(i, j));
				    fprintf(fp, ")");
				}
			    }
			}
		    }
		}
		/* Reset the row flags in preparation for the next group
		   of rows whose contents match each other. */
		for (k = 0; k < dim1; ++k)
		  indexes1[k] = FALSE;
	    }
	}
    } else {
        /* Analyze the table by columns. */
        /* Don't work as hard to optimize; this case should be uncommon,
	   since there are usually more types of units than
	   materials or terrain. */
	for (j = 0; j < dim2; ++j) {
	    constcol = TRUE;
	    colvalue = (*getter)(0, j);
	    for (i = 0; i < dim1; ++i) {
		if ((*getter)(i, j) != colvalue) {
		    constcol = FALSE;
		    break;
		}
	    }
	    if (!constcol || colvalue != dflt) {
		if (firstclause)
		  firstclause = FALSE;
		else
		  newline_form();
		fprintf(fp, "  (%s %s",
			star_from_typ(typ1),
			escaped_symbol(name_from_typ(typ2, j)));
		/* Write out either a single constant value or a list of
		   varying values, as appropriate. */
		if (constcol) {
		    add_num_to_form(colvalue);
		} else {
		    fprintf(fp, " (");
		    for (i = 0; i < dim1; ++i) {
			add_num_to_form((*getter)(i, j));
		    }
		    end_form();
		}
		end_form();
	    }
	}
    }
    end_form();  newline_form();
}

/* Write info about the whole world. */

static void
write_world()
{
    newline_form();
    start_form(key(K_WORLD));
    add_num_to_form((doreshape ? reshaper->finalcircumference : world.circumference));
    write_num_prop(key(K_DAY_LENGTH), world.daylength, 1, 0, 1);
    write_num_prop(key(K_YEAR_LENGTH), world.yearlength, 1, 0, 1);
    end_form();  newline_form();
}

/* Write info about the area in the world.  This code uses run-length encoding
   to reduce the size of each written layer as much as possible.  Note
   also that each layer is written as a separate form, so that the Lisp
   reader doesn't have to read really large forms back in. */

static void
write_areas(module)
Module *module;
{
    int all = module->defall, compress = module->compresslayers;

    newline_form();
    /* Write the basic dimensions. */
    start_form(key(K_AREA));
    add_num_to_form((doreshape ? reshaper->finalwidth : area.width));
    add_num_to_form((doreshape ? reshaper->finalheight : area.height));
    /* Write all the scalar properties. */
    write_num_prop(key(K_LATITUDE), area.latitude, 0, 0, 0);
    write_num_prop(key(K_LONGITUDE), area.longitude, 0, 0, 0);
    write_num_prop(key(K_CELL_WIDTH), area.cellwidth, 0, 0, 0);
    end_form();
    newline_form();
    /* Write the area's layers, each as a separate form. */
    if (all || module->defareaterrain)
	  write_area_terrain(compress);
    if (all || module->defareaterrain)
	  write_area_aux_terrain(compress);
    if (all || module->defareamisc)
	  write_area_features(compress);
    if (all || module->defareamisc)
	  write_area_elevations(compress);
    if (all || module->defareamisc)
	  write_area_people_sides(compress);
    if (all || module->defareamaterial)
	  write_area_materials(compress);
    if (all || module->defareaweather)
	  write_area_temperatures(compress);
    if (all || module->defareaweather)
	  write_area_clouds(compress);
    if (all || module->defareaweather)
	  write_area_winds(compress);
}

static void
write_area_terrain(compress)
int compress;
{
    int t;

    start_form(key(K_AREA));  space_form();
    start_form(key(K_TERRAIN));  newline_form();
    fprintf(fp, "  ");
    start_form(key(K_BY_NAME));
    for_all_terrain_types(t) {
	/* Break the list into groups of 5 per line. */
    	if (t % 5 == 0) {
	    newline_form(); fprintf(fp, "   ");
	}
    	fprintf(fp, " (%s %d)", escaped_symbol(t_type_name(t)), t);
    }
    end_form();  newline_form();
    write_rle(fn_terrain_at, 0, numttypes-1, NULL, compress);
    end_form();  end_form();  newline_form();
}

static void
write_area_aux_terrain(compress)
int compress;
{
    int t;

    for_all_terrain_types(t) {
	if (aux_terrain_defined(t)) {
	    start_form(key(K_AREA));  space_form();
	    start_form(key(K_AUX_TERRAIN));
	    add_to_form(escaped_symbol(t_type_name(t)));
	    newline_form();
	    tmpttype = t;
	    write_rle(fn_aux_terrain_at, 0, 127, NULL, compress);
	    end_form();  end_form();  newline_form();
	}
    }
}

static void
write_area_features(compress)
int compress;
{
    Feature *feature;
    extern Feature *featurelist;

    if (featurelist == NULL || !features_defined()) return;
    renumber_features();
    start_form(key(K_AREA));  space_form();
    start_form(key(K_FEATURES));
    fprintf(fp, " (\n");
    /* Dump out the list of features first. */
    for (feature = featurelist; feature != NULL; feature = feature->next) {
	fprintf(fp, "   (%d %s",
		feature->id, escaped_string(feature->typename));
	fprintf(fp, " %s)\n",
		escaped_string(feature->name));
    }
    fprintf(fp, "  )\n");
    /* Now record which features go with which cells. */
    write_rle(fn_feature_at, 0, -1, NULL, compress);
    fprintf(fp, "))\n");
}

static void
write_area_elevations(compress)
int compress;
{
    if (elevations_defined()) {
	start_form(key(K_AREA));  space_form();
	start_form(key(K_ELEVATIONS));
	newline_form();
	write_rle(fn_elevation_at, minelev, maxelev, NULL, compress);
	end_form();  end_form();  newline_form();
    }
}

static void
write_area_people_sides(compress)
int compress;
{
    if (people_sides_defined()) {
	start_form(key(K_AREA));  space_form();
	start_form(key(K_PEOPLE_SIDES));
	newline_form();
	write_rle(fn_people_side_at, 0, MAXSIDES+1, NULL, compress);
	end_form();  end_form();  newline_form();
    }
}

static void
write_area_materials(compress)
int compress;
{
    int m;

    if (any_cell_materials_defined()) {
	for_all_material_types(m) {
	    if (cell_material_defined(m)) {
		start_form(key(K_AREA));  space_form();
		start_form(key(K_MATERIAL));
		add_to_form(escaped_symbol(m_type_name(m)));
		newline_form();
		tmpmtype = m;
		write_rle(fn_material_at, 0, 127, NULL, compress);
		end_form(); end_form(); newline_form();
	    }
	}
    }
}

static void
write_area_temperatures(compress)
int compress;
{
    if (temperatures_defined()) {
	start_form(key(K_AREA));  space_form();
	start_form(key(K_TEMPERATURES));
	newline_form();
	write_rle(fn_temperature_at, 0, 9999, NULL, compress);
	end_form(); end_form(); newline_form();
    }
}

static void
write_area_clouds(compress)
int compress;
{
    if (clouds_defined()) {
	start_form(key(K_AREA));  space_form();
	start_form(key(K_CLOUDS));
	newline_form();
	write_rle(fn_raw_cloud_at, 0, 127, NULL, compress);
	end_form(); end_form(); newline_form();
    }
    if (cloud_bottoms_defined()) {
	start_form(key(K_AREA));  space_form();
	start_form(key(K_CLOUD_BOTTOMS));
	newline_form();
	write_rle(fn_raw_cloud_bottom_at, 0, 9999, NULL, compress);
	end_form(); end_form(); newline_form();
    }
    if (cloud_heights_defined()) {
	start_form(key(K_AREA));  space_form();
	start_form(key(K_CLOUD_HEIGHTS));
	newline_form();
	write_rle(fn_raw_cloud_height_at, 0, 9999, NULL, compress);
	end_form(); end_form(); newline_form();
    }
}

static void
write_area_winds(compress)
int compress;
{
    if (winds_defined()) {
	start_form(key(K_AREA));  space_form();
	start_form(key(K_WINDS));
	newline_form();
	write_rle(fn_raw_wind_at, 0, 127, NULL, compress);
	end_form(); end_form(); newline_form();
    }
}

/* Write the globals.  The "complete" flag forces all values out, even
   if they match the compiled-in defaults. */

/* These decls are needed because the functions are mentioned in gvar.def. */

static void
write_globals()
{
    int complete = FALSE;
    time_t now;

    /* Snapshot realtime values. */
    time(&now);
    set_g_elapsed_time(idifftime(now, game_start_in_real_time));

    newline_form();

#undef  DEF_VAR_I
#define DEF_VAR_I(STR,FNAME,setfname,doc,var,lo,DFLT,hi)  \
    if (complete || FNAME() != DFLT)  { \
      start_form(key(K_SET));  \
      fprintf(fp, " %s %d)\n", STR, FNAME());  \
    }
#undef  DEF_VAR_S
#define DEF_VAR_S(STR,FNAME,setfname,doc,var,DFLT)  \
    if (complete || string_not_default(FNAME(), DFLT))  { \
      start_form(key(K_SET));  \
      fprintf(fp, " %s %s)\n", STR, escaped_string(FNAME()));  \
    }
#undef  DEF_VAR_L
#define DEF_VAR_L(STR,FNAME,setfname,doc,var,DFLT)  \
    if (complete || ((DFLT) == NULL && FNAME() != lispnil))  {  \
      if (FNAME() != NULL) {  \
        start_form(key(K_SET));  \
        fprintf(fp, " %s ", STR);  \
        fprintlisp(fp, FNAME());  \
        fprintf(fp, ")\n");  \
      } else {  \
        fprintf(fp, "; %s was unbound?\n", STR);  \
      }  \
    }

#include "gvar.def"

}

/* Write all the scorekeepers. */

static void
write_scorekeepers()
{
    Scorekeeper *sk;

    for_all_scorekeepers(sk) {
	start_form(key(K_SCOREKEEPER));  add_num_to_form(sk->id);
	write_str_prop(key(K_TITLE), sk->title, "", 0, 1);
	write_lisp_prop(key(K_WHEN), sk->when, lispnil, 0, FALSE, 1);
	write_lisp_prop(key(K_APPLIES_TO), sk->who, lispnil, 0, FALSE, 1);
	write_lisp_prop(key(K_KNOWN_TO), sk->who, lispnil, 0, FALSE, 1);
	write_lisp_prop(key(K_TRIGGER), sk->trigger, lispnil, 0, FALSE, 1);
	write_lisp_prop(key(K_DO), sk->body, lispnil, 0, FALSE, 1);
	write_num_prop(key(K_TRIGGERED), sk->triggered, 0, 0, 1); 
	write_num_prop(key(K_INITIAL), sk->initial, 0, 0, 1); 
	write_lisp_prop(key(K_NOTES), sk->notes, lispnil, 0, FALSE, 1);
	end_form();  newline_form();
    }
}

/* Write declarations of all the sides. */

static void
write_sides(module)
Module *module;
{
    Side *side;

    fprintf(fp, "\n; %d sides\n", numsides);
    Dprintf("Will try to write %d sides ...\n", numsides);
    for_all_sides(side) {
	start_form(key(K_SIDE));  add_num_to_form(side->id);
	write_side_properties(side);
	end_form();  newline_form();
	if (module->defall || module->defsideviews)
	  write_side_view(side, module->compresslayers);
	Dprintf("  Wrote side %s\n", side_desig(side));
    }
    Dprintf("... Done writing sides\n");
}

/* Write random properties of a side. */

static void
write_side_properties(side)
Side *side;
{
    int i;

    write_str_prop(key(K_NAME), side->name, "", 0, 1);
    write_str_prop(key(K_LONG_NAME), side->longname, "", 0, 1);
    write_str_prop(key(K_SHORT_NAME), side->shortname, "", 0, 1);
    write_str_prop(key(K_NOUN), side->noun, "", 0, 1);
    write_str_prop(key(K_PLURAL_NOUN), side->pluralnoun, "", 0, 1);
    write_str_prop(key(K_ADJECTIVE), side->adjective, "", 0, 1);
    write_str_prop(key(K_COLOR), side->colorscheme, "", 0, 1);
    write_str_prop(key(K_EMBLEM_NAME), side->emblemname, "", 0, 1);
    write_str_prop(key(K_CLASS), side->sideclass, "", 0, 1);
    write_utype_string_list(key(K_UNIT_NAMERS), side->unitnamers, "", 1);
    write_bool_prop(key(K_ACTIVE), side->ingame, FALSE, 0, 1);
    write_num_prop(key(K_PRIORITY), side->priority, 0, 0, 1);
    write_num_prop(key(K_STATUS), side->status, 0, 0, 1);
    write_num_prop(key(K_TURN_TIME_USED), side->turntimeused, 0, 0, 1);
    write_num_prop(key(K_TOTAL_TIME_USED), side->totaltimeused, 0, 0, 1);
    write_num_prop(key(K_FINISHED_TURN), side->finishedturn, 0, 0, 1);
    write_num_prop(key(K_CONTROLLED_BY), side_number(side->controlledby), 0, 0, 1);
    write_side_value_list(key(K_TRUSTS), side->trusts, 0, 1);
    write_side_value_list(key(K_TRADES), side->trades, 0, 1);
    write_utype_value_list(key(K_START_WITH), side->startwith, 0, 1);
    write_utype_value_list(key(K_NEXT_NUMBERS), side->counts, 0, 1);
    write_utype_value_list(key(K_TECH), side->tech, 0, 1);
    write_utype_value_list(key(K_INIT_TECH), side->inittech, 0, 1);
    /* (should do this with a generic array-writing routine) */
    if (side->scores) {
	newline_form();
	start_form(key(K_SCORES));
	for (i = 0; i < numscores; ++i) {
	    add_num_to_form(side->scores[i]);
	}
	end_form();
	newline_form();
    }
    if (side_has_ai(side)) {
	newline_form();
	start_form(key(K_AI_DATA));
	ai_write_state(fp, side);
	end_form();
	newline_form();
    }
}

/* Write about what has been seen in the area. */

/* (should have option to spec symbolic dict of sides and units) */

static int
fn_terrain_view(x, y)
int x, y;
{
    return terrain_view(tmpside, x, y);
}

static int
fn_unit_view(x, y)
int x, y;
{
    return unit_view(tmpside, x, y);
}

static int
fn_unit_view_date(x, y)
int x, y;
{
    return unit_view_date(tmpside, x, y);
}

static void
write_side_view(side, compress)
Side *side;
int compress;
{
    /* View layers are not defined if see-all is in effect. */
    if (g_see_all())
      return;
    tmpside = side;
    newline_form();
    start_form(key(K_SIDE));  add_num_to_form(side->id);  space_form();
    start_form(key(K_TERRAIN_VIEW));  newline_form();
    write_rle(fn_terrain_view, 0, 10000, NULL, compress);
    end_form();  end_form();  newline_form();
    start_form(key(K_SIDE));  add_num_to_form(side->id);  space_form();
    start_form(key(K_UNIT_VIEW));  newline_form();
    write_rle(fn_unit_view, 0, 10000, NULL, compress);
    end_form();  end_form();  newline_form();
    start_form(key(K_SIDE));  add_num_to_form(side->id);  space_form();
    start_form(key(K_UNIT_VIEW_DATES));  newline_form();
    write_rle(fn_unit_view_date, 0, 10000, NULL, compress);
    end_form();  end_form();  newline_form();
}

static void
write_players()
{
    Side *side;

    fprintf(fp, "; %d players\n", numplayers);
    Dprintf("Will try to write %d players ...\n", numplayers);
    for_all_sides(side) {
	if (side->player != NULL) {
	    write_player(side->player);
	    Dprintf("Wrote player %s,\n", player_desig(side->player));
	}
    }
    Dprintf("... Done writing players.\n");
}

static void
write_player(player)
Player *player;
{
    start_form(key(K_PLAYER));  add_num_to_form(0);  newline_form();
    write_str_prop(key(K_NAME), player->name, "", 0, 1);
    write_str_prop(key(K_CONFIG_NAME), player->configname, "", 0, 1);
    write_str_prop(key(K_DISPLAY_NAME), player->displayname, "", 0, 1);
    write_str_prop(key(K_AI_TYPE_NAME), player->aitypename, "", 0, 1);
    end_form();  newline_form();
}

/* Should write out "unit groups" with dict prepended, then can use with
   multiple games */

/* Write the unit section of a game module. */

static void
write_units(module)
Module *module;
{
    int x, y;
    Unit *unit;
    Side *loopside;

    /* Make sure no dead units get saved. */
    flush_dead_units();
    /* Make a consistent ordering. */
    sort_units();
    fprintf(fp, "; %d units\n", numunits);
    /* Need to write out the defaults being assumed subsequently. */
    /* maybe use those in postprocessing. */
    fprintf(fp, "(unit-defaults)\n");
    Dprintf("Writing %d units ...\n", numunits);
    for_all_sides_plus_indep(loopside) {
	for_all_side_units(loopside, unit) {
	    if (alive(unit)) {
		if (doreshape) {
		    reshaped_point(unit->x, unit->y, &x, &y);
		} else {
		    x = unit->x;  y = unit->y;
		}
		start_form(shortest_escaped_name(unit->type));
		fprintf(fp, " %d %d %d", x, y, side_number(unit->side));
		write_num_prop(key(K_Z), unit->z, 0, 0, 0);
		write_str_prop(key(K_N), unit->name, NULL, 0, 0);
		/* Maybe write the unit's id. */
		if (module->defall || module->defunitids || unit->occupant)
		  write_num_prop(key(K_SHARP), unit->id, 0, 0, 0);
		/* Need this to get back into the right transport. */
		if (unit->transport)
		  write_num_prop(key(K_IN), unit->transport->id, 0, 0, 0);
		/* Write optional info about the units. */
		if (module->defall || module->defunitprops)
		  write_unit_properties(unit);
		if (module->defall || module->defunitacts)
		  write_unit_act(unit);
		if (module->defall || module->defunitplans)
		  write_unit_plan(unit);
		/* close the unit out */
		end_form();
		newline_form();
		Dprintf("Wrote %s\n", unit_desig(unit));
	    }
	}
	newline_form();
    }
    Dprintf("... Done writing units.\n");
}

/* Write random properties, but only if they have non-default values. */

static void
write_unit_properties(unit)
Unit *unit;
{
    write_num_prop(key(K_NB), unit->number, 0, 0, 0);
    write_num_prop(key(K_HP), unit->hp, u_hp(unit->type), 0, 0);
    write_num_prop(key(K_CP), unit->cp, u_cp(unit->type), 0, 0);
    write_num_prop(key(K_CXP), unit->cxp, 0, 0, 0);
    write_num_prop(key(K_MO), unit->morale, 0, 0, 0);
    write_utype_value_list(key(K_TP), unit->tooling, 0, 0);
    write_side_value_list(key(K_OPINIONS), unit->opinions, 0, 0);
    write_mtype_value_list(key(K_M), unit->supply, 0, 0);
    write_lisp_prop(key(K_X), unit->hook, lispnil, 0, TRUE, FALSE);
}

/* Write out the unit's current actor state. */

static void
write_unit_act(unit)
Unit *unit;
{
    int acp = u_acp(unit->type), atype, i, slen;
    ActorState *act = unit->act;

    /* Actor state is kind of meaningless for dead units. */
    if (!alive(unit))
      return;
    if (act != NULL
	&& (act->acp != acp
	    || act->initacp != acp
	    || act->nextaction.type != A_NONE)) {
	if (1) {
	   newline_form();  space_form();
	}
	space_form();
	start_form(key(K_ACT));
	if (act->acp != acp)
	  write_num_prop(key(K_ACP), act->acp, acp, FALSE, FALSE);
	if (act->initacp != acp)
	  write_num_prop(key(K_ACP0), act->initacp, acp, FALSE, FALSE);
	if (act->nextaction.type != A_NONE) {
	    atype = act->nextaction.type;
	    space_form();
	    start_form(key(K_A));
	    add_to_form(actiondefns[atype].name);
	    slen = strlen(actiondefns[atype].argtypes);
    	    for (i = 0; i < slen; ++i)
	      add_num_to_form(act->nextaction.args[i]);
    	    if (act->nextaction.actee != 0) {
    	    	space_form();
		add_num_to_form(act->nextaction.actee);
    	    }
    	    end_form();
	}
	end_form();
    }
}

/* Write out the unit's current plan. */

static void
write_unit_plan(unit)
Unit *unit;
{
    Task *task;
    Plan *plan = unit->plan;

    /* The plan is kind of meaningless for dead units. */
    if (!alive(unit))
      return;
    if (plan) {
    	newline_form();  space_form();  space_form();
    	start_form(key(K_PLAN));
    	add_to_form(plantypenames[plan->type]);
	add_num_to_form(plan->creationturn);
	write_num_prop(key(K_START_TURN), plan->startturn, 0, 0, 0);
	write_num_prop(key(K_END_TURN), plan->endturn, 0, 0, 0);
	write_bool_prop(key(K_ASLEEP), plan->asleep, FALSE, 0, 0);
	write_bool_prop(key(K_RESERVE), plan->reserve, FALSE, 0, 0);
	write_bool_prop(key(K_DELAYED), plan->delayed, FALSE, 0, 0);
	write_bool_prop(key(K_WAIT), plan->waitingfortasks, FALSE, 0, 0);
	write_bool_prop(key(K_AUTOTASK), plan->autotask, FALSE, 0, 0);
	write_bool_prop(key(K_AI_CONTROL), plan->aicontrol, TRUE, 0, 0);
	write_bool_prop(key(K_SUPPLY_ALARM), plan->supply_alarm, TRUE, 0, 0);
	write_bool_prop(key(K_SUPPLY_IS_LOW), plan->supply_is_low, FALSE, 0, 0);
	write_bool_prop(key(K_WAIT_TRANSPORT), plan->waitingfortransport, FALSE, 0, 0);
	if (plan->maingoal)
	  write_goal(plan->maingoal, K_GOAL);
	if (plan->formation)
	  write_goal(plan->formation, K_FORMATION);
	if (plan->tasks) {
    	    space_form();
    	    start_form(key(K_TASKS));
	    for (task = plan->tasks; task != NULL; task = task->next) {
	    	space_form();
	    	write_task(task);
	    }
	    end_form();
	}
	end_form();
    }
}

static void
write_task(task)
Task *task;
{
    int i, numargs;
    char *argtypes;

    start_form(taskdefns[task->type].name);
    add_num_to_form(task->execnum);
    add_num_to_form(task->retrynum);
    argtypes = taskdefns[task->type].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i)
      add_num_to_form(task->args[i]);
    end_form();
}

static void
write_goal(goal, keyword)
Goal *goal;
int keyword;
{
    int i, numargs;
    char *argtypes;

    space_form();
    start_form(key(keyword));
    add_num_to_form(side_number(goal->side));
    add_num_to_form(goal->tf);
    add_to_form(goaldefns[goal->type].name);
    argtypes = goaldefns[goal->type].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i)
      add_num_to_form(goal->args[i]);
    end_form();
}

/* Write all the historical events recorded so far. */

static void sort_past_units PROTO ((void));

static void
write_history()
{
    PastUnit *pastunit;
    HistEvent *hevt;

    sort_past_units();
    /* Write all the past units that might be mentioned in events. */
    /* (should sort these by descending (negative) id first) */
    for (pastunit = past_unit_list; pastunit != NULL; pastunit = pastunit->next)
      write_past_unit(pastunit);
    write_historical_event(history);
    for (hevt = history->next; hevt != history; hevt = hevt->next)
      write_historical_event(hevt);
}

static int compare_past_units PROTO ((CONST void *pu1, CONST void *pu2));

static int
compare_past_units(pu1, pu2)
CONST void *pu1, *pu2;
{
    return ((*((PastUnit **) pu2))->id - (*((PastUnit **) pu1))->id);
}

static void
sort_past_units()
{
    int numpast = 0, i;
    PastUnit *pastunit, **tmparray;

    for (pastunit = past_unit_list; pastunit != NULL; pastunit = pastunit->next)
      ++numpast;
    /* Don't bother "sorting" if no more than one past unit. */
    if (numpast <= 1)
      return;
    tmparray = (PastUnit **) xmalloc(numpast * sizeof(PastUnit *));
    i = 0;
    for (pastunit = past_unit_list; pastunit != NULL; pastunit = pastunit->next)
      tmparray[i++] = pastunit;
    qsort(tmparray, numpast, sizeof(PastUnit *), compare_past_units);
    for (i = 0; i < numpast - 1; ++i)
      tmparray[i]->next = tmparray[i + 1];
    tmparray[numpast - 1]->next = NULL;
    past_unit_list = tmparray[0];
    free(tmparray);
}

static void
write_past_unit(pastunit)
PastUnit *pastunit;
{
    start_form(key(K_EXU));
    add_num_to_form(pastunit->id);
    add_to_form(shortest_escaped_name(pastunit->type));
    add_num_to_form(pastunit->x);
    add_num_to_form(pastunit->y);
    add_num_to_form(side_number(pastunit->side));
    write_num_prop(key(K_Z), pastunit->z, 0, 0, 0);
    write_str_prop(key(K_N), pastunit->name, NULL, 0, 0);
    write_num_prop(key(K_NB), pastunit->number, 0, 0, 0);
    end_form();
    newline_form();
}

static void
write_historical_event(hevt)
HistEvent *hevt;
{
    int i;
    char *descs;

    start_form(key(K_EVT));
    add_num_to_form(hevt->startdate);
    add_to_form(hevtdefns[hevt->type].name);
    add_num_to_form(hevt->observers);
    descs = hevtdefns[hevt->type].datadescs;
    for (i = 0; descs[i] != '\0'; ++i) {
	switch (descs[i]) {
	  case 'm':
	  case 'n':
	  case 'S':
	  case 'u':
	  case 'U':
	  case 'x':
	  case 'y':
	    add_num_to_form(hevt->data[i]);
	    break;
	  default:
	    run_warning("'%c' is not a recognized data desc char", descs[i]);
	    break;
	}
    }
    end_form();
    newline_form();
}

/* This is a generalized routine to do run-length-encoding of area layers.
   It uses hook fns to acquire data at a point and an optional translator to
   do any last-minute fixing.  It can use either a char or numeric encoding,
   depending on the expected range of values. */

static void
write_rle(datafn, lo, hi, translator, compress)
int (*datafn) PROTO ((int, int)), lo, hi, (*translator) PROTO ((int)), compress;
{
    int width, height, x, y, x0, y0, run, runval, val, trval;
    int numbad = 0;

    width = area.width;  height = area.height;
    if (doreshape) {
	width = reshaper->finalwidth;  height = reshaper->finalheight;
    }
    for (y = height-1; y >= 0; --y) {
	fprintf(fp, "  \"");
	run = 0;
	x0 = 0;  y0 = y;
	if (doreshape)
	  original_point(0, y, &x0, &y0);
	val = (*datafn)(x0, y0);
	/* Zero out anything not in the world, unless reshaping. */
	if (!doreshape && !in_area(x0, y0))
	  val = 0;
	/* Check that the data falls within bounds, clip if not. */
	if (lo <= hi && !between(lo, val, hi) && in_area(x0, y0)) {
	    ++numbad;
	    if (val < lo)
	      val = lo;
	    if (val > hi)
	      val = hi;
	}
	runval = val;
	for (x = 0; x < width; ++x) {
	    x0 = x;  y0 = y;
	    if (doreshape)
	      original_point(x, y, &x0, &y0);
	    val = (*datafn)(x0, y0);
	    /* Zero out anything not in the world, unless reshaping. */
	    if (!doreshape && !in_area(x0, y0))
	      val = 0;
	    /* Check that the data falls within bounds, clip if not. */
	    if (lo <= hi && !between(lo, val, hi) && in_area(x0, y0)) {
		++numbad;
		if (val < lo)
		  val = lo;
		if (val > hi)
		  val = hi;
	    }
	    if (val == runval && compress) {
		run++;
	    } else {
		trval = (translator != NULL ? (*translator)(runval) : runval);
		write_run(run, trval);
		/* Start a new run. */
		runval = val;
		run = 1;
	    }
	}
	/* Finish off the row. */
	trval = (translator != NULL ? (*translator)(val) : val);
	write_run(run, trval);
	fprintf(fp, "\"\n");
    }
    if (numbad > 0) {
	run_warning("%d values not between %d and %d", numbad, lo, hi);
    }
}

/* Write a single run, using the most compact encoding possible.
   0 - 29 is 'a' - '~', 30 - 63 is ':' - '[' */ 

static void
write_run(run, val)
int run, val;
{
    if (run > 1) {
	fprintf(fp, "%d", run);
	if (val > 63)
	  fprintf(fp, "*");
    }
    if (between(0, val, 29)) {
	fprintf(fp, "%c", val + 'a');
    } else if (between(30, val, 63)) {
	fprintf(fp, "%c", val - 30 + ':');
    } else {
	fprintf(fp, "%d,", val);
    }
}

/* Compute and return the corresponding point in an area being reshaped. */

static int
reshaped_point(x1, y1, x2p, y2p)
int x1, y1, *x2p, *y2p;
{
    *x2p = (((x1 - reshaper->subareax) * reshaper->finalsubareawidth )
	    / reshaper->subareawidth ) + reshaper->finalsubareax;
    *y2p = (((y1 - reshaper->subareay) * reshaper->finalsubareaheight)
	    / reshaper->subareaheight) + reshaper->finalsubareay;
    return TRUE;
}

static int
original_point(x1, y1, x2p, y2p)
int x1, y1, *x2p, *y2p;
{
    *x2p = (((x1 - reshaper->finalsubareax) * reshaper->subareawidth )
	    / reshaper->finalsubareawidth ) + reshaper->subareax;
    *y2p = (((y1 - reshaper->finalsubareay) * reshaper->subareaheight)
	    / reshaper->finalsubareaheight) + reshaper->subareay;
    return inside_area(*x2p, *y2p);
}

