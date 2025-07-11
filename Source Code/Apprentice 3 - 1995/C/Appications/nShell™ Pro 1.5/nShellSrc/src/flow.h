/* ===========================================

	flow.h

	Copyright (c) 1994 by Newport Software Development
	
   =========================================== */

#define __FLOW__

#define FLOW_POSITIONS 4

typedef struct {
		Handle	next;
		Handle	prev;
		long	id;
		int		type;
		int		result;
		int		not;
		long	position[FLOW_POSITIONS];
} t_fl_rec;

typedef t_fl_rec **t_fl_hndl;
