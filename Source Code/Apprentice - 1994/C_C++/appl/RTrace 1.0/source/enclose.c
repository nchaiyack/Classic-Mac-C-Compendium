/*
 * Copyright (c) 1988, 1992 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * This code received contributions from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Reid Judd		- corrections
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
 *    RAY TRACING - Enclose - Version 7.3.1                           *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, July 1992              *
 **********************************************************************/

/***** Enclosing box *****/
static int
find_axis(first, last)
  int             first, last;
{
  REG int         i, axis;
  REG real        delta;
  xyz_struct      min, max;

  STRUCT_ASSIGN(min, *(object[first]->min));
  STRUCT_ASSIGN(max, *(object[first]->max));
  for (i = SUCC(first); i <= last; POSINC(i))
  {
    if (object[i]->min->x < min.x)
      min.x = object[i]->min->x;
    if (object[i]->max->x > max.x)
      max.x = object[i]->max->x;
    if (object[i]->min->y < min.y)
      min.y = object[i]->min->y;
    if (object[i]->max->y > max.y)
      max.y = object[i]->max->y;
    if (object[i]->min->z < min.z)
      min.z = object[i]->min->z;
    if (object[i]->max->z > max.z)
      max.z = object[i]->max->z;
  }
  delta = max.x - min.x;
  axis = X_AXIS;
  if (max.y - min.y > delta)
  {
    delta = max.y - min.y;
    axis = Y_AXIS;
  }
  if (max.z - min.z > delta)
    return Z_AXIS;
  return axis;
}
static boolean
compare(axis, first, second)
  int             axis, first, second;
{
  REG real        sum1, sum2;

  switch (axis)
  {
  case X_AXIS:
    sum1 = object[first]->min->x + object[first]->max->x;
    sum2 = object[second]->min->x + object[second]->max->x;
    break;
  case Y_AXIS:
    sum1 = object[first]->min->y + object[first]->max->y;
    sum2 = object[second]->min->y + object[second]->max->y;
    break;
  case Z_AXIS:
    sum1 = object[first]->min->z + object[first]->max->z;
    sum2 = object[second]->min->z + object[second]->max->z;
    break;
  }
  return (boolean) (sum1 <= sum2);
}
static void
sort(axis, first, size)
  int             axis, first, size;
/***** Shell Sort *****/
{
  REG int         step, i, j, first0;
  object_ptr      temp;

  step = size DIV 2;
  POSDEC(size);
  while (step > 0)
  {
    for (i = step; i <= size; POSINC(i))
    {
      j = i - step;
      while (j >= 0)
        if (compare(axis, first + j, first + j + step))
          break;                /* already in order */
        else
        {
          /* Out of order, so exchange */
          first0 = first + j;
          temp = object[first0];
          object[first0] = object[first0 + step];
          object[first0 + step] = temp;
          /* Check previous */
          j = j - step;
        }
    }
    step = step DIV 2;
  }
}
static void
sort_and_split(first, last)
  REG int         first, last;
{
  REG int         i, size, axis;
  xyz_struct      min, max;
  cluster_ptr     cluster;

  /* Find axis to sort */
  axis = find_axis(first, last);
  size = SUCC(last - first);
  /* Sort objects according to axis */
  sort(axis, first, size);
  if (size <= cluster_size)
  {
    /* Put objects in Cluster */
    ALLOCATE(cluster, cluster_struct, 1);
    ALLOCATE(cluster->object, void_ptr, size);
    cluster->size = size;
    for (i = 0; i < size; POSINC(i))
      cluster->object[i] = (void_ptr) (object[first + i]);
    POSINC(objects);
    if (objects >= OBJECTS_MAX)
      runtime_abort("too many OBJECTS and CLUSTERS");
    ALLOCATE(object[objects], object_struct, 1);
    object[objects]->id = objects;
    object[objects]->object_type = CLUSTER_TYPE;
    object[objects]->transf = NULL;
    object[objects]->inv_transf = NULL;
    object[objects]->texture = NULL;
    object[objects]->data = (void_ptr) cluster;
    ALLOCATE(object[objects]->min, xyz_struct, 1);
    ALLOCATE(object[objects]->max, xyz_struct, 1);
    STRUCT_ASSIGN(min, *(((object_ptr) (cluster->object[0]))->min));
    STRUCT_ASSIGN(max, *(((object_ptr) (cluster->object[0]))->max));
    for (i = 1; i < size; POSINC(i))
    {
    
		PROCESS_MAC_EVENT

      if (((object_ptr) (cluster->object[i]))->min->x < min.x)
        min.x = ((object_ptr) (cluster->object[i]))->min->x;
      if (((object_ptr) (cluster->object[i]))->max->x > max.x)
        max.x = ((object_ptr) (cluster->object[i]))->max->x;
      if (((object_ptr) (cluster->object[i]))->min->y < min.y)
        min.y = ((object_ptr) (cluster->object[i]))->min->y;
      if (((object_ptr) (cluster->object[i]))->max->y > max.y)
        max.y = ((object_ptr) (cluster->object[i]))->max->y;
      if (((object_ptr) (cluster->object[i]))->min->z < min.z)
        min.z = ((object_ptr) (cluster->object[i]))->min->z;
      if (((object_ptr) (cluster->object[i]))->max->z > max.z)
        max.z = ((object_ptr) (cluster->object[i]))->max->z;
    }
    STRUCT_ASSIGN(*(object[objects]->min), min);
    STRUCT_ASSIGN(*(object[objects]->max), max);
    ROOT_OBJECT = object[objects];
  } else
  {
    i = (first + last) DIV 2;
    sort_and_split(first, i);
    sort_and_split(SUCC(i), last);
  }
}

#define CSG_LEVEL1_MAX (OBJECTS_MAX / 3)
#define CSG_LEVEL2_MAX (OBJECTS_MAX - 1)

static void
csg_copy(csg_object, csg_objects_ptr, node)
  object_ptr     *csg_object;
  int            *csg_objects_ptr, node;
{
  csg_ptr         csg;

  if (object[node] == NULL)
    runtime_abort("invalid CSG TREE (empty NODE)");
  if (*csg_objects_ptr >= CSG_LEVEL2_MAX)
    runtime_abort("too many CSG secondary NODES");
  csg_object[*csg_objects_ptr] = object[node];
  POSINC(*csg_objects_ptr);
  if (object[node]->object_type != CSG_TYPE)
  {
    object[node] = NULL;
    return;
  }
  csg = (csg_ptr) object[node]->data;
  object[node] = NULL;
  node = csg->left;
  csg->left = *csg_objects_ptr;
  csg_copy(csg_object, csg_objects_ptr, node);
  node = csg->right;
  csg->right = *csg_objects_ptr;
  csg_copy(csg_object, csg_objects_ptr, node);
}
void
validate_object(object, id)
  object_ptr      object;
  int             id;
{
  object->id = id;
  if ((object->transf == NULL) AND(object->inv_transf != NULL))
  {
    FREE(object->inv_transf);
    object->inv_transf = NULL;
  }
  if (object->transf != NULL)
    normalize_transform(object->transf);
  if (object->inv_transf != NULL)
    normalize_transform(object->inv_transf);
}
void
enclose_all()
{
  REG int         i, j, total, first, last;
  int             main_objects, csg_created;
  int             csg_removed, non_csg_removed, csg_union_removed;
  int             csg_prim_objects, csg_sec_objects;
  csg_ptr         csg;
  texture_ptr     texture, old_texture;

#ifdef THINK_C
	
	/* The macintosh dynamically determines OBJECTS_MAX, which determines
		CSG_LEVELn_MAX.  So we must dynamically allocate the arrays */

	object_ptr	*csg_prim_object;
	object_ptr	*csg_sec_object;
  	
	ALLOCATE (csg_prim_object, object_ptr, CSG_LEVEL1_MAX);
	ALLOCATE (csg_sec_object, object_ptr, CSG_LEVEL1_MAX);


	/* Also update the status window to say we're enclosing */
	if (status_dialog_visible) set_status_text("\pEnclosing Objects…");
	
	/* Set the maximum value of the progress bar to be the number of objects */
	set_progress_bar_max(objects);

#else

  object_ptr      csg_prim_object[CSG_LEVEL1_MAX];
  object_ptr      csg_sec_object[CSG_LEVEL2_MAX];

#endif

  total = 0;
  for (i = 1; i <= objects; POSINC(i))
  {
  
#ifdef THINK_C

	/* On the mac, we update the progress bar periodically to show the number
		of objects read */
	if (status_dialog_visible) set_progress_bar_value(i);

	PROCESS_MAC_EVENT

#endif

    OBJECT_ENCLOSE(object[i]);
    if (CHECK_BOUNDS(object[i]->object_type))
      POSINC(total);
    if (object[i]->texture != NULL)
    {
      for(texture = object[i]->texture; texture != NULL;
          texture = (texture_ptr) texture->next)
        if (MODIFY_NORMAL(texture->type))
        {
          object[i]->texture_modify_normal = TRUE;
          break;
        }
    }
  }
  /* Enclose CSG objects; must be from last to first */
  csg_created = 0;
  csg_removed = 0;
  non_csg_removed = 0;
  for (i = objects; i > 0; POSDEC(i))
    if (object[i]->object_type == CSG_TYPE)
    {
      POSINC(csg_created);
      csg_enclose(i, &csg_removed, &non_csg_removed);

	PROCESS_MAC_EVENT

    }
  if (verbose_mode > 1)
  {
    if (total > 0)
      WRITE(results, "%d Object bounding volume(s) computed\n", total);
    if (csg_created > 0)
      WRITE(results, "%d CSG object(s) created\n", csg_created);
    if (csg_removed > 0)
      WRITE(results, "%d CSG object(s) removed\n", csg_removed);
    if (non_csg_removed > 0)
      WRITE(results, "%d Primitive object(s) removed\n", non_csg_removed);
    FLUSH(results);
  }
  main_objects = objects;
  if (csg_created > 0)
  {
    /* Rearrange object list; CSG objects are not processed */
    csg_prim_objects = 0;
    csg_sec_objects = 0;
    csg_union_removed = 0;
    if (csg_created > csg_removed)
      for (i = 1; i <= objects; POSINC(i))
      {

		PROCESS_MAC_EVENT

        if ((object[i] == NULL) OR(object[i]->object_type != CSG_TYPE))
          continue;
        csg = (csg_ptr) object[i]->data;
        /* Convert CSG union objects to normal objects */
        if ((csg->op == CSG_UNION)
            AND(object[i]->surface_id < 0)
	    AND(object[i]->refraction < 0.0)
	    AND(object[i]->texture == NULL)
	    AND(object[i]->transf == NULL))
        {
          /* Remove this CSG object */
          FREE(object[i]->min);
          FREE(object[i]->max);
          if (object[i]->transf != NULL)
            FREE(object[i]->transf);
          if (object[i]->inv_transf != NULL)
            FREE(object[i]->inv_transf);
          texture = object[i]->texture;
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
          FREE(object[i]->data);
          FREE(object[i]);
          object[i] = NULL;
	  POSINC(csg_union_removed);
	  POSDEC(csg_created);
	  POSINC(csg_removed);
	  continue;
        }
        if (csg_prim_objects >= CSG_LEVEL1_MAX)
          runtime_abort("too many CSG primary NODES");
        csg_prim_object[csg_prim_objects] = object[i];
        POSINC(csg_prim_objects);
        j = csg->left;
        csg->left = csg_sec_objects;
        csg_copy(csg_sec_object, &csg_sec_objects, j);
        j = csg->right;
        csg->right = csg_sec_objects;
        csg_copy(csg_sec_object, &csg_sec_objects, j);
        object[i] = NULL;
      }
    for (i = objects; i > 0; POSDEC(i))
      if (object[i] == NULL)
      {
	for (j = i + 1; j <= main_objects; POSINC(j))
	  object[j - 1] = object[j];
	POSDEC(main_objects);
      }
    if ((verbose_mode > 1) AND(csg_union_removed > 0))
    {
      WRITE(results, "%d CSG UNION object(s) removed\n", csg_union_removed);
      if (csg_created > 0)
        WRITE(results, "%d CSG object(s) created (total)\n", csg_created);
      if (csg_removed > 0)
        WRITE(results, "%d CSG object(s) removed (total)\n", csg_removed);
    }
    j = main_objects + 1 + csg_prim_objects;
    for (i = 0; i < csg_prim_objects; POSINC(i))
    {
      POSINC(main_objects);
      object[main_objects] = csg_prim_object[i];
      csg = (csg_ptr) object[main_objects]->data;
      csg->left += j;
      csg->right += j;
    }
    if (main_objects == 0)
      runtime_abort("no OBJECTS (after CSG sorting)");
  }
  /* Create cluster hierarchy */

#ifdef THINK_C

	/* On the mac, we change the status text to show we're creating the
		cluster hierarchy */
	if (status_dialog_visible) set_status_text("\pCreating Cluster Hierarchy…Please Wait…");

#endif

  objects = main_objects;
  i = objects;
  last = objects;
  sort_and_split(1, last);

  while (objects - last > 1)
  {
    first = SUCC(last);
    last = objects;
    sort_and_split(first, last);

	PROCESS_MAC_EVENT

  }
  if (verbose_mode > 1)
    WRITE(results, "%d Cluster object(s) created\n", objects - i);
  if (csg_created > 0)
  {
    /* Adjust CSG primary objects */
    j = objects - i;
    for (i = 1; i <= main_objects; POSINC(i))
    {
      if (object[i]->object_type != CSG_TYPE)
	continue;
      csg = (csg_ptr) object[i]->data;
      csg->left += j;
      csg->right += j;
    }
    /* Insert CSG secondary objects */
    j = objects + 1;
    for (i = 0; i < csg_sec_objects; POSINC(i))
    {
      POSINC(objects);
      if (objects >= OBJECTS_MAX)
        runtime_abort("too many OBJECTS");
      object[objects] = csg_sec_object[i];
      if (object[objects]->object_type != CSG_TYPE)
	continue;
      csg = (csg_ptr) object[objects]->data;
      csg->left += j;
      csg->right += j;
    }
  }
  if (verbose_mode > 1)
  {
    WRITE(results, "%d Total object(s)\n", objects);
    FLUSH(results);
  }
  /* Clean objects */
  for (i = 1; i <= objects; POSINC(i))
  {
    /* Sanity check */
    validate_object(object[i], i);
    if (object[i]->object_type == CSG_TYPE)
    {
      csg = (csg_ptr) object[i]->data;
      if ((csg->left < 1) OR(csg->left > objects))
      {
        WRITE(ERROR, "Incorrect CSG LEFT NODE pointer (%d) at OBJECT %d\n",
              csg->left, i);
        runtime_abort("invalid CSG NODE");
      }
      if ((csg->right < 1) OR(csg->right > objects))
      {
        WRITE(ERROR, "Incorrect CSG RIGHT pointer (%d) at OBJECT %d\n",
              csg->right, i);
        runtime_abort("invalid CSG NODE");
      }
    }
  }
  
#ifdef THINK_C
	
	/* Free the arrays we dynamically allocated */
	FREE (csg_prim_object);
	FREE (csg_sec_object);

#endif

}
