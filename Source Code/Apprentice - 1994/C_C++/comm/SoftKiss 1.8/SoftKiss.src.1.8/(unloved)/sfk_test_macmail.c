/*
 * test mmc_macmail_glue
 */

#include <stdio.h>
#include <string.h>

#define NIL 0L

#include "mmc_core.h"
#include "mmc_macmail.h"
#include "mmc_drvr_find.h"
#include "mmc_prep.h"
#include "mmc_print_state.h"

struct {
	int drvr_refnum;
	int drvr_res_file;
}gl;

#define fatal_error(xx_arg) {printf xx_arg;fflush(stdout);exit(0);}
extern void exit(int);

char get_cmd(void);
char get_cmd()
{
	char buf[100];
	printf("mmc> ");
	if(gets(buf)==NIL)
		return 0;
	return buf[0];
}

void get_state(void);
void get_state()
{
  mmc_state s;
  int err=mmc_get_drvr_state(gl.drvr_refnum,&s);
  printf("getstate error code = %d\n",err);
  if(err!=0)
  	return;
  mmc_print_state(&s);
}

LDEF(void mmc_set_macmail(int on,char *name))
{
  int err;
  mmc_state s;
  mmc_invent_state(&s);
  if(on)
  	TURN_ON_FLAGS(s,MCS_snooze);
  memcpy(s.mmc_uname,name,sizeof(s.mmc_uname));
  err=mmc_set_drvr_state(gl.drvr_refnum,&s,MMC_setmmuser);
  mmc_print_state(&s);
  printf("init error code = %d\n",err);
}

LDEF(void gmmc_set_macmail(int on,char *name))
{
  char **cproc;
  void (*gproc)();
  long inhibit;
  cproc=GetNamedResource('PROC',"\pMailCheckGlue");
  if(cproc==0)
    fatal_error(("can't get MailCheckGlue resource %d",ResError()));
  if((*cproc)==0)
    fatal_error(("Got a empty handle MailCheckGlue resource %d",ResError()));
  HLock(cproc);
  gproc=((void (*)())(*cproc));
  inhibit=MMC_normal;
  if(on)
  	inhibit=MMC_inhibit;
  (*gproc)(inhibit,name);
  HUnlock(cproc);
  ReleaseResource(cproc);
  get_state();
}

void cmd_loop(void);
void cmd_loop()
{
	while(TRUE) {
		char ch=get_cmd();
		switch(ch) {
		case 'q':
			return;
		case 'g':
			get_state();
			break;
		case '1':
			mmc_set_macmail(TRUE,"\paw0g");
			break;
		case '2':
			mmc_set_macmail(FALSE,"\paw0g");
			break;
		case '3':
			mmc_set_macmail(TRUE,"\pjs5l");
			break;
		case '4':
			mmc_set_macmail(FALSE,"\pjs5l");
			break;
 		case 'a':
			gmmc_set_macmail(TRUE,"\paw0g");
			break;
		case 'b':
			gmmc_set_macmail(FALSE,"\paw0g");
			break;
		case 'c':
			gmmc_set_macmail(TRUE,"\pjs5l");
			break;
		case 'd':
			gmmc_set_macmail(FALSE,"\pjs5l");
			break;
		default:
			printf("unrecignized command '%c'\n",ch);
		}
	}
}

void open_res(void);
void open_res()
{
  int res_file;
  res_file=OpenResFile("\pmmc_macmail_glue.rsrc");
  if(res_file== -1)		/*find the resource file?*/
    fatal_error(("can't open resource file mmc_macmail_glue.rsrc file %d",ResError()));
  UseResFile(res_file);
}

void print_status(void);
void print_status()
{
	int new_ref_num;
	gl.drvr_refnum=mmc_drvr_find(MMC_name,&new_ref_num);
	printf("found ref=%d new ref=%d\n",gl.drvr_refnum,new_ref_num);
}

int main()
{
	printf("mmc macmail glue test client test jig built on %s %s\n",__DATE__,__TIME__);
	print_status();
	open_res();
	cmd_loop();
}
