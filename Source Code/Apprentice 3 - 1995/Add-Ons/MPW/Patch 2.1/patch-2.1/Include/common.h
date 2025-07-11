/* $Header: common.h,v 2.0.1.2 88/06/22 20:44:53 lwall Locked $
 *
 * $Log:	common.h,v $
 * Revision 2.0.1.2  88/06/22  20:44:53  lwall
 * patch12: sprintf was declared wrong
 * 
 * Revision 2.0.1.1  88/06/03  15:01:56  lwall
 * patch10: support for shorter extensions.
 * 
 * Revision 2.0  86/09/17  15:36:39  lwall
 * Baseline for netwide release.
 * 
 */

// globals

EXT int Argc;				// guess
EXT char** Argv;
EXT int optind_last;			// for restarting plan_b

EXT struct stat filestat;		// file statistics area
EXT int filemode INIT(0644);

EXT char buf[MAXLINELEN];		// general purpose buffer
EXT FILE *ofp INIT(Nullfp);		// output file pointer
EXT FILE *rejfp INIT(Nullfp);		// reject file pointer

EXT int myuid;				// cache getuid return value

EXT bool using_plan_a INIT(TRUE);	// try to keep everything in memory
EXT bool out_of_mem INIT(FALSE);	// ran out of memory in plan a

#define MAXFILEC 2
EXT int filec INIT(0);			// how many file arguments?
EXT char* filearg[MAXFILEC];
EXT bool ok_to_create_file INIT(FALSE);
EXT char* bestguess INIT(Nullch);	// guess at correct filename

EXT char* outname INIT(Nullch);
EXT char rejname[128];

EXT char* origprae INIT(Nullch);

EXT char* TMPOUTNAME;
EXT char* TMPINNAME;
EXT char* TMPREJNAME;
EXT char* TMPPATNAME;
EXT bool toutkeep INIT(FALSE);
EXT bool trejkeep INIT(FALSE);

EXT LINENUM last_offset INIT(0);
#ifdef DEBUGGING
EXT int debug INIT(0);
#endif
EXT LINENUM maxfuzz INIT(2);
EXT bool force INIT(FALSE);
EXT bool batch INIT(FALSE);
EXT bool verbose INIT(TRUE);
EXT bool reverse INIT(FALSE);
EXT bool noreverse INIT(FALSE);
EXT bool skip_rest_of_patch INIT(FALSE);
EXT int strippath INIT(957);
EXT bool canonicalize INIT(FALSE);

#define CONTEXT_DIFF 1
#define NORMAL_DIFF 2
#define ED_DIFF 3
#define NEW_CONTEXT_DIFF 4
#define UNI_DIFF 5
EXT int diff_type INIT(0);

EXT bool do_defines INIT(FALSE);	// patch using ifdef, ifndef, etc.
EXT char if_defined[128];		// #ifdef xyzzy
EXT char not_defined[128];		// #ifndef xyzzy
EXT char else_defined[] INIT("#else\n");// #else
EXT char end_defined[128];		// #endif xyzzy

EXT char* revision INIT(Nullch);	// prerequisite revision, if any
