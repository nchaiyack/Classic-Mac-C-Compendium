/**

	info.h
	Copyright (c) 1990-1992, joe holt

 **/


#ifndef __info__
#define __info__

#include <Processes.h>

#ifndef __heap_list__
#include "heap_list.h"
#endif


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

typedef struct _t_ExtendedProcessInfoRec {
	char unknown1[4];					// $00-$03
	int32 processType;					// $04
	OSType processSignature;			// $08
	char unknown2[8];					// $0C-$13
	Ptr unknown_link1;					// $14
	Ptr unknown_link2;					// $18
	Ptr unknown_link3;					// $1C
	char unknown3[4];					// $20-$23
	ProcessSerialNumber processNumber;	// $24
	Ptr processLocation;				// $2C
	Ptr pointer1;						// $30
	Ptr unknown_link4;					// $34
	Ptr unknown_link5;					// $38
	int32 processMode;					// $3C
	char unknown4[8];					// $40-$47
	int32 processSize;					// $48
	char unknown5[8];					// $4C-$53
	Ptr processInfoD4;					// $54
	Ptr processSP;						// $58
	char unknown7[66];					// $5C-$9D
	Handle processLowMemory;			// $9E
	char unknown6[182];					// $A2-$157
} ExtendedProcessInfoRec;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

void Info_track_heap( EventRecord *the_event, Heap_info_handle_t h );

#endif
