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
#include "wt.h"
#include "wtmem.h"
#include "list.h"


List *new_list(void)
{
     List *l;

     l = wtmalloc(sizeof(List));
     l->node = NULL;
     l->next = NULL;

     return l;
}


void add_node(List *l, void *node)
{
     List *t;

     t = l->next;
     l->next = new_list();
     l->next->node = node;
     l->next->next = t;
}


void delete_node(List *l)
{
     List *t;

     t = l->next->next;
     if (l->next->node != NULL)
	  wtfree(l->next->node);
     wtfree(l->next);
     l->next = t;
}


/* Remove a node from a list, but don't free the memory used by the node.
**   Useful for moving an element from one list to another . . .
*/
void remove_node(List *l)
{
     List *t;

     t = l->next->next;
     wtfree(l->next);
     l->next = t;
}     


void delete_list(List *l)
{
     while (l->next)
	  delete_node(l);

     wtfree(l);
}


List *scan_list(List *l, void *data, Scan_list_function *func)
{
     while (l->next != NULL) {
	  if (func(l, data))
	       break;
	  else
	       l = l->next;
     }

     if (l->next == NULL)
	  return NULL;
     else
	  return l;
}


/* Basic search function for use with scan list--find an object with
**   a node pointing to the same place as data.
*/
Boolean find_node(List *l, void *data)
{
     if (LIST_NODE(l, void *) == data)
	  return True;
     else
	  return False;
}
