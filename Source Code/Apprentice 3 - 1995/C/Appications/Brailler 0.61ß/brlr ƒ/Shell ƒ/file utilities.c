#include "file utilities.h"
#include "util.h"
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

OSType GetFileCreator(FSSpec *myFSS)
{
	HParamBlockRec	paramBlock;
	
	paramBlock.fileParam.ioCompletion=0L;
	paramBlock.fileParam.ioNamePtr=myFSS->name;
	paramBlock.fileParam.ioVRefNum=myFSS->vRefNum;
	paramBlock.fileParam.ioFDirIndex=0;
	paramBlock.fileParam.ioDirID=myFSS->parID;
	if (PBHGetFInfo(&paramBlock, FALSE)!=noErr)
		return 0L;
	
	return paramBlock.fileParam.ioFlFndrInfo.fdCreator;
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

OSErr TouchFolder(FSSpec *theFS)
{
	CInfoPBRec		pb;
	OSErr			isHuman;
	
	pb.dirInfo.ioCompletion=0L;
	pb.dirInfo.ioVRefNum=theFS->vRefNum;
	pb.dirInfo.ioDrDirID=theFS->parID;
	pb.dirInfo.ioNamePtr=theFS->name;
	pb.dirInfo.ioFDirIndex=0;
	if ((isHuman=PBGetCatInfo(&pb, FALSE))!=noErr)
	{
		pb.dirInfo.ioDrDirID=pb.dirInfo.ioDrParID;
		pb.dirInfo.ioFDirIndex=0;
		GetDateTime(&(pb.dirInfo.ioDrMdDat));
		isHuman=PBSetCatInfo(&pb, FALSE);
	}
	
	return isHuman;
}

OSErr FileExists(FSSpec *theFS)
{
	CInfoPBRec		pb;
	
	pb.dirInfo.ioCompletion=0L;
	pb.dirInfo.ioVRefNum=theFS->vRefNum;
	pb.dirInfo.ioDrDirID=theFS->parID;
	pb.dirInfo.ioNamePtr=theFS->name;
	pb.dirInfo.ioFDirIndex=0;
	
	return PBGetCatInfo(&pb, FALSE);
}

void GetVersionString(Str255 theVersion)
{
	Handle			theversResource;
	
	theversResource=GetResource('vers', 1);
	Mymemcpy(theVersion, (unsigned char*)(*theversResource+6),
		*((unsigned char*)(*theversResource+6))+1);
	ReleaseResource(theversResource);
}
