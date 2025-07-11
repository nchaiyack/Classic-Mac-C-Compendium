/*
 * SoftKiss - dynamic memory
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1992
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core.h"
#include "sfk_core_private.h"
#include "driver_shell.h"

/*
 * allocate some memory
 */
void *sfk_malloc(unsigned long msize)
{
	void *result;
#if __option(a4_globals)
		result=NewPtrSysClear(msize);
#else
		result=NewPtrClear(msize);
#endif
	return result;
}


/*
 * free some memory
 */
void sfk_free(void *buf)
{
	DisposPtr(buf);
}

