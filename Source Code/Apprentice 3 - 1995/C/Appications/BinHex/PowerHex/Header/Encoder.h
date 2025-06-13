#ifndef	_ENCODER_
#define	_ENCODER_

#ifndef	_STATUS_
#include "Status.h"
#endif

#ifndef	_BIN2HEX_
#include "Bin2Hex.h"
#endif





enum {	kGETINFO = 0,
			kINFOWAIT,
			kSETUPFILE,
			kNEWBIN,
			kSTARTDATABIN,
			kREADDATABIN,
			kREADDATAWAIT,
			kCONVERTDATABIN,
			kWRITEDATABIN,
			kWRITEDATAWAIT,
			kSTOPDATABIN,
			kSTARTRESOURCEBIN,
			kREADRESOURCEBIN,
			kREADRESOURCEWAIT,
			kCONVERTRESOURCEBIN,
			kWRITERESOURCEBIN,
			kWRITERESOURCEWAIT,
			kSTOPRESOURCEBIN,
			kDONEBIN,
			kCANCEL,
			kDONE };


typedef struct EncoderInfo
{
	StatusInfoPtr	statusInfo;
	WindowPtr		status;
	long				state;
	short				createdFile;
	unsigned long	lastSize;
	
	FSSpec			source;
	FSSpec			destination;
	short				srcRefNum;
	short				destRefNum;
	
	HFileParam 		hfp;
	Bin2HexParam	bhp;
	Handle			buffers;
	
	IOParam			wpb;
	IOParam			rpb;
	
	unsigned char	*inBuffer;
	unsigned char	*outBuffer;
	
	unsigned long				chunkSize;
	unsigned long				outOffset;
	unsigned long				inOffset;
	unsigned long				inTotal;
	unsigned long				outTotal;
	unsigned long				inSize;
	unsigned long				outSize;
	unsigned long				lastInAmount;
} EncoderInfo, *EncoderInfoPtr;


void DoEncode(FSSpecPtr	spec);
void DoEncodeKill(EncoderInfoPtr info);
short DoEncodeWork(EncoderInfoPtr info);
short DoGetInfo(EncoderInfoPtr info);
short DoInfoWait(EncoderInfoPtr info);
short	DoSetupFile(EncoderInfoPtr info);
short DoNewBin(EncoderInfoPtr info);
short DoStartDataBin(EncoderInfoPtr info);
short DoReadDataBin(EncoderInfoPtr info);
short DoReadDataWait(EncoderInfoPtr info);
short DoConvertDataBin(EncoderInfoPtr info);
short DoWriteDataBin(EncoderInfoPtr info);
short DoWriteDataWait(EncoderInfoPtr info);
short DoStopDataBin(EncoderInfoPtr info);
short DoStartResourceBin(EncoderInfoPtr info);
short DoReadResourceBin(EncoderInfoPtr info);
short DoReadResourceWait(EncoderInfoPtr info);
short DoConvertResourceBin(EncoderInfoPtr info);
short DoWriteResourceBin(EncoderInfoPtr info);
short DoWriteResourceWait(EncoderInfoPtr info);
short DoStopResourceBin(EncoderInfoPtr info);
short DoDoneBin(EncoderInfoPtr info);
short DoCancel(EncoderInfoPtr info);


#endif	_ENCODER_