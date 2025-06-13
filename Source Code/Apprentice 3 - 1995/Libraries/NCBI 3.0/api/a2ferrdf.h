/****************************************************************************
*   a2ferrdf.h:
*
*                                     
****************************************************************************/
/* $Revision: 1.3 $ */ 
/************************************
*
* $Log: a2ferrdf.h,v $
 * Revision 1.3  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
************************************/


#ifndef _A2FERRDF_
#define _A2FERRDF_

#include "a2ferr.h"

/*  -- write error message to filename.err                          3-30-93 */

static char *this_module ="a2ferr";

#ifdef THIS_MODULE
#undef THIS_MODULE
#endif
#define THIS_MODULE this_module

#endif
