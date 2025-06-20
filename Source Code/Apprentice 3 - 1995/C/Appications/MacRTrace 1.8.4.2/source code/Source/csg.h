/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Craig Kolb          - CSG
 *  Reid Judd           - portability
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
#include "defs.h"
#include "extern.h"

/**********************************************************************
 *    RAY TRACING - CSG - Version 8.3.4                               *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, June 1992              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, November 1993          *
 **********************************************************************/

/***** CSG *****/
#define CSG_NODES_MAX (224)

typedef
struct
{
  unsigned short int depth;
  unsigned char   path[(CSG_NODES_MAX + 7) / 8];
} tree_struct;
typedef
tree_struct    *tree_ptr;

#define PATH_LEFT  (0)
#define PATH_RIGHT (1)

#define SAVE_TREE_PATH(side)\
do {\
  if ((side) == PATH_LEFT)\
    tree.path[tree.depth SHR 8] &= BIT_NOT(1 SHL (tree.depth BIT_AND 7));\
  if ((side) == PATH_RIGHT)\
    tree.path[tree.depth SHR 8] |= 1 SHL (tree.depth BIT_AND 7);\
} while (0)

typedef
struct
{
  object_ptr      object;
  real            distance;
  xyz_struct      vector;
  unsigned short int enter;
  tree_struct     tree;
} hit_node_struct;

typedef
struct
{
  short int       nodes;
  hit_node_struct data[CSG_NODES_MAX];
} hit_struct;
typedef
hit_struct     *hit_ptr;

#define ENTERING (2)

#define CSG_SET_ENTER(hit, flag) (hit)->data[0].enter = (flag) + 1

#ifdef _Windows
#pragma option -zEcsg1s -zFcsg1c -zHcsg1g
#define FAR far
#else
#define FAR
#endif

static tree_struct FAR tree;
static hit_struct FAR hit_global;

#ifdef _Windows
#pragma option -zE* -zF* -zH*
#endif

