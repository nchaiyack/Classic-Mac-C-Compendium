#include <Script.h>
#include <string.h>
#include "Event.h"
#include "Status.h"
#include "Encoder.h"





#define	kChunkSize	25*1024





void DoEncode(FSSpecPtr	spec)
{
	EncoderInfoPtr	info;
	
	
	info = (EncoderInfoPtr)NewPtrClear(sizeof(EncoderInfo));
	info->status = NewStatusWindow("Encode BinHex",&info->statusInfo);
	NewWorker((WorkProc)DoEncodeWork,(KillProc)DoEncodeKill,info);
	
	strcpy(info->statusInfo->action,"Encoding");
	spec->name[spec->name[0]+1] = '\0';
	strcpy(info->statusInfo->file,(char*)&spec->name[1]);
	
	info->source = *spec;
	info->state = kGETINFO;
}





void DoEncodeKill(EncoderInfoPtr info)
{
	if (info)
	{
		if (info->status)
			DisposeStatusWindow(info->status);

		DisposePtr((Ptr)info);
	}
}





short DoEncodeWork(EncoderInfoPtr info)
{
	if (info->statusInfo->cancel)
		info->state = kCANCEL;
	
	switch(info->state)
	{
		case kGETINFO:
			info->state = DoGetInfo(info);
			break;
		case kINFOWAIT:
			info->state = DoInfoWait(info);
			break;
		case kSETUPFILE:
			info->state = DoSetupFile(info);
			break;
		case kNEWBIN:
			info->state = DoNewBin(info);
			break;
		case kSTARTDATABIN:
			info->state = DoStartDataBin(info);
			break;
		case kREADDATABIN:
			info->state = DoReadDataBin(info);
			break;
		case kREADDATAWAIT:
			info->state = DoReadDataWait(info);
			break;
		case kCONVERTDATABIN:
			info->state = DoConvertDataBin(info);
			break;
		case kWRITEDATABIN:
			info->state = DoWriteDataBin(info);
			break;
		case kWRITEDATAWAIT:
			info->state = DoWriteDataWait(info);
			break;
		case kSTOPDATABIN:
			info->state = DoStopDataBin(info);
			break;
		case kSTARTRESOURCEBIN:
			info->state = DoStartResourceBin(info);
			break;
		case kREADRESOURCEBIN:
			info->state = DoReadResourceBin(info);
			break;
		case kREADRESOURCEWAIT:
			info->state = DoReadResourceWait(info);
			break;
		case kCONVERTRESOURCEBIN:
			info->state = DoConvertResourceBin(info);
			break;
		case kWRITERESOURCEBIN:
			info->state = DoWriteResourceBin(info);
			break;
		case kWRITERESOURCEWAIT:
			info->state = DoWriteResourceWait(info);
			break;
		case kSTOPRESOURCEBIN:
			info->state = DoStopResourceBin(info);
			break;
		case kDONEBIN:
			info->state = DoDoneBin(info);
			break;
		case kCANCEL:
			info->state = DoCancel(info);
			break;
	}
	
	return (info->state == kDONE);
}





short DoGetInfo(EncoderInfoPtr info)
{
	info->hfp.ioCompletion = NULL;
	info->hfp.ioNamePtr = info->source.name;
	info->hfp.ioVRefNum = info->source.vRefNum;
	info->hfp.ioFVersNum = 0;
	info->hfp.ioFDirIndex = 0;
	info->hfp.ioDirID = info->source.parID;
	PBHGetFInfo((HParmBlkPtr)&info->hfp,true);
	return kINFOWAIT;
}





short DoInfoWait(EncoderInfoPtr info)
{
	unsigned long	maxfreemem,wanted;
	short	err;
	
	
	if (info->hfp.ioResult == 1)
		return kINFOWAIT;
	
	maxfreemem = TempFreeMem();
	wanted = info->hfp.ioFlLgLen+info->hfp.ioFlRLgLen;
	wanted = wanted + wanted*3/2;
	
	if (maxfreemem < 10240)
		return kCANCEL;
	else
		maxfreemem = (maxfreemem/2) < wanted ? (maxfreemem/2) : wanted;
	
	info->buffers = TempNewHandle(maxfreemem,&err);
	if (err)
	{
		info->buffers = NULL;
		return kCANCEL;
	}
	
	HLock(info->buffers);
	
	BlockMove(info->source.name,info->bhp.info.name,info->source.name[0]+1);
	info->bhp.info.type = info->hfp.ioFlFndrInfo.fdType;
	info->bhp.info.creator = info->hfp.ioFlFndrInfo.fdCreator;
	info->bhp.info.flags = info->hfp.ioFlFndrInfo.fdFlags;
	info->bhp.info.dfLength = info->hfp.ioFlLgLen;
	info->bhp.info.rfLength = info->hfp.ioFlRLgLen;
	info->bhp.addLFs = FALSE;
	
	info->inSize = info->bhp.inBufferSize = maxfreemem*2/5;
	info->outSize = info->bhp.outBufferSize = maxfreemem*3/5;
	info->bhp.inBuffer = (unsigned char*)*info->buffers+maxfreemem*3/5;
	info->bhp.outBuffer = (unsigned char*)*info->buffers;
	
	info->inBuffer = info->bhp.inBuffer;
	info->outBuffer = info->bhp.outBuffer;
	
	info->wpb.ioBuffer = *info->buffers;
	info->wpb.ioResult = 0;
	info->rpb.ioResult = 0;
	info->chunkSize = kChunkSize > info->inSize ? info->inSize : kChunkSize;
	
	return kSETUPFILE;
}





short	DoSetupFile(EncoderInfoPtr info)
{
	StandardFileReply	reply;
	FSSpec				test;
	short					err,ask = 0;
	
	
	info->destination = info->source;;
	BlockMove(".hqx",info->destination.name+1+info->destination.name[0],4);
	info->destination.name[0] += 4;
	
	if (info->destination.name[0] > 31)
	{
		info->destination.name[0] = 31;
		ask++;
	}
	
	err = FSMakeFSSpec(info->destination.vRefNum,info->destination.parID,info->destination.name,&test);
	if (!err || (err && (err != fnfErr)))
		ask++;
	
	if (ask)
	{
		StandardPutFile("\p",info->destination.name,&reply);
		if (!reply.sfGood)
			return kCANCEL;
		
		info->destination = reply.sfFile;
	}
	
	FSpDelete(&info->destination);
	if (FSpCreate(&info->destination,'MMCC','TEXT',smSystemScript))
		return kCANCEL;
	
	info->createdFile = 1;
	info->statusInfo->total = info->hfp.ioFlLgLen + info->hfp.ioFlRLgLen;
	info->statusInfo->current = info->statusInfo->total;
	
	ShowWindow(info->status);
	return kNEWBIN;
}





short DoNewBin(EncoderInfoPtr info)
{
	FSpOpenDF(&info->destination,fsRdWrPerm,&info->destRefNum);
	if (!NewBin2Hex(&info->bhp))
	{
		FSWrite(info->destRefNum,&info->bhp.outAmount,info->bhp.outBuffer);
		return kSTARTDATABIN;
	}
	
	return kCANCEL;
}





short DoStartDataBin(EncoderInfoPtr info)
{
	if (info->bhp.info.dfLength)
	{
		FSpOpenDF(&info->source,fsRdPerm,&info->srcRefNum);
		SetFPos(info->srcRefNum,fsFromStart,0);
		return kREADDATABIN;
	}
	
	return kSTARTRESOURCEBIN;
}





short DoReadDataBin(EncoderInfoPtr info)
{
	if (info->rpb.ioResult == 1)
		return kREADDATABIN;
	
	info->rpb.ioCompletion = NULL;
	info->rpb.ioRefNum = info->srcRefNum;
	info->rpb.ioReqCount = info->bhp.inBufferSize;
	info->rpb.ioBuffer = (Ptr)info->inBuffer;
	info->rpb.ioPosMode = fsFromMark;
	info->rpb.ioPosOffset = 0;
	PBRead((ParmBlkPtr)&info->rpb,true);
	return kREADDATAWAIT;
}





short DoReadDataWait(EncoderInfoPtr info)
{
	if (info->rpb.ioResult == 1)
		return kREADDATAWAIT;
	
	info->bhp.inOffset = 0;
	info->bhp.inAmount = info->chunkSize > info->rpb.ioActCount ? info->rpb.ioActCount : info->chunkSize;
	info->lastInAmount = info->bhp.inAmount;
	info->inTotal = info->rpb.ioActCount;
	info->outTotal = 0;
	info->inOffset = 0;
	info->outOffset = 0;
	
	if (!info->statusInfo->start)
		info->statusInfo->start = TickCount();
	
	if (info->bhp.inAmount)
		return kCONVERTDATABIN;
	else
		return kSTOPDATABIN;
}





short DoConvertDataBin(EncoderInfoPtr info)
{
	short	done;
	
	
	if (info->wpb.ioResult == 1)
		return kCONVERTDATABIN;
	
	info->bhp.inBuffer = info->inBuffer + info->inOffset;
	info->bhp.inBufferSize = info->inSize - info->inOffset;
	info->bhp.outBuffer = info->outBuffer + info->outOffset;
	info->bhp.outBufferSize = info->outSize - info->outOffset;
	done = DataBin2Hex(&info->bhp);
	
	info->statusInfo->current = 2+info->bhp.context.rfState + info->bhp.context.dfState;
	UpdateStatusItem(info->status,0);
	
	if (!done && !info->bhp.inAmount)
	{
		info->outOffset += info->bhp.outAmount;
		info->inOffset += info->lastInAmount;
		info->inTotal -= info->lastInAmount;
		info->outTotal += info->bhp.outAmount;
		info->bhp.inAmount = info->inTotal;
		if (info->bhp.inAmount > info->chunkSize)
			info->bhp.inAmount = info->chunkSize;
		info->lastInAmount = info->bhp.inAmount;
		
		if (info->lastInAmount)
			return kCONVERTDATABIN;
		else
			return kWRITEDATABIN;
	}
	
	info->outOffset += info->bhp.outAmount;
	info->inOffset += info->chunkSize - info->bhp.inAmount;
	info->inTotal -= info->chunkSize - info->bhp.inAmount;
	info->outTotal += info->bhp.outAmount;
	
	return kWRITEDATABIN;
}





short DoWriteDataBin(EncoderInfoPtr info)
{
	if (info->wpb.ioResult == 1)
		return kWRITEDATABIN;
	
	info->wpb.ioCompletion = NULL;
	info->wpb.ioRefNum = info->destRefNum;
	info->wpb.ioReqCount = info->outTotal;
	info->wpb.ioBuffer = (Ptr)info->outBuffer;
	info->wpb.ioPosMode = fsFromMark;
	info->wpb.ioPosOffset = 0;
	PBWrite((ParmBlkPtr)&info->wpb,true);
	return kWRITEDATAWAIT;
}





short DoWriteDataWait(EncoderInfoPtr info)
{
	if (info->wpb.ioResult == 1)
		return kWRITEDATAWAIT;
	
	info->lastInAmount = info->bhp.inAmount;
	info->outTotal = 0;
	info->outOffset = 0;
	
	if (info->bhp.inAmount)
		return kCONVERTDATABIN;
	else
		return kREADDATABIN;
}





short DoStopDataBin(EncoderInfoPtr info)
{
	if (info->wpb.ioResult == 1)
		return kSTOPDATABIN;
	if (info->rpb.ioResult == 1)
		return kSTOPDATABIN;
	
	FSClose(info->srcRefNum);
	info->srcRefNum = 0;
	
	return kSTARTRESOURCEBIN;
}





short DoStartResourceBin(EncoderInfoPtr info)
{
	if (info->bhp.info.rfLength)
	{
		FSpOpenRF(&info->source,fsRdPerm,&info->srcRefNum);
		SetFPos(info->srcRefNum,fsFromStart,0);
		return kREADRESOURCEBIN;
	}
	
	return kDONEBIN;
}





short DoReadResourceBin(EncoderInfoPtr info)
{
	if (info->rpb.ioResult == 1)
		return kREADRESOURCEBIN;
	
	info->rpb.ioCompletion = NULL;
	info->rpb.ioRefNum = info->srcRefNum;
	info->rpb.ioReqCount = info->bhp.inBufferSize;
	info->rpb.ioBuffer = (Ptr)info->inBuffer;
	info->rpb.ioPosMode = fsFromMark;
	info->rpb.ioPosOffset = 0;
	PBRead((ParmBlkPtr)&info->rpb,true);
	return kREADRESOURCEWAIT;
}





short DoReadResourceWait(EncoderInfoPtr info)
{
	if (info->rpb.ioResult == 1)
		return kREADRESOURCEWAIT;
	
	info->bhp.inOffset = 0;
	info->bhp.inAmount = info->chunkSize > info->rpb.ioActCount ? info->rpb.ioActCount : info->chunkSize;
	info->lastInAmount = info->bhp.inAmount;
	info->inTotal = info->rpb.ioActCount;
	info->outTotal = 0;
	info->inOffset = 0;
	info->outOffset = 0;
	
	if (!info->statusInfo->start)
		info->statusInfo->start = TickCount();
	
	if (info->bhp.inAmount)
		return kCONVERTRESOURCEBIN;
	else
		return kSTOPRESOURCEBIN;
}





short DoConvertResourceBin(EncoderInfoPtr info)
{
	short	done;
	
	
	if (info->wpb.ioResult == 1)
		return kCONVERTRESOURCEBIN;
	
	info->bhp.inBuffer = info->inBuffer + info->inOffset;
	info->bhp.inBufferSize = info->inSize - info->inOffset;
	info->bhp.outBuffer = info->outBuffer + info->outOffset;
	info->bhp.outBufferSize = info->outSize - info->outOffset;
	done = ResourceBin2Hex(&info->bhp);
	
	info->statusInfo->current = 2+info->bhp.context.rfState + info->bhp.context.dfState;
	UpdateStatusItem(info->status,0);
	
	if (!done && !info->bhp.inAmount)
	{
		info->outOffset += info->bhp.outAmount;
		info->inOffset += info->lastInAmount;
		info->inTotal -= info->lastInAmount;
		info->outTotal += info->bhp.outAmount;
		info->bhp.inAmount = info->inTotal;
		if (info->bhp.inAmount > info->chunkSize)
			info->bhp.inAmount = info->chunkSize;
		info->lastInAmount = info->bhp.inAmount;
		
		if (info->lastInAmount)
			return kCONVERTRESOURCEBIN;
		else
			return kWRITERESOURCEBIN;
	}
	
	info->outOffset += info->bhp.outAmount;
	info->inOffset += info->chunkSize - info->bhp.inAmount;
	info->inTotal -= info->chunkSize - info->bhp.inAmount;
	info->outTotal += info->bhp.outAmount;
	return kWRITERESOURCEBIN;
}





short DoWriteResourceBin(EncoderInfoPtr info)
{
	if (info->wpb.ioResult == 1)
		return kWRITERESOURCEBIN;
	
	info->wpb.ioCompletion = NULL;
	info->wpb.ioRefNum = info->destRefNum;
	info->wpb.ioReqCount = info->outTotal;
	info->wpb.ioBuffer = (Ptr)info->outBuffer;
	info->wpb.ioPosMode = fsFromMark;
	info->wpb.ioPosOffset = 0;
	PBWrite((ParmBlkPtr)&info->wpb,true);
	return kWRITERESOURCEWAIT;
}





short DoWriteResourceWait(EncoderInfoPtr info)
{
	if (info->wpb.ioResult == 1)
		return kWRITERESOURCEWAIT;
	
	info->lastInAmount = info->bhp.inAmount;
	info->outTotal = 0;
	info->outOffset = 0;
	
	if (info->bhp.inAmount)
		return kCONVERTRESOURCEBIN;
	else
		return kREADRESOURCEBIN;
}





short DoStopResourceBin(EncoderInfoPtr info)
{
	if (info->wpb.ioResult == 1)
		return kSTOPRESOURCEBIN;
	if (info->rpb.ioResult == 1)
		return kSTOPRESOURCEBIN;
	
	FSClose(info->srcRefNum);
	info->srcRefNum = 0;
	
	return kDONEBIN;
}





short DoDoneBin(EncoderInfoPtr info)
{
	short	err;
	
	
	if (info->destRefNum)
		FSClose(info->destRefNum);
	info->destRefNum = 0;
	
	if (info->buffers)
	{
		HUnlock(info->buffers);
		TempDisposeHandle(info->buffers,&err);
	}
	
	return kDONE;
}





short DoCancel(EncoderInfoPtr info)
{
	short	err;
	
	
	if (info->wpb.ioResult == 1)
		return kCANCEL;
	if (info->rpb.ioResult == 1)
		return kCANCEL;
		
	if (info->destRefNum)
		FSClose(info->destRefNum);
	if (info->srcRefNum)
		FSClose(info->srcRefNum);
	
	if (info->createdFile)
		FSpDelete(&info->destination);
	
	if (info->buffers)
	{
		HUnlock(info->buffers);
		TempDisposeHandle(info->buffers,&err);
	}
	
	return kDONE;
}