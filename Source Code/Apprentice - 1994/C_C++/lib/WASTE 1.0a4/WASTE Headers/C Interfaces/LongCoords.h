/*
 *	LongCoords.h
 *
 *	C interface to the WASTE text engine:
 *	Long Coordinates
 *
 *	Copyright (c) 1993-1994 Merzwaren
 *	All Rights Reserved
 */
 
#ifndef _LongCoords_
#define _LongCoords_

typedef struct LongPoint {
	long v;
	long h;
} LongPoint;

typedef struct LongRect {
	long top;
	long left;
	long bottom;
	long right;
} LongRect;

#endif _LongCoords_
