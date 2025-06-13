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


#define INTENT_END_GAME 0
#define INTENT_JUMP     1
#define INTENT_ACTION1  2
#define INTENT_ACTION2  3
#define INTENT_ACTION3  4
#define INTENT_ACTION4  5
#define INTENT_ACTION5  6


#define MAX_SPECIAL_INTENTIONS  20

#define MOVE_FORCE 30.0
#define TURN_FORCE  5.0

typedef struct {
     double force_x, force_y, force_z;
     double force_rotate;
     int n_special;
     int special[MAX_SPECIAL_INTENTIONS];
} Intent;


extern void init_input_devices(void);
extern Intent *read_input_devices(void);
extern void end_input_devices(void);
