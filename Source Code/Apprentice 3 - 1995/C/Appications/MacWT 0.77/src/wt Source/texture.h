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

/* GIF87a code from Thomas Hamren (d0hampe@dtek.chalmers.se) */

#ifndef TEXTURE_H_
#define TEXTURE_H_


typedef struct {
     int width, height;
     int log2height;
     Pixel *texels;
     Boolean opaque;
} Texture;

#define TEXTURE_COLUMN(tex, col) ((tex)->texels + ((col) << (tex)->log2height))

extern Texture *new_texture(int width, int height);
extern Texture *read_texture_file(char *filename);
extern void set_texture_trans(int cube_size, long *color_lookup);

#endif
