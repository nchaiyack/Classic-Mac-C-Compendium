/*
 * dbo_test.c - test interupt level debugging output
 */

#include <stdio.h>
#include <console.h>

#include "dbo_stdio.h"

/*
 * loop to test drawing characters on the screen
 */
static void dbo_test()
{
	dbo_FILE af;
	char ch;
	char buf[100];
	dbo_fopen(&af);
	/*window is in upper left quater of screen*/
	af.win_tl.h=8;
	af.win_tl.v=10;
	af.win_cur=af.win_tl;	/*start writeing in window*/
	af.win_br.h=200;
	af.win_br.v/=2;
	dbo_fclear(&af);

	while(TRUE) {
		fgets(buf,sizeof(buf),stdin);
//		dbo_fputc(&af,ch);
		dbo_fprintf(&af,"abcdefghijklmnopqrstuvwxyz`~!@#$%^&*()[]\\;',./{}|:<>?\n");
//		dbo_fprintf(&af,"ABCDEFGHIJKLMNOPQRSTUVWXYZ`~!@#$%^&*()[]\\;',./{}|:<>?\n");
		af.wrap=FALSE;
		dbo_fprintf(&af,"'%5d' '%5ld' %04ld '%-4ld' %s %#s\n",-1,-1000L,23L,25L,"hi","\pPHI");
		dbo_fprintf(&af,"Hi %d %ld %x %lx %c\n",1,2L,0x8081,0x12345L,'g');
		af.wrap=TRUE;
	}
}

int main()
{
  console_options.left=40;
  console_options.ncols=40;
  printf("dbo_test test client test jig built on %s %s\n",__DATE__,__TIME__);
  dbo_test();
}
