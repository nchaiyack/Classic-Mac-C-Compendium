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

#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "type.defs.h"

TEXTURE make_texture (void);
void free_texture (TEXTURE);

void texture_checker (TEXTURE, short);
void texture_read (TEXTURE, char *);

#define texture_address(t)            ((t)->addr)
#define texture_size(t)               ((t)->size)
#define texture_size_log2(t)          ((t)->size_log2)
#define texture_bytes_per_texel(t)    ((t)->bytes_per_texel)
#define texture_element8(t, i, j)     (*((char*) texture_address(t) + \
((j) << texture_size_log2(t)) + (i)))
#define texture_element16(t, i, j)    (*((short*) texture_address(t) + \
((j) << texture_size_log2(t)) + (i)))
#define texture_element32(t, i, j)    (*((long*) texture_address(t) + \
((j) << texture_size_log2(t)) + (i)))

#define set_texture_address(t, k)           (texture_address(t) = (k))
#define set_texture_size(t, k)               (texture_size(t) = (k))
#define set_texture_size_log2(t, k)          (texture_size_log2(t) = (k))
#define set_texture_bytes_per_texel(t, k)    (texture_bytes_per_texel(t) = (k))
#define set_texture_element8(t, i, j, k)     (texture_element8(t, i, j) = (k))
#define set_texture_element16(t, i, j, k)    (texture_element16(t, i, j) = (k))
#define set_texture_element32(t, i, j, k)    (texture_element32(t, i, j) = (k))

#define texel8      texture_element8
#define texel16     texture_element16
#define texel24     texture_element32
#define texel32     texture_element32

#define set_texel8   set_texture_element8
#define set_texel16  set_texture_element16
#define set_texel32  set_texture_element32
#define set_texel32  set_texture_element32

#endif /* __TEXTURE_H__ */
