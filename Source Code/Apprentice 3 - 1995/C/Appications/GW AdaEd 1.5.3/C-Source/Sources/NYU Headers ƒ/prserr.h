/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

*/

void prserr(struct prsstack *);
void add_to_top(struct prsstack *tok);	// added prototype [Fabrizio Oddone]

extern int *open_seq;
/* struct two_pool *closer_toksyms;
 struct two_pool *closer_bottom; */
extern int n_open_seq;
extern int n_closer_toksyms;
extern int nps;

extern struct two_pool *closer_toksyms;
//extern char *CLOSER_MESSAGE_SYMS();

