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

#ifndef __TYPE_DEFS_H__
#define __TYPE_DEFS_H__

#include "my_math.h"
#include "sys.types.h"

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#ifndef ABS
#define ABS(x)	(((x) < 0) ? -(x) : (x))
#endif /* ABS */

#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif /* MAX */

#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif /* MIN */

#ifndef ROUND
#define ROUND(x)   ((my_frac(x) < NUM_ONE_HALF) ? my_floor(x) : my_ceil(x))
#endif /* ROUND */

typedef short BOOLEAN;

typedef NUM VECTOR[4];
typedef NUM MATRIX[4][4];

typedef struct _MAZE_CELL MAZE_CELL;
typedef struct _MAZE _MAZE, *MAZE;
typedef struct _LIST _LIST, *LIST;
typedef struct _TEXTURE _TEXTURE, *TEXTURE;
typedef struct _POINT _POINT, *POINT;
typedef struct _FACE _FACE, *FACE;
typedef struct _OBJECT _OBJECT, *OBJECT;
typedef struct _SECTOR _SECTOR, *SECTOR;
typedef struct _CAMERA _CAMERA, *CAMERA;
typedef struct _WORLD _WORLD, *WORLD;

typedef LIST POINTS;
typedef LIST FACES;
typedef LIST OBJECTS;
typedef LIST SECTORS;

struct _MAZE_CELL
{
	short id;
	BOOLEAN state;
	SECTOR sector;
};

struct _MAZE
{
	MAZE_CELL *data;
	short size;
};

struct _LIST
{
	void *data;
	LIST next;
	LIST prev;
};

struct _TEXTURE
{
	void *addr;
	long size;
	long size_log2;
	long bytes_per_texel;
};

struct _POINT
{
	VECTOR coord;
	NUM intensity;
};

struct _FACE
{
	POINTS points;
	POINT normal;
	POINT texture_o;
	POINT texture_u;
	POINT texture_v;
	NUM intensity;
	TEXTURE texture;
	BOOLEAN obstructs;
};

struct _OBJECT
{
	long id;
	long val;

	FACES faces;
	MATRIX matrix;

	NUM red, green, blue;

	NUM radius;
	VECTOR pos;
	VECTOR dir;
	NUM pos_vel, pos_drag;
	NUM dir_vel, dir_drag;
	NUM bounce;

	CAMERA camera;
	SECTOR sector;
	SECTORS sectors;

	void (*move_func) (OBJECT);
	void *data;
};

struct _SECTOR
{
	long val;
	FACES faces;
	OBJECTS objects;
	SECTORS neighbors;
};

struct _CAMERA
{
	VECTOR pos;
	VECTOR vpn;
	VECTOR vup;

	NUM u_min, u_max;
	NUM v_min, v_max;

	MATRIX orientation;
	MATRIX mapping;
};

struct _WORLD
{
	POINT points;
	FACES faces;
};

#endif /* __TYPE_DEFS_H__ */
