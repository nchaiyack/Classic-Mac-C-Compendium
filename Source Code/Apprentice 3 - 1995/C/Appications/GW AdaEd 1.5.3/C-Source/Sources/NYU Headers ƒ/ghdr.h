
#define TK_BYTE 1
#define TK_WORD 2
#define TK_ADDR 3
#define TK_LONG 4
#define TK_DBLE 5
#define TK_XLNG 6

#define MAIN_CS 2	/* code segment number for main code segment */
 

#define SLOTS_DATA 1
#define SLOTS_CODE 2
#define SLOTS_EXCEPTION 3
#define SLOTS_DATA_BORROWED 4
#define SLOTS_CODE_BORROWED 5

#define SELECT_CODE 0
#define SELECT_DATA 1
#define SELECT_EXCEPTIONS 2

#define D_NONE	0
#define D_ALL	1
#define D_INT	2
#define	D_FIX	3
#define D_FLOAT 4
#define D_PSEUDO 5

#define A_NONE	0
#define A_BOTH	1
#define	A_LOCAL	2
#define A_GLOBAL 3
#define A_CODE	4
#define	A_PREDEF 5
#define A_EXCEPTION 6
#define A_IMM	7
#define A_ATTR	8
#define A_PSEUDO 9

typedef struct Explicit_ref_s {
	short	explicit_ref_seg;
	short	explicit_ref_off;
} Explicit_ref_s;
typedef struct Explicit_ref_s *Explicit_ref;

#define TASK_INIT_PROC	1
#define PROC_TEMPLATE	1
#define RETURN_TEMPLATE	2
#define FORMAL_TEMPLATE	1
#define ACTUAL_TEMPLATE	2
#define INIT_SPEC	1
#define INIT_BODY	2
#define INIT_TASKS	3

#ifdef EXPORT
#define compiler_error(r) 	  exit_internal_error()
#define compiler_error_k(r,n) 	  exit_internal_error()
#define compiler_error_c(r,c)	  exit_internal_error()
#define compiler_error_s(r,s)	  exit_internal_error()
#endif

#define COMPONENT_TYPE(type_name) component_type(type_name)

#define DEFAULT_EXPR(obj_name) default_expr(obj_name)

#define DESIGNATED_TYPE(acc_typ) designated_type(acc_typ)

#define FIELD_NUMBER(x)           MISC(x)

#define FIELD_OFFSET(x)      S_OFFSET(x)

#define INDEX_TYPES(type_name) index_types(type_name)

#define INVARIANT_PART(record) invariant_part(record)

#ifdef TBSN
macro NEXT_NODE;                 (NODE_COUNT += 1)                endm;
#endif

#define ROOT_TYPE(typ) root_type(typ)

#define size_of(typ) TYPE_SIZE(typ)

#define USER_WARNING(s1,s2) user_warning(s1,s2)

#define USER_INFO(line) user_info(line)

#define TO_ERR(line,filename) to_err(line,filename)

#define IS_ANCESTOR(na) is_ancestor()

#define IS_GENERIC(na) is_generic_gen(na)

#define IS_ENUMERATION_TYPE(typ) (nature_root_type(typ) == na_enum)

#define CONTAINS_TASK(typ)     MISC(typ)

#define F_TO_I(x) x
#define I_TO_F(x) x

#define rat_lt(a,b) rat_lss(a,b)

#ifdef TBSN
macro AIS_INFO(name);
   [COMP_DATE(name) ,PRE_COMP(name), UNIT_DECL(name), CODE_UNITS(name)]
endm;
#endif

#define   SETL                           1
#define   VAXC                           2
#define   IBMPC                          3

#define   IBMPC_MAX_INTEGER              +32767
#define   IBMPC_MIN_INTEGER              -32768
#define   IBMPC_MAX_SHORT                +127
#define   IBMPC_MIN_SHORT                -128
#ifdef TBSN
-- these are defined by SEM
#define   ADA_MIN_REAL                   -1.0E30
#define   ADA_MAX_REAL                   +1.0E30
#define   ADA_REAL_DIGITS                 6
#endif

#define co_range 0
#define co_digits 1
#define co_delta 2
#define co_discr 3
#define co_array 4
#define co_index 5
#define co_access 6

#define   mu_byte			 1
#define   mu_word			 2
#define   mu_addr			 3
#define   mu_long			 4
#define   mu_dble			 5
#define   mu_xlng			 6
#define   mu_fixed1			mu_word  /* check this */
#define   mu_fixed2			mu_dble	 /* review this too */

#define  WORD_SIZE  32	

#define  SFP_SIZE   4

#define  max_width  80

#define LABEL_SIZE 4
#define LABEL_STATIC_DEPTH 1
#define LABEL_POSITION 2
#define LABEL_PATCHES 3
#define LABEL_EQUAL 4

typedef struct Gref_s {
	Symbol gref_sym; /* symbol */
	int    gref_off; /* offset */
	short  gref_seg; /* segment */
    } Gref_s;
typedef struct Gref_s *Gref;
