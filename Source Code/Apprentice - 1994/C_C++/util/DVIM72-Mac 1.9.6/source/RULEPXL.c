/* -*-C-*- rulepxl.h */
/*-->rulepxl*/
/**********************************************************************/
/****************************** rulepxl *******************************/
/**********************************************************************/
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "m72.h"
#include "egblvars.h"

COORDINATE
rulepxl(number, cnvfac)/* return number of pixels in a rule */
register UNSIGN32 number;/* in DVI units	   */
register float cnvfac;	/* conversion factor */

{
    register COORDINATE n;

    n = (COORDINATE)(number*cnvfac);
    if ((float)n < ((float)(number))*cnvfac)
	return((COORDINATE)(n+1));
    else
	return((COORDINATE)n);
}


