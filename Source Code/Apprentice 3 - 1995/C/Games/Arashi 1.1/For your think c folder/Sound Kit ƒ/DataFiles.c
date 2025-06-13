/*/
     Project Arashi: DataFiles.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:39
     Created: Saturday, October 6, 1990, 16:32

     Copyright � 1990-1992, Juri Munkki
/*/

/*
>>	This subroutines reads all data forks of a certain type from
>>	the folder where it is run.
>>
>>	ReadDataFiles is only used by the compressing
>>	part of the sound kit. You have to include this
>>	file even if you are only decompressing the
>>	sounds. Of course, the linker will remove this
>>	routine when (and if) it notices that it's not used.
*/

#include "Shuddup.h"


Handle	ReadDataFiles(ftype)
long	ftype;
{
	register	Handle			fdata;
	register	Handle			sinfo;
	register	long			datasize;
				long			len;
	register	int				numfiles,i,numread;
				int				ref;
				volumeParam		volupram;
				fileParam		filepram;
				char			fname[256];
	
	sinfo=GetResource(SKRESTYPE,SKSTABLE);
	fdata=NewHandle(0);
	datasize=0;

	volupram.ioCompletion=0;
	volupram.ioNamePtr=0;
	volupram.ioVRefNum=0;
	volupram.ioVolIndex=0;
	
	PBGetVInfo(&volupram,0);

	numfiles=volupram.ioVNmFls;

	filepram.ioCompletion=0;
	filepram.ioVRefNum=0;
	filepram.ioFVersNum=0;
	filepram.ioNamePtr=(void *)fname;
	
	numread=0;
	for(i=1;i<=numfiles;i++)
	{	filepram.ioFDirIndex=i;
		if(PBGetFInfo(&filepram,0)==0)
		{	if(filepram.ioFlFndrInfo.fdType==ftype)
			{	if(FSOpen(fname,0,&ref)==0)
				{	if(GetEOF(ref,&len)==0)
					{	SetHandleSize(fdata,datasize+len);
						if(!MemErr)
						{	HLock(fdata);
							FSRead(ref,&len,datasize+*fdata);
							datasize+=len;
							HUnlock(fdata);
							
							numread++;
							SetHandleSize(sinfo,numread*sizeof(long));
							if(!MemErr)
							{	((long *)(*sinfo))[numread-1]=len;
							}
						}
					}
					FSClose(ref);
				}
			}
		}
	}
	if(datasize==0)
	{	datasize=10;
	}
	SetHandleSize(fdata,datasize);
	ChangedResource(sinfo);
	WriteResource(sinfo);
	return fdata;
}