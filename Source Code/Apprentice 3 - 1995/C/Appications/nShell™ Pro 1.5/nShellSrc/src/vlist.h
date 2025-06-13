/* ===========================================

	vlist.h

	Copyright (c) 1994 by Newport Software Development
	
   =========================================== */

#define __VLIST__

#define	SHELL_VARIABLE	'svar'
#define	SHELL_PARAMETER	'spar'
#define	POP_BEAD		'bead'

typedef struct {

		long	data_type;
		Handle	next;
		Handle	prev;
		Str32	name;
		Str255	value;
		
} t_vl_rec;

typedef t_vl_rec **t_vl_hndl;

typedef struct {

		long	data_type;
		Handle	next;
		Handle	prev;
		
} t_vbead_rec;

typedef t_vbead_rec **t_vbead_hndl;
