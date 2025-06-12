/*
 *
 * mac mail check network checking module
 * by Aaron Wohl (aw0g+@andrew.cmu.edu) jul 1990
 * Carnegie-Mellon University
 * Pittsburgh, PA 15213-3890
 * (412)-268-5032
 *
 */
#ifdef NYI

#include <string.h>
#include "sfk_core.h"
#include "sfk_notify.h"
#include "sfk_core_private.h"
#include "sfk_os_preserve.h"
#include <SoundDvr.h>
#include "sfk_config.h"

static note_vars nv;

/*
 * rememer a private copy of text for use by the notification mgr
 */
LDEF(char *remember(char *text))
{
	static char buf[256];
	int len=strlen(text);
	char *dst=buf+1;
	len=imax(0,imin(250,len));
	strcpy(buf+1,"* ");
	dst+=strlen(dst);
	if(len!=0) {
		memcpy(dst,text,len);
		dst+=len;
		*dst= 0;
	}
	buf[0]=strlen(buf+1);
	return buf;
}

/*
 * get and detach a resource if there is space
 */
LDEF(char **get_and_detach(long reskind,int resid,int *goterror))
{
	char **result;
	long res_size;
	result=GetResource(reskind,resid);
	SetResLoad(TRUE);
	if(result==0) {
		  *goterror= TRUE;
	  	  strcpy(sfk_gl.cs.sfk_text_state,
	  	  	"New MailCheck configuration set, but can't find your sound resource.");
		return 0;
	}
	res_size=SizeResource(result);
#ifdef XXXX
Multifinder seems to expand the system heap when it wants so this isn't accurate.
	if(res_size>40) {
		long max_free=MaxBlock();
		if((max_free-res_size)<SYS_RESERVE) {
		  *goterror= TRUE;
		  ReleaseResource(result);
	  	  strcpy(sfk_gl.cs.sfk_text_state,
	  	  	"New MailCheck configuration set, but not enough space for sound in system heap.");
		  return 0;	
		}
	}
#endif
	LoadResource(result);
	if(ResError()!=0) {
		ReleaseResource(result);
		return 0;
	}
	DetachResource(result);
	return result;
}

/*
 * get and detach a resource into the system zone if there is space
 */
LDEF(char **get_res_into_sys(long reskind,int resid,int *goterror))
{
	register char **result;
	result=(char **)OSP_protected_call(OSP_sys|OSP_noload,get_and_detach,reskind,resid,goterror);
	return result;
}

LDEF(void sfk_uninit_sound(void))
{
	if(nv.snd_handle!=0) {
		DisposHandle(nv.snd_handle);
		nv.snd_handle=0;
	}
}

int sfk_note_sound_init(int new_sound_res_num)
{
	int got_error=0;
	if(nv.sicn_handle[sfk_note_mail]==0)
	  nv.sicn_handle[sfk_note_mail]=get_res_into_sys('SICN',-4064,&got_error);
	if(nv.sicn_handle[sfk_note_error]==0)
	  nv.sicn_handle[sfk_note_error]=get_res_into_sys('SICN',-4063,&got_error);
	if(new_sound_res_num!=sfk_gl.cs.sfk_sound_id) {
	  sfk_uninit_sound();
	  if(gFLSET(MCS_nsound)&&(nv.snd_handle==0))
	    nv.snd_handle=get_res_into_sys('snd ',new_sound_res_num,&got_error);
	}
	sfk_gl.cs.sfk_sound_id=new_sound_res_num;
	return got_error;
}

/*
 * remove a notification event
 */
LDEF(pascal void notify_done_routine(struct NMRec *arec))
{
	if(arec==0)
		DebugStr("pMailCheck nremove NIL");
	if(arec->nmRefCon==0)return;
	NMRemove(((QElemPtr)arec));
	arec->nmRefCon=0;
}

/*
 * install a notification event
 */
LDEF(int sfk_note_install(
	char *text,
	Handle sound,
	Handle sicn,
	struct NMRec *arec))
{
	int err;

	if(arec==0)
		DebugStr("pMailCheck ninstall NIL");
	if(!gFLSET(MCS_nblink))
		sicn=0;
	if(!gFLSET(MCS_nsound)||(SdVolume==0))
		sound=0;
	if(gFLSET(MCS_nsound)&&(SdVolume==0))
		SysBeep(30);			/*flash the title bar if can't speak*/

	if(arec->nmRefCon!=0) {
		if((sound==0)&&(text==0)&&(sicn==arec->nmSIcon))
			return;			/*already doing this*/
		notify_done_routine(arec);
	}

	if((sound==0)&&(text==0)&&(sicn==0))
		return;

	memset(arec,0,sizeof(*arec));
	arec->qType=nmType;
	arec->nmSound=sound;
	arec->nmStr=(StringPtr)text;
	arec->nmSIcon=sicn;
	if(sicn==0)
		arec->nmResp= ((ProcPtr)&notify_done_routine);
	arec->nmRefCon=1;

	err=NMInstall(((QElemPtr)arec));
	if(err!=0) {
		arec->nmRefCon=0;
		SysBeep(30);
		return FALSE;
	}
	return TRUE;
}

/*
 * insert a notify alert into the system queue
 */
LDEF(void sfk_note_post(Handle sound))
{
	char *text=0;
	if(sound==0)
		sound=((Handle)-1);
	if(!gFLSET(MCS_nsound))
		sound=0;

	if(gFLSET(MCS_npost)) {
		textify_error();
		text=remember(sfk_gl.cs.sfk_text_state);
	}

	sfk_note_install(text,sound,0L,&nv.post_note_rec);
}

LDEF(void remind_error(void))
{
  sfk_note_install(0L,0L,
	nv.sicn_handle[sfk_note_error],
	&nv.blink_note_rec);
  sfk_note_post(0);
  set_event(EV_remind,remind_error,sfk_gl.cs.sfk_remind_time);
}

LDEF(void remind_mail(void))
{
  sfk_note_install(0L,0L,
	nv.sicn_handle[sfk_note_mail],
	&nv.blink_note_rec);
  sfk_note_post(nv.snd_handle);
  set_event(EV_remind,remind_mail,sfk_gl.cs.sfk_remind_time);
}

void set_event(int evnum,void (*dowhat)(),long dowhen)
{
	register sfk_event_pt anev= &sfk_gl.cps.ev[evnum];
	anev->ev_time=dowhen+TickCount();
	anev->ev_event=dowhat;
}

/*
 * uninitialize the notification system
 */
void sfk_note_done(void)
{
	notify_done_routine(&nv.post_note_rec);
	notify_done_routine(&nv.blink_note_rec);
    set_event(EV_remind,EV_NOTHING,0);
    set_event(EV_error,EV_NOTHING,0);
}

void sfk_note_new_error(void)
{
  if(sfk_gl.cps.ev[EV_remind].ev_event==remind_error)
    return;
  set_event(EV_remind,remind_error,0);
}

void sfk_note_old_mail(void)
{
  if(sfk_gl.cps.ev[EV_remind].ev_event!=remind_mail)
  	set_event(EV_remind,remind_mail,MIN_REMIND);
}

void sfk_note_no_error(void)
{
  sfk_gl.cs.sfk_last_err_kind=MCE_NOERRR;
  if(sfk_gl.cps.ev[EV_remind].ev_event==remind_error)
    sfk_note_done();
}

void sfk_note_new_mail(void)
{
  set_event(EV_remind,remind_mail,0);
}

void sfk_note_no_mail(void)
{
  if(sfk_gl.cps.ev[EV_remind].ev_event==remind_mail)
    sfk_note_done();
}

void sfk_note_init(void)
{
	memset(&nv,0,sizeof(nv));
}

void sfk_note_uninit()
{
	register int i;
	sfk_note_done();
	for(i=0;i<sfk_note_kinds;i++)
	  if(nv.sicn_handle[i]!=0) {
		DisposHandle(nv.sicn_handle[i]);
		nv.sicn_handle[i]=0;
	  }
	sfk_uninit_sound();
	sfk_note_init();
}
#endif NYI