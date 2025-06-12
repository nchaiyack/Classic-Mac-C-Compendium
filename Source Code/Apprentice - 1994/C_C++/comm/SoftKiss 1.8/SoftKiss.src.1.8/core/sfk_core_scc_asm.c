/*
 * SoftKiss scc access routines in c and assembler
 * for use by code inside the device driver.
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

/*
 * define SCC_ASM_DEFS here for the assembler version of routines
 * The assembler and c versions are compatable except for speed.
 * At higher baudrates the c versions may not be able to keep up.
 */
#define SCC_ASM_DEFS

#include "sfk_core.h"
#include "sfk_core_private.h"

#ifdef SCC_ASM_DEFS
#pragma options(!require_protos)
#endif

/*
 * write a byte to a scc register
 */
#ifndef SCC_ASM_DEFS
void sfk_write_scc(sfk_prt_pt p,uint8 rnum,uint8 val)
{
	*(p->ctl_wr)=rnum;
	p->scc_reg_cache[rnum]=val;
	p->sfk_IVAR(scc_ctl_writes)++;
	sfk_SCC_COUNT_DELAY;
	*(p->ctl_wr)=val;
}
#else
void sfk_write_scc( /*sfk_prt_pt p,uint8 rnum,uint8 val */ )
{
asm {
	move.l 4(sp),a1
	move.l sfk_prt.ctl_wr(a1),a0 	//a0=address of write reg
	moveq.l #0,d0
	move.b 8(sp),d0					//d1 location to write
	move.b d0,(a0)
	addq.l #1,sfk_IVAR_ASM(scc_ctl_writes)(a1)
	lea sfk_prt.scc_reg_cache(a1),a1 //get addr of write cache
	move.b 0xa(sp),d1
	move.b d1,(a1,d0)
	move.b d1,(a0)
	}
}
#endif

/*
 * read a byte to a scc register
 */
#ifndef SCC_ASM_DEFS
uint8 sfk_read_scc(sfk_prt_pt p,uint8 rnum)
{
	uint8 result;
	if(rnum!=0)
	 *(p->ctl_wr)=rnum;
	p->sfk_IVAR(scc_ctl_reads)++;
	sfk_SCC_COUNT_DELAY;
	result= *(p->ctl_rd);
 	return result;
}
#else
uint8 sfk_read_scc( /* sfk_prt_pt p,uint8 rnum */)
{
asm {
	move.l 4(sp),a1
	move.l sfk_prt.ctl_wr(a1),a0 	//a0=address of read reg
	move.b 8(sp),(a0)				//d1 location to read
	addq.l #1,sfk_IVAR_ASM(scc_ctl_reads)(a1)
	move.l sfk_prt.ctl_rd(a1),a0 	//a0=address of read reg
	move.b (a0),d0
	}
}
#endif

/*
 * read a data byte
 */
#ifndef SCC_ASM_DEFS
uint8 sfk_read_scc_data(sfk_prt_pt p)
{
	uint8 result;
	result= *(p->data_rd);
	p->sfk_IVAR(scc_data_reads)++;
	sfk_SCC_COUNT_DELAY;
 	return result;
}
#else
uint8 sfk_read_scc_data( /*sfk_prt_pt p */)
{
asm {
	move.l 4(sp),a1
	addq.l #1,sfk_IVAR_ASM(scc_data_reads)(a1)
	move.l sfk_prt.data_rd(a1),a0 	//a0=address of read reg
	move.b (a0),d0
	}
}
#endif

/*
 * write a data to the scc
 */
#ifndef SCC_ASM_DEFS
void sfk_write_scc_data(sfk_prt_pt p,uint8 val)
{
	*(p->data_wr)=val;
	p->sfk_IVAR(scc_data_writes)++;
	sfk_SCC_COUNT_DELAY;
}
#else
void sfk_write_scc_data( /*sfk_prt_pt p,uint8 val*/ )
{
asm {
	move.l 4(sp),a1
	addq.l #1,sfk_IVAR_ASM(scc_data_writes)(a1)
	move.l sfk_prt.data_wr(a1),a0 	//a0=address of read reg
	move.b 8(sp),(a0)
	}
}
#endif

