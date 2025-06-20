#define INIT(v) =v 
#define GEN

#include <stdio.h>

#include "hdr.h"

  int	list_code INIT(0); /* set if GEN option selected */

#ifdef TBSN
PREDEF_UNITS; /* predefined units */
#endif

  int	debug_flag;
  int	debug_line INIT(0);

  char *MAINunit; /* name of main unit (from MAIN opo */
  char *interface_files;


  int	bind_option; /* bind option */
  int   bind_only_option INIT(0); /* set when binding only, no generation */
  int	line_option INIT(0); /* LINE option use -L to set */
  int	gen_option INIT(1); /* GEN option: set if want to generate code */


  int ada_line; /* line number for error file (mixed_case) */
  int save_ada_line; /* used to save value of ada_line  */


#ifdef TBSN
STIME; /* ?? time */
BTIME; /* binding time */
#endif

  int 	NB_INSTRUCTIONS; /* number of instructions generated */
  int 	NB_STATEMENTS; /* number of statements processed?? */

  int 	ERROR_IN_UNIT; /* boolean set if errors in unit */
  int 	STUBS_IN_UNIT; /* boolean set if stubs in unit */

  int 	TARGET; 


  Node	FIRST_NODE;		 /* first node in a compilation unit */

  Tuple 	UNIT_FIRST_NODE;	 /* map { unit -> FIRST_NODE } */


  Tuple    RENAME_MAP;          

#ifdef TBSN
-- this is not referenced		ds 22-feb-85
 	Node ROOT_NODE;           /* root node of unit to be expanded */
#endif



  Tuple    CODE_SLOTS;          /* map showing code_slots occupation */

  Tuple	DATA_SLOTS;          /* map showing data_slots occupation */

  Tuple	EXCEPTION_SLOTS;     /* map showing exception_slots occupation */
  Tuple    CODE_SEGMENT_MAP;    /* map { number -> [actual code] } */
  Tuple    DATA_SEGMENT_MAP;    /* map { number -> [actual data] } */


  int    CURRENT_DATA_SEGMENT;   /* number of current data segment */
  int    CURRENT_CODE_SEGMENT;   /* number of current code  segment */

  Tuple    GENERATED_OBJECTS;



#ifdef TBSN
    MISC,           /* map used to store miscellanous infos like: */

#endif

  Tuple LOCAL_REFERENCE_MAP;

  int REFERENCE_SEGMENT;
  int REFERENCE_OFFSET;

#ifdef TBSN
Tuple	CONSTANT_MAP;
#endif


  int    LAST_OFFSET;    /* first available offset in current stack frame */
  int    MAX_OFFSET;     /* largest offset in current stack frame */
  int    TASKS_DECLARED; /* flag indicating possible presence of tasks in frame */
  int	SPECS_DECLARED; /* count of # of specs requiring a */
  Tuple  SUBPROG_SPECS; /* set of subprograms having an explicit spec in the */

  Tuple	SOURCE;         /* the current list of statements to be processed */


  Tuple    EMAP;           /* Various temporary storage: */
  Tuple	EMAP_VALUE; /* value of emap if defined, set by emap_get */

  int	CURRENT_LEVEL;  /* used for static depth of blocks */

  Tuple    PARAMETER_SET; /* Tuple of symbols for formal parameters */

  Tuple    RELAY_SET;

  Tuple    DANGLING_RELAY_SETS;

  Tuple    SUBPROG_PATCH;
	
  Tuple   CODE_PATCH_SET;
  Tuple    DATA_PATCH_SET;


  int	CURRENT_FIELD_NUMBER;
  int	CURRENT_FIELD_OFFSET;
  int	STATIC_REC; /* boolean */
  Tuple   INTERNAL_ACCESSED_TYPES; /* of symbols */

  Tuple  axqfiles_read;	    /* set of already read AXQfiles */
  Tuple  call_lib_unit;	    /* Accumulates code for idle_task to call library */
  Tuple  PRECEDES_MAP;      /* Map representing relationship between units */
  Tuple  DELAYED_MAP;
  Tuple  compilation_table; /* Table of compilation units giving the order of */
  Tuple  late_instances;    /* Map from unit unique name to a set of late */
  Tuple interfaced_procedures;
  int	 interface_counter INIT(256);
  int    interface_flag;    /* equals 1 if there are interfaced procedures, 
                                      0 otherwise */


  int deleted_instructions;
#ifdef TBSN
    optimizable_codes,
#endif



     Tuple	just_read;
  Node	unit_node;

  Symbol symbol_constrained_type;
  Symbol symbol_accept_return INIT((Symbol)0); /* see gen_accept */
  Symbol symbol_used,symbol_unused;
  Symbol symbol_main_task;
  Symbol symbol_type_mark;
  Symbol symbol_task_block;
  Symbol symbol_mulfix; /* expr.c ...*/

  long RAT_TOF_1,RAT_TOF_2;

  Explicit_ref explicit_ref_0; /* for explicit reference of [0,0] */
  Tuple unit_slots  INIT((Tuple) 0);
  int *ivalue_1,*ivalue_10; /* long integer forms of 1 and 10 */
  Const int_const_0; /* Const for integer 0 */
  Rational rat_value_10; /* 10 as rational */
  Tuple global_reference_tuple INIT((Tuple)0);

  Const int_const_null_task; /* for NULL_TASK */
#ifdef BINDER_GEN
  int binder_phase  INIT(0); /* set non-zero if binder phase */
#endif
