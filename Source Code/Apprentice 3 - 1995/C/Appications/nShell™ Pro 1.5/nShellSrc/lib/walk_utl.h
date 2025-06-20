/* ========================================

	walk_utl.proto.h
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ======================================== */

/* ======================================== */

// Constant Definitions

#define	MAX_WALK_LEVELS	50

// Note: Since levels 0 and 1 in the t_walk_data structure
// contain the same thing, this is really 49 folders total.

/* ======================================== */

// Type Definitions

// This structure defines the data for one level of a directory search

typedef struct {

	FSSpec	fss;			// fsspec of this level
	
	short	index;			// the item we are looking at within this level

} t_level_data;

// This structure defines the control data for a whole search

typedef struct {

	t_level_data	levels[MAX_WALK_LEVELS];	// data for each level
	
	short			level;		// the level we are looking at

} t_walk_data;

typedef	t_walk_data	**t_walk_hndl;

