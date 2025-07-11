/*
**  wt -- a 3d game engine
**
**  Copyright (C) 1994 by Chris Laurel
**  email:  claurel@mr.net
**  snail mail:  Chris Laurel, 5700 W Lake St #208,  St. Louis Park, MN  55416
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef _LIST_H_
#define _LIST_H_

typedef struct List_s {
     void *node;
     struct List_s *next;
} List;

#define LIST_NODE(l, type) ((type) (l)->next->node)


typedef Boolean Scan_list_function(List *l, void *data);

extern List *new_list(void);
extern void add_node(List *l, void *node);
extern void delete_node(List *l);
extern void remove_node(List *l);
extern void delete_list(List *l);

extern List *scan_list(List *l, void *data, Scan_list_function *func);
extern Boolean find_node(List *l, void *data);

#endif /* _LIST_H_ */

