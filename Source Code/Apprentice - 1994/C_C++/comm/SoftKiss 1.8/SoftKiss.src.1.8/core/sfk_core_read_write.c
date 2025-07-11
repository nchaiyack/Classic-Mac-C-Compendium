/*
 * SoftKiss drivre read/write packet routines
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core.h"
#include "sfk_core_private.h"
#include <asm.h>

/*
 * queue up a packet for write and start transmit if it isn't
 * already running
 */
void sfk_enqueue_write(sfk_prt_pt p,sfk_packet_pt pak)
{
    sfk_SCC_interupts_off;
	sfk_enqueue(&p->sfk_xmitq,pak);
	sfk_initiate_xmit(p);
	sfk_SCC_interupts_on;
}

/*
 * enqueue a packet in a write queue
 */
void sfk_write(sfk_iio_pt cmd)
{
  int i;
  for(i=0;i<sfk_NUM_PORTS;i++) {
    sfk_prt_pt p=sfk_PN(i);
	sfk_packet_pt pak;
	long real_cnt;
    if(!p->sfk_IVAR(online))
    	continue;
    if(p->pnum!=cmd->uio->do_this.cknd.sfk_rw.portno)
    	continue;
	pak=sfk_allocate_protected(p);
	if(pak==0)continue;
	/*copy over packet header*/
	memcpy(pak,&cmd->uio->do_this.cknd.sfk_rw,OFFSET(sfk_packet,data));
	real_cnt=sfk_imin(cmd->uio->do_this.cknd.sfk_rw.cnt,p->sfk_IVAR(max_packet_size));
	if(real_cnt>0)
	  memcpy(pak->data,cmd->uio->do_this.cknd.sfk_rw.data,real_cnt);
	sfk_enqueue_write(p,pak);
	return;
  }
  sfk_control_fail(cmd,0,sfk_NO_SUCH_PORT);
}

/*
 * read a packet from recieve queues
 */
void sfk_read(sfk_iio_pt cmd)
{
  int i;
  for(i=0;i<sfk_NUM_PORTS;i++) {
    sfk_prt_pt p=sfk_PN(i);
	sfk_packet_pt pak;
	long real_cnt;
    if(!p->sfk_IVAR(online))
    	continue;
	pak=sfk_dequeue_protected(&p->sfk_recvq);
	if(pak==0)continue;
	/*copy over packet header*/
	memcpy(&cmd->uio->do_this.cknd.sfk_rw,pak,OFFSET(sfk_packet,data));
	real_cnt=sfk_imin(pak->cnt,p->sfk_IVAR(max_packet_size));
	if(real_cnt>0)
	  memcpy(cmd->uio->do_this.cknd.sfk_rw.data,pak->data,real_cnt);
	sfk_free_packet_protected(p,pak);
	return;
  }
  sfk_control_fail(cmd,0,sfk_NOTHING_TO_READ);
}
