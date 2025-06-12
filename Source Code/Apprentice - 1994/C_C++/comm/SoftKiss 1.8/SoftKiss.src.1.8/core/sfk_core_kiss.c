/*
 * SoftKiss - kiss mode emulator
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1992
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core.h"
#include "sfk_core_private.h"
#include "driver_shell.h"
#include <Serial.h>
#include <ctype.h>

#define FEND 	0xc0		//end/start of frame
#define FESC 	0xdb		//transpose next character
#define TFEND	0xdc		//transposed end
#define TFESC	0xdd		//transposed escape

#define TRANSPOSE_PROTECT 0x100

//input port states
#define KWP_LEAD_SYNC 	0 //send out leading sync
#define KWP_PORT		1 //send port number/cmd
#define KWP_DATA		2 //send data byte
#define KWP_TFEND		3 //send transposed end
#define KWP_TFESC		4 //send transposed esc

/*
 * refill current packet being read
 */
static sfk_packet_pt kiss_refill_pak(sfk_prt_pt p)
{
	sfk_packet_pt pak=p->K_IN.kp;
	if(pak!=0)
		return pak;
	pak=sfk_dequeue_protected(&p->sfk_recvq);
	if(pak==0)
		return 0;		//no data available
	p->K_IN.kp=pak;
	p->K_IN.kp_offset=0;
	return pak;
}

/*
 * read some data from packets into the serial input buffer
 */
static int kiss_read(sfk_prt_pt p,int is_async)
{
	register IOParam *ipb= p->K_IN.kp_pb;
	sfk_packet_pt pak=kiss_refill_pak(p);
	unsigned char ch;
	if(pak==0)
		return 1;
	if(ipb==0)
		return 1;	//no serial buffer to read into
	while(TRUE) {
		if(ipb->ioActCount>=ipb->ioReqCount)
			goto done;	//output buffer changed
		switch(p->K_IN.kp_state) {
		case KWP_LEAD_SYNC:
			*(ipb->ioBuffer+ipb->ioActCount++)=FEND;
			p->K_IN.kp_state=KWP_PORT;
			break;
		case KWP_PORT:	//send recieved port number
			*(ipb->ioBuffer+ipb->ioActCount++)=0;
			p->K_IN.kp_state=KWP_DATA;
			break;
		case KWP_DATA:
			//end of packet?
			if(p->K_IN.kp_offset>=pak->cnt) { //yes
				*(ipb->ioBuffer+ipb->ioActCount++)=FEND;
				p->K_IN.kp_state=KWP_LEAD_SYNC;
				p->K_IN.kp=0;
				p->K_IN.kp_offset=0;
				sfk_free_packet_protected(p,pak);
				if((pak=kiss_refill_pak(p))==0)
					goto done;	//no more data to send
				continue;		//more data so start next
			}
			ch= *(pak->data+p->K_IN.kp_offset++);
			if(ch==FEND) {
				p->K_IN.kp_state=KWP_TFEND;
				ch=FESC;
			} else if(ch==FESC) {
				p->K_IN.kp_state=KWP_TFESC;
				ch=FESC;
			}
			*(ipb->ioBuffer+ipb->ioActCount++)=ch;
			break;
		case KWP_TFEND:
			*(ipb->ioBuffer+ipb->ioActCount++)=TFEND;
			p->K_IN.kp_state=KWP_DATA;
			break;
		case KWP_TFESC:
			*(ipb->ioBuffer+ipb->ioActCount++)=TFESC;
			p->K_IN.kp_state=KWP_DATA;
			break;
		}
	}
/*
 * done for one of two reasons, no more data to send
 * or no more room to put it.  In eithor case
 * let the pending read request finish
 * since something was read
 */
done:
	if(is_async)
		sfk_mark_read_done(p,0);	//read all we need to
	else {
		p->K_IN.kp_pb->ioResult=0;
		p->K_IN.kp_pb=0;
	}
	return 0;
}

#define KCMD_TXDELAY	1
#define KCMD_P			2
#define KCMD_SlotTime	3
#define KCMD_TXtail		4
#define KCMD_FullDuplex	5
#define KCMD_SetHardware 6
#define KCMD_Return		0x0FF

/*
 * recieve a kiss command
 */
static void kiss_cmd(sfk_prt_pt p,unsigned long cmd_arg)
{
	switch(p->K_OUT.kp_cmd) {
	case KCMD_TXDELAY:
		p->sfk_IVAR(tx_delay)=(cmd_arg*10L)*1000L;
		sfk_recompute_derived_nums(p);
		break;
	case KCMD_P:
		p->sfk_IVAR(xmit_persist)= (cmd_arg*1000L)/255L;
		break;
	case KCMD_SlotTime:
		p->sfk_IVAR(xmit_slottime)=(cmd_arg*10L)*1000L;
		sfk_recompute_derived_nums(p);
		break;
	case KCMD_TXtail:
		p->sfk_IVAR(tail_delay)=(cmd_arg*10L)*1000L;
		sfk_recompute_derived_nums(p);
		break;
	case KCMD_FullDuplex:
		break;
	case KCMD_SetHardware:
		break;
	case KCMD_Return:
		sfk_go_offline(p);
		break;
	}
}

/*
 * write some output characters
 */
static void kiss_write_out(sfk_prt_pt p,long cnt,unsigned char *buf)
{
  unsigned int ch;
  while(cnt-- >0) {
  	ch= *buf++;
  	if(p->K_OUT.kp_transpose) {
  		p->K_OUT.kp_transpose=FALSE;
		switch(ch) {
 		case TFEND:
 			ch=FEND|TRANSPOSE_PROTECT;
 			break;
 		case TFESC:
 			ch=FESC|TRANSPOSE_PROTECT;
 			break;
 		default:
 			p->K_OUT.kp_state=KRP_NP;	/*hunt for sync character*/
 		}
 	} else if(ch==FESC) {
 		p->K_OUT.kp_transpose=TRUE;
 		continue;
 	}
    switch(p->K_OUT.kp_state) {
   	case KRP_NP:		//no packet hunt for sync
   		if(p->K_OUT.kp!=0) { //if have a packet buffer loose it
   			sfk_free_packet_protected(p,p->K_OUT.kp);
			p->K_OUT.kp=0;
		}
    	if(ch!=FEND)continue;
   		p->K_OUT.kp_state=KRP_SYNC;
   		break;
   	case KRP_READ_CMD:	//in sync wait for start of packet
     	p->K_OUT.kp_state=KRP_NP; //all commands 1 byte, wait for sync
     	kiss_cmd(p,ch);
     	break;
   	case KRP_SYNC:	//in sync wait for start of packet
   	{	int cmd= ch&0x0FF;
   		int pnum= (ch>>4)&0x0FF;
    	if(ch==FEND)continue;
    	/*first byte is command and port*/
    	if(cmd!=0) {
    		p->K_OUT.kp_state=KRP_READ_CMD;
    		p->K_OUT.kp_cmd=cmd;
    		continue;
    	}
    	if((pnum!=0)||(p->K_OUT.kp=sfk_allocate_protected(p))==0) {
        		p->K_OUT.kp_state=KRP_NP;
 				continue;
     	}
   		p->K_OUT.kp_state=KRP_WP;
   		p->K_OUT.kp->cnt=0;
   		p->K_OUT.kp->data_flags=0;
   		break;			//chuck first (command) byte
   	case KRP_WP:
   		if(ch!=FEND) {
   			*(p->K_OUT.kp->cnt+p->K_OUT.kp->data)=ch;
   			p->K_OUT.kp->cnt++;
   			if(p->K_OUT.kp->cnt>=p->sfk_IVAR(max_packet_size))
          		p->K_OUT.kp_state=KRP_NP;
 			continue;
   		}
   		// end of input packet
        p->K_OUT.kp_state=KRP_SYNC;
   		if(p->K_OUT.kp->cnt<SFK_MIN_PACK_SIZE)
			sfk_free_packet_protected(p,p->K_OUT.kp);
		else	//queue compleated packet for transmit
			sfk_enqueue_write(p,p->K_OUT.kp);
		p->K_OUT.kp=0;
   		break;
    	}
 	 }
	}
}

/*
 * fake driver for serial output in kiss mode
 */
static short kiss_out(CntrlParam *pb,DCtlPtr de,short op,void *extra)
{
	register IOParam *ipb= (IOParam *)pb;
	sfk_prt_pt p=extra;
	switch(op) {
	case sh_OPEN:
		if(sfk_open_me(de))
			return ioErr;
		break;
	case sh_CLOSE:
		sfk_close_me(de);
		break;
	case sh_PRIME:
		if(p->sfk_IVAR(online))
			kiss_write_out(p,ipb->ioReqCount,(unsigned char*)ipb->ioBuffer);
		break;
	default:
		// DBO_fprintf((&p->dbo,"kiss_out csCode=%d\n",pb->csCode));
		break;
	}
	return 0;
}

/*
 * return an estimate of the number of bytes to read
 */
static long kiss_bytes_available(sfk_prt_pt p)
{
	long result=0;
	sfk_packet_pt pak;
	if(!p->sfk_IVAR(online))	//if not online, no data to read
		return 0;
	pak=kiss_refill_pak(p);		//get current packet
	if(pak==0)
		return 0;			//no packet so no data
	if(p->K_IN.kp_state==KWP_LEAD_SYNC)
		result+=2;				//count sync and port
	result+=(pak->cnt-p->K_IN.kp_offset)+1;
	return result; 				//return amount of unread data
}

/*
 * init the variables in a kiss port
 */
static void kiss_clear_port(kiss_port_pt k)
{
	k->kp=0;
	k->kp_state=0;
	k->kp_offset=0;
	k->kp_transpose=0;
}

/*
 * kiss_note_offline - tell kiss that the port went offline
 */
void kiss_note_offline(sfk_prt_pt p)
{
	kiss_clear_port(&p->K_IN);
	kiss_clear_port(&p->K_OUT);
}

/*
 * read from packet queue
 */
static short kiss_in(CntrlParam *pb,DCtlPtr de,short op,void *extra)
{
	short csCode=pb->csCode;
	register IOParam *ipb= (IOParam *)pb;
	sfk_prt_pt p=extra;
	switch(op) {
	case sh_OPEN:
		if(sfk_open_me(de))
			return ioErr;
		break;
	case sh_CLOSE:
		sfk_close_me(de);
		break;
	case sh_STATUS:
		if(csCode==2) { //SetGetBuf
			long *iocnt= (long*)&pb->csParam;
			*iocnt=kiss_bytes_available(p);
		} else if(csCode==8) { //SetStatus
			SerStaRec st;
			memset(&st,0,sizeof(st));
			st.rdPend= (kiss_bytes_available(p)>0);
			memcpy(&pb->csParam,&st,5);	//6? instead of 5?
		}
		break;
	case sh_PRIME:
		p->K_IN.kp_pb=(IOParam *)pb;
		return kiss_read(p,FALSE);
	case sh_CONTROL:
		if (csCode==1)
			sfk_mark_read_done(p,-27);
		else if (csCode==accRun)
			kiss_read(p,TRUE);
		break;
	default:
		// DBO_fprintf((&p->dbo,"kiss_in op=%d csCode=%d\n",op,pb->csCode));
		break;
	}
	return 0;
}

/*
 * turn on kiss mode on the passed port
 */
static void kiss_on(sfk_iio_pt cmd,sfk_prt_pt p)
{
	if(!p->sfk_IVAR(online))
		sfk_parse_fail(cmd,TEXT_NUM(26),0);
	sfk_allocate_soft_tnc_memory(cmd,p);
	p->K_OUT.kp_transpose=FALSE;
	sfk_fake_serial_install(cmd,p,
		&p->K_IN,(long)&kiss_in,
		&p->K_OUT,(long)&kiss_out);
	p->sfk_IVAR(kiss)=TRUE;
}

/*
 * turn off kiss mode on the passed port
 */
static void sfk_kiss_off(sfk_iio_pt cmd,sfk_prt_pt p)
{
	p->sfk_IVAR(kiss)=FALSE;
	sfk_soft_tnc_install(cmd,p);
}

/*
 * change kiss mode to on or off
 */
void sfk_change_kiss(sfk_iio_pt cmd,int new_setting)
{
	if(cmd->pqx.p->sfk_IVAR(kiss)==new_setting)
		return;
	if(cmd->pqx.p->pnum>1)
	  sfk_parse_fail(cmd,TEXT_NUM(27),0);
	if(new_setting)
		kiss_on(cmd,cmd->pqx.p);
	else
		sfk_kiss_off(cmd,cmd->pqx.p);
}
