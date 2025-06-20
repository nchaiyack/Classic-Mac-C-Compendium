//#define INIT(v) =v 

#include <stdlib.h>

#include "config.h"
#include "int.h"


int *stack_segments [MAX_TASKS]; /* one stack segment for each task */
int task_stackptr [MAX_TASKS];   /* stack pointers for task stacks */
unsigned int data_segments_dim;  /* dimension of data_segments */
int **data_segments;             /* one data segment for each cunit */
int *data_seglen;                /* length of data segments */
unsigned int code_segments_dim;  /* dimension of code_segments */
char **code_segments;            /* segments containing code */
int *code_seglen;                /* length of code segments */
int *cur_stack;                  /* current stack segment */
int cur_stackptr;                /* stack ptr for cur_stack */
char *cur_code;                  /* current code segment */
int num_cunits;                  /* number of compilation units */


int cs;       /* index of current code_segment in code_segments */
int ip;       /* instruction pointer in cur_code */
int sp;       /* stack pointer */
int sfp;      /* stack frame pointer */
int bfp;      /* block frame pointer */
int exr;      /* exception register */
int lin;      /* line number for tracing */


int tp;                /* index of current task in stack_segments */
int *heap_next;        /* top of heap for allocation */
int heap_base;         /* segment number of heap */
int *heap_addr;        /* address of beginning of heap */
long next_clock;        /* next time to call clock_interrupts */
long time_offset;       /* cumulative skipped time for simulated time */
int next_clock_flag;   /* request to check the clock (task waiting) */
int simul_time_flag;   /* run with simulated time (versus real time) */
int time_slicing_flag; /* time slicing (vs schedule at system call) */
long time_slice;        /* length of time slice (500 ms default) */
int tasking_trace;     /* TRUE to trace tasking */
int rendezvous_trace;  /* TRUE to trace rendezvous */
int debug_trace;       /* TRUE for general debugging trace */
int exception_trace;   /* TRUE to trace exceptions */
int call_trace;        /* TRUE to trace calls */
int line_trace;        /* TRUE to trace line numbers */
int instruction_trace; /* TRUE to trace instructions */
int context_sw_trace;  /* TRUE to trace context switches */
int rr_flag;           /* TRUE to allow round-robin scheduling */
int rr_counter;        /* counter of statements - round_robin scheme*/
int rr_nb_max_stmts;   /* max value of rr_counter. */


unsigned int data_slots_dim;      /* dimension of data_slots */
char **data_slots;
unsigned int code_slots_dim;      /* dimension of code_slots */
char **code_slots;
#ifdef MONITOR
char **code_slots_file;
#endif
unsigned int exception_slots_dim; /* dimension of exception_slots */
char **exception_slots;


int max_mem;                      /* size of a heap segment */
int w;                            /* used by GET_WORD */
struct tt_fx_range *temp_template;/* fixed point work template */

int bse,bas1,bas2,bas3,bas4;      /* address base values */
int off,off1,off2,off3,off4;      /* address offset values */
int *ptr,*ptr1,*ptr2,*ptr3,*ptr4; /* memory addresses */
int value,val1,val2;              /* operands for int operations */
int length1,length2;              /* for array operations */
long lvalue,lval1,lval2;          /* operands for long operations */
long fval1,fval2,fvalue;          /* operands for fixed operations */
float rvalue,rval1,rval2;         /* operands for float operations */
int val_high,val_low;             /* low/high of INT range */
int result,delta;                 /* used in fixed point */
int size;                         /* size of object */
int attribute;                    /* attribute code */
int slot;                         /* for create structure */
int component_size;               /* size of array component */
long now_time;                     /* current time */
int to_type;                      /* type from template */
int discr_list[MAX_DISCR];        /* accumulate list of discriminants */
int nb,i;                         /* loop counters etc */
int overflow;                     /* overflow flag from arith routines */
int nb_discr;                     /* number of discriminants */
int nb_dim;                       /* dimension of array */
int ratio,power;                  /* used in attribute processing */
int d,n;                          /* temporaries */
int old_cs;                       /* old value of cs */
int jump;                         /* jump address */
int lim;                          /* limit in for loop */
struct bf *bfptr;                 /* pointer to block frame */
int *tfptr1,*tfptr2;              /* pointers to task frames */
int cur_col;                      /* temporary for predef */

int break_point;       /* next break point */  

int fix_val1[20];      /* fixed point intermediate values */
int fix_val2[20];     
int fix_resu[20];     
int fix_temp[20];     
int mul_fact[20];     
int div_fact[20];     

int arith_overflow;
int sgn;
int last_task;
int *free_list = NULL;
int raise_cs;
int raise_lin;
char *raise_reason;




#define MAXFILES 20
struct afcb *(afcbs[MAXFILES]);


int operation;         /* operation for predef. set in the main loop */
int data_exception;    /* exception to raise on data error */
int filenum;           /* number of current file */
int standard_in_file;  /* index of standard input file */
int standard_out_file; /* index of standard output file */
int current_in_file;   /* index of current default input file */
int current_out_file;  /* index of current default output file */
int current_in_file_saved;   
int current_out_file_saved;  
int file_offset;       /* Offset due to file argument in the stack */
struct tfile *tfiles;  /* linked list of temporary files */


char work_string[4096];


int main_task_size;
int new_task_size;

