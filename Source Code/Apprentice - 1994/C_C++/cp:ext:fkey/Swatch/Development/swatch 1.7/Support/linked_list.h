/**

	linked_list.h
	Copyright (c) 1992, joe holt

 **/


#ifndef __linked_list__
#define __linked_list__


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#ifndef __ctypes__
#include "ctypes.h"
#endif


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

typedef struct _t_header **l_anchor_t;
typedef struct _t_header **l_elem_t;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

l_anchor_t l_new_list(void);
void l_old_list(l_anchor_t anchor);
l_elem_t l_new(l_anchor_t anchor, int32 size);
void l_old(l_elem_t h);
void *l_access(l_elem_t h);
void l_release(l_elem_t h);
l_elem_t l_next( l_anchor_t anchor, l_elem_t this );
l_elem_t l_prev( l_anchor_t anchor, l_elem_t this );

#endif
