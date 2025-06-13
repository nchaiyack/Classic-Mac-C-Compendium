/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "config.h"
#include "ivars.h"
#include "slot.h"
#include "ifile.h"
#include "imain.h"
#include "inta.h"
#include "load.h"
#include "misc.h"
#include "libf.h"

#ifdef MAC_GWADA
#include <Profiler.h>
#include "TermEmulationIntf.h"
#endif

#ifdef MONITOR
#include "monitor.h"
#include "mon_ext.h"
#endif

#ifdef DEBUG
int heap_store_offset = 0;
#endif


static void fold_lower(char *);
static void fold_upper(char *);

/* global variable needed for imain.c, derived from generator */
//FILE *efopen();

#ifdef MAC_GWADA
char *FILENAME = NULL;
FILE *MSGFILE = NULL;
FILE *errfile = NULL;
#endif

int adamain(int argc, char **argv)											/*;main*/
{
int         c, i, n, status;
int         errflg = 0, nobuffer = 0;
#ifndef MAC_GWADA
char        *FILENAME;
IFILE		*ifile;
#else
fabIFILE	*ifile;
#endif
extern int  optind;
extern char *optarg;
int			lib_opt = FALSE;
char	*library_name, *fname;
char	*t_name;
Axq		axq;
char	*main_unit = NULL;
//	char 	*tname;

// sets up the profiler for gathering statistics [Fabrizio Oddone]
#if __profile__
if (noErr == ProfilerInit(collectSummary, bestTimeBase
// bestTimeBase is new with CW5:
// many thanks to the guy who came up with this useful idea!

/*
#ifdef __MC68020__
microsecondsTimeBase
#elif defined(__powerc)
PPCTimeBase
#else
timeMgrTimeBase
#endif
*/
, 1100, 700)) {
#endif

rr_flag = FALSE;
max_mem = MAX_MEM;
#ifdef IBM_PC
new_task_size = 2048;
main_task_size = 4096;
#else
main_task_size = 10240;
new_task_size = 10240;
#endif
#ifdef IBM_PC
while((c = getopt(argc,argv,"BbH:h:L:l:M:m:p:P:s:S:t:T:w:R:r:")) != EOF)
#else
while((c = getopt(argc,argv,"bh:l:m:p:s:t:w:r:")) != EOF)
#endif
{
	/*    user:
	 *      h       heap size in kilobytes
	 *	l	library name
	 *	m	main unit name
	 *	p	main program task stack size
	 *	r	nb max consecutive stmts for the same task 
     *              (round-robin)
	 *	s	task stack size
	 *	t	tracing, followed by list of options:
	 *                a	Ada lines
	 *                c	calls 
	 *                e	exceptions
	 *                r	rendezvous
	 *                s	context-switches
	 *                t	tasks
	 *    debug (only):
	 *                d	debug
	 *		  i 	instruction
	 *	b	do not buffer standard output
	 *	w	off	trace stores at specified offset in heap
	 */
#ifdef IBM_PC
            if (isupper(c)) c = tolower(c);
#endif
	switch(c) {
#ifdef DEBUG
	case 'b':	/* do not buffer standard output (for debugging) */
		nobuffer++;
		break;
	case 'w':	/* storage write trace */
		heap_store_offset = atoi(optarg);
		break;
#endif
	case 'h': /* heap size in kilo bytes */
#ifndef IBM_PC
		max_mem = 1024*atoi(optarg);
#else
		{
			int optval; /* avoid too large value */
			optval = atoi(optarg);
			if (optval > 0 && optval < MAX_MEM/1024) 
				max_mem = 1024*optval;
		}
#endif
		break;
	case 'l': /* specify library name */
                    lib_opt = TRUE;
		library_name = strjoin(optarg,"");
		break;
	case 'm': /* specify main unit name */
		main_unit = strjoin(optarg,"");
		fold_upper(main_unit);
		break;
	case 'p': /* main task stack size */
		i = atoi(optarg);
		if (i > 0 && i < 31)	 /* small value gives kilowords */
			main_task_size = i * 1024;
		else if (i > 31)
			main_task_size = i;
		break;
	case 's': /* task stack size */
		i = atoi(optarg);
		if (i > 0 && i < 31)	/* small value gives kilowords */
			new_task_size = i * 1024;
		else if (i > 31)
			new_task_size = i;
		break;
	case 'r': /* nb max consecutive stmts (round-robin) */
		i = atoi(optarg);
		if (i > 0) {
			rr_nb_max_stmts = i;
			rr_flag = TRUE;
		}
		else
            errflg++;
		break;
	case 't': /* interpreter trace arguments */
		n = strlen(optarg);
		for (i = 0; i < n; i++) {
#ifdef IBM_PC
			if (isupper(optarg[i]))
                               optarg[i] = tolower(optarg[i]);
#endif
			switch(optarg[i]) {
			case 'c': /* calls */
				call_trace++;
				break;
			case 'e': /* exceptions */
				exception_trace++;
				break;
			case 'a': /* Ada lines */
				line_trace++;
				break;
			case 'r': /* rendezvous */
				rendezvous_trace++;
				break;
			case 't': /* tasks */
				tasking_trace++;
				break;
			case 's': /* context-switches */
				context_sw_trace++;
				break;
#ifdef DEBUG
			case 'd': /* debug */
				debug_trace++;
				break;
			case 'i': /* instructions */
				instruction_trace++;
				break;
#endif
			default:
				errflg++;
				break;
			}
		}
		break;
	case '?':
		errflg++;
	}
}
#ifdef DEBUG
if (debug_trace) {
	sprintf(scrBuffer, "program, new task stack sizes %d %d\n",
	  main_task_size, new_task_size);
	scrPutStr(scrBuffer);
}
#endif
    fname = NULL;
if (optind < argc) fname = argv[optind];
    if (!lib_opt && fname == NULL) { 
	fname = getenv("ADALIB");
}
if ((!lib_opt && fname == NULL) || errflg) {
   fprintf(stderr,
	  "Usage: adaexec -m main_unit -h size -r nb_stmts -t[acerst] [-l library]\n");
	exitp(RC_ABORT);
}
    if (!lib_opt) {
       library_name = emalloc(strlen(fname) + 1);
       strcpy(library_name, fname);
    }
#ifdef DEBUG
//if (nobuffer)
//	setbuf(stdout,(char *) 0);/* do not buffer output(for debug) */
#endif
FILENAME = library_name;
t_name = libset(library_name);

/* AXQFILE is opened by load_axq or library read (TBSL);*/
axq = (Axq) emalloc((unsigned) sizeof(Axq_s));

#ifdef MAC_GWADA
fab_load_slots(&ifile, axq);
#else
load_slots(FILENAME, &ifile, axq);
#endif

#if GWUMON
{
    int i;
    for (i = 1; i < code_slots_dim; i++) 
	CWK_BLKS[i].TN = code_slots[i];
}
#endif
/* second arg to load_lib and load_axq is non-null if file open */
#ifdef MAC_GWADA
fab_load_lib(ifile, axq, main_unit);
#else
load_lib(FILENAME, ifile, axq, main_unit, argv);
#endif

#ifdef GWUMON
GWU_GET_MON();
#endif
status = int_main();
#ifdef GWUMON
CWK_CLEANUP_MON(0);
#endif

// dumps profiler statistics [Fabrizio Oddone]
#if __profile__
//	DebugStr("\pProfilerDump imminent!");
	if (ProfilerDump("\padafront.prof"))
		SysBeep(30);
	ProfilerTerm();
	}
#endif

return status;
}

// modified to use the Mac OS [Fabrizio Oddone]
// string should be less than 32K long
static void fold_upper(char *s)					/*;fold_upper*/
{
#ifndef MAC_GWADA
register char c;

while (c = *s) {
	if (islower(c)) *s = toupper(c);
	s++;
	}
#else
UpperText((Ptr)s, strlen(s));
#endif
}

