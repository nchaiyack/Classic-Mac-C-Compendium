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

#include <assert.h>
#include "list.h"
#include "sys.stuff.h"

/*
 * Keep a list of free list nodes for fast allocation.
 */

static LIST free_list_list = NULL;

/*
 * Make a list object and initialize its values to something decent.
 */

LIST
make_list ()
{
	LIST list;

	if (free_list_list)
	{
		list = free_list_list;
		free_list_list = list_next (free_list_list);
	}
	else
	{
		list = (LIST) ALLOC (sizeof (_LIST));
	}

	set_list_datum (list, NULL);
	set_list_next (list, NULL);

	return list;
}

/*
 * Free a list by placing it on the free list.
 */

void
free_list (list)
	LIST list;
{
	set_list_next (list, free_list_list);
	free_list_list = list;
}

/*
 * Return the nth node of a list. (recursive)
 */

LIST
list_nth (list, n)
	LIST list;
	short n;
{
	if (list != NULL)
		if (n > 0)
			return list_nth (list_next (list), n - 1);

	return list;
}

/*
 * Append a list.
 */

LIST
list_append_list (set, lst)
	LIST set, lst;
{
	LIST temp;

	if (!set)
	{
		return lst;
	}
	else
	{
		temp = set;
		while (list_next (temp))
			temp = list_next (temp);

		set_list_next (temp, lst);
		set_list_prev (lst, temp);

		return set;
	}
}

/*
 * Prepend a list.
 */

LIST
list_prepend_list (set, lst)
	LIST set, lst;
{
	if (set)
		set_list_prev (set, lst);
	set_list_next (lst, set);
	set_list_prev (lst, NULL);

	return lst;
}

/*
 * Remove the node that contains "datum"
 */

LIST
list_remove_list (set, datum)
	LIST set;
	void *datum;
{
	LIST tmp;

	if (!set)
		return NULL;

	if (list_datum (set) == datum)
	{
		tmp = set;
		set = list_next (set);
		if (set && list_prev (set))
			set_list_prev (set, NULL);
		free_list (tmp);
	}
	else
	{
		tmp = list_next (set);
		while (tmp)
		{
			if (list_datum (tmp) == datum)
			{
				if (list_prev (tmp))
					set_list_next (list_prev (tmp), list_next (tmp));
				if (list_next (tmp))
					set_list_prev (list_next (tmp), list_prev (tmp));

				free_list (tmp);
				break;
			}

			tmp = list_next (tmp);
		}
	}

	return set;
}
