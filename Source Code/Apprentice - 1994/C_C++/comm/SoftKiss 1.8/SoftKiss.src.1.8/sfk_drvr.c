/*
 * SoftKiss driver
 * By Aaron Wohl (aw0g+@andrew.cmu.edu)
 * Carnegie-Mellon University
 * Pittsburgh, PA 15213-3890
 * (412)-268-5032
 */

#include "sfk_core.h"
#include "sfk_core_private.h"

void main()
{
	asm { bra sfk_driver }		/*main entrypoint*/
	asm { bra sfk_init }		/*first time*/
}
