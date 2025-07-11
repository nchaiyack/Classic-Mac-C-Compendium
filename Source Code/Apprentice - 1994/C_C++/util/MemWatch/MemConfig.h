/*
 * configuration file for building memwatch into a new project
 */

/*
 * commands (from the debug menu) between MIN_CMD and MAX_CMD
 * are used by MemWatch.  If you change the range then change the
 * memwatch debug menu also.
 */
#define MEM_WATCH_MIN_CMD 123000
#define MEM_WATCH_MAX_CMD 123010
#define MEM_WATCH_MENU_debug (32)

#define MEM_DUMP_ALL_CMD	(MEM_WATCH_MIN_CMD)
#define MEM_EPOCH_CMD		(MEM_WATCH_MIN_CMD+1)
#define MEM_DUMP_SINCE_CMD	(MEM_WATCH_MIN_CMD+2)

/*
 * MemWatch will only install its patches if the disk the application
 * is run from matches MEM_WATCH_DISK_NAME_TO_ENABLE_DEBUG
 * If you have a method to enable debugging you like better, then edit
 * The function is_development_system() in MemTCL.c
 */
#define MEM_WATCH_DISK_NAME_TO_ENABLE_DEBUG "boot"
#define MEM_WATCH_FILE_NAME MEM_WATCH_DISK_NAME_TO_ENABLE_DEBUG ":mem.dump"

/*
 * MemWatch maintains a backtrace buffer of recently entered routines
 * for debugging.  This is done by turning on the profile option and
 * memwatch will record each proceedure as it is entered
 *
 * The cost of MEM_NUM_LAST_PROCS is 4 bytes per item.
 */
#define MEM_NUM_LAST_PROCS (256)

/*
 * at startup space is allocated to record this many heap blocks
 */
#define MEM_INITIAL_HEAP_BLOCKS	(1000)

/*
 * If the debugged program ends up exceeding MEM_INITIAL_HEAP_BLOCKS then
 * allocate enough space to record this many more blocks each time we run out
 */
#define MEM_ADDITIONAL_HEAP_BLOCKS	(500)
