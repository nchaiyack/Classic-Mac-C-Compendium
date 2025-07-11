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
#include	"inta-sw2.h"
#include	"inta-mac.h"

//extern int main_loop(void);
//extern int get_word(void);
#ifdef DEBUG_INT
//extern void zbreak(int);
#endif

#ifdef MAC_GWADA
#include "TermEmulationIntf.h"
#endif

#define TRACE
/* MAIN PROGRAM */

#ifdef DEBUG_STORES
//extern int *heap_store_addr;
/* set heap_store_offset non zero to trace stores to that offset
 * in primary heap 
 */
//extern int heap_store_offset;
//extern int heap_store_now=0;
#endif

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


int DoSwitch(int opCode)
{
	switch( opCode) {

	case I_NOP:
		break;

		/* Instructions Dealing with Tasking */

	case I_ABORT:
		value = GET_WORD; /* number of tasks in stack */
		abort(value);
		break;

	case I_ACTIVATE:
		if (BLOCK_FRAME->bf_tasks_declared != 0) {
			value = pop_task_frame();
			start_activation(value, tp, bfp);
			/* master is current block frame */
		}
		break;

	case I_ACTIVATE_NEW_L:
		GET_LAD(bse, off);
		if (BLOCK_FRAME->bf_tasks_declared != 0) {
			value = pop_task_frame();
			ptr = ADDR(bse, off);
			start_activation(value, ACCESS(ptr)->master_task, 
			ACCESS(ptr)->master_bfp);
		}
		break;

	case I_ACTIVATE_NEW_G:
		GET_GAD(bse, off);
		if (BLOCK_FRAME->bf_tasks_declared != 0) {
			value = pop_task_frame();
			ptr = ADDR(bse, off);
			start_activation(value, ACCESS(ptr)->master_task, 
			ACCESS(ptr)->master_bfp);
		}
		break;

	case I_CREATE_TASK_G:
		GET_GAD(bse, off);
		start_creation(bse, off);
		break;

	case I_CREATE_TASK_L:
		GET_LAD(bse, off);
		start_creation(bse, off);
		break;

	case I_POP_TASKS_DECLARED_G:
		GET_GAD(bse, off);
		if (BLOCK_FRAME->bf_tasks_declared != 0)
			value = pop_task_frame();
		else
			value = 0;
		*ADDR(bse, off) = value;
		break;

	case I_POP_TASKS_DECLARED_L:
		GET_LAD(bse, off);
		if (BLOCK_FRAME->bf_tasks_declared != 0)
			value = pop_task_frame();
		else
			value = 0;
		*ADDR(bse, off) = value;
		break;

	case I_LINK_TASKS_DECLARED:
		POP(value);
		push_task_frame(value);
		break;

	case I_CURRENT_TASK:
		PUSH(tp);
		break;

	case I_END_ACTIVATION:
		value = GET_BYTE;
		end_activation(value); /* 0=error during activation, 1=ok */
		break;

	case I_END_RENDEZVOUS:
		end_rendezvous();
		break;

	case I_ENTRY_CALL:
		value = GET_WORD; /* retrieve parameter from code */
		entry_call((long) ENDLESS,value);
		break;

	case I_RAISE_IN_CALLER:
		raise_in_caller();
		break;

	case I_SELECTIVE_WAIT:
		value = GET_WORD; /* number of alternatives */

		/* if = 0 then it is a simple accept, entry addr is on stack. */
		/* else: alternative descriptors on to of stack are scanned by */
		/*   the procedure, which leaves the index of the chosen one.  */

		selective_wait(value);
		break;

	case I_TERMINATE:
		purge_rdv(tp);
		value = GET_BYTE;
		deallocate(BLOCK_FRAME->bf_data_link);

		/* bf_tasks_declared always null here */

		switch(value) {

		case 0:/* task terminates because reaches the end */
			break;

		case 1:/* task terminates because of terminate alternative */
			break;

		case 2:
			value = 0;
#ifdef GWUMON
			{
				char msg[240];
				sprintf(msg,"task %d terminated due to unhandled exception: %s\n"
					,tp,exception_slots[exr]);
				CWK_Exception_Raised( tp, msg );
			}
#else
			if (exception_trace) {
				sprintf(scrBuffer, "task %d terminated due to unhandled exception: %s\n",
					tp,exception_slots[exr]);
				scrPutStr(scrBuffer);
			}
#endif
			break;

		case 3:
			sprintf(scrBuffer, "unhandled exception in library unit %s\n",
				exception_slots[exr]);
			scrPutStr(scrBuffer);
			return RC_ERRORS;

		case 4:
			sprintf(scrBuffer, "main task terminated due to unhandled exception %s\n",
				exception_slots[exr]);
			scrPutStr(scrBuffer);
			sprintf(scrBuffer, "propagated from %s",code_slots[raise_cs]);
			scrPutStr(scrBuffer);
			if (raise_lin) {
				sprintf(scrBuffer, " at line %d",raise_lin);
				scrPutStr(scrBuffer);
			}
			sprintf(scrBuffer, " (%s)\n",raise_reason);
			scrPutStr(scrBuffer);
			return RC_ERRORS;

		case 5:/* normal end of main */
			return RC_SUCCESS;

		case 6:/* dead-lock */
			sprintf(scrBuffer, "dead-lock: system inactive\n");
			scrPutStr(scrBuffer);
			return RC_ERRORS;
		}
		complete_task();
		break;

	case I_TIMED_ENTRY_CALL:
		POPL(lvalue);
		/* retrieve length of parameter table from code */
		entry_call((lvalue >= 0) ? lvalue : (long) 0, GET_WORD);
		break;

	case I_WAIT:/* delay */
		POPL(lvalue);
		delay_stmt(lvalue);
		break;

		/* Instructions for Memory Allocation */

	case I_CREATE_B:
	case I_CREATE_W:
		exec_create(1, &bse, &off, &ptr);
		PUSH_ADDR(bse, off);
		break;

	case I_CREATE_L:
		exec_create(WORDS_LONG, &bse, &off, &ptr);
		PUSH_ADDR(bse, off);
		break;

	case I_CREATE_A:
		exec_create(2, &bse, &off, &ptr);
		PUSH_ADDR(bse, off);
		break;

	case I_CREATE_STRUC:
		create_structure();
		break;

	case I_CREATE_COPY_STRUC:
		create_copy_struc();
		break;

	case I_CREATE_COPY_B:
	case I_CREATE_COPY_W:
		exec_create(1, &bse, &off, &ptr);
		POP(value);
		PUSH_ADDR(bse, off);
		*ptr = value;
		break;

	case I_CREATE_COPY_L:
		exec_create(WORDS_LONG, &bse, &off, &ptr);
		POPL(lvalue);
		PUSH_ADDR(bse, off);
		*LONG(ptr) = lvalue;
		break;

	case I_CREATE_COPY_A:
		exec_create(2, &bse, &off, &ptr);
		POP_ADDR(bas1, off1);
		PUSH_ADDR(bse, off);
		*ptr++ = bas1;
		*ptr = off1;
		break;

	case I_DECLARE_B:
	case I_DECLARE_W:
		exec_create(1, &bse, &off, &ptr);
		sp = sfp + GET_WORD;
		cur_stack[sp] = bse;
		cur_stack[sp + 1] = off;
		break;

	case I_DECLARE_D:
		exec_create(4, &bse, &off, &ptr);
		sp = sfp + GET_WORD;
		cur_stack[sp] = bse;
		cur_stack[sp + 1] = off;
		break;

	case I_DECLARE_L:
		exec_create(WORDS_LONG, &bse, &off, &ptr);
		sp = sfp + GET_WORD;
		cur_stack[sp] = bse;
		cur_stack[sp + 1] = off;
		break;

	case I_DECLARE_A:
		exec_create(2, &bse, &off, &ptr);
		sp = sfp + GET_WORD;
		cur_stack[sp] = bse;
		cur_stack[sp + 1] = off;
		break;

	case I_ALLOCATE:
		allocate_new();
		break;

	case I_ALLOCATE_COPY_G:
		GET_GAD(bse, off); /* addr. of the type template */
		allocate_copy(bse, off);
		break;

	case I_ALLOCATE_COPY_L:
		GET_LAD(bse, off); /* addr. of the type template */
		allocate_copy(bse, off);
		break;

	case I_UPDATE:
		sp = sfp + GET_WORD;
		cur_stack[sp] = TOSM(1); /* base */
		cur_stack[sp + 1] = TOS; /* offset */
		break;

	case I_UPDATE_AND_DISCARD:
		sp = sfp + GET_WORD;
		POP_ADDR(bse, off);
		cur_stack[sp] = bse;
		cur_stack[sp + 1] = off;
		break;

	case I_UNCREATE:
		POP_ADDR(bse, off);
		ptr = ADDR(bse, off) - WORDS_PTR - 1;
		*ptr = - *ptr;
		break;
		/* should withdraw the variable from bf_data_link TBSL */

		/* Data Transfer Instructions */

	case I_COMPARE_B:
	case I_COMPARE_W:
		POP(val1);
		POP(val2);
		value = (val1 == val2) + 2 *((val1 < val2) ? 1:0);
		/* 0 1 2 for < = > */
		PUSH(value);
		break;

	case I_COMPARE_L:
		POPL(lval1);
		POPL(lval2);
		value = (lval1 == lval2) + 2 *((lval1 < lval2) ? 1:0);
		/* 0 1 2 for < = > */
		PUSH(value);
		break;

	case I_COMPARE_A:
		POP_ADDR(bas1, off1);
		POP_ADDR(bas2, off2);
		value = (off1 == off2 && bas1 == bas2);
		PUSH(value);
		break;

	case I_COMPARE_ARRAYS:
		compare_arrays();
		break;

	case I_COMPARE_STRUC:
		compare_struc();
		break;

	case I_DEREF_B:
	case I_DEREF_W:
		POP_ADDR(bse, off);
		if (bse == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else {
			value = *ADDR(bse, off);
			PUSH(value);
		}
		break;

	case I_DEREF_L:
		POP_ADDR(bse, off);
		if (bse == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else {
			lvalue = *ADDRL(bse, off);
			PUSHL(lvalue);
		}
		break;

	case I_DEREF_A:
		POP_ADDR(bse, off);
		if (bse == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else {
			value = *ADDR(bse, off);
			PUSH(value);
			value = *ADDR(bse, off + 1);
			PUSH(value);
		}
		break;

	case I_DEREF_D:
		POP_ADDR(bse, off);
		if (bse == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else {
			value = *ADDR(bse, off);
			PUSH(value);
			value = *ADDR(bse, off + 1);
			PUSH(value);
			value = *ADDR(bse, off + 2);
			PUSH(value);
			value = *ADDR(bse, off + 3);
			PUSH(value);
		}
		break;

	case I_DISCARD_ADDR:
		value = GET_WORD;
		cur_stackptr -= (2 * value);
		break;

	case I_DUPLICATE_B:
	case I_DUPLICATE_W:
		value = TOS;
		PUSH(value);
		break;

	case I_DUPLICATE_L:
		lvalue = TOSL;
		PUSHL(lvalue);
		break;

	case I_DUPLICATE_A:
		POP_ADDR(bse, off);
		PUSH_ADDR(bse, off);
		PUSH_ADDR(bse, off);
		break;

	case I_DUPLICATE_D:
		value = TOSM(3);
		PUSH(value);
		value = TOSM(3);
		PUSH(value);
		value = TOSM(3);
		PUSH(value);
		value = TOSM(3);
		PUSH(value);
		break;

	case I_INDIRECT_MOVE_B:
	case I_INDIRECT_MOVE_W:
		POP_ADDR(bas1, off1);
		POP_ADDR(bas2, off2);
		if (bas1 == 255 || bas2 == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else
			*ADDR(bas2, off2) = *ADDR(bas1, off1);
		break;

	case I_INDIRECT_MOVE_L:
		POP_ADDR(bas1, off1);
		POP_ADDR(bas2, off2);
		if (bas1 == 255 || bas2 == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else
			*ADDRL(bas2, off2) = *ADDRL(bas1, off1);
		break;

	case I_INDIRECT_MOVE_A:
		POP_ADDR(bas1, off1);
		POP_ADDR(bas2, off2);
		if (bas1 == 255 || bas2 == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else {
			*ADDR(bas2, off2) = *ADDR(bas1, off1);
			*ADDR(bas2, off2 + 1) = *ADDR(bas1, off1 + 1);
		}
		break;

	case I_INDIRECT_POP_B_G:
	case I_INDIRECT_POP_W_G:
		GET_GAD(bse, off);
		POP_ADDR(bas1, off1);
		if (bas1 == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else
			*ADDR(bse, off) = *ADDR(bas1, off1);
		break;

	case I_INDIRECT_POP_L_G:
		GET_GAD(bse, off);
		POP_ADDR(bas1, off1);
		if (bas1 == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else
			*ADDRL(bse, off) = *ADDRL(bas1, off1);
		break;

	case I_INDIRECT_POP_A_G:
		GET_GAD(bse, off);
		POP_ADDR(bas1, off1);
		if (bas1 == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else {
			*ADDR(bse, off) = *ADDR(bas1, off1);
			*ADDR(bse, off + 1) = *ADDR(bas1, off1 + 1);
		}
		break;

	case I_INDIRECT_POP_B_L:
	case I_INDIRECT_POP_W_L:
		GET_LAD(bse, off);
		POP_ADDR(bas1, off1);
		if (bas1 == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else
			*ADDR(bse, off) = *ADDR(bas1, off1);
		break;

	case I_INDIRECT_POP_L_L:
		GET_LAD(bse, off);
		POP_ADDR(bas1, off1);
		if (bas1 == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else
			*ADDRL(bse, off) = *ADDRL(bas1, off1);
		break;

	case I_INDIRECT_POP_A_L:
		GET_LAD(bse, off);
		POP_ADDR(bas1, off1);
		if (bas1 == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else {
			*ADDR(bse, off) = *ADDR(bas1, off1);
			*ADDR(bse, off + 1) = *ADDR(bas1, off1 + 1);
		}
		break;

	case I_MOVE_B:
	case I_MOVE_W:
		POP(value);
		POP_ADDR(bse, off);
		if (bse == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else 
			*ADDR(bse, off) = value;
		break;

	case I_MOVE_L:
		POPL(lvalue);
		POP_ADDR(bse, off);
		if (bse == 255)
			exec_raise(CONSTRAINT_ERROR, "Null access value");
		else 
			*ADDRL(bse, off) = lvalue;
		break;

	case I_MOVE_A:
		POP_ADDR(bas1, off1);
		POP_ADDR(bse, off);
		ptr = ADDR(bse, off);
		*ptr++ = bas1;
		*ptr = off1;
		break;

	case I_POP_B_G:
	case I_POP_W_G:
		GET_GAD(bse, off);
		POP(value);
		*ADDR(bse, off) = value;
		break;

	case I_POP_L_G:
		GET_GAD(bse, off);
		POPL(lvalue);
		*ADDRL(bse, off) = lvalue;
		break;

	case I_POP_D_G:
		/* This has to be set later  TBSL:
					 * for the moment, we do not take care of the poped value. We
					 * beleive this is only being used for the evaluation of object size
					 */
		GET_GAD(bse, off);
		for (i=1; i <= 4 ; i++)
			POP (value);
		break;

	case I_POP_D_L:
		GET_LAD(bse, off);
		for (i=1; i <= 4; i++)
			POP (value);
		break;

	case I_POP_A_G:
		GET_GAD(bse, off);
		POP_ADDR(bas1, off1);
		*ADDR(bse, off) = bas1;
		*ADDR(bse, off + 1) = off1;
		break;

	case I_POP_B_L:
	case I_POP_W_L:
		GET_LAD(bse, off);
		POP(value);
		*ADDR(bse, off) = value;
		break;

	case I_POP_L_L:
		GET_LAD(bse, off);
		POPL(lvalue);
		*ADDRL(bse, off) = lvalue;
		break;

	case I_POP_A_L:
		GET_LAD(bse, off);
		POP_ADDR(bas1, off1);
		*ADDR(bse, off) = bas1;
		*ADDR(bse, off + 1) = off1;
		break;

	case I_PUSH_B_G:
	case I_PUSH_W_G:
		GET_GAD(bse, off);
		value = *ADDR(bse, off);
		PUSH(value);
		break;

	case I_PUSH_L_G:
		GET_GAD(bse, off);
		lvalue = *ADDRL(bse, off);
		PUSHL(lvalue);
		break;

	case I_PUSH_A_G:
		GET_GAD(bse, off);
		ptr = ADDR(bse, off);
		bas1 = *ptr++;
		off1 = *ptr;
		PUSH_ADDR(bas1, off1);
		break;

	case I_PUSH_B_L:
	case I_PUSH_W_L:
		GET_LAD(bse, off);
		value = *ADDR(bse, off);
		PUSH(value);
		break;

	case I_PUSH_L_L:
		GET_LAD(bse, off);
		lvalue = *ADDRL(bse, off);
		PUSHL(lvalue);
		break;

	case I_PUSH_A_L:
		GET_LAD(bse, off);
		ptr = ADDR(bse, off);
		bas1 = *ptr++;
		off1 = *ptr;
		PUSH_ADDR(bas1, off1);
		break;

	case I_PUSH_EFFECTIVE_ADDRESS_G:
	case I_PUSH_IMMEDIATE_A:
		GET_GAD(bse, off);
		PUSH_ADDR(bse, off);
		break;

	case I_PUSH_EFFECTIVE_ADDRESS_L:
		GET_LAD(bse, off);
		PUSH_ADDR(bse, off);
		break;

	case I_PUSH_IMMEDIATE_B:
		PUSH(GET_WORD);
		break;

	case I_PUSH_IMMEDIATE_W:
		PUSH(GET_WORD);
		break;

	case I_PUSH_IMMEDIATE_L:
#ifdef ALIGN_WORD
		lvalue = get_long(LONG(cur_code + ip));
#else
		lvalue = *LONG(cur_code + ip);
#endif
		PUSHL(lvalue);
		ip += sizeof(long);
		break;

		/* Floating Point Instructions */

	case I_FLOAT_ADD_L:
		POPF(rval2);
		POPF(rval1);
		rvalue = rval1 + rval2;
		if (ABS(rvalue) > ADA_MAX_REAL)
			exec_raise(NUMERIC_ERROR, "Floating point addition overflow");
		PUSHF(rvalue);
		break;

	case I_FLOAT_SUB_L:
		POPF(rval2);
		POPF(rval1);
		rvalue = rval1 - rval2;
		if (ABS(rvalue) > ADA_MAX_REAL)
			exec_raise(NUMERIC_ERROR, "Floating point subtraction overflow");
		PUSHF(rvalue);
		break;

	case I_FLOAT_MUL_L:
		POPF(rval2);
		POPF(rval1);
		rvalue = rval1 * rval2;
		if (ABS(rvalue) > ADA_MAX_REAL)
			exec_raise(NUMERIC_ERROR, "Floating point multiplication overflow");
		PUSHF(rvalue);
		break;

	case I_FLOAT_DIV_L:
		POPF(rval2);
		POPF(rval1);
		if (rval2 == 0.0)
			exec_raise(NUMERIC_ERROR, "Floating point division by zero");
		else {
			rvalue = rval1 /rval2;
			if (ABS(rvalue) > ADA_MAX_REAL)
				exec_raise(NUMERIC_ERROR, "Floating point division overflow");
		}
		PUSHF(rvalue);
		break;

	case I_FLOAT_COMPARE_L:
		POPF(rval1);
		POPF(rval2);
		value = (rval1 == rval2) + 2 *(rval1 < rval2);
		/* 0 1 2 for < = > */
		PUSH(value);
		break;

	case I_FLOAT_POW_L:
		POP(val2);
		POPF(rval1);
		if (val2 == 0)
			rvalue = 1.0; /* x ** 0 = 0.0 */
		else if (rval1 == 0.0) {
			if (val2 < 0) /* 0 ** -x = error */
				exec_raise(NUMERIC_ERROR, "Negative power of zero");
			else
				rvalue = 0.0;/* 0 ** +x = 0.0 */
		}
		else {
			rvalue = rval1;
			for (i = 1; i < ABS(val2); i++) {
				rvalue = rvalue * rval1;
				if (ABS(rvalue) > ADA_MAX_REAL) {
					if (val2 > 0) {
						/* the exception has to be raised only if the
													 * exponent is positive. If it is negative, the
													 * result will converge towards 0
													 */
						exec_raise(NUMERIC_ERROR, "Exponentiation");
						break;
					}
					else { 
						rvalue = 0.0; 
						val2 = 1;
						break ; 
					}
				}
			}
			if (val2 < 0)
				rvalue = 1.0 /rvalue;
		}
		PUSHF(rvalue);
		break;

	case I_FLOAT_NEG_L:
		POPF(rval1);
		rvalue = -rval1;
		PUSHF(rvalue);
		break;

	case I_FLOAT_ABS_L:
		POPF(rval1);
		rvalue = ABS(rval1);
		PUSHF(rvalue);
		break;

		/* Logical and Arithmetic Instructions */

	case I_ADD_B:
		POP(val2);
		POP(val1);
		value = val1 + val2;
		if (value < -128 || value > 127)
			exec_raise(NUMERIC_ERROR, "Overflow");
		else
			PUSH(value);
		break;

	case I_ADD_W:
		POP(val2);
		POP(val1);
		value = word_add(val1, val2, &overflow);
		if (overflow)
			exec_raise(NUMERIC_ERROR, "Overflow");
		else
			PUSH(value);
		break;

	case I_ADD_L:
		POPL(lval2);
		POPL(lval1);
		lvalue = long_add(lval1, lval2, &overflow);
		if (overflow)
			exec_raise(NUMERIC_ERROR, "Overflow");
		else
			PUSHL(lvalue);
		break;

	case I_ADD_IMMEDIATE_B:
		POP(val1);
		val2 = GET_WORD;
		value = val1 + val2;
		if (value < -128 || value > 127)
			exec_raise(NUMERIC_ERROR, "Overflow");
		else
			PUSH(value);
		break;

	case I_ADD_IMMEDIATE_W:
		POP(val1);
		val2 = GET_WORD;
		value = word_add(val1, val2, &overflow);
		if (overflow)
			exec_raise(NUMERIC_ERROR, "Overflow");
		PUSH(value);
		break;

	case I_ADD_IMMEDIATE_L:
		POPL(lval1);
#ifdef ALIGN_WORD
		lval2 = get_long(LONG(cur_code + ip));
#else
		lval2 = *(LONG(cur_code + ip));
#endif
		ip += WORDS_LONG;
		lvalue = long_add(lval1, lval2, &overflow);
		if (overflow)
			exec_raise(NUMERIC_ERROR, "Overflow");
		PUSHL(lvalue);
		break;

	case I_DIV_B:
		POP(val2);
		POP(val1);
		if (val2 == 0)
			exec_raise(NUMERIC_ERROR, "Division by zero");
		else if (val1 == -128 && val2 == -1)
			exec_raise(NUMERIC_ERROR, "Overflow");
		else {
			value = val1 /val2;
			PUSH(value);
		}
		break;

	case I_DIV_W:
		POP(val2);
		POP(val1);
		if (val2 == 0)
			exec_raise(NUMERIC_ERROR, "Division by zero");
		else if (val1 == MIN_INTEGER && val2 == -1)
			exec_raise(NUMERIC_ERROR, "Overflow");
		else {
			value = val1 /val2;
			PUSH(value);
		}
		break;

	case I_DIV_L:
		POPL(lval2);
		POPL(lval1);
		if (lval2 == 0)
			exec_raise(NUMERIC_ERROR, "Division by zero");
		else if (lval1 == MIN_LONG && lval2 == -1)
			exec_raise(NUMERIC_ERROR, "Overflow");
		else {
			lvalue = lval1 /lval2;
			PUSHL(lvalue);
		}
		break;

	case I_REM_B:
	case I_REM_W:
		/*
					 * Remainder Operation
					 * -------------------
					 * 
					 * The modification has been done in order to prevent complex
					 * calculation. The remonder operator of Ada is equivallent to "%"
					 * of C. The modification is straightfoward.
					 * 
					 * NB : The previous program was not satisfying. The first operation
					 * was to transform the second parameter into a positive one. The
					 * assignment "val2 = -val2" can be incorrect if the value is the
					 * first integer (-2 ** 31) since 2**31 is not an integer.
					 */

		POP(val2);
		POP(val1);
		if (val2 == 0)
			exec_raise(NUMERIC_ERROR, "Division by zero");
		else {
			value = val1 % val2;
			PUSH(value);
		}
		break;

	case I_REM_L:
		POPL(lval2);
		POPL(lval1);
		if (lval2 == 0)
			exec_raise(NUMERIC_ERROR, "Division by zero");
		else {
			lvalue = lval1 % lval2;
			PUSHL(lvalue);
		}
		break;

	case I_MOD_B:
	case I_MOD_W:

		/* Modulo Operation
					 * ----------------
					 * 
					 * The idea of the modification is to reduce the complexity of the
					 * calculation. The, modulo can be calculated quite easily if the
					 * first parameter is positive. Therefore if the first parameter is
					 * negative then we calculate the first positive number according
					 * to the following equality:
		 			 * a mod b = (a + n*b) mod b
					 */

		POP(val2);
		POP(val1);
		if (val2 == 0)
			exec_raise(NUMERIC_ERROR, "Division by zero");
		else {
			/* the idea is to transform val1 in a positive value.
							 * a mod b = (a + k*b) mod b
							 */
			if ( (val1 <= 0) && ( val2 > 0)) {
				/* val1 = (val1 + (1 - val1/val2)* val2  */
				val1 = val1 - ((val1/val2) * val2) + val2; 
			}
			if ( (val1 <= 0) && ( val2 < 0)) {
				/* val1 = (val1 + (-1 - val1/val2)* val2  */
				val1 = (val1 - val2) - (val1/val2)*val2; 
			}
			if (val2 > 0)
				value = val1 % val2;
			else
				value = (val2 + (val1 % val2)) % val2;
			PUSH(value);
		}
		break;

	case I_MOD_L:
		POPL(lval2);
		POPL(lval1);
		if (lval2 == 0)
			exec_raise(NUMERIC_ERROR, "Division by zero");
		else {
			/* the idea is to transform lval1 in a positive value.
							 * a mod b = (a + k*b) mod b
							 */
			if ( (lval1 <= 0) && ( lval2 > 0)) {
				/* lval1 = (lval1 + (1 - lval1/lval2)* lval2  */
				lval1 = lval1 - ((lval1/lval2) * lval2) + lval2; 
			}
			if ( (lval1 <= 0) && ( lval2 < 0)) {
				/* lval1 = (lval1 + (-1 - lval1/lval2)* lval2  */
				lval1 = (lval1 - lval2) - (lval1/lval2)*lval2; 
			}
			if (lval2 > 0)
				lvalue = lval1 % lval2;
			else
				lvalue = (lval2 + (lval1 % lval2)) % lval2;
			PUSHL(lvalue);
		}
		break;

	case I_MUL_B:
		POP(val2);
		POP(val1);
		value = val1 * val2;
		if (value < -128 || value > 127)
			exec_raise(NUMERIC_ERROR, "Overflow");
		else
			PUSH(value);
		break;

	case I_MUL_W:
		POP(val2);
		POP(val1);
		value = word_mul(val1, val2, &overflow);
		if (overflow)
			exec_raise(NUMERIC_ERROR, "Overflow");
		PUSH(value);
		break;

	case I_MUL_L:
		POPL(lval2);
		POPL(lval1);
		lvalue = long_mul(lval1, lval2, &overflow);
		if (overflow)
			exec_raise(NUMERIC_ERROR, "Overflow");
		PUSHL(lvalue);
		break;

	default:
		return DoSwitchCont(opCode);
	}/* end switch on operation code */

	return 1;
}


