/*
 * Copyright (c) 1992 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Opcode execution module
 */

#include "xstdarg.h"
#include "calc.h"
#include "opcodes.h"
#include "func.h"
#include "symbol.h"

#define	QUICKLOCALS	20		/* local vars to handle quickly */


VALUE *stack;				/* current location of top of stack */
static VALUE stackarray[MAXSTACK];	/* storage for stack */
static VALUE oldvalue;			/* previous calculation value */
static char *funcname;			/* function being executed */
static long funcline;			/* function line being executed */

FLAG traceflags;			/* current trace flags */


/*
 * Routine definitions
 */
static long o_nop(), o_localaddr(), o_globaladdr(), o_paramaddr();
static long o_globalvalue(), o_paramvalue(), o_number(), o_indexaddr();
static long o_indexvalue(), o_assign(), o_add(), o_sub(), o_mul(), o_div();
static long o_mod(), o_save(), o_negate(), o_invert(), o_int(), o_frac();
static long o_numerator(), o_denominator(), o_duplicate(), o_pop();
static long o_jumpeq(), o_jumpne(), o_jump(), o_usercall(), o_getvalue();
static long o_eq(), o_ne(), o_le(), o_ge(), o_lt(), o_gt(), o_preinc();
static long o_postinc(), o_postdec(), o_debug(), o_print(), o_assignpop();
static long o_zero(), o_one(), o_printeol(), o_printspace(), o_printstring();
static long o_oldvalue(), o_quo(), o_power(), o_quit(), o_call(), o_swap();
static long o_dupvalue(), o_getepsilon(), o_and(), o_or(), o_not();
static long o_abs(), o_sgn(), o_isint(), o_condorjump(), o_condandjump();
static long o_square(), o_string(), o_isnum(), o_undef(), o_isnull();
static long o_matinit(), o_ismat(), o_isstr(), o_getconfig(), o_predec();
static long o_leftshift(), o_rightshift(), o_casejump();
static long o_isodd(), o_iseven(), o_fiaddr(), o_fivalue(), o_argvalue();
static long o_isreal(), o_imaginary(), o_re(), o_im(), o_conjugate();
static long o_objinit(), o_isobj(), o_norm(), o_elemaddr(), o_elemvalue();
static long o_istype(), o_scale(), o_localvalue(), o_return(), o_islist();
static long o_issimple(), o_cmp(), o_quomod(), o_setconfig(), o_setepsilon();
static long o_printresult(), o_isfile();


/*
 * Types of opcodes (depends on arguments saved after the opcode).
 */
#define OPNUL	1	/* opcode has no arguments */
#define OPONE	2	/* opcode has one integer argument */
#define OPTWO	3	/* opcode has two integer arguments */
#define OPJMP	4	/* opcode is a jump (with one integer argument) */
#define OPRET	5	/* opcode is a return (with no argument) */
#define OPGLB	6	/* opcode has global symbol pointer argument */
#define OPPAR	7	/* opcode has parameter index argument */
#define OPLOC	8	/* opcode needs local variable pointer (with one arg) */
#define OPSTR	9	/* opcode has a string constant arg */
#define OPARG	10	/* opcode is given number of arguments */


/*
 * Information about each opcode.
 */
static struct opcode {
	long (*o_func)();	/* routine to call for opcode */
	int o_type;		/* type of opcode */
	char *o_name;		/* name of opcode */
} opcodes[MAX_OPCODE+1] = {
	o_nop,		OPNUL,  "NOP",		/* no operation */
	o_localaddr,	OPLOC,  "LOCALADDR",	/* address of local variable */
	o_globaladdr,	OPGLB,  "GLOBALADDR",	/* address of global variable */
	o_paramaddr,	OPPAR,  "PARAMADDR",	/* address of paramater variable */
	o_localvalue,	OPLOC,  "LOCALVALUE",	/* value of local variable */
	o_globalvalue,	OPGLB,  "GLOBALVALUE",	/* value of global variable */
	o_paramvalue,	OPPAR,  "PARAMVALUE", 	/* value of paramater variable */
	o_number,	OPONE,  "NUMBER",	/* constant real numeric value */
	o_indexaddr,	OPONE,  "INDEXADDR",	/* array index address */
	o_indexvalue,	OPONE,  "INDEXVALUE", 	/* array value */
	o_assign,	OPNUL,  "ASSIGN",	/* assign value to variable */
	o_add,		OPNUL,  "ADD",		/* add top two values */
	o_sub,		OPNUL,  "SUB",		/* subtract top two values */
	o_mul,		OPNUL,  "MUL",		/* multiply top two values */
	o_div,		OPNUL,  "DIV",		/* divide top two values */
	o_mod,		OPNUL,  "MOD",		/* take mod of top two values */
	o_save,		OPNUL,  "SAVE",		/* save value for later use */
	o_negate,	OPNUL,  "NEGATE",	/* negate top value */
	o_invert,	OPNUL,  "INVERT",	/* invert top value */
	o_int,		OPNUL,  "INT",		/* take integer part */
	o_frac,		OPNUL,  "FRAC",		/* take fraction part */
	o_numerator,	OPNUL,  "NUMERATOR",	/* take numerator */
	o_denominator,	OPNUL,  "DENOMINATOR",	/* take denominator */
	o_duplicate,	OPNUL,  "DUPLICATE",	/* duplicate top value */
	o_pop,		OPNUL,  "POP",		/* pop top value */
	o_return,	OPRET,  "RETURN",	/* return value of function */
	o_jumpeq,	OPJMP,  "JUMPEQ",	/* jump if value zero */
	o_jumpne,	OPJMP,  "JUMPNE",	/* jump if value nonzero */
	o_jump,		OPJMP,  "JUMP",		/* jump unconditionally */
	o_usercall,	OPTWO,  "USERCALL",	/* call a user function */
	o_getvalue,	OPNUL,  "GETVALUE",	/* convert address to value */
	o_eq,		OPNUL,  "EQ",		/* test elements for equality */
	o_ne,		OPNUL,  "NE",		/* test elements for inequality */
	o_le,		OPNUL,  "LE",		/* test elements for <= */
	o_ge,		OPNUL,  "GE",		/* test elements for >= */
	o_lt,		OPNUL,  "LT",		/* test elements for < */
	o_gt,		OPNUL,  "GT",		/* test elements for > */
	o_preinc,	OPNUL,  "PREINC",	/* add one to variable (++x) */
	o_predec,	OPNUL,  "PREDEC",	/* subtract one from variable (--x) */
	o_postinc,	OPNUL,  "POSTINC",	/* add one to variable (x++) */
	o_postdec,	OPNUL,  "POSTDEC",	/* subtract one from variable (x--) */
	o_debug,	OPONE,  "DEBUG",	/* debugging point */
	o_print,	OPONE,  "PRINT",	/* print value */
	o_assignpop,	OPNUL,  "ASSIGNPOP",	/* assign to variable and pop it */
	o_zero,		OPNUL,  "ZERO",		/* put zero on the stack */
	o_one,		OPNUL,  "ONE",		/* put one on the stack */
	o_printeol,	OPNUL,  "PRINTEOL",	/* print end of line */
	o_printspace,	OPNUL,  "PRINTSPACE",	/* print a space */
	o_printstring,	OPSTR,  "PRINTSTR",	/* print constant string */
	o_dupvalue,	OPNUL,  "DUPVALUE",	/* duplicate value of top value */
	o_oldvalue,	OPNUL,  "OLDVALUE",	/* old value from previous calc */
	o_quo,		OPNUL,  "QUO",		/* integer quotient of top values */
	o_power,	OPNUL,  "POWER",	/* value raised to a power */
	o_quit,		OPSTR,  "QUIT",		/* quit program */
	o_call,		OPTWO,  "CALL",		/* call built-in routine */
	o_getepsilon,	OPNUL,  "GETEPSILON",	/* get allowed error for calculations */
	o_and,		OPNUL,  "AND",		/* arithmetic and or top two values */
	o_or,		OPNUL,  "OR",		/* arithmetic or of top two values */
	o_not,		OPNUL,  "NOT",		/* logical not or top value */
	o_abs,		OPNUL,  "ABS",		/* absolute value of top value */
	o_sgn,		OPNUL,  "SGN",		/* sign of number */
	o_isint,	OPNUL,  "ISINT",	/* whether number is an integer */
	o_condorjump,	OPJMP,  "CONDORJUMP",	/* conditional or jump */
	o_condandjump,	OPJMP,  "CONDANDJUMP",	/* conditional and jump */
	o_square,	OPNUL,  "SQUARE",	/* square top value */
	o_string,	OPSTR,  "STRING",	/* string constant value */
	o_isnum,	OPNUL,  "ISNUM",	/* whether value is a number */
	o_undef,	OPNUL,  "UNDEF",	/* load undefined value on stack */
	o_isnull,	OPNUL,  "ISNULL",	/* whether value is the null value */
	o_argvalue,	OPARG,  "ARGVALUE",	/* load value of arg (parameter) n */
	o_matinit,	OPONE,  "MATINIT",	/* initialize matrix */
	o_ismat,	OPNUL,  "ISMAT",	/* whether value is a matrix */
	o_isstr,	OPNUL,  "ISSTR",	/* whether value is a string */
	o_getconfig,	OPNUL,  "GETCONFIG",	/* get value of configuration parameter */
	o_leftshift,	OPNUL,  "LEFTSHIFT",	/* left shift of integer */
	o_rightshift,	OPNUL,  "RIGHTSHIFT",	/* right shift of integer */
	o_casejump,	OPJMP,  "CASEJUMP",	/* test case and jump if not matched */
	o_isodd,	OPNUL,  "ISODD",	/* whether value is odd integer */
	o_iseven,	OPNUL,  "ISEVEN",	/* whether value is even integer */
	o_fiaddr,	OPNUL,  "FIADDR",	/* 'fast index' matrix address */
	o_fivalue,	OPNUL,  "FIVALUE",	/* 'fast index' matrix value */
	o_isreal,	OPNUL,  "ISREAL",	/* whether value is real number */
	o_imaginary,	OPONE,  "IMAGINARY",	/* constant imaginary numeric value */
	o_re,		OPNUL,  "RE",		/* real part of complex number */
	o_im,		OPNUL,  "IM",		/* imaginary part of complex number */
	o_conjugate,	OPNUL,  "CONJUGATE",	/* complex conjugate */
	o_objinit,	OPONE,  "OBJINIT",	/* initialize object */
	o_isobj,	OPNUL,  "ISOBJ",	/* whether value is an object */
	o_norm,		OPNUL,  "NORM",		/* norm of value (square of abs) */
	o_elemaddr,	OPONE,  "ELEMADDR",	/* address of element of object */
	o_elemvalue,	OPONE,  "ELEMVALUE",	/* value of element of object */
	o_istype,	OPNUL,  "ISTYPE",	/* whether types are the same */
	o_scale,	OPNUL,  "SCALE",	/* scale value by a power of two */
	o_islist,	OPNUL,	"ISLIST",	/* whether value is a list */
	o_swap,		OPNUL,	"SWAP",		/* swap values of two variables */
	o_issimple,	OPNUL,	"ISSIMPLE",	/* whether value is simple type */
	o_cmp,		OPNUL,	"CMP",		/* compare values returning -1, 0, 1 */
	o_quomod,	OPNUL,	"QUOMOD",	/* calculate quotient and remainder */
	o_setconfig,	OPNUL,	"SETCONFIG",	/* set configuration parameter */
	o_setepsilon,	OPNUL,  "SETEPSILON",	/* set allowed error for calculations */
	o_printresult,	OPNUL,  "PRINTRESULT",	/* print result of top-level expression */
	o_isfile,	OPNUL,  "ISFILE"	/* whether value is a file */
};



/*
 * Initialize the stack.
 */
void
initstack()
{
	if (stack == NULL)
		stack = stackarray;
	while (stack != stackarray)
		freevalue(stack--);
}


/*
 * Compute the result of a function by interpreting opcodes.
 * Arguments have just been pushed onto the evaluation stack.
 */
void
calculate(fp, argcount)
	register FUNC *fp;		/* function to calculate */
	int argcount;			/* number of arguments called with */
{
	register unsigned long pc;	/* current pc inside function */
	register struct opcode *op;	/* current opcode pointer */
	register VALUE *locals;		/* pointer to local variables */
	long oldline;			/* old value of line counter */
	unsigned int opnum;		/* current opcode number */
	int origargcount;		/* original number of arguments */
	int i;				/* loop counter */
	char *oldname;			/* old function name being executed */
	VALUE *beginstack;		/* beginning of stack frame */
	VALUE *args;			/* pointer to function arguments */
	VALUE retval;			/* function return value */
	VALUE localtable[QUICKLOCALS];	/* some local variables */

	oldname = funcname;
	oldline = funcline;
	funcname = fp->f_name;
	funcline = 0;
	origargcount = argcount;
	while (argcount < fp->f_paramcount) {
		stack++;
		stack->v_type = V_NULL;
		argcount++;
	}
	locals = localtable;
	if (fp->f_localcount > QUICKLOCALS) {
		locals = (VALUE *) malloc(sizeof(VALUE) * fp->f_localcount);
		if (locals == NULL)
			error("No memory for local variables");
	}
	for (i = 0; i < fp->f_localcount; i++)
		locals[i].v_type = V_NULL;
	pc = 0;
	beginstack = stack;
	args = beginstack - (argcount - 1);
	for (;;) {
		if (abortlevel >= ABORT_OPCODE)
			error("Calculation aborted in opcode");
		if (pc >= fp->f_opcodecount)
			error("Function pc out of range");
		if (stack > &stackarray[MAXSTACK-3])
			error("Evaluation stack depth exceeded");
		opnum = fp->f_opcodes[pc];
		if (opnum > MAX_OPCODE)
			error("Function opcode out of range");
		op = &opcodes[opnum];
		if (traceflags & TRACE_OPCODES) {
			printf("%8s, pc %4ld:  ", fp->f_name, pc);
			(void)dumpop(&fp->f_opcodes[pc]);
		}
		/*
		 * Now call the opcode routine appropriately.
		 */
		pc++;
		switch (op->o_type) {
		case OPNUL:	/* no extra arguments */
			(*op->o_func)(fp);
			break;

		case OPONE:	/* one extra integer argument */
			(*op->o_func)(fp, fp->f_opcodes[pc++]);
			break;

		case OPTWO:	/* two extra integer arguments */
			(*op->o_func)(fp, fp->f_opcodes[pc],
				fp->f_opcodes[pc+1]);
			pc += 2;
			break;

		case OPJMP:	/* jump opcodes (one extra integer arg) */
			pc = (*op->o_func)(fp, pc);
			break;

		case OPGLB:	/* global symbol reference (pointer arg) */
		case OPSTR:	/* string constant address */
			(*op->o_func)(fp, *((char **) &fp->f_opcodes[pc]));
			pc += PTR_SIZE;
			break;

		case OPLOC:	/* local variable reference */
			(*op->o_func)(fp, locals, fp->f_opcodes[pc++]);
			break;

		case OPPAR:	/* parameter variable reference */
			(*op->o_func)(fp, argcount, args, fp->f_opcodes[pc++]);
			break;

		case OPARG:	/* parameter variable reference */
			(*op->o_func)(fp, origargcount, args);
			break;

		case OPRET:	/* return from function */
			if (stack->v_type == V_ADDR)
				copyvalue(stack->v_addr, stack);
			for (i = 0; i < fp->f_localcount; i++)
				freevalue(&locals[i]);
			if (locals != localtable)
				free(locals);
			if (stack != &beginstack[1])
				error("Misaligned stack");
			if (argcount <= 0) {
				funcname = oldname;
				funcline = oldline;
				return;
			}
			retval = *stack--;
			while (--argcount >= 0)
				freevalue(stack--);
			*++stack = retval;
			funcname = oldname;
			funcline = oldline;
			return;

		default:
			error("Unknown opcode type");
		}
	}
}


/*
 * Dump an opcode at a particular address.
 * Returns the size of the opcode so that it can easily be skipped over.
 */
int
dumpop(pc)
	long *pc;		/* location of the opcode */
{
	unsigned long op;	/* opcode number */

	op = *pc++;
	if (op <= MAX_OPCODE)
		printf("%s", opcodes[op].o_name);
	else
		printf("OP%ld", op);
	switch (op) {
		case OP_LOCALADDR: case OP_LOCALVALUE:
			printf(" %s\n", localname(*pc));
			return 2;
		case OP_GLOBALADDR: case OP_GLOBALVALUE:
			printf(" %s\n", globalname((GLOBAL *) pc));
			return (1 + PTR_SIZE);
		case OP_PARAMADDR: case OP_PARAMVALUE:
			printf(" %s\n", paramname(*pc));
			return 2;
		case OP_PRINTSTRING: case OP_STRING:
			printf(" \"%s\"\n", *((char **) pc));
			return (1 + PTR_SIZE);
		case OP_QUIT:
			if (*(char **) pc)
				printf(" \"%s\"\n", *((char **) pc));
			else
				printf("\n");
			return (1 + PTR_SIZE);
		case OP_MATINIT: case OP_INDEXADDR: case OP_INDEXVALUE:
		case OP_PRINT: case OP_JUMPEQ: case OP_JUMPNE: case OP_JUMP:
		case OP_CONDORJUMP: case OP_CONDANDJUMP: case OP_CASEJUMP:
		case OP_OBJINIT:
			printf(" %ld\n", *pc);
			return 2;
		case OP_NUMBER: case OP_IMAGINARY:
			qprintf(" %r\n", constvalue(*pc));
			return 2;
		case OP_DEBUG:
			printf(" line %ld\n", *pc);
			return 2;
		case OP_CALL:
			printf(" %s with %ld args\n", builtinname(pc[0]), pc[1]);
			return 3;
		case OP_USERCALL:
			printf(" %s with %ld args\n", namefunc(pc[0]), pc[1]);
			return 3;
		default:
			printf("\n");
			return 1;
	}
}


/*
 * The various opcodes
 */

static long
o_nop()
{
	return 0;
}


static long
o_localaddr(fp, locals, index)
	FUNC *fp;
	VALUE *locals;
	long index;
{
	if ((unsigned long)index >= fp->f_localcount)
		error("Bad local variable index");
	locals += index;
	stack++;
	stack->v_addr = locals;
	stack->v_type = V_ADDR;
	return 0;
}


/*ARGSUSED*/
static long
o_globaladdr(fp, sp)
	FUNC *fp;
	GLOBAL *sp;
{
	if (sp == NULL)
		error("Global variable \"%s\" not initialized", sp->g_name);
	stack++;
	stack->v_addr = &sp->g_value;
	stack->v_type = V_ADDR;
	return 0;
}


/*ARGSUSED*/
static long
o_paramaddr(fp, argcount, args, index)
	FUNC *fp;
	int argcount;
	VALUE *args;
	long index;
{
	if ((unsigned long)index >= argcount)
		error("Bad parameter index");
	args += index;
	stack++;
	if (args->v_type == V_ADDR)
		stack->v_addr = args->v_addr;
	else
		stack->v_addr = args;
	stack->v_type = V_ADDR;
	return 0;
}


static long
o_localvalue(fp, locals, index)
	FUNC *fp;
	VALUE *locals;
	long index;
{
	if ((unsigned long)index >= fp->f_localcount)
		error("Bad local variable index");
	locals += index;
	copyvalue(locals, ++stack);
	return 0;
}


/*ARGSUSED*/
static long
o_globalvalue(fp, sp)
	FUNC *fp;
	GLOBAL *sp;		/* global symbol */
{
	if (sp == NULL)
		error("Global variable not defined");
	copyvalue(&sp->g_value, ++stack);
	return 0;
}


/*ARGSUSED*/
static long
o_paramvalue(fp, argcount, args, index)
	FUNC *fp;
	int argcount;
	VALUE *args;
	long index;
{
	if ((unsigned long)index >= argcount)
		error("Bad paramaeter index");
	args += index;
	if (args->v_type == V_ADDR)
		args = args->v_addr;
	copyvalue(args, ++stack);
	return 0;
}


static long
o_argvalue(fp, argcount, args)
	FUNC *fp;
	int argcount;
	VALUE *args;
{
	VALUE *vp;
	long index;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if ((vp->v_type != V_NUM) || qisneg(vp->v_num) ||
		qisfrac(vp->v_num))
			error("Illegal argument for arg function");
	if (qiszero(vp->v_num)) {
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = itoq((long) argcount);
		stack->v_type = V_NUM;
		return 0;
	}
	index = qtoi(vp->v_num) - 1;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	(void) o_paramvalue(fp, argcount, args, index);
	return 0;
}


/*ARGSUSED*/
static long
o_number(fp, arg)
	FUNC *fp;
	long arg;
{
	NUMBER *q;

	q = constvalue(arg);
	if (q == NULL)
		error("Numeric constant value not found");
	stack++;
	stack->v_num = qlink(q);
	stack->v_type = V_NUM;
	return 0;
}


/*ARGSUSED*/
static long
o_imaginary(fp, arg)
	FUNC *fp;
	long arg;
{
	NUMBER *q;
	COMPLEX *c;

	q = constvalue(arg);
	if (q == NULL)
		error("Numeric constant value not found");
	stack++;
	if (qiszero(q)) {
		stack->v_num = qlink(q);
		stack->v_type = V_NUM;
		return 0;
	}
	c = comalloc();
	c->real = qlink(&_qzero_);
	c->imag = qlink(q);
	stack->v_com = c;
	stack->v_type = V_COM;
	return 0;
}


/*ARGSUSED*/
static long
o_string(fp, cp)
	FUNC *fp;
	char *cp;
{
	stack++;
	stack->v_str = cp;
	stack->v_type = V_STR;
	stack->v_subtype = V_STRLITERAL;
	return 0;
}


static long
o_undef()
{
	stack++;
	stack->v_type = V_NULL;
	return 0;
}


/*ARGSUSED*/
static long
o_matinit(fp, dim)
	FUNC *fp;
	long dim;
{
	register MATRIX *mp;	/* matrix being defined */
	NUMBER *num1;		/* first number from stack */
	NUMBER *num2;		/* second number from stack */
	VALUE *vp;		/* value being defined */
	VALUE *v1, *v2;
	long min[MAXDIM];	/* minimum range */
	long max[MAXDIM];	/* maximum range */
	long i;			/* index */
	long tmp;		/* temporary */
	long size;		/* size of matrix */

	if ((dim <= 0) || (dim > MAXDIM))
		error("Bad dimension %ld for matrix", dim);
	if (stack[-2*dim].v_type != V_ADDR)
		error("Attempting to init matrix for non-address");
	size = 1;
	for (i = dim - 1; i >= 0; i--) {
		v1 = &stack[-1];
		v2 = &stack[0];
		if (v1->v_type == V_ADDR)
			v1 = v1->v_addr;
		if (v2->v_type == V_ADDR)
			v2 = v2->v_addr;
		if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM))
			error("Non-numeric bounds for matrix");
		num1 = v1->v_num;
		num2 = v2->v_num;
		if (qisfrac(num1) || qisfrac(num2))
			error("Non-integral bounds for matrix");
		if (isbig(num1->num) || isbig(num2->num))
			error("Very large bounds for matrix");
		min[i] = qtoi(num1);
		max[i] = qtoi(num2);
		if (min[i] > max[i]) {
			tmp = min[i];
			min[i] = max[i];
			max[i] = tmp;
		}
		size *= (max[i] - min[i] + 1);
		if (size > 1000000)
			error("Very large size for matrix");
		freevalue(stack--);
		freevalue(stack--);
	}
	mp = matalloc(size);
	mp->m_dim = dim;
	for (i = 0; i < dim; i++) {
		mp->m_min[i] = min[i];
		mp->m_max[i] = max[i];
	}
	vp = mp->m_table;
	for (i = 0; i < size; i++) {
		vp->v_type = V_NUM;
		vp->v_num = qlink(&_qzero_);
		vp++;
	}
	vp = stack[0].v_addr;
	vp->v_type = V_MAT;
	vp->v_mat = mp;
	stack--;
	return 0;
}


/*ARGSUSED*/
static long
o_indexaddr(fp, dim)
	FUNC *fp;
	long dim;		/* dimension of matrix */
{
	register MATRIX *mp;	/* current matrix element */
	VALUE *curvp;		/* current stack address */
	VALUE *vp;		/* real stack value */
	NUMBER *q;		/* index value */
	long index;		/* index value as an integer */
	long offset;		/* current offset into array */
	int i;			/* loop counter */

	if ((dim <= 0) || (dim > MAXDIM))
		error("Bad dimension %ld for matrix", dim);
	if (stack[-dim].v_type != V_ADDR)
		error("Non-pointer for index operation");
	if (stack[-dim].v_addr->v_type != V_MAT)
		error("Attempting to index a non-matrix variable");
	mp = stack[-dim].v_addr->v_mat;
	if (mp->m_dim != dim)
		error("Indexing a %ldd matrix as a %ldd matrix", mp->m_dim, dim);
	offset = 0;
	curvp = &stack[-dim + 1];
	for (i = 0; i < dim; i++) {
		vp = curvp;
		if (vp->v_type == V_ADDR)
			vp = vp->v_addr;
		if (vp->v_type != V_NUM)
			error("Non-numeric index for array");
		q = vp->v_num;
		if (qisfrac(q))
			error("Non-integral index for array");
		index = qtoi(q);
		if (isbig(q->num) || (index < mp->m_min[i]) || (index > mp->m_max[i]))
			error("Index out of bounds");
		offset *= (mp->m_max[i] - mp->m_min[i] + 1);
		offset += (index - mp->m_min[i]);
		freevalue(curvp++);
	}
	stack -= dim;
	stack->v_type = V_ADDR;
	stack->v_addr = mp->m_table + offset;
	return 0;
}


static long
o_indexvalue(fp, dim)
	FUNC *fp;
	long dim;
{
	(void) o_indexaddr(fp, dim);
	(void) o_getvalue();
	return 0;
}


/*ARGSUSED*/
static long
o_elemaddr(fp, index)
	FUNC *fp;
	long index;
{
	if (stack->v_type != V_ADDR)
		error("Non-pointer for element reference");
	if (stack->v_addr->v_type != V_OBJ)
		error("Referencing element of non-object");
	index = objoffset(stack->v_addr->v_obj, index);
	if (index < 0)
		error("Element does not exist for object");
	stack->v_addr = &stack->v_addr->v_obj->o_table[index];
	return 0;
}


static long
o_elemvalue(fp, index)
	FUNC *fp;
	long index;
{
	if (stack->v_type != V_OBJ) {
		(void) o_elemaddr(fp, index);
		(void) o_getvalue();
		return 0;
	}
	index = objoffset(stack->v_obj, index);
	if (index < 0)
		error("Element does not exist for object");
	copyvalue(&stack->v_obj->o_table[index], stack);
	return 0;
}


/*ARGSUSED*/
static long
o_objinit(fp, arg)
	FUNC *fp;
	long arg;
{
	OBJECT *op;		/* object being created */
	VALUE *vp;		/* value being defined */

	if (stack->v_type != V_ADDR)
		error("Attempting to init object for non-address");
	op = objalloc(arg);
	vp = stack->v_addr;
	vp->v_type = V_OBJ;
	vp->v_obj = op;
	stack--;
	return 0;
}


static long
o_assign()
{
	VALUE *var;		/* variable value */
	VALUE *vp;

	var = &stack[-1];
	if (var->v_type != V_ADDR)
		error("Assignment into non-variable");
	var = var->v_addr;
	stack[-1] = stack[0];
	stack--;
	vp = stack;
	if (vp->v_type == V_ADDR) {
		vp = vp->v_addr;
		if (vp == var)
			return 0;
	}
	freevalue(var);
	copyvalue(vp, var);
	return 0;
}


static long
o_assignpop()
{
	VALUE *var;		/* variable value */
	VALUE *vp;

	var = &stack[-1];
	if (var->v_type != V_ADDR)
		error("Assignment into non-variable");
	var = var->v_addr;
	vp = &stack[0];
	if ((vp->v_type == V_ADDR) && (vp->v_addr == var)) {
		stack -= 2;
		return 0;
	}
	freevalue(var);
	if (vp->v_type == V_ADDR)
		copyvalue(vp->v_addr, var);
	else
		*var = *vp;
	stack -= 2;
	return 0;
}


static long
o_swap()
{
	VALUE *v1, *v2;		/* variables to be swapped */
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if ((v1->v_type != V_ADDR) || (v2->v_type != V_ADDR))
		error("Swapping non-variables");
	tmp = v1->v_addr[0];
	v1->v_addr[0] = v2->v_addr[0];
	v2->v_addr[0] = tmp;
	stack--;
	stack->v_type = V_NULL;
	return 0;
}


static long
o_add()
{
	VALUE *v1, *v2;
	NUMBER *q;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM)) {
		addvalue(v1, v2, &tmp);
		freevalue(stack--);
		freevalue(stack);
		*stack = tmp;
		return 0;
	}
	q = qadd(v1->v_num, v2->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_sub()
{
	VALUE *v1, *v2;
	NUMBER *q;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM)) {
		subvalue(v1, v2, &tmp);
		freevalue(stack--);
		freevalue(stack);
		*stack = tmp;
		return 0;
	}
	q = qsub(v1->v_num, v2->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_mul()
{
	VALUE *v1, *v2;
	NUMBER *q;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM)) {
		mulvalue(v1, v2, &tmp);
		freevalue(stack--);
		freevalue(stack);
		*stack = tmp;
		return 0;
	}
	q = qmul(v1->v_num, v2->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_power()
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	powivalue(v1, v2, &tmp);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
	return 0;
}


static long
o_div()
{
	VALUE *v1, *v2;
	NUMBER *q;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM)) {
		divvalue(v1, v2, &tmp);
		freevalue(stack--);
		freevalue(stack);
		*stack = tmp;
		return 0;
	}
	q = qdiv(v1->v_num, v2->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_quo()
{
	VALUE *v1, *v2;
	NUMBER *q;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM)) {
		quovalue(v1, v2, &tmp);
		freevalue(stack--);
		freevalue(stack);
		*stack = tmp;
		return 0;
	}
	q = qquo(v1->v_num, v2->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_mod()
{
	VALUE *v1, *v2;
	NUMBER *q;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM)) {
		modvalue(v1, v2, &tmp);
		freevalue(stack--);
		freevalue(stack);
		*stack = tmp;
		return 0;
	}
	q = qmod(v1->v_num, v2->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_quomod()
{
	VALUE *v1, *v2, *v3, *v4;
	VALUE valquo, valmod;
	BOOL res;

	v1 = &stack[-3];
	v2 = &stack[-2];
	v3 = &stack[-1];
	v4 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v3->v_type != V_ADDR) || (v4->v_type != V_ADDR))
		error("Non-variable for quomod");
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM))
		error("Non-reals for quomod");
	v3 = v3->v_addr;
	v4 = v4->v_addr;
	valquo.v_type = V_NUM;
	valmod.v_type = V_NUM;
	res = qquomod(v1->v_num, v2->v_num, &valquo.v_num, &valmod.v_num);
	freevalue(stack--);
	freevalue(stack--);
	stack--;
	stack->v_num = (res ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
	freevalue(v3);
	freevalue(v4);
	*v3 = valquo;
	*v4 = valmod;
	return 0;
}


static long
o_and()
{
	VALUE *v1, *v2;
	NUMBER *q;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM))
		error("Non-numerics for and");
	q = qand(v1->v_num, v2->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_or()
{
	VALUE *v1, *v2;
	NUMBER *q;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM))
		error("Non-numerics for or");
	q = qor(v1->v_num, v2->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_not()
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = testvalue(vp);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qzero_) : qlink(&_qone_));		
	stack->v_type = V_NUM;
	return 0;
}


static long
o_negate()
{
	VALUE *vp;
	NUMBER *q;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		q = qneg(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = q;
		stack->v_type = V_NUM;
		return 0;
	}
	negvalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
	return 0;
}


static long
o_invert()
{
	VALUE *vp;
	NUMBER *q;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		q = qinv(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = q;
		stack->v_type = V_NUM;
		return 0;
	}
	invertvalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
	return 0;
}


static long
o_scale()
{
	VALUE *v1, *v2;
	NUMBER *q;
	VALUE tmp;

	v1 = &stack[0];
	v2 = &stack[-1];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM)) {
		scalevalue(v2, v1, &tmp);
		freevalue(stack--);
		freevalue(stack);
		*stack = tmp;
		return 0;
	}
	q = v1->v_num;
	if (qisfrac(q))
		error("Non-integral scaling factor");
	if (isbig(q->num))
		error("Very large scaling factor");
	q = qscale(v2->v_num, qtoi(q));
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_int()
{
	VALUE *vp;
	NUMBER *q;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		if (qisint(vp->v_num) && (stack->v_type == V_NUM))
			return 0;
		q = qint(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = q;
		stack->v_type = V_NUM;
		return 0;
	}
	intvalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
	return 0;
}


static long
o_frac()
{
	VALUE *vp;
	NUMBER *q;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		q = qfrac(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = q;
		stack->v_type = V_NUM;
		return 0;
	}
	fracvalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
	return 0;
}


static long
o_abs()
{
	VALUE *v1, *v2;
	NUMBER *q;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM) ||
		!qispos(v2->v_num))
	{
		absvalue(v1, v2, &tmp);
		freevalue(stack--);
		freevalue(stack);
		*stack = tmp;
		return 0;
	}
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	if ((stack->v_type == V_NUM) && !qisneg(v1->v_num))
		return 0;
	q = qabs(v1->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_norm()
{
	VALUE *vp;
	NUMBER *q;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		q = qsquare(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = q;
		stack->v_type = V_NUM;
		return 0;
	}
	normvalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
	return 0;
}


static long
o_square()
{
	VALUE *vp;
	NUMBER *q;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		q = qsquare(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = q;
		stack->v_type = V_NUM;
		return 0;
	}
	squarevalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
	return 0;
}


static long
o_istype()
{
	VALUE *v1, *v2;
	int r;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_OBJ) || (v2->v_type != V_OBJ))
		r = (v1->v_type == v2->v_type);
	else
		r = (v1->v_obj->o_actions == v2->v_obj->o_actions);
	freevalue(stack--);
	freevalue(stack);
	stack->v_num = itoq((long) r);
	stack->v_type = V_NUM;
	return 0;
}


static long
o_isint()
{
	VALUE *vp;
	NUMBER *q;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = stack->v_addr;
	if (vp->v_type != V_NUM) {
		freevalue(stack);
		stack->v_num = qlink(&_qzero_);
		stack->v_type = V_NUM;
		return 0;
	}
	if (qisint(vp->v_num))
		q = qlink(&_qone_);
	else
		q = qlink(&_qzero_);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_isnum()
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	switch (vp->v_type) {
		case V_NUM:
			if (stack->v_type == V_NUM)
				qfree(stack->v_num);
			break;
		case V_COM:
			if (stack->v_type == V_COM)
				comfree(stack->v_com);
			break;
		default:
			freevalue(stack);
			stack->v_num = qlink(&_qzero_);
			stack->v_type = V_NUM;
			return 0;
	}
	stack->v_num = qlink(&_qone_);
	stack->v_type = V_NUM;
	return 0;
}


static long
o_ismat()
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_MAT) {
		freevalue(stack);
		stack->v_num = qlink(&_qzero_);
		stack->v_type = V_NUM;
		return 0;
	}
	freevalue(stack);
	stack->v_type = V_NUM;
	stack->v_num = qlink(&_qone_);
	return 0;
}


static long
o_islist()
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = (vp->v_type == V_LIST);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
	return 0;
}


static long
o_isobj()
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = (vp->v_type == V_OBJ);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
	return 0;
}


static long
o_isstr()
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = (vp->v_type == V_STR);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
	return 0;
}


static long
o_isfile()
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = (vp->v_type == V_FILE);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
	return 0;
}


static long
o_issimple()
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = 0;
	switch (vp->v_type) {
		case V_NULL:
		case V_NUM:
		case V_COM:
		case V_STR:
			r = 1;
	}
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
	return 0;
}


static long
o_isodd()
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if ((vp->v_type == V_NUM) && qisodd(vp->v_num)) {
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = qlink(&_qone_);
		stack->v_type = V_NUM;
		return 0;
	}
	freevalue(stack);
	stack->v_num = qlink(&_qzero_);
	stack->v_type = V_NUM;
	return 0;
}


static long
o_iseven()
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if ((vp->v_type == V_NUM) && qiseven(vp->v_num)) {
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = qlink(&_qone_);
		stack->v_type = V_NUM;
		return 0;
	}
	freevalue(stack);
	stack->v_num = qlink(&_qzero_);
	stack->v_type = V_NUM;
	return 0;
}


static long
o_isreal()
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = qlink(&_qone_);
		stack->v_type = V_NUM;
		return 0;
	}
	freevalue(stack);
	stack->v_num = qlink(&_qzero_);
	stack->v_type = V_NUM;
	return 0;
}


static long
o_isnull()
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_NULL) {
		freevalue(stack);
		stack->v_num = qlink(&_qzero_);
		stack->v_type = V_NUM;
		return 0;
	}
	freevalue(stack);
	stack->v_num = qlink(&_qone_);
	stack->v_type = V_NUM;
	return 0;
}


static long
o_re()
{
	VALUE *vp;
	NUMBER *q;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		if (stack->v_type == V_ADDR) {
			stack->v_num = qlink(vp->v_num);
			stack->v_type = V_NUM;
		}
		return 0;
	}
	if (vp->v_type != V_COM)
		error("Taking real part of non-number");
	q = qlink(vp->v_com->real);
	if (stack->v_type == V_COM)
		comfree(stack->v_com);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_im()
{
	VALUE *vp;
	NUMBER *q;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = qlink(&_qzero_);
		stack->v_type = V_NUM;
		return 0;
	}
	if (vp->v_type != V_COM)
		error("Taking imaginary part of non-number");
	q = qlink(vp->v_com->imag);
	if (stack->v_type == V_COM)
		comfree(stack->v_com);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_conjugate()
{
	VALUE *vp;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		if (stack->v_type == V_ADDR) {
			stack->v_num = qlink(vp->v_num);
			stack->v_type = V_NUM;
		}
		return 0;
	}
	conjvalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
	return 0;
}


static long
o_fiaddr()
{
	register MATRIX *m;	/* current matrix element */
	NUMBER *q;		/* index value */
	LIST *lp;		/* list header */
	VALUE *vp;		/* stack value */
	long index;		/* index value as an integer */

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_NUM)
		error("Fast indexing by non-number");
	q = vp->v_num;
	if (qisfrac(q))
		error("Fast indexing by non-integer");
	index = qtoi(q);
	if (isbig(q->num) || (index < 0))
		error("Index out of range for fast indexing");
	if (stack->v_type == V_NUM)
		qfree(q);
	stack--;
	vp = stack;
	if (vp->v_type != V_ADDR)
		error("Bad value for fast indexing");
	switch (vp->v_addr->v_type) {
		case V_OBJ:
			if (index >= vp->v_addr->v_obj->o_actions->count)
				error("Index out of bounds for object");
			vp->v_addr = vp->v_addr->v_obj->o_table + index;
			break;
		case V_MAT:
			m = vp->v_addr->v_mat;
			if (index >= m->m_size)
				error("Index out of bounds for matrix");
			vp->v_addr = m->m_table + index;
			break;
		case V_LIST:
			lp = vp->v_addr->v_list;
			vp->v_addr = listindex(lp, index);
			if (vp->v_addr == NULL)
				error("Index out of bounds for list");
			break;
		default:
			error("Bad variable type for fast indexing");
	}
	return 0;
}


static long
o_fivalue()
{
	(void) o_fiaddr();
	(void) o_getvalue();
	return 0;
}


static long
o_sgn()
{
	VALUE *vp;
	NUMBER *q;
	VALUE val;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	switch (vp->v_type) {
		case V_NUM:
			q = qsign(vp->v_num);
			if (stack->v_type == V_NUM)
				qfree(vp->v_num);
			stack->v_num = q;
			stack->v_type = V_NUM;
			break;
		case V_OBJ:
			val = objcall(OBJ_SGN, vp);
			q = itoq(val.v_int);
			freevalue(stack);
			stack->v_num = q;
			stack->v_type = V_NUM;
			break;
		default:
			error("Bad value for sgn");
	}
	return 0;
}


static long
o_numerator()
{
	VALUE *vp;
	NUMBER *q;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_NUM)
		error("Numerator of non-number");
	if ((stack->v_type == V_NUM) && qisint(vp->v_num))
		return 0;
	q = qnum(vp->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_denominator()
{
	VALUE *vp;
	NUMBER *q;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_NUM)
		error("Denominator of non-number");
	q = qden(vp->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
	return 0;
}


static long
o_duplicate()
{
	copyvalue(stack, stack + 1);
	stack++;
	return 0;
}


static long
o_dupvalue()
{
	if (stack->v_type == V_ADDR)
		copyvalue(stack->v_addr, stack + 1);
	else
		copyvalue(stack, stack + 1);
	stack++;
	return 0;
}


static long
o_pop()
{
	freevalue(stack--);
	return 0;
}


static long
o_return()
{
	return 0;
}


static long
o_jumpeq(fp, pc)
	FUNC *fp;
	long pc;
{
	VALUE *vp;
	int i;			/* result of comparison */

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		i = !qiszero(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
	} else {
		i = testvalue(vp);
		freevalue(stack);
	}
	stack--;
	if (i)
		return (pc + 1);
	return fp->f_opcodes[pc];
}


static long
o_jumpne(fp, pc)
	FUNC *fp;
	long pc;
{
	VALUE *vp;
	int i;			/* result of comparison */

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		i = !qiszero(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
	} else {
		i = testvalue(vp);
		freevalue(stack);
	}
	stack--;
	if (i)
		return fp->f_opcodes[pc];
	return (pc + 1);
}


static long
o_condorjump(fp, pc)
	FUNC *fp;
	long pc;
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		if (!qiszero(vp->v_num))
			return fp->f_opcodes[pc];
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack--;
		return pc + 1;
	}
	if (testvalue(vp))
		return fp->f_opcodes[pc];
	freevalue(stack--);
	return pc + 1;
}


static long
o_condandjump(fp, pc)
	FUNC *fp;
	long pc;
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		if (qiszero(vp->v_num))
			return fp->f_opcodes[pc];
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack--;
		return pc + 1;
	}
	if (!testvalue(vp))
		return fp->f_opcodes[pc];
	freevalue(stack--);
	return pc + 1;
}


/*
 * Compare the top two values on the stack for equality and jump if they are
 * different, popping off the top element, leaving the first one on the stack.
 * If they are equal, pop both values and do not jump.
 */
static long
o_casejump(fp, pc)
	FUNC *fp;
	long pc;
{
	VALUE *v1, *v2;
	int r;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	r = comparevalue(v1, v2);
	freevalue(stack--);
	if (r)
		return (fp->f_opcodes[pc]);
	freevalue(stack--);
	return (pc + 1);
}


static long
o_jump(fp, pc)
	FUNC *fp;
	long pc;
{
	return fp->f_opcodes[pc];
}


static long
o_usercall(fp, index, argcount)
	FUNC *fp;
	long index, argcount;
{
	fp = findfunc(index);
	if (fp == NULL)
		error("Function \"%s\" is undefined", namefunc(index));
	calculate(fp, (int) argcount);
	return 0;
}


/*ARGSUSED*/
static long
o_call(fp, index, argcount)
	FUNC *fp;
	long index, argcount;
{
	VALUE result;

	result = builtinfunc(index, (int) argcount, stack);
	while (--argcount >= 0)
		freevalue(stack--);
	stack++;
	*stack = result;
	return 0;
}


static long
o_getvalue()
{
	if (stack->v_type == V_ADDR)
		copyvalue(stack->v_addr, stack);
	return 0;
}


static long
o_cmp()
{
	VALUE *v1, *v2;
	int r;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	r = relvalue(v1, v2);
	freevalue(stack--);
	freevalue(stack);
	stack->v_num = itoq((long) r);
	stack->v_type = V_NUM;
	return 0;
}


static long
o_eq()
{
	VALUE *v1, *v2;
	int r;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	r = comparevalue(v1, v2);
	freevalue(stack--);
	freevalue(stack);
	stack->v_num = itoq((long) (r == 0));
	stack->v_type = V_NUM;
	return 0;
}


static long
o_ne()
{
	VALUE *v1, *v2;
	int r;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	r = comparevalue(v1, v2);
	freevalue(stack--);
	freevalue(stack);
	stack->v_num = itoq((long) (r != 0));
	stack->v_type = V_NUM;
	return 0;
}


static long
o_le()
{
	VALUE *v1, *v2;
	int r;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	r = relvalue(v1, v2);
	freevalue(stack--);
	freevalue(stack);
	stack->v_num = itoq((long) (r <= 0));
	stack->v_type = V_NUM;
	return 0;
}


static long
o_ge()
{
	VALUE *v1, *v2;
	int r;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	r = relvalue(v1, v2);
	freevalue(stack--);
	freevalue(stack);
	stack->v_num = itoq((long) (r >= 0));
	stack->v_type = V_NUM;
	return 0;
}


static long
o_lt()
{
	VALUE *v1, *v2;
	int r;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	r = relvalue(v1, v2);
	freevalue(stack--);
	freevalue(stack);
	stack->v_num = itoq((long) (r < 0));
	stack->v_type = V_NUM;
	return 0;
}


static long
o_gt()
{
	VALUE *v1, *v2;
	int r;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	r = relvalue(v1, v2);
	freevalue(stack--);
	freevalue(stack);
	stack->v_num = itoq((long) (r > 0));
	stack->v_type = V_NUM;
	return 0;
}


static long
o_preinc()
{
	NUMBER *q, **np;
	VALUE *vp, tmp;

	if (stack->v_type != V_ADDR)
		error("Preincrementing non-variable");
	if (stack->v_addr->v_type == V_NUM) {
		np = &stack->v_addr->v_num;
		q = qinc(*np);
		qfree(*np);
		*np = q;
		stack->v_type = V_NUM;
		stack->v_num = qlink(q);
		return 0;
	}
	vp = stack->v_addr;
	incvalue(vp, &tmp);
	freevalue(vp);
	*vp = tmp;
	copyvalue(&tmp, stack);
	return 0;
}


static long
o_predec()
{
	NUMBER *q, **np;
	VALUE *vp, tmp;

	if (stack->v_type != V_ADDR)
		error("Predecrementing non-variable");
	if (stack->v_addr->v_type == V_NUM) {
		np = &stack->v_addr->v_num;
		q = qdec(*np);
		qfree(*np);
		*np = q;
		stack->v_type = V_NUM;
		stack->v_num = qlink(q);
		return 0;
	}
	vp = stack->v_addr;
	decvalue(vp, &tmp);
	freevalue(vp);
	*vp = tmp;
	copyvalue(&tmp, stack);
	return 0;
}


static long
o_postinc()
{
	NUMBER *q, **np;
	VALUE *vp, tmp;

	if (stack->v_type != V_ADDR)
		error("Postincrementing non-variable");
	if (stack->v_addr->v_type == V_NUM) {
		np = &stack->v_addr->v_num;
		q = *np;
		*np = qinc(q);
		stack->v_type = V_NUM;
		stack->v_num = q;
		return 0;
	}
	vp = stack->v_addr;
	tmp = *vp;
	incvalue(&tmp, vp);
	*stack = tmp;
	return 0;
}


static long
o_postdec()
{
	NUMBER *q, **np;
	VALUE *vp, tmp;

	if (stack->v_type != V_ADDR)
		error("Postdecrementing non-variable");
	if (stack->v_addr->v_type == V_NUM) {
		np = &stack->v_addr->v_num;
		q = *np;
		*np = qdec(q);
		stack->v_type = V_NUM;
		stack->v_num = q;
		return 0;
	}
	vp = stack->v_addr;
	tmp = *vp;
	decvalue(&tmp, vp);
	*stack = tmp;
	return 0;
}


static long
o_leftshift()
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	shiftvalue(v1, v2, FALSE, &tmp);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
	return 0;
}


static long
o_rightshift()
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	shiftvalue(v1, v2, TRUE, &tmp);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
	return 0;
}


/*ARGSUSED*/
static long
o_debug(fp, line)
	FUNC *fp;
	long line;
{
	funcline = line;
	if (abortlevel >= ABORT_STATEMENT)
		error("Calculation aborted at statement boundary");
	return 0;
}


static long
o_printresult()
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_NULL) {
		printf("\t");
		printvalue(vp, PRINT_UNAMBIG);
		printf("\n");
		fflush(stdout);
	}
	freevalue(stack--);
	return 0;
}


/*ARGSUSED*/
static long
o_print(fp, flags)
	FUNC *fp;
	long flags;
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	printvalue(vp, (int) flags);
	freevalue(stack--);
	if (traceflags & TRACE_OPCODES)
		printf("\n");
	fflush(stdout);
	return 0;
}


static long
o_printeol()
{
	putchar('\n');
	fflush(stdout);
	return 0;
}


static long
o_printspace()
{
	putchar(' ');
	if (traceflags & TRACE_OPCODES)
		printf("\n");
	return 0;
}


/*ARGSUSED*/
static long
o_printstring(fp, cp)
	FUNC *fp;
	char *cp;
{
	fputs(cp, stdout);
	if (traceflags & TRACE_OPCODES)
		printf("\n");
	fflush(stdout);
	return 0;
}


static long
o_zero()
{
	stack++;
	stack->v_type = V_NUM;
	stack->v_num = qlink(&_qzero_);
	return 0;
}


static long
o_one()
{
	stack++;
	stack->v_type = V_NUM;
	stack->v_num = qlink(&_qone_);
	return 0;
}


static long
o_save(fp)
	FUNC *fp;
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	freevalue(&fp->f_savedvalue);
	copyvalue(vp, &fp->f_savedvalue);
	return 0;
}


/*ARGSUSED*/
static long
o_oldvalue(fp)
	FUNC *fp;
{
	copyvalue(&oldvalue, ++stack);
	return 0;
}


static long
o_quit(fp, cp)
	FUNC *fp;
	char *cp;
{
	if ((fp->f_name[0] == '*') && (fp->f_name[1] == '\0')) {
		if (cp)
			printf("%s\n", cp);
		exit(0);
	}
	if (cp)
		error("%s", cp);
	error("quit statement executed");
	return 0;
}


static long
o_getepsilon()
{
	stack++;
	stack->v_type = V_NUM;
	stack->v_num = qlink(_epsilon_);
	return 0;
}


static long
o_setepsilon()
{
	VALUE *vp;
	NUMBER *new;

	vp = &stack[0];
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_NUM)
		error("Non-numeric for epsilon");
	new = vp->v_num;
	stack->v_num = qlink(_epsilon_);
	setepsilon(new);
	qfree(new);
	return 0;
}


static long
o_setconfig()
{
	int type;
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if (v1->v_type != V_STR)
		error("Non-string for config");
	type = configtype(v1->v_str);
	if (type < 0)
		error("Unknown config name \"%s\"", v1->v_str);
	getconfig(type, &tmp);
	setconfig(type, v2);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
	return 0;
}


static long
o_getconfig()
{
	int type;
	VALUE *vp;

	vp = &stack[0];
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_STR)
		error("Non-string for config");
	type = configtype(vp->v_str);
	if (type < 0)
		error("Unknown config name \"%s\"", vp->v_str);
	freevalue(stack);
	getconfig(type, stack);
	return 0;
}


/*
 * Set the 'old' value to the last value saved during the calculation.
 */
void
updateoldvalue(fp)
	FUNC *fp;
{
	if (fp->f_savedvalue.v_type == V_NULL)
		return;
	freevalue(&oldvalue);
	oldvalue = fp->f_savedvalue;
	fp->f_savedvalue.v_type = V_NULL;
	return;
}


/*
 * Routine called on any runtime error, to complain about it (with possible
 * arguments), and then longjump back to the top level command scanner.
 */
#ifdef VARARGS
# define VA_ALIST fmt, va_alist
# define VA_DCL char *fmt; va_dcl
#else
# ifdef __STDC__
#  define VA_ALIST char *fmt, ...
#  define VA_DCL
# else
#  define VA_ALIST fmt
#  define VA_DCL char *fmt;
# endif
#endif
/*VARARGS*/
void
error(VA_ALIST)
	VA_DCL
{
	va_list ap;
	char buf[MAXERROR+1];

	if (funcname && (*funcname != '*'))
		fprintf(stderr, "\"%s\": ", funcname);
	if (funcline && ((funcname && (*funcname != '*')) || !inputisterminal()))
		fprintf(stderr, "line %ld: ", funcline);
#ifdef VARARGS
	va_start(ap);
#else
	va_start(ap, fmt);
#endif
	vsprintf(buf, fmt, ap);
	va_end(ap);
	fprintf(stderr, "%s\n", buf);
	funcname = NULL;
	longjmp(jmpbuf, 1);
	return;
}

/* END CODE */
