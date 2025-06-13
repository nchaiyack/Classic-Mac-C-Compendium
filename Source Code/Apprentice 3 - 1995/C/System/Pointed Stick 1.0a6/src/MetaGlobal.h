#ifndef __METAGLOBAL__
#define __METAGLOBAL__

#define NIL NULL

typedef struct {
	short		branch;
	short		flags;
	long		resType;
	short		resNumber;
	short		version;
	long		refCon;
} StdHeader, *StdHeaderPtr, **StdHeaderHdl;

#endif
