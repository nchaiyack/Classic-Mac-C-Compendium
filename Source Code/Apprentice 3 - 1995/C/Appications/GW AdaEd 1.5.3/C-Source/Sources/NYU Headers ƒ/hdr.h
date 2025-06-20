#ifndef _hdr_h
#define _hdr_h

#define SEQ_NODE_INC 50
#define SEQ_SYMBOL_INC	50

#ifndef _STDLIB
#include <stdlib.h>
#endif

#ifndef _STDIO
#include <stdio.h>
#endif

#ifndef _CTYPE
#include <ctype.h>
#endif

#ifndef _STRING
#include <string.h>
#endif


#ifndef _config_h
#include "config.h"
#endif

#ifndef _arith_h
#include "arith.h"
#endif

#ifndef _set_h
#include "set.h"
#endif

typedef struct Declaredmap_s *Declaredmap;

#ifdef AMIABLE
#else

#ifndef _symbol_h
#include "symbol.h"
#endif

#endif

typedef struct Span_s *Span;   /* pointer to spans information */

typedef struct Span_s {
	short line;
	short col;
} Span_s;

typedef struct Node_s *Node; /* Node is pointer to Node_s */
#define NODE_SIZE  sizeof(Node_s)

typedef struct Node_s
{
#ifndef IBM_PC
	struct {
	unsigned n_kind:8;
	unsigned n_unit:8;
	unsigned n_side:1;
	unsigned n_overloaded:1;
	} n_flags;
	
	short n_seq;
#else
	struct {
	unsigned n_kind:8;
	unsigned n_unit:8;
	} n_flag1;

	struct {
	unsigned n_side:1;
	unsigned n_seq:14;
	unsigned n_overloaded:1;
	} n_flag2;

#endif

	union {
	Node	n_ast1;
	Span_s	n_span;
	} nu1;

 	union {
	Node	n_ast2;
	Tuple	n_list;
	char	*n_val; 
	int n_id;
	} nu2;
 	union { 
	Node	n_ast3;
	Set	n_names;
	Symbol	n_unq; 
	} nu3;
 	union {
	Node	n_ast4;
	Set	n_ptypes;
	Symbol	n_type; 
	} nu4;
#ifdef AMIABLE
	Operand_s n_operand;
#endif
}  Node_s;

struct unit {
	char *name;
	int isMain;
	char *libUnit;
	struct {
		char *fname;
		char *obsolete;
		char *currCodeSeg;
		char *localRefMap;
		char *compDate;
	} libInfo;
	struct {
		char *preComp;
		char *pragmaElab;
		char *compDate;
		char *symbols;
		int  numberSymbols;
		char *unitDecl;
	} aisInfo;
	struct {
		char *tableAllocated;
		int nodeCount;
		int rootSeq;
	} treInfo;
};

#define MAX_UNITS 100

#ifdef IBM_PC
#define N_KIND(p)	((p)->n_flag1.n_kind)
#define N_UNIT(p)	((p)->n_flag1.n_unit)
#define N_OVERLOADED(p)	((p)->n_flag2.n_overloaded)
#define N_SIDE(p)	((p)->n_flag2.n_side)
#define N_SEQ(p)	((p)->n_flag2.n_seq)
#else
#define N_KIND(p)	((p)->n_flags.n_kind)
#define N_UNIT(p)	((p)->n_flags.n_unit)
#define N_OVERLOADED(p)	((p)->n_flags.n_overloaded)
#define N_SIDE(p)	((p)->n_flags.n_side)
#define N_SEQ(p)	((p)->n_seq)
#endif
#define N_SPAN(p)	(&(p)->nu1.n_span)
#define N_SPAN0(p)	((p)->nu1.n_span.line)
#define N_SPAN1(p)	((p)->nu1.n_span.col)
#define N_AST1(p)	((p)->nu1.n_ast1)
#define N_VAL(p)	((p)->nu2.n_val)
#define N_LIST(p)	((p)->nu2.n_list)
#define N_AST2(p)	((p)->nu2.n_ast2)
#define N_AST3(p)	((p)->nu3.n_ast3)
#define N_UNQ(p)	((p)->nu3.n_unq)
#define N_ID(p)	 	((p)->nu2.n_id)
#define N_NAMES(p)	((p)->nu3.n_names)
#define N_AST4(p)	((p)->nu4.n_ast4)
#define N_TYPE(p)	((p)->nu4.n_type)
#define N_PTYPES(p)	((p)->nu4.n_ptypes)
#ifdef AMIABLE
#define N_OPERAND(p)    ((p)->n_operand)
#endif

#define N_D_AST1 1
#define N_D_AST2 2
#define N_D_AST3 4
#define N_D_AST4 8
#define N_D_LIST 16
#define N_D_VAL 32
#define N_D_UNQ 64
#define N_D_TYPE 256

#define N_AST1_DEFINED(p) (N_DEFINED[p]&N_D_AST1)
#define N_AST2_DEFINED(p) (N_DEFINED[p]&N_D_AST2)
#define N_AST3_DEFINED(p) (N_DEFINED[p]&N_D_AST3)
#define N_AST4_DEFINED(p) (N_DEFINED[p]&N_D_AST4)
#define N_VAL_DEFINED(p) (N_DEFINED[p]&N_D_VAL)
#define N_UNQ_DEFINED(p) (N_DEFINED[p]&N_D_UNQ)
#define N_TYPE_DEFINED(p) (N_DEFINED[p]&N_D_TYPE)
#define N_LIST_DEFINED(p) (N_DEFINED[p]&N_D_LIST)

#define as_pragma  0
#define as_arg	1
#define as_obj_decl  2
#define as_const_decl  3
#define as_num_decl  4
#define as_type_decl  5
#define as_subtype_decl	 6
#define as_subtype_indic  7
#define as_derived_type	 8
#define as_range  9
#define as_range_attribute  10
#define as_constraint  11
#define as_enum	 12
#define as_int_type  13
#define as_float_type  14
#define as_fixed_type  15
#define as_digits  16
#define as_delta  17
#define as_array_type  18
#define as_box	19
#define as_subtype  20
#define as_record  21
#define as_component_list  22
#define as_field  23
#define as_discr_spec  24
#define as_variant_decl	 25
#define as_variant_choices  26
#define as_string  27
#define as_simple_choice  28
#define as_range_choice	 29
#define as_choice_unresolved  30
#define as_others_choice  31
#define as_access_type	32
#define as_incomplete_decl  33
#define as_declarations	 34
#define as_labels  35
#define as_character_literal  36
#define as_simple_name	37
#define as_call_unresolved  38
#define as_selector  39
#define as_all	40
#define as_attribute  41
#define as_aggregate  42
#define as_parenthesis	43
#define as_choice_list	44
#define as_op  45
#define as_in  46
#define as_notin  47
#define as_un_op  48
#define as_int_literal	49
#define as_real_literal	 50
#define as_string_literal  51
#define as_null	 52
#define as_name	 53
#define as_qualify  54
#define as_new_init  55
#define as_new	56
#define as_statements  57
#define as_statement  58
#define as_null_s  59
#define as_assignment  60
#define as_if  61
#define as_cond_statements  62
#define as_condition  63
#define as_case	 64
#define as_case_statements  65
#define as_loop	 66
#define as_while  67
#define as_for	68
#define as_forrev  69
#define as_block  70
#define as_exit	 71
#define as_return  72
#define as_goto	 73
#define as_subprogram_decl  74
#define as_procedure  75
#define as_function  76
#define as_operator  77
#define as_formal  78
#define as_mode	 79
#define as_subprogram  80
#define as_call	 81
#define as_package_spec	 82
#define as_package_body	 83
#define as_private_decl	 84
#define as_use	85
#define as_rename_obj  86
#define as_rename_ex  87
#define as_rename_pack	88
#define as_rename_sub  89
#define as_task_spec  90
#define as_task_type_spec  91
#define as_task	 92
#define as_entry  93
#define as_entry_family	 94
#define as_accept  95
#define as_delay  96
#define as_selective_wait  97
#define as_guard  98
#define as_accept_alt  99
#define as_delay_alt  100
#define as_terminate_alt  101
#define as_conditional_entry_call  102
#define as_timed_entry_call  103
#define as_abort  104
#define as_unit	 105
#define as_with_use_list  106
#define as_with	 107
#define as_subprogram_stub  108
#define as_package_stub	 109
#define as_task_stub  110
#define as_separate  111
#define as_exception  112
#define as_except_decl	113
#define as_handler  114
#define as_others  115
#define as_raise  116
#define as_generic_function  117
#define as_generic_procedure  118
#define as_generic_package  119
#define as_generic_formals  120
#define as_generic_obj	121
#define as_generic_type	 122
#define as_gen_priv_type  123
#define as_generic_subp	 124
#define as_generic  125
#define as_package_instance  126
#define as_function_instance  127
#define as_procedure_instance  128
#define as_instance  129
#define as_length_clause  130
#define as_enum_rep_clause  131
#define as_rec_rep_clause  132
#define as_compon_clause  133
#define as_address_clause  134
#define as_any_op  135
#define as_opt	136
#define as_list	 137
#define as_range_expression  138
#define as_arg_assoc_list  139
#define as_private  140
#define as_limited_private  141
#define as_code	 142
#define as_line_no  143
#define as_index  144
#define as_slice  145
#define as_number  146
#define as_convert  147
#define as_entry_name  148
#define as_array_aggregate  149
#define as_record_aggregate  150
#define as_ecall  151
#define as_call_or_index  152
#define as_ivalue  153
#define as_qual_range  154
#define as_qual_index  155
#define as_qual_discr  156
#define as_qual_arange	157
#define as_qual_alength	 158
#define as_qual_adiscr	159
#define as_qual_aindex	160
#define as_check_bounds	 161
#define as_discr_ref  162
#define as_row	163
#define as_current_task	 164
#define as_check_discr	165
#define as_end	166
#define as_terminate 167
#define as_exception_accept 168
#define as_test_exception 169
#define as_create_task 170
#define as_predef	171
#define as_deleted 172
#define as_insert 173
#define as_arg_convert 174
#define as_end_activation 175
#define as_activate_spec 176
#define as_delayed_type 177
#define as_qual_sub 178
#define as_static_comp 179
#define as_array_ivalue 180
#define as_record_ivalue 181
#define as_expanded 182
#define as_choices 183
#define as_init_call 184
#define as_type_and_value 185
#define as_discard 186
#define as_unread 187
#define as_string_ivalue 188
#define as_instance_tuple 189
#define as_entry_family_name 190
#define as_astend	191
#define as_astnull	192
#define as_aggregate_list 193
#define as_interfaced 194
#define as_record_choice 195
#define as_subprogram_decl_tr 196
#define as_subprogram_tr 197
#define as_subprogram_stub_tr 198
#define as_rename_sub_tr 199

#define na_op  1
#define na_un_op  2
#define na_attribute  3
#define na_obj	4
#define na_constant  5
#define na_type	 6
#define na_subtype  7
#define na_array  8
#define na_record  9
#define na_enum	 10
#define na_literal  11
#define na_access  12
#define na_aggregate  13
#define na_block  14
#define na_procedure_spec  15
#define na_function_spec  16
#define na_procedure  17
#define na_function  18
#define na_in  19
#define na_inout  20
#define na_out	21
#define na_package_spec	 22
#define na_package  23
#define na_task_type  24
#define na_task_type_spec  25
#define na_task_obj  26
#define na_task_obj_spec  27
#define na_entry  28
#define na_entry_family	 29
#define na_entry_former	 30
#define na_generic_procedure_spec  31
#define na_generic_function_spec  32
#define na_generic_package_spec	 33
#define na_generic_procedure  34
#define na_generic_function  35
#define na_generic_package  36
#define na_exception  37
#define na_private_part	 38
#define na_void	 39
#define na_null	 40
#define na_discriminant	 41
#define na_field  42
#define na_label  43
#define na_generic_part	 44
#define na_subprog 45
#define na_body 46
#define na_task 47
#define na_task_body 48

#define BLOCK_BLOCK	0
#define BLOCK_LOOP	1
#define BLOCK_HANDLER	2

typedef struct Private_declarations_s 
{
	Tuple	private_declarations_tuple;	
} Private_declarations_s;
typedef Private_declarations_s	*Private_declarations;

typedef struct	Forprivate_decls {
	Tuple	forprivate_tup;
	int	forprivate_i;
	int	forprivate_n;
} Forprivate_decls;

// FOR optimized [Fabrizio Oddone]

#define FORPRIVATE_DECLS(s1,s2,pd,fp) {\
	Tuple	forprivate_tup = (Tuple) (pd)->private_declarations_tuple;\
	Tuple	limit_tup = forprivate_tup + tup_size(forprivate_tup);\
	forprivate_tup++;\
	while (forprivate_tup <= limit_tup) {\
		s1 = (Symbol) *forprivate_tup++;\
		s2 = (Symbol) *forprivate_tup++;

#define ENDFORPRIVATE_DECLS(fp) }}

#define TA_ISPRIVATE  1
#define TA_INCOMPLETE 2
#define TA_LIMITED	4
#define TA_LIMITED_PRIVATE	8
#define TA_PRIVATE		16
#define TA_OUT			32
#define TA_GENERIC		64
#define TA_CONSTRAIN		128

#define CONSTRAINT_RANGE 0
#define CONSTRAINT_DIGITS 1
#define CONSTRAINT_DELTA 2
#define CONSTRAINT_DISCR 3
#define CONSTRAINT_ARRAY 4
#define CONSTRAINT_ACCESS 6

#define numeric_constraint_kind(p) p[1]
#define numeric_constraint_low(p) p[2]
#define numeric_constraint_high(p) p[3]
#define numeric_constraint_digits(p) p[4]
#define numeric_constraint_delta(p) p[4]
#define numeric_constraint_small(p) p[5]
#define numeric_constraint_discr(p) p[2]

typedef struct Declaredmap_s
{
	short		dmap_curlen;	/* current number of elements */
	short		dmap_maxlen;	/* maximum number of elements */
	struct Dment	*dmap_table;	/* pointer to entry list */
 } Declaredmap_s;

typedef struct Dment
{
	Symbol		dment_symbol;	    /* symbol table pointer */
	struct {
#ifdef IBM_PC
	unsigned	dment_idnum ;   /* source identifier number */
	unsigned	dment_visible ;  /* non-zero if visible */
#else
	unsigned	dment_visible : 1;  /* non-zero if visible */
	unsigned	dment_idnum : 15;   /* source identifier number */
#endif
	} dment_i;
}  Dment;

typedef struct	Fordeclared {
	Declaredmap fordeclared_map;
	unsigned short	fordeclared_i;
	unsigned short	fordeclared_n;
	struct Dment *fordeclared_dment;
} Fordeclared;

extern char *dstrings;

#define FORDECLARED(str,sym,dmap,iv) {\
	Declaredmap fordeclared_map = dmap;\
	struct Dment *fordeclared_dment = fordeclared_map->dmap_table;\
	struct Dment *limit_dment = fordeclared_dment + fordeclared_map->dmap_curlen;\
	for ( ; fordeclared_dment < limit_dment; fordeclared_dment++) {\
		sym = fordeclared_dment->dment_symbol;\
		str = dstrings + fordeclared_dment->dment_i.dment_idnum;

#define ENDFORDECLARED(iv) }}
#define IS_VISIBLE(iv)	fordeclared_dment->dment_i.dment_visible 
#define SETDECLAREDVISIBLE(iv,n)	IS_VISIBLE(iv) = n;	
#define FORVISIBLE(str,sym,dmap,iv)\
	FORDECLARED(str,sym,dmap,iv)\
		if (IS_VISIBLE(iv)==0) continue;

#define ENDFORVISIBLE(iv) }}

typedef struct Const_s	*Const;
typedef struct Const_s {
	int	const_kind;
//#if (defined(powerc) || defined(__powerc)) && INT_MAX <= 2147483647
//	int	padding;	// we just want to make sure that the double is well aligned [Fabrizio Oddone]
//#endif
	union	{
		int	const_int;
		int	*const_uint;
		double	const_real;
		char	*const_str;
		Rational	const_rat;
		long	const_fixed;
		} const_value;
	} Const_s;

#ifdef IVALUE
typedef struct Ivalue_s *Ivalue;
typedef struct Ivalue_s {
	int	ivalue_kind;
//#if (defined(powerc) || defined(__powerc)) && INT_MAX <= 2147483647
//	int	padding;	// we just want to make sure that the double is well aligned [Fabrizio Oddone]
//#endif
	union	{
		int	ivalue_int;
		int	*ivalue_uint;
		double	ivalue_real;
		char	*ivalue_str;
		Rational	ivalue_rat;
		long	ivalue_fixed;
		} ivalue_value;
	} Ivalue_s;
#endif
#define CONST_OM	0
#define CONST_INT	1
#define CONST_REAL	2
#define CONST_STR	3
#define CONST_RAT	4
#define CONST_CONSTRAINT_ERROR 5
#define CONST_UINT	6
#define CONST_FIXED	7

#define is_const_om(c) ((c)->const_kind == CONST_OM)
#define is_const_int(c) ((c)->const_kind == CONST_INT)
#define is_const_real(c) ((c)->const_kind == CONST_REAL)
#define is_const_str(c) ((c)->const_kind == CONST_STR)
#define is_const_rat(c) ((c)->const_kind == CONST_RAT)
#define is_const_constraint_error(c) ((c)->const_kind == CONST_CONSTRAINT_ERROR)
#define is_const_uint(c) ((c)->const_kind == CONST_UINT)
#define is_const_fixed(c) ((c)->const_kind == CONST_FIXED)

#ifdef IVALUE
#define IVALUE_OM	0
#define IVALUE_INT	1
#define IVALUE_REAL	2
#define IVALUE_STR	3
#define IVALUE_RAT	4
#define IVALUE_CONSTRAINT_ERROR 5
#define IVALUE_UINT	6
#define IVALUE_FIXED	7
#endif

#define INTV(op) (op)->const_value.const_int
#define UINTV(op) (op)->const_value.const_uint
#define REALV(op) (op)->const_value.const_real
#define RATV(op) (op)->const_value.const_rat
#define FIXEDV(op) (op)->const_value.const_fixed

	

typedef struct Symbolmap_s 
{
	Tuple	symbolmap_tuple;	
} Symbolmap_s;
typedef Symbolmap_s	*Symbolmap;

typedef struct	Forsymbol {
	Tuple	forsymbolmap_tuple;
	int	forsymbolmap_i;
	int	forsymbolmap_n;
} Forsymbol;

// FOR optimized [Fabrizio Oddone]

#define FORSYMBOL(s1,s2,pd,fp) {\
	Tuple	forsymbolmap_tuple = (Tuple) (pd)->symbolmap_tuple;\
	Tuple	limit_tup = forsymbolmap_tuple + tup_size(forsymbolmap_tuple);\
	forsymbolmap_tuple++;\
	while (forsymbolmap_tuple <= limit_tup) {\
		s1 = (Symbol) *forsymbolmap_tuple++;\
		s2 = (Symbol) *forsymbolmap_tuple++;

#define ENDFORSYMBOL(fp) }}

typedef struct Nodemap_s 
{
	Tuple	nodemap_tuple;	
} Nodemap_s;
typedef Nodemap_s	*Nodemap;

#define find ???

#define is_empty(node) ((int)node[0] == 0)

#define attribute_name(node) N_VAL(N_AST1(node))

#define attribute_kind(node) N_VAL(N_AST1((node)))

#define root_type(type_mark) ((type_mark)->alias)

#define index_types(array_type) ((Tuple) ((array_type)->signature)[1])

#define component_type(array_type) ((Symbol) ((array_type)->signature)[2])

#define index_type(array_type)	((Tuple) (index_types(array_type))[1])

#define literal_map(enumeration) ((enumeration)->overloads)

#define record_declarations(record) ((record)->signature)

#ifdef TBSN
-- all_components should be dead	ds 14 aug
#define all_components(record) (((record)->signature)[1])
#endif

#define discriminant_list(record) ((Tuple) ((record)->signature)[3])

#define invariant_part(record) ((Tuple) ((record)->signature)[1])

#define variant_part(record)	((Tuple) ((record)->signature)[2])

#define declared_components(record) ((Tuple) ((record)->signature)[4])

#define discr_decl_tree(record) ((Tuple) ((record)->signature)[5])

#define has_discriminants(record) \
	(discriminant_list(root_type(record)) != (Tuple) 0  &&   \
	 tup_size(discriminant_list(root_type(record))) != 0)

#define designated_type(access_type) 					 \
	(((NATURE(access_type)==na_subtype) 				 \
		? (Symbol)((access_type)->signature)[2] 		 \
		: (Symbol)((access_type)->signature) ))

#define label_status(lab) ((lab)->signature)

#define default_expr(nam) ((nam)->signature)

#define formal_decl_tree(proc_name)  ((proc_name)->init_proc)

#define IS_COMP_UNIT (tup_size(scope_st)==0)

#define is_literal(name) (NATURE((name)) == na_literal)

#define is_constant(name) ((name)!=(Symbol) 0 && NATURE((name))== na_constant)

#define is_anonymous(typ)  (*(ORIG_NAME(typ)) == '&')

#define is_array(typ) (NATURE(base_type(typ)) == na_array)

#define is_formal(id) (NATURE(id)==na_in || NATURE(id)==na_out	 \
			|| NATURE(id)==na_inout)

#define misc_type_attributes(type_mark) TYPE_ATTR(type_mark)

#define private_dependents(type_mark)	OVERLOADS(type_mark)

#define private_decls(package) OVERLOADS(package) 

#ifdef TBSN
#define use_declarations(package) \
	SIGNATURE(dcl_get(DECLARED(package),"$used"))
#endif

#define TO_XREF(name)

#define TO_ERRFILE(text) to_errfile(text)

#define		errmsg_l(msg1,msg2,lrm,node)		\
			errmsg(strjoin(msg1,msg2),lrm,node)
#define		errmsg_l_id(msg1,msg2,name,lrm,node)	\
			errmsg_id(strjoin(msg1,msg2),name,lrm,node)
#define		errmsg_l_str(msg1,msg2,str,lrm,node)		\
			errmsg_str(strjoin(msg1,msg2),str,lrm,node)
extern int N_DEFINED[];

#define POWER_OF_2_EXACT 0
#define POWER_OF_2_APPROXIMATE 1

#ifdef GEN
#include "ghdr.h"
#endif

#define TAG_RECORD              0
#define TAG_TASK                1
#define TAG_ACCESS              2
#define TAG_ARRAY               3
#define TAG_ENUM                4
#define TAG_FIXED               5
#define TAG_INT                 6
#define TAG_FLOAT               7

#endif
