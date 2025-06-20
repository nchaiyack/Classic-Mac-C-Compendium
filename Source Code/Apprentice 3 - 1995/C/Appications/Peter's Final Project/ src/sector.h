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

#ifndef __SECTOR_H__
#define __SECTOR_H__

#include "face.h"

SECTOR make_sector (void);
void free_sector (SECTOR);

SECTORS sectors_last (SECTORS);
SECTORS sectors_append_sector (SECTORS, SECTOR);
SECTORS sectors_prepend_sector (SECTORS, SECTOR);
void free_sectors (SECTORS);

long sector_mem_usage (void);

#define sector_val(s)        ((s)->val)
#define sector_faces(s)      ((s)->faces)
#define sector_objects(s)    ((s)->objects)
#define sector_neighbors(s)  ((s)->neighbors)

#define set_sector_val(s, k)        (sector_val(s) = (k))
#define set_sector_faces(s, k)      (sector_faces(s) = (k))
#define set_sector_objects(s, k)    (sector_objects(s) = (k))
#define set_sector_neighbors(s, k)  (sector_neighbors(s) = (k))

#define sectors_first(p)   ((SECTOR) list_datum((LIST) p))
#define sectors_rest(p)    ((SECTORS) list_next((LIST) p))
#define sectors_prev(p)    ((SECTORS) list_prev((LIST) p))

#define set_sectors_first(p, k)  (set_list_datum((LIST) p, k))
#define set_sectors_rest(p, k)   (set_list_next((LIST) p, k))
#define set_sectors_prev(p, k)   (set_list_prev((LIST) p, k))

#define sectors_add_sector  sectors_prepend_sector

#endif /* __SECTOR_H__ */
