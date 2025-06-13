/*
 *  Peter's Final Project -- A texture mapping demonstration
 *  © 1995, Peter Mattis
 *
 *  E-mail:
 *  petm@soda.csua.berkeley.edu
 *
 *  Snail-mail:
 *   Peter Mattis
 *   557 Fort Laramie Dr.
 *   Sunnyvale, CA 94087
 *
 *  Avaible from:
 *  http://www.csua.berkeley.edu/~petm/final.html
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __LIST_H__
#define __LIST_H__

#include "type.defs.h"

LIST make_list (void);
void free_list (LIST);
LIST list_nth (LIST, short);
LIST list_append_list (LIST, LIST);
LIST list_prepend_list (LIST, LIST);
LIST list_remove_list (LIST, void *);

#define list_datum(lst)			((lst)->data)
#define list_next(lst)			((lst)->next)
#define list_prev(lst)          ((lst)->prev)

#define set_list_datum(lst, k)  	(list_datum(lst) = (k))
#define set_list_next(lst, k)    	(list_next(lst) = (k))
#define set_list_prev(lst, k)       (list_prev(lst) = (k))

#endif /* __LIST_H__ */
