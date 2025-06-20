/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */
/* libr - procedures for reading (in C format) ais and tre files*/

#include "hdr.h"
#include "vars.h"
#include "libhdr.h"
#include "ifile.h"
#include "dbx.h"
#include "chapp.h"
#include "arith.h"
#include "dclmap.h"
#include "misc.h"
#include "smisc.h"
#include "set.h"
#include "libf.h"
#include "lib.h"
#include "libr.h"
#include "adafront.h"

#ifdef MAC_GWADA
#include "AdaFileTypes.h"
#include "memman.h"
#endif


static void fab_getudecl(fabIFILE *ifile, short ui);
static Node fab_getnodref(fabIFILE *ifile);
static Declaredmap fab_getdcl(fabIFILE *ifile);
static void fab_getnod(fabIFILE *ifile, Node node, short unum);
static void fab_getovl(fabIFILE *ifile, Symbol sym);
static void fab_getsig(fabIFILE *ifile, Symbol sym, Boolean is_private);
static char *fab_getmisc(fabIFILE *ifile, Symbol sym, short mval);
static void fab_getrepr(fabIFILE *ifile, Symbol sym);
static void fab_getnval(fabIFILE *ifile, Node node);
static void fab_getlitmap(fabIFILE *ifile, Symbol sym);
static int *fab_getuint(fabIFILE *ifile);

char *fab_read_ais(char *fname, char *uname, int comp_index, Boolean tree_is_needed, Boolean is_aic_file)
{
/* read aic or axq for unit with name uname from file fname.
 * is_aic_file indicates whether we are reading from an aic or axq file.
 * if uname is the null pointer, read 'comp_index'th unit from the file.
 * return TRUE if read ok, FALSE if not. tree_is_needed is a flag to
 * indicate whether retrieve_tree_nodes needs to be called. Is is always
 * TRUE for the semantic phase and when called by the expander but is
 * FALSE when called by BIND in the code generator.
 */

long	rec, genoff;
int		indx, n, nodes, symbols, i;
Tuple	symptr, tup, nodes_group;
Set		set;
struct unit *pUnit;
char	*funame, *retrieved ;
Unitdecl	ud;
fabIFILE	*ifile;
char    *lname, *tname, *full_fname;
/* Read information from the current compilation to
 * 'file', restructuring the separate compilation maps
 * to improve the readability of the AIS code.
 */
short	fnum, unum;
Boolean	is_main_unit;
Boolean	is_predef; /* set when reading predef file */

retrieved = NULL;
indx = 0;
is_predef = streq(fname, "0") && strlen(PREDEFNAME);
if (is_predef) {
	/* reading predef, but not compiling it ! */
	lname = libset(PREDEFNAME);
	full_fname = "predef" ;
}
else {
	full_fname = fname;
}
if (is_aic_file) {
	/* Open it in binary mode */
	FileType('AAIC');
	ifile = fab_ifopenread(full_fname, "aic", 0);
}
else {
	FileType('AAXQ');
	ifile = fab_ifopenread(full_fname, "axq", 0);
}

if (is_predef)
	tname = libset(lname); /* restore library name after predef read */
for (rec = fab_read_init(ifile); rec != 0; rec = fab_read_next(ifile, rec)) {
	indx++;

	funame = fab_getstr(ifile/*, "unit-name"*/);
	if (uname == NULL && indx != comp_index)
		continue;

	if (uname != NULL  && streq(uname, funame) == 0)
		continue;

	fnum = fab_getnum(ifile/*, "unit-number"*/);
	unum = unit_number(funame);
	if (unum != fnum)
		chaos(__FILE__, __LINE__, "read_ais sequence number error");

	pUnit = pUnits[unum];
	genoff = fab_getlong(ifile/*, "code-gen-offset"*/);
	is_main_unit = streq(unit_name_type(funame), "ma");

	if (!is_main_unit) { /* read only if NOT main unit (it has no ais info*/

		symbols = fab_getnum(ifile/*, "seq-symbol-n"*/);
		nodes = fab_getnum(ifile/*, "seq-node-n"*/);
		/* install tre node info and symbol count in the case where the
		*  generator reads semantic aisfile and therefore bypasses
		*  read_lib where the info is normally installed.
 		*/

		if (is_aic_file) {
			pUnit->treInfo.nodeCount = nodes;
			pUnit->treInfo.tableAllocated = (char *) tup_new(nodes);
			pUnit->aisInfo.numberSymbols = symbols;

			/* May be old value of aistup[7] may be freed at this point
			 *  of this is recompilation of unit within the last compilation.
 			 */

			pUnit->aisInfo.symbols = (char *) tup_new(symbols);
			pUnit->libInfo.fname = AISFILENAME;
			pUnit->libInfo.obsolete = string_ok;
		}

		symptr = (Tuple) pUnit->aisInfo.symbols;
		if (symptr == (Tuple)0) { /* if tuple not yet allocated */
			symptr = tup_new(symbols);
			pUnit->aisInfo.symbols = (char *) symptr;
		}

		/* ELABORATE PRAGMA INFO */
		n = fab_getnum(ifile/*, "pragma-info-size"*/);
		tup = tup_new(n);
		for (i = 1; i <= n; i++)
			tup[i] = fab_getstr(ifile/*, "pragma-info-value"*/);

		pUnit->aisInfo.pragmaElab = (char *) tup;

		/* UNIT_DECL */
		fab_getudecl(ifile, unum);

		/* PRE_COMP */
		n = fab_getnum(ifile/*, "precomp-size"*/);
		set = (Set) set_new(n);
		for (i = 1; i <= n; i++)
			set = set_with(set, (char *) fab_getnum(ifile/*, "precomp-value"*/));

		pUnit->aisInfo.preComp = (char *) set;

		/* tuple of symbol table pointers */
		aisunits_read = tup_with(aisunits_read, funame);
	}
	retrieved = funame;
	break;
}

if (tree_is_needed && retrieved) {

	ud = (Unitdecl) pUnit->aisInfo.unitDecl;
	tup = (Tuple) ud->ud_nodes;
	n = tup_size(tup);
	nodes_group = tup_new(n);
	for (i = 1; i <= n; i++)
		nodes_group[i] = (char *) N_SEQ((Node)tup[i]);
	fab_retrieve_tree_nodes(ifile, nodes_group, unum);
}
fab_ifclose(ifile);
return retrieved;
}

static void fab_getudecl(fabIFILE *ifile, short ui)
{
Tuple	tup, cent, ctup, cntup, symtup;
Symbol	usym;
Unitdecl	ud;
int	i, n, ci, cn;

ud = unit_decl_new();
pUnits[ui]->aisInfo.unitDecl = (char *) ud;

/* The second entry is the sequence of the symbol table entry
 * identifying the unit. We use this sequence number to find
 * the actual entry alread allocated.
 */

usym = fab_getsym(ifile/*, "ud-unam"*/);
ud->ud_unam = usym;
ud->ud_useq = S_SEQ(usym);
ud->ud_unit = S_UNIT(usym);
fab_get_unit_unam(ifile/*, usym*/);

/* context */
n = fab_getnum(ifile/*, "decl-context-size"*/);
if (n > 0) {
	n -= 1; /* true tuple size */
	ctup = tup_new(n);

	for (i = 1; i <= n; i++) {
		cent = (Tuple) tup_new(2);
		cent[1] = (char *) fab_getnum(ifile/*, "decl-ctup-1"*/);
		cn = fab_getnum(ifile/*, "decl-cntup-size"*/); 
		cntup = tup_new(cn);

		for (ci = 1; ci <= cn; ci++)
			cntup[ci] = fab_getstr(ifile/*, "decl-tupstr-str"*/);
		cent[2] = (char *) cntup;
		ctup[i] = (char *) cent;
		}

	ud->ud_context =  ctup;
}

/* unit_nodes */
n = fab_getnum(ifile/*, "decl-ud-nodes-size"*/);
tup = tup_new(n);

for (i = 1; i <= n; i++)
	tup[i] = (char *) fab_getnodref(ifile/*, "decl-nodref"*/);

ud->ud_nodes = tup;

/* tuple of symbol table pointers */
n = fab_getnum(ifile/*, "decl-tuple-size"*/);

if (n > 0) {
	n -= 1; /* true tuple size */
	tup = tup_new(n);
	for (i = 1; i <= n; i++)
		tup[i] = (char *) fab_getsym(ifile/*, "decl-symref"*/);

	ud->ud_symbols = tup;
	}

/* decscopes - tuple of scopes */
n = fab_getnum(ifile/*, "decl-descopes-tuple-size"*/);
if (n > 0) {
	n -= 1; /* true tuple size */
	symtup = tup_new(n);
	for (i = 1; i <= n; i++) {
		symtup[i] = (char *) fab_getsym(ifile/*, "decl-decscopes-symref"*/);
	}
	ud->ud_decscopes =	symtup;
}

/* decmaps - tuple of declared maps */
n = fab_getnum(ifile/*, "decmaps-tuple-size"*/);
if (n > 0) {
	n -= 1; /* true tuple size */
	tup = tup_new(n);
	for (i = 1; i <= n; i++) {
#ifdef TBSN
		-- use decl maps read in with symbols	ds 21 dec 
		-- but read in anyway for completeness
#endif
	    tup[i] = (char *) fab_getdcl(ifile);
		tup[i] = (char *) DECLARED((Symbol)symtup[i]);
	}
	ud->ud_decmaps = tup;
}
/* oldvis - tuple of unit names */
n = fab_getnum(ifile/*, "vis"*/);
if (n > 0) {
	n -= 1; /* true tuple size */
	tup = tup_new(n);
	for (i = 1; i <= n; i++) {
		tup[i] = fab_getstr(ifile/*, "vis-str"*/);
	}
	ud->ud_oldvis = tup;
}
return;
}

void fab_retrieve_tree_nodes(fabIFILE *ifile, Tuple nodes_list, short node_unit)
{
long	rec, *off;
char  	*fname;
char    *tfname;
Node	fn, nd;
//Fortup	ft1;
char    *lname, *tname;
short	unum, items;
short	node_seq, nkind;

/* read tree file for unit with unit number "node_unit" and load only
 * the nodes in nodes_list.
 */

fname = lib_unit_get(pUnits[node_unit]->name);
if (streq(fname, "0") && !streq(PREDEFNAME, "")) {
	/* reading predef, but not compiling it ! */
	lname = libset(PREDEFNAME);
	tfname = "predef";
}
else {
	tfname = fname;
}
ifile = fab_ifopenread(tfname, "trc", 0);
if (streq(fname, "0") && !streq(PREDEFNAME, ""))
	tname= libset(lname); /* restore library name */

for (rec = fab_read_init(ifile); rec != 0; rec = fab_read_next(ifile, rec)) {
	(void) fab_getstr(ifile/*, "unit_name"*/); /* skip over unit name */
	unum = fab_getnum(ifile/*, "unit-number"*/);
	if (unum != node_unit) continue;
	items = fab_getnum(ifile/*, "node-count"*/);
	off = (long *) fcalloct((items+1)* sizeof(long), "read-tree");
	fab_fread((char *) off, sizeof(long) * (items+1), ifile->fh_file);
	break;
	}
while (tup_size(nodes_list)) {
	node_seq = (short)tup_frome(nodes_list);
	fab_ifseek(ifile, off[node_seq]);
	fn = getnodptr(node_seq, node_unit);
	fab_getnod(ifile/*, "unit-node"*/, fn, unum);

	nkind = N_KIND(fn);
	if (N_AST1_DEFINED(nkind) && N_AST1(fn) != (Node)0)
		nodes_list = add_tree_node(nodes_list, N_AST1(fn));
	if (N_AST2_DEFINED(nkind) && N_AST2(fn) != (Node)0)
		nodes_list = add_tree_node(nodes_list, N_AST2(fn));
	if (N_AST3_DEFINED(nkind) && N_AST3(fn) != (Node)0)
		nodes_list = add_tree_node(nodes_list, N_AST3(fn));
	if (N_AST4_DEFINED(nkind) && N_AST4(fn) != (Node)0)
		nodes_list = add_tree_node(nodes_list, N_AST4(fn));

	if (N_LIST_DEFINED(N_KIND(fn)) && N_LIST(fn) != (Tuple)0) {
		FORTUP(nd=(Node), N_LIST(fn), ft1);
		nodes_list = add_tree_node(nodes_list, nd);
		ENDFORTUP(ft1);
	}
}
tup_free((Tuple) off);
tup_free(nodes_list);
fab_ifclose(ifile);
}

Symbol fab_getsym(fabIFILE *ifile)
{
struct f_symbol_s fs;
Symbol	sym, tmp_sym;
short	i, nat;
Boolean	is_private;

/* read description for symbol sym to input file */
fab_fread((char *) &fs, sizeof(f_symbol_s), ifile->fh_file);

sym = getsymptr(fs.f_symbol_seq, fs.f_symbol_unit);
nat = fs.f_symbol_nature;
NATURE(sym) = nat;
S_SEQ(sym) = fs.f_symbol_seq;
S_UNIT(sym) = fs.f_symbol_unit;

#ifdef DEBUG
if (trapss>0 && trapss == fs.f_symbol_seq 
    && trapsu == fs.f_symbol_unit) traps(sym);
#endif
TYPE_OF(sym) = getsymptr(fs.f_symbol_type_of_seq,
    fs.f_symbol_type_of_unit);
SCOPE_OF(sym) = getsymptr(fs.f_symbol_scope_of_seq,
    fs.f_symbol_scope_of_unit);
ALIAS(sym) = getsymptr(fs.f_symbol_alias_seq,
    fs.f_symbol_alias_unit);
if (fs.f_symbol_type_attr & TA_ISPRIVATE) {
	is_private = TRUE;
	fs.f_symbol_type_attr ^= TA_ISPRIVATE; /* turn off ISPRIVATE bit*/
}
else {
	is_private = FALSE;
}
TYPE_ATTR(sym) = fs.f_symbol_type_attr;
ORIG_NAME(sym) = fab_getstr(ifile/*, "orig-name"*/);
/* process overloads separately due to variety of cases */
if (fs.f_symbol_overloads) fab_getovl(ifile, sym);

/* read out declared map, treating na_enum case separately */
if (fs.f_symbol_declared) DECLARED(sym)= fab_getdcl(ifile);

/* signature */
if (fs.f_symbol_signature) {
	fab_getsig(ifile, sym, is_private);
	}

MISC(sym) = fab_getmisc(ifile, sym, fs.f_symbol_misc);

/* the following fields are extracted for the code generator use only */
if (TYPE_KIND(sym) == 0) TYPE_KIND(sym) = fs.f_symbol_type_kind;
if (TYPE_SIZE(sym) == 0) TYPE_SIZE(sym) = fs.f_symbol_type_size;
if (is_type(sym))
	INIT_PROC(sym) = getsymptr(fs.f_symbol_init_proc_seq,
	  fs.f_symbol_init_proc_unit);
else 		 /* formal_decl_tree for subprogram specs */
	INIT_PROC(sym) = (Symbol) getnodptr(fs.f_symbol_init_proc_seq,
	  fs.f_symbol_init_proc_unit);
if (ASSOCIATED_SYMBOLS(sym)) {
	for (i = 1; i < fs.f_symbol_assoc_list; i++) {
		tmp_sym = (Symbol) fab_getsymref(ifile/*, "assoc-symbol-symref"*/);
		if (tmp_sym)
			ASSOCIATED_SYMBOLS(sym)[i] = (char *) tmp_sym;
	}
}
else {
	if (fs.f_symbol_assoc_list == 0)
		ASSOCIATED_SYMBOLS(sym) = NULL;
	else 
		ASSOCIATED_SYMBOLS(sym) = tup_new(fs.f_symbol_assoc_list -1);
	if (fs.f_symbol_assoc_list > 1) {
		for (i = 1; i < fs.f_symbol_assoc_list; i++)
			ASSOCIATED_SYMBOLS(sym)[i] =
			  (char *) fab_getsymref(ifile/*, "assoc-symbol-symref"*/);
	}
}
fab_getrepr(ifile, sym);
if (S_SEGMENT(sym) == -1) S_SEGMENT(sym) = fs.f_symbol_s_segment;
if (S_OFFSET(sym) == 0)   S_OFFSET(sym) = fs.f_symbol_s_offset;
return sym;
}

void fab_get_unit_unam(fabIFILE *ifile/*, Symbol sym*/)
/*  
 * reads the full symbol definitions of the associated symbol field of the
 * unit name symbol. This is needed since when binding is done we want to
 * load the symbols from this field which represent the procedures to 
 * elaborate packages.
 */
{
short	i;

for (i = 2; i >= 0; i--)
	(void)fab_getsym(ifile/*, "ud-assoc-sym"*/);
}

Node fab_getnodref(fabIFILE *ifile)
{
Node	node;
short	seq, unit;

/* 
 * OPT_NODE is node in unit 0 with sequence 1, and needs
 * no special handling here
 */
seq = fab_getnum(ifile/*, "nref-seq"*/);
unit = fab_getnum(ifile/*, "nref-unt"*/);

node = (seq == 1 && unit == 0) ? OPT_NODE : getnodptr(seq, unit);
/*if (seq == 1 && unit == 0) {
	node = OPT_NODE;
	}
else {
	node = getnodptr(seq, unit);*/

#ifdef DEBUG
	if (trapns>0 && trapns == seq && trapnu == unit) trapn(node);
#endif

//	}
return node;
}

Declaredmap fab_getdcl(fabIFILE *ifile)
{
Declaredmap d = NULL;
char	*id;
Symbol	sym;
short	n;

if (fab_getnum(ifile/*, "dcl_is_map_defined"*/)) {
	n = fab_getnum(ifile/*, "dcl-number-defined*/); /* get item count */
	d = dcl_new(n);
	if (n) {
		do {
			id = fab_getstr(ifile/*, "sym-str"*/);
			sym = fab_getsymref(ifile);
			dcl_put_vis(d, id, sym, fab_getnum(ifile/*, "sym-vis"*/));
			}
		while (--n);
		}
	}
return d;
}

static void fab_getnod(fabIFILE *ifile, Node node, short unum)
{
/* 
 * Read information for the node from a file (ifile)
 * Since all the nodes in the tree all have the same N_UNIT value, 
 * the node can be read from the file in a more compact format.
 * The N_UNIT of the node itself and of its children (N_AST1...) need not
 * be read only their N_SEQ filed needs to be read. There is one 
 * complication of this scheme. OPT_NODE which is (seq=1, unit=0) will
 * conflict with (seq=1,unit=X)  of current unit. Therefore, in this case a 
 * sequence # of -1 will signify OPT_NODE.
 */
Tuple	ltup;
short	i;
short	nk, num1, num2, has_n_list;
short	fnum[24], fnums, fnumr = 0;

/* copy standard info */
fnums = fab_getnum(ifile/*, desc*/);
/*fread((char *) &fnums, sizeof(short), 1, ifile->fh_file);*/
if (fnums == 0) {
	chaos(__FILE__, __LINE__, "getnod-fnums-zero");
	}
fab_fread((char *) fnum,  sizeof(short) * fnums, ifile->fh_file);
fnumr = 0;
nk = fnum[fnumr++];
N_KIND(node) = nk;
N_SEQ(node) = fnum[fnumr++];
N_UNIT(node) = unum;
#ifdef DEBUG
if (trapns>0 && N_SEQ(node)== trapns && N_UNIT(node) == trapnu) trapn(node);
#endif

N_SPAN0(node) = N_SPAN1(node) = 0;

if (N_LIST_DEFINED(nk)) {
	has_n_list = fnum[fnumr++];
	ltup = (has_n_list) ? tup_new(has_n_list - 1) : (Tuple) 0;
}
else {
	has_n_list = 0;
}
/* ast fields */
/* See comment above for description of compact format of node */
N_AST1(node) = N_AST2(node) = N_AST3(node) = N_AST4(node) = (Node)0;
if (N_AST1_DEFINED(nk)) {
	num1 = fnum[fnumr++];
	N_AST1(node) = (num1 == -1) ? OPT_NODE : getnodptr(num1, unum);
}
if (N_AST2_DEFINED(nk)) {
	num1 = fnum[fnumr++];
	N_AST2(node) = (num1 == -1) ? OPT_NODE : getnodptr(num1, unum);
}
if (N_AST3_DEFINED(nk)) {
	num1 = fnum[fnumr++];
	N_AST3(node) = (num1 == -1) ? OPT_NODE : getnodptr(num1, unum);
}
if (N_AST4_DEFINED(nk)) {
	num1 = fnum[fnumr++];
	N_AST4(node) = (num1 == -1) ? OPT_NODE : getnodptr(num1, unum);
}

if (N_UNQ_DEFINED(nk)) {
	num1 = fnum[fnumr++]; 
	num2 = fnum[fnumr++];
	if (num1>0 || num2>0)
		N_UNQ(node) = getsymptr(num1, num2);
}
if (N_TYPE_DEFINED(nk)) {
	num1 = fnum[fnumr++]; 
	num2 = fnum[fnumr++];
	if (num1>0 || num2>0) {
		N_TYPE(node) = getsymptr(num1, num2);
	}
}

/* read out n_list if needed */
if (has_n_list > 0) {
	for (i = 1; i < has_n_list; i++) {
		ltup[i] = (char *) fab_getnodref(ifile/*, "n-list-nodref"*/);
	}
	if (ltup != (Tuple)0) {
		N_LIST(node) = ltup;
	}
}
if (N_VAL_DEFINED(nk))
	fab_getnval(ifile, node);
}

static void fab_getovl(fabIFILE *ifile, Symbol sym)
{
Set		ovl;
Private_declarations	pd;
Tuple	tup;
int	n, i;
short	nat;

nat = NATURE(sym);
ovl = (Set) 0;
/* 
 * It is the private declarations for na_package and na_package_spec,
 * and na_generic_package_spec.
 * Otherwise it is a set of symbols:
 *	na_aggregate  na_entry	na_function  na_function_spec
 *	na_literal  na_op  na_procedure	 na_procedure_spec
 * It is literal map for enumeration type (na_enum).
 */
if(nat == na_enum) {
	fab_getlitmap(ifile, sym);
	return;
	}
else if (nat == na_package || nat == na_package_spec
  || nat == na_generic_package_spec || nat == na_generic_package
  || nat == na_task_type || nat == na_task_obj) {
	/* read in private declarations (rebuild tuple) */
	n = fab_getnum(ifile/*, "ovl-private-decls-size"*/);
	pd = private_decls_new(n);
	tup = tup_new(n+n);
// written the loop below in a clearer form [Fabrizio Oddone]
//	for (i = 1; i <= n; i++) {
//		tup[2*i-1] =  (char *) fab_getsym(ifile/*, "ovl-pdecl-1-sym"*/);
//		tup[2*i] =  (char *) fab_getsym(ifile/*, "ovl-pdecl-2-sym"*/);
//		}
	for (i = 1; i <= (n+n); i++)
		tup[i] = (char *) fab_getsym(ifile/*, "ovl-pdecl-sym"*/);

	pd->private_declarations_tuple = tup;
	ovl = (Set) pd;
	}
else {	 /* if (ovl != (Set)0) */
	/* this is condition for write, but for read, we call this routine */
 	/* iff overloads field is defined	 (gs Nov 9 ) */
	n = fab_getnum(ifile/*, "ovl-set-size"*/);
	ovl = set_new(n);

	for (i = 1; i <= n; i++)
		ovl = set_with(ovl, (char *) fab_getsymref(ifile/*, "ovl-set-symref"*/));
}
if (nat != na_package || SCOPE_OF(sym) != symbol_standard0)
	/* otherwise the private dcls are inherited from the spec.*/
	OVERLOADS(sym) = ovl;
}

static void fab_getsig(fabIFILE *ifile, Symbol sym, Boolean is_private)
{
Tuple	sig, tup, sigtup;
Node	node;
int	i, n;
short	nat;
//	Symbol	s, s2;

/* The signature field is used as follows:
 * It is a symbol for:
 *	na_access
 * It is a node for
 *	na_constant  na_in  na_inout
 * It is also a node (always OPT_NODE) for na_out. For now we read this
 * out even though it is not used. 
 * It is a pair for na_array.
 * It is a triple for na_enum.
 * It is a triple for na_generic_function_spec na_generic_procedure_spec
 * The first component is a tuple of pairs, each pair consisting of
 * a symbol and a (default) node.
 * The second component is a tuple of symbols.
 * The third component is a node.
 * It is a tuple with four elements for na_generic_package_spec:
 * the first is a tuple of pairs, with same for as for generic procedure.
 * the second third,and fourth components are nodes.
 *	(see libw.c for format)
 * It is a 5-tuple for na_record.
 * It is a constraint for na_subtype and na_type.
 * It is a node for na_obj.
 * It is a tuple of nodes for na_task_type, na_task_type_spec
 * Otherwise it is the signature for a procedure, namely a tuple
 * of quadruples.
 * In the expand tasks are converted to procedures so their signature is
 * like that of procs.
 */

nat = NATURE(sym);
/* is_private indicates signature has form of that for record */
if (is_private)
	nat = na_record;

switch (nat) {
  case	na_access:
			/* access: signature is designated_type;*/
			sig = (Tuple) fab_getsymref(ifile/*, "sig-access-symref"*/);
			break;
  case	na_array:
array_case:
			/* array: signature is pair [i_types, comp_type] where
 			 * i_types is tuple of type names
 			 */
			sig = tup_new(2);
			n = fab_getnum(ifile/*, "sig-array-itypes-size"*/);
			tup = tup_new(n);
			for (i = 1; i <= n; i++)
				tup[i] = (char *)fab_getsymref(ifile/*, "sig-array-i-types-type"*/);
			sig[1] = (char *) tup;
			sig[2] = (char *) fab_getsymref(ifile/*, "sig-array-comp-type"*/);
			break;
  case	na_block:
			/* block: miscellaneous information */
			/* This information not needed externally*/
			chaos(__FILE__, __LINE__, "getsig: signature for block");
			break;
  case	na_constant:
  case	na_in:
  case	na_inout:
  case	na_out:
  case	na_discriminant:
			sig = (Tuple) fab_getnodref(ifile/*, "sig-discriminant-nodref"*/);
			break;
  case	na_entry:
  case	na_entry_family:
  case	na_entry_former:
  /* entry: list of symbols */
  case	na_function:
  case	na_function_spec:
  case	na_literal:
  case	na_op:
  case	na_procedure:
  case	na_procedure_spec:
  case	na_task_body:
			n = fab_getnum(ifile/*, "sig-tuple-size"*/);
			sig = tup_new(n);
			for (i = 1; i <= n; i++)
				sig[i] = (char *) fab_getsymref(ifile/*, "sig-tuple-symref"*/);
			break;
  case	na_enum:
			/* enum: tuple in form ['range', lo, hi]*/
			/* we read this as two node references*/
			sig = tup_new(3);
			/*sig[1] = ???;*/
			sig[2] = (char *) fab_getnodref(ifile/*, "sig-enum-low-nodref"*/);
			sig[3] = (char *) fab_getnodref(ifile/*, "sig-enum-high-nodref"*/);
			break;
  case	na_type:
#ifdef TBSL
			s  = TYPE_OF(sym);
			s2 = TYPE_OF(root_type(sym));
			if ((s != (Symbol)0 && NATURE(s) == na_access) || 
	    		(s2 != (Symbol)0 && NATURE(s2) == na_access))  {
				fab_getsymref(ifile/*, "sig-access-symref"*/);
				break;
				}
#endif
                    	i = fab_getnum(ifile/*, "sig-type-is-access"*/);
                    	if (i == 1) break; 
			/* for private types, is_private will be true, and
			*  signature is that of record 
 			*/
			n = fab_getnum(ifile/*, "sig-type-size"*/);
			i = fab_getnum(ifile/*, "sig-constraint-kind"*/);
			sig = tup_new(n);
			sig[1] = (char *) i;
			for (i=2; i <= n; i++)
				sig[i] = (char *) fab_getnodref(ifile/*, "sig-type-nodref"*/);
			break;
  case na_subtype:
			n = fab_getnum(ifile/*, "sig-subtype-size"*/);
			i = fab_getnum(ifile/*, "sig-constraint-kind"*/);
			if (i == CONSTRAINT_ARRAY) goto array_case;
			sig = tup_new(n);
			sig[1] = (char *) i;
			if (i == CONSTRAINT_DISCR) {
				/* discriminant map */
				n = fab_getnum(ifile/*, "sig-constraint-discrmap-size"*/);
				tup = tup_new(n);
				for (i = 1; i <= n; i+=2) {
					tup[i] = (char *)fab_getsymref(ifile/*, "sig-constraint-discr-map-symref"*/);
					tup[i+1] = (char *)fab_getnodref(ifile/*, "sig-constraint-discr-map-nodref"*/);
				}
				sig[2] = (char *) tup;
			}
			else if (i == CONSTRAINT_ACCESS) {
				sig[2] = (char *)fab_getsymref(ifile/*, "sig-subtype-acc-symref"*/);
			}
			else {
				for (i=2; i <= n; i++)
					sig[i] = (char *)fab_getnodref(ifile/*, "sig-subtype-nodref"*/);
			}
			break;
  case	na_generic_function:
  case	na_generic_procedure:
  case	na_generic_function_spec:
  case	na_generic_procedure_spec:
			sig = tup_new(4);
			if (tup_size(sig) != 4) chaos(__FILE__, __LINE__, 
				"getsig: bad signature for na_generic_procedure_spec");
			/* tuple count known to be four, just put elements */
			/* the first component is a tuple of pairs, just read count
 			* and the values of the successive pairs 
 			*/
			n = fab_getnum(ifile/*, "sig-generic-size"*/);
			sigtup = tup_new(n);
			for (i = 1;i <= n; i++) {
				tup = tup_new(2);
				tup[1] = (char *) fab_getsymref(ifile/*, "sig-generic-symref"*/);
				tup[2] = (char *) fab_getnodref(ifile/*, "sig-generic-nodref"*/);
				sigtup[i] = (char *) tup;
			}
			sig[1] = (char *) sigtup;
			n = fab_getnum(ifile/*, "sig-generic-typ-size"*/); /* symbol list */
			tup = tup_new(n);
			for (i = 1;i <= n; i++)
				tup[i] = (char *) fab_getsymref(ifile/*, "sig-generic-symbol-symref"*/);
			sig[2] = (char *) tup;
			node = fab_getnodref(ifile/*, "sig-generic-3-nodref"*/);
			if (nat == na_generic_procedure || nat == na_generic_function)
				sig[3] = (char *) node;
			else sig[3] = (char *) OPT_NODE;
			/* the four component is tuple of must_constrain symbols */
			n = fab_getnum(ifile/*, "sig-generic-package-tupsize"*/);
			tup = tup_new(n);
			for (i = 1;i <= n; i++)
				tup[i] = (char *) fab_getsymref(ifile/*, "sig-generic-package-symref"*/);
			sig[4] = (char *) tup;
			break;
  case	na_generic_package_spec:
  case	na_generic_package:
			/* signature is tuple with four elements */
			sig = tup_new(5);
			/* the first component is a tuple of pairs, just write count
 			* and the values of the successive pairs 
 			*/
			n = fab_getnum(ifile/*, "sig-generic-package-tupsize"*/);
			tup = tup_new(n);
			for (i = 1;i <= n; i++) {
				sigtup = tup_new(2);
				sigtup[1] = (char *) fab_getsymref(ifile/*, "sig-generic-package-symref"*/);
				sigtup[2] = (char *) fab_getnodref(ifile/*, "sig-generic-package-nodref"*/);
				tup[i] = (char *) sigtup;
			}
			sig[1] = (char *) tup;
			/* the second third, and fourth components are just nodes */
			sig[2] = (char *) fab_getnodref(ifile/*, "sig-generic-node-2"*/);
			sig[3] = (char *) fab_getnodref(ifile/*, "sig-generic-node-3"*/);
			sig[4] = (char *) fab_getnodref(ifile/*, "sig-generic-node-4"*/);
			/* the fifth component is tuple of must_constrain symbols */
			n = fab_getnum(ifile/*, "sig-generic-package-tupsize"*/);
			tup = tup_new(n);
			for (i = 1;i <= n; i++)
				tup[i] = (char *) fab_getsymref(ifile/*, "sig-generic-package-symref"*/);
			sig[5] = (char *) tup;
			break;
  case	na_record:
			/* the signature is tuple with five components:
	 		* [node, node, tuple of symbols, declaredmap, node]
	 		* NOTE: we do not read component count - 5 assumed 
	 		*/
			sig = tup_new(5);
			sig[1] = (char *) fab_getnodref(ifile/*, "sig-record-1-nodref"*/);
			sig[2] = (char *) fab_getnodref(ifile/*, "sig-record-2-nodref*/);
			n = fab_getnum(ifile/*, "sig-record-3-size"*/);
			tup = tup_new(n);
			for (i = 1; i <= n; i++)
				tup[i] = (char *) fab_getsymref(ifile/*, "sig-record-3-nodref"*/);
			sig[3]= (char *) tup;
			sig[4] = (char *) fab_getdcl(ifile);
			sig[5] = (char *) fab_getnodref(ifile/*, "sig-record-5-nodref"*/);
			break;
  case	na_void:
			/* special case assume entry for $used, in which case is tuple
	 		* of symbols
	 		*/
			if (streq(ORIG_NAME(sym), "$used") ) {
				n = fab_getnum(ifile/*, "sig-$used-size"*/);
				sig = tup_new(n);
				for (i = 1; i <= n; i++)
					sig[i] = (char *) fab_getsymref(ifile/*, "sig-$used-symref"*/);
			}
			else {
#ifdef DEBUG
				zpsym(sym);
#endif
				chaos(__FILE__, __LINE__, "getsig: na_void, not $used");
			}
			break;
  case	na_obj:
			sig = (Tuple) fab_getnodref(ifile/*, "sig-obj-nodref"*/);
			break;
  case	na_task_type:
  case	na_task_type_spec:
			/* a tuple of nodes */
			n = fab_getnum(ifile/*, "task-type-spec-size"*/);
			sig = tup_new(n);
			for (i = 1; i <= n; i++)
				sig[i] = (char *)fab_getnodref(ifile/*, "sig-task-nodref"*/);
			break;
default:
#ifdef DEBUG
			printf("getsig: default error\n");
			zpsym(sym);
#endif
			chaos(__FILE__, __LINE__, "getsig: default");
} /* End of switch */
SIGNATURE(sym) = sig;
}

static char *fab_getmisc(fabIFILE *ifile, Symbol sym, short mval)
{
/* read MISC information if present 
* MISC is integer except for package, in which case it is a triple.
* The first two components are integers, the last is  a tuple of
* symbols
*/
Tuple  tup, stup;
int	i, n;
short	nat;

nat = NATURE(sym);
if ((nat == na_package || nat == na_package_spec)) {
	if (mval) {
		tup = tup_new(3);
		tup[1] = (char *) fab_getnum(ifile/*, "misc-package-1"*/);
		tup[2] = (char *) fab_getnum(ifile/*, "misc-package-2"*/);
		n = fab_getnum(ifile/*, "misc-package-tupsize"*/);
		stup = tup_new(n);
		for (i = 1; i<= n; i++)
			stup[i] = (char *) fab_getsymref(ifile/*, "misc-package-symref"*/);
		tup[3] = (char *) stup;
		return (char *) tup;
	}
	else {
		fab_getnum(ifile/*, "misc"*/);
		return  (char *)MISC(sym);
	}
}
else if ((nat == na_procedure || nat == na_function) && mval) {
	tup = tup_new(2);
	tup[1] = (char *) fab_getnum(ifile/*, "misc-number"*/);
	tup[2] = (char *) fab_getsymref(ifile/*, "misc-symref"*/);
	return (char *) tup;
}
else
	return  (char *)fab_getnum(ifile/*, "misc"*/);
}

static void fab_getrepr(fabIFILE *ifile, Symbol sym)
{
/* read int representation information if present */

Tuple 	align_mod_tup,align_tup,repr_tup;
Tuple 	tup4;
int 	i, n;
int	repr_tag;

repr_tag = fab_getnum(ifile/*, "repr-type"*/);
if (repr_tag != -1) {
    	if (repr_tag == TAG_RECORD) 	{ /* record type */
			repr_tup = tup_new(4);
			repr_tup[1] = (char *) TAG_RECORD;
       		repr_tup[2] = (char *) fab_getnum(ifile/*,"repr-rec-size"*/);
        	align_mod_tup = tup_new(2);
        	align_mod_tup[1] = (char *) fab_getnum(ifile/*,"repr-rec-mod"*/);
        	n = fab_getnum(ifile/*,"repr-align_tup_size"*/);
			align_tup = tup_new(0);
        	for (i=1; i<=n; i++) {
			    tup4 = tup_new(4);
				tup4[1] = (char *) fab_getsymref(ifile/*,"repr-rec-align-1"*/);
            	tup4[2] = (char *) fab_getnum(ifile/*,"repr-rec-align-2"*/);
            	tup4[3] = (char *) fab_getnum(ifile/*,"repr-rec-align-3"*/);
            	tup4[4] = (char *) fab_getnum(ifile/*,"repr-rec-align-4"*/);
				align_tup = tup_with(align_tup, (char *) tup4);
			}
			align_mod_tup[2] = (char *) align_tup;
       		repr_tup[4] = (char *) align_mod_tup;
			REPR(sym) = repr_tup;
    	}
		else if (repr_tag == TAG_ACCESS || 
				 repr_tag == TAG_TASK) { /* access or task type */
			repr_tup = tup_new(3);
			repr_tup[1] = (char *) repr_tag;
			repr_tup[2] = (char *) fab_getnum(ifile/*, "repr-size-2"*/);
        	repr_tup[3] = (char *) fab_getnodref(ifile/*, "repr-storage-size"*/);
			REPR(sym) = repr_tup;
		}
    	else { 		/* non-record, non-access, non-task type */
        	n = fab_getnum(ifile/*, "repr-tup-size"*/);
			repr_tup = tup_new(n);
			repr_tup[1] = (char *) repr_tag;
        	for (i=2; i <= n; i++)
            	repr_tup[i] = (char *) fab_getnum(ifile/*, "repr-info"*/);
			REPR(sym) = repr_tup;
		}
	}
}

static void fab_getnval(fabIFILE *ifile, Node node)
{
/* read N_VAL field for node to AISFILE */
Const	con;
char	*nv;
Tuple	tup;
int		i, n, *rn, *rd;
//double	doub;
Symbolmap   smap;
Symbol	s1, s2;
int	nk, ck;

nv = NULL;       /* gs nov 1: added to avoid setting N_VAL incorrectly
					at end of this routine */
switch (nk = N_KIND(node)) {
  case	as_simple_name:
  case	as_int_literal:
  case	as_real_literal:
  case	as_string_literal:
  case	as_character_literal:
  case	as_subprogram_stub_tr:
  case	as_package_stub:
  case	as_task_stub:
			nv = (char *) fab_getstr(ifile/*, "nval-name"*/);
			break;
  case	as_line_no:
  case	as_number:
  case	as_predef:
			nv = (char *) fab_getnum(ifile/*, "nval-int"*/);
			break;
  case	as_mode:
			/* convert mode, indeed, the inverse of change made in astread*/
			nv = (char *) fab_getnum(ifile/*, "nval-mode"*/);
			break;
  case	as_ivalue:
			ck = fab_getnum(ifile/*, "nval-const-kind"*/);
			con = const_new(ck);
			nv = (char *) con;
			switch (ck) {
			  case	CONST_INT:
				con->const_value.const_int =
				  fab_getint(ifile/*, "nval-const-int-value"*/);
				break;
			  case	CONST_REAL:
				fab_fread((char *) &con->const_value.const_real, sizeof(double), ifile->fh_file);
				break;
			  case	CONST_UINT:
				con->const_value.const_uint =
				  fab_getuint(ifile/*, "nval-const-uint"*/);
				break;
			  case	CONST_OM:
				break; /* no further data needed if OM */
			  case	CONST_RAT:
				rn = fab_getuint(ifile/*, "nval-const-rat-num"*/);
				rd = fab_getuint(ifile/*, "nval-const-rat-den"*/);
				con->const_value.const_rat = rat_fri(rn, rd);
				break;
			  case	CONST_CONSTRAINT_ERROR:
				break;
			};
			break;
  case	as_terminate_alt:
			/*: terminate_statement (9)  nval is depth_count (int)*/
			nv = (char *) fab_getnum(ifile/*, "nval-terminate-depth"*/);
			break;
  case	as_string_ivalue:
			/* nval is tuple of integers */
			n = fab_getnum(ifile/*, "nval-string-ivalue-size"*/);
			tup	 = tup_new(n);
			for (i = 1;i <= n; i++)
				tup[i] = (char *)fab_getchr(ifile/*, "nval-string-ivalue"*/);
			nv = (char *) tup;
			break;
  case	as_instance_tuple:
			n = fab_getnum(ifile/*, "nval-instance-size"*/);
			if (n != 0) {
				if (n != 2)
					chaos(__FILE__, __LINE__, "getnval: bad nval for instantiation");
				tup = tup_new(n);
				/* first component is instance map */
				n = fab_getnum(ifile/*, "nval-symbolmap-size"*/);
				smap = symbolmap_new();
				for (i = 1; i <= n/2; i++) {
					s1 = fab_getsymref(ifile/*, "symbolmap-1"*/);
					s2 = fab_getsymref(ifile/*, "symbolmap-2"*/);
					symbolmap_put(smap, s1, s2);
				}
				tup[1] = (char *)smap;
				/* second component is needs_body flag */
				tup [2] = (char *)fab_getnum(ifile/*, "nval-flag"*/);
				nv = (char *)tup;
			}
			else nv = NULL;
			break;
};

if (N_VAL_DEFINED(nk)) N_VAL(node) = nv;
if (N_VAL_DEFINED(nk) == FALSE && nv != NULL) {
	chaos(__FILE__, __LINE__, "libr.c: nval exists, but N_VAL_DEFINED not");
}

/* need to handle following cases:
as_simple_name:
otherwise	identifier string

procedure package_instance (12)
  this procedure builds a node of type as_simple_name
  with N_VAL a symbol pointeger.
as_pragma??
as_array aggregate
as_generic: (cf. 12)

*/

}

static void fab_getlitmap(fabIFILE *ifile, Symbol sym)
/* called for na_enum to input literal map.
 * The literal map is a tuple, entries consisting of string followed
 * by integer.
 */
{
Tuple	tup;
int i, n;

n = fab_getnum(ifile/*, "litmap-n"*/);
tup = tup_new(n);
for (i = 1; i <= n; i+=2) {
	tup[i] = fab_getstr(ifile/*, "litmap-str"*/);
	tup[i+1] = (char *) fab_getnum(ifile/*, "litmap-value"*/);
	}
OVERLOADS(sym) = (Set) tup;
}

static int *fab_getuint(fabIFILE *ifile)
{
int n, *res;

n = fab_getnum(ifile/*, "uint-size"*/);
res = (int *) fcalloct((n+1) * sizeof(int), "getuint");
fab_fread((char *) res, sizeof(int) * (n+1), ifile->fh_file);
return res;
}

int fab_read_stub(char *fname, char *uname, char *ext)
{
long	rec;
Stubenv	ev;
int		i, j, k, n, m, si;
char	*funame;
Tuple	stubtup, tup, tup2, tup3;
int		ci, cn;
int		parent_unit;
Tuple	cent, ctup, cntup, nodes_group;
Symbol	sym;
int		retrieved = FALSE;
fabIFILE	*ifile;

/* open so do not fail if no file */
ifile = fab_ifopenread(fname, ext, 1);
if (ifile == NULL) return retrieved; /* if not stub file */

for (rec = fab_read_init(ifile); rec != 0; rec = fab_read_next(ifile, rec)) {
	funame = fab_getstr(ifile/*, "stub-name"*/);
	if (uname != NULL  && !streq(uname, funame)) continue;
	si = stub_number(funame);
	if (uname == NULL) lib_stub_put(funame, fname);
	ev = stubenv_new();
	stubtup = (Tuple) stub_info[si];
	stubtup[2] = (char *) ev;
	n = fab_getnum(ifile/*, "scope-stack-size"*/);
	tup = tup_new(n);
	for (i = 1; i <= n; i++) {
		tup2 = tup_new(4);
		tup2[1] = (char *) fab_getsymref(ifile/*, "scope-stack-symref"*/);
		for (j = 2; j <= 4; j++) {
			m = fab_getnum(ifile/*, "scope-stack-m"*/);
			tup3 = tup_new(m);
			for (k=1; k <= m; k++)
				tup3[k] = (char *) fab_getsymref(ifile/*, "scope-stack-m-symref"*/);
			tup2[j] = (char *) tup3;
		}
		tup[i] = (char *) tup2;
	}
	ev->ev_scope_st = tup;
	ev->ev_unit_unam = fab_getsymref(ifile/*, "ev-unit-name-symref"*/);
	ev->ev_decmap = fab_getdcl(ifile);

	/* unit_nodes */
	n = fab_getnum(ifile/*, "ev-nodes-size"*/);
	tup = tup_new(n);
	for (i = 1; i <= n; i++) {
		tup[i] = (char *) fab_getnodref(ifile/*, "ev-nodes-nodref"*/);
	}
	ev->ev_nodes = tup;

	/* context */
	n = fab_getnum(ifile/*, "stub-context-size"*/);
	if (n > 0) {
		n -= 1; /* true tuple size */
		ctup = tup_new(n);
		for (i = 1; i <= n; i++) {
			cent = (Tuple) tup_new(2);
			cent[1] = (char *) fab_getnum(ifile/*, "stub-cent-1"*/);
			cn = fab_getnum(ifile/*, "stub-cent-2-size"*/); 
			cntup = tup_new(cn);
			for (ci = 1; ci <= cn; ci++)
				cntup[ci] = fab_getstr(ifile/*, "stub-cent-2-str"*/);
			cent[2] = (char *) cntup;
			ctup[i] = (char *) cent;
		}
		ev->ev_context =  ctup;
	}
	/* tuple of symbol table pointers */
	/* read in but ignore symbol table references. This is for
	 * read_stub_short so that the generator can rewrite the stubfile
	 * without reading in full symbol table info from semantics phase.
	 */
	n = fab_getnum(ifile/*, "ev-decls-refs-size"*/);
	if (n > 0) {
		n -= 1; /* true tuple size */
		for (i = 1; i <= n; i++)
			sym = fab_getsymref(ifile/*, "ev-decls-sym-ref"*/);
	}
	/* tuple of symbol table pointers */
	n = fab_getnum(ifile/*, "ev-open-decls-size"*/);
	if (n > 0) {
		n -= 1; /* true tuple size */
		tup = tup_new(n);
		for (i = 1; i <= n; i++) {
			sym = fab_getsym(ifile/*, "ev-open-decls-sym"*/);
/*
if (NATURE(sym) == na_package || NATURE(sym) == na_procedure) {
	sym_temp = sym_new_noseq(na_void);
	sym_copy(sym_temp, sym);
	tup[i] = (char *) sym_temp;
}
else {
	tup[i] = (char *) sym;
}
*/
			tup[i] = (char *) sym;
		}
		ev->ev_open_decls = tup;
	}
	ev->ev_current_level = fab_getnum(ifile/*, "ev-current-level"*/);
	/* tuple of relay-set symbols */
	n = fab_getnum(ifile/*, "ev-relay-set-size"*/);
	if (n > 0) {
		n -= 1; /* true tuple size */
		tup = tup_new(n);
		for (i = 1; i <= n; i++) {
			tup[i] = (char *) fab_getsymref(ifile/*, "relay-set-sym"*/);
			}
		ev->ev_relay_set = tup;
		}
	else {
		ev->ev_relay_set = tup_new(0);
		}
	/* tuple of dang-relay-set symbols */
	n = fab_getnum(ifile/*, "ev-dang-relay-set-size"*/);
	if (n > 0) {
		n -= 1; /* true tuple size */
		tup = tup_new(n);
		for (i = 1; i <= n; i++)
			tup[i] = (char *) fab_getnum(ifile/*, "dang-relay-set-ent"*/);
		ev->ev_dangling_relay_set = tup;
		}
	else {
		ev->ev_dangling_relay_set = tup_new(0);
		}
	retrieved = TRUE;
	if (uname != NULL)  break;
	}
if (retrieved)  {
	tup = ev->ev_nodes;
	n = tup_size(tup);
	nodes_group = tup_new(n);
	for (i = 1; i <= n; i++)
		nodes_group[i] = (char *) N_SEQ((Node)tup[i]);
	parent_unit = stub_parent_get(funame);
	fab_retrieve_tree_nodes(ifile, nodes_group, parent_unit);
	}
fab_ifclose(ifile);
return retrieved;
}

void fab_load_tre(fabIFILE *ifile, int comp_index)
{
/* load entire tree file. */

long	rec, *off;
int		i, fnum, unum, n, nodes, rootseq;
char	*funame; 

i=0;
for (rec = fab_read_init(ifile); rec!=0; rec = fab_read_next(ifile, rec)) {

	i++;
	if (i != comp_index) continue;
	funame = fab_getstr(ifile/*, "unit-name"*/);
	fnum = fab_getnum(ifile/*, "unit-number"*/);
	unum = unit_number(funame);
	if (unum != fnum)
		chaos(__FILE__, __LINE__, "load_tre sequence number error");
	nodes = fab_getnum(ifile/*, "node-count"*/);
	/* the rest of the tree info is set in read_ais. Perhaps all can be
	 * done there.
	 */
	off= (long *)fcalloct((nodes+1) * sizeof(long),"load-tree-tup-3");
	fab_fread((char *) off, sizeof(long) * (nodes+1), ifile->fh_file);
	rootseq = fab_getnum(ifile/*, "root-seq"*/);
	pUnits[unum]->treInfo.rootSeq = rootseq;
	for (n = 1; n <= nodes; n++) {
		if (off[n] == 0) { /* node not needed */
   			continue;
		}
		else {
   			fab_ifseek(ifile/*, "seek-node"*/, off[n]);
   			fab_getnod(ifile/*, "unit-node"*/, getnodptr(n, unum), unum);
		}
	}
	break;
}
tup_free((Tuple) off);
fab_ifclose(ifile);
}

int fab_last_comp_index(fabIFILE *ifile)
{
/* determine the number of comp units in ifile. */
long	rec;
int		i;

i = 0;
for (rec = fab_read_init(ifile); rec != 0; rec = fab_read_next(ifile,rec)) i++; 
return i;
}

Symbol fab_getsymref(fabIFILE *ifile)
{
#ifdef DEBUG
Symbol	sym;
short	unit;
#endif
short	seq;

seq = fab_getnum(ifile/*, "sym-seq"*/);

#ifdef DEBUG
unit = fab_getnum(ifile/*, "sym-unt"*/);
sym = getsymptr(seq, unit);
if (trapss > 0 && trapss == seq && trapsu == unit) traps(sym);
return sym;
#else
return getsymptr(seq, fab_getnum(ifile));
#endif
}

