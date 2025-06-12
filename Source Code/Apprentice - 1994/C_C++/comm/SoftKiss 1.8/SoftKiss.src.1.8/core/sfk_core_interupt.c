/*
 * SoftKiss device handling code interupt handling code
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core.h"
#include "sfk_core_private.h"
#include "asm.h"
#include "8530.h"

/*
 * option a45_globals is set by the thinkc compiler when
 * making a driver or code resource
 */
#if __option(a4_globals)
#define sfk_GLOBAL_REG a4
#else
#define sfk_GLOBAL_REG a5
#endif

#define MAGIC_UNPATCHED_NUMBER 0x12345678

#ifdef CODECHECK				//codecheck doesn't know about asm constructs
#define sfk_SETUP_GLOBAL_REG	//fake macro for codecheck
#define sfk_RESTORE_GLOBAL_REG	//fake macro for codecheck
#else
#define sfk_SETUP_GLOBAL_REG \
	asm { jsr sfk_find_address } \
	asm { move.l sfk_GLOBAL_REG,-(a7) } \
	asm { move.l #MAGIC_UNPATCHED_NUMBER,sfk_GLOBAL_REG } /*patch at runtime*/

#define sfk_RESTORE_GLOBAL_REG \
	asm { move.l (a7)+,sfk_GLOBAL_REG }
#endif

/*system interupt vectors*/
#define sfk_Lvl2_B_transm 0		/*Lvl2DT index for SCC B transmit interupt*/
#define sfk_Lvl2_B_data 2		/*Lvl2DT index for SCC B data interupt*/
#define sfk_Lvl2_B_special 3	/*Lvl2DT index for SCC B special interupt*/
#define sfk_Ext_B 0				/*ExtStsDT index for SCC B external*/

#define sfk_Lvl2_A_transm 4		/*Lvl2DT index for SCC A transmit interupt*/
#define sfk_Lvl2_A_data 6		/*Lvl2DT index for SCC A data interupt*/
#define sfk_Lvl2_A_special 7	/*Lvl2DT index for SCC A special interupt*/
#define sfk_Ext_A 2				/*ExtStsDT index for SCC A external*/

/*
 * helper routine to help locate where code is actually
 * located.  when you take the address of code with a & you
 * get the jump table address
 * returns to callers caller with result in a0
 */
static void sfk_find_address()
{
#ifndef CODECHECK
	asm { move.l (a7)+,a0 }
#endif
}

/*
 * begin actually transmiting data
 * -interupt
 */
static void sfk_xmit_first_byte(sfk_prt_pt p)
{
	sfk_go_state(p,PX_XMIT);
}

/*
 * clear an overrun condition
 */
static void sfk_recv_overrun(sfk_prt_pt p)
{
	p->sfk_IVAR(recv_overrun)++;
 	sfk_write_scc(p,R3,Rx8|ENT_HM|RxCRC_ENAB|SYNC_L_INH|RxENABLE); /*enter hunt mode*/
 	/*bug, deal with partial packet*/
}

/*
 * end of current input packet
 */
static void sfk_end_of_rx_packet(sfk_prt_pt p,int isgood)
{
  register long pak_len=p->in_count;
  if(pak_len > p->sfk_IVAR(recv_largest_seen))
  	 p->sfk_IVAR(recv_largest_seen)=pak_len;
  if((pak_len > p->sfk_IVAR(max_packet_size))||
    	(pak_len < (SFK_MIN_PACK_SIZE+1)))
  		isgood=FALSE;		/*too short*/
  p->sfk_in->cnt=pak_len-1;	/*don't count the one FCS byte buffered*/
  if(isgood&&(p->sfk_freehq.sfk_size>0)&&(p->sfk_freedq.sfk_size>0)) { /*can't use last packet*/
	sfk_enqueue(&p->sfk_recvq,p->sfk_in);
 	p->sfk_in=0;
  }
  sfk_prep_rx(p);
  sfk_go_state(p,PX_RX);
}

/*
 * prepare the sfk_in packet to be ready to start recieving
 */
void sfk_prep_rx(sfk_prt_pt p)
{
  register sfk_packet_pt ap=p->sfk_in;
  if(ap==0)
    SFK_ASSERT(((ap=sfk_allocate_packet(p))!=0),SFK_TEXT(61));
  p->in_count=0;
  p->sfk_in=ap;
  ap->cnt=0;
  ap->att_read=0;
  ap->data_flags=0;
}

/*
 * handle data arrived interupt but no more buffer space
 */
static void sfk_recv_chuck_data_int(sfk_prt_pt p)
{
 	sfk_write_scc(p,R3,Rx8|ENT_HM|RxCRC_ENAB|SYNC_L_INH|RxENABLE); /*enter hunt mode*/
	while((sfk_read_scc(p,R0)&Rx_CH_AV)!=0)
		sfk_read_scc_data(p);
}

/*
 * handle special interupt
 */
static void sfk_special_int(sfk_prt_pt p)
{
  int status;
  status=sfk_read_scc(p,R1);
  sfk_write_scc(p,R0,ERR_RES);		/*reset rx and error*/
  if(status&Rx_OVR)
    sfk_recv_overrun(p);
  sfk_recv_chuck_data_int(p);
  sfk_end_of_rx_packet(p,
        ((status&(CRC_ERR|Rx_OVR|RES18))==RES8));		/*good if no overrun or bad crc or bad residue*/
  if(WANT_XMIT(p))
  	sfk_initiate_xmit(p);
}

/*
 * force keydown
 */
void sfk_keydown(sfk_prt_pt p)
{
  /*save time by not clearing timer now, if it wakes up it will
    not do anything if xmit abort is set*/
  // sfk_clear_timer(p);
  p->timer_ignore=TRUE;
  sfk_write_scc(p,R5,
  		(p->scc_reg_cache[R5]&
  			(~(DTR|TxENAB|RTS|TxCRC_ENAB)))|
  			p->extra_tx_bits_for_line_power);

}

/*
 * clear the timer if it is set
 */
void sfk_clear_timer(sfk_prt_pt p)
{
  if(p->timer_installed)
	RmvTime((QElemPtr)&p->timer_task);
  p->timer_installed=FALSE;
}

/*
 * set the timer to call to the specified routine after delay
 * delay is in milliseconds if posative or microseconds if negative
 */
void sfk_install_timer(sfk_prt_pt p,void (*rtn)(sfk_prt_pt p),long delay)
{
  // sfk_clear_timer(p);
  p->timer_ignore=TRUE;
  if(!p->timer_installed) {
  	p->timer_installed=TRUE;
  	memset(&p->timer_task,0,sizeof(p->timer_task));
  	p->timer_task.tmAddr= (ProcPtr)(p->low_level_timer_routine);
  	InsTime((QElemPtr)&p->timer_task);
  }
  p->timer_routine=rtn;
  p->timer_ignore=FALSE;
  PrimeTime((QElemPtr)&p->timer_task,delay);
}

/*
 * handler for an unexpected xmit interupt
 * shouldn't happen, only listed possible cause in manual
 * is xmit underflow
 * an abort is automaticly transmited by the sc
 */
static void sfk_unexpected_xmit_int(sfk_prt_pt p)
{
	p->sfk_IVAR(xmit_overrun)++;
	sfk_go_state(p,PX_TAIL);
}

/*
 * got an xmit interupt when we don't need one
 */
static void sfk_end_xmit(sfk_prt_pt p)
{
	sfk_write_scc(p,R0,RES_Tx_P);
}

/*
 * put scc in xmit mode and schedule transmit
 */
void sfk_tell_scc_to_xmit(sfk_prt_pt p)
{
	sfk_scc_set_buad_divider_xmit(p);
	sfk_cand_scc(p,R15,TxUIE);
	sfk_or_scc(p,R5,DTR|TxENAB|RTS|TxCRC_ENAB);		/*turn on DTR for xmit*/
	sfk_keyup(p,p->sfk_IVAR(os_tx_delay));
}

/*
 * ignore an interupt
 */
static void sfk_ignore_int(sfk_prt_pt p)
{
}

/*
 * keyup and schedule transmit
 */
void sfk_keyup(sfk_prt_pt p,long delay_time)
{
	p->pr_recv_int=sfk_recv_chuck_data_int;
	p->pr_spcl_int=sfk_ignore_int;
	p->pr_extr_int=sfk_ignore_int;
	sfk_install_timer(p,sfk_xmit_first_byte,delay_time);
}

/*
 * call timer routine for port b
 */
static void timer_handler_b()
{
	sfk_SETUP_GLOBAL_REG;
	sfk_SCC_interupts_off;
	if(!sfk_PB->timer_ignore)
		(*sfk_PB->timer_routine)(sfk_PB);
	sfk_SCC_interupts_on;
	sfk_RESTORE_GLOBAL_REG;
}

/*
 * call timer routine for port a
 */
static void timer_handler_a()
{
	sfk_SETUP_GLOBAL_REG;
	sfk_SCC_interupts_off;
	if(!sfk_PA->timer_ignore)
		(*sfk_PA->timer_routine)(sfk_PA);
	sfk_SCC_interupts_on;
	sfk_RESTORE_GLOBAL_REG;
}

/*
 * begin transmiting
 * call with interupts off
 */
void sfk_initiate_xmit(sfk_prt_pt p)
{
	if(p->sfk_state!=PX_RX)
		return;
	if(!WANT_XMIT(p))
		return;
	sfk_go_state(p,PX_SLOT_WAIT);
}

/*
 * handle external/status interupt
 */
static void sfk_recv_external_int(sfk_prt_pt p)
{
  sfk_end_of_rx_packet(p,FALSE);
}

/*
 * handle data arrived interupt in recive mode
 */
static void sfk_recv_data_read_int(sfk_prt_pt p)
{
  	register sfk_packet_pt ap=p->sfk_in;
	uint8 abyte;
	while((sfk_read_scc(p,R0)&Rx_CH_AV)!=0) {
		char ch=sfk_read_scc_data(p);
		*(ap->data+p->in_count++)=ch;
		dbo_fputc(&p->dbo,ch);
		// dbo_fputc(&p->dbo,'R');
		if(p->in_count >= p->sfk_IVAR(max_packet_size)) {
			ap->data_flags|=sfk_db_LONG;
			sfk_go_state(p,PX_RX_CHUCK);
			return;
		}
	}
}

/*
 * transition to actually recieving a packet
 */
void sfk_set_recv_now_mode(sfk_prt_pt p)
{
	register sfk_packet_pt ap=p->sfk_in;
	ap->sfk_time_in=Ticks;
	p->pr_recv_int=sfk_recv_data_read_int;
 	sfk_recv_data_read_int(p);
}

/*
 * transition to actually recieve packet but throw away data
 */
void sfk_set_recv_chuck_mode(sfk_prt_pt p)
{
	p->pr_recv_int=sfk_recv_chuck_data_int;
	sfk_recv_chuck_data_int(p);
}

/*
 * send bytes interupt
 */
static void sfk_xmit_int(sfk_prt_pt p)
{
  	register sfk_packet_pt ap=p->sfk_out;
  	if(p->sfk_state!=PX_XMIT) {
		sfk_end_xmit(p);
		return;
  	}
	while((sfk_read_scc(p,R0)&Tx_BUF_EMP)!=0) {
		char ch=*(ap->data+p->out_count++);
		sfk_write_scc_data(p,ch);
		dbo_fputc(&p->dbo,ch);
		// dbo_fputc(&p->dbo,'S');
		if(p->out_count >= ap->cnt) {
			sfk_cand_scc(p,R10,ABUNDER);	//let crc and flag go out
			sfk_go_state(p,PX_TAIL);
			return;
		}
	}
}

/*
 * set state to allow crc/sync char to go out
 */
void sfk_tail(sfk_prt_pt p)
{
	sfk_end_xmit(p);
	sfk_cand_scc(p,R15,TxUIE);
	sfk_cand_scc(p,R1,TxINT_ENAB);	//no xmit interupt
	if(p->sfk_out!=0)
	 	sfk_free_packet(p,p->sfk_out);
	p->sfk_out=0;
	sfk_keyup(p,p->sfk_IVAR(os_tail_delay));
}

/*
 * start transmiting
 */
void sfk_send_first_byte(sfk_prt_pt p)
{
	register sfk_packet_pt ap;
	if((ap=sfk_dequeue(&p->sfk_xmitq))==0) {
		sfk_go_state(p,PX_RX);			//strange should have had a packet
		return;
	}
	p->pr_extr_int=sfk_unexpected_xmit_int;
	p->sfk_out=ap;
	p->out_count=0;
	sfk_or_scc(p,R10,ABUNDER);			//enable underrun protection
	sfk_write_scc(p,R0,RES_Tx_CRC);		//reset tx crc
	sfk_xmit_int(p);
	if(p->sfk_state==PX_XMIT) {
		// must be reset after first and before last byte
		// see z85c30 amd serial communications control technical manual
		// page 4-21
		sfk_write_scc(p,R0,RES_EOM_L);	//reset tx eom latch
		sfk_or_scc(p,R15,TxUIE);		//interupt on underflow
		sfk_or_scc(p,R1,TxINT_ENAB);	//no xmit interupt
	}
}

/*
 * handle data arrived to start a packet
 */
static void sfk_recv_start_of_packet_data_int(sfk_prt_pt p)
{
	sfk_go_state(p,PX_RX_NOW);
}

/*
 * comment on external interupt
 */
static void ecomment(sfk_prt_pt p)
{
	long estatus;
	asm { move.l d0,estatus }
	if((estatus&BRK_ABRT)!=0)
		dbo_fputc(&p->dbo,'a');
	if((estatus&TxEOM)!=0)
		dbo_fputc(&p->dbo,'u');
	if((estatus&CTS)!=0)
		dbo_fputc(&p->dbo,'c');
	if((estatus&SYNC_HUNT)!=0)
		dbo_fputc(&p->dbo,'h');
	if((estatus&DCD)!=0)
		dbo_fputc(&p->dbo,'d');
	if((estatus&ZCOUNT)!=0)
		dbo_fputc(&p->dbo,'z');
 }

/*
 * set interupts and buad rate for recieve mode
 */
void sfk_set_recv_mode(sfk_prt_pt p)
{
	sfk_scc_set_buad_divider_recv(p);
	p->pr_recv_int=sfk_recv_start_of_packet_data_int;
	p->pr_spcl_int=sfk_special_int;
	p->pr_extr_int=sfk_recv_external_int;
}

/*
 * port b transmit interupt
 */
sfk_LDEF(void sfk_b_xmit_interupt())
{
	sfk_SETUP_GLOBAL_REG;
  	sfk_SCC_interupts_off;
 	sfk_PB->sfk_IVAR(xmit_interupt)++;
	sfk_xmit_int(sfk_PB);
	sfk_reset_scc_ius(sfk_PB);
 	sfk_SCC_interupts_on;
	sfk_RESTORE_GLOBAL_REG;
}

/*
 * port b data interupt
 */
sfk_LDEF(void sfk_b_data_interupt())
{
	sfk_SETUP_GLOBAL_REG;
  	sfk_SCC_interupts_off;
 	(*sfk_PB->pr_recv_int)(sfk_PB);
 	sfk_PB->sfk_IVAR(data_interupt)++;
 	sfk_reset_scc_ius(sfk_PB);
  	sfk_SCC_interupts_on;
	sfk_RESTORE_GLOBAL_REG;
}

/*
 * port b special interupt
 */
sfk_LDEF(void sfk_b_special_interupt())
{
	sfk_SETUP_GLOBAL_REG;
  	sfk_SCC_interupts_off;
   	dbo_fputc(&sfk_PB->dbo,'S');
	(*sfk_PB->pr_spcl_int)(sfk_PB);
 	sfk_PB->sfk_IVAR(special_interupt)++;
	sfk_reset_scc_ius(sfk_PB);
  	sfk_SCC_interupts_on;
	sfk_RESTORE_GLOBAL_REG;
}

/*
 * port b external interupt
 */
sfk_LDEF(void sfk_b_external_interupt())
{
	sfk_SETUP_GLOBAL_REG;
  	sfk_SCC_interupts_off;
  	ecomment(sfk_PB);
  	dbo_fputc(&sfk_PB->dbo,'E');
	(*sfk_PB->pr_extr_int)(sfk_PB);
	sfk_PB->sfk_IVAR(external_interupt)++;
   	sfk_SCC_interupts_on;
	sfk_RESTORE_GLOBAL_REG;
}

/*
 * port a transmit interupt
 */
sfk_LDEF(void sfk_a_xmit_interupt())
{
	sfk_SETUP_GLOBAL_REG;
  	sfk_SCC_interupts_off;
	sfk_xmit_int(sfk_PA);
	sfk_PA->sfk_IVAR(xmit_interupt)++;
	sfk_reset_scc_ius(sfk_PA);
  	sfk_SCC_interupts_on;
	sfk_RESTORE_GLOBAL_REG;
}

/*
 * port a data interupt
 */
sfk_LDEF(void sfk_a_data_interupt())
{
	sfk_SETUP_GLOBAL_REG;
  	sfk_SCC_interupts_off;
 	sfk_PA->sfk_IVAR(data_interupt)++;
	(*sfk_PA->pr_recv_int)(sfk_PA);
  	sfk_reset_scc_ius(sfk_PA);
  	sfk_SCC_interupts_on;
	sfk_RESTORE_GLOBAL_REG;
}

/*
 * port a special interupt
 */
sfk_LDEF(void sfk_a_special_interupt())
{
	sfk_SETUP_GLOBAL_REG;
   	sfk_SCC_interupts_off;
   	dbo_fputc(&sfk_PA->dbo,'S');
	(*sfk_PA->pr_spcl_int)(sfk_PA);
	sfk_PA->sfk_IVAR(special_interupt)++;
 	sfk_reset_scc_ius(sfk_PA);
  	sfk_SCC_interupts_on;
	sfk_RESTORE_GLOBAL_REG;
}

/*
 * port a external interupt
 */
sfk_LDEF(void sfk_a_external_interupt())
{
	sfk_SETUP_GLOBAL_REG;
   	sfk_SCC_interupts_off;
  	ecomment(sfk_PA);
   	dbo_fputc(&sfk_PA->dbo,'E');
	(*sfk_PA->pr_extr_int)(sfk_PA);
  	sfk_PA->sfk_IVAR(external_interupt)++;
  	sfk_SCC_interupts_on;
	sfk_RESTORE_GLOBAL_REG;
}

/*
 * this table paralells the system level 2 interupt table
 * it used to know which vectors to swap in
 * if an entry in this table is zero then the corrisponding
 * system table is not updated
 */
static void *sfk_lvl2_interupt_routines[8];
static void *sfk_ext_interupt_routines[4];

/*offsets into level table based on port*/
static int port_level_2_offset[]={sfk_Lvl2_B_transm,sfk_Lvl2_A_transm};
static int ext_offset[]={sfk_Ext_B,sfk_Ext_A};

/*
 * set scc level 2 interupt vectors to point to our routines
 * save the old ones
 * must be called with interupts off
 */
sfk_LDEF(void install_lvl2_interupts(sfk_prt_pt p))
{
	int vec_num=port_level_2_offset[p->pnum];
	int num_vecs=4;
	while(num_vecs-- > 0) {
		if(sfk_lvl2_interupt_routines[vec_num]!=0) {
			sfk_gl.saved_lvl2[vec_num]=Lvl2DT[vec_num];
			Lvl2DT[vec_num]=sfk_lvl2_interupt_routines[vec_num];
		}
		vec_num++;
	}
}

/*
 * must be called with interupts off
 * set scc external interupt vectors to point to our routines
 * save the old ones
 */
sfk_LDEF(void install_ext_interupts(sfk_prt_pt p))
{
	int vec_num=ext_offset[p->pnum];
	int num_vecs=2;
	while(num_vecs-- > 0) {
		if(sfk_ext_interupt_routines[vec_num]!=0) {
			sfk_gl.saved_ext[vec_num]=ExtStsDT[vec_num];
			ExtStsDT[vec_num]=sfk_ext_interupt_routines[vec_num];
		}
		vec_num++;
	}
}

/*
 * set scc interupt vectors to point to our routines
 * save the old ones
 * must be called with interupts off
 */
void sfk_install_interupt_vectors(sfk_prt_pt p)
{
	if(p->sfk_IVAR(our_vectors)) return;	/*if already installed no problem*/
	p->sfk_IVAR(our_vectors)=TRUE;
	install_lvl2_interupts(p);
	install_ext_interupts(p);
}

/*
 * restore system level 2 interupts
 * must be called with interupts off
 */
sfk_LDEF(void remove_lvl2_interupts(sfk_prt_pt p))
{
	int vec_num=port_level_2_offset[p->pnum];
	int num_vecs=4;
	while(num_vecs-- > 0) {
		if(sfk_lvl2_interupt_routines[vec_num]!=0)
			Lvl2DT[vec_num]=sfk_gl.saved_lvl2[vec_num];
		vec_num++;
		}
}

/*
 * restore system external interupts
 * must be called with interupts off
 */
sfk_LDEF(void remove_ext_interupts(sfk_prt_pt p))
{
	int vec_num=ext_offset[p->pnum];
	int num_vecs=2;
	while(num_vecs-- > 0) {
		if(sfk_ext_interupt_routines[vec_num]!=0)
			ExtStsDT[vec_num]=sfk_gl.saved_ext[vec_num];
		vec_num++;
		}
}

/*
 * set scc interupt vectors to point to our routines
 * save the old ones
 * must be called with interupts off
 */
void sfk_remove_interupt_vectors(sfk_prt_pt p)
{
	if(!p->sfk_IVAR(our_vectors))	/*if already uninstalled no problem*/
		return;
	p->sfk_IVAR(our_vectors)=FALSE;
	remove_lvl2_interupts(p);
	remove_ext_interupts(p);
}

/*
 * return the contents of the global register (A5 or A4)
 */
static long get_global_reg()
{
	asm { move.l sfk_GLOBAL_REG,d0 }
}

/*
 * patch in global register to interupt routines
 */
static void patch_in_global(void *iroutine)
{
	long *patch_point=((long *)(((char *)iroutine)+4));
	SFK_ASSERT(((*patch_point)==MAGIC_UNPATCHED_NUMBER),SFK_TEXT(62));
	(*patch_point)=get_global_reg();
}

/*
 * initialize tables that can't be done as static initialization
 * in a think code segment.  patch global register into driver routines
 */
static void *mk_int(void *iroutine)
{
	void (*rtn)(void)=iroutine;
	/*call the routine to ask it where it really lives*/
	(*rtn)();
	asm { move.l a0,iroutine }
	patch_in_global(iroutine);
	return iroutine;
}

/*
 * patch each interupt routine to known where our data lives
 */
void sfk_patch_in_global_data()
{
  static int patched_in=FALSE;
  if(patched_in)
  	return;
  patched_in=TRUE;
 
  sfk_PB->low_level_timer_routine=mk_int(timer_handler_b);
  
  sfk_lvl2_interupt_routines[sfk_Lvl2_B_transm]=mk_int(sfk_b_xmit_interupt);
  sfk_lvl2_interupt_routines[sfk_Lvl2_B_data]=mk_int(sfk_b_data_interupt);
  sfk_lvl2_interupt_routines[sfk_Lvl2_B_special]=mk_int(sfk_b_special_interupt);
  sfk_ext_interupt_routines[sfk_Ext_B]=mk_int(sfk_b_external_interupt);

  sfk_PA->low_level_timer_routine=mk_int(timer_handler_a);
  sfk_lvl2_interupt_routines[sfk_Lvl2_A_transm]=mk_int(sfk_a_xmit_interupt);
  sfk_lvl2_interupt_routines[sfk_Lvl2_A_data]=mk_int(sfk_a_data_interupt);
  sfk_lvl2_interupt_routines[sfk_Lvl2_A_special]=mk_int(sfk_a_special_interupt);
  sfk_ext_interupt_routines[sfk_Ext_A]=mk_int(sfk_a_external_interupt);

  sfk_some_flush();
}

