#include "bmminclude.h"
#if MEM_DEBUG>0
/*
 * AAMemDisplay.c - display MemWatch tables to a file
 * by Aaron Wohl (n3liw+@cmu.edu)
 * public domain no rights reserved
 */

#pragma options(!profile)

#include "MemWatchInternal.h"
#include "bbt.h"
#include "unix.h"
#include <ctype.h>
#include <stdio.h>
#include <Packages.h>

/*
 * write out all the memory blocks
 */
static void dump_blocks(FILE *ofile,unsigned long here_key)
{
	bbt_table_pt by_time=mem_get_by_time();
	mem_entry_pt here;
	char buf[20000];
	long tot_size=0;
	long tot_blocks=0;
	unsigned long last_block=mem_get_epoch();
	if(here_key!=MEM_FIRST_BLOCK) {
		if(!bbt_find(by_time,(bbt_key_t)here_key,(bbt_key_t *)&here))
		  if(!bbt_next(by_time,(bbt_key_t)here_key,(bbt_key_t *)&here_key,(bbt_value_t*)&here))
			return;
	} else if(!bbt_first(by_time,(bbt_key_t *)&here_key,(bbt_value_t*)&here))
		return;
	do {
		if(((unsigned long) here_key)>=last_block)	/*don't display blocks created while we run*/
			break;
		mem_dump_block(buf,here,&tot_size);
		tot_blocks++;
		fprintf(ofile,"%s\n",buf);
	} while(bbt_next(by_time,(bbt_key_t)here_key,(bbt_key_t *)&here_key,(bbt_value_t*)&here));
	fprintf(ofile,"# blocks=%ld, space used=%ld, blocks available=%ld\n",
		tot_blocks,
		tot_size,
		mem_blocks_left());
}

/*
 * display a dump of memory blocks and who allocated them
 */
void mem_dump_memory_to_file(char *fname,unsigned long first_block)
{
	FILE *ofile=fopen(fname,"w");
	unsigned char now_date_string[256];
	unsigned char now_time_string[256];
	unsigned long now;
	if(ofile==0)return;
	GetDateTime(&now);
	IUDateString(now,longDate,now_date_string);
	IUTimeString(now,TRUE,now_time_string);
	fprintf(ofile,"memory dump at %#s %#s\n",
		now_date_string,now_time_string);
	mem_make_up_descriptions();
	mem_try_describe(ofile->buf,"mem_display_log",'JUNK');
	dump_blocks(ofile,first_block);
	fclose(ofile);
}
#endif
