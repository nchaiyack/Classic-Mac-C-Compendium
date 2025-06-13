/*
 * Copyright (c) 1991, 1992 Antonio Costa, INESC-Norte.
 * All rights reserved.
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
 *    SCENE - Defs and Typedefs - Version 1.3.2                       *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, November 1991          *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, August 1992            *
 **********************************************************************/

#define SCREEN_SIZE_X_MAX (2049)/* Screen width maximum + 1  */
#define SCREEN_SIZE_Y_MAX (2049)/* Screen height maximum + 1 */

#define CHAR_MAX (255)

#define X_MAX ((real) 100000.0)
#define X_MIN (-X_MAX)
#define Y_MAX (X_MAX)
#define Y_MIN (X_MIN)
#define Z_MAX (X_MAX)
#define Z_MIN (X_MIN)
#define W_MAX (X_MAX)
#define W_MIN (X_MIN)

#define LIGHTING_FACTOR_MAX   ((real) 300.0)
#define SPECULAR_FACTOR_MAX   ((real) 300.0)
#define REFRACTION_FACTOR_MAX ((real) 300.0)

#define LIGHTS_MAX     (12)

#define OBJECTS_MAX    (500)
#define SURFACES_MAX   (64)

#define TEXTURES_MAX   (64)
#define TRANSFORMS_MAX (64)
#define LEVELS_MAX     (32)

#define SCALE_MIN ((real) 0.0)
#define SCALE_MAX ((real) MAXINT)

#define END(buffer) &(buffer)[strlen(buffer)]

#define DEFINE sprintf
#define APPEND sprintf

#define CREATE(counter_max, counter, list, data)\
do {\
  int		  s_TMP;\
\
  s_TMP = strlen(data);\
  if (counter >= counter_max)\
    yyerror("maximum number of ENTITIES exceeded");\
  ALLOCATE(list[counter], char, s_TMP + 1, OTHER_TYPE);\
  strncpy(list[counter], data, s_TMP + 1);\
  ((char *) list[counter])[s_TMP] = '\0';\
  counter++;\
} while (0)


