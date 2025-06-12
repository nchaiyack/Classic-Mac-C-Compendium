/*
 * Routines to handle reading and writing the prep file
 * and driver state
 *
 * by Aaron Wohl (aw0g+@andrew.cmu.edu) jul 1990
 * Carnegie-Mellon University
 * Special Projects
 * Pittsburgh, PA 15213-3890
 * (412)-268-5032
 */

#ifdef NYI

#include "sfk_core.h"
#include "sfk_prep.h"
#include "string.h"
#include "sfk_os_preserve.h"

/*
 * create the resource fork of a the Prep file
 */
LDEF(void make_pref(void))
{
	SysEnvRec info;
	short int current_directory;
	Str255 vol_name;
	int err;
	SysEnvirons(1, &info);
	GetVol(vol_name,&current_directory);
	SetVol("\p",info.sysVRefNum);
	Create(PREP_NAME,0,'sFK0','Prep');
	CreateResFile(PREP_NAME);
	SetVol("\p",current_directory);
}

/*
 * return the resource fork
 * return a system error code < 0 if can't open fork
 */
LDEF(int find_res_file(Str255 name,int permission))
{
	int result;
	int ref_num;
	SysEnvRec info;
	short int current_directory;
	Str255 vol_name;
	int err;
	SysEnvirons(1, &info);
	GetVol(vol_name,&current_directory);
#ifdef RUBBISH
	result=HOpenRF(info.sysVRefNum,0L,name,permission,&ref_num);
	if(result>=0)
		result=ref_num;
#endif
	result=OpenRFPerm(name,info.sysVRefNum,permission);
	return result;
}

/*
 * get the state contained in the Prep file in the system folder
 * returns < 0 on error (a system error code, or -1)
 * returns 0 on success
 */
LDEF(long sfk_read_prep_internal(sfk_state_pt s))
{
  int old_res_file;
  int err=0;
  int prep_res;
  sfk_state_pt **fhandle;
  old_res_file=CurResFile();	/*remember so we can put current resource file back*/
  prep_res=find_res_file(PREP_NAME,fsRdPerm); /*open the device driver resource file*/
  if(prep_res < 0)		/*find the resource file?*/
    return prep_res;
  fhandle=(sfk_state_pt **)Get1Resource('Prep',PREP_RES_NUM);
 if(fhandle==0)
  	err=ResError();
  else
  	if(GetHandleSize(fhandle)!=sizeof(sfk_state))
  		err= -1;
  	else {
  		memcpy(s,*fhandle,sizeof(*s));
  		s->sfk_st&= ~MCS_unsaveable;		/*don't restore debug state, etc*/
  		if(s->sfk_st_maj_ver!=sfk_ST_maj_ver)
  		  err= -1;
  		if(s->sfk_st_min_ver<sfk_ST_min_ver)
  		  err= -1;
  		if(s->sfk_check_num!=sfk_check_num)
  		  err= -1;
  	}
  CloseResFile(prep_res);
  UseResFile(old_res_file);	/*go back to the normal resource file*/
  return err;
}

int sfk_read_prep(sfk_state_pt s)
{
	return OSP_protected_call(OSP_same,sfk_read_prep_internal,s);
}

LDEF(long sfk_write_prep_internal(sfk_state_pt s))
{
  int old_res_file;
  int err=0;
  int prep_res;
  sfk_state_pt *fhandle;
  old_res_file=CurResFile();	/*remember so we can put current resource file back*/
  prep_res=find_res_file(PREP_NAME,fsRdWrPerm); /*open the device driver resource file*/
  if(prep_res< 0) {
    make_pref();
    prep_res=find_res_file(PREP_NAME,fsRdWrPerm); /*open the device driver resource file*/
  }
  if(prep_res < 0)		/*find the resource file?*/
    return prep_res;
  UseResFile(prep_res);	/*may already be open, force to front*/
  fhandle=(sfk_state_pt *)Get1Resource('Prep',PREP_RES_NUM);
  if(fhandle==0) {
    fhandle=(sfk_state_pt *)NewHandle(sizeof(*s));
    if(fhandle!=0)
      memcpy(*fhandle,s,sizeof(*s));
    strcpy((*fhandle)->sfk_text_state,"SavedState");
	AddResource(fhandle,'Prep',PREP_RES_NUM,"\p");
	err=ResError();
  } else {
  	 if(GetHandleSize(fhandle)!=sizeof(sfk_state)) {
  	 	asm {
  	 		move.l fhandle,a0
  	 		move.w #sizeof(sfk_state),d0
  	 		_SetHandleSize
  	 		move.w d0,err
  	 	}
  	 }
  	 if(err==0) {
     	memcpy(*fhandle,s,sizeof(*s));
     	strcpy((*fhandle)->sfk_text_state,"SavedState");
    	ChangedResource(fhandle);
     	WriteResource(fhandle);
     }
  }
  CloseResFile(prep_res);
  UseResFile(old_res_file);	/*go back to the normal resource file*/
  return err;
}

int sfk_write_prep(sfk_state_pt s)
{
	return OSP_protected_call(OSP_same,sfk_write_prep_internal,s);
}

LDEF(char *chooser_name(char *buf))
{
  unsigned char **handle;
  int len;
  handle=(unsigned char**)GetString(-16096);
  len=GetHandleSize(handle);
  if((handle==0)||(len<=1))
   return "no-chooser-name";
  if((**handle)>(len-1))
   return "bad-chooser-name";
  len=imin(sfk_uname_len-1,len);
  len=imin(len,**handle);
  if(len!=0)
   memcpy(buf,(*handle)+1,len);
  buf[len]=0;
  return buf;
}

int sfk_dont_like_prep(sfk_state_pt s)
{
  int result=((s->sfk_st_maj_ver!=sfk_ST_maj_ver)||
     (s->sfk_st_min_ver!=sfk_ST_min_ver)||
     (s->sfk_check_num!=sfk_check_num));
  return result;
}

void sfk_init_state(sfk_state_pt s)
{
  memset(s,0,sizeof(*s));
  s->sfk_st_maj_ver=sfk_ST_maj_ver;
  s->sfk_st_min_ver=sfk_ST_min_ver;
  s->sfk_check_num=sfk_check_num;
}

void sfk_invent_state(sfk_state_pt s)
{
  char buf[256];
  sfk_init_state(s);
  s->sfk_remind_time=DEFAULT_REMIND_TIME;
  s->sfk_sound_id=DEFAULT_SOUND_ID;
  TURN_ON_FLAGS(*s,MCS_want_open|MCS_hearmm|MCS_nsound|MCS_nblink|MCS_npost);
  strcpy(s->sfk_uname,chooser_name(buf));
  strcpy(s->sfk_text_state,"Default configuration used");
}

int sfk_get_drvr_state(int refnum,sfk_state_pt s)
{
  int err;
  sfk_io_record ios;
  INIT_CPB(ios,refnum,sfk_get_state);
  ios.iostate.sfk_st_maj_ver=sfk_ST_maj_ver;
  ios.iostate.sfk_st_min_ver=sfk_ST_min_ver;
  err=PBControl(&ios,FALSE);
  *s=ios.iostate;
  return err;
}

sound_res_state sfk_use_sound_res(void)
{
	sound_res_state sstate;
	sstate.prev_res=CurResFile();
  	sstate.sound_res=find_res_file("\pMailCheck Sounds",fsRdPerm);
  	if(sstate.sound_res>0)
  	  UseResFile(sstate.sound_res);
  	return sstate;
}

void sfk_close_sound_res(sound_res_state sstate)
{
 	if(sstate.sound_res>0)
	  CloseResFile(sstate.sound_res);
	UseResFile(sstate.prev_res);
}

/*
 * set driver state where the sound has possibly changed
 */
int sfk_set_drvr_state_sound_yes(int refnum,sfk_state_pt s,int iokind)
{
  sound_res_state cur_res=sfk_use_sound_res();
  int err=sfk_set_drvr_state_sound_no(refnum,s,iokind);
  sfk_close_sound_res(cur_res);
  return err;
}

/*
 * set driver state where the sound has not changed
 */
int sfk_set_drvr_state_sound_no(int refnum,sfk_state_pt s,int iokind)
{
  int err;
  sfk_io_record ios;
  ios.iostate=*s;
  INIT_CPB(ios,refnum,iokind);
  err=PBControl(&ios,FALSE);
  *s=ios.iostate;
  return err;
}

#endif

