/*
 * glue to tell mailcheck about userids and that macmail is running
 */

void real_main(long want_inhibit,char *name);

void main()
{
  asm {
  	move.l 4(sp),a0
  	move.l 8(sp),a1
  	move.l a4,-(sp)
  	move.l a1,-(sp)
  	move.l a0,-(sp)
  	lea main,a4
  	jsr real_main
  	add.l #8,sp
  	move.l (sp)+,a4
  }
}

#include "mmc_macmail.h"

void real_main(long want_inhibit,char *name)
{
	mmc_macmail(want_inhibit,name);
}
