/*
 * INIT to load and start the softkiss driver
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

void real_main(void);

void main()
{
  asm {
  	jsr @strip_pc
  	move.l a4,-(sp)
  	lea main,a4
  	jsr real_main
  	move.l (sp)+,a4
	rts

@strip_pc
	move.l Lo3Bytes,d0
	and.l d0,(sp)
  }
}

#include "sfk_core.h"
#include "sfk_core_private.h"

#include "sfk_drvr_find.h"
#include "string.h"
#include "CShowINIT_PROC.h"

#define cpanal_ICON	(-4064)
#define on_ICON 	(-4064)

#define start_ICON	(-4063)
#define off_ICON	(-4062)
#define sick_ICON	(-4061)
#define finder_ICON	(-4059)

static void tick(int refnum)
{
  sfk_io_record s;
  sfk_INIT_CPB(s,refnum,sfk_CMD_run);
  PBControl((void *)&s,FALSE);
}

static int sfk_start(void)
{
  int err;
  short refnum;

  if(Button())
  	return off_ICON;
  err=sfk_drvr_find_or_install(&refnum);
  if(err==0)
  	return sick_ICON;

  err=OpenDriver(sfk_driver_name,&refnum);
  if(err!=0)
  	return sick_ICON;

  return on_ICON;
}

void real_main()
{
  int final_icon;
  ShowINIT(start_ICON,0);
  final_icon=sfk_start();
  ShowINIT(final_icon,-1);
}
