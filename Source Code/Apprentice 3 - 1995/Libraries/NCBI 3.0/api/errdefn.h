/****************************************************************************
*   errdefn.h:
*
*
* $Log: errdefn.h,v $
 * Revision 1.4  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
*                                     
****************************************************************************/
#ifndef _ERRDEFN_
#define _ERRDEFN_

#include "validatr.h"

/*  -- write error message to filename.err                          3-30-93 */

static char *this_module ="validatr";

#ifdef THIS_MODULE
#undef THIS_MODULE
#endif
#define THIS_MODULE this_module

#endif
