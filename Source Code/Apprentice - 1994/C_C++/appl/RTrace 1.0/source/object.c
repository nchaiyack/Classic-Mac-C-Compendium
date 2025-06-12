/*
 * Copyright (c) 1988, 1992 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * This code received contributions from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
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
#include "pp_ext.h"

/**********************************************************************
 *    RAY TRACING - Scene (Objects) - Version 7.3.1                   *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, July 1992              *
 **********************************************************************/

#define CREATE_OBJECT(type, object_data)\
do {\
  ALLOCATE(object[objects], object_struct, 1);\
  object[objects]->id = objects;\
  object[objects]->surface_id = surface_id;\
  object[objects]->refraction = refraction;\
  ALLOCATE(object[objects]->min, xyz_struct, 1);\
  ALLOCATE(object[objects]->max, xyz_struct, 1);\
  object[objects]->transf = NULL;\
  object[objects]->inv_transf = NULL;\
  object[objects]->texture = NULL;\
  object[objects]->texture_modify_normal = FALSE;\
  object[objects]->object_type = type;\
  object[objects]->data = (void_ptr) object_data;\
} while (0)

void
get_sphere()
{
  int             surface_id;
  real            value, refraction, radius;
  xyz_struct      center;
  sphere_ptr      sphere;

  get_valid(scene, &value, 1.0, (real) surfaces,
            "OBJECT (SPHERE) SURFACE ID");
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 1.0, REFRACTION_FACTOR_MAX,
            "OBJECT (SPHERE) REFRACTION Factor");
  refraction = value;
  get_valid(scene, &value, X_MIN, X_MAX, "OBJECT (SPHERE) CENTER X");
  center.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "OBJECT (SPHERE) CENTER Y");
  center.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "OBJECT (SPHERE) CENTER Z");
  center.z = value;
  get_valid(scene, &value, ROUNDOFF, X_MAX * 0.5, "OBJECT (SPHERE) Radius");
  radius = value;
  ADVANCE(scene);
  /* Create Sphere */
  ALLOCATE(sphere, sphere_struct, 1);
  STRUCT_ASSIGN(sphere->center, center);
  sphere->radius = radius;
  sphere->radius2 = SQR(radius);
  CREATE_OBJECT(SPHERE_TYPE, sphere);
}
void
get_box()
{
  int             surface_id;
  real            value, refraction;
  xyz_struct      center, size;
  box_ptr         box;

  get_valid(scene, &value, 1.0, (real) surfaces,
            "OBJECT (BOX) SURFACE ID");
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 1.0, REFRACTION_FACTOR_MAX,
            "OBJECT (BOX) REFRACTION Factor");
  refraction = value;
  get_valid(scene, &value, X_MIN, X_MAX, "OBJECT (BOX) CENTER X");
  center.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "OBJECT (BOX) CENTER Y");
  center.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "OBJECT (BOX) CENTER Z");
  center.z = value;
  get_valid(scene, &value, ROUNDOFF, X_MAX * 0.5, "OBJECT (BOX) SIZE X");
  size.x = value;
  get_valid(scene, &value, ROUNDOFF, Y_MAX * 0.5, "OBJECT (BOX) SIZE Y");
  size.y = value;
  get_valid(scene, &value, ROUNDOFF, Z_MAX * 0.5, "OBJECT (BOX) SIZE Z");
  size.z = value;
  ADVANCE(scene);
  /* Create Box */
  ALLOCATE(box, box_struct, 1);
  STRUCT_ASSIGN(box->center, center);
  STRUCT_ASSIGN(box->size, size);
  CREATE_OBJECT(BOX_TYPE, box);
}
void
get_patch()
{
  int             surface_id, i, vertices;
  real            value, refraction;
  xyz_struct      origin, scale;
  patch_ptr       patch;
  vertex_ptr      vertex_top, vertex_bottom, vertex;
  char            name[STRING_MAX];
  file_ptr        patch_file;

  get_valid(scene, &value, 1.0, (real) surfaces,
            "OBJECT (PATCH) SURFACE ID");
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 1.0, REFRACTION_FACTOR_MAX,
            "OBJECT (PATCH) REFRACTION Factor");
  refraction = value;
  get_valid(scene, &value, X_MIN, X_MAX, "OBJECT (PATCH) ORIGIN X");
  origin.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "OBJECT (PATCH) ORIGIN Y");
  origin.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "OBJECT (PATCH) ORIGIN Z");
  origin.z = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX, "OBJECT (PATCH) SCALE X");
  scale.x = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX, "OBJECT (PATCH) SCALE Y");
  scale.y = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX, "OBJECT (PATCH) SCALE Z");
  scale.z = value;
  READ_STRING(scene, name);
  ADVANCE(scene);
  if ((name[0] == '-') AND(name[1] == EOT))
    patch_file = scene;
  else
  {
    OPEN(patch_file, name, READ_TEXT);
    if (IO_status != IO_OK)
    {
      WRITE(results, "Error: unable to open PATCH (%s)\n", name);
      HALT;
    }
  }
  vertices = 0;
  vertex_top = NULL;
  while (NOT END_OF_LINE(patch_file))
  {
    ALLOCATE(patch, patch_struct, 1);
    for (i = 0; i < 12; POSINC(i))
    {
      get_valid(patch_file, &value, 1.0, INFINITY,
                "OBJECT (PATCH) VERTEX ID");
      patch->p[i] = vertex_pointer(ROUND(value), &vertices,
                                   &vertex_top, &vertex_bottom);
    }
    ADVANCE(patch_file);
    if (objects >= OBJECTS_MAX)
      runtime_abort("too many OBJECTS");
    /* Create Patch */
    CREATE_OBJECT(PATCH_TYPE, patch);
    POSINC(objects);
  }
  ADVANCE(patch_file);
  POSDEC(objects);
  patch->vertex = vertex_top;
  vertex = vertex_top;
  i = 0;
  while (NOT END_OF_LINE(patch_file))
  {
    if (vertex == NULL)
      runtime_abort("no OBJECT (PATCH) VERTEX Coordinates");
    POSINC(i);
    if (i > vertices)
      runtime_abort("too many OBJECT (PATCH) VERTEX Coordinates");
    get_valid(patch_file, &value, X_MIN, X_MAX, "OBJECT (PATCH) VERTEX X");
    vertex->coords.x = value * scale.x + origin.x;
    get_valid(patch_file, &value, Y_MIN, Y_MAX, "OBJECT (PATCH) VERTEX Y");
    vertex->coords.y = value * scale.y + origin.y;
    get_valid(patch_file, &value, Z_MIN, Z_MAX, "OBJECT (PATCH) VERTEX Z");
    vertex->coords.z = value * scale.z + origin.z;
    vertex = (vertex_ptr) (vertex->next);
    ADVANCE(patch_file);
  }
  if (i < vertices)
    runtime_abort("not enough OBJECT (PATCH) VERTEX Coordinates");
  if (patch_file != scene)
    CLOSE(patch_file);
  else
    ADVANCE(scene);
}
void
get_cone()
{
  int             surface_id;
  real            value, refraction, base_radius, apex_radius, k;
  xyz_struct      base, apex, temp;
  cone_ptr        cone;

  get_valid(scene, &value, 1.0, (real) surfaces,
            "OBJECT (CONE) SURFACE ID");
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 1.0, REFRACTION_FACTOR_MAX,
            "OBJECT (CONE) REFRACTION Factor");
  refraction = value;
  get_valid(scene, &value, X_MIN, X_MAX, "OBJECT (CONE) BASE X");
  base.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "OBJECT (CONE) BASE Y");
  base.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "OBJECT (CONE) BASE Z");
  base.z = value;
  get_valid(scene, &value, ROUNDOFF, X_MAX * 0.5,
            "OBJECT (CONE) BASE Radius");
  base_radius = value;
  get_valid(scene, &value, X_MIN, X_MAX, "OBJECT (CONE) APEX X");
  apex.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "OBJECT (CONE) APEX Y");
  apex.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "OBJECT (CONE) APEX Z");
  apex.z = value;
  get_valid(scene, &value, ROUNDOFF, X_MAX * 0.5,
            "OBJECT (CONE) APEX Radius");
  apex_radius = value;
  if (apex_radius > base_radius)
    runtime_abort("bad OBJECT (CONE) APEX Radius");
  ADVANCE(scene);
  /* Create Cone / Cylinder */
  ALLOCATE(cone, cone_struct, 1);
  STRUCT_ASSIGN(cone->base, base);
  STRUCT_ASSIGN(cone->apex, apex);
  cone->base_radius = base_radius;
  cone->apex_radius = apex_radius;
  cone->w.x = apex.x - base.x;
  cone->w.y = apex.y - base.y;
  cone->w.z = apex.z - base.z;
  cone->height = LENGTH(cone->w);
  if (cone->height < ROUNDOFF)
    runtime_abort("bad OBJECT (CONE) APEX and BASE");
  NORMALIZE(cone->w);
  cone->slope = (apex_radius - base_radius) / cone->height;
  cone->base_d = -DOT_PRODUCT(base, cone->w);
  if (ABS(cone->w.z) > 1.0 - ROUNDOFF)
  {
    temp.x = 0.0;
    temp.y = 1.0;
    temp.z = 0.0;
  } else
  {
    temp.x = 0.0;
    temp.y = 0.0;
    temp.z = 1.0;
  }
  CROSS_PRODUCT(cone->u, cone->w, temp);
  NORMALIZE(cone->u);
  CROSS_PRODUCT(cone->v, cone->u, cone->w);
  cone->min_d = DOT_PRODUCT(cone->w, base);
  cone->max_d = DOT_PRODUCT(cone->w, apex);
  if (cone->max_d < cone->min_d)
  {
    k = cone->max_d;
    cone->max_d = cone->min_d;
    cone->min_d = k;
  }
  CREATE_OBJECT(CONE_TYPE, cone);
}
typedef
struct
{
  vertex_ptr     *point;
  void_ptr        next;
} data_struct;
typedef
data_struct    *data_ptr;

void
get_polygon()
{
  int             surface_id, i, j, first, vertices;
  real            value, refraction;
  xyz_struct      origin, scale;
  polygon_ptr     polygon;
  vertex_ptr      vertex_top, vertex_bottom, vertex;
  data_ptr        data_top, data_bottom, data;
  char            name[STRING_MAX];
  file_ptr        poly_file;

  get_valid(scene, &value, 1.0, (real) surfaces,
            "OBJECT (POLYGON) SURFACE ID");
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 1.0, REFRACTION_FACTOR_MAX,
            "OBJECT (POLYGON) REFRACTION Factor");
  refraction = value;
  get_valid(scene, &value, X_MIN, X_MAX, "OBJECT (POLYGON) ORIGIN X");
  origin.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "OBJECT (POLYGON) ORIGIN Y");
  origin.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "OBJECT (POLYGON) ORIGIN Z");
  origin.z = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX,
            "OBJECT (POLYGON) SCALE X");
  scale.x = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX,
            "OBJECT (POLYGON) SCALE Y");
  scale.y = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX,
            "OBJECT (POLYGON) SCALE Z");
  scale.z = value;
  READ_STRING(scene, name);
  ADVANCE(scene);
  if ((name[0] == '-') AND(name[1] == EOT))
    poly_file = scene;
  else
  {
    OPEN(poly_file, name, READ_TEXT);
    if (IO_status != IO_OK)
    {
      WRITE(results, "Error: unable to open POLYGON (%s)\n", name);
      HALT;
    }
  }
  first = objects;
  data_top = NULL;
  vertices = 0;
  vertex_top = NULL;
  while (NOT END_OF_LINE(poly_file))
  {
    ALLOCATE(polygon, polygon_struct, 1);
    ALLOCATE(data, data_struct, 1);
    if (data_top == NULL)
      data_top = data;
    else
      data_bottom->next = (void_ptr) data;
    data_bottom = data;
    get_valid(poly_file, &value, 3.0, (real) MAXINT,
              "OBJECT (POLYGON) VERTICES Polygon");
    polygon->points = PRED(ROUND(value));
    ALLOCATE(data->point, vertex_ptr, SUCC(polygon->points));
    ALLOCATE(polygon->coords, array1, SUCC(polygon->points));
    for (i = 0; i <= polygon->points; POSINC(i))
    {
      get_valid(poly_file, &value, 1.0, INFINITY,
                "OBJECT (POLYGON) VERTEX ID");
      data->point[i] = vertex_pointer(ROUND(value), &vertices,
                                      &vertex_top, &vertex_bottom);
    }
    data->next = NULL;
    ADVANCE(poly_file);
    if (objects >= OBJECTS_MAX)
      runtime_abort("too many OBJECTS");
    /* Create Polygon */
    CREATE_OBJECT(POLYGON_TYPE, polygon);
    POSINC(objects);

#ifdef THINK_C

	/* On the mac, we update the sub_progress bar periodically to show the number
		of objects read */
	if (status_dialog_visible) set_progress_bar_value(objects);

#endif

	PROCESS_MAC_EVENT

  }
  ADVANCE(poly_file);
  POSDEC(objects);
  vertex = vertex_top;
  i = 0;
  while (NOT END_OF_LINE(poly_file))
  {
    if (vertex == NULL)
      runtime_abort("no OBJECT (POLYGON) VERTEX Coordinates");
    POSINC(i);
    if (i > vertices)
      runtime_abort("too many OBJECT (POLYGON) VERTEX Coordinates");
    get_valid(poly_file, &value, X_MIN, X_MAX,
              "OBJECT (POLYGON) VERTEX X");
    vertex->coords.x = value * scale.x + origin.x;
    get_valid(poly_file, &value, Y_MIN, Y_MAX,
              "OBJECT (POLYGON) VERTEX Y");
    vertex->coords.y = value * scale.y + origin.y;
    get_valid(poly_file, &value, Z_MIN, Z_MAX,
              "OBJECT (POLYGON) VERTEX Z");
    vertex->coords.z = value * scale.z + origin.z;
    vertex = (vertex_ptr) (vertex->next);
    ADVANCE(poly_file);
  }
  if (i < vertices)
    runtime_abort("not enough OBJECT (POLYGON) VERTEX Coordinates");
  vertex_bottom->next = NULL;
  data = data_top;

#ifdef THINK_C

	/* On the mac, we change the status text to show we're generating a polygon */
	if (status_dialog_visible) set_status_text("\pGenerating Polygon…");

	/* Set the bar to show the loop progress */
	if (objects != first)
		set_sub_progress_bar_max(objects-first);

#endif

  for (i = first; i <= objects; POSINC(i))
  {
    polygon = (polygon_ptr) object[i]->data;
    for (j = 0; j <= polygon->points; POSINC(j))
    {
      polygon->coords[j][0] = data->point[j]->coords.x;
      polygon->coords[j][1] = data->point[j]->coords.y;
      polygon->coords[j][2] = data->point[j]->coords.z;
    }
    data_top = data;
    data = (data_ptr) (data->next);
    FREE(data_top);

#ifdef THINK_C

	/* On the mac, we set the sub-progress bar to show loop progress */
	if (status_dialog_visible) set_sub_progress_bar_value(i-first);

	PROCESS_MAC_EVENT

#endif

  }
  
#ifdef THINK_C

	/* On the mac, we change the status text to show we're cleaning up */
	if (status_dialog_visible) set_status_text("\pCleaning up after Polygon…");

	/* Set the bar to show the loop progress */
	set_sub_progress_bar_max(vertices);
	i = 0;

#endif

  while (vertex_top != NULL)
  {
    vertex = vertex_top;
    vertex_top = (vertex_ptr) (vertex_top->next);
    FREE(vertex);

#ifdef THINK_C

	/* On the mac, we set the sub_progress bar to show vertex freeing progress */
	if (status_dialog_visible) set_sub_progress_bar_value(++i);

#endif

  }
  if (poly_file != scene)
    CLOSE(poly_file);
  else
    ADVANCE(scene);

#ifdef THINK_C

	/* On the mac, we change the status text to show we're reading objects again */
	if (status_dialog_visible)
		{
		set_status_text("\pReading Objects…");
	
		/* And set the sub-progress bar back to zero */
		set_sub_progress_bar_value(0);
		}

#endif

}
void
get_triangle()
{
  int             surface_id, i;
  real            value, refraction;
  xyz_struct      origin, scale;
  triangle_ptr    triangle;
  char            name[STRING_MAX];
  file_ptr        tri_file;

  get_valid(scene, &value, 1.0, (real) surfaces,
            "OBJECT (TRIANGLE) SURFACE ID");
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 1.0, REFRACTION_FACTOR_MAX,
            "OBJECT (TRIANGLE) REFRACTION Factor");
  refraction = value;
  get_valid(scene, &value, X_MIN, X_MAX, "OBJECT (TRIANGLE) ORIGIN X");
  origin.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "OBJECT (TRIANGLE) ORIGIN Y");
  origin.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "OBJECT (TRIANGLE) ORIGIN Z");
  origin.z = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX,
            "OBJECT (TRIANGLE) SCALE X");
  scale.x = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX,
            "OBJECT (TRIANGLE) SCALE Y");
  scale.y = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX,
            "OBJECT (TRIANGLE) SCALE Z");
  scale.z = value;
  READ_STRING(scene, name);
  ADVANCE(scene);
  if ((name[0] == '-') AND(name[1] == EOT))
    tri_file = scene;
  else
  {
    OPEN(tri_file, name, READ_TEXT);
    if (IO_status != IO_OK)
    {
      WRITE(results, "Error: unable to open TRIANGLE (%s)\n", name);
      HALT;
    }
  }
  while (NOT END_OF_LINE(tri_file))
  {
    ALLOCATE(triangle, triangle_struct, 1);
    for (i = 0; i < 3; POSINC(i))
    {
      get_valid(tri_file, &value, X_MIN, X_MAX,
                "OBJECT (TRIANGLE) VERTEX Coord X");
      triangle->coords[i].x = value * scale.x + origin.x;
      get_valid(tri_file, &value, Y_MIN, Y_MAX,
                "OBJECT (TRIANGLE) VERTEX Coord Y");
      triangle->coords[i].y = value * scale.y + origin.y;
      get_valid(tri_file, &value, Z_MIN, Z_MAX,
                "OBJECT (TRIANGLE) VERTEX Coord Z");
      triangle->coords[i].z = value * scale.z + origin.z;
      get_valid(tri_file, &value, X_MIN, X_MAX,
                "OBJECT (TRIANGLE) VERTEX Normal X");
      triangle->normal[i].x = value;
      get_valid(tri_file, &value, Y_MIN, Y_MAX,
                "OBJECT (TRIANGLE) VERTEX Normal Y");
      triangle->normal[i].y = value;
      get_valid(tri_file, &value, Z_MIN, Z_MAX,
                "OBJECT (TRIANGLE) VERTEX Normal Z");
      triangle->normal[i].z = value;
      if (LENGTH(triangle->normal[i]) < ROUNDOFF)
        runtime_abort("no OBJECT (TRIANGLE) VERTEX Normal");
    }
    ADVANCE(tri_file);
    if (objects >= OBJECTS_MAX)
      runtime_abort("too many OBJECTS");
    /* Create Triangle */
    CREATE_OBJECT(TRIANGLE_TYPE, triangle);
    POSINC(objects);
  }
  POSDEC(objects);
  if (tri_file != scene)
    CLOSE(tri_file);
  else
    ADVANCE(scene);
}
void
get_text()
{
  real            value;
  char            name[STRING_MAX];
  file_ptr        text_file;

  get_valid(scene, &value, 1.0, (real) surfaces,
            "OBJECT (TEXT) SURFACE ID");
  pp_surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 1.0, REFRACTION_FACTOR_MAX,
            "OBJECT (TEXT) REFRACTION Factor");
  pp_refraction = value;
  READ_STRING(scene, name);
  ADVANCE(scene);
  if ((name[0] == '-') AND(name[1] == EOT))
    text_file = scene;
  else
  {
    OPEN(text_file, name, READ_TEXT);
    if (IO_status != IO_OK)
    {
      WRITE(results, "Error: unable to open TEXT (%s)\n", name);
      HALT;
    }
  }
  /* Create Text */
  POSDEC(objects);
  get_pp_obj(text_file);
  if (text_file != scene)
    CLOSE(text_file);
}

void
get_csg()
{
  int             surface_id;
  real            value, refraction;
  csg_ptr         csg;
  int             op;

  get_valid(scene, &value, 0.0, (real) surfaces,
            "OBJECT (CSG) SURFACE ID");
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 0.0, REFRACTION_FACTOR_MAX,
            "OBJECT (CSG) REFRACTION Factor");
  if (value < 1.0 - ROUNDOFF)
    refraction = -1.0;
  else
    refraction = value;
  get_valid(scene, &value, (real) CSG_UNION, (real) CSG_INTERSECTION,
	    "OBJECT (CSG) OPERATION");
  op = ROUND(value);
  ADVANCE(scene);
  /* Create CSG */
  ALLOCATE(csg, csg_struct, 1);
  switch (op)
  {
  case CSG_UNION:
    csg->op = CSG_UNION;
    break;
  case CSG_SUBTRACTION:
    csg->op = CSG_SUBTRACTION;
    break;
  case CSG_INTERSECTION:
    csg->op = CSG_INTERSECTION;
    break;
  }
  CREATE_OBJECT(CSG_TYPE, csg);
}

void
get_list()
{
  int             surface_id;
  real            value, refraction;
  list_ptr        list;
  int             op;

  get_valid(scene, &value, 0.0, (real) surfaces,
            "OBJECT (LIST) SURFACE ID");
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 0.0, REFRACTION_FACTOR_MAX,
            "OBJECT (LIST) REFRACTION Factor");
  if (value < 1.0 - ROUNDOFF)
    refraction = -1.0;
  else
    refraction = value;
  ADVANCE(scene);
  /* Create List */
  ALLOCATE(list, list_struct, 1);
  CREATE_OBJECT(LIST_TYPE, list);
}

#define MULTIPLY(result, first, second, index, coord)\
do {\
  result[index].coord = first[index].x * second[0].coord +\
                        first[index].y * second[1].coord +\
                        first[index].z * second[2].coord +\
                        first[index].w * second[3].coord;\
} while (0)

void
get_object_transform(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  boolean         null_transf;
  int             i, id;
  real            value;
  boolean         first_transf;
  xyzw_ptr        transf, new_transf;

  get_valid(scene, &value, 0.0, (real) scene_objects, "OBJECT ID");
  id = PRED(ROUND(value));
  if (id < 0)
    id = PRED(scene_objects);
  ALLOCATE(transf, xyzw_struct, 4);
  null_transf = TRUE;
  get_valid(scene, &(transf[0].x), X_MIN, X_MAX, "TRANSFORM X0");
  null_transf = null_transf AND ABS(transf[0].x) < ROUNDOFF;
  get_valid(scene, &(transf[0].y), Y_MIN, Y_MAX, "TRANSFORM Y0");
  null_transf = null_transf AND ABS(transf[0].y) < ROUNDOFF;
  get_valid(scene, &(transf[0].z), Z_MIN, Z_MAX, "TRANSFORM Z0");
  null_transf = null_transf AND ABS(transf[0].z) < ROUNDOFF;
  get_valid(scene, &(transf[0].w), W_MIN, W_MAX, "TRANSFORM W0");
  null_transf = null_transf AND ABS(transf[0].w) < ROUNDOFF;
  get_valid(scene, &(transf[1].x), X_MIN, X_MAX, "TRANSFORM X1");
  null_transf = null_transf AND ABS(transf[1].x) < ROUNDOFF;
  get_valid(scene, &(transf[1].y), Y_MIN, Y_MAX, "TRANSFORM Y1");
  null_transf = null_transf AND ABS(transf[1].y) < ROUNDOFF;
  get_valid(scene, &(transf[1].z), Z_MIN, Z_MAX, "TRANSFORM Z1");
  null_transf = null_transf AND ABS(transf[1].z) < ROUNDOFF;
  get_valid(scene, &(transf[1].w), W_MIN, W_MAX, "TRANSFORM W1");
  null_transf = null_transf AND ABS(transf[1].w) < ROUNDOFF;
  get_valid(scene, &(transf[2].x), X_MIN, X_MAX, "TRANSFORM X2");
  null_transf = null_transf AND ABS(transf[2].x) < ROUNDOFF;
  get_valid(scene, &(transf[2].y), Y_MIN, Y_MAX, "TRANSFORM Y2");
  null_transf = null_transf AND ABS(transf[2].y) < ROUNDOFF;
  get_valid(scene, &(transf[2].z), Z_MIN, Z_MAX, "TRANSFORM Z2");
  null_transf = null_transf AND ABS(transf[2].z) < ROUNDOFF;
  get_valid(scene, &(transf[2].w), W_MIN, W_MAX, "TRANSFORM W2");
  null_transf = null_transf AND ABS(transf[2].w) < ROUNDOFF;
  get_valid(scene, &(transf[3].x), X_MIN, X_MAX, "TRANSFORM X3");
  null_transf = null_transf AND ABS(transf[3].x) < ROUNDOFF;
  get_valid(scene, &(transf[3].y), Y_MIN, Y_MAX, "TRANSFORM Y3");
  null_transf = null_transf AND ABS(transf[3].y) < ROUNDOFF;
  get_valid(scene, &(transf[3].z), Z_MIN, Z_MAX, "TRANSFORM Z3");
  null_transf = null_transf AND ABS(transf[3].z) < ROUNDOFF;
  get_valid(scene, &(transf[3].w), W_MIN, W_MAX, "TRANSFORM W3");
  null_transf = null_transf AND ABS(transf[3].w) < ROUNDOFF;
  ADVANCE(scene);
  if (null_transf)
  {
    FREE(transf);
    return;
  }
  first_transf = FALSE;
  for (i = first_object[id]; i <= last_object[id]; POSINC(i))
  {
    if (object[i]->transf == NULL)
    {
      object[i]->transf = transf;
      first_transf = TRUE;
    } else
    {
      ALLOCATE(new_transf, xyzw_struct, 4);
      MULTIPLY(new_transf, transf, object[i]->transf, 0, x);
      MULTIPLY(new_transf, transf, object[i]->transf, 0, y);
      MULTIPLY(new_transf, transf, object[i]->transf, 0, z);
      MULTIPLY(new_transf, transf, object[i]->transf, 0, w);
      MULTIPLY(new_transf, transf, object[i]->transf, 1, x);
      MULTIPLY(new_transf, transf, object[i]->transf, 1, y);
      MULTIPLY(new_transf, transf, object[i]->transf, 1, z);
      MULTIPLY(new_transf, transf, object[i]->transf, 1, w);
      MULTIPLY(new_transf, transf, object[i]->transf, 2, x);
      MULTIPLY(new_transf, transf, object[i]->transf, 2, y);
      MULTIPLY(new_transf, transf, object[i]->transf, 2, z);
      MULTIPLY(new_transf, transf, object[i]->transf, 2, w);
      MULTIPLY(new_transf, transf, object[i]->transf, 3, x);
      MULTIPLY(new_transf, transf, object[i]->transf, 3, y);
      MULTIPLY(new_transf, transf, object[i]->transf, 3, z);
      MULTIPLY(new_transf, transf, object[i]->transf, 3, w);
      if (i == last_object[id])
        FREE(object[i]->transf);
      object[i]->transf = new_transf;
    }
  }
  if (NOT first_transf)
    FREE(transf);
}
