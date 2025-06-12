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

/**********************************************************************
 *    RAY TRACING - Parameters - Version 7.3.2                        *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, August 1992            *
 **********************************************************************/

/***** Command line parameters *****/
#define THRESHOLD_COLOR(x) (3.0 * SQR(x))

static void
defaults()
{
  screen_size_x = 0;
  screen_size_y = 0;
  threshold_level = 0.01;
  threshold_color = THRESHOLD_COLOR(0.05);
  last_shade_level = 7;
  sampling_levels = 0;
  sampling_divisions = 1 SHL sampling_levels;
  sampling_weight = 4 * SQR(sampling_divisions);
  cluster_size = 4;
  last_ambient_level = 0;
  ambient_sample_rays = 16;
  antialiasing_mode = 0;
  background_mode = 0;
  raw_mode = 0;
  light_mode = 0;
  shade_mode = 1;
  normal_mode = 0;
  normal_check_mode = 0;
  texture_mode = 0;
  view_mode = 0;
  intersect_mode = 0;
  intersect_adjust_mode = 0;
  jittering_mode = 0;
  distributed_cache_mode = 0;
  focal_aperture = 0.0;
  focal_distance = 0.0;
  stereo_separation = 0.0;
  output_format = 0;	/* PIC */
#ifdef ECHO
  verbose_mode = 3;
#else
  verbose_mode = 2;
#endif
}
static void
usage()
{
#ifndef lint
  extern char     copyright[];

  WRITE(ERROR, "%s", &(copyright[5]));
  WRITE(ERROR, "Version %s created on %s %s.\n",
        PROGRAM_VERSION, __DATE__, __TIME__);
#endif
  WRITE(ERROR, "Usage: {PROGRAM}\n");
  WRITE(ERROR, "  [w]Screen Width       -> 16..%d (256)\n",
        PRED(SCREEN_SIZE_X_MAX));
  WRITE(ERROR, "  [h]Screen Height      -> 16..%d (256)\n",
        PRED(SCREEN_SIZE_Y_MAX));
  WRITE(ERROR, "  [p]Sampling Levels    -> 0..%d (0)\n", SAMPLING_LEVEL_MAX);
  WRITE(ERROR, "  [A]Aliasing Threshold -> 0..1 (0.05)\n");
  WRITE(ERROR, "  [s]Shading Levels     -> 1..%d (8)\n", last_shade_level);
  WRITE(ERROR, "  [S]Shading Threshold  -> 0..1 (0.01) \n");
  WRITE(ERROR, "  [d]Ambient Levels     -> 0..%d (0)\n", last_shade_level);
  WRITE(ERROR, "  [D]Ambient Samples    -> 2..%d (16)\n", SCREEN_SIZE_X_MAX);
  WRITE(ERROR, "  [T]Ambient Threshold  -> 0..1 (0)\n");
  WRITE(ERROR, "  [c]Cluster Size       -> %d..%d (4)\n",
        CLUSTER_SIZE_MIN, CLUSTER_SIZE_MAX);
  WRITE(ERROR, "  [a]Antialiasing Mode  -> 0, 1, 2 (0)\n");
  WRITE(ERROR, "  [i]Intersect Mode     -> 0, 1 (0)\n");
  WRITE(ERROR, "  [I]Inter. Adjust Mode -> 0, 1 (0)\n");
  WRITE(ERROR, "  [j]Jittering Mode     -> 0, 1 (0)\n");
  WRITE(ERROR, "  [l]Lighting Mode      -> 0, 1, 2 (0)\n");
  WRITE(ERROR, "  [m]Shading Mode       -> 0, 1 (1)\n");
  WRITE(ERROR, "  [n]Normal Mode        -> 0, 1 (0)\n");
  WRITE(ERROR, "  [t]Texture Mode       -> 0, 1, 2 (0)\n");
  WRITE(ERROR, "  [v]View Mode          -> 0, 1, 2 (0)\n");
  WRITE(ERROR, "  [z]Normal Check Mode  -> 0, 1 (0)\n");
  WRITE(ERROR, "  [B]Mask Back File     -> FILE (PIC format)\n");
  WRITE(ERROR, "  [R]Raw Picture File   -> FILE (PIC format)\n");
  WRITE(ERROR, "  [F]Focal Distance     -> 0..%0.0f (GAZE)\n", X_MAX);
  WRITE(ERROR, "  [E]Stereo Separation  -> 0..%0.0f\n", X_MAX);
  WRITE(ERROR, "  [P]Focal Aperture     -> 0..1\n");
  WRITE(ERROR, "  [O]Output Format      -> 0 - PIC format, 1 - PPM (0)\n");
  WRITE(ERROR, "  [V]Verbose Mode       -> 0, 1, 2, 3 (2)\n");
  WRITE(ERROR, "  {-|INFILE} {-|OUTFILE} [>LOGFILE]\n");
}
#define MIN_PARAMETERS (2)

#define GET_INT(number)\
do { PROCESS_MAC_EVENT\
  IO_status = (sscanf(&(parameter[option][1]), "%hd", &(number)) == 1) ?\
    IO_OK : IO_READ;\
} while (0)

#define GET_REAL(number)\
do { PROCESS_MAC_EVENT\
  IO_status = (sscanf(&(parameter[option][1]), "%lf", &(number)) == 1) ?\
    IO_OK : IO_READ;\
} while (0)

void
get_parameters(parameters, parameter)
  int             parameters;   /* Command parameter count */
  char_ptr        parameter[];
{
  int             option;

  defaults();
  if (parameters < MIN_PARAMETERS)
  {
    WRITE(ERROR, "Error: PARAMETER(S) missing\n");
    usage();
    HALT;
  }
  for (option = 0; option < parameters - MIN_PARAMETERS; POSINC(option))
  {
    switch (parameter[option][0])
    {
      case 'a':
        GET_INT(antialiasing_mode);
        if ((IO_status != IO_OK)
            OR(antialiasing_mode < 0) OR(antialiasing_mode > 2))
        {
          WRITE(ERROR, "Error: bad ANTIALIASING MODE\n");
          HALT;
        }
        break;

      case 'c':
        GET_INT(cluster_size);
        if ((IO_status != IO_OK)
            OR(cluster_size < CLUSTER_SIZE_MIN)
            OR(cluster_size > CLUSTER_SIZE_MAX))
        {
          WRITE(ERROR, "Error: bad CLUSTER SIZE\n");
          HALT;
        }
        break;

      case 'd':
        GET_INT(last_ambient_level);
        if ((IO_status != IO_OK)
            OR(last_ambient_level < 0)
            OR(last_ambient_level > last_shade_level))
        {
          WRITE(ERROR, "Error: bad AMBIENT LEVELS\n");
          HALT;
        }
        break;

      case 'h':
        GET_INT(screen_size_y);
        if ((IO_status != IO_OK) OR(screen_size_y < 16)
            OR(screen_size_y >= SCREEN_SIZE_Y_MAX))
        {
          WRITE(ERROR, "Error: bad SCREEN HEIGHT\n");
          HALT;
        }
        break;

      case 'i':
        GET_INT(intersect_mode);
        if ((IO_status != IO_OK)
            OR(intersect_mode < 0) OR(intersect_mode > 1))
        {
          WRITE(ERROR, "Error: bad INTERSECT MODE\n");
          HALT;
        }
        break;

      case 'j':
        GET_INT(jittering_mode);
        if ((IO_status != IO_OK)
            OR(jittering_mode < 0) OR(jittering_mode > 1))
        {
          WRITE(ERROR, "Error: bad JITTERING MODE\n");
          HALT;
        }
        break;

      case 'l':
        GET_INT(light_mode);
        if ((IO_status != IO_OK)
            OR(light_mode < 0) OR(light_mode > 2))
        {
          WRITE(ERROR, "Error: bad LIGHTING MODE\n");
          HALT;
        }
        break;

      case 'm':
        GET_INT(shade_mode);
        if ((IO_status != IO_OK)
            OR(shade_mode < 0) OR(shade_mode > 1))
        {
          WRITE(ERROR, "Error: bad SHADING MODE\n");
          HALT;
        }
        break;

      case 'n':
        GET_INT(normal_mode);
        if ((IO_status != IO_OK)
            OR(normal_mode < 0) OR(normal_mode > 1))
        {
          WRITE(ERROR, "Error: bad NORMAL MODE\n");
          HALT;
        }
        break;

      case 'p':
        GET_INT(sampling_levels);
        if ((IO_status != IO_OK)
            OR(sampling_levels < 0) OR(sampling_levels > SAMPLING_LEVEL_MAX))
        {
          WRITE(ERROR, "Error: bad SAMPLING LEVEL(S)\n");
          HALT;
        }
        sampling_divisions = 1 SHL sampling_levels;
        sampling_weight = 4 * SQR(sampling_divisions);
        break;

      case 's':
        GET_INT(last_shade_level);
        if ((IO_status != IO_OK) OR(last_shade_level <= 0))
        {
          WRITE(ERROR, "Error: bad SHADING LEVEL(S)\n");
          HALT;
        }
        POSDEC(last_shade_level);
        break;

      case 't':
        GET_INT(texture_mode);
        if ((IO_status != IO_OK)
            OR(texture_mode < 0) OR(texture_mode > 2))
        {
          WRITE(ERROR, "Error: bad TEXTURE MODE\n");
          HALT;
        }
        break;

      case 'v':
        GET_INT(view_mode);
        if ((IO_status != IO_OK)
            OR(view_mode < 0) OR(view_mode > 2))
        {
          WRITE(ERROR, "Error: bad VIEW MODE\n");
          HALT;
        }
        break;

      case 'w':
        GET_INT(screen_size_x);
        if ((IO_status != IO_OK) OR(screen_size_x < 16)
            OR(screen_size_x >= SCREEN_SIZE_X_MAX))
        {
          WRITE(ERROR, "Error: bad SCREEN WIDTH\n");
          HALT;
        }
        break;

      case 'z':
        GET_INT(normal_check_mode);
        if ((IO_status != IO_OK)
            OR(normal_check_mode < 0) OR(normal_check_mode > 1))
        {
          WRITE(ERROR, "Error: bad NORMAL CHECK MODE\n");
          HALT;
        }
        break;

      case 'A':
        GET_REAL(threshold_color);
        if ((IO_status != IO_OK) OR(threshold_color <= 0.0)
            OR(threshold_color >= 1.0))
        {
          WRITE(ERROR, "Error: bad ALIASING THRESHOLD\n");
          HALT;
        }
        threshold_color = SQR(threshold_color) * 3.0;
        break;

      case 'B':
        if (background_mode == 1)
        {
          WRITE(ERROR, "Error: BACKGROUND MODE already set\n");
          HALT;
        }
        OPEN(background, &(parameter[option][1]), WRITE_BINARY);
        if (IO_status != IO_OK)
        {
          WRITE(ERROR, "Error: unable to open BACKGROUND MASK FILE (%s)\n",
                &(parameter[option][1]));
          HALT;
        }
        background_mode = 1;
        break;

      case 'D':
        GET_INT(ambient_sample_rays);
        if ((IO_status != IO_OK)
            OR(ambient_sample_rays < 2)
            OR(ambient_sample_rays > SCREEN_SIZE_X_MAX))
        {
          WRITE(ERROR, "Error: bad AMBIENT SAMPLES\n");
          HALT;
        }
        break;

      case 'E':
        GET_REAL(stereo_separation);
        if ((IO_status != IO_OK)
            OR(stereo_separation < -10.0) OR(stereo_separation > X_MAX))
        {
          WRITE(ERROR, "Error: bad STEREO SEPARATION\n");
          HALT;
        }
        break;

      case 'F':
        GET_REAL(focal_distance);
        if ((IO_status != IO_OK)
            OR(focal_distance < ROUNDOFF) OR(focal_distance > X_MAX))
        {
          WRITE(ERROR, "Error: bad FOCAL DISTANCE\n");
          HALT;
        }
        break;

      case 'I':
        GET_INT(intersect_adjust_mode);
        if ((IO_status != IO_OK)
            OR(intersect_adjust_mode < 0) OR(intersect_adjust_mode > 1))
        {
          WRITE(ERROR, "Error: bad INTERSECT ADJUST MODE\n");
          HALT;
        }
        break;

      case 'O':
        GET_INT(output_format);
        if ((IO_status != IO_OK)
            OR(output_format < 0) OR(output_format > 1))
        {
          WRITE(ERROR, "Error: bad OUTPUT FORMAT\n");
          HALT;
        }
        break;

      case 'P':
        GET_REAL(focal_aperture);
        if ((IO_status != IO_OK)
            OR(focal_aperture < ROUNDOFF) OR(focal_aperture > 1.0))
        {
          WRITE(ERROR, "Error: bad FOCAL APERTURE\n");
          HALT;
        }
        break;

      case 'R':
        if (raw_mode == 1)
        {
          WRITE(ERROR, "Error: RAW MODE already set\n");
          HALT;
        }
        OPEN(raw_picture, &(parameter[option][1]), WRITE_BINARY);
        if (IO_status != IO_OK)
        {
          WRITE(ERROR, "Error: unable to open RAW PICTURE FILE (%s)\n",
                &(parameter[option][1]));
          HALT;
        }
        raw_mode = 1;
        break;

      case 'S':
        GET_REAL(threshold_level);
        if ((IO_status != IO_OK) OR(threshold_level <= 0.0)
            OR(threshold_level >= 1.0))
        {
          WRITE(ERROR, "Error: bad SHADING THRESHOLD\n");
          HALT;
        }
        break;

      case 'T':
        GET_REAL(threshold_vector);
        if ((IO_status != IO_OK) OR(threshold_vector < 0.0)
            OR(threshold_vector >= 1.0))
        {
          WRITE(ERROR, "Error: bad AMBIENT THRESHOLD\n");
          HALT;
        }
        distributed_cache_mode = (threshold_vector > ROUNDOFF);
        break;

      case 'V':
        GET_INT(verbose_mode);
        if ((IO_status != IO_OK)
            OR(verbose_mode < 0) OR(verbose_mode > 3))
        {
          WRITE(ERROR, "Error: bad VERBOSE MODE\n");
          HALT;
        }
        break;

      default:
        WRITE(ERROR, "Error: bad OPTION [%c]\n", parameter[option][0]);
        usage();
        HALT;
        break;
    }
  }
  if ((screen_size_x == 0) AND(screen_size_y == 0))
  {
    screen_size_x = 256;
    screen_size_y = 256;
  } else
  if (screen_size_x == 0)
    screen_size_x = screen_size_y;
  else
  if (screen_size_y == 0)
    screen_size_y = screen_size_x;
  if ((view_mode == 0) AND(ABS(stereo_separation) > ROUNDOFF))
  {
    WRITE(ERROR, "Error: cannot have STEREO SEPARATION\n");
    HALT;
  }
  if ((view_mode != 0) AND(ABS(stereo_separation) < ROUNDOFF))
  {
    WRITE(ERROR, "Error: no STEREO SEPARATION\n");
    HALT;
  }
  if (threshold_color > THRESHOLD_COLOR(0.9))
  {
    sampling_levels = 0;
    sampling_divisions = 1 SHL sampling_levels;
    sampling_weight = 4 * SQR(sampling_divisions);
  }
  if (distributed_cache_mode != 0)
  {
    distributed_cache_repetitions = MAX(8, threshold_vector * screen_size_x);
    threshold_vector = 1.0 - threshold_vector;
  } else
    threshold_vector = 1.0;
  if ((parameter[option][0] == '-') AND(parameter[option][1] == EOT))
    scene = INPUT;
  else
  {
    OPEN(scene, parameter[option], READ_TEXT);
    if (IO_status != IO_OK)
    {
      WRITE(ERROR, "Error: unable to open SCENE (%s)\n",
            parameter[option]);
      HALT;
    }
  }
  POSINC(option);
  if ((parameter[option][0] == '-') AND(parameter[option][1] == EOT))
  {
    picture = OUTPUT;
    results = ERROR;
  } else
  {
    OPEN(picture, parameter[option], WRITE_BINARY);
    if (IO_status != IO_OK)
    {
      WRITE(ERROR, "Error: unable to open PICTURE (%s)\n",
            parameter[option]);
      HALT;
    }
    results = OUTPUT;
  }
  /*
   * Memory allocation
   */
 
#ifdef THINK_C

	/* On the mac, when do_enclose is TRUE, the scene is already in memory, so
		we don't want to throw away the existing ALLOCations, since they
		contain important scene information.  We use the old ones. */

	if (do_enclose)
		{
		ALLOCATE(object, object_ptr, OBJECTS_MAX);
		ALLOCATE(surface, surface_ptr, SURFACES_MAX);
		ALLOCATE(pqueue, pqueue_struct, PQUEUE_SIZE_MAX);
		}
	
	else
		{
		
		/* Free memory used by new_line and old_line in previous render */
		FREE(new_line);
		FREE(old_line);
		
		}

#else

  ALLOCATE(object, object_ptr, OBJECTS_MAX);
  ALLOCATE(surface, surface_ptr, SURFACES_MAX);
  ALLOCATE(pqueue, pqueue_struct, PQUEUE_SIZE_MAX);

#endif

  ALLOCATE(new_line, pixel_struct, SCREEN_SIZE_X_MAX);
  ALLOCATE(old_line, pixel_struct, SCREEN_SIZE_X_MAX);

  if (verbose_mode == 0)
    return;

  WRITE(results, "Image width        : %d\n", screen_size_x);
  WRITE(results, "Image height       : %d\n", screen_size_y);
  WRITE(results, "Aliasing threshold : %g\n", sampling_levels ?
        SQRT(threshold_color / 3.0) : 1.0);
  WRITE(results, "Shading threshold  : %g\n", last_shade_level ?
        threshold_level : 1.0);
  WRITE(results, "Ambient threshold  : %g\n", last_ambient_level ?
        1.0 - threshold_vector : 1.0);
  WRITE(results, "Sampling levels    : %d\n", sampling_levels);
  WRITE(results, "Shading levels     : %d\n", SUCC(last_shade_level));
  WRITE(results, "Ambient levels     : %d\n", last_ambient_level);
  WRITE(results, "Ambient samples    : %d\n", last_ambient_level ?
        ambient_sample_rays : 0);
  WRITE(results, "Cluster size       : %d\n", cluster_size);
  WRITE(results, "Antialiasing mode  : ");
  if (sampling_levels == 0)
    WRITE(results, "PIXEL CORNER AVERAGE\n");
  else
    switch (antialiasing_mode)
    {
    case 0:
      WRITE(results, "PIXEL ADAPTIVE SUPERSAMPLING\n");
      break;
    case 1:
      WRITE(results, "PIXEL SEMI-ADAPTIVE SUPERSAMPLING\n");
      break;
    case 2:
      WRITE(results, "PIXEL SUPERSAMPLING\n");
      break;
    }
  WRITE(results, "Background mode    : %s\n", background_mode ? "ON" : "OFF");
  WRITE(results, "Intersect mode     : %s\n",
        (intersect_mode OR(antialiasing_mode == 2)) ?
        "ALL OBJECTS" : "PIXEL CORNER OBJECTS");
  WRITE(results, "Inters. adjust mode: %s\n",
        intersect_adjust_mode ? "ON" : "OFF");
  WRITE(results, "Jittering mode     : %s\n", jittering_mode ? "ON" : "OFF");
  WRITE(results, "Lighting mode      : ");
  switch (light_mode)
  {
  case 0:
    WRITE(results, "NO TRANSLUCENT SHADOWS\n");
    break;
  case 1:
    WRITE(results, "PARTIAL TRANSLUCENT SHADOWS\n");
    break;
  case 2:
    WRITE(results, "FULL TRANSLUCENT SHADOWS\n");
    break;
  }
  WRITE(results, "Normal mode        : %s\n", normal_mode ?
        "NORMAL CORRECTION" : "NO NORMAL CORRECTION");
  WRITE(results, "Normal check mode  : %s\n", normal_check_mode ?
        "NORMAL TEXTURE CORRECTION" : "NO NORMAL TEXTURE CORRECTION");
  WRITE(results, "Shading mode       : %s\n", shade_mode ?
        "STRAUSS MODEL" : "PHONG MODEL");
  WRITE(results, "Raw mode           : %s\n", raw_mode ? "ON" : "OFF");
  WRITE(results, "Texture mode       : ");
  switch (texture_mode)
  {
  case 0:
    WRITE(results, "NO TEXTURES\n");
    break;
  case 1:
    WRITE(results, "TEXTURES INSIDE OBJECTS DESCRIPTION\n");
    break;
  case 2:
    WRITE(results, "TEXTURES AFTER OBJECTS DESCRIPTION\n");
    break;
  }
  WRITE(results, "View mode          : ");
  switch (view_mode)
  {
  case 0:
    WRITE(results, "NORMAL\n");
    break;
  case 1:
    WRITE(results, "LEFT EYE\n");
    break;
  case 2:
    WRITE(results, "RIGHT EYE\n");
    break;
  }
  if (focal_aperture > ROUNDOFF)
    WRITE(results, "Focal aperture     : %g\n", focal_aperture);
  else
    WRITE(results, "Focal aperture     : %s\n", "0 - PINHOLE CAMERA");
  if (focal_distance > ROUNDOFF)
    WRITE(results, "Focal distance     : %g\n", focal_distance);
  else
    WRITE(results, "Focal distance     : %s\n", "GAZE");
  if (stereo_separation > ROUNDOFF)
    WRITE(results, "Stereo separation  : %g\n", stereo_separation);
  if (stereo_separation < -ROUNDOFF)
    WRITE(results, "Stereo separation  : %g%% of GAZE\n",
          -stereo_separation * 100.0);
  WRITE(results, "Output format      : %s\n", output_format ? "PPM" : "PIC");
  WRITE(results, "Info: options ok\n");
  FLUSH(results);
}
