/*
 * add/remove packets to a queue
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core.h"
#include "sfk_core_private.h"

struct data_header_R {
	uint16 pnum;		/*home port*/
	uint16 ref_cnt;		/*number of sharers*/
};
typedef struct data_header_R data_header,*data_header_pt;


/*
 * free a packet header and packet
 * may be called with interupts on
 */
void sfk_free_packet_protected(sfk_prt_pt p,sfk_packet_pt free_me)
{
    sfk_SCC_interupts_off;
	sfk_free_packet(p,free_me);
	sfk_SCC_interupts_on;
}

/*
 * free a packet header and packet
 */
void sfk_free_packet(sfk_prt_pt p,sfk_packet_pt free_me)
{
	data_header_pt dh= (data_header_pt)(free_me->data-sizeof(data_header));
	sfk_enqueue(&p->sfk_freehq,free_me);
	dh->ref_cnt--;
	if(dh->ref_cnt > 0)
 {		SFK_ASSERT(FALSE,SFK_TEXT(63));
		return;	/*data is still being shared*/
}
	if((dh->pnum<0)||(dh->pnum>=sfk_NUM_PORTS))
		SFK_ASSERT(FALSE,SFK_TEXT(63));
	p=sfk_PN(dh->pnum);				/*data goes back to home port*/
	sfk_enqueue(&p->sfk_freedq,(sfk_packet_pt)dh);
}

/*
 * copy a packet header and share a packet
 */
sfk_packet_pt sfk_copy_packet(sfk_prt_pt p,sfk_packet_pt copy_me)
{
	data_header_pt dh= (data_header_pt)(copy_me->data-sizeof(data_header));
	register sfk_packet_pt result=sfk_dequeue(&p->sfk_freehq);
	if(result==0)
		return 0;
	*result=*copy_me;
	dh->ref_cnt++;
	dh++;
	result->data=(uint8*)dh;
	return result;
}

/*
 * allocate a packet header and packet
 * may be called with interupts on
 */
sfk_packet_pt sfk_allocate_protected(sfk_prt_pt p)
{
	register sfk_packet_pt result;
    sfk_SCC_interupts_off;
	result=sfk_allocate_packet(p);
	sfk_SCC_interupts_on;
	return result;
}

/*
 * allocate a packet header and packet
 */
sfk_packet_pt sfk_allocate_packet(sfk_prt_pt p)
{
	register sfk_packet_pt result;
	register data_header_pt data_result;
	if((p->sfk_freehq.sfk_size==0)||
	   (p->sfk_freedq.sfk_size==0))
	   	return 0;
	result=sfk_dequeue(&p->sfk_freehq);
	data_result= (data_header_pt)sfk_dequeue(&p->sfk_freedq);
	data_result->pnum=p->pnum;
	data_result->ref_cnt=1;		/*one header shareing this data*/
	data_result++;				/*skip to start of real data*/
	result->data= (uint8*)data_result;
	return result;
}

/*
 * remove a packet from the passed queue and return it
 * returns nil if the queue was empty
 */
sfk_packet_pt sfk_dequeue_protected(sfk_queue_pt aq)
{
	register sfk_packet_pt result;
    sfk_SCC_interupts_off;
	result=sfk_dequeue(aq);
	sfk_SCC_interupts_on;
	return result;
}

/*
 * remove a packet from the passed queue and return it
 * returns nil if the queue was empty
 */
void sfk_enqueue_protected(sfk_queue_pt aq,sfk_packet_pt ap)
{
    sfk_SCC_interupts_off;
	sfk_enqueue(aq,ap);
	sfk_SCC_interupts_on;
}

/*
 * remove a packet from the passed queue and return it
 * returns nil if the queue was empty
 */
sfk_packet_pt sfk_dequeue(sfk_queue_pt aq)
{
	register sfk_packet_pt result;
	if(aq->sfk_size==0)
		return 0;
	result=aq->sfk_first;
	aq->sfk_first=result->next;
	aq->sfk_size--;
	if(aq->sfk_size<=0)
		aq->sfk_last=0;
	return result;
}

/*
 * initialize the passed queue to be empty
 */
void sfk_init_queue(sfk_queue_pt aq)
{
	memset(aq,0,sizeof(*aq));
}

/*
 * enqueue the passed packet in the passed queue
 */
void sfk_enqueue(sfk_queue_pt aq,sfk_packet_pt ap)
{
	ap->next=0;
	if(aq->sfk_size==0)
		aq->sfk_first=ap;
	else
		aq->sfk_last->next=ap;
	aq->sfk_last=ap;
	aq->sfk_size++;
}
