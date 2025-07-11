/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */

void fab_load_lib(fabIFILE *libfile, Axq axq, char *main_unit);
void load_lib(char *, IFILE *, Axq, char *, char **);
long fab_load_slots(fabIFILE **ifile, Axq axq);
long load_slots(char *, IFILE **, Axq);
