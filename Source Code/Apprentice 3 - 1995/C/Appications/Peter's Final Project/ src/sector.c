/*
 *  Peter's Final Project -- A texture mapping demonstration
 *  � 1995, Peter Mattis
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
#include "sector.h"
#include "sys.stuff.h"

/*
 * Keep a list of free sectors for fast allocation.
 * Keep track of the memory used by sectors.
 */

static SECTORS free_sectors_list = NULL;
static long sector_mem = 0;

/*
 * Make a sector object and initialize its values to something decent.
 */

SECTOR
make_sector ()
{
	SECTORS sectors;
	SECTOR sector;

	if (free_sectors_list)
	{
		sectors = free_sectors_list;
		sector = sectors_first (sectors);
		free_sectors_list = sectors_rest (free_sectors_list);
		free_list (sectors);
	}
	else
	{
		sector_mem += sizeof (_SECTOR);

		sector = (SECTOR) ALLOC (sizeof (_SECTOR));
	}

	set_sector_val (sector, 0);
	set_sector_faces (sector, NULL);
	set_sector_objects (sector, NULL);
	set_sector_neighbors (sector, NULL);

	return sector;
}

/*
 * Free a sector by adding it to the free list.
 */

void
free_sector (s)
	SECTOR s;
{
	LIST temp;

	temp = make_list ();
	set_list_datum (temp, s);
	set_list_next (temp, free_sectors_list);
	free_sectors_list = temp;
}

/*
 * Return the last sector in a list of sectors.
 * (Actually, the list containing the last sector).
 */

SECTORS
sectors_last (sectors)
	SECTORS sectors;
{
	if (sectors)
	{
		while (sectors_rest (sectors))
			sectors = sectors_rest (sectors);

		return sectors;
	}

	return NULL;
}

/*
 * Append a sector to a list of sectors.
 */

SECTORS
sectors_append_sector (set, s)
	SECTORS set;
	SECTOR s;
{
	LIST n;

	n = make_list ();
	set_list_datum (n, s);

	return ((SECTORS) list_append_list ((LIST) set, n));
}

/*
 * Prepend a sector to a list of sectors.
 */

SECTORS
sectors_prepend_sector (set, s)
	SECTORS set;
	SECTOR s;
{
	LIST n;

	n = make_list ();
	set_list_datum (n, s);

	return ((SECTORS) list_prepend_list ((LIST) set, n));
}

/*
 * Free a list of sectors.
 */

void
free_sectors (set)
	SECTORS set;
{
	if (set == NULL)
		return;

	free_sectors (sectors_rest (set));

	set_sectors_rest (set, free_sectors_list);
	free_sectors_list = set;
}

/*
 * Return sector memory usage.
 */

long
sector_mem_usage ()
{
	return sector_mem;
}
