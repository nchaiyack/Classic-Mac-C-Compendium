/*
 * sfk_state_change.c - handle major state changes
 */

#include "sfk_core.h"
#include "sfk_core_private.h"
#include "sfk_die.h"

/*
 * transition from off to recieve mode
 */
static void sfk_leave_off(sfk_prt_pt p)
{
	sfk_setup_scc(p);
	sfk_install_interupt_vectors(p); /*also sets recieve mode*/
	sfk_keydown(p);
	p->sfk_IVAR(online)=TRUE;
}

/*
 * key transmit
 * -interupt
 */
static void sfk_key_xmit_timer(sfk_prt_pt p)
{
	sfk_go_state(p,PX_KEY_UP);
}

/*
 * handle a transition to a new state
 */
void sfk_go_state(sfk_prt_pt p,px_states new_state)
{
	px_states old_state=p->sfk_state;
	p->sfk_state=new_state;
	p->sfk_IVAR(state_var)=new_state;
	if(new_state==old_state)
		return;	//transition to same state
	if(old_state==PX_OFF)		//leaveing off go to recieve first
		sfk_leave_off(p);
	dbo_fputc(&p->dbo,'0'+new_state);
	switch(new_state) {
	case PX_OFF:
		sfk_keydown(p);
		sfk_reset_scc(p);
		sfk_clear_timer(p);
		sfk_remove_interupt_vectors(p);
		p->sfk_IVAR(online)=FALSE;
		break;
	case PX_RX:
		sfk_keydown(p);
		sfk_set_recv_mode(p);
		break;
	case PX_RX_NOW:
		sfk_set_recv_now_mode(p);
		break;
	case PX_RX_CHUCK:
		sfk_set_recv_chuck_mode(p);
		break;
	case PX_SLOT_WAIT:
		{
			unsigned long wait_time=0;
			wait_time+=p->sfk_IVAR(os_xmit_dwait);
			if(die(PERSISTANCE_LIMIT)>p->sfk_IVAR(xmit_persist))
				wait_time+=p->sfk_IVAR(os_xmit_slottime);
			if(wait_time>0)
				sfk_install_timer(p,
					sfk_key_xmit_timer,
					wait_time);
			else
				sfk_go_state(p,PX_KEY_UP);
		}
		break;
	case PX_KEY_UP:
		sfk_tell_scc_to_xmit(p);
		break;
	case PX_XMIT:
		sfk_send_first_byte(p);
		break;
	case PX_TAIL:
		sfk_tail(p);
		break;
	default:
		SFK_ASSERT(FALSE,SFK_TEXT(52));
	}
}
