#include "Hex2Bin.h"
#include "DecodeBinHex.h"





#define	kBufferSize		(1024*100)





void DecodeBinHex(FSSpec *file)
{
	Hex2BinParam		pb;
	Ptr					inBuff,outBuff;
	StandardFileReply	reply;
	short					srcRefnum,dstRefnum,err;
	
	
	srcRefnum = dstRefnum = err = 0;
	inBuff = outBuff = NULL;
	
	inBuff = NewPtr(kBufferSize);
	outBuff = NewPtr(kBufferSize);
	err = FSpOpenDF(file,fsCurPerm,&srcRefnum);
	
	if (!err && srcRefnum && inBuff && outBuff)
	{
		pb.buffer.inBuffer = (unsigned char*)inBuff;
		pb.buffer.inBufferSize = kBufferSize;
		pb.buffer.outBuffer = (unsigned char*)outBuff;
		pb.buffer.outBufferSize = kBufferSize;
		err = NewHex2Bin(&pb);
		
		while(!err)
		{
			if (!pb.inAmount)
			{
				pb.inOffset = 0;
				pb.inAmount = pb.buffer.inBufferSize;
				FSRead(srcRefnum,(long*)&pb.inAmount,pb.buffer.inBuffer);
				if (pb.inAmount == 0)
					err = eofErr;
			}
			
			if (!err)
				err = ParseHexData(&pb);
			
			if (!err)
				switch(pb.decodeState)
				{
					case kScanning:
						break;
					case kDecodingHeader:
						break;
					case kDecodedHeader:
						if (FSMakeFSSpec(file->vRefNum,file->parID,pb.info.name,&reply.sfFile) != fnfErr)
						{
							StandardPutFile("\pSave file as?",pb.info.name,&reply);
							if (!reply.sfGood)
							{
								if (srcRefnum)
									FSClose(srcRefnum);
								DisposePtr(inBuff);
								DisposePtr(outBuff);
								return;
							}
							
							if (reply.sfReplacing)
								err = FSpDelete(&reply.sfFile);
						}
						
						err = FSpCreate(&reply.sfFile,pb.info.creator,pb.info.type,reply.sfScript);
						break;
					case kDecodingDataFork:
						if (!dstRefnum)
							err = FSpOpenDF(&reply.sfFile,fsRdWrPerm,&dstRefnum);
						if (!err)
							err = FSWrite(dstRefnum,(long*)&pb.outAmount,pb.buffer.outBuffer);
						break;
					case kDecodedDataFork:
						if (!dstRefnum)
							err = FSpOpenDF(&reply.sfFile,fsRdWrPerm,&dstRefnum);
						if (!err)
							err = FSWrite(dstRefnum,(long*)&pb.outAmount,pb.buffer.outBuffer);
						if (dstRefnum)
							FSClose(dstRefnum);
						dstRefnum = 0;
						break;
					case kDecodingResourceFork:
						if (!dstRefnum)
							err = FSpOpenRF(&reply.sfFile,fsRdWrPerm,&dstRefnum);
						if (!err)
							err = FSWrite(dstRefnum,(long*)&pb.outAmount,pb.buffer.outBuffer);
						break;
					case kDecodedResourceFork:
						if (!dstRefnum)
							err = FSpOpenRF(&reply.sfFile,fsRdWrPerm,&dstRefnum);
						if (!err)
							err = FSWrite(dstRefnum,(long*)&pb.outAmount,pb.buffer.outBuffer);
						if (dstRefnum)
							FSClose(dstRefnum);
						break;
					case kFileDecoded:
						if (srcRefnum)
							FSClose(srcRefnum);
						if (dstRefnum)
							FSClose(dstRefnum);
						DisposePtr(inBuff);
						DisposePtr(outBuff);
						return;
						break;
				}
		}
	}
	
	if (srcRefnum)
		FSClose(srcRefnum);
	if (dstRefnum)
		FSClose(dstRefnum);
	DisposePtr(inBuff);
	DisposePtr(outBuff);
	
	if (err)
	{
		SysBeep(1);
		FSpDelete(&reply.sfFile);
	}
}