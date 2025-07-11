#include "stuff.h"

long prog_div;
long uncomp_tot,comp_tot;

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

struct sitHdr sh;
struct fileHdr fh;

short ofd;
ushort crc;
int rmfiles;
long total;
char *Creator, *Type;

static unsigned short crctab[1<<8] = {
0x0, 0xc0c1, 0xc181, 0x140, 0xc301, 0x3c0, 0x280, 0xc241,
0xc601, 0x6c0, 0x780, 0xc741, 0x500, 0xc5c1, 0xc481, 0x440,
0xcc01, 0xcc0, 0xd80, 0xcd41, 0xf00, 0xcfc1, 0xce81, 0xe40,
0xa00, 0xcac1, 0xcb81, 0xb40, 0xc901, 0x9c0, 0x880, 0xc841,
0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040,
} ;

unsigned short updcrc(unsigned short icrc, unsigned char *icp, int icnt);
void BeginProcessing(void);
void EndProcessing(int numDocs);
void ProcessDoc (FSSpec *curDocFSS, char mode);
void scanfolder(FSSpec *);

unsigned short updcrc(unsigned short icrc, unsigned char *icp, int icnt)
	{
	register unsigned short crc = icrc;
	register unsigned char *cp = icp;
	register int cnt = icnt;
	while( cnt-- ) 
		{
		crc = (crc>>8) ^ crctab[(crc & ((1<<8)-1)) ^ *cp++];
		}
	return( crc );
	}

static cp2(
unsigned short x,
char dest[])
{
	dest[0] = x>>8;
	dest[1] = x;
}

static cp4(
unsigned long x,
char dest[])
{
	dest[0] = x>>24;
	dest[1] = x>>16;
	dest[2] = x>>8;
	dest[3] = x;
}

static FSSpec new;
void	UpdateFileName(FSSpec *name);

extern long bytes_out;	/* Total number of byte to output */

void BeginProcessing(void)
	{
	uncomp_tot = 0;
	comp_tot = 0;
	total = 0;
	}
	
void EndProcessing(int numDocs)
	{
	long inOutCount = 22;
	ChkOsErr(SetFPos(ofd,1,0));
	total += inOutCount;
	UpdateFileName(&new);
	/* header header */
    memcpy(sh.sig1,"SIT!",4);
    cp2(numDocs,(char *)sh.numFiles);
    cp4(total,(char *)sh.arcLen);
    memcpy(sh.sig2,"rLau",4);
    sh.version = 1;
	ChkOsErr(FSWrite(ofd,&inOutCount,&sh));
	FSClose(ofd);
	ofd = 0;
	}

void ProcessDoc (FSSpec *curDocFSS, char mode)
	{
	long inOutCount;
	OSErr err,iErr;
	int siz;
	int i,n;
	short fd;
	long fpos1, fpos2;
	if (!total)
		{
		new = *curDocFSS;
		if (new.name[0] < 28)
			{
			BlockMove(".sit", &new.name[new.name[0]+1], 4);
			new.name[0] += 4;
			}
		else
			BlockMove("\parchive.sit", &new.name, 12);
		iErr = FSpCreate(&new,'SIT!','SIT!',0);
		if (iErr != dupFNErr) ChkOsErr(iErr);
		ChkOsErr(FSpOpenDF(&new,0,&ofd));	
		ChkOsErr(SetEOF(ofd,22));
		ChkOsErr(SetFPos(ofd,1,22));
		}
	else
		{
		if (!memcmp(&new, curDocFSS, sizeof(FSSpec))) return;
		}
	UpdateFileName(curDocFSS);
	ChkOsErr(GetFPos(ofd,&fpos1));
	/* write empty header, will seek back and fill in later */
	inOutCount = 112;
	memset(&fh,0,inOutCount);
	memcpy((void *)fh.fName, &(curDocFSS->name[0]), 1+curDocFSS->name[0]);
	if (mode)	/* operate on directory entry */
		{
		CInfoPBRec curDocPB;
		curDocPB.hFileInfo.ioVRefNum = curDocFSS->vRefNum;
		curDocPB.hFileInfo.ioDirID = curDocFSS->parID;
		curDocPB.hFileInfo.ioNamePtr = curDocFSS->name;
		curDocPB.hFileInfo.ioFDirIndex = 0;	
		err = PBGetCatInfoSync (&curDocPB);
		memcpy((void *)fh.cDate, (void *)&curDocPB.hFileInfo.ioFlCrDat, 4);
		memcpy((void *)fh.mDate, (void *)&curDocPB.hFileInfo.ioFlMdDat, 4);
		fh.compRMethod = fh.compDMethod = mode;
		cp2(updcrc(0,(unsigned char *)&fh,110), (char *)fh.hdrCRC);
		ChkOsErr(FSWrite(ofd,&inOutCount,&fh));
		total += inOutCount;
		return;
		}
	else
		{
		HParamBlockRec pb;
		ChkOsErr(FSWrite(ofd,&inOutCount,&fh));
		memset(&pb, 0, sizeof(HParamBlockRec));
		pb.fileParam.ioVRefNum = curDocFSS->vRefNum;
		pb.fileParam.ioDirID = curDocFSS->parID;
		pb.fileParam.ioNamePtr = curDocFSS->name;
		err = PBHGetFInfoSync (&pb);
		memcpy((void *)fh.fType, (void *)&pb.fileParam.ioFlFndrInfo.fdType, 4);
		memcpy((void *)fh.fCreator, (void *)&pb.fileParam.ioFlFndrInfo.fdCreator, 4);
		memcpy((void *)fh.FndrFlags, (void *)&pb.fileParam.ioFlFndrInfo.fdFlags, 2);
		memcpy((void *)fh.cDate, (void *)&pb.fileParam.ioFlCrDat, 4);
		memcpy((void *)fh.mDate, (void *)&pb.fileParam.ioFlMdDat, 4);
		cp4(pb.fileParam.ioFlRLgLen,(char *)fh.rLen);
		cp4(pb.fileParam.ioFlLgLen,(char *)fh.dLen);
		/* look for resource fork */
		if (pb.fileParam.ioFlRLgLen) 
			{	/* resource fork exists */
			long fpos3;
			OSErr err = FSpOpenRF(curDocFSS, fsRdPerm, &fd);
			crc = 0;
			prog_div = 0x4000000L/pb.fileParam.ioFlRLgLen;
			ChkOsErr(GetFPos(ofd,&fpos3));
			compress(fd, ofd);
			if (bytes_out > pb.fileParam.ioFlRLgLen)
				{
				ChkOsErr(SetFPos(fd,1,0));
				ChkOsErr(SetEOF(ofd,fpos3));
				copy(fd, ofd);
				bytes_out = pb.fileParam.ioFlRLgLen;
				}
			else fh.compRMethod = lpzComp;
			FSClose(fd);
			fd = 0;
			uncomp_tot += pb.fileParam.ioFlRLgLen;
			comp_tot += bytes_out;
			cp4(bytes_out,(char *)fh.cRLen);
			cp2(crc,(char *)fh.rsrcCRC);
			}
		/* look for data fork */
		if (pb.fileParam.ioFlLgLen) 
			{	/* data fork exists */
			long fpos3;
			OSErr err = FSpOpenDF(curDocFSS, fsRdPerm, &fd);
			crc = 0;
			prog_div = 0x4000000L/pb.fileParam.ioFlLgLen;
			ChkOsErr(GetFPos(ofd,&fpos3));
			compress(fd, ofd);
			if (bytes_out > pb.fileParam.ioFlLgLen)
				{
				ChkOsErr(SetFPos(fd,1,0));
				ChkOsErr(SetEOF(ofd,fpos3));
				copy(fd, ofd);
				bytes_out = pb.fileParam.ioFlLgLen;
				}
			else fh.compDMethod = lpzComp;
			FSClose(fd);
			fd = 0;
			uncomp_tot += pb.fileParam.ioFlLgLen;
			comp_tot += bytes_out;
			cp4(bytes_out,(char *)fh.cDLen);
			cp2(crc,(char *)fh.dataCRC);
			}
		cp2(updcrc(0,(unsigned char *)&fh,110), (char *)fh.hdrCRC);
		ChkOsErr(GetFPos(ofd,&fpos2));
		ChkOsErr(SetFPos(ofd,1,fpos1));
		inOutCount = 112;
		ChkOsErr(FSWrite(ofd,&inOutCount,&fh));
		ChkOsErr(SetFPos(ofd,1,fpos2));
		total += (fpos2 - fpos1);
		}
	}

void scanfolder(FSSpec *curDocFSS)
	{
	CInfoPBRec curDocPB;
	long fpos1,fpos2,inOutCount;
	long old_uncomp_tot = uncomp_tot;
	long old_comp_tot = comp_tot;
	int i;
	FSSpec	new;
	OSErr err;
	OSErr iErr;
	ProcessDoc(curDocFSS, 0x20);
	ChkOsErr(GetFPos(ofd,&fpos1));
	curDocPB.hFileInfo.ioVRefNum = curDocFSS->vRefNum;
	curDocPB.hFileInfo.ioDirID = curDocFSS->parID;
	curDocPB.hFileInfo.ioNamePtr = curDocFSS->name;
	curDocPB.hFileInfo.ioFDirIndex = 0;	
	err = PBGetCatInfoSync (&curDocPB);
	new.parID = curDocPB.hFileInfo.ioDirID;
	new.vRefNum = curDocFSS->vRefNum;
	for (i = 1; err != fnfErr; i++) 
		{
		curDocPB.hFileInfo.ioVRefNum = new.vRefNum;
		curDocPB.hFileInfo.ioDirID = new.parID;
		curDocPB.hFileInfo.ioNamePtr = new.name;
		curDocPB.hFileInfo.ioFDirIndex = i;	
		err = PBGetCatInfoSync (&curDocPB);
		if (err == noErr)
			{
			Boolean isFolder,wasAlias;
			err = ResolveAliasFile (&new, true, &isFolder, &wasAlias);
			if (err == noErr) 
				{
				if (isFolder) scanfolder(&new);
				else ProcessDoc (&new, 0x0);
				}
			}
		}
	ProcessDoc(curDocFSS, 0x21);
	ChkOsErr(GetFPos(ofd,&fpos2));
	inOutCount = 112;
	fpos1 -= inOutCount;
	ChkOsErr(SetFPos(ofd,1,fpos1));
	ChkOsErr(FSRead(ofd,&inOutCount,&fh));
	inOutCount = 112;
	cp4(fpos2-fpos1-inOutCount,(char *)fh.cDLen);
	cp4(uncomp_tot-old_uncomp_tot,(char *)fh.dLen);
	cp2(updcrc(0,(unsigned char *)&fh,110), (char *)fh.hdrCRC);
	ChkOsErr(SetFPos(ofd,1,fpos1));
	ChkOsErr(FSWrite(ofd,&inOutCount,&fh));
	ChkOsErr(SetFPos(ofd,1,fpos2));
	}
