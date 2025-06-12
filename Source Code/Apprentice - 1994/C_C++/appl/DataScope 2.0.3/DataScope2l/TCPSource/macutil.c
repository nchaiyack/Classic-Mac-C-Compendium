/*  MACUTIL.C
*   Utilities for the network library that are Macintosh specific
*/
#include "stdio.h"
#include "memory.h"

#ifdef UNDERNOCIRCUMSTANCES
#include "protocol.h"
#include "data.h"
#endif

#ifdef MPW
#include "mpw.h"
#endif

#include <Files.h>
#include <Types.h>
#include <appletalk.h>
#include <events.h>
#include <errors.h>
#include <OSutils.h>
#include "whatami.h"

/* Some globals for file lookup */
int defaultv;

ParamBlockRec FLpb;
CInfoPBRec HFLpb, tHFLpb;
char FileName[256], FileTemplate[256];
int DirTree[256];
char *iobufptr=0L,*iobuf=0L,*iobufmax=0L;
int iobufsize,lastcr;

int getWDnum()
{
	WDPBRec pb;

	pb.ioNamePtr=0L;

	if (isHFS())  PBHGetVol(&pb,FALSE);
		else PBGetVol( (ParmBlkPtr) &pb,FALSE);
	return(pb.ioWDVRefNum);		/* This proc returns the Vol reference */
}

Str255 *getVname()
{
	WDPBRec pb;
	char *temp;

	temp= (char *)NewPtr(256);

	pb.ioNamePtr=temp;
	PBGetVol( (ParmBlkPtr) &pb,FALSE);
	(*temp)++;
	temp[*temp]=':';
	temp[*temp+1]=0;
	return( (Str255 *)temp);		/* This proc returns the Vol reference */
}

Str255 *getWDname()
{
	WDPBRec pb;
	CInfoPBRec cpb;
	char tempst[100],*wdtemp,*wdout,*start,*store,*trav;
	int i,j;

	if (!isHFS()) return(getVname()); /* if not HFS then only volume exists */

	wdout= (char *) NewPtr(256);
	wdtemp= (char *) NewPtr(256);

	sprintf(tempst,"ioNamePtr = %lx",wdout);
	putln (tempst);

	pb.ioNamePtr=wdout;
	PBHGetVol(&pb,FALSE);

#ifdef WANNACOLON
	(*wdout)++;
	wdout[*wdout]=':';
#endif WANNACOLON

	cpb.dirInfo.ioCompletion=0L;
	cpb.dirInfo.ioVRefNum=pb.ioWDVRefNum;
	cpb.dirInfo.ioDrDirID=pb.ioWDDirID;
	trav=wdtemp;
	DirTree[ (j=0) ]=pb.ioVRefNum;

	while(cpb.dirInfo.ioDrDirID!=0) {
		cpb.hFileInfo.ioNamePtr=(StringPtr) trav;
		cpb.hFileInfo.ioFDirIndex=(short) -1;
		if (PBGetCatInfo(&cpb,FALSE)!=0)  {cpb.dirInfo.ioDrDirID=0; break;}
		if (*trav==0) {cpb.dirInfo.ioDrDirID=0;break;}
		i=*trav; *trav=0; start=trav+1;
		trav+=i+1; *trav=0;
		DirTree[j++]=cpb.dirInfo.ioDrDirID;
		cpb.dirInfo.ioDrDirID=cpb.dirInfo.ioDrParID;
		}

	*wdtemp=0;
	store=wdout+*wdout+1;
	if (trav-wdtemp) *wdout=(trav-wdtemp);
	if (trav!=wdtemp) start=trav-1; else start=wdtemp;

	if (start!=wdtemp) {
		while(*start) start--;		/* Go to beginning of string */
		if (start!=wdtemp) start--;
		}

	while(start!=wdtemp) {
		while(*start) start--;		/* Go to beginning of string */
		trav=start+1; *store='/';	/* Ready to move directory name */
		store++;
		while (*trav) {*store=*trav; store++;trav++; } /* store it */
		if (start!=wdtemp) start --;
		*store=0;
		}
	DisposPtr ((Ptr)wdtemp);
	return( (Str255 *)wdout);
}

setWDnum( num)
int num;
{
	WDPBRec pb;

	pb.ioNamePtr=0L; 
	pb.ioVRefNum=num;
	pb.ioWDDirID=0L;
	PBHSetVol(&pb,FALSE);
}

/*
*  setSFdir
*  trick to set up the working directory from the last SFgetfile. TK
*/

#define SFSaveDisk  0x214
#define CurDirStore 0x398

setSFdir( )
{
	WDPBRec pb;

	if (!isHFS()) {
		defaultv = - (* (short *) SFSaveDisk);
		return;
	}
	
	pb.ioNamePtr=0L; 
	pb.ioWDProcID=0L; 
	pb.ioVRefNum=  - (* (short *) SFSaveDisk);		/* saved from last SF package call */
	pb.ioWDDirID= (* (int *) CurDirStore);
	PBOpenWD(&pb,FALSE);
	
	defaultv = pb.ioVRefNum;
	setvol(NULL, defaultv);			/* make working directory */

}

setWDname( num, name)
int num;
Str255 *name;
{
	WDPBRec pb;

	pb.ioNamePtr=0L; 
	pb.ioVRefNum=num;
	pb.ioWDDirID=0L;
	PBHSetVol(&pb,FALSE);
}

wccheck(file,template)
char *template,*file;
{
	while(*template) {
		if (*template=='*') {
			template++;
			if (*template) {
				while((*file) && (*file!=*template)) file++;
				if ((*file)==0) return(0);
				}
			else return(1);
			continue;
			}
		else
			if ((*template!='?') && (*template!=*file)) return(0);
		template++;file++;
		}
	return(1);
}

/**********************************************************************/
/*
*   Find directory name -- return a code that indicates whether the
*   directory exists or not.
*   0 = dir name ok
*   -1 = error
*   > 0 = dos error code, no dir by this name
*
*   Accept certain unix conventions, like '/' for separator
*
*   Also, append a '\' to the name before returning
* 
*  Note:  There must be enough room in the string to append the '\'
*/

direxist(dirname)
	char dirname[];
	{
	return(-1);
	}

/**********************************************************************/
/* firstname
*  find the first name in the given directory which matches the wildcard
*  specification
*
*  must NewPtr enough space for the path plus a full filename
*
*  expand '*' (unix) to '*.*' (dos)
*/
char *firstname(spec)
char *spec;
{
	char *loader;
	int HFS;

	setvol(0L, defaultv);							/* Go to default Directory */

	if ( (HFS=isHFS()) )
		PBHGetVol( (WDPBPtr) &HFLpb, FALSE);
	else
		 PBGetVol(&FLpb,FALSE);

	loader=FileTemplate;
	while(*loader++=*spec++);
	putln(spec);
	putln(FileTemplate);
	if ( HFS) {
		HFLpb.hFileInfo.ioCompletion=(ProcPtr) NULL;
		HFLpb.hFileInfo.ioNamePtr=(StringPtr) FileName;
		HFLpb.hFileInfo.ioFVersNum=(char) 0;
		HFLpb.hFileInfo.ioFDirIndex=(short) 1;
		tHFLpb=HFLpb;
		if (PBGetCatInfo(&HFLpb,FALSE)!=0)  return(0L);
		}
	else {
		FLpb.fileParam.ioCompletion=0;
		FLpb.fileParam.ioNamePtr=FileName;
		FLpb.fileParam.ioFVersNum=0;
		FLpb.fileParam.ioFDirIndex=1;
		if(PBGetFInfo(&FLpb,FALSE)!=0) return(0L);
		}
	FileName[FileName[0]+1]=0;
	while(!wccheck(&FileName[1],FileTemplate)) {
		if (HFS) {
			tHFLpb.hFileInfo.ioFDirIndex++;
			HFLpb=tHFLpb;
			if (PBGetCatInfo(&HFLpb,FALSE)!=0)  return(0L);
			}
		else {
			FLpb.fileParam.ioFDirIndex++;
			if(PBGetFInfo(&FLpb,FALSE)!=0) return(0L);
			}
		FileName[FileName[0]+1]=0;
		}

	if (isHFS() && ((HFLpb.hFileInfo.ioFlAttrib) & 16))  {
		FileName[++FileName[0]]='/';
		FileName[FileName[0]+1]=0;
		}

	return(&FileName[1]);
}

/**********************************************************************/
/* nextname
*  modify the path spec to contain the next file name in the
*  sequence as given by DOS
*
*  if at the end of the sequence, return NULL
*/
char *nextname()
{

	if (isHFS() ) {
		tHFLpb.hFileInfo.ioFDirIndex++;
		HFLpb=tHFLpb;
		if (HFLpb.hFileInfo.ioNamePtr!=FileName) return(0L);
		if (PBGetCatInfo(&HFLpb,FALSE)!=0)  return(0L);
		FileName[FileName[0]+1]=0;
		while(!wccheck(&FileName[1],FileTemplate)) {
			putln(&FileName[1]);
			tHFLpb.hFileInfo.ioFDirIndex++;
			HFLpb=tHFLpb;
			if (PBGetCatInfo(&HFLpb,FALSE)!=0)  return(0L);
			FileName[FileName[0]+1]=0;
			}
		}
	else {
		if (FLpb.fileParam.ioNamePtr!=FileName) return(0L);
		FLpb.fileParam.ioFDirIndex++;
		if(PBGetFInfo(&FLpb,FALSE)!=0) return(0L);
		FileName[FileName[0]+1]=0;
		while(!wccheck(&FileName[1],FileTemplate)) {
			FLpb.fileParam.ioFDirIndex++;
			if(PBGetFInfo(&FLpb,FALSE)!=0) return(0L);
			FileName[FileName[0]+1]=0;
			}
		}

	if (isHFS() && (HFLpb.hFileInfo.ioFlAttrib & 16))  {
		FileName[++FileName[0]]='/';
		FileName[FileName[0]+1]=0;
		}

	return(&FileName[1]);
}


/************************************************************************/
/* userabort
*  check to see if the user wants to abort
*  For the PC, just see if the user has pressed ESC
*  return true if the user wants to abort
*/
userabort()
{
	EventRecord theEvent;

	while (GetNextEvent(24,&theEvent)) {
		if ((theEvent.modifiers & cmdKey) &&
			(theEvent.message & 0xff) =='.') 
			return(-1);
		}
	return(0);
}

dopwd(where,howmuch)
char *where;
int howmuch;
{
	char *temp;
	int i;

	if (howmuch<255) return(-1);
	setvol(0L, defaultv);							/* Go to default Directory */
	temp= (char *)getWDname();
	where[0]='/';
	for( i=1;i<=temp[0];i++) where[i]=temp[i];
	where[temp[0]]=0;
	putln(&temp[1]);
	DisposPtr ((Ptr)temp);
	putln(where);
	return(0);
}


chgdir( modifier)
char *modifier;
{
	WDPBRec pb;
	char tempst[256], *nSlash, *start;
	int i=0, j=0;

	if (! isHFS()) return(-1);

	setvol(0L, defaultv);							/* Go to default Directory */

	start= (char *)getWDname();
	DisposPtr ((Ptr)start);

	start=modifier;
	tempst[0]=':';

	putln(start);

	if ( strcmp("..",modifier)==0) {
		modifier[0]=0;
		j++;
		}
	else
	while ( (nSlash = (char *)index(modifier,'/') ) !=0L) {
		if (( (nSlash -modifier) ==2) && (*modifier=='.') && (*(modifier+1) =='.') ) {
			modifier +=3L;
			start=modifier;
			j++;
			}
		else {
			*nSlash=':';
			modifier=nSlash+1L;
			}
		}

	if ( *start==':') {
		strcpy( tempst, start+1L);
		j= strlen( tempst);
		if (tempst[j-1] !=':') {
			tempst[j] = ':';
			tempst[j+1]='\000';
			}
		}
	else
		strcpy( tempst+1L, start);

	putln(tempst);

	pb.ioNamePtr=ctop(tempst);
#ifdef MPW
	c2pstr(tempst);
#endif
	pb.ioVRefNum=0;
	pb.ioWDDirID=DirTree[j];
	pb.ioCompletion=0L;
	pb.ioWDProcID='NCSA';

	j = PBOpenWD( &pb,0);

	if ( (j==-35) || (j==-43) ) return(1);

	j = setvol(0L, (defaultv=pb.ioVRefNum));

	if ( (j==-35) || (j==-43) ) return(1);

	return(0);
}

/*
 *
 */

Scolorset( ip, s)
char *s;
unsigned int *ip;
{
	if (3==sscanf(s,"{%d,%d,%d}", &ip[0], &ip[1],&ip[2]) )
		return(1);
	return(0);
}

long time(x)
long x;
{
	return( TickCount()/60);
}

int memcmp( a, b, len)
char *a, *b;
int len;
{
	while (len --)
		if (*a!=*b) 
			return(1);
	return(0);
}

/**************************************************************************/
/*  defdir and sysdir
*   Use to change to and from the system folder directory.
*/
#define CurrentVersion 1			/* Last known SysEnvirons version */

sysdir()
{
	int err;
	char s[50];
	SysEnvRec theWorld;				/* System Environment record */
	
	err = SysEnvirons(CurrentVersion, &theWorld);
	if (err == envVersTooBig) {
		/* should post a message saying we need to be updated */
		putln("SysEnvirons out of date - macutil.c");
	}
	if (err != noErr)				/* possibly not available assume nothing here! */
		return(0);

	(void) setvol(NULL, theWorld.sysVRefNum);	/* change default volume */
	sprintf(s,"set sysdir: %d",theWorld.sysVRefNum);
	putln(s);
	
}

setmydir()
{
	char s[50];
	
	(void) setvol(NULL, defaultv);
	sprintf(s,"set defdir: %d",defaultv);
	putln(s);
	return(defaultv);
}

makemydir(dir)
	int dir;
{
	char s[50];
	
	defaultv = dir;
	setvol(NULL, defaultv);
	sprintf(s,"make dir: %d",defaultv);
	putln(s);
	return(defaultv);
	
}
