/**

	swatch.types.h
	Copyright (c) 1990, Adobe Systems, Inc.

 **/


#ifndef _H_SWATCH_TYPES
#define _H_SWATCH_TYPES

#include <ctypes.h>
#include "drvr.globals.h"


/*******************************************************************************
 **
 **	Public Macros
 **
 **/


/*******************************************************************************
 **
 **	Public Constants
 **
 **/

#define MAX_HEAP_TRANSITIONS	200
#define MIN_HEAP_SCALE			8
#define MAX_HEAP_SCALE			4194304

#define HEAP_FREE_RUN			0
#define HEAP_LOCKED_RUN			1
#define HEAP_UNLOCKED_RUN		2
#define HEAP_PURGEABLE_RUN		3
#define HEAP_BASE_USER_RUN		4

#define HEAP_MAX_USER_RUN_TYPES	10


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

typedef struct {
	App_stat_t app;
	Boolean heap_is_32_bit;
	int32 last_size, last_free;
	int32 current_size, current_free;
	Boolean current_heap_ok, last_heap_ok;
	int16 current_heap_transitions, last_heap_transitions;
	int32 current_heap[MAX_HEAP_TRANSITIONS], last_heap[MAX_HEAP_TRANSITIONS];
} Display_stat_t;


#endif  /* ifndef _H_SWATCH_TYPES */
