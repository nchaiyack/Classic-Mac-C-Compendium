/*
 * Copyright (c) 1988, 1992 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * This code received contributions from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Craig Kolb          - CSG
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
#include "csg.h"

/**********************************************************************
 *    RAY TRACING - CSG - Version 7.3.1                               *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, June 1992              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, July 1992              *
 **********************************************************************/

/***** CSG *****/
#define CSG_NODES_MAX (224)

extern tree_struct tree;
extern hit_struct hit_global;

static void
csg_remove(rem_object, csg_removed, non_csg_removed)
  object_ptr      rem_object;
  int            *csg_removed, *non_csg_removed;
{
  csg_ptr         csg;
  texture_ptr     texture, old_texture;

  if (rem_object == NULL)
    return;
  FREE(rem_object->min);
  FREE(rem_object->max);
  if (rem_object->transf != NULL)
    FREE(rem_object->transf);
  if (rem_object->inv_transf != NULL)
    FREE(rem_object->inv_transf);
  texture = rem_object->texture;
  while (texture != NULL)
  {
    if (texture->transf != NULL)
      FREE(texture->transf);
    if (texture->data != NULL)
      FREE(texture->data);
    old_texture = texture;
    texture = texture->next;
    FREE(old_texture);
  }
  if (rem_object->object_type != CSG_TYPE)
  {
    FREE(rem_object->data);
    FREE(rem_object);
    POSINC(*non_csg_removed);
    return;
  }
  csg = (csg_ptr) rem_object->data;
  if (csg->position != NULL)
    FREE(csg->position);
  texture = csg->texture;
  while (texture != NULL)
  {
    if (texture->transf != NULL)
      FREE(texture->transf);
    if (texture->data != NULL)
      FREE(texture->data);
    old_texture = texture;
    texture = texture->next;
    FREE(old_texture);
  }
  csg_remove(object[csg->left], csg_removed, non_csg_removed);
  object[csg->left] = NULL;
  csg_remove(object[csg->right], csg_removed, non_csg_removed);
  object[csg->right] = NULL;
  FREE(rem_object->data);
  FREE(rem_object);
  POSINC(*csg_removed);
}

void
csg_normal(position, csg_object, normal)
  xyz_ptr         position;
  object_ptr      csg_object;
  xyz_ptr         normal;
{
  REG int         i;
  REG real        k;
  xyz_struct      old_position, new_normal;
  object_ptr      new_csg_object;
  csg_ptr         csg;
  int             transfs;
  xyzw_ptr        transf[CSG_NODES_MAX];
  xyz_ptr         transf_position[CSG_NODES_MAX];

  new_csg_object = csg_object;
  csg = (csg_ptr) csg_object->data;
  if (csg_object->transf != NULL)
    STRUCT_ASSIGN(old_position, *(csg->position));
  else
    STRUCT_ASSIGN(old_position, *position);
  transfs = 0;
  csg_object->surface_id = csg->surface_id;
  csg_object->refraction = csg->refraction;
  csg_object->texture = csg->texture;
  /* CSG transformations */
  for (i = 0; i < (int) hit_global.data[0].tree.depth; POSINC(i))
  {
    if (RIGHT_TREE_PATH(i))
      new_csg_object = object[csg->right];
    else
      new_csg_object = object[csg->left];
    if (new_csg_object->object_type != CSG_TYPE)
      break;
    csg = (csg_ptr) (new_csg_object->data);
    if ((csg_object->surface_id < 0) AND(csg->surface_id >= 0))
      csg_object->surface_id = csg->surface_id;
    if ((csg_object->refraction < 0.0) AND(csg->refraction > 0.0))
      csg_object->refraction = csg->refraction;
    if ((csg_object->texture == NULL) AND(csg->texture != NULL))
      csg_object->texture = csg->texture;
    if (new_csg_object->transf != NULL)
    {
      STRUCT_ASSIGN(old_position, *(csg->position));
      transf[transfs] = new_csg_object->transf;
      transf_position[transfs] = csg->position;
      POSINC(transfs);
    }
  }
  if (csg_object->surface_id < 0)
    csg_object->surface_id = hit_global.data[0].object->surface_id;
  if (csg_object->refraction < 0.0)
    csg_object->refraction = hit_global.data[0].object->refraction;
  if (csg_object->texture == NULL)
    csg_object->texture = hit_global.data[0].object->texture;
  OBJECT_NORMAL(&old_position, hit_global.data[0].object, normal);
  /* CSG Normal inverse transformations */
  while (transfs > 0)
  {
    POSDEC(transfs);
    transform_normal_vector(transf[transfs], transf_position[transfs],
                            normal, &new_normal);
    STRUCT_ASSIGN(*normal, new_normal);
    NORMALIZE(*normal);
  }
  if (csg_object->transf != NULL)
  {
    csg = (csg_ptr) csg_object->data;
    transform_normal_vector(csg_object->transf, csg->position, normal,
                            &new_normal);
    STRUCT_ASSIGN(*normal, new_normal);
    NORMALIZE(*normal);
  }
  k = DOT_PRODUCT(hit_global.data[0].vector, *normal);
  if ((hit_global.data[0].enter == ENTERING) == (k > 0.0))
  {
    /* Flip normal */
    normal->x = -normal->x;
    normal->y = -normal->y;
    normal->z = -normal->z;
  }
}

void
csg_enclose(csg_object, csg_removed, non_csg_removed)
  int             csg_object, *csg_removed, *non_csg_removed;
{
  xyz_struct      max, min, temp, vertex;
  csg_ptr         csg;
  object_ptr      parent, left, right;
  xyz_struct      tmp;

  parent = object[csg_object];
  csg = (csg_ptr) parent->data;
  left = object[csg->left];
  right = object[csg->right];
  switch (csg->op)
  {
  case CSG_UNION:
    if ((left == NULL) AND(right == NULL))
    {
      WRITE(results, "Warning: null CSG UNION (OBJECT %d)\n", csg_object);
      csg_remove(parent, csg_removed, non_csg_removed);
      object[csg_object] = NULL;
      return;
    }
    if (right == NULL)
    {
      WRITE(results, "Warning: dummy CSG UNION (null RIGHT OBJECT %d)\n",
            csg->right);
      object[csg_object] = left;
      object[csg->left] = NULL;
      csg_remove(parent, csg_removed, non_csg_removed);
      return;
    }
    if (left == NULL)
    {
      WRITE(results, "Warning: dummy CSG UNION (null LEFT OBJECT %d)\n",
            csg->left);
      object[csg_object] = right;
      object[csg->right] = NULL;
      csg_remove(parent, csg_removed, non_csg_removed);
      return;
    }
    parent->min->x = MIN(left->min->x, right->min->x);
    parent->min->y = MIN(left->min->y, right->min->y);
    parent->min->z = MIN(left->min->z, right->min->z);
    parent->max->x = MAX(left->max->x, right->max->x);
    parent->max->y = MAX(left->max->y, right->max->y);
    parent->max->z = MAX(left->max->z, right->max->z);
    break;
  case CSG_INTERSECTION:
    if ((left == NULL) OR(right == NULL)
        OR(left->max->x < right->min->x)
	OR(left->min->x > right->max->x)
        OR(left->max->y < right->min->y)
	OR(left->min->y > right->max->y)
        OR(left->max->z < right->min->z)
	OR(left->min->z > right->max->z))
    {
      WRITE(results, "Warning: null CSG INTERSECTION (OBJECT %d)\n",
	    csg_object);
      csg_remove(parent, csg_removed, non_csg_removed);
      object[csg_object] = NULL;
      return;
    }
    parent->min->x = MAX(left->min->x, right->min->x);
    parent->min->y = MAX(left->min->y, right->min->y);
    parent->min->z = MAX(left->min->z, right->min->z);
    parent->max->x = MIN(left->max->x, right->max->x);
    parent->max->y = MIN(left->max->y, right->max->y);
    parent->max->z = MIN(left->max->z, right->max->z);
    break;
  case CSG_SUBTRACTION:
    if (left == NULL)
    {
      WRITE(results, "Warning: null CSG SUBTRACTION (OBJECT %d)\n",
	    csg_object);
      csg_remove(parent, csg_removed, non_csg_removed);
      object[csg_object] = NULL;
      return;
    }
    if ((right == NULL)
        OR(left->max->x < right->min->x) OR(left->min->x > right->max->x)
        OR(left->max->y < right->min->y) OR(left->min->y > right->max->y)
        OR(left->max->z < right->min->z) OR(left->min->z > right->max->z))
    {
      WRITE(results, "Warning: dummy CSG SUBTRACTION (null RIGHT OBJECT %d)\n",
            csg->right);
      object[csg_object] = left;
      object[csg->left] = NULL;
      csg_remove(parent, csg_removed, non_csg_removed);
      return;
    }
    parent->min->x = left->min->x;
    parent->min->y = left->min->y;
    parent->min->z = left->min->z;
    parent->max->x = left->max->x;
    parent->max->y = left->max->y;
    parent->max->z = left->max->z;
    break;
  }
  csg->surface_id = parent->surface_id;
  csg->refraction = parent->refraction;
  csg->texture = parent->texture;
  if (parent->transf != NULL)
  {
    ALLOCATE(csg->position, xyz_struct, 1);
    STRUCT_ASSIGN(max, *(parent->max));
    STRUCT_ASSIGN(min, *(parent->min));
    temp.x = min.x;
    temp.y = min.y;
    temp.z = min.z;
    transform(parent->inv_transf, &temp, parent->min);
    STRUCT_ASSIGN(*(parent->max), *(parent->min));
    temp.x = min.x;
    temp.y = min.y;
    temp.z = max.z;
    transform(parent->inv_transf, &temp, &vertex);
    if (vertex.x < parent->min->x)
      parent->min->x = vertex.x;
    if (vertex.x > parent->max->x)
      parent->max->x = vertex.x;
    if (vertex.y < parent->min->y)
      parent->min->y = vertex.y;
    if (vertex.y > parent->max->y)
      parent->max->y = vertex.y;
    if (vertex.z < parent->min->z)
      parent->min->z = vertex.z;
    if (vertex.z > parent->max->z)
      parent->max->z = vertex.z;
    temp.x = min.x;
    temp.y = max.y;
    temp.z = min.z;
    transform(parent->inv_transf, &temp, &vertex);
    if (vertex.x < parent->min->x)
      parent->min->x = vertex.x;
    if (vertex.x > parent->max->x)
      parent->max->x = vertex.x;
    if (vertex.y < parent->min->y)
      parent->min->y = vertex.y;
    if (vertex.y > parent->max->y)
      parent->max->y = vertex.y;
    if (vertex.z < parent->min->z)
      parent->min->z = vertex.z;
    if (vertex.z > parent->max->z)
      parent->max->z = vertex.z;
    temp.x = min.x;
    temp.y = max.y;
    temp.z = max.z;
    transform(parent->inv_transf, &temp, &vertex);
    if (vertex.x < parent->min->x)
      parent->min->x = vertex.x;
    if (vertex.x > parent->max->x)
      parent->max->x = vertex.x;
    if (vertex.y < parent->min->y)
      parent->min->y = vertex.y;
    if (vertex.y > parent->max->y)
      parent->max->y = vertex.y;
    if (vertex.z < parent->min->z)
      parent->min->z = vertex.z;
    if (vertex.z > parent->max->z)
      parent->max->z = vertex.z;
    temp.x = max.x;
    temp.y = min.y;
    temp.z = min.z;
    transform(parent->inv_transf, &temp, &vertex);
    if (vertex.x < parent->min->x)
      parent->min->x = vertex.x;
    if (vertex.x > parent->max->x)
      parent->max->x = vertex.x;
    if (vertex.y < parent->min->y)
      parent->min->y = vertex.y;
    if (vertex.y > parent->max->y)
      parent->max->y = vertex.y;
    if (vertex.z < parent->min->z)
      parent->min->z = vertex.z;
    if (vertex.z > parent->max->z)
      parent->max->z = vertex.z;
    temp.x = max.x;
    temp.y = min.y;
    temp.z = max.z;
    transform(parent->inv_transf, &temp, &vertex);
    if (vertex.x < parent->min->x)
      parent->min->x = vertex.x;
    if (vertex.x > parent->max->x)
      parent->max->x = vertex.x;
    if (vertex.y < parent->min->y)
      parent->min->y = vertex.y;
    if (vertex.y > parent->max->y)
      parent->max->y = vertex.y;
    if (vertex.z < parent->min->z)
      parent->min->z = vertex.z;
    if (vertex.z > parent->max->z)
      parent->max->z = vertex.z;
    temp.x = max.x;
    temp.y = max.y;
    temp.z = min.z;
    transform(parent->inv_transf, &temp, &vertex);
    if (vertex.x < parent->min->x)
      parent->min->x = vertex.x;
    if (vertex.x > parent->max->x)
      parent->max->x = vertex.x;
    if (vertex.y < parent->min->y)
      parent->min->y = vertex.y;
    if (vertex.y > parent->max->y)
      parent->max->y = vertex.y;
    if (vertex.z < parent->min->z)
      parent->min->z = vertex.z;
    if (vertex.z > parent->max->z)
      parent->max->z = vertex.z;
    temp.x = max.x;
    temp.y = max.y;
    temp.z = max.z;
    transform(parent->inv_transf, &temp, &vertex);
    if (vertex.x < parent->min->x)
      parent->min->x = vertex.x;
    if (vertex.x > parent->max->x)
      parent->max->x = vertex.x;
    if (vertex.y < parent->min->y)
      parent->min->y = vertex.y;
    if (vertex.y > parent->max->y)
      parent->max->y = vertex.y;
    if (vertex.z < parent->min->z)
      parent->min->z = vertex.z;
    if (vertex.z > parent->max->z)
      parent->max->z = vertex.z;
  }
}
