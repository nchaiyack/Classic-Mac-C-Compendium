/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */

/* interpreter procedures - interpreter part a */


/* Include standard header modules */
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "int.h"
#include "ivars.h"

#include "farith.h"
#include "predef.h"
#include "machine.h"
#include "tasking.h"
#include "imisc.h"

#include "intb.h"
#include "intc.h"
#include "inta.h"
#include	"inta-sw1.h"
#include	"inta-mac.h"

#include "misc.h"

#ifndef		INTERFACE
static
#else
extern
#endif
int main_loop(void);


#ifdef DEBUG_INT
static void zbreak(int);
#endif

#ifdef MAC_GWADA
#include "MacMemory.h"
#include "ExecMain.h"
#endif


#define TRACE
/* MAIN PROGRAM */

#ifdef DEBUG_STORES
int *heap_store_addr;
/* set heap_store_offset non zero to trace stores to that offset
 * in primary heap 
 */
//extern int heap_store_offset;
int heap_store_now = 0;
#endif

int int_main(void)												/*;int_main*/
{
	int	    status;

	reset_clock();
	num_cunits = 0;

	/* Memory initialization, allocate primary heap segment. */

	if(!allocate_new_heap()) {
		fprintf(stderr,"Unable to allocate primary heap\n");
		exitp(RC_ABORT);
	}

	/* Initialize working template for fixed point arithmetic */

	*heap_next++ = 1 + WORDS_PTR + WORDS_FX_RANGE;
	heap_next += WORDS_PTR;
	temp_template = FX_RANGE(heap_next);
	temp_template->ttype = TT_FX_RANGE;
	temp_template->object_size = 2;
	temp_template->small_exp_2 = 0;
	temp_template->small_exp_5 = 0;
	temp_template->fxlow = MIN_LONG;
	temp_template->fxhigh = MAX_LONG;
	heap_next += WORDS_FX_RANGE;

	/* Other initialization */

	sfp = bfp = 0;
	initialize_predef();
	initialize_tasking();

	/* Perform the main loop of the interpretor(terminates at end of pgm) */

	status = main_loop();

	/* Termination processing */

	predef_term();

	return status;
}

/*
 *  MAIN LOOP
 *  =========
 */

/*
 *  GET_BYTE		Next code byte (char), IP is incremented
 *  GET_WORD		Next code word (int), IP is incremented
 *  GET_GAD(bse,off)	Get base/offset from code, IP incremented
 *  GET_LAD(bse,off)	Get local addr from code, and get corr global addr
 */
#define GET_BYTE	  (0xff & (int)cur_code[ip++])
#ifdef ALIGN_WORD
#define GET_WORD	  (w=get_word(), w)
#else
#define GET_WORD          (w = *((int *)(cur_code+ip)), ip += sizeof(int), w)
#endif
#define GET_GAD(bse,off)  bse=GET_BYTE,off=GET_WORD
#define GET_LAD(bse,off)  sp=GET_WORD+sfp,bse=cur_stack[sp],off=cur_stack[sp+1]


#ifndef		INTERFACE
static
#endif
int main_loop(void)											/*;main_loop*/
{
#ifdef DEBUG_INT
	int     iparg;
#endif
#ifdef ALIGN_WORD
	/* auxiliary procedures if must unpack from code stream byte by byte */
#endif
	static unsigned long time = 0UL;

	/* General purpose work locations */

	/* Loop through instructions */

	for (;;) {

		// Let the user pause the monitor.  If it is paused, then do no
		// processing here.  Just handle UI events and go back to the
		// top of the loop

		if (ApplicationIsPaused()) {
			HandleEvent(ULONG_MAX);	// relinquish CPU [Fabrizio Oddone]
			continue;
			}


#ifdef GWUMON
		/*  Calculate task timing for each task, one tick is one */
		/*  pass through the loop */

		CWK_TIME_TASK();
#endif
		/* Simulate the Clock Interrupt */

		if (next_clock_flag &&(next_clock <(now_time = itime() + time_offset)))
			clock_interrupt(now_time);

		/* Round-robin scheme: next task's turn ? */

		if (rr_flag && (rr_counter++ > rr_nb_max_stmts))
			round_robin();

#ifdef DEBUG_INT
#ifdef DEBUG_STORES
		if (heap_store_offset!=0 && 
		  heap_store_now != heap_store_addr[heap_store_offset]) {
			sprintf(scrBuffer, "heap stores change %d from %d to %d\n",
			  heap_store_offset, heap_store_now, 
			  heap_store_addr[heap_store_offset]);
			scrPutStr(scrBuffer);
			heap_store_now = heap_store_addr[heap_store_offset];
		}
#endif
		iparg = ip;
		if (instruction_trace)
			i_list1(&iparg, cur_code);		/* debug */
		if(break_point && (ip >= break_point))
			zbreak(0);
#endif

		//
		// Process UI events every 1/2 second
		//

// changed a little bit [Fabrizio Oddone]

		if (TickCount() > time + 30) {
			HandleEvent(0L);	// process user interface events so we can
								// cancel execution... this might not return
			time = TickCount();
			}



		/* Get next opcode, bump instruction pointer and switch to routine */

		/* Well, this file is way to big for THINK C to handle in just one
		 * compilation.  We must split this file.
		 */
		/* Get next opcode, bump instruction pointer and switch to routine */
		if (!DoSwitch( GET_BYTE))
			return 0;


	}				/* end loop through instructions */
return 0;
}					/* end main_loop procedure */

#ifdef DEBUG_INT
int get_word(void)			/*;get_word*/
{
	int     w;
	w = *((int *)(cur_code + ip));
	ip += sizeof(int);
	return w;
}

#endif
#ifdef ALIGN_WORD
int get_int(int *n)										/*;get_int*/
{
	register int i;
	int v;
	register char *sp,*tp;

	sp = (char *) n;
	tp = (char *) &v;
	for (i=0; i<sizeof(int); i++) *tp++ = *sp++;
	return v;
}

long get_long(long *n)								/*;get_long*/
{
	register int i;
	long v;
	register char *sp,*tp;

	sp = (char *) n;
	tp = (char *) &v;
	for (i=0; i<sizeof(long); i++) *tp++ = *sp++;
	return v;
}

int get_word(void)									/*;get_word*/
{
	/* if integers must be aligned, get byte by byte */
	int w,i;
	char *sp,*tp;
	sp = (char *) ((int *)(cur_code+ip));
	ip += sizeof(int);
	tp = (char *) &w;
	for (i=0; i<sizeof(int); i++)
		*tp++ = *sp++;
	return w;
}
#endif

int allocate_new_heap(void)								/*;allocate_new_heap*/
{
	/* This procedure attempts to allocate a new heap.
	 * It returns 1 if it succeeds, 0 otherwise.
	 * The size of the heap is defined by max_mem (see config.h).
	 */

	char *temporary;

	/* First tries to reallocate data_segments.  */
	temporary = mrealloc(data_segments,
	  (data_segments_dim + 2) * sizeof(char **));
	if(temporary == (char *)0) return 0;
	data_segments = (int **)temporary;

	/* Now tries to allocate the new heap. */
	temporary = mmalloc((unsigned) max_mem * sizeof(int));
	if(temporary == (char *)0) return 0;

	/* Everything ok: increment data_segments_dim and set heap_base,
	 * heap_addr and heap_next.
	 */
	heap_addr = (int *)temporary;
	heap_base = ++data_segments_dim;
	data_segments[heap_base] = heap_addr;
	heap_next = heap_addr;
#ifdef DEBUG_STORES
	heap_store_addr = heap_addr;
#endif
	return 1;
}

#ifdef DEBUG_INT
static void zbreak(int before)							            /*;zbreak*/
{
	break_point = before;
}
#endif
