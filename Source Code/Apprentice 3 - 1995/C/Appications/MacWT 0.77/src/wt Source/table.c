/*
**  MacWT -- a 3d game engine for the Macintosh
**  � 1995, Bill Hayden and Nikol Software
**
**  On the Internet:
**  bmoc1@aol.com (my personal address)
**  nikolsw@grove.ufl.edu (my school address)
**	MacWT anonymous FTP site: ftp.circa.ufl.edu/pub/software/ufmug/mirrors/LocalSW/Hayden/
**  http://grove.ufl.edu:80/~nikolsw (my WWW page, containing MacWT info)
**
**  based on wt, by Chris Laurel (claurel@mr.net)
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <stdlib.h>
#include <string.h>
#include "wt.h"
#include "error.h"
#include "wtmem.h"
#include "table.h"


#define STARTING_TABLE_SIZE  16


/* For a dynamic table, pass 0 for the table_size; otherwise the table size
**   will be fixed at table_size entries;
*/
Table *new_table(size_t entry_size, int table_size)
{
     Table *t;
     
     t = wtmalloc(sizeof(Table));
     t->entry_size = entry_size;
     t->current_entries = 0;
     if (table_size != 0) {
	  t->fixed_size = True;
	  t->max_entries = table_size;
	  t->table = wtmalloc(entry_size * table_size);
     } else {
	  t->fixed_size = False;
	  t->max_entries = 0;
	  t->table = NULL;
     }

     return t;
}


/* Add an entry to a table and return its index. */
int add_table_entry(Table *t, void *entry)
{
     if (t->current_entries >= t->max_entries) {
	  if (t->fixed_size)
	       fatal_error("Fixed size table full.");

	  if (t->max_entries == 0) {
	       t->max_entries = STARTING_TABLE_SIZE;
	       t->table = wtmalloc(t->max_entries * t->entry_size);
	  } else {
	       t->max_entries = (t->current_entries * 3) / 2;
	       t->table = wtrealloc(t->table,
				    t->max_entries * t->entry_size);
	  }
     }

     memcpy((char *) t->table + t->entry_size * t->current_entries,
	    entry, t->entry_size);

     return t->current_entries++;
}

