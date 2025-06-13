/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */

Declaredmap getdcl(IFILE *);
Symbol fab_getsym(fabIFILE *ifile);
Symbol getsym(IFILE *, char *);
Node getnodptr(short, short);
Node getnodref(IFILE *, char *);
char *fab_read_ais(char *fname, char *uname, int comp_index, Boolean tree_is_needed, Boolean is_aic_file);
char *read_ais(char *, int, char *, int, int);
int fab_read_stub(char *fname, char *uname, char *ext);
int read_stub(char *, char *, char *);
int read_lib(void);
void fab_load_tre(fabIFILE *ifile, int comp_index);
void load_tre(IFILE *, int);
Tuple add_tree_node(Tuple, Node);
Symbol fab_getsymref(fabIFILE *ifile);
Symbol getsymref(IFILE *, char *);
void fab_retrieve_tree_nodes(fabIFILE *ifile, Tuple nodes_list, short node_unit);
void retrieve_generic_tree(Node, Node);
char *lib_aisname(void);
void fab_get_unit_unam(fabIFILE *ifile);
void get_unit_unam(IFILE *, Symbol);
int fab_last_comp_index(fabIFILE *ifile);
