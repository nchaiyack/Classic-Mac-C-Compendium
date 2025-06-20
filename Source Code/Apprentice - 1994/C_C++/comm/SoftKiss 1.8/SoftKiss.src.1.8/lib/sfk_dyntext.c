/*
 * sfk_dyntext.c - dynamicly loaded text
 */

#include "sfk_codecheck.h"
#include "sfk_dyntext.h"
#include <string.h>

typedef unsigned short uint16;

/*
 * clean up a dt text block
 */
void sfk_dt_free(dyn_text_pt dt)
{
	if((dt->fr!=0)&&(dt->tstrs!=0)) {
		(dt->fr)(dt->tstrs);
		dt->tstrs=0;
	}
	if((dt->thand!=0)&&(dt->dispose_thand))
		DisposeHandle(dt->thand);
	dt->thand=0;
	dt->dispose_thand=0;
}

/*
 * read in a dynamic text block
 * return TRUE if successfull
 */
int sfk_dt_load(short res_id,dyn_text_pt dt,alloc_rtn ar,free_rtn fr)
{
	unsigned char *s;
	unsigned short len;
	unsigned short nstrs;
	unsigned short *dst;
	memset(dt,0,sizeof(*dt));
	dt->fr=fr;
	if((dt->thand=(void*)GetResource('STR#',res_id))==0)
		return FALSE;
	HUnlock(dt->thand);
	if(MemError()!=0)goto fail;
	MoveHHi(dt->thand);
	HLock(dt->thand);
	if(MemError()!=0)goto fail;
	DetachResource(dt->thand);
	if(ResError()!=0)goto fail;
	// dt->dispose_thand=TRUE;
	dt->tptr=(void*)StripAddress(*(dt->thand));
	dt->nstrs= (*((unsigned short*)(dt->tptr)));
	dt->tstrs= (*ar)((dt->nstrs+1)*sizeof(unsigned short));
	s=dt->tptr+2;
	dst= dt->tstrs;
	*dst++=0;
	for(nstrs=dt->nstrs;nstrs>0;nstrs--) {
		len= *s;
		PtoCstr(s);
		*dst++= (s-dt->tptr);
		s+= (len+1);
	}
  return TRUE;

  fail:
  	sfk_dt_free(dt);
  	return FALSE;
}
