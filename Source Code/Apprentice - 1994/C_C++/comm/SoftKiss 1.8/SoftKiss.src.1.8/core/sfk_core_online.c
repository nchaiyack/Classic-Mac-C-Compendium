/*
 * SoftKiss command parser
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core.h"
#include "sfk_core_private.h"
#include "sfk_core_some_macs.h"

/*
 * compute baud rate.
 */
static uint16 compute_br_generator_delay(sfk_iio_pt cmd,sfk_prt_pt p,int32 baud,int32 div)
{
	long effective_buad=baud*div;
	long divider=(effective_buad*2L);
	long result=0;
	if((effective_buad!=0)&&(divider!=0))
	  result= ((p->sfk_IVAR(data_clock_rate)+effective_buad)/divider)-2L;
	if((result==0)||(effective_buad==0)||(divider==0))
		sfk_parse_fail(cmd,TEXT_NUM(28),0);
	return result;
}

/*
 * take size user wanted, round up for allignment add add space
 * for the header record
 */
static void set_physical_bytes_needed_for_given_packet_size(sfk_prt_pt p,long request_size)
{
#define SFK_SHARE_COUNT_OVERHEAD (4)
	request_size+=3;		/*round up to a multiple of 4 bytes*/
	request_size&= ~3;
	request_size+=SFK_SHARE_COUNT_OVERHEAD;
//	request_size+=sizeof(sfk_packet);	/*account for packet header*/
	p->sfk_IVAR(packet_phys_size)=request_size;
}

/*
 * relase memory block used by queue, if any
 */
static sfk_release_queue_memory(sfk_prt_pt p)
{
	/*if and old queue existed forget it*/
	if(p->sfk_IVAR(queue_mem)==0) return;
	if(p->sfk_IVAR(queue_no_swap))
		sfk_unlock_if_vm_implimented((void *)(p->sfk_IVAR(queue_mem)),p->sfk_IVAR(queue_mem_phys_size));
	sfk_free((void*)(p->sfk_IVAR(queue_mem)));
	p->sfk_IVAR(queue_mem)=0;
	p->sfk_CHANGED(header_queue_size)=TRUE;
	p->sfk_CHANGED(data_queue_size)=TRUE;
}

/*
 * allocate the storage that will become the packet queues
 */
void sfk_allocate_queue_memory(sfk_iio_pt cmd,sfk_prt_pt p)
{
	if((p->sfk_IVAR(header_queue_size)<10)||
		(p->sfk_IVAR(data_queue_size)<10))
		sfk_parse_fail(cmd,TEXT_NUM(29),0);
	if(p->sfk_IVAR(max_packet_size)<100)
		sfk_parse_fail(cmd,TEXT_NUM(30),0);
	set_physical_bytes_needed_for_given_packet_size(p,p->sfk_IVAR(max_packet_size));
	sfk_release_queue_memory(p);	/*forget old queue*/
	p->sfk_IVAR(queue_mem_phys_size)=(p->sfk_IVAR(packet_phys_size)*p->sfk_IVAR(data_queue_size));
	p->sfk_IVAR(queue_mem_phys_size)+=p->sfk_IVAR(header_queue_size)*sizeof(sfk_packet);
/*
 * if we are a device driver then allocate the packet queue in the system
 * heap.  if we are a program put it in the application heap.
 * under multifinder it is hard to preflight the success of this allocation
 * in the system heap as it may be able to expand.  perhaps the gestalt
 * function for this could be checked and if the system heap doesn't
 * expand see if there is room.  also we need to figure out what will hapen
 * if this allocation fails.
 */
	p->sfk_IVAR(queue_mem)= (long)sfk_malloc(p->sfk_IVAR(queue_mem_phys_size));
	if(p->sfk_IVAR(queue_mem)==0)
		sfk_parse_fail(cmd,TEXT_NUM(31),0);
	p->sfk_IVAR(queue_no_swap)=
		sfk_lock_if_vm_implimented((void *)(p->sfk_IVAR(queue_mem)),p->sfk_IVAR(queue_mem_phys_size));
	p->sfk_CHANGED(queue_mem)=TRUE;		/*mark as changed to rebuild queues*/
	p->sfk_CHANGED(header_queue_size)=FALSE;	/*memory is allocated ok now*/
	p->sfk_CHANGED(data_queue_size)=FALSE;	/*memory is allocated ok now*/
	p->sfk_CHANGED(max_packet_size)=FALSE;
}

/*
 * initialize the queues from the raw block of queue memory
 */
static void initialize_queues(sfk_iio_pt cmd,sfk_prt_pt p)
{
	int i;
	char *space_allocator= (void *)(p->sfk_IVAR(queue_mem));
	sfk_init_queue(&p->sfk_freehq);
	sfk_init_queue(&p->sfk_freedq);
	sfk_init_queue(&p->sfk_xmitq);
	sfk_init_queue(&p->sfk_recvq);
	/*cut up all the space for packets and put them on the free queue*/
	for(i=0;i<p->sfk_IVAR(data_queue_size);i++ ) {
		sfk_enqueue(&p->sfk_freedq,(sfk_packet_pt)space_allocator);	/*put new packet on free queue*/
		space_allocator+=p->sfk_IVAR(packet_phys_size); /*step to next packet*/
	}
	for(i=0;i<p->sfk_IVAR(header_queue_size);i++ ) {
		sfk_packet_pt ap= (void *)space_allocator;	/*allocate the packet header*/
		space_allocator+=sizeof(sfk_packet); /*step to next packet*/
		ap->portno=p->pnum;				/*mark which port owns the packet*/
		sfk_enqueue(&p->sfk_freehq,ap);	/*put new packet on free queue*/
	}
	p->sfk_in=0;		/*no current packet comming in*/
	p->sfk_out=0;		/*no current packet going out*/
 	sfk_prep_rx(p);
}

/*
 * punt bogus time values
 */
static void check_os_delay(
	sfk_iio_pt cmd,
	sfk_prt_pt p,
	int32 a_delay)
{
#define TX_FAIL_SAFE (20)	/*sanity check in seconds*/
	if((a_delay<0)||(a_delay>(TX_FAIL_SAFE*1000000)))
		sfk_parse_fail(cmd,TEXT_NUM(34),0);
}

/*
 * figure delay depending on what the operating system supports
 */
static long compute_os_delay(
	sfk_prt_pt p,
	int32 a_delay)
{
	if(time_manager_version()<2)
		return (a_delay+999)/1000;	/*time mgr only does usecs*/
	return -a_delay;
}

/*
 * recompute values that depend on other variables
 */
void sfk_recompute_derived_nums(sfk_prt_pt p)
{
	p->sfk_IVAR(os_tx_delay)=
		compute_os_delay(p,p->sfk_IVAR(tx_delay));
	p->sfk_IVAR(os_tail_delay)=
		compute_os_delay(p,p->sfk_IVAR(tail_delay));
	p->sfk_IVAR(os_xmit_slottime)=
		compute_os_delay(p,p->sfk_IVAR(xmit_slottime));
	p->sfk_IVAR(os_xmit_dwait)=
		compute_os_delay(p,p->sfk_IVAR(xmit_dwait));
}

/*
 * set the interface online
 */
static void sfk_go_online(sfk_iio_pt cmd,sfk_prt_pt p)
{
	if(!p->sfk_IVAR(soft_tnc))
	  sfk_parse_fail(cmd,TEXT_NUM(35),0);
	if(p->sfk_CHANGED(header_queue_size)||
	   p->sfk_CHANGED(data_queue_size)||
	   p->sfk_CHANGED(max_packet_size))
		sfk_allocate_queue_memory(cmd,p);
	initialize_queues(cmd,p);
	p->sfk_IVAR(recv_rate_divider)=
		compute_br_generator_delay(cmd,p,p->sfk_IVAR(recv_speed),32);
	p->sfk_IVAR(xmit_rate_divider)=
		compute_br_generator_delay(cmd,p,p->sfk_IVAR(xmit_speed),1);
	check_os_delay(cmd,p,p->sfk_IVAR(tx_delay));
	check_os_delay(cmd,p,p->sfk_IVAR(tail_delay));
	check_os_delay(cmd,p,p->sfk_IVAR(xmit_slottime));
	check_os_delay(cmd,p,p->sfk_IVAR(xmit_dwait));
	sfk_recompute_derived_nums(p);
	if(!have_scc())
	  sfk_parse_fail(cmd,TEXT_NUM(36),0);
	sfk_init_register_addresses(p);
	power_up_port(p->pnum,p->sfk_IVAR(ignore_internal_modem));
	p->extra_tx_bits_for_line_power=0;
	if(p->sfk_IVAR(line_powered_modem))
		p->extra_tx_bits_for_line_power=0x0a;
  sfk_SCC_interupts_off;
	sfk_go_state(p,PX_RX);
  sfk_SCC_interupts_on;
}

/*
 * set the interface offline
 */
void sfk_go_offline(sfk_prt_pt p)
{
	kiss_note_offline(p);
  sfk_SCC_interupts_off;
  	sfk_go_state(p,PX_OFF);
 	sfk_release_queue_memory(p);	/*forget old queue*/
  sfk_SCC_interupts_on;
	power_down_port(p->pnum);
}

/*
 * go online or go offline
 */
void sfk_change_online(sfk_iio_pt cmd,int new_setting)
{
	if(cmd->pqx.p->sfk_IVAR(online)==new_setting)
		return;
	if(cmd->pqx.p->pnum>1)
	  sfk_parse_fail(cmd,TEXT_NUM(37),0);
	if(new_setting)
		sfk_go_online(cmd,cmd->pqx.p);
	else
		sfk_go_offline(cmd->pqx.p);
}

/*
 * shutdown everything
 */
void sfk_shutdown(sfk_iio_pt cmd)
{
  int i;
  for(i=0;i<sfk_NUM_PORTS;i++) {
    cmd->pqx.p=sfk_PN(i);
    sfk_change_online(cmd,FALSE);
    sfk_change_soft_tnc(cmd,FALSE);
  }
}
