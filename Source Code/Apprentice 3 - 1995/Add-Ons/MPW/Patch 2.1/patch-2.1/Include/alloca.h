/*
	alloca.h -- header file for alloca (stack allocate)
	
	Copyright (C) 1995, Christopher E. Hyde.  All rights reserved.
	
	This program is free software; you can redistribute it and/or
	modifiy it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __ALLOCA__
#define __ALLOCA__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma parameter __A0 __al_start()
extern pascal void* __al_start (void) = 0x204F;		// MOVE.L	A7,A0

#pragma parameter __al_end(__A0)
extern pascal void __al_end (void*) = 0x2E48;		// MOVE.L	A0,A7

#pragma parameter __A0 alloca(__D0)
extern pascal void* alloca (size_t size)
 = { 0x5680, 0x0240, 0xFFFC, 0x9FC0, 0x204F, 0x598F };
/*
	5680			// ADDQ.L	#3,D0
	0240 FFFC		// AND.W	#$FFFC,D0
	9FC0			// SUB.L	D0,A7
	204F			// MOVE.L	A7,A0
	598F			// SUBQ.L	#4,A7
*/

#define	al_start	void* __al = __al_start()
#define	al_end		__al_end(__al)


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ALLOCA__
