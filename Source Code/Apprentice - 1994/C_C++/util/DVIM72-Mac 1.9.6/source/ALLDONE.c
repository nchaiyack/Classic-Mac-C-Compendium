/* -*-C-*- alldone.h */
/*-->alldone*/
/**********************************************************************/
/****************************** alldone  ******************************/
/**********************************************************************/
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "egblvars.h"
#include "m72.h"
#include "sillymalloc.h"

void	TD_close_log( void );

void
alldone()

{
    /*******************************************************************
    This routine  is called  on both  success and  failure to  terminate
    execution.  All open files (except stdin, stdout, stderr) are closed
    before calling EXIT() to quit.
    *******************************************************************/

    UNSIGN16 k;

    for (k = 0; k < (UNSIGN16)nopen; ++k)
	if (font_files[k].font_id != (FILE*)NULL)
	    (void)fclose(font_files[k].font_id);

    if (dvifp != (FILE*)NULL)
        (void)fclose(dvifp);

	sillyfree();
	TD_close_log();

    ExitToShell();
}
