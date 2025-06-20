/* -*-C-*- dbgopen.h */
/*-->dbgopen*/
/**********************************************************************/
/****************************** dbgopen *******************************/
/**********************************************************************/
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "egblvars.h"
#include "m72.h"

/* This used to be a long in-line macro, but some compilers could not */
/* handle it. */

void
dbgopen(fp, fname, openmode)
FILE* fp;				/* file pointer */
char* fname;				/* file name */
char* openmode;				/* open mode flags */
{
    if (DBGOPT(DBG_OKAY_OPEN) && (fp != (FILE *)NULL))
    {
	(void)printf("%%Open [%s] mode [%s]--[OK]",fname,openmode);
	printf("\n");
    }
    if (DBGOPT(DBG_FAIL_OPEN) && (fp == (FILE *)NULL))
    {
	(void)printf("%%Open [%s] mode [%s]--[FAILED]",fname,openmode);
	printf("\n");
    }
}
