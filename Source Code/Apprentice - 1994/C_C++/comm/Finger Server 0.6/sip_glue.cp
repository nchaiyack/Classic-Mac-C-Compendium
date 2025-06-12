/*
 * tcp io routines to for finger server
 * by Aaron Wohl n3liw+@cmu.edu
 * 412-268-5032 / 412-731-3691
 */

#include <stdlib.h>
#include <string.h>
#include "sip_glue.h"
#include "sip_interface.h"

static struct {
	tcp_conv_pt  all_tcp_convs;
	int refnum;
	process_wakeup_control server_proc;
} ip_gl;

/*
 * wake the passed process
 */
static void tcp_wake(process_wakeup_control_pt wake_me)
{
	if(wake_me->tcp_wake_magic_num!=TCP_WAKE_MAGIC_NUM)
		Debugger();
	wake_me->can_sleep=FALSE;
	if(!wake_me->need_wake_up)
		return;
	// wake waiting process so it can handle the event
	if(WakeUpProcess(&wake_me->server))
		wake_me->need_wake_up=FALSE;
}

/*
 * io compleation proc to awake the server process if needed
 */
static pascal void tcp_io_compleation_proc(void)
{
	asm {
		movem.l	a0-a2/a5/d0-d2,-(sp)
		move.l	tcp_side.sconv(a0),a0
		move.l	tcp_conv.server_proc(a0),a0
		move.l	process_wakeup_control.a5(a0),a5
		move.l 	a0,-(sp)
		bsr		tcp_wake
		move.l	(sp)+,a0			//remove the argument
		movem.l	(sp)+,a0-a2/a5/d0-d2
	}
}

/*
 * Do a tcp io operation on a connection
 */
static int tcp_do_tio_any(tcp_conv_pt sconv,TCPiopb *ipb,int is_async)
{
	int iErr;
    ipb->ioCRefNum = ip_gl.refnum;
    ipb->tcpStream = sconv->tcpStream;
	iErr=PBControl((ParmBlkPtr)ipb,is_async);
	if(iErr>=0)
		return FALSE;
	tcp_release_connection(sconv);
	return TRUE;
}

/*
 * Do a tcp io operation on a connection
 */
static int tcp_do_tio(tcp_conv_pt sconv,TCPiopb *ipb)
{
	return tcp_do_tio_any(sconv,ipb,FALSE);
}

/*
 * Do a tcp io operation on a connection
 */
static int tcp_do_tio_async(tcp_conv_pt sconv,side_pt aside)
{
	aside->sconv=sconv;
	aside->ipb.ioCompletion= (void*)&tcp_io_compleation_proc;
	return tcp_do_tio_any(sconv,&aside->ipb,TRUE);
}

/*
 * passive connection to listen for incomming tcp connections
 */
int tcp_passive_connect(tcp_conv_pt sconv,tcp_port port_number)
{
    memset(&sconv->write_side.ipb,0,sizeof(sconv->write_side.ipb));
    sconv->write_side.ipb.csParam.open.localPort = port_number;
   	sconv->write_side.ipb.csParam.open.userDataPtr = (char*)sconv;
   	sconv->write_side.ipb.csCode = TCPPassiveOpen;
    if(tcp_do_tio_async(sconv,&sconv->write_side))
    	return TRUE;
	sconv->state=ST_LISTEN;
	return FALSE;
}

#include <stdio.h>

/*
 * setup one stream
 */
int tcp_init_stream(tcp_conv_pt sconv)
{
	memset(sconv,0,sizeof(*sconv));
	if(tcp_create_stream(sconv))
		return TRUE;
	if(sconv->tcpStream==0)
		return TRUE;
	return tcp_passive_connect(sconv,TCP_FINGER_PORT);
}

/*
 * just woke up and will service any requests
 */
void tcp_just_awoke(void)
{
	ip_gl.server_proc.need_wake_up=FALSE;
	ip_gl.server_proc.can_sleep=TRUE;
}

/*
 * did any new events happens since we last awoke?
 */
int tcp_ok_to_sleep_now(void)
{
	ip_gl.server_proc.need_wake_up=TRUE;		//in case interupt hits after can_sleep check
	return ip_gl.server_proc.can_sleep;
}

/*
 * init system to wake us up when async io finishes
 */
static int tcp_init_wakeup(void)
{
	// get serial num so it can wake us later
	if(GetCurrentProcess(&ip_gl.server_proc.server))
		return TRUE;
	ip_gl.server_proc.need_wake_up=FALSE;
	ip_gl.server_proc.can_sleep=FALSE;
	ip_gl.server_proc.a5=CurrentA5;
	ip_gl.server_proc.tcp_wake_magic_num=TCP_WAKE_MAGIC_NUM;
	return FALSE;
}

/*
 * setup all the streams
 */
int tcp_init_streams(void)
{
 	int i;
 	if(tcp_init_wakeup())
 		return TRUE;
 	for(i=0;i<MAX_NUM_CONVS;i++)
 		tcp_init_stream(ip_gl.all_tcp_convs+i);
 	return FALSE;
}

/*
 * clean up all the streams
 */
void tcp_release_streams(void)
{
 	int i;
 	for(i=0;i<MAX_NUM_CONVS;i++) {
 		tcp_conv_pt astrm=ip_gl.all_tcp_convs+i;
		tcp_release_connection(astrm);
 	}
}

/*
 * handle action on each tcp connection
 */
void tcp_service_event(void)
{
 	int i;
 	for(i=0;i<MAX_NUM_CONVS;i++) {
 		tcp_conv_pt astrm=ip_gl.all_tcp_convs+i;
 		if(astrm->state==ST_FREE)
 			tcp_init_stream(astrm);
 		if(astrm->state==ST_FREE)
 			continue;
  		if(astrm->terminating || (astrm->write_side.ipb.ioResult<0)) {
 			tcp_release_connection(astrm);
 			continue;
 		}
		if(astrm->data_arrived)
 			tcp_recv_packet(astrm);
 		if(astrm->write_side.ipb.ioResult==1)
 			continue;
 		if(astrm->state==ST_LISTEN) {
 			get_finger_info(astrm->mactcp_send_buffer,
 				astrm->mactcp_send_buffer+(sizeof(astrm->mactcp_send_buffer)-1000));
 			if(tcp_send_packet(astrm))
 				continue;
 			astrm->state=ST_SEND;
 		}
 		if((astrm->write_side.ipb.ioResult!=1)&&(astrm->state==ST_SEND)) {
  			if(tcp_close(astrm))
  				continue;
  			astrm->state=ST_CLOSE;
  		}
  		if((astrm->write_side.ipb.ioResult!=1)&&(astrm->state==ST_CLOSE))
 			tcp_release_connection(astrm);
 	}
}

/*
 * handle incomming data or connection drop
 */
static pascal void myTCPNotifyProc(StreamPtr tcpStream,
	unsigned short eventCode,
	Ptr userDataPtr,
	unsigned short terminReason,
	struct ICMPReport *icmpMsg)
{
	register tcp_conv_pt acnv=(tcp_conv_pt)userDataPtr;

	acnv->last_event=eventCode;
	acnv->service_me=TRUE;

	switch (eventCode) {
		case TCPTerminate:
			acnv->terminating++;
			break;
		case TCPClosing:
			acnv->closing++;
			break;
		case TCPULPTimeout:
			break;
		case TCPDataArrival:
			acnv->data_arrived++;
			break;
		case TCPUrgent:
			break;
		case TCPICMPReceived:
			break;
		default:	
			break;
		}
	tcp_wake(acnv->server_proc);
}

/*
 * release a tcp stream, aborting any io in progress
 */
void tcp_release_connection(tcp_conv_pt sconv)
{
	if(sconv->tcpStream!=0) {
		int err;
		TCPiopb ipb;    	/* io parameters for current io */
		memset(&ipb,0,sizeof(ipb));
		ipb.csCode = TCPRelease;
		ipb.ioCRefNum = ip_gl.refnum;
		ipb.tcpStream = sconv->tcpStream;
		ipb.csParam.globalInfo.userDataPtr= (char*)sconv; //use globalInfo, has same offset
		err=PBControl((ParmBlkPtr)&ipb, FALSE);
		if(err!=0)
			DebugStr("\pError releasing a TCP stream");
	}
	memset(sconv,0,sizeof(*sconv));
}

/*
 * allocate io buffers
 * return true on error
 */
int tcp_allocate_memory(void)
{
	if(ip_gl.all_tcp_convs!=0)
		return FALSE;
	ip_gl.all_tcp_convs=(tcp_conv_pt)NewPtrClear(sizeof(*ip_gl.all_tcp_convs)*MAX_NUM_CONVS);
	if(ip_gl.all_tcp_convs==0)
		return TRUE;
	return FALSE;
}

/*
 * open the mactcp device driver
 * returns true on an error
 */
int tcp_open_driver(void)
{
	int err;
    TCPiopb pb;
	if(ip_gl.refnum!=0)
		return TRUE;
    memset(&pb,0,sizeof(pb));

    pb.ioNamePtr = (char*)"\p.IPP";
    err=PBOpen((ParmBlkPtr)&pb,FALSE);
    if(err!=0)
    	return TRUE;
    ip_gl.refnum = pb.ioCRefNum;
    return FALSE;
}

/*
 * create a tcp stream
 */
int tcp_create_stream(tcp_conv_pt uptr)
{
    TCPiopb ipb;
	uptr->server_proc= &ip_gl.server_proc;
    memset(&ipb,0,sizeof(ipb));
    ipb.csParam.create.rcvBuff = (Ptr)(uptr->mactcp_recv_buffer);
    ipb.csParam.create.rcvBuffLen = MACTCP_RECV_BUFFER_LEN;
    ipb.csParam.create.notifyProc = myTCPNotifyProc;
	ipb.csParam.create.userDataPtr = (Ptr)uptr;

    ipb.csCode = TCPCreate;
    if(tcp_do_tio(uptr,&ipb))
    	return TRUE;
    uptr->tcpStream=ipb.tcpStream;
    return FALSE;
}

/*
 * is any incomming data available on this connection
 */
static int tcp_is_data_available(tcp_conv_pt sconv)
{
    TCPiopb ipb;    			/* io parameters for current io */
	if(sconv->data_arrived==0)
		return FALSE;
	sconv->data_arrived=0;		/*we will notice it*/

    memset(&ipb,0,sizeof(ipb));
    ipb.csCode = TCPStatus;
	ipb.csParam.status.userDataPtr= (char*)sconv;
    if(tcp_do_tio(sconv,&ipb))
    	return FALSE;

	if (ipb.csParam.status.amtUnreadData <= 0)	
		return FALSE;

	sconv->data_arrived++;
	return TRUE;
}

/*
 * close a tcp stream
 */
int tcp_close(tcp_conv_pt uptr)
{
	memset(&uptr->write_side.ipb,0,sizeof(uptr->write_side.ipb));
	uptr->write_side.ipb.csCode = TCPClose;
	
   	if(tcp_do_tio_async(uptr,&uptr->write_side))
    	return TRUE;
    return FALSE;
}

/*
 * transfer outgoing packets into MacTCP output buffer
 */
int tcp_send_packet(tcp_conv_pt sconv)
{
	memset(&sconv->write_side.ipb,0,sizeof(sconv->write_side.ipb));
    sconv->write_side.awds[1].length = 0;
	sconv->write_side.awds[1].ptr="mumble";
	sconv->write_side.awds[0].length = (short)strlen(sconv->mactcp_send_buffer);
	sconv->write_side.awds[0].ptr=sconv->mactcp_send_buffer;

	sconv->write_side.ipb.csParam.send.wdsPtr  =(Ptr)&sconv->write_side.awds;
	sconv->write_side.ipb.csParam.send.ulpTimeoutValue=SEND_TIMEOUT;
	sconv->write_side.ipb.csParam.send.validityFlags=timeoutValue;
	sconv->write_side.ipb.csParam.send.pushFlag=TRUE;
	sconv->write_side.ipb.csParam.send.userDataPtr= (char*)sconv;
	sconv->write_side.ipb.csCode = TCPSend;
   	if(tcp_do_tio_async(sconv,&sconv->write_side))
    	return TRUE;
 	return FALSE;
}

#define NUM_RDS_ENTIRES (1)

/*
 * recieve data on this connection
 * the data is left in an internal mactcp buffer which must be returned to mactcp
 * with the returnTCPbuffer proceedure
 */
int tcp_recv_packet(tcp_conv_pt sconv)
{
    TCPiopb 	ipb;    			/* io parameters for current io */
    wdsEntry   	awds[NUM_RDS_ENTIRES];

    if (sconv->data_arrived <= 0)
		return FALSE; 			/*no data*/

    memset(&awds,0,sizeof(awds));
    memset(&ipb,0,sizeof(ipb));
   	ipb.csParam.receive.commandTimeoutValue=1;		//abort as soon as possible on an error
    ipb.csParam.receive.rdsPtr=(Ptr)&awds;
    ipb.csParam.receive.rdsLength = NUM_RDS_ENTIRES;
	ipb.csParam.receive.userDataPtr= (char*)sconv;
    ipb.csCode = TCPNoCopyRcv;
    if(tcp_do_tio(sconv,&ipb))
    	return FALSE;
	if(ipb.csParam.receive.rdsLength>0) {
		if((ipb.csParam.receive.rdsLength>0) &&
			(awds[0].length==3) &&
			(memcmp(awds[0].ptr,"die",3)==0))
				sip_keep_running=FALSE;
	   	ipb.csCode = TCPRcvBfrReturn;
		if(tcp_do_tio(sconv,&ipb))
	    	return FALSE;
	}

	sconv->data_arrived--;
    return TRUE;
}
