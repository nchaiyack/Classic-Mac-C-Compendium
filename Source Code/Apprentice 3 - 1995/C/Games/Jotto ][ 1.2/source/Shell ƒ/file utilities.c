#include "file utilities.h"
#include <Processes.h>

OSErr GetApplicationFSSpec(FSSpec *theFS)
{
	OSErr				oe;
	ProcessSerialNumber	psn;
	ProcessInfoRec		info;
	
	oe=GetCurrentProcess(&psn);
	if (oe==noErr)
	{
		info.processInfoLength=60;
		info.processName=0L;
		info.processAppSpec=theFS;
		oe=GetProcessInformation(&psn, &info);
	}
	
	return oe;
}

OSErr GetApplicationVRefNum(short *vRefNum)
{
	FSSpec			theFS;
	OSErr			err;
	
	err=GetApplicationFSSpec(&theFS);
	if (err!=noErr)
		return err;
	
	*vRefNum=theFS.vRefNum;
	
	return noErr;
}

OSErr GetApplicationParID(long *parID)
{
	FSSpec			theFS;
	OSErr			err;
	CInfoPBRec		pb;
	
	err=GetApplicationFSSpec(&theFS);
	if (err!=noErr)
		return err;
	
	pb.hFileInfo.ioCompletion=0L;
	pb.hFileInfo.ioNamePtr=theFS.name;
	pb.hFileInfo.ioVRefNum=theFS.vRefNum;
	pb.hFileInfo.ioFDirIndex=0;
	pb.hFileInfo.ioDirID=theFS.parID;
	err=PBGetCatInfo(&pb, FALSE);
	if (err!=noErr)
		return err;
	
	*parID=pb.hFileInfo.ioFlParID;
	
	return noErr;
}

OSType GetFileType(FSSpec *myFSS)
{
	HParamBlockRec	paramBlock;
	
	paramBlock.fileParam.ioCompletion=0L;
	paramBlock.fileParam.ioNamePtr=myFSS->name;
	paramBlock.fileParam.ioVRefNum=myFSS->vRefNum;
	paramBlock.fileParam.ioFDirIndex=0;
	paramBlock.fileParam.ioDirID=myFSS->parID;
	if (PBHGetFInfo(&paramBlock, FALSE)!=noErr)
		return 0L;
	
	return paramBlock.fileParam.ioFlFndrInfo.fdType;
}

unsigned long GetModificationDate(FSSpec *myFSS)
{
	HParamBlockRec	paramBlock;
	
	paramBlock.fileParam.ioCompletion=0L;
	paramBlock.fileParam.ioNamePtr=myFSS->name;
	paramBlock.fileParam.ioVRefNum=myFSS->vRefNum;
	paramBlock.fileParam.ioFDirIndex=0;
	paramBlock.fileParam.ioDirID=myFSS->parID;
	if (PBHGetFInfo(&paramBlock, FALSE)!=noErr)
		return 0L;
	
	return paramBlock.fileParam.ioFlMdDat;
}

OSErr SetModificationDate(FSSpec *myFSS, unsigned long theDate)
{
	HParamBlockRec	paramBlock;
	OSErr			err;
	
	paramBlock.fileParam.ioCompletion=0L;
	paramBlock.fileParam.ioNamePtr=myFSS->name;
	paramBlock.fileParam.ioVRefNum=myFSS->vRefNum;
	paramBlock.fileParam.ioFDirIndex=0;
	paramBlock.fileParam.ioDirID=myFSS->parID;
	if ((err=PBHGetFInfo(&paramBlock, FALSE))!=noErr)
		return err;
	
	FlushVol(0L, myFSS->vRefNum);
	
	paramBlock.fileParam.ioFlMdDat=theDate;
	paramBlock.fileParam.ioCompletion=0L;
	paramBlock.fileParam.ioNamePtr=myFSS->name;
	paramBlock.fileParam.ioVRefNum=myFSS->vRefNum;
	paramBlock.fileParam.ioFDirIndex=0;
	paramBlock.fileParam.ioDirID=myFSS->parID;
	err=PBHSetFInfo(&paramBlock, FALSE);
	
	return err;
}
