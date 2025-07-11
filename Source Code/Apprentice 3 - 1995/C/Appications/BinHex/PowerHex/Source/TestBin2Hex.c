#include <Script.h>
#include "Bin2Hex.h"
#include "TestBin2Hex.h"





#define	kBufferSize	(1024*100)




void TestBin2Hex(void)
{
	Bin2HexParam		bhp;
	StandardFileReply	reply;
	HFileParam 			hfp;
	FSSpec				destSpec;
	SFTypeList			types;
	short					refnum,srcRefNum;
	long					inOutCount;
	
	
	StandardGetFile(nil,-1,types,&reply);
	if (reply.sfGood)
	{
		destSpec = reply.sfFile;
		BlockMove(".hqx",destSpec.name+1+destSpec.name[0],4);
		destSpec.name[0] += 4;
		FSpDelete(&destSpec);
		if (FSpCreate(&destSpec,'MMCC','TEXT',smSystemScript))
		{
			SysBeep(1);
			return;
		}
		
		
		hfp.ioCompletion = NULL;
		hfp.ioNamePtr = reply.sfFile.name;
		hfp.ioVRefNum = reply.sfFile.vRefNum;
		hfp.ioFVersNum = 0;
		hfp.ioFDirIndex = 0;
		hfp.ioDirID = reply.sfFile.parID;
		PBHGetFInfo((HParmBlkPtr)&hfp,false);
		
		BlockMove(reply.sfFile.name,bhp.info.name,reply.sfFile.name[0]+1);
		bhp.info.type = hfp.ioFlFndrInfo.fdType;
		bhp.info.creator = hfp.ioFlFndrInfo.fdCreator;
		bhp.info.flags = hfp.ioFlFndrInfo.fdFlags;
		bhp.info.dfLength = hfp.ioFlLgLen;
		bhp.info.rfLength = hfp.ioFlRLgLen;
		bhp.addLFs = FALSE;
		bhp.inBufferSize = kBufferSize;
		bhp.outBufferSize = kBufferSize*3/2;
		bhp.inBuffer = (unsigned char*)NewPtr(bhp.inBufferSize);
		bhp.outBuffer = (unsigned char*)NewPtr(bhp.outBufferSize);
		FSpOpenDF(&destSpec,fsRdWrPerm,&refnum);
		
		if (!NewBin2Hex(&bhp))
			FSWrite(refnum,&bhp.outAmount,bhp.outBuffer);
		
		if (!FSpOpenDF(&reply.sfFile,fsRdWrPerm,&srcRefNum))
		{
			do
			{
				inOutCount = kBufferSize;
				FSRead(srcRefNum,&inOutCount,bhp.inBuffer);
				
				bhp.inAmount = inOutCount;
				bhp.inOffset = 0;
				while(bhp.inAmount)
				{
					if (!DataBin2Hex(&bhp))
						FSWrite(refnum,&bhp.outAmount,bhp.outBuffer);
					else
						break;
				}
			}while(inOutCount);
			FSClose(srcRefNum);
		}
		
		if (!FSpOpenRF(&reply.sfFile,fsRdWrPerm,&srcRefNum))
		{
			do
			{
				inOutCount = kBufferSize;
				FSRead(srcRefNum,&inOutCount,bhp.inBuffer);
				
				bhp.inAmount = inOutCount;
				bhp.inOffset = 0;
				while(bhp.inAmount)
				{
					if (!ResourceBin2Hex(&bhp))
						FSWrite(refnum,&bhp.outAmount,bhp.outBuffer);
					else
						break;
				}
			}while(inOutCount);
			FSClose(srcRefNum);
		}
		
		FSClose(refnum);
		DisposePtr((Ptr)bhp.inBuffer);
		DisposePtr((Ptr)bhp.outBuffer);
	}
}