/*
 * Group of include files for translators, etc. 
 */

#include "::h:define.h"

#if CSET2V2
#include <io.h>
#endif					/* CSet/2 ver 2 */

#include <stdio.h>

#if VMS
/* don't need path.h */
#else					/* VMS */
#include "::h:path.h"
#endif					/* VMS */

#include "::h:config.h"
#include "::h:typedefs.h"
#include "::h:cstructs.h"
#include "::h:proto.h"
#include "::h:cpuconf.h"
