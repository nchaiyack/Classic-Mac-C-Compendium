/*
 * get/save resource manager state
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#define OSP_same (0)
#define OSP_sys  (1)
#define OSP_noload (2)

/*
 * call a routine possibly switching to the system heap
 * take up to 4 arguments which are passed to the routine
 * to call.
 * The reserr proc in low memory is replaces so that if passed
 * function gets a resource error it won't call the installed
 * reserr procedure.
 */
#ifndef NO_OSP_DEF
long OSP_protected_call(int new_heap,
	...);
#endif