/*
 * Copyright (c) 1988, 1992 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * This code received contributions from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Paulo Almeida       - TEXT3D primitive
 *  Pedro Borges        - TEXT3D primitive
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Antonio Costa, at INESC-Norte. The name of the author and
 * INESC-Norte may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/**********************************************************************
 *    RAY TRACING - Version 7.2                                       *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, June 1992              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, June 1992              *
 **********************************************************************/

/* Modified from defs.h /*
/***** Types *****/
typedef
int             boolean;

typedef
double          real;

typedef
char           *char_ptr;

typedef
FILE           *file_ptr;

/***** If there is no void type, define NOVOID *****/
#ifdef NOVOID
#define void char
#define void_ptr char_ptr
#else
typedef
void           *void_ptr;
#endif

/***** Ray Tracing types *****/
typedef
struct
{
  real            x, y, z;
} xyz_struct;
typedef
xyz_struct     *xyz_ptr;

typedef
struct
{
  real            x, y, z, w;
} xyzw_struct;
typedef
xyzw_struct    *xyzw_ptr;

typedef
struct
{
  short int       type;
  xyzw_ptr        transf;
  void_ptr        next;
  void_ptr        data;
} texture_struct;
typedef
texture_struct *texture_ptr;

typedef
struct
{
  xyz_ptr         position;
  void_ptr        data;
} text_struct;
typedef
text_struct    *text_ptr;

typedef
struct
{
  int             id;           /* Object identifier  */
  short int       surface_id;   /* Surface identifier */
  real            refraction;   /* Refraction index   */
  xyz_ptr         min, max;     /* Bounding volume    */
  xyzw_ptr        transf;       /* Transformation     */
  xyzw_ptr        inv_transf;   /* Inverse transf.    */
  texture_ptr     texture;      /* List of textures   */
  boolean         texture_modify_normal;
  short int       object_type;
  void_ptr        data;
} object_struct;
typedef
object_struct  *object_ptr;
