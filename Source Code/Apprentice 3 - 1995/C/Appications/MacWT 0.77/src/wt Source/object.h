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


typedef struct {
     double mass;
     double xsize, ysize, height;
     double x, y, z;
     double dx, dy, dz;
     double angle;
     double angular_v;
     double drag;
     World *world;
     Region *region;
     Texture *image;
} Object;

typedef struct {
	double left;
	double top;
	double right;
	double bottom;
} Bounds;

extern Object *new_object(double mass,
			  double xsize, double ysize, double height,
			  double drag);
extern void object_update(Object *o);
extern void object_apply_force(Object *o, double fx, double fy, double fz);
extern void object_apply_torque(Object *o, double torque);
extern void object_set_position(Object *o, double x, double y, double z);

extern void add_object(World *w, Object *o);
extern Boolean remove_object(World *w, Object *o);

extern void object_view(Object *o, View *v);
