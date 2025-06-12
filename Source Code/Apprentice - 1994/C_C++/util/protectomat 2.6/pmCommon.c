/*
 * pmCommon.h - appleshare dump/load protections common code
 * by Aaron Wohl (aw0g+@andrew.cmu.edu)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "pmCommon.h"

cache_entry none_cache={"\p*none*",0};

void allocate_cache_entries()
{
	group_cache=(cache_entry_pt)NewPtr(CACHE_SIZE*sizeof(*group_cache));
	user_cache=(cache_entry_pt)NewPtr(CACHE_SIZE*sizeof(*group_cache));
	if((group_cache==0)||
	   (user_cache==0)) {
	     printf("fatal:not enough memory to allocate user and group caches");
	     exit(1);
	}
	memset(group_cache,0,CACHE_SIZE*sizeof(*group_cache));
	memset(user_cache,0,CACHE_SIZE*sizeof(*user_cache));
	*group_cache = none_cache;
	*user_cache = none_cache;
}

/*
 * read a line from a file
 */
int read_line(char *aline,FILE *afile)
{
	char *s;
	if(fgets(aline,ERR_BUF_SIZE-10,afile)!=aline)
		return TRUE;
	aline[ERR_BUF_SIZE-10]=0;
	if((s=strchr(aline,'\n'))!=0)
		*s=0;
	return FALSE;
}

/*
 * get volume id of a volume
 * returns -1 on an error
 */
long get_vol_id(char *in_name)
{
	int os;
	HParamBlockRec vinfo;
	char aname[300];
	memset(&vinfo,0,sizeof(vinfo));
	strcpy(aname+1,in_name);
	strcat(aname+1,":");
	aname[0]=strlen(aname+1);
	vinfo.volumeParam.ioNamePtr=(StringPtr)aname;
	vinfo.volumeParam.ioVRefNum= 0;
	vinfo.volumeParam.ioVolIndex= -1;
	os=PBHGetVInfo(&vinfo,FALSE);
	if(os!=0) 
		return BAD_VOL_NUM;
	return vinfo.volumeParam.ioVRefNum;
}
