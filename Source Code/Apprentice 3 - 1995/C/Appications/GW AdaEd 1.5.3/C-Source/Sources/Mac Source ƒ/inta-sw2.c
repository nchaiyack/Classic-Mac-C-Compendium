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
#include	"inta-sw2.h"
#include	"inta-mac.h"

//extern int main_loop();
//extern int get_word();
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

int DoSwitchCont(int opCode)
{
	switch (opCode) {
	case I_POW_B:
		POP(val2);
		POP(val1);
		if (val2 < 0)
			exec_raise(NUMERIC_ERROR, "Overflow");
		else if (val2 == 0)
			value = 1;
		else {
			value = val1;
			for (i = 1; i < val2; i++) {
				value = value * val1;
				if (value > 127)
					exec_raise(NUMERIC_ERROR, "Overflow");
			}
		}
		PUSH(value);
		break;

	case I_POW_W:
		POP(val2);
		POP(val1);
		if (val2 < 0)
			exec_raise(NUMERIC_ERROR, "Overflow");
		else if (val2 == 0)
			value = 1;
		else
			value = val1;
		for (i = 1; i < val2; i++) {
			value = word_mul(value, val1, &overflow);
			if (overflow)
				exec_raise(NUMERIC_ERROR, "Overflow");
		}
		PUSH(value);
		break;

	case I_POW_L:
		POPL(lval2);
		POPL(lval1);
		if (lval2 < 0)
			exec_raise(NUMERIC_ERROR, "Overflow");
		else if (lval2 == 0)
			lvalue = 1;
		else {
			lvalue = lval1;
			for (i = 1; i < lval2; i++) {
				lvalue = long_mul(lvalue, lval1, &overflow);
				if (overflow)
					exec_raise(NUMERIC_ERROR, "Overflow");
			}
		}
		PUSHL(lvalue);
		break;

	case I_FIX_MUL:
		POP_ADDR(bas1, off1);/* type and value of op2 */
		ptr2 = ADDR(bas1, off1);
		POPL(fval2);

		POP_ADDR(bas1, off1);/* type and value of op1 */
		ptr1 = ADDR(bas1, off1);
		POPL(fval1);

		POP_ADDR(bas1, off1);/* result type */
		ptr = ADDR(bas1, off1);

		if (fval2 == 0 || fval1 == 0) {
			fvalue = 0;
			PUSHL(fvalue);
		}
		else {
			to_type = TYPE(ptr);
			if (to_type == TT_FX_RANGE) {

				sgn = SIGN(fval1);
				fval1 = ABS(fval1);
				sgn *= SIGN(fval2);
				fval2 = ABS(fval2);
				int_tom(fix_val1,fval1);
				int_tom(fix_val2,fval2);

				temp_template->small_exp_2 = FX_RANGE(ptr1)->small_exp_2 +
					FX_RANGE(ptr2)->small_exp_2;
				temp_template->small_exp_5 = FX_RANGE(ptr1)->small_exp_5 +
					FX_RANGE(ptr2)->small_exp_5;

				int_mul(fix_val1, fix_val2, fix_resu);
				fix_convert(fix_resu, temp_template, FX_RANGE(ptr));
				fvalue = int_tol(fix_resu);
				if (arith_overflow)
					exec_raise(NUMERIC_ERROR,
					"Fixed point multiplication overflow");
				if (fix_out_of_bounds(fvalue, ptr))
					exec_raise(CONSTRAINT_ERROR,
					"Fixed point value out of bounds");
				PUSHL(sgn*fvalue);
			}
			else
				exec_raise(SYSTEM_ERROR, "Conversion to invalid type");
		}
		break;

	case I_FIX_DIV:
		POP_ADDR(bas1, off1);/* type and value of op2 */
		ptr2 = ADDR(bas1, off1);
		POPL(fval2);

		POP_ADDR(bas1, off1);/* type and value of op1 */
		ptr1 = ADDR(bas1, off1);
		POPL(fval1);

		POP_ADDR(bas1, off1);/* result type */
		ptr = ADDR(bas1, off1);

		if (fval2 == 0) {
			exec_raise(NUMERIC_ERROR, "Fixed point division by zero");
			fvalue = 0;
			PUSHL(fvalue);
		}
		else {
			to_type = TYPE(ptr);
			if (to_type == TT_FX_RANGE) {

				sgn = SIGN(fval1);
				fval1 = ABS(fval1);
				sgn *= SIGN(fval2);
				fval2 = ABS(fval2);
				int_tom(fix_val1,fval1);
				int_tom(fix_val2,fval2);

				temp_template->small_exp_2 = FX_RANGE(ptr)->small_exp_2 +
					FX_RANGE(ptr2)->small_exp_2;
				temp_template->small_exp_5 = FX_RANGE(ptr)->small_exp_5 +
					FX_RANGE(ptr2)->small_exp_5;

				fix_convert(fix_val1, FX_RANGE(ptr1), temp_template);
				int_div(fix_val1, fix_val2, fix_resu);
				fvalue = int_tol(fix_resu);
				if (arith_overflow)
					exec_raise(NUMERIC_ERROR, "Fixed point division overflow");
				if (fix_out_of_bounds(fvalue, ptr))
					exec_raise(CONSTRAINT_ERROR,
					"Fixed point value out of bounds");
				PUSHL(sgn*fvalue);
			}
			else
				exec_raise(SYSTEM_ERROR, "Conversion to invalid type");
		}
		break;

	case I_CONVERT_TO_L:
		GET_LAD(bse, off);
		convert(bse, off);
		break;

	case I_CONVERT_TO_G:
		GET_GAD(bse, off);
		convert(bse, off);
		break;

	case I_NEG_B:
		if (TOS == -128)
			exec_raise(NUMERIC_ERROR,"Byte overflow");
		else
			TOS = -TOS;
		break;

	case I_NEG_W:
		if (TOS == MIN_INTEGER)
			exec_raise(NUMERIC_ERROR,"Overflow");
		else
			TOS = -TOS;
		break;

	case I_NEG_L:
		if (TOS == MIN_LONG)
			exec_raise(NUMERIC_ERROR,"Overflow");
		else
			TOSL = -TOSL;
		break;

	case I_ABS_B:
		if (TOS == -128)
			exec_raise(NUMERIC_ERROR,"Byte overflow");
		else
			TOS = ABS(TOS);
		break;

	case I_ABS_W:
		if (TOS == MIN_INTEGER)
			exec_raise(NUMERIC_ERROR,"Overflow");
		else
			TOS = ABS(TOS);
		break;

	case I_ABS_L:
		if (TOS == MIN_LONG)
			exec_raise(NUMERIC_ERROR,"Overflow");
		else
			TOSL = ABS(TOSL);
		break;

	case I_NOT:
		TOS = 1 - TOS;
		break;

	case I_AND:
		POP(val2);
		POP(val1);
		value = (val1 & val2);
		PUSH(value);
		break;

	case I_XOR:
		POP(val2);
		POP(val1);
		value = (val1 ^ val2);
		PUSH(value);
		break;

	case I_OR:
		POP(val2);
		POP(val1);
		value = (val1 | val2);
		PUSH(value);
		break;

	case I_IS_EQUAL:
		TOS = (TOS == 1);
		break;

	case I_IS_GREATER:
		TOS = (TOS == 2);
		break;

	case I_IS_GREATER_OR_EQUAL:
		TOS = (TOS >= 1);
		break;

	case I_IS_LESS:
		TOS = (TOS == 0);
		break;

	case I_IS_LESS_OR_EQUAL:
		TOS = (TOS <= 1);
		break;

	case I_MEMBERSHIP:
		membership();
		break;

	case I_QUAL_RANGE_G:
		GET_GAD(bse, off);
		ptr1 = ADDR(bse, off);
		if (TYPE(ptr1) == TT_FX_RANGE) {
			if (fix_out_of_bounds(TOSL, ptr1))
				exec_raise(CONSTRAINT_ERROR, "Fixed point value out of bounds");
		}
		else if (TYPE(ptr1) == TT_FL_RANGE) {
			rval1 = FL_RANGE(ptr1)->fllow;
			rval2 = FL_RANGE(ptr1)->flhigh;
			if (TOSF < rval1 || TOSF > rval2)
				exec_raise(CONSTRAINT_ERROR,
				"Floating point value out of bounds");
		}
		else if ((TYPE(ptr1) == TT_I_RANGE) ||
			(TYPE(ptr1) == TT_E_RANGE) ||
			(TYPE(ptr1) == TT_ENUM)) {
			val_low = I_RANGE(ptr1)->ilow;
			val_high = I_RANGE(ptr1)->ihigh;
			if (TOS < val_low || TOS > val_high)
				exec_raise(CONSTRAINT_ERROR, "Out of bounds");
		}
#ifdef LONG_INT
		else if (TYPE(ptr1) == TT_L_RANGE) {
			lvalue = TOSL;
			lval_low = L_RANGE(ptr1)->llow;
			lval_high = L_RANGE(ptr1)->lhigh;
			if (lvalue < lval_low || lvalue > lval_high)
				exec_raise (CONSTRAINT_ERROR, "Out of bounds");
		}
#endif
		else /* error here */
			;
		break;

	case I_QUAL_RANGE_L:
		GET_LAD(bse, off);
		ptr1 = ADDR(bse, off);
		if (TYPE(ptr1) == TT_FX_RANGE) {
			fval1 = TOSL;
			if (fix_out_of_bounds(fval1, ptr1))
				exec_raise(CONSTRAINT_ERROR, "Fixed point value out of bounds");
		}
		else if (TYPE(ptr1) == TT_FL_RANGE) {
			rvalue = TOSF;
			rval1 = FL_RANGE(ptr1)->fllow;
			rval2 = FL_RANGE(ptr1)->flhigh;
			if (rvalue < rval1 || rvalue > rval2)
				exec_raise(CONSTRAINT_ERROR,
				"Floating point value out of bounds");
		}
		else if ((TYPE(ptr1) == TT_I_RANGE) ||
			(TYPE(ptr1) == TT_E_RANGE) ||
			(TYPE(ptr1) == TT_ENUM)) {
			val_low = I_RANGE(ptr1)->ilow;
			val_high = I_RANGE(ptr1)->ihigh;
			if (TOS < val_low || TOS > val_high)
				exec_raise(CONSTRAINT_ERROR, "Out of bounds");
		}
#ifdef LONG_INT
		else if (TYPE(ptr1) == TT_L_RANGE) {
			lvalue = TOSL;
			lval_low = L_RANGE(ptr1)->llow;
			lval_high = L_RANGE(ptr1)->lhigh;
			if (lvalue < lval_low || lvalue > lval_high)
				exec_raise (CONSTRAINT_ERROR, "Out of bounds");
		}
#endif
		else /* error here */
			;
		break;

	case I_QUAL_DISCR_G:
		GET_GAD(bse, off);
		qual_discr(bse, off);
		break;

	case I_QUAL_DISCR_L:
		GET_LAD(bse, off);
		qual_discr(bse, off);
		break;

	case I_QUAL_INDEX_G:
		GET_GAD(bse, off);
		ptr = ADDR(bse, off);
		POP_ADDR(bse, off);
		PUSH_ADDR(bse, off);
		ptr1 = ADDR(bse, off);
		if (!qual_index(ptr, ptr1))
			exec_raise(CONSTRAINT_ERROR, "Wrong bounds");
		break;

	case I_QUAL_INDEX_L:
		GET_LAD(bse, off);
		ptr = ADDR(bse, off);
		POP_ADDR(bse, off);
		PUSH_ADDR(bse, off);
		ptr1 = ADDR(bse, off);
		if (!qual_index(ptr, ptr1))
			exec_raise(CONSTRAINT_ERROR, "Wrong bounds");
		break;

	case I_QUAL_SUB_G:
		GET_GAD(bse, off);
		ptr = ADDR(bse, off);
		POP_ADDR(bse, off);
		PUSH_ADDR(bse, off);
		ptr1 = ADDR(bse, off);
		if (!qual_sub(ptr, ptr1))
			exec_raise(CONSTRAINT_ERROR, "Wrong bounds");
		break;

	case I_QUAL_SUB_L:
		GET_LAD(bse, off);
		ptr = ADDR(bse, off);
		POP_ADDR(bse, off);
		PUSH_ADDR(bse, off);
		ptr1 = ADDR(bse, off);
		if (!qual_sub(ptr, ptr1))
			exec_raise(CONSTRAINT_ERROR, "Wrong bounds");
		break;

	case I_SUB_B:
		POP(val2);
		POP(val1);
		value = val1 - val2;
		if (value < -128 || value > 127)
			exec_raise(NUMERIC_ERROR, "Overflow");
		else
			PUSH(value);
		break;

	case I_SUB_W:
		POP(val2);
		POP(val1);
		value = word_sub(val1, val2, &overflow);
		if (overflow)
			exec_raise(NUMERIC_ERROR, "Overflow");
		else
			PUSH(value);
		break;

	case I_SUB_L:
		POPL(lval2);
		POPL(lval1);
		lvalue = long_sub(lval1, lval2, &overflow);
		if (overflow)
			exec_raise(NUMERIC_ERROR, "Overflow");
		else
			PUSHL(lvalue);
		break;

		/* Array Instructions */

	case I_ARRAY_CATENATE:
		array_catenate();
		break;

	case I_ARRAY_MOVE:
		array_move();
		break;

	case I_ARRAY_SLICE:
		array_slice();
		break;

	case I_ARRAY_AND:
		POP_ADDR(bas1, off1);/* right type */
		POP_ADDR(bas2, off2);/* right object */
		POP_ADDR(bse, off);/* left type */
		value = SIZE(ADDR(bse, off));
		if (SIZE(ADDR(bas1, off1)) != value)
			exec_raise(CONSTRAINT_ERROR, "Arrays not same size for AND");
		else {
			POP_ADDR(bas1, off1);/* left object */
			ptr1 = ADDR(bas1, off1);
			ptr2 = ADDR(bas2, off2);
			exec_create(value, &bas1, &off1, &ptr);
			for (i = 0; i <= value - 1; i++)
				*ptr++ = (*ptr1++ & *ptr2++);
			PUSH_ADDR(bas1, off1);/* result object */
			PUSH_ADDR(bse, off);/* result type */
		}
		break;

	case I_ARRAY_OR:
		POP_ADDR(bas1, off1);/* right type */
		POP_ADDR(bas2, off2);/* right object */
		POP_ADDR(bse, off);/* left type */
		value = SIZE(ADDR(bse, off));
		if (SIZE(ADDR(bas1, off1)) != value)
			exec_raise(CONSTRAINT_ERROR, "Arrays not same size for OR");
		else {
			POP_ADDR(bas1, off1);/* left object */
			ptr1 = ADDR(bas1, off1);
			ptr2 = ADDR(bas2, off2);
			exec_create(value, &bas1, &off1, &ptr);
			for (i = 0; i <= value - 1; i++)
				*ptr++ = (*ptr1++ | *ptr2++);
			PUSH_ADDR(bas1, off1);/* result object */
			PUSH_ADDR(bse, off);/* result type */
		}
		break;

	case I_ARRAY_XOR:
		POP_ADDR(bas1, off1);/* right type */
		POP_ADDR(bas2, off2);/* right object */
		POP_ADDR(bse, off);/* left type */
		value = SIZE(ADDR(bse, off));
		if (SIZE(ADDR(bas1, off1)) != value)
			exec_raise(CONSTRAINT_ERROR, "Arrays not same size for XOR");
		else {
			POP_ADDR(bas1, off1);/* left object */
			ptr1 = ADDR(bas1, off1);
			ptr2 = ADDR(bas2, off2);
			exec_create(value, &bas1, &off1, &ptr);
			for (i = 0; i <= value - 1; i++) {
				*ptr++ = (*ptr1++ ^ *ptr2++);
			}
			PUSH_ADDR(bas1, off1);/* result object */
			PUSH_ADDR(bse, off);/* result type */
		}
		break;

	case I_ARRAY_NOT:
		POP_ADDR(bse, off);/* type */
		value = SIZE(ADDR(bse, off));
		POP_ADDR(bas1, off1);/* object */
		ptr1 = ADDR(bas1, off1);
		exec_create(value, &bas1, &off1, &ptr);
		for (i = 0; i <= value - 1; i++)
			*ptr++ = (1 - *ptr1++);
		PUSH_ADDR(bas1, off1);/* result object */
		PUSH_ADDR(bse, off);/* result type */
		break;

		/* Record Instructions */

	case I_RECORD_MOVE_G:
		GET_GAD(bse, off);
		ptr = ADDR(bse, off);
		POP_ADDR(bas1, off1);/* value */
		ptr1 = ADDR(bas1, off1);
		POP_ADDR(bas2, off2);/* object */
		ptr2 = ADDR(bas2, off2);
		record_move(ptr2, ptr1, ptr);
		break;

	case I_RECORD_MOVE_L:
		GET_LAD(bse, off);
		ptr = ADDR(bse, off);
		POP_ADDR(bas1, off1);/* value */
		ptr1 = ADDR(bas1, off1);
		POP_ADDR(bas2, off2);/* object */
		ptr2 = ADDR(bas2, off2);
		record_move(ptr2, ptr1, ptr);
		break;

		/* Attributes */

	case I_ATTRIBUTE:
		attribute = GET_BYTE;
		/* So that all reads from code segment are done in this
					 * procedure, we retrieve the dim argument used for
					 * some attributes
					 */
		if (attribute==ATTR_O_FIRST || attribute==ATTR_O_LAST
			|| attribute == ATTR_O_LENGTH || attribute==ATTR_O_RANGE)
			value = GET_WORD;
		else
			value = 0;
		main_attr(attribute,value);
		break;

		/* Control Instructions */

	case I_ENTER_BLOCK:
#ifdef DEBUG_TASKING
		if (tasking_trace) {
			sprintf(scrBuffer, "enter block pushing %d for previous\n",bfp);
			scrPutStr(scrBuffer);
		}
#endif
		PUSH(bfp); /* save previous BFP */
		bfp = cur_stackptr;
#ifdef DEBUG_TASKING
		if (tasking_trace) {
			sprintf(scrBuffer, "enter block bfp %d\n",bfp);
			scrPutStr(scrBuffer);
		}
#endif
		PUSHP(0L); /* data_link */
		PUSHP(0L); /* tasks_declared */
		PUSH(1); /* num noterm */
		PUSH(1); /* num deps */
		PUSH(NULL_TASK);/* subtasks */
		PUSH(0); /* exception vector */
		break;

	case I_EXIT_BLOCK:
#ifdef DEBUG_TASKING
		if (tasking_trace) {
#ifdef IBM_PC
			printf("exit block bfp %d %p\n",bfp,cur_stack+bfp);
#else
			sprintf(scrBuffer, "exit block bfp %d %ld\n",bfp,cur_stack+bfp);
			scrPutStr(scrBuffer);
#endif
		}
#endif
		if (BLOCK_FRAME->bf_num_deps >= 1) {
			--ip; /* to reexecute the 'leave_block' */
			complete_block();
		}
		else {
			deallocate(BLOCK_FRAME->bf_data_link);
			sp = BLOCK_FRAME->bf_previous_bfp;
			if ((tfptr1 = BLOCK_FRAME->bf_tasks_declared) != 0) {
				bfptr = (struct bf *)(&cur_stack[sp]);
				tfptr2 = bfptr->bf_tasks_declared;
				if (tfptr2 != 0) {
					value = pop_task_frame();
					*tfptr2 = union_tasks_declared(value, *tfptr2);
				}
				else /* put task frame on previous bfp */
					bfptr->bf_tasks_declared = tfptr1;
			}
			cur_stackptr = bfp - 1;
			bfp = sp;
#ifdef DEBUG_TASKING
			if (tasking_trace) {
				sprintf(scrBuffer, "exit block setting bfp %d\n",bfp);
				scrPutStr(scrBuffer);
			}
#endif
		}
		break;

	case I_LEAVE_BLOCK:
#ifdef DEBUG_TASKING
		if (tasking_trace) {
#ifdef IBM_PC
			printf("leave block bfp %d %p\n",bfp,cur_stack+bfp);
#else
			sprintf(scrBuffer, "leave block bfp %d %ld\n",bfp,cur_stack+bfp);
			scrPutStr(scrBuffer);
#endif
		}
#endif
		if (BLOCK_FRAME->bf_num_deps >= 1) {
			--ip; /* to reexecute the 'leave_block' */
			complete_block();
		}
		else {
			deallocate(BLOCK_FRAME->bf_data_link);
			sp = BLOCK_FRAME->bf_previous_bfp;
			if ((tfptr1 = BLOCK_FRAME->bf_tasks_declared) != 0) {
				bfptr = (struct bf *)(&cur_stack[sp]);
				tfptr2 = bfptr->bf_tasks_declared;
				if (tfptr2 != 0) {
					value = pop_task_frame();
					*tfptr2 = union_tasks_declared(value, *tfptr2);
				}
				else /* put task frame on previous bfp */
					bfptr->bf_tasks_declared = tfptr1;
			}
			if (sp < sfp) {/* return to previous stack_frame */
				cur_stackptr = sfp - 1;/* get rid of the relay set */
				/* in case an exception is propagated, ip */
				/* must point again to the default handler */
#ifdef ALIGN_WORD
				val2 = get_int((int *)(cur_code + code_seglen[cs] 
					- sizeof(int) - 1));
#else
				val2 = *(int *)(cur_code+code_seglen[cs] - sizeof(int) - 1);
#endif
				/* length of local variables */
				if (ip < 2) {
					--cur_stackptr;/* to discard it */
#ifdef TRACE
					if (call_trace) {
						sprintf(scrBuffer, "abandoning %s\n", code_slots[cs]);
						scrPutStr(scrBuffer);
					}
#endif
				}
				else {
					POP(ip);
#ifdef TRACE
					if (call_trace) {
						if (code_slots[cs]) {
							sprintf(scrBuffer, "returning from %s (tos %d)\n",
								code_slots[cs],cur_stackptr- 3-val2);
							scrPutStr(scrBuffer);
						}
						else {
							sprintf(scrBuffer, "returning from %s (tos %d)\n", 
								"compiler_generated_procedure",	cur_stackptr-3-val2);
							scrPutStr(scrBuffer);
						}
					}
#endif
#ifdef GWUMON
					if (code_slots[cs])
					{
						CWK_LEAVE_BLOCK(cs);
					}
#endif
				}
				POP(lin);
				POP(cs);
#ifdef GWUMON
				if (code_slots[cs])
				{
					CWK_Switch_Block(cs, code_slots[cs],
					code_slots_file[cs],1 );
				}
#endif
				cur_code = code_segments[cs];
				POP(sfp);
				cur_stackptr -= val2;/* to get rid of it */
			}
			else
				cur_stackptr = bfp - 1;
			bfp = sp;
#ifdef DEBUG_TASKING
			if (tasking_trace) {
				sprintf(scrBuffer, "leave block setting bfp %d\n",bfp);
				scrPutStr(scrBuffer);
			}
#endif
		}
		break;

	case I_CALL_L:
		GET_LAD(bse, off);/* addr of proc. object */
		ptr = ADDR(bse, off);
		value = *ptr;
		if (value < 0)
			exec_raise(PROGRAM_ERROR, "Access before elaboration");
		else {
			if (cur_stackptr+SECURITY_LEVEL>new_task_size)
				exec_raise(STORAGE_ERROR, "Stack overflow");
			else {
				old_cs = cs;
				cs = value;
#ifdef GWUMON
				if ( code_slots[cs] )
					CWK_Switch_Block(cs,
					code_slots[cs],
					code_slots_file[cs],0 );
#endif
#ifdef TRACE
				if (call_trace) {
					if (code_slots[cs]) {
						sprintf(scrBuffer, "calling %s (tos %d -> ",
							code_slots[cs], cur_stackptr);
						scrPutStr(scrBuffer);
					}
					else {
						sprintf(scrBuffer, "calling %s (tos %d -> ",
							"compiler_generated_procedure", cur_stackptr);
						scrPutStr(scrBuffer);
					}
				}
#endif
				cur_code = code_segments[cs];
#ifdef ALIGN_WORD
				val1 = get_int((int *)(cur_code + code_seglen[cs] 
					- sizeof(int) - 1));
#else
				val1 = *(int *)(cur_code+code_seglen[cs] - sizeof(int) - 1);
#endif
				/* reserve space for locals */
				if (val1 < 0)
					exec_raise(SYSTEM_ERROR, "Negative size of locals");
				else
					cur_stackptr += val1;
				PUSH(sfp);
				PUSH(old_cs);
				PUSH(lin);
				PUSH(ip);
				sfp = cur_stackptr + 1;
				ip = 2;
				val2 = *(++ptr) * 2;/* length of relay set */
				for (i = 1; i <= val2; i++) /* copy relay set */
					PUSH(*++ptr);
#ifdef TRACE
				if (call_trace) {
					sprintf(scrBuffer, "%d)\n",cur_stackptr);
					scrPutStr(scrBuffer);
				}
#endif
			}
		}
		break;

	case I_CALL_G:
		GET_GAD(bse, off);/* addr of proc. object */
		ptr = ADDR(bse, off);
		value = *ptr;
		if (value < 0)
			exec_raise(PROGRAM_ERROR, "Access before elaboration");
		else {
			if (cur_stackptr+SECURITY_LEVEL>new_task_size)
				exec_raise(STORAGE_ERROR, "Stack overflow");
			else {
				old_cs = cs;
				cs = value;
#ifdef GWUMON
				if ( code_slots[cs] )
					CWK_Switch_Block(cs,
					code_slots[cs],
					code_slots_file[cs],0 );
#endif
#ifdef TRACE
				if (call_trace) {
					if (code_slots[cs]) {
						sprintf(scrBuffer, "calling %s (tos %d -> ",
							code_slots[cs],cur_stackptr);
						scrPutStr(scrBuffer);
					}
					else {
						sprintf(scrBuffer, "calling %s (tos %d -> ",
							"compiler_generated_procedure", cur_stackptr);
						scrPutStr(scrBuffer);
					}
				}
#endif
				cur_code = code_segments[cs];
				/* reserve space for local variables */
#ifdef ALIGN_WORD
				val1 = get_int((int *)(cur_code + code_seglen[cs] 
					- sizeof(int) - 1));
#else
				val1 = *(int *)(cur_code+code_seglen[cs] - sizeof(int) - 1);
#endif
				/* reserve space for locals */
				if (val1 < 0)
					exec_raise(SYSTEM_ERROR, "Negative size of locals");
				else
					cur_stackptr += val1;
				PUSH(sfp);
				PUSH(old_cs);
				PUSH(lin);
				PUSH(ip);
				sfp = cur_stackptr + 1;
				ip = 2;
				/* copy relay set */
				val2 = *(++ptr) * 2;/* length of relay set */
				for (i = 1; i <= val2; i++) /* copy relay set */
					PUSH(*++ptr);
#ifdef TRACE
				if (call_trace) {
					sprintf(scrBuffer, "%d)\n",cur_stackptr);
					scrPutStr(scrBuffer);
				}
#endif
			}
		}
		break;

	case I_CALL_PREDEF:
		operation = GET_BYTE;
		predef();
		break;

#ifdef INTERFACE
	case I_CALL_INTERFACE:
		interface(GET_WORD);
		break;
#endif

	case I_CASE_B:
	case I_CASE_W:
	case I_CASE_L:
		POP(value);
		nb = GET_WORD;
		jump = GET_WORD;
		for (i = 1; i <= nb; i++) {
			val_high = GET_WORD;
			if (value < val_high)
				break;
			jump = GET_WORD;
		}
		ip = jump;
		break;

	case I_RETURN_B:
	case I_RETURN_W:
		POP(value);
		cur_stack[sfp + GET_WORD] = value;
		break;

	case I_RETURN_L:
		POPL(lvalue);
		*(LONG(&cur_stack[sfp + GET_WORD])) = lvalue;
		break;

	case I_RETURN_A:
		POP_ADDR(bse, off);
		sp = GET_WORD + sfp;
		cur_stack[sp] = bse;
		cur_stack[sp + 1] = off;
		break;

	case I_RETURN_STRUC:
		sp = GET_WORD + sfp;
		POP_ADDR(bse, off);/* 	type */
		ptr = ADDR(bse, off);
		POP_ADDR(bas2, off2);/* value */
		ptr2 = ADDR(bas2, off2);

		val1 = TYPE(ptr);/* type of type */
		val2 = SIZE(ptr);
		allocate(val2, &bas1, &off1, &ptr1);
		cur_stack[sp] = bas1;
		cur_stack[sp + 1] = off1;

		for (i = 0; i < val2; i++)
			*ptr1++ = *ptr2++;

		switch(val1) {
			case TT_U_ARRAY:
			case TT_C_ARRAY:
			case TT_S_ARRAY:
			case TT_D_ARRAY:
				if (bse >= heap_base) {/* non static template */
					/* create new type template */
					/* size of template */
					val2 = *(ptr - WORDS_HDR) - WORDS_HDR;
					allocate(val2, &bse, &off, &ptr1);
	
					for (i = 0; i < val2; i++)
						*ptr1++ = *ptr++;
				}
				cur_stack[sp + 2] = bse;
				cur_stack[sp + 3] = off;
				break;
	
			case TT_RECORD:
			case TT_U_RECORD:
			case TT_C_RECORD:
			case TT_D_RECORD:
			case TT_V_RECORD:
				break;
			}
		break;

	case I_END_FOR_LOOP_B:
	case I_END_FOR_LOOP_W:
	case I_END_FOR_LOOP_L:
		val2 = GET_WORD;
		off = TOS;
		bse = TOSM(1);
		lim = TOSM(2);
		value = *ADDR(bse, off);
		if (value >= lim) {
			POP_ADDR(bse, off);
			POP(val1);
		}
		else {
			*ADDR(bse, off) = value + 1;
			ip = val2;
		}
		break;

	case I_END_FORREV_LOOP_B:
	case I_END_FORREV_LOOP_W:
	case I_END_FORREV_LOOP_L:
		val2 = GET_WORD;
		off = TOS;
		bse = TOSM(1);
		lim = TOSM(2);
		value = *ADDR(bse, off);
		if (value <= lim) {
			POP_ADDR(bse, off);
			POP(val1);
		}
		else {
			*ADDR(bse, off) = value - 1;
			ip = val2;
		}
		break;

	case I_JUMP:
		val2 = GET_WORD;
		ip = val2;
		break;

	case I_JUMP_IF_FALSE:
		val2 = GET_WORD;
		POP(value);
		if (BOOL(value) == 0)
			ip = val2;
		break;

	case I_JUMP_IF_TRUE:
		val2 = GET_WORD;
		POP(value);
		if (BOOL(value) == 1)
			ip = val2;
		break;

	case I_JUMP_IF_GREATER:
		val2 = GET_WORD;
		POP(value);
		if (value == 2)
			ip = val2;
		break;

	case I_JUMP_IF_GREATER_OR_EQUAL:
		val2 = GET_WORD;
		POP(value);
		if (value >= 1)
			ip = val2;
		break;

	case I_JUMP_IF_LESS:
		val2 = GET_WORD;
		POP(value);
		if (value == 0)
			ip = val2;
		break;

	case I_JUMP_IF_LESS_OR_EQUAL:
		val2 = GET_WORD;
		POP(value);
		if (value <= 1)
			ip = val2;
		break;

		/* Miscellanous Instructions */

	case I_LOAD_EXCEPTION_REGISTER:
		exr = GET_WORD;
		raise_cs = cs;
		raise_lin = lin;
		raise_reason = "Instruction";
		break;

	case I_INSTALL_HANDLER:
		BLOCK_FRAME->bf_handler = GET_WORD;
		break;

	case I_RAISE:
		exec_raise(exr, "");
		break;

	case I_RESTORE_STACK_POINTER:
		sp = GET_WORD + sfp;
		sp = cur_stack[sp];
		cur_stackptr = sp;
		break;

	case I_SAVE_STACK_POINTER:
		sp = GET_WORD + sfp;
		cur_stack[sp] = cur_stackptr;
		break;

	case I_STMT:
		lin = GET_WORD;
#ifdef TRACE
		if (line_trace) {
			sprintf(scrBuffer, "at line %d (tos %d)\n",lin,cur_stackptr);
			scrPutStr(scrBuffer);
		}
#endif
#ifdef GWUMON
		CWK_SET_TASK_LINE(lin, 1);
#endif
		break;

	case I_SUBSCRIPT:
		subscript();
		break;

	case I_SELECT:
		value = GET_WORD; /* retrieve parameter for select */
		rselect(value);
		break;

	case I_TEST_EXCEPTION_REGISTER:
		PUSH(exr == GET_WORD);
		break;

	case I_TYPE_LOCAL:
		GET_GAD(bse, off);
		type_elaborate(1,bse,off);
		break;

	case I_TYPE_GLOBAL:
		GET_GAD(bse, off);
		type_elaborate(0,bse,off);
		break;

	case I_SUBPROGRAM:
		GET_LAD(bse,off);
		subprogram(bse,off);
		break;

	case I_CHECK_REC_SUBTYPE:
		POP_ADDR(bse, off);
		check_subtype_with_discr (ADDR (bse, off), NULL_INT);
		break;

	default:
		exec_raise(SYSTEM_ERROR, "Bad opcode");
		return 0;
	}

	return 1;
}
