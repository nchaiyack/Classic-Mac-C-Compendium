/* structs.h */

#ifndef __dil_structs__

#define __dil_structs__

typedef struct dil_rec {
	PicHandle	pict_handle;
	short		pict_id;
	Str63		fname;
	Boolean		marked;
	struct dil_rec *next;
} dil_rec;

#endif