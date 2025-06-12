/*
 * SoftKiss scc setup/unitialize routines
 * for use by code inside the device driver.
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core.h"
#include "sfk_core_private.h"
#include "8530.h"

#define sfk_bCtl 0				/*offset from SCCRd or SCCWr to b control port*/
#define sfk_bData 4				/*offset from SCCRd or SCCWr to b data port*/
#define sfk_aCtl 2				/*offset from SCCRd or SCCWr to a control port*/
#define sfk_aData 6				/*offset from SCCRd or SCCWr to a data port*/

#define sfk_xCtl 0				/*offset for interpt routine to control*/
#define sfk_xData 4

#define sfk_END_OF_SCC_LIST (0x0FF)

/*
 * shutdown port (undo scc_init values)
 * this list has all reset values except the actual reset
 * command.  the reset command is written to port 9 wich is
 * not duplicated but actuallyt the same register for A and B.
 */
static uint8 sfk_scc_uninit[]={
 R10,0x00,			/*flag under (no abort), flag idle (no abort*/
 R0,RES_EXT_INT,	/*reset ext/sts interrupts*/
 R15,DCDIE,			/*enable DCD interrupt (used by the mouse driver)*/
 R1,EXT_INT_ENAB,	/*no receive interrupts, just external interrupts*/
 R5,0x00,			/*stop tx, lower dtr*/
 sfk_END_OF_SCC_LIST,sfk_END_OF_SCC_LIST
};

/*
 * set buad rate regiser for the passed divider
 * due to a hardware bug (see amd scc manual)
 * we can't stop the divider while we do this
 */
static void sfk_scc_set_buad_divider(sfk_prt_pt p,uint16 divider)
{
	uint8 high=divider>>8;
	uint8 low=divider;
	if((sfk_read_scc(p,R13)==high)&&
		(sfk_read_scc(p,R12)==low))
			return;
	/* choose write order to mimimize long count if underflow
	 * happens between writes and don't make a zero br divider
	 */
	if(high!=0) {
		sfk_write_scc(p,R13,high);
		sfk_write_scc(p,R12,low);
	} else {
		sfk_write_scc(p,R12,low);
		sfk_write_scc(p,R13,high);
	}
		
}

/*
 * set divider for proper rate to transmit
 * need to wait one underflow to pick up proper rate
 */
void sfk_scc_set_buad_divider_xmit(sfk_prt_pt p)
{
	sfk_scc_set_buad_divider(p,p->sfk_IVAR(xmit_rate_divider));
}

/*
 * set divider for proper rate to recieve
 * need to wait one underflow to pick up proper rate
 */
void sfk_scc_set_buad_divider_recv(sfk_prt_pt p)
{
	sfk_scc_set_buad_divider(p,p->sfk_IVAR(recv_rate_divider));
}

static void sfk_scc_wrt_ctl_list(p,data_list)
sfk_prt_pt p;
register uint8 *data_list;
{
	register uint8 register_number;
 	while((register_number=*data_list++)!=sfk_END_OF_SCC_LIST)
 		sfk_write_scc(p,register_number,*data_list++);
}

static void extra_delay_for_scc_reset()
{
	register long i=40;	/*should be fast enough on any speed processor*/
	while(i-- > 0)
		;
}

static int sfk_scc_reset_val[2]={0x4a,0x8a};
void sfk_reset_scc(sfk_prt_pt p)
{
	sfk_write_scc(p,R9,sfk_scc_reset_val[p->pnum]);
	extra_delay_for_scc_reset();
	sfk_scc_wrt_ctl_list(p,sfk_scc_uninit);
}

/*
 * initilize the scc chip port sdlc
 */
static uint8 sfk_scc_init[]={
  R4,0x20,	/*x1 clock, SDLC mode, syncronious mode, no parity*/
  R1,0x13,	/*interupt on any char tx and external*/
  R15,SYNCIE|DCDIE,	/*interupt on zc sync*/
  R3,0xcb,	/*Rx 8 bits/char, hunt mode, Rx CRC on, Rx on*/
  R5,0x61,	/*8 bits/char dtr low,rts low, crc16*/
  R6,0x7e,	/*we are sdlc address zero*/
  R7,0x7e,	/*sync character is 7e*/
  R10,0xa0,	/*CRC preset, NRZI mode, 8 bit sync,abort underun*/
  R11,0x70,	/*receive clk is DPLL, TR clk is BR gen*/
  R14,0x81,	/*dpll source is br*/
  R14,0xe1,	/*NRZI mode, baud generator off*/

  R14,0x21,	/*enter search mode, turn on transmit BR*/
  R3,0xdb,	/*Rx 8 bits/char, hunt mode, Rx CRC on, Rx on*/
  R0,0x10,	/*reset external ints*/
  R0,0x10,

  sfk_END_OF_SCC_LIST,sfk_END_OF_SCC_LIST	/*terminate list*/
};

/*
 * setup pointers to registers
 */
void sfk_init_register_addresses(sfk_prt_pt p)
{
	if(p->pnum==0)	{
		p->ctl_rd=((uint8*)SCCRd)+sfk_bCtl;
		p->ctl_wr=((uint8*)SCCWr)+sfk_bCtl;
		p->data_rd=((uint8*)SCCRd)+sfk_bData;
		p->data_wr=((uint8*)SCCWr)+sfk_bData;
	} else {
		p->ctl_rd=((uint8*)SCCRd)+sfk_aCtl;
		p->ctl_wr=((uint8*)SCCWr)+sfk_aCtl;
		p->data_rd=((uint8*)SCCRd)+sfk_aData;
		p->data_wr=((uint8*)SCCWr)+sfk_aData;
	}
}

/*
 * initialize the scc for normal operation
 */
void sfk_setup_scc(sfk_prt_pt p)
{
	sfk_reset_scc(p);
	sfk_scc_set_buad_divider_recv(p);
	sfk_scc_wrt_ctl_list(p,sfk_scc_init);
}

