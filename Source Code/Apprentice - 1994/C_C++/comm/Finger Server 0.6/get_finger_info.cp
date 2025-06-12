/*
 * return the current system finger info
 * by Aaron Wohl n3liw+@cmu.edu
 * 412-268-5032 / 412-731-3691
 */

#include <stdlib.h>
#include <stdio.h>
#include <GestaltEQU.h>
#include <Processes.h>
#include <string.h>
#include <Packages.h>
#include "fcb.h"

#include "sip_interface.h"

#define dst_printf(xx_arg) do { sprintf xx_arg ; dst+=strlen(dst); } while(0)

#define BYTE0(xx_arg) ((xx_arg)&0x0FF)
#define BYTE1(xx_arg) BYTE0(xx_arg>>8)

#define BSTR_LEN			(256)		//general string buffer length
#define	MAX_DISKS			(10)		//limit of number of disks in bug report
#define CLEAR_PARITY(xx)	((xx)&0x7F)
#define DEL_CH				0x7F
#define MAX_DISK_LEN		(17)		//maxmimum length of one disk name
#define KSIZE				(1024)
#define MIN_DISK_SIZE		(2000000.0)		//ignore disks smaller than this

/*
 * return a gestalt long or zero if not found
 */
static long gestalt_read(long selector)
{
	long response;
	int iErr = Gestalt(selector, &response);
	if ( iErr )
		return 0;
	return response;
}

/*
 * count fcb blocks that are in use
 */
static long count_fcbs_inuse(int num_fcbs,FCBrec_pt fcbs)
{
	register int result=0;
	while(--num_fcbs >=0)
	  if((fcbs++)->fcbFlags!=0)
	    result++;
	return result;
}

/*
 * print the open files
 */
static char *print_fcbs(char *dst,char *limit,int num_fcbs,FCBrec_pt fcbs)
{
	for(;num_fcbs>0;num_fcbs--, fcbs++)
	  if(fcbs->fcbFlags!=0) {
	    int len=(fcbs->fcbCName[0]&0xFF);
	    if(len>0) {
	    	dst_printf((dst,"File "));
	    	dst_printf((dst,"%ld,%ld,",fcbs->fcbCrPs,fcbs->fcbEOF));
	  		memcpy(dst,fcbs->fcbCName+1,len);
	  		dst+=len;
	  		*dst++ = '\n';
	  	if((dst-limit)>=0)
	  		break;
	  	}
	 }
	 *dst =0;
	 return dst;
}

/*
 * report on file control block usage
 */
static char *fcb_report(char *dst,char *limit)
{
	char **xxFCBSPtr = (char **)0x34e;
	unsigned short *fcb_space=(unsigned short *) (*xxFCBSPtr);
	FCBrec_pt fcbs=((FCBrec_pt)(fcb_space+1));
	long num_fcbs= *fcb_space;
	num_fcbs/=sizeof(FCBrec);
	dst_printf((dst,"Files %ld/%ld\n",count_fcbs_inuse(num_fcbs,fcbs),num_fcbs));
	dst=print_fcbs(dst,limit,num_fcbs,fcbs);
	return dst;
}

/*
 * convert illegal characters to dots so so they don't mess up the guardian logs
 */
static void clean_file_name(unsigned char *buf,int len)
{
	register unsigned char ch;
    while (len-- > 0) {
		ch=CLEAR_PARITY(*buf);
		if((ch<' ')||
			(ch==DEL_CH)||
			(ch==':')||
			(ch==','))
		  		ch='.';
		*buf++ = ch;
	}
	*buf++ =0;
}

/*
 * is the passed volume an appleshare server?
 */
static int is_aftp_volume(short vnum)
{
	int err;
	HParamBlockRec pblk;
	GetVolParmsInfoBuffer dst;
	memset(&pblk,0,sizeof(pblk));
	pblk.ioParam.ioVRefNum=vnum;
	pblk.ioParam.ioBuffer= (Ptr)&dst;
	pblk.ioParam.ioReqCount=sizeof(dst);
	err = PBHGetVolParms(&pblk,FALSE);
	if(err!=0)
		return FALSE;
	if((dst.vMAttrib & (1L<<bHasExtFSVol) )!=0)
		return TRUE;
	return FALSE;
}

/*
 * get info about a particular disk drive
 */
static char *vol_dinfo(char *dst,int dnum)
{
	HParamBlockRec pblk;
	int err;
	unsigned char buf[BSTR_LEN];
	int len;
	double fdisk_size;
	double fdisk_free;
	long percent_used;
	long disk_size;
	long disk_free;

	memset(&pblk,0,sizeof(pblk));

	pblk.volumeParam.ioVolIndex= dnum;
	pblk.volumeParam.ioNamePtr=(StringPtr)buf;
	err=PBHGetVInfo(&pblk,TRUE);
	if(err!=0)
		return dst;

	if(is_aftp_volume(pblk.volumeParam.ioVRefNum))
		return dst;

	len=buf[0];
	clean_file_name(&buf[1],len);
	buf[MAX_DISK_LEN]=0;		/*possibly truncate file name*/

	fdisk_size=pblk.volumeParam.ioVNmAlBlks;
	fdisk_free=pblk.volumeParam.ioVFrBlk;
	fdisk_size*=((unsigned)pblk.volumeParam.ioVAlBlkSiz);
	fdisk_free*=((unsigned)pblk.volumeParam.ioVAlBlkSiz);
	if(fdisk_size<=MIN_DISK_SIZE)
		return dst;
	percent_used=(((fdisk_size-fdisk_free)/fdisk_size)*100.0);
	if(percent_used<0)
		percent_used=0;
	else if (percent_used>100)
		percent_used=100;
	fdisk_size/=KSIZE;
	fdisk_free/=KSIZE;
	disk_size=fdisk_size;
	disk_free=fdisk_free;

//		pblk.volumeParam.ioVCrDate,
//		pblk.volumeParam.ioVFSID,

	dst_printf((dst,"Disk %3ld%% %7ld %7ld %s\n",
		percent_used,
		disk_size,
		disk_free,
		&buf[1]));
	return dst;
}

/*
 * report online disks
 */
static char *report_disks(char *dst,char *limit)
{
	int i;
	for(i=1;i<MAX_DISKS;i++) {
		if ((dst-limit)>=0)
			break;
		dst=vol_dinfo(dst,i);
		}
	return dst;
}

#define MIN 		((unsigned long)(60L))
#define HOUR		(MIN*60)
#define DAY			(24*HOUR)
#define TICKS_PER_SECOND ((unsigned long)(60L))

/*
 * convert a time in seconds to X-hh:mm:ss
 */
static void relative_time(char *dst,unsigned long atime)
{
	unsigned long days;
	unsigned long hours;
	unsigned long mins;
	unsigned long secs;

	atime/=TICKS_PER_SECOND;
	days=atime/DAY;
	atime%=DAY;
	hours=atime/HOUR;
	atime%=HOUR;
	mins=atime/MIN;
	atime%=MIN;
	secs=atime;
	sprintf(dst,"%2ld-%02ld:%02ld:%02ld",days,hours,mins,secs);
}

/*
 * display running processes
 */
static char *proc_report(char *dst,char *limit)
{
	ProcessInfoRec infoRec;
	ProcessSerialNumber process;
	FSSpec aFSSpec;
	int len;
	unsigned char proc_name[32];
	char cpu_time[BSTR_LEN];
	char start_time[BSTR_LEN];
	process.highLongOfPSN = 0;
	process.lowLongOfPSN = kNoProcess; 	// start from the beginning
	infoRec.processInfoLength = sizeof(ProcessInfoRec);
	infoRec.processName = (StringPtr) proc_name;
	infoRec.processAppSpec = &aFSSpec;
	dst_printf((dst,"# start-time cpu-used heap-size heap-free name\n"));
	while (GetNextProcess(&process)==0) {
		if((dst-limit)>=0)
			break;
		if ( GetProcessInformation(&process,&infoRec) != 0)
			continue;
		len=proc_name[0];
		clean_file_name(&proc_name[1],len);
		proc_name[sizeof(proc_name)-1]=0;		/*possibly truncate file name*/
		relative_time(cpu_time,infoRec.processActiveTime);
		relative_time(start_time,TickCount()-infoRec.processLaunchDate);
		dst_printf((dst,"Proc %s %s %5ld %5ld %s\n",
			start_time,
			cpu_time,
			infoRec.processSize/KSIZE,
			infoRec.processFreeMem/KSIZE,
			proc_name+1));
	}
	return dst;
}

/*
 * report current data and time
 */
static char *report_time(char *dst,char *limit)
{
	unsigned long secs;
	Str255	todayStr;
	Str255	time_string;
	GetDateTime(&secs);
	IUDateString( secs, longDate, todayStr );	/* text w/o abbreviations */
	IUTimeString(secs,FALSE,time_string);
	dst_printf((dst,"Time %#s %#s\n",todayStr,time_string));
	return dst;
}

/*
 * return info about this system
 */
void get_finger_info(char *dst,char *limit)
{
	dst_printf((dst,"FingerServ %ld.%ld ",(long)FSERV_MAJ,(long)FSERV_MIN));
	{
		long sys_vers=gestalt_read('sysv');
		long sys_maj=BYTE1(sys_vers);
		long sys_min=BYTE0(sys_vers);
		dst_printf((dst,"Sys %ld.%ld, ",sys_maj,sys_min));
	}
	dst_printf((dst,"MacTCP v%ld\n",gestalt_read('mtcp')));
	dst=report_time(dst,limit);
	dst_printf((dst,"# %%used sizeK freeK name\n"));
	dst=report_disks(dst,limit);
	dst=proc_report(dst,limit);
	dst=fcb_report(dst,limit);
}
