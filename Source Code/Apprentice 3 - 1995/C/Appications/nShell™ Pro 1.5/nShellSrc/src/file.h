/* ==========================================

	file.h - definitions for shell-instance files
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

/*
	Note: the file structure for shell instances is as follows:
	
	position	data
	========	====
		0		four byte file version
		4		file header record (format determined by file version)
		.		data (as specified by header record)
		.		data (as specified by header record)

*/

// The FILE_VERSION is used to identify a specific version of the shell
// instance file.  An ascii-long is used to make a better signature.

#define FILE_VERSION '0000'

// this structure defines the version '0000' file header

typedef struct {

		long	text_start;		// file position of historical text
		long	text_count;		// size of historical text (in chars)
		long	vars_start;		// file position of first variable record
		long	vars_count;		// number of variable records
		
		} t_file_header;
		
// this structure is used to hold variable strings within the shell file

typedef struct {

		Str32	name;
		Str255	value;
		
		} t_file_variable;
