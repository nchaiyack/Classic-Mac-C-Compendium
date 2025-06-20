/*
 * File xdiff.h
 *
 * Tim Krauskopf
 *
 * Storage declarations for diff operations
 *
 */

#include "stdio.h"
#include "memory.h"

#define malloc(A) NewPtr(A)
#define free(A) DisposPtr( (Ptr) A)


typedef int int32;
typedef unsigned int uint32;
 
struct lndiff {			/* data about segments in file */
 	int32
		id,				/* unique checksum for line */
		num;			/* segment number */
	char
		*ld;			/* data for line */
};

 struct stdiff {
 	int32
		len,			/* file length */
		lines,			/* number of lines */
		tsum,			/* total checksum of all lines */
		curstart;		/* wrap around point to use */
	int
		fd;				/* file descriptor */
	char
		fn[256],		/* file name */
		*store;			/* malloced space for text storage */
	struct lndiff
		*lstore;		/* malloced array of segment data elements */
		
};

typedef struct stdiff diffile;

#ifdef MASTERDEF
int linealloc=3000;

#else
extern int linealloc;
#endif