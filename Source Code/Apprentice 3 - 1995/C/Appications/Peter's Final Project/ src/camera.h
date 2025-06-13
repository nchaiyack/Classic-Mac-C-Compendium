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

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "matrix.vector.h"
#include "type.defs.h"

CAMERA make_camera (void);
void free_camera (CAMERA);

void camera_determine_matrices (CAMERA);

#define camera_pos(c)                  ((c)->pos)
#define camera_vpn(c)                  ((c)->vpn)
#define camera_vup(c)                  ((c)->vup)
#define camera_u_min(c)                ((c)->u_min)
#define camera_u_max(c)                ((c)->u_max)
#define camera_v_min(c)                ((c)->v_min)
#define camera_v_max(c)                ((c)->v_max)
#define camera_matrix_orientation(c)   ((c)->orientation)
#define camera_matrix_mapping(c)       ((c)->mapping)

#define camera_pos_x(c)  (vector_x(camera_pos(c)))
#define camera_pos_y(c)  (vector_y(camera_pos(c)))
#define camera_pos_z(c)  (vector_z(camera_pos(c)))
#define camera_pos_w(c)  (vector_w(camera_pos(c)))

#define camera_vpn_x(c)  (vector_x(camera_vpn(c)))
#define camera_vpn_y(c)  (vector_y(camera_vpn(c)))
#define camera_vpn_z(c)  (vector_z(camera_vpn(c)))
#define camera_vpn_w(c)  (vector_w(camera_vpn(c)))

#define camera_vup_x(c)  (vector_x(camera_vup(c)))
#define camera_vup_y(c)  (vector_y(camera_vup(c)))
#define camera_vup_z(c)  (vector_z(camera_vup(c)))
#define camera_vup_w(c)  (vector_w(camera_vup(c)))

#define set_camera_pos_x(c, k)     (camera_pos_x(c) = (k))
#define set_camera_pos_y(c, k)     (camera_pos_y(c) = (k))
#define set_camera_pos_z(c, k)     (camera_pos_z(c) = (k))
#define set_camera_pos_w(c, k)     (camera_pos_w(c) = (k))

#define set_camera_vpn_x(c, k)     (camera_vpn_x(c) = (k))
#define set_camera_vpn_y(c, k)     (camera_vpn_y(c) = (k))
#define set_camera_vpn_z(c, k)     (camera_vpn_z(c) = (k))
#define set_camera_vpn_w(c, k)     (camera_vpn_w(c) = (k))

#define set_camera_vup_x(c, k)     (camera_vup_x(c) = (k))
#define set_camera_vup_y(c, k)     (camera_vup_y(c) = (k))
#define set_camera_vup_z(c, k)     (camera_vup_z(c) = (k))
#define set_camera_vup_w(c, k)     (camera_vup_w(c) = (k))

#define set_camera_u_min(c, k)     (camera_u_min(c) = (k))
#define set_camera_u_max(c, k)     (camera_u_max(c) = (k))
#define set_camera_v_min(c, k)     (camera_v_min(c) = (k))
#define set_camera_v_max(c, k)     (camera_v_max(c) = (k))

#endif /* __CAMERA_H__ */
