/*
 * Copyright (c) 1988, 1992 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * This code received contributions from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Paul Strauss        - shading model
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

/**********************************************************************
 *    RAY TRACING - Data - Version 7.3.1                              *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, July 1992              *
 **********************************************************************/

/***** Global Variables *****/
short int       antialiasing_mode;
short int       view_mode;
short int       texture_mode;
short int       light_mode;
short int       last_shade_level;
short int       background_mode;
short int       sampling_levels;
short int       sampling_divisions;
short int       sampling_weight;
short int       cluster_size;
short int       ambient_sample_rays;
short int       last_ambient_level;
short int       distributed_cache_mode;
short int       distributed_cache_repetitions;
short int       intersect_mode;
short int       intersect_adjust_mode;
short int       jittering_mode;
short int       raw_mode;
short int       normal_mode;
short int       normal_check_mode;
short int       shade_mode;
short int       output_format;
short int       verbose_mode;
int             objects;        /* Number of Objects         */
short int       lights;         /* Number of Light sources   */
short int       surfaces;       /* Number of Surfaces        */
short int       screen_size_x, screen_size_y;   /* Screen dimensions */
short int       shade_level, shade_level_max;
short int       pqueue_size;
short int       ray_node;

real            gaze_distance, light_distance;
real            pixel_distance, threshold_distance;
real            view_angle_x, view_angle_y;     /* View field angles */
real            threshold_level, threshold_color, threshold_vector;
real            focal_aperture, focal_distance;
real            stereo_separation;

real            eye_rays, reflected_rays, refracted_rays, shadow_rays;
real            ambient_rays;
real            shadow_hits, shadow_cache_hits;
real            octant_tests, bound_tests, sphere_tests, box_tests,
                patch_tests, cone_tests, polygon_tests,
                triangle_tests, text_tests, csg_tests, list_tests;
real            pqueue_resets, pqueue_insertions, pqueue_extractions;
real            ray_hits, ray_cache_resets, ray_cache_hits;
real            distributed_cache_resets, distributed_cache_hits;
real            pixel_divisions;

rgb_struct      back_color,     /* Background color */
                light_ambient;  /* Ambient lighting */

xyz_struct      eye, look, up, gaze, screen_x, screen_y;

int             ray_cache[RAY_SIZE_MAX];

object_ptr     *object;

surface_ptr    *surface;


#ifdef THINK_C

/* On the mac, these are allocated dynamically from the mac segment */

rgb_struct		*true_color;
real			*back_mask;

#else

rgb_struct      true_color[SCREEN_SIZE_X_MAX];

real            back_mask[SCREEN_SIZE_X_MAX];

#endif

file_ptr        scene, picture, results, background, raw_picture;

light_struct    light[LIGHTS_MAX];

pqueue_struct  *pqueue;

pixel_ptr       new_line, old_line;

wave_struct     wave;

