/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Game module writing. */

#include "conq.h"

#define key(x) (keyword_name(x))

/* (should make the fp a global, would shrink code significantly) */

#define start_form(fp,hd) (fprintf((fp), "(%s", (hd)))
#define add_to_form(fp,x) (fprintf((fp), " %s", (x)))
#define add_num_to_form(fp,x) (fprintf((fp), " %d", (x)))
#define end_form(fp) (fprintf((fp), ")"))
#define newline_form(fp) (fprintf((fp), "\n"))
#define space_form(fp) (fprintf((fp), " "))

extern int gamestatesafe;

int doreshape = FALSE;

Module *reshaper = NULL;

char *shortestbuf = NULL;

/* These two routines make sure that any symbols and strings can
   be read in again. */

char *escapedthing = NULL;

char *
escaped_symbol(str)
char *str;
{
    char *tmp = str;

    if (str[0] == '|' && str[strlen(str)-1] == '|') return str;
    while (*tmp != '\0') {
	if (((char *) strchr(" ()#\";|", *tmp)) != NULL || isdigit(str[0])) {
	    sprintf(escapedthing, "|%s|", str);
	    return escapedthing;
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
    char *tmp = str, *rslt = escapedthing;

    *rslt++ = '"';
    if (str != NULL) {
	while (*tmp != 0) {
	    if (*tmp == '"') *rslt++ = '\\';
	    *rslt++ = *tmp++;
	}
    }
    *rslt++ = '"';
    *rslt = '\0';
    return escapedthing;
}

write_bool_prop(fp, name, value, dflt, nodefaulting, addnewline)
FILE *fp;
char *name;
int value, dflt, nodefaulting, addnewline;
{
    if (nodefaulting || value != dflt) {
	fprintf(fp, " (%s %s)", name, (value ? "true" : "false"));
	if (addnewline) newline_form(fp);
    }
}

write_num_prop(fp, name, value, dflt, nodefaulting, addnewline)
FILE *fp;
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
	if (addnewline) newline_form(fp);
    }
}

/* Handle the writing of a single string-valued property. */

write_str_prop(fp, name, value, dflt, nodefaulting, addnewline)
FILE *fp;
char *name, *value, *dflt;
int nodefaulting, addnewline;
{
    if (nodefaulting || string_not_default(value, dflt)) {
	fprintf(fp, " (%s %s)", name, escaped_string(value));
	if (addnewline) newline_form(fp);
    }
}

string_not_default(str, dflt)
char *str, *dflt;
{
    if (dflt == NULL || strlen(dflt) == 0) {
	if (str == NULL || strlen(str) == 0) {
	    return FALSE;
	} else {
	    return TRUE;
	}
    } else {
	if (str == NULL || strlen(str) == 0) {
	} else {
	    return (strcmp(str, dflt) != 0);
	}
    }
}

write_lisp_prop(fp, name, value, dflt, nodefaulting, ascdr, addnewline)
FILE *fp;
char *name;
Obj *value, *dflt;
int nodefaulting, ascdr, addnewline;
{
    Obj *rest;

    if (nodefaulting || !equal(value, dflt)) {
	space_form(fp);
	start_form(fp, name);
	if (ascdr && consp(value)) {
	    for (rest = value; rest != lispnil; rest = cdr(rest)) {
	    	space_form(fp);
	    	fprintlisp(fp, car(rest));
	    }
	} else {
	    space_form(fp);
	    fprintlisp(fp, value);
	}
	end_form(fp);
	if (addnewline) newline_form(fp);
    }
}

write_utype_value_list(fp, name, arr, dflt)
FILE *fp;
char *name;
short *arr, dflt;
{
    int u;

    if (arr == NULL) return;
    start_form(fp, name);
    for_all_unit_types(u) {
	fprintf(fp, " %hd", arr[u]);
    }
    end_form(fp);
}

write_mtype_value_list(fp, name, arr, dflt)
FILE *fp;
char *name;
short *arr, dflt;
{
    int m;

    if (arr == NULL) return;
    start_form(fp, name);
    for_all_material_types(m) {
	fprintf(fp, " %hd", arr[m]);
    }
    end_form(fp);
}

write_side_value_list(fp, name, arr, dflt)
FILE *fp;
char *name;
short *arr, dflt;
{
    int s;

    if (arr == NULL) return;
    start_form(fp, name);
    for (s = 0; s < numsides; ++s) {
	fprintf(fp, " %hd", arr[s]);
    }
    end_form(fp);
}

write_utype_string_list(fp, name, arr, dflt)
FILE *fp;
char *name;
char *arr, dflt;
{
    int u;

    if (arr == NULL) return;
    start_form(fp, name);
    for_all_unit_types(u) {
	fprintf(fp, " %s", escaped_string(arr[u]));
    }
    end_form(fp);
}

char *
shortest_unique_name(u)
int u;
{
    char *typename = u_type_name(u);

    if (shortestbuf == NULL) shortestbuf = xmalloc(BUFSIZE);
    if (strlen(u_internal_name(u)) < strlen(typename)) {
	typename = u_internal_name(u);
    }
    sprintf(shortestbuf, "%s", escaped_symbol(typename));
    return shortestbuf;
}

/* A saved game should include everything necessary to recreate a game
   exactly.  It is important that this routine not attempt to use graphics,
   since it may be called when graphics code fails. */

write_entire_game_state(fname)
char *fname;
{
    Module *module = create_game_module(fname);
    int rslt;

    module->filename = fname;
    module->compresslayers = TRUE;
    module->defall = TRUE;
    rslt = write_game_module(module);
    if (rslt) gamestatesafe = TRUE;
    return rslt;
}

/* Given a game module telling what is in the module, write out a file
   containing the requested content.  Return true if everything went OK. */

write_game_module(module)
Module *module;
{
    int i;
    FILE *fp;
    Obj *rest;

    if (escapedthing == NULL) escapedthing = xmalloc(BUFSIZE);
    if ((fp = fopen(module->filename, "w")) != NULL) {
	/* Write the definition of this game module. */
	start_form(fp, key(K_GAME_MODULE));
	add_to_form(fp, escaped_string(module->name));  newline_form(fp);
	if (module->defall) {
	    write_str_prop(fp, key(K_TITLE), module->title,
			   "", 1, 0);
	    write_str_prop(fp, key(K_VERSION), module->version,
			   "", 1, 0);
	    write_str_prop(fp, key(K_BLURB), module->blurb,
			   "", 1, 0);
	    write_str_prop(fp, key(K_BASE_MODULE), module->basemodulename,
			   "", 1, 0);
	    write_str_prop(fp, key(K_PROGRAM_VERSION), version,
			   "", 1, 0);
	    /* etc */
	}
	newline_form(fp);
	end_form(fp);  newline_form(fp);  newline_form(fp);
	if (module->defall || module->deftypes)
	  write_types(fp, module);
	if (module->defall || module->deftables)
	  write_tables(fp, module);
	if (module->defall || module->defglobals)
	  write_globals(fp, module);
	if (1 /* need to suppress synthesis after reload */) {
	  start_form(fp, key(K_SET));
	  add_to_form(fp, "synthesis-methods");
	  add_to_form(fp, "nil");
	  end_form(fp);
	  newline_form(fp);
	}
	if (module->defall || module->defscoring)
	  write_scorekeepers(fp, module);
	doreshape = reshape_the_output(module);
	reshaper = module;
	if (module->defall || module->defworld)
	  write_world(fp, module);
	if (module->defall || module->defareas)
	  write_areas(fp, module);
	if (module->defall || module->defsides)
	  write_sides(fp, module);
	if (module->defall || module->defplayers)
	  write_players(fp, module);
	if (module->defall || module->defunits)
	  write_units(fp, module);
	if (module->defall || module->defhistory)
	  write_history(fp, module);
	/* Write the game notes here (seems reasonable, no deeper reason). */
	if (module->instructions != lispnil) {
		start_form(fp, key(K_GAME_MODULE));  space_form(fp);
	    write_lisp_prop(fp, key(K_INSTRUCTIONS), module->instructions,
			    lispnil, 0, FALSE, 1);
		newline_form(fp);  end_form(fp);  newline_form(fp);  newline_form(fp);
	}
	if (module->notes != lispnil) {
		start_form(fp, key(K_GAME_MODULE));  space_form(fp);
	    write_lisp_prop(fp, key(K_NOTES), module->notes,
			    lispnil, 0, FALSE, 1);
		newline_form(fp);  end_form(fp);  newline_form(fp);  newline_form(fp);
	}
	if (module->designnotes != lispnil) {
		start_form(fp, key(K_GAME_MODULE));  space_form(fp);
	    write_lisp_prop(fp, key(K_DESIGN_NOTES), module->designnotes,
			    lispnil, 0, FALSE, 1);
		newline_form(fp);  end_form(fp);  newline_form(fp);  newline_form(fp);
	}
	fclose(fp);
	return TRUE;
    } else {
	return FALSE;
    }
}

/* Write definitions of all the types. */

write_types(fp, module)
FILE *fp;
Module *module;
{
    int complete = FALSE;
    int u, m, t, i, ival;
    char *sval;
    Obj *obj;

    /* (or write out all the default values first for doc, then
       only write changed values) */

    for_all_unit_types(u) {
	start_form(fp, key(K_UNIT_TYPE));
	add_to_form(fp, shortest_unique_name(u));
	newline_form(fp);
	for (i = 0; utypedefns[i].name != NULL; ++i) {
	    if (utypedefns[i].intgetter) {
		ival = (*(utypedefns[i].intgetter))(u);
		write_num_prop(fp, utypedefns[i].name, ival,
			       utypedefns[i].dflt, 0, 1);
	    } else if (utypedefns[i].strgetter) {
		sval = (*(utypedefns[i].strgetter))(u);
		write_str_prop(fp, utypedefns[i].name, sval,
			       utypedefns[i].dfltstr, 0, 1);
	    } else {
		obj = (*(utypedefns[i].objgetter))(u);
		write_lisp_prop(fp, utypedefns[i].name, obj,
				lispnil, FALSE, FALSE, TRUE);
	    }
	}
	end_form(fp);  newline_form(fp);
    }
    for_all_material_types(m) {
	start_form(fp, key(K_MATERIAL_TYPE));
	add_to_form(fp, escaped_symbol(m_type_name(m)));  newline_form(fp);
	for (i = 0; mtypedefns[i].name != NULL; ++i) {
	    if (mtypedefns[i].intgetter) {
		ival = (*(mtypedefns[i].intgetter))(m);
		write_num_prop(fp, mtypedefns[i].name, ival,
			       mtypedefns[i].dflt, 0, 1);
	    } else if (mtypedefns[i].strgetter) {
		sval = (*(mtypedefns[i].strgetter))(m);
		write_str_prop(fp, mtypedefns[i].name, sval,
			       mtypedefns[i].dfltstr, 0, 1);
	    } else {
		obj = (*(mtypedefns[i].objgetter))(m);
		write_lisp_prop(fp, mtypedefns[i].name, obj,
				lispnil, FALSE, FALSE, TRUE);
	    }
	}
	end_form(fp);  newline_form(fp);
    }
    for_all_terrain_types(t) {
	start_form(fp, key(K_TERRAIN_TYPE));
	add_to_form(fp, escaped_symbol(t_type_name(t)));  newline_form(fp);
	for (i = 0; ttypedefns[i].name != NULL; ++i) {
	    if (ttypedefns[i].intgetter) {
		ival = (*(ttypedefns[i].intgetter))(t);
		write_num_prop(fp, ttypedefns[i].name, ival,
			       ttypedefns[i].dflt, 0, 1);
	    } else if (ttypedefns[i].strgetter) {
		sval = (*(ttypedefns[i].strgetter))(t);
		write_str_prop(fp, ttypedefns[i].name, sval,
			       ttypedefns[i].dfltstr, 0, 1);
	    } else {
		obj = (*(ttypedefns[i].objgetter))(t);
		write_lisp_prop(fp, ttypedefns[i].name, obj,
				lispnil, FALSE, FALSE, TRUE);
	    }
	}
	end_form(fp);  newline_form(fp);
    }
    newline_form(fp);
}

/* Write definitions of all the tables. */

write_tables(fp, module)
FILE *fp;
Module *module;
{
    int tbl;

    newline_form(fp);
    for (tbl = 0; tabledefns[tbl].name != 0; ++tbl) {
	if (*(tabledefns[tbl].table) != NULL) {
	    write_table(fp, tabledefns[tbl].name,
			tabledefns[tbl].getter, tabledefns[tbl].dflt,
			tabledefns[tbl].index1, tabledefns[tbl].index2);
	}
    }
}

/* Write out a single table.  Only write it if it contains non-default
   values, and try to find runs of constant value. */

/* (should merge multiple rows also) */

#define star_from_typ(typ)  \
  ((typ) == UTYP ? "u*" : ((typ) == MTYP ? "m*" :"t*"))

#define name_from_typ(typ, i)  \
  ((typ) == UTYP ? shortest_unique_name(i) : ((typ) == MTYP ? m_type_name(i) : t_type_name(i)))

write_type_name_list(fp, typ, arr, dim)
FILE *fp;
int typ, *arr, dim;
{
    int j, first, listlen = 0;

    if (arr == NULL) return;
    for (j = 0; j < dim; ++j) if (arr[j]) ++listlen;
    if (listlen > 1) fprintf(fp, "(");
    first = TRUE;
    for (j = 0; j < dim; ++j) {
	if (arr[j]) {
	    if (first) first = FALSE; else fprintf(fp, " ");
	    fprintf(fp, "%s", escaped_symbol(name_from_typ(typ, j)));
	}
    }
    if (listlen > 1) end_form(fp);
}

write_type_value_list(fp, typ, arr, dim, getter, i)
FILE *fp;
int typ, *arr, dim, (*getter)(), i;
{
    int j, first, listlen = 0;

    for (j = 0; j < dim; ++j) if (arr == NULL || arr[j]) ++listlen;
    if (listlen > 1) fprintf(fp, "(");
    first = TRUE;
    for (j = 0; j < dim; ++j) {
	if (arr == NULL || arr[j]) {
	    if (first) first = FALSE; else fprintf(fp, " ");
	    fprintf(fp, "%d", (*getter)(i, j));
	}
    }
    if (listlen > 1) end_form(fp);
}

struct histo { int count, val; };

/* Sort into *descending* order by count. */

histo_compare(x, y)
struct histo *x, *y;
{
    return y->count - x->count;
}

write_table(fp, name, getter, dflt, typ1, typ2)
FILE *fp;
char *name;
int (*getter)(), dflt, typ1, typ2;
{
    int i, j, k, colvalue, constcol, next, numrandoms, first, nextrowdiffers, allsame;
    int dim1 = numtypes_from_index_type(typ1);
    int dim2 = numtypes_from_index_type(typ2);
    struct histo mostcommon[200]; /* needs to be more than MAXxTYPES */
    int indexes1[200], randoms[200];

    start_form(fp, key(K_TABLE));
    add_to_form(fp, name);
    fprintf(fp, "  ; %d", dflt);
    /* Choose the index that will result in fewest rows. */
    if (dim1 <= dim2) {
	for (k = 0; k < dim1; ++k) indexes1[k] = FALSE;
	for (i = 0; i < dim1; ++i) {
	    /* First see if this row has all the same values as the next one. */
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
	    if (nextrowdiffers) {
		    /* Make a histogram of all the values in this row. */
		    mostcommon[0].count = 1;  mostcommon[0].val = (*getter)(i, 0);
		    next = 1;
		    for (j = 0; j < dim2; ++j) {
			for (k = 0; k < next; ++k) {
			    if (mostcommon[k].val == (*getter)(i, j)) {
			    	++(mostcommon[k].count);
			    	break;
			    }
			}
			if (k == next) {
			    mostcommon[next].count = 1;  mostcommon[next].val = (*getter)(i, j);
			    ++next;
			}
		    }
		    if (next == 1 && mostcommon[0].val == dflt) {
		    	/* Entire row(s) is just the default table value. */
		    } else {
		    	allsame = FALSE;
		    	numrandoms = 0;
			if (next == 1) {
				allsame = TRUE;
			} else {
			    qsort(mostcommon, next, sizeof(struct histo), histo_compare);
			    if (mostcommon[0].count < (3 * dim2) / 4) {
			    } else {
			    	allsame = TRUE;
				for (j = 0; j < dim2; ++j) {
				    randoms[j] = (mostcommon[0].val != (*getter)(i, j));
				    if (randoms[j]) ++numrandoms;
				}
			    }
			}
			if (mostcommon[0].val != dflt) {
				fprintf(fp, "\n  (");
				write_type_name_list(fp, typ1, indexes1, dim1);
				fprintf(fp, " %s ", star_from_typ(typ2));
				if (allsame) {
			    		fprintf(fp, "%d", mostcommon[0].val);
				} else {
			    		write_type_value_list(fp, typ2, NULL, dim2, getter, i);
				}
				fprintf(fp, ")");
			}
			if (numrandoms > 0) {
			    fprintf(fp, "\n  (");
			    write_type_name_list(fp, typ1, indexes1, dim1);
			    fprintf(fp, " ");
			    write_type_name_list(fp, typ2, randoms, dim2);
			    fprintf(fp, " ");
			    write_type_value_list(fp, typ2, randoms, dim2, getter, i);
			    fprintf(fp, ")");
			}
		    }
		    for (k = 0; k < dim1; ++k) indexes1[k] = FALSE;
	    }
	}
    } else {
	for (i = 0; i < dim2; ++i) {
	    constcol = TRUE;
	    colvalue = (*getter)(0, i);
	    for (j = 0; j < dim1; ++j) {
		if ((*getter)(j, i) != colvalue) {
		    constcol = FALSE;
		    break;
		}
	    }
	    if (!constcol || colvalue != dflt) {
		fprintf(fp, "\n  (%s %s",
			star_from_typ(typ1),
			escaped_symbol(name_from_typ(typ2, i)));
		if (constcol) {
		    fprintf(fp, " %d", colvalue);
		} else {
		    fprintf(fp, " (");
		    for (j = 0; j < dim1; ++j) {
			fprintf(fp, " %d", (*getter)(j, i));
		    }
		    fprintf(fp, ")");
		}
		fprintf(fp, ")");
	    }
	}
    }
    end_form(fp);  newline_form(fp);
}

/* Write info about the whole world. */

write_world(fp, module)
FILE *fp;
Module *module;
{
	newline_form(fp);
	start_form(fp, key(K_WORLD));
    fprintf(fp, " %d",
	    (doreshape ? reshaper->finalcircumference : world.circumference));
    write_num_prop(fp, key(K_DAY_LENGTH), world.daylength, 1, 0, 1);
    write_num_prop(fp, key(K_YEAR_LENGTH), world.yearlength, 1, 0, 1);
    end_form(fp);  newline_form(fp);
}

/* Write info about the area in the world.  This code uses run-length encoding
   to reduce the size of each written layer as much as possible.  Note
   also that each layer is written as a separate form, so that the Lisp
   reader doesn't have to read really large forms back in. */

write_areas(fp, module)
FILE *fp;
Module *module;
{
    int all = module->defall, compress = module->compresslayers;

    newline_form(fp);
    /* Write the basic dimensions. */
    start_form(fp, key(K_AREA));
    add_num_to_form(fp, (doreshape ? reshaper->finalwidth : area.width));
    add_num_to_form(fp, (doreshape ? reshaper->finalheight : area.height));
    /* Write all the scalar properties. */
    write_num_prop(fp, key(K_LATITUDE), area.latitude, 0, 0, 1);
    write_num_prop(fp, key(K_LONGITUDE), area.longitude, 0, 0, 1);
    write_num_prop(fp, key(K_CELL_WIDTH), area.cellwidth, 0, 0, 1);
    end_form(fp);
    newline_form(fp);
    /* Write the area's layers, each as a separate form. */
    if (all || module->defareaterrain) write_area_terrain(fp, compress);
    if (all || module->defareaterrain) write_area_aux_terrain(fp, compress);
    if (all || module->defareamisc) write_area_features(fp, compress);
    if (all || module->defareamisc) write_area_elevations(fp, compress);
    if (all || module->defareamisc) write_area_people_sides(fp, compress);
    if (all || module->defareamaterial) write_area_materials(fp, compress);
    if (all || module->defareaweather) write_area_temperatures(fp, compress);
    if (all || module->defareaweather) write_area_clouds(fp, compress);
    if (all || module->defareaweather) write_area_winds(fp, compress);
}

fn_terrain_at(x, y)
int x, y;
{
    return terrain_at(x, y);
}

write_area_terrain(fp, compress)
FILE *fp;
int compress;
{
    int t;

    start_form(fp, key(K_AREA));  space_form(fp);
    start_form(fp, key(K_TERRAIN));  newline_form(fp);
    fprintf(fp, "  ");
    start_form(fp, key(K_BY_NAME));
    for_all_terrain_types(t) {
    	if (t % 5 == 0) { newline_form(fp); fprintf(fp, "   "); }
    	fprintf(fp, " (%s %d)", escaped_symbol(t_type_name(t)), t);
    }
    end_form(fp);  newline_form(fp);
    write_rle(fp, fn_terrain_at, 0, numttypes-1, NULL, compress);
    end_form(fp);  end_form(fp);  newline_form(fp);
}

fn_aux_terrain_at(x, y)
int x, y;
{
    return aux_terrain_at(x, y, tmpttype);
}

write_area_aux_terrain(fp, compress)
FILE *fp;
int compress;
{
    int t;

    for_all_terrain_types(t) {
	if (aux_terrain_defined(t)) {
	    start_form(fp, key(K_AREA));  space_form(fp);
	    start_form(fp, key(K_AUX_TERRAIN));
	    add_to_form(fp, escaped_symbol(t_type_name(t)));
	    newline_form(fp);
	    tmpttype = t;
	    write_rle(fp, fn_aux_terrain_at, 0, 127, NULL, compress);
	    end_form(fp);  end_form(fp);  newline_form(fp);
	}
    }
}

fn_feature_at(x, y)
int x, y;
{
    return raw_feature_at(x, y);
}

write_area_features(fp, compress)
FILE *fp;
int compress;
{
    Feature *feature;
    extern Feature *featurelist;

    if (featurelist == NULL || !features_defined()) return;
    renumber_features();
    start_form(fp, key(K_AREA));  space_form(fp);
    start_form(fp, key(K_FEATURES));
    fprintf(fp, " (\n");
    /* Dump out the list of features first. */
    for (feature = featurelist; feature != NULL; feature = feature->next) {
	fprintf(fp, "   (%d %s",
		feature->id,
		escaped_string(feature->typename));
	fprintf(fp, " %s)\n",
		escaped_string(feature->name));
    }
    fprintf(fp, "  )\n");
    /* Now record which features go with which hexes. */
    write_rle(fp, fn_feature_at, 0, -1, NULL, compress);
    fprintf(fp, "))\n");
}

fn_elevation_at(x, y)
int x, y;
{
    return elev_at(x, y);
}

write_area_elevations(fp, compress)
FILE *fp;
int compress;
{
    if (elevations_defined()) {
	start_form(fp, key(K_AREA));  space_form(fp);
	start_form(fp, key(K_ELEVATIONS));
	newline_form(fp);
	write_rle(fp, fn_elevation_at, minelev, maxelev, NULL, compress);
	end_form(fp);  end_form(fp);  newline_form(fp);
    }
}

fn_people_side_at(x, y)
int x, y;
{
    return people_side_at(x, y);
}

write_area_people_sides(fp, compress)
FILE *fp;
int compress;
{
    if (people_sides_defined()) {
	start_form(fp, key(K_AREA));  space_form(fp);
	start_form(fp, key(K_PEOPLE_SIDES));
	newline_form(fp);
	write_rle(fp, fn_people_side_at, 0, MAXSIDES+1, NULL, compress);
	end_form(fp);  end_form(fp);  newline_form(fp);
    }
}

fn_material_at(x, y)
int x, y;
{
    return material_at(x, y, tmpmtype);
}

write_area_materials(fp, compress)
FILE *fp;
int compress;
{
    int m;

    if (any_cell_materials_defined()) {
	for_all_material_types(m) {
	    if (cell_material_defined(m)) {
		start_form(fp, key(K_AREA));  space_form(fp);
		start_form(fp, key(K_MATERIAL));
		add_to_form(fp, escaped_symbol(m_type_name(m)));
		newline_form(fp);
		tmpmtype = m;
		write_rle(fp, fn_material_at, 0, 127, NULL, compress);
		end_form(fp); end_form(fp); newline_form(fp);
	    }
	}
    }
}

fn_temperature_at(x, y)
int x, y;
{
    return temperature_at(x, y);
}

write_area_temperatures(fp, compress)
FILE *fp;
int compress;
{
    if (temperatures_defined()) {
	start_form(fp, key(K_AREA));  space_form(fp);
	start_form(fp, key(K_TEMPERATURES));
	newline_form(fp);
	write_rle(fp, fn_temperature_at, 0, 9999, NULL, compress);
	end_form(fp); end_form(fp); newline_form(fp);
    }
}

fn_raw_cloud_at(x, y)
int x, y;
{
    return raw_cloud_at(x, y);
}

fn_raw_cloud_bottom_at(x, y)
int x, y;
{
    return raw_cloud_bottom_at(x, y);
}

fn_raw_cloud_height_at(x, y)
int x, y;
{
    return raw_cloud_height_at(x, y);
}

write_area_clouds(fp, compress)
FILE *fp;
int compress;
{
    if (clouds_defined()) {
	start_form(fp, key(K_AREA));  space_form(fp);
	start_form(fp, key(K_CLOUDS));
	newline_form(fp);
	write_rle(fp, fn_raw_cloud_at, 0, 127, NULL, compress);
	end_form(fp); end_form(fp); newline_form(fp);
    }
    if (cloud_bottoms_defined()) {
	start_form(fp, key(K_AREA));  space_form(fp);
	start_form(fp, key(K_CLOUD_BOTTOMS));
	newline_form(fp);
	write_rle(fp, fn_raw_cloud_bottom_at, 0, 9999, NULL, compress);
	end_form(fp); end_form(fp); newline_form(fp);
    }
    if (cloud_heights_defined()) {
	start_form(fp, key(K_AREA));  space_form(fp);
	start_form(fp, key(K_CLOUD_HEIGHTS));
	newline_form(fp);
	write_rle(fp, fn_raw_cloud_height_at, 0, 9999, NULL, compress);
	end_form(fp); end_form(fp); newline_form(fp);
    }
}

fn_raw_wind_at(x, y)
int x, y;
{
    return raw_wind_at(x, y);
}

write_area_winds(fp, compress)
FILE *fp;
int compress;
{
    if (winds_defined()) {
	start_form(fp, key(K_AREA));  space_form(fp);
	start_form(fp, key(K_WINDS));
	newline_form(fp);
	write_rle(fp, fn_raw_wind_at, 0, 127, NULL, compress);
	end_form(fp); end_form(fp); newline_form(fp);
    }
}

/* Write the globals.  The "complete" flag forces all values out, even
   if they match the compiled-in defaults. */

/* These decls are needed because the functions are mentioned in gvar.def. */

int set_g_side_lib_default();
int set_g_synth_methods_default();

write_globals(fp, module)
FILE *fp;
Module *module;
{
    int complete = FALSE;

    newline_form(fp);

#undef  DEF_VAR_I
#define DEF_VAR_I(STR,FNAME,setfname,doc,var,lo,DFLT,hi)  \
    if (complete || FNAME() != DFLT)  { \
      start_form(fp, key(K_SET));  \
      fprintf(fp, " %s %d)\n", STR, FNAME());  \
    }
#undef  DEF_VAR_S
#define DEF_VAR_S(STR,FNAME,setfname,doc,var,DFLT)  \
    if (complete || string_not_default(FNAME(), DFLT))  { \
      start_form(fp, key(K_SET));  \
      fprintf(fp, " %s %s)\n", STR, escaped_string(FNAME()));  \
    }
#undef  DEF_VAR_L
#define DEF_VAR_L(STR,FNAME,setfname,doc,var,DFLT)  \
    if (complete || ((DFLT) == NULL && FNAME() != lispnil))  {  \
      if (FNAME() != NULL) {  \
        start_form(fp, key(K_SET));  \
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

write_scorekeepers(fp, module)
FILE *fp;
Module *module;
{
    int i, u, r;
    Scorekeeper *sk;

    for_all_scorekeepers(sk) {
	start_form(fp, key(K_SCOREKEEPER));  fprintf(fp, " %d", sk->id);
	write_str_prop(fp, key(K_TITLE), sk->title, "", 0, 1);
	write_lisp_prop(fp, key(K_WHEN), sk->when, lispnil, 0, FALSE, 1);
	write_lisp_prop(fp, key(K_APPLIES_TO), sk->who, lispnil, 0, FALSE, 1);
/*	write_lisp_prop(fp, key(K_KNOWN_TO), sk->who, lispnil, 0, FALSE, 1); */
	write_lisp_prop(fp, key(K_TRIGGER), sk->trigger, lispnil, 0, FALSE, 1);
	write_lisp_prop(fp, key(K_DO), sk->body, lispnil, 0, FALSE, 1);
	write_num_prop(fp, key(K_TRIGGERED), sk->triggered, 0, 0, 1); 
	write_num_prop(fp, key(K_INITIAL), sk->initial, 0, 0, 1); 
	write_lisp_prop(fp, key(K_NOTES), sk->notes, lispnil, 0, FALSE, 1);
	end_form(fp);  newline_form(fp);
    }
}

/* Write declarations of all the sides. */

write_sides(fp, module)
FILE *fp;
Module *module;
{
    Side *side;

    fprintf(fp, "\n; %d sides\n", numsides);
    Dprintf("Will try to write %d sides ...\n", numsides);
    for_all_sides(side) {
	start_form(fp, key(K_SIDE));  fprintf(fp, " %d", side->id);
	write_side_properties(side, fp);
	end_form(fp);  newline_form(fp);
	if (module->defall || module->defsideviews) write_side_view(side, fp);
	Dprintf("  Wrote side %s\n", side_desig(side));
    }
    Dprintf("... Done writing sides\n");
}

/* Write random properties of a side. */

write_side_properties(side, fp)
Side *side;
FILE *fp;
{
    int u, s, i;
    extern int numscores;

    write_str_prop(fp, key(K_NAME), side->name, "", 0, 1);
    write_str_prop(fp, key(K_LONG_NAME), side->longname, "", 0, 1);
    write_str_prop(fp, key(K_SHORT_NAME), side->shortname, "", 0, 1);
    write_str_prop(fp, key(K_NOUN), side->noun, "", 0, 1);
    write_str_prop(fp, key(K_PLURAL_NOUN), side->pluralnoun, "", 0, 1);
    write_str_prop(fp, key(K_ADJECTIVE), side->adjective, "", 0, 1);
    write_str_prop(fp, key(K_COLOR), side->colorscheme, "", 0, 1);
    write_str_prop(fp, key(K_EMBLEM_NAME), side->emblemname, "", 0, 1);
    write_str_prop(fp, key(K_CLASS), side->sideclass, "", 0, 1);
    write_utype_string_list(fp, key(K_UNIT_NAMERS), side->unitnamers, "");
    write_bool_prop(fp, key(K_ACTIVE), side->ingame, "", 0, 1);
    write_num_prop(fp, key(K_STATUS), side->status, 0, 0, 1);
    write_num_prop(fp, key(K_TURN_TIME_USED), side->turntimeused, 0, 0, 1);
    write_num_prop(fp, key(K_TOTAL_TIME_USED), side->totaltimeused, 0, 0, 1);
    write_num_prop(fp, key(K_FINISHED_TURN), side->finishedturn, 0, 0, 1);
    write_num_prop(fp, key(K_CONTROLLED_BY), side_number(side->controlledby), 0, 0, 1);
    write_side_value_list(fp, key(K_TRUSTS), side->trusts, 0);
    write_side_value_list(fp, key(K_TRADES), side->trades, 0);
    write_utype_value_list(fp, key(K_START_WITH), side->startwith, 0);
    write_utype_value_list(fp, key(K_NEXT_NUMBERS), side->counts, 0);
    write_utype_value_list(fp, key(K_TECH), side->tech, 0);
    write_utype_value_list(fp, key(K_INIT_TECH), side->inittech, 0);
    if (side->scores) {
	newline_form(fp);
	start_form(fp, key(K_SCORES));
	for (i = 0; i < numscores; ++i) {
	    fprintf(fp, " %d", side->scores[i]);
	}
	end_form(fp);
	newline_form(fp);
    }
    if (side_has_ai(side)) {
	newline_form(fp);
	start_form(fp, key(K_AI));
	ai_write_state(fp, side);
	end_form(fp);
	newline_form(fp);
    }
}

/* Write about what has been seen in the area. */

/* (should have option to spec symbolic dict of sides and units) */

fn_terrain_view(x, y)
int x, y;
{
    return terrain_view(tmpside, x, y);
}

fn_unit_view(x, y)
int x, y;
{
    return unit_view(tmpside, x, y);
}

fn_unit_view_date(x, y)
int x, y;
{
    return unit_view_date(tmpside, x, y);
}

write_side_view(side, fp, compress)
Side *side;
FILE *fp;
int compress;
{
    /* View layers are not defined if see-all is in effect. */
    if (g_see_all()) return;
    tmpside = side;
    newline_form(fp);
    start_form(fp, key(K_SIDE));  fprintf(fp, " %d", side->id);  space_form(fp);
    start_form(fp, key(K_TERRAIN_VIEW));  newline_form(fp);
    write_rle(fp, fn_terrain_view, 0, 10000, NULL, compress);
    end_form(fp);  end_form(fp);  newline_form(fp);
    start_form(fp, key(K_SIDE));  fprintf(fp, " %d", side->id);  space_form(fp);
    start_form(fp, key(K_UNIT_VIEW));  newline_form(fp);
    write_rle(fp, fn_unit_view, 0, 10000, NULL, compress);
    end_form(fp);  end_form(fp);  newline_form(fp);
    start_form(fp, key(K_SIDE));  fprintf(fp, " %d", side->id);  space_form(fp);
    start_form(fp, key(K_UNIT_VIEW_DATES));  newline_form(fp);
    write_rle(fp, fn_unit_view_date, 0, 10000, NULL, compress);
    end_form(fp);  end_form(fp);  newline_form(fp);
}

write_players(fp, module)
FILE *fp;
Module *module;
{
    Side *side;
    Player *player;

    fprintf(fp, "; %d players\n", numplayers);
    Dprintf("Will try to write %d players ...\n", numplayers);
    for_all_sides(side) {
	if ((player = side->player) != NULL) {
	    write_player(fp, player);
	    Dprintf("Wrote player %s,\n", player_desig(player));
	}
    }
    Dprintf("... Done writing players.\n");
}

write_player(fp, player)
FILE *fp;
Player *player;
{
    start_form(fp, key(K_PLAYER));  fprintf(fp, " %d", 0);  newline_form(fp);
    write_str_prop(fp, key(K_NAME), player->name, "", 0, 1);
    write_str_prop(fp, key(K_CONFIG_NAME), player->configname, "", 0, 1);
    write_str_prop(fp, key(K_DISPLAY_NAME), player->displayname, "", 0, 1);
    write_str_prop(fp, key(K_AI_TYPE_NAME), player->aitypename, "", 0, 1);
    end_form(fp);  newline_form(fp);
}

/* Should write out "unit groups" with dict prepended, then can use with
   multiple games */

/* Write the unit section of a game module. */

write_units(fp, module)
FILE *fp;
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
	if (1 /* test unit for worthiness to be written */) {
	    if (doreshape) {
		reshaped_point(unit->x, unit->y, &x, &y);
	    } else {
		x = unit->x;  y = unit->y;
	    }
	    start_form(fp, shortest_unique_name(unit->type));
	    fprintf(fp, " %d %d %d", x, y, side_number(unit->side));
	    write_num_prop(fp, key(K_Z), unit->z, 0, 0, 0);
	    write_str_prop(fp, key(K_N), unit->name, NULL, 0, 0);
	    /* Maybe write the unit's id. */
	    if (module->defall || module->defunitids || unit->occupant) {
		write_num_prop(fp, key(K_SHARP), unit->id, 0, 0, 0);
	    }
	    /* Need this to get back into the right transport. */
	    if (unit->transport) {
		write_num_prop(fp, key(K_IN), unit->transport->id, 0, 0, 0);
	    }
	    /* Write optional info about the units. */
	    if (module->defall || module->defunitprops)
	      write_unit_properties(unit, fp);
	    if (module->defall || module->defunitacts)
	      write_unit_act(unit, fp);
	    if (module->defall || module->defunitplans)
	      write_unit_plan(unit, fp);
	    /* close the unit out */
	    end_form(fp);
	    newline_form(fp);
	    Dprintf("Wrote %s\n", unit_desig(unit));
	}
      }
      newline_form(fp);
    }
    Dprintf("... Done writing units.\n");
}

/* Write random properties, but only if they have non-default values. */

write_unit_properties(unit, fp)
Unit *unit;
FILE *fp;
{
    int m, allfull = TRUE;

    write_num_prop(fp, key(K_NM), unit->number, 0, 0, 0);
    write_num_prop(fp, key(K_HP), unit->hp, u_hp(unit->type), 0, 0);
    write_num_prop(fp, key(K_CP), unit->cp, u_cp(unit->type), 0, 0);
    write_num_prop(fp, key(K_CXP), unit->cxp, 0, 0, 0);
    write_utype_value_list(fp, key(K_TP), unit->tooling, 0);
    /* (should use general write mtype value list) */
    if (nummtypes > 0) {
	for_all_material_types(m) {
	    if (unit->supply[m] < um_storage(unit->type, m)) {
		allfull = FALSE;
		break;
	    }
	}
	if (!allfull) {
	    start_form(fp, key(K_M));
	    if (1 /* write all of them */) {
		for_all_material_types(m) {
		    fprintf(fp, " %d", unit->supply[m]);
		}
	    } else {
	        /* (should write a named list) */
	    }
	    end_form(fp);
	}
    }
    write_lisp_prop(fp, key(K_X), unit->hook, lispnil, 0, TRUE, FALSE);
}

/* Write out the unit's current actor state. */

write_unit_act(unit, fp)
Unit *unit;
FILE *fp;
{
    int acp = u_acp(unit->type), atype, i;
    ActorState *act = unit->act;

    if (act != NULL
	&& (act->acp != acp
	    || act->initacp != acp
	    || act->nextaction.type != A_NONE)) {
	if (1) {
	   newline_form(fp);  space_form(fp);
	}
	space_form(fp);
	start_form(fp, key(K_ACT));
	if (act->acp != acp) /* dubious */
	  write_num_prop(fp, key(K_ACP), act->acp, acp, FALSE, FALSE);
	if (act->initacp != acp)
	  write_num_prop(fp, key(K_ACP0), act->initacp, acp, FALSE, FALSE);
	if (act->nextaction.type != A_NONE) {
	    atype = act->nextaction.type;
	    space_form(fp);
	    start_form(fp, key(K_A));
	    add_to_form(fp, actiondefns[atype].name);
    	    for (i = 0; i < strlen(actiondefns[atype].argtypes); ++i) {
		add_num_to_form(fp, act->nextaction.args[i]);
    	    }
    	    if (act->nextaction.actee != 0) {
    	    	space_form(fp);
		add_num_to_form(fp, act->nextaction.actee);
    	    }
    	    end_form(fp);
	}
	end_form(fp);
    }
}

/* Write out the unit's current plan. */

write_unit_plan(unit, fp)
Unit *unit;
FILE *fp;
{
    Task *task;
    Plan *plan = unit->plan;

    if (plan) {
    	newline_form(fp);  space_form(fp);  space_form(fp);
    	start_form(fp, key(K_PLAN));
    	add_to_form(fp, plantypenames[plan->type]);
	if (plan->asleep)
	  write_bool_prop(fp, key(K_ASLEEP), plan->asleep, FALSE, 0, 0);
	if (plan->reserve)
	  write_bool_prop(fp, key(K_RESERVE), plan->reserve, FALSE, 0, 0);
	if (plan->waitingfortasks)
	  write_bool_prop(fp, key(K_WAIT), plan->waitingfortasks, FALSE, 0, 0);
	if (plan->maingoal) {
	    write_goal(plan->maingoal, fp);
	}
	if (plan->tasks) {
    	    space_form(fp);
    	    start_form(fp, key(K_TASKS));
	    for (task = plan->tasks; task != NULL; task = task->next) {
	    	space_form(fp);
	    	write_task(task, fp);
	    }
	    end_form(fp);
	}
	end_form(fp);
    }
}

write_task(task, fp)
Task *task;
FILE *fp;
{
    int i;
    char *argtypes;

    start_form(fp, taskdefns[task->type].name);
    add_num_to_form(fp, task->execnum);
    add_num_to_form(fp, task->retrynum);
    argtypes = taskdefns[task->type].argtypes;
    for (i = 0; i < strlen(argtypes); ++i) {
	add_num_to_form(fp, task->args[i]);
    }
    end_form(fp);
}

write_goal(goal, fp)
Goal *goal;
FILE *fp;
{
    int i;
    char *argtypes;

    space_form(fp);
    start_form(fp, key(K_GOAL));
    add_num_to_form(fp, side_number(goal->side));
    add_num_to_form(fp, goal->tf);
    add_to_form(fp, goaldefns[goal->type].name);
    argtypes = goaldefns[goal->type].argtypes;
    for (i = 0; i < strlen(argtypes); ++i) {
	add_num_to_form(fp, goal->args[i]);
    }
    end_form(fp);
}

/* This is a generalized routine to do run-length-encoding of area layers.
   It uses hook fns to acquire data at a point and an optional translator to
   do any last-minute fixing.  It can use either a char or numeric encoding,
   depending on the expected range of values. */

/* The default translator. */

fn_identity(x) int x; { return x; }

write_rle(fp, datafn, lo, hi, translator, compress)
FILE *fp;
int (*datafn)(), lo, hi, (*translator)(), compress;
{
    int width, height, x, y, x0, y0, run, runval, val, trval, i, iter;

    if (translator == NULL) translator = fn_identity;
    width = area.width;  height = area.height;
    if (doreshape) {
	width = reshaper->finalwidth;  height = reshaper->finalheight;
    }
    for (y = height-1; y >= 0; --y) {
	fprintf(fp, "  \"");
	run = 0;
	x0 = 0;  y0 = y;
	if (doreshape) {
	    original_point(0, y, &x0, &y0);
	}
	val = (*datafn)(x0, y0);
	/* Zero out anything not in the world, unless reshaping. */
	if (!doreshape && !in_area(x0, y0)) val = 0;
	runval = val;
	for (x = 0; x < width; ++x) {
	    x0 = x;  y0 = y;
	    if (doreshape) {
		original_point(x, y, &x0, &y0);
	    }
	    val = (*datafn)(x0, y0);
	    /* Zero out anything not in the world, unless reshaping. */
	    if (!doreshape && !in_area(x0, y0)) val = 0;
	    if (val == runval && compress) {
		run++;
	    } else {
		trval = (*translator)(runval);
		write_run(fp, run, trval);
		runval = val;
		run = 1;
	    }
	}
	/* Finish off the row. */
	trval = (*translator)(val);
	write_run(fp, run, trval);
	fprintf(fp, "\"\n");
    }
}

/* Write a single run, using the most compact encoding possible. */
/* 0 - 29 is 'a' - '~', 30 - 63 is ':' - '[' */ 

write_run(fp, run, val)
FILE *fp;
int run, val;
{
    if (run > 1) {
	fprintf(fp, "%d", run);
	if (val > 63) fprintf(fp, "*");
    }
    if (between(0, val, 29)) {
	fprintf(fp, "%c", val + 'a');
    } else if (between(30, val, 63)) {
	fprintf(fp, "%c", val - 30 + ':');
    } else {
	fprintf(fp, "%d,", val);
    }
}

/* Write all the historical events recorded so far. */

write_history(fp, selector)
FILE *fp;
char *selector;
{
    HistEvent *hevt;

    write_historical_event(fp, history);
    for (hevt = history->next; hevt != history; hevt = hevt->next) {
	write_historical_event(fp, hevt);
    }
}

write_historical_event(fp, hevt)
FILE *fp;
HistEvent *hevt;
{
    char comment[BUFSIZE];

    sprintf(comment, "");
    start_form(fp, key(K_EVT));
    add_num_to_form(fp, hevt->startdate);
    add_to_form(fp, hevtdefns[hevt->type].name);
    /* Print the event's data in an appropriate form. */
    switch (hevt->type) {
      case H_LOG_STARTED:
      case H_GAME_STARTED:
      case H_LOG_ENDED:
      case H_GAME_ENDED:
	break;
      case H_SIDE_JOINED:
      case H_SIDE_WITHDREW:
      case H_SIDE_LOST:
      case H_SIDE_WON:
	fprintf(fp, " %d", hevt->data[0]);
	break;
      case H_UNIT_CREATED:
      case H_UNIT_COMPLETED:
      case H_UNIT_ACQUIRED:
      case H_UNIT_MOVED:
      case H_UNIT_ASSAULTED:
      case H_UNIT_DAMAGED:
      case H_UNIT_CAPTURED:
      case H_UNIT_KILLED:
      case H_UNIT_WRECKED:
      case H_UNIT_VANISHED:
      case H_UNIT_GARRISONED:
      case H_UNIT_DISBANDED:
      case H_UNIT_STARVED:
      case H_UNIT_LEFT_WORLD:
	fprintf(fp, " %d", hevt->data[0]);
	sprintf(comment, unit_desig(find_unit(hevt->data[0])));
	break;
      default:
	break;
    }
    end_form(fp);
    if (strlen(comment) > 0) {
    	fprintf(fp, " ; %s", comment);
    }
    newline_form(fp);
}

/* Statistics are a compressed form of historical data. */

#if 0

write_side_statistics(side, fp)
Side *side;
FILE *fp;
{
    int anystuff;
    int u, u2, i;

    /* The balance sheet of gains and losses. */
    fprintf(fp, "  (balance-sheet\n");
    for_all_unit_types(u) {
	anystuff = FALSE;
	for (i = 0; i < NUMREASONS; ++i) {
	    if (side_balance(side, u, i) > 0) { anystuff = TRUE; break; }
	}
	if (anystuff) {
	    fprintf(fp, "    (%s", u_type_name(u));
	    for (i = 0; i < NUMREASONS; ++i) {
		fprintf(fp, " %d", side_balance(side, u, i));
	    }
	    fprintf(fp, ")\n");
	}
    }
    fprintf(fp, "  )\n");
    /* The total number of attacks. */
    fprintf(fp, "  (attack-totals\n");
    for_all_unit_types(u) {
	anystuff = FALSE;
	for_all_unit_types(u2) {
	    if (side_atkstats(side, u, u2) > 0) { anystuff = TRUE; break; }
	}
	if (anystuff) {
	    fprintf(fp, "    (%s", u_type_name(u));
	    for_all_unit_types(u2) {
		fprintf(fp, " %d", side_atkstats(side, u, u2));
	    }
	    fprintf(fp, "    )\n");
	}
    }
    fprintf(fp, "  )\n");
    /* The total number of hits inflicted. */
    fprintf(fp, "  (hit-totals\n");
    for_all_unit_types(u) {
	anystuff = FALSE;
	for_all_unit_types(u2) {
	    if (side_hitstats(side, u, u2) > 0) { anystuff = TRUE; break; }
	}
	if (anystuff) {
	    fprintf(fp, "    (%s", u_type_name(u));
	    for_all_unit_types(u2) {
		fprintf(fp, " %d", side_hitstats(side, u, u2));
	    }
	    fprintf(fp, "    )\n");
	}
    }
    fprintf(fp, "  )\n");
}
#endif

reshaped_point(x1, y1, x2p, y2p)
int x1, y1, *x2p, *y2p;
{
    *x2p = (((x1 - reshaper->subareax) * reshaper->finalsubareawidth )
	    / reshaper->subareawidth ) + reshaper->finalsubareax;
    *y2p = (((y1 - reshaper->subareay) * reshaper->finalsubareaheight)
	    / reshaper->subareaheight) + reshaper->finalsubareay;
    return TRUE;
}

original_point(x1, y1, x2p, y2p)
int x1, y1, *x2p, *y2p;
{
    *x2p = (((x1 - reshaper->finalsubareax) * reshaper->subareawidth )
	    / reshaper->finalsubareawidth ) + reshaper->subareax;
    *y2p = (((y1 - reshaper->finalsubareay) * reshaper->subareaheight)
	    / reshaper->finalsubareaheight) + reshaper->subareay;
    return inside_area(*x2p, *y2p);
}

