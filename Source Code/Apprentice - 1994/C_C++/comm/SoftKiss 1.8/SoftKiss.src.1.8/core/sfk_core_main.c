/*
 * SoftKiss driver entry point for SoftKiss device driver
 * By Aaron Wohl (aw0g+@andrew.cmu.edu)
 * Carnegie-Mellon University
 * Pittsburgh, PA 15213-3890
 * (412)-268-5032
 */

#include "sfk_core.h"
#include "sfk_core_private.h"
#include "driver_shell.h"

/*
 * entry point of softkiss packet driver
 * This entry is for softkiss itself.
 * The fake serial entrypoints are in sfk_core_kiss.c
 */
short sfk_driver(CntrlParam *pb,DCtlPtr de,short op,void *extra)
{
	register sfk_io_record_pt ipb= (sfk_io_record_pt)pb;
	sfk_gl.driver_template=extra;
	switch(op) {
	case sh_OPEN:
		return sfk_init();
	case sh_CLOSE:
		return sfk_uninit(ipb);
	case sh_CONTROL:
		return sfk_control(ipb);
	default:
 		break;
	}
	return 0;
}
