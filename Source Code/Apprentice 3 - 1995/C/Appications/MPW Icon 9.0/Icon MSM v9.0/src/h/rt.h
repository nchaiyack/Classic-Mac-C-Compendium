#ifndef RT_DOT_H	/* only include once */
#define RT_DOT_H 1

/*
 * Include files.
 */

#include "::h:define.h"
#if VMS
/* don't need path.h */
#else					/* VMS */
#include "::h:path.h"
#endif					/* VMS */
#include "::h:config.h"
#include "::h:sys.h"
#include "::h:typedefs.h"
#include "::h:cstructs.h"
#include "::h:cpuconf.h"
#include "::h:monitor.h"
#include "::h:rmacros.h"
#include "::h:rstructs.h"

#ifdef Graphics
#include "::h:graphics.h"
#endif					/* Graphics */

#include "::h:rexterns.h"
#include "::h:proto.h"
#include "::h:rproto.h"
#endif					/* RT_DOT_H */
