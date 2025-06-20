#ifndef _libhdr_h
#define _libhdr_h

	

#define LIB_INFO_TUPLEN		5

typedef struct Unitdecl_s
{
	Symbol	ud_unam;  /* unit name */
	int	ud_unit;  /* unit number of unit name     */
	int	ud_useq;  /* sequence number of unit name */
	Tuple	ud_context;
	Tuple	ud_nodes;
	Tuple	ud_symbols;
	Tuple	ud_decscopes;
	Tuple	ud_decmaps;
	Tuple	ud_oldvis;
} Unitdecl_s;

typedef Unitdecl_s *Unitdecl;

typedef struct Stubenv_s
{
	Tuple	ev_scope_st;
	Tuple	ev_open_decls;
	Tuple   ev_nodes;
	Symbol	ev_unit_unam;
	Declaredmap ev_decmap;
	Tuple	ev_context;
	int	ev_current_level;
	Tuple	ev_relay_set;
	Tuple   ev_dangling_relay_set;
} Stubenv_s;

typedef Stubenv_s *Stubenv;

 

#define AIS_INFO_TUPLEN		8

	

#define TRE_INFO_TUPLEN 4

#ifdef __MWERKS__
#pragma options align=mac68k
#endif
typedef struct f_symbol_s
{
	short		f_symbol_nature;		
	short		f_symbol_seq;	
	short		f_symbol_unit;	
	short		f_symbol_type_of_seq;
	short		f_symbol_type_of_unit;
	short		f_symbol_scope_of_seq;
	short		f_symbol_scope_of_unit;
	short		f_symbol_signature;	
	short		f_symbol_overloads;
	short		f_symbol_declared;
	short		f_symbol_alias_seq;
	short		f_symbol_alias_unit;
	short		f_symbol_type_attr;
	short		f_symbol_misc;
	short		f_symbol_type_kind;
	short		f_symbol_type_size;
	short		f_symbol_init_proc_seq;
	short		f_symbol_init_proc_unit;
	short		f_symbol_assoc_list;  /* for _type, etc */
	short		f_symbol_s_segment; /* REFERENCE_MAP segment */
	short	f_symbol_s_offset; /* REFERENCE_MAP offset */
} f_symbol_s;
#ifdef __MWERKS__
#pragma options align=reset
#endif

#endif
