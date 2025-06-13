/* ==========================================

	memory.c
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

#include "memory.proto.h"
#include "utility.proto.h"

/* ========================================== */

#define	LOW_MEM_1		80000
#define	LOW_MEM_2		50000
#define	LOW_MEM_3		30000

/* ========================================== */

int		g_low_mem_1;
int		g_low_mem_2;
int		g_low_mem_3;

/* ========================================== */

void mem_init( void )
{
	g_low_mem_1 = 0;
	g_low_mem_2 = 0;
	g_low_mem_3 = 0;
}

/* ========================================== */

void mem_test( void )
{
	long	mem;
	int		low;
	
	mem = FreeMem();
	
	// ***** final warning ******
	
	low = mem < LOW_MEM_3;
	
	if (low) {
		if (!g_low_mem_3) error_note( "\pMemory is getting EXTREMELY low.  Please exit immediately" );
		g_low_mem_3 = 1;
		return;
		}
	else
		g_low_mem_3 = 0;

	// ***** second warning ******
	
	low = mem < LOW_MEM_2;
	
	if (low) {
		if (!g_low_mem_2) error_note( "\pMemory is getting VERY low.  Please close all shells and exit" );
		g_low_mem_2 = 1;
		return;
		}
	else
		g_low_mem_2 = 0;
	
	// ***** first warning ******
	
	low = mem < LOW_MEM_1;
	
	if (low) {
		if (!g_low_mem_1)
			error_note( "\pMemory is getting low.  Please close unused shells" );
		g_low_mem_1 = 1;
		return;
		}
	else
		g_low_mem_1 = 0;
	
}
