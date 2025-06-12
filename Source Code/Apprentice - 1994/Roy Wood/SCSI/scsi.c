#include <stdio.h>
#include <SCSI.h>



enum	{ 
			RETRYEXPIRE = 1, SHORTRESPLENGTH
		};

#define kVendorIDSize 8
#define kProductIDSize 16
#define kRevisionSize 4

#define SCSICmd_TestUnitReady 0x00
#define SCSICmd_Format 0x04
#define SCSICmd_Read 0x08
#define SCSICmd_Write 0x0A
#define SCSICmd_Inquiry 0x12
#define SCSICmd_ReadCap 0x25
#define SCSICmd_Mode 0x15
#define SCSICmd_SendDiagnostic 0x1D
#define SCSICmd_ReceiveDiagnostic 0x1C
#define SCSICmd_RequestSense 0x03
#define SCSICmd_RezeroUnit 0x01


#define kIOTimeout (60 * 60) /* a full minute for I/O operations */
#define kFormatTimeout (60 * 60 * 60) /* an hour for formatting */
#define kShortTimeout (1 * 60) /* one second for commands that we know are easy */

#define kDoWrite true
#define kDoRead false
#define kDoBlind true
#define kDoPolled false
#define kHoldIt true
#define kUnholdIt false
#define kIgnored false
#define kNoBuffer NULL, 0, 0, kIgnored, kIgnored
#define kNoLoop 0




typedef short SCSIAddress;
typedef unsigned char SCSICommandBlock[6];


typedef struct SCSIResponseBlock 
{	// see ANSI SCSI standard for more info about these fields
		unsigned char fDeviceType;					// these two fields indentify the device currently 
		unsigned char fDeviceQualifier;				// connected to the logical unit
		unsigned char fVersion;						// this defines the version level (vendor + standard)
		unsigned char fResponseFormat;				// defines the used data format (SCSI INQUIRY or something else)		
		unsigned char fAdditionalLength;			// length of bytes of the parameters
		unsigned char fVendorUse1;					// vendor specific data
		unsigned short fReserved1;
		unsigned char fVendorID[kVendorIDSize];		// vendor ID data
		unsigned char fProductID[kProductIDSize];	// product data
		unsigned char fRevision[kRevisionSize];		// product version data
		unsigned char fVendorUse2[20];				// vendor specific data
		unsigned char fReserved2[42];
		unsigned char fVendorUse3[158];				// vendor specific data
};

	
typedef struct SCSIResponseBlock SCSIResponseBlock;


typedef struct SCSIModeBlock 
{
		unsigned char		reserved1;
		unsigned char		mediumType;
		unsigned char		reserved2;
		unsigned char		descLength;
		unsigned char		descriptor[8];
		unsigned char		pageDescriptor;
		unsigned char		pageLength;
		unsigned char		opParams;
		unsigned char		devTypeQualifier;
};

	
typedef struct SCSIModeBlock SCSIModeBlock;






typedef struct SCSIFormatBlock
{
	unsigned char	reserved1;
	unsigned char	formatOptions;
	unsigned char	defectListMSB;
	unsigned char	defectListLSB;
};

typedef struct SCSIFormatBlock SCSIFormatBlock;






typedef struct SCSIReqSenseBlock
{
	unsigned char	errorCode;
	unsigned char	segNum;
	unsigned char	senseKey;
	unsigned char	infoMSB;
	unsigned char	infoLSB;
	unsigned char	senseLength;
	unsigned char	commandInfoMSB;
	unsigned char	commandInfoLSB;
	unsigned char	senseCode;
	unsigned char	senseQualifier;
	unsigned char	unitCode;
	unsigned char	keySpecific1;
	unsigned char	keySpecific2;
	unsigned char	keySpecific3;
};

typedef struct SCSIReqSenseBlock SCSIReqSenseBlock;



typedef struct DeviceDescriptorMap
{
	unsigned int	sbSig;
	unsigned int	sbBlockSize;
	unsigned long	sbBlkCount;
	unsigned int	sbDevType;
	unsigned int	sbDevID;
	unsigned long	sbData;
	unsigned int	sbDrvrCount;
	
	unsigned long	ddBlock1;
	unsigned int	ddSize1;
	unsigned int	ddType1;
	
	unsigned long	ddBlock2;
	unsigned int	ddSize2;
	unsigned int	ddType2;
	
	unsigned long	ddBlock3;
	unsigned int	ddSize3;
	unsigned int	ddType3;
	
	unsigned long	ddBlock4;
	unsigned int	ddSize4;
	unsigned int	ddType4;
};

typedef struct DeviceDescriptorMap DeviceDescriptorMap;



typedef struct OldPartitionMap
{
	unsigned int	pdSig;
	
	unsigned long	pdStart1;
	unsigned long	pdSize1;
	unsigned long	pdFSID1;
	
	unsigned long	pdStart2;
	unsigned long	pdSize2;
	unsigned long	pdFSID2;
	
	unsigned long	pdStart3;
	unsigned long	pdSize3;
	unsigned long	pdFSID3;
	
	unsigned long	pdStart4;
	unsigned long	pdSize4;
	unsigned long	pdFSID4;
	
	unsigned long	pdStart5;
	unsigned long	pdSize5;
	unsigned long	pdFSID5;
};

typedef struct OldPartitionMap OldPartitionMap;

typedef SCSIInstr TransferInstructionBlock[];






char	errorMessage[256];

OSErr CheckSCSIDevice(SCSIAddress device);
OSErr SCSIDoInquiryCommand(SCSIAddress device);
void StuffSCSICommandBlock(SCSICommandBlock cmd, unsigned char a, unsigned char b,unsigned char c, unsigned char d, unsigned char e, unsigned char f);
OSErr SCSIOp(SCSICommandBlock cmd, short cmdSize, SCSIAddress targetSCSIID, unsigned long completionTimeout, void *buffer, unsigned long blockSize, unsigned long loopCount, Boolean writing, Boolean blind);
OSErr SetDeviceMode(SCSIAddress device);
OSErr PerformDiagnostic(SCSIAddress device);
OSErr FormatDevice(SCSIAddress device);
void ZeroStructure(Ptr theStruct,long structSize);
OSErr WriteDevicePartition(SCSIAddress device);
OSErr ReadDevicePartition(SCSIAddress device);
OSErr BorrowBlocks(SCSIAddress device);



void StuffSCSICommandBlock(SCSICommandBlock cmd, unsigned char a, unsigned char b,unsigned char c, unsigned char d, unsigned char e, unsigned char f)
{
	cmd[0] = a;
	cmd[1] = b;
	cmd[2] = c;
	cmd[3] = d;
	cmd[4] = e;
	cmd[5] = f;
}



void StuffTransferInstructionBlock(TransferInstructionBlock tib, void *buffer, unsigned long blockSize, unsigned long loopCount)
{
register SCSIInstr *instr;
	
	
	instr = tib;
	
	instr->scOpcode = scInc;
	
	instr->scParam1 = (unsigned long) buffer;
	
	(instr++)->scParam2 = blockSize;
	
	if (loopCount != 0)
	{
		instr->scOpcode = scLoop;
		instr->scParam1 = - sizeof(SCSIInstr);
		(instr++)->scParam2 = loopCount;
	}
	
	instr->scOpcode = scStop;
	
	instr->scParam1 = 0;
	instr->scParam2 = 0;
}





OSErr SCSITestUnitReady(SCSIAddress device)
{
OSErr 				errCode;
short				retries;
SCSICommandBlock 	testCmd;
	
	
	errCode = noErr;
	
	StuffSCSICommandBlock(testCmd, SCSICmd_TestUnitReady, 0, 0, 0, 0, 0);
	
	retries = 3;
	
	do 
	{
		errCode = SCSIOp(testCmd, sizeof(SCSICommandBlock), device, kShortTimeout,NULL, 0, 0, kIgnored, kIgnored);
		
		if (errCode == noErr)
		{
			goto EXITPOINT;
		}
		
		retries--;
		
	} while (retries > 0);
	
	
	if (retries <= 0)
	{
		sprintf(errorMessage,"SCSITestUnitReady: retry count expired without response");
		errCode = RETRYEXPIRE;
		goto EXITPOINT;
	}
	

EXITPOINT:

	return(errCode);
}





OSErr SCSIDoInquiryCommand(SCSIAddress device)
{
OSErr				errCode;
SCSICommandBlock 	inquiryCmdBlock;
SCSIResponseBlock	inquiryResponse;
short 				minLength;
int					i;


	errCode = noErr;

	ZeroStructure((Ptr) &inquiryResponse, sizeof(inquiryResponse));
	
	minLength = (int) ((long) &(inquiryResponse.fProductID) - (long) &(inquiryResponse.fDeviceType));
		
	
	StuffSCSICommandBlock(inquiryCmdBlock, SCSICmd_Inquiry, 0, 0, 0, 5, 0);
	
	errCode = SCSIOp(inquiryCmdBlock, sizeof(SCSICommandBlock), device, kShortTimeout, &inquiryResponse, 5, kNoLoop, kDoRead, kDoPolled);
	
	printf("inquiryResponse.fAdditionalLength + 4 = %d, displayed data includes chars 0-%d\n\n",(int) (inquiryResponse.fAdditionalLength + 4),(int) ((long) &(inquiryResponse.fVendorUse2) - (long) &(inquiryResponse.fDeviceType) - 1));
	
	
	if (inquiryResponse.fAdditionalLength < minLength)
	{
		sprintf(errorMessage,"SCSIDoInquiryCommand: inquiryResponse.fAdditionalLength = %d, expected %d",(int) (inquiryResponse.fAdditionalLength),(int) minLength);
		errCode = SHORTRESPLENGTH;
		goto EXITPOINT;
	}
	
	if ((errCode != noErr) || (inquiryResponse.fAdditionalLength < minLength))  
		goto EXITPOINT;
	
	
	StuffSCSICommandBlock(inquiryCmdBlock, SCSICmd_Inquiry, 0, 0, 0, inquiryResponse.fAdditionalLength, 0);
	
	errCode = SCSIOp(inquiryCmdBlock, sizeof(SCSICommandBlock), device, kShortTimeout, &inquiryResponse, inquiryResponse.fAdditionalLength, kNoLoop, kDoRead, kDoPolled);
	
	if (errCode != noErr)
		goto EXITPOINT;
	
	
	
	printf("Peripheral Qualifier = 0x%02X : ",(int) (inquiryResponse.fDeviceType >> 5));
	
	switch (inquiryResponse.fDeviceType >> 5)
	{
		case 0:
			printf("device connected\n");
			break;
		
		case 1:
			printf("device not currently connected\n");
			break;
			
		case 2:
			printf("Reserved\n");
			break;
		
		case 3:
			printf("unsupported device\n");
			break;
			
		default:
			printf("vendor specific\n");
			break;
	} 
	
	
	printf("Peripheral Device Type = 0x%02X : ",(int) (inquiryResponse.fDeviceType & 0x1F));
	
	switch(inquiryResponse.fDeviceType & 0x1F)
	{
		case 0:
			printf("direct access device\n");
			break;
		
		case 5:
			printf("CD ROM device\n");
			break;
		
		default:
			printf("other device\n");
			break;
	}
	
	
	if (inquiryResponse.fDeviceQualifier & 0x80)
	{
		printf("Device Qualifier RMB = removable media\n");
	}
	
	else
	{
		printf("Device Qualifier RMB = non-removable media\n");
	}
	
	
	printf("Vendor ID: ");
	
	for (i=0;i<kVendorIDSize;i++)
		printf("%c",inquiryResponse.fVendorID[i]);
		
	printf("\n");
	
	
	printf("Product ID: ");
	
	for (i=0;i<kProductIDSize;i++)
		printf("%c",inquiryResponse.fProductID[i]);
		
	printf("\n");
	
	
	printf("Product ID: ");
	
	for (i=0;i<kRevisionSize;i++)
		printf("%c",inquiryResponse.fRevision[i]);
		
	printf("\n");
	
	
EXITPOINT:

	return(errCode);
}





OSErr SetDeviceMode(SCSIAddress device)
{
OSErr				errCode;
SCSICommandBlock 	modeCmdBlock;
SCSIModeBlock		modeDataBlock;
int					i;

	errCode = noErr;
	
	StuffSCSICommandBlock(modeCmdBlock, SCSICmd_Mode, 0, 0, 0, 16, 0);
	
	modeDataBlock.reserved1 = 0x00;
	modeDataBlock.mediumType = 0x00;
	modeDataBlock.reserved2 = 0x00;
	modeDataBlock.descLength = 0x08;
	
	for (i=0;i<8;i++)
		modeDataBlock.descriptor[i] = 0x00;
	
	modeDataBlock.pageDescriptor = 0x00;
	modeDataBlock.pageLength = 0x02;
	modeDataBlock.opParams = 0x10;
	modeDataBlock.devTypeQualifier = 0x00;
	
	errCode = SCSIOp(modeCmdBlock, sizeof(SCSICommandBlock), device, kShortTimeout, &modeDataBlock, sizeof(SCSIModeBlock), kNoLoop, kDoWrite, kDoPolled);
	
	return(errCode);
}





OSErr CheckSCSIDevice(SCSIAddress device) 
{
OSErr errCode = noErr;

	errCode = SCSITestUnitReady(device);
	
	if (errCode != noErr)
		goto EXITPOINT;
		
	errCode = SCSIDoInquiryCommand(device);
	
EXITPOINT:

	return(errCode);
}






OSErr SCSIOp(SCSICommandBlock cmd, short cmdSize, SCSIAddress targetSCSIID, unsigned long completionTimeout, void *buffer, unsigned long blockSize, unsigned long loopCount, Boolean writing, Boolean blind)
{
SCSIInstr	tib[3];
short 		status, message;
OSErr 		errCode;
char		gotBus;


	gotBus = false;
	errCode = noErr;
	
	
	if (buffer != NULL)
		StuffTransferInstructionBlock(tib, buffer, blockSize, loopCount);
	
	
	errCode = SCSIGet();
	
	if (errCode != noErr) 
	{
		sprintf(errorMessage,"SCSIOp: SCSIGet() error %d",errCode);
		goto EXITPOINT;
	}
	
	gotBus = true;
	
	errCode = SCSISelect(targetSCSIID);
	
	if (errCode != noErr)
	{
		sprintf(errorMessage,"SCSIOp: SCSISelect(%d) error %d",(int) (targetSCSIID),errCode);
		goto EXITPOINT;
	}
	
	
	errCode = SCSICmd((Ptr) cmd, cmdSize);
	
	if (errCode != noErr)
	{
		sprintf(errorMessage,"SCSIOp: SCSICmd() error %d",errCode);
		goto EXITPOINT;
	}
	
	if (buffer != NULL)
	{
		if (writing)
		{
			if (blind)
			{
				errCode = SCSIWBlind((Ptr) tib);
				
				if (errCode != noErr)
				{
					sprintf(errorMessage,"SCSIOp: SCSIWBlind() error %d",errCode);
					goto EXITPOINT;
				}
			}
			
			else
			{
				errCode = SCSIWrite((Ptr) tib);
				
				if (errCode != noErr)
				{
					sprintf(errorMessage,"SCSIOp: SCSIWrite() error %d",errCode);
					goto EXITPOINT;
				}
			}
		}
		
		else
		{
			if (blind)
			{
				errCode = SCSIRBlind((Ptr) tib);
				
				if (errCode != noErr)
				{
					sprintf(errorMessage,"SCSIOp: SCSIRBlind() error %d",errCode);
					goto EXITPOINT;
				}
			}
			
			else
			{
				errCode = SCSIRead((Ptr) tib);
				
				if (errCode != noErr)
				{
					sprintf(errorMessage,"SCSIOp: SCSIRead() error %d",errCode);
					goto EXITPOINT;
				}
			}
		}
	}
	
	
EXITPOINT:
	
	if (gotBus)
	{
		if (errCode == noErr)
		{
			errCode = SCSIComplete(&status, &message, completionTimeout);
			
			if (errCode != noErr)
			{
				sprintf(errorMessage,"SCSIOp: SCSIComplete() error %d",errCode);
			}
		}
		
		else
			SCSIComplete(&status, &message, completionTimeout);
	}


	//	Attempt to return the most valuable result possible.  A SCSI Manager error is
	//	negated to distinguish it from a positive SCSI status byte code, which may be
	//	returned for a failed transaction even if the SCSI Manager is "successful."
	//
	//	If there were no errors from the SCSI Manager, the result is the SCSI status
	//	byte, otherwise it is the negation of the SCSI Manager error.
	//
	
	if (errCode == noErr)
	{
		sprintf(errorMessage,"SCSIOp: device returned status = %d, message = %d",(int) status,(int) message);
		
		return(status);
	}
	
	else
		return(errCode);
}




OSErr FormatDevice(SCSIAddress device)
{
OSErr				errCode;
SCSICommandBlock 	formatCmdBlock;

	errCode = noErr;
	
	StuffSCSICommandBlock(formatCmdBlock, SCSICmd_Format, 0, 0, 0, 0x03, 0);
	
	errCode = SCSIOp(formatCmdBlock, sizeof(SCSICommandBlock), device, kFormatTimeout, NULL, 0, 0, kIgnored, kIgnored);
	
	return(errCode);
}




OSErr PerformDiagnostic(SCSIAddress device)
{
OSErr				errCode;
SCSICommandBlock 	sendDiagnosticCmdBlock;
SCSICommandBlock 	receiveDiagnosticCmdBlock;

	errCode = noErr;
	
	StuffSCSICommandBlock(sendDiagnosticCmdBlock, SCSICmd_SendDiagnostic, 0x04, 0, 0, 0, 0x01);
	
	errCode = SCSIOp(sendDiagnosticCmdBlock, sizeof(SCSICommandBlock), device, kFormatTimeout, NULL, 0, 0, kIgnored, kIgnored);
	
	if (errCode != noErr)
		goto EXITPOINT;


	StuffSCSICommandBlock(receiveDiagnosticCmdBlock, SCSICmd_ReceiveDiagnostic, 0x04, 0, 0, 0, 0);
	
	errCode = SCSIOp(receiveDiagnosticCmdBlock, sizeof(SCSICommandBlock), device, kFormatTimeout, NULL, 0, 0, kIgnored, kIgnored);
	
	if (errCode != noErr)
		goto EXITPOINT;
		
EXITPOINT:

	return(errCode);
}





OSErr CheckDeviceCondition(SCSIAddress device)
{
OSErr				errCode;
SCSICommandBlock 	checkConditionCmdBlock;
SCSIReqSenseBlock	reqSenseDataBlock;

	errCode = noErr;
	
	ZeroStructure((Ptr) &reqSenseDataBlock, sizeof(reqSenseDataBlock));
	
	StuffSCSICommandBlock(checkConditionCmdBlock, SCSICmd_RequestSense, 0, 0, 0, sizeof(SCSIReqSenseBlock), 0);
	
	errCode = SCSIOp(checkConditionCmdBlock, sizeof(SCSICommandBlock), device, kIOTimeout, &reqSenseDataBlock, sizeof(SCSIReqSenseBlock), kNoLoop, kDoRead, kDoPolled);
	
	if (errCode != noErr)
		goto EXITPOINT;

	printf("\n\n");
	
	if (reqSenseDataBlock.errorCode & 0x80 == 0)
		printf("Uh oh-- reqSenseDataBlock.errorCode & 0x80 == 0, returned sense data is probably invalid\n\n");
	
	printf("errorCode = 0x%02X\n",(int) (reqSenseDataBlock.errorCode & 0x7F));
	
	printf("segment number = 0x%02X\n",(int) (reqSenseDataBlock.segNum));
	
	printf("filemark = 0x%02X\n",(int) ((reqSenseDataBlock.senseKey & 0x80) >> 7));
	
	printf("EOM = 0x%02X\n",(int) ((reqSenseDataBlock.senseKey & 0x40) >> 6));
	printf("ILI = 0x%02X\n",(int) ((reqSenseDataBlock.senseKey & 0x20) >> 5));
	
	printf("sense key = 0x%02X\n",(int) (reqSenseDataBlock.senseKey & 0x07));
	
	printf("information = 0x%04X\n",(unsigned int) ((unsigned int) reqSenseDataBlock.infoMSB) * 256 + ((unsigned int) reqSenseDataBlock.infoLSB));
	
	printf("additional sense length = 0x%02X\n",(int) (reqSenseDataBlock.senseLength));
	
	printf("command-specific information = 0x%04X\n",(unsigned int) ((unsigned int) reqSenseDataBlock.commandInfoMSB) * 256 + ((unsigned int) reqSenseDataBlock.commandInfoLSB));
	
	printf("additional sense code = 0x%02X\n",(int) (reqSenseDataBlock.senseCode));
	
	printf("additional sense code qualifier = 0x%02X\n",(int) (reqSenseDataBlock.senseQualifier));
	
	printf("unit code = 0x%02X\n",(int) (reqSenseDataBlock.unitCode));
	
	if ((reqSenseDataBlock.senseKey & 0x07) == 0x05)
	{
		printf("SKSV = 0x%02X\n",(int) ((reqSenseDataBlock.keySpecific1 & 0x80) >> 7));
		printf("C/D = 0x%02X\n",(int) ((reqSenseDataBlock.keySpecific1 & 0x40) >> 6));
		printf("BPV = 0x%02X\n",(int) ((reqSenseDataBlock.keySpecific1 & 0x08) >> 3));
		printf("bit pointer = 0x%02X\n",(int) (reqSenseDataBlock.keySpecific1 & 0x07));
	}
	
	else if ((reqSenseDataBlock.senseKey & 0x07) == 0x02)
	{
		printf("SKSV = 0x%02X\n",(int) ((reqSenseDataBlock.keySpecific1 & 0x80) >> 7));
		printf("progress = 0x%04X\n",(unsigned int) ((unsigned int) reqSenseDataBlock.keySpecific2) * 256 + ((unsigned int) reqSenseDataBlock.keySpecific3));
	}
	
	else if (((reqSenseDataBlock.senseKey & 0x07) == 1) ||
			((reqSenseDataBlock.senseKey & 0x07) == 4) ||
			((reqSenseDataBlock.senseKey & 0x07) == 3))
	{
		printf("SKSV = 0x%02X\n",(int) ((reqSenseDataBlock.keySpecific1 & 0x80) >> 7));
		printf("actual retry count = 0x%04X\n",(unsigned int) ((unsigned int) reqSenseDataBlock.keySpecific2) * 256 + ((unsigned int) reqSenseDataBlock.keySpecific3));
	}
	
EXITPOINT:
	
	return(errCode);
}




void ZeroStructure(Ptr theStruct,long structSize)
{
register char 	*ch;
register long	ci;

	if (theStruct)
	{
		ch = (char *) theStruct;
		ci = structSize;
		
		while (ci--)
			*ch = 0x00;
	}
}




OSErr WriteDevicePartition(SCSIAddress device)
{
OSErr				errCode;
SCSICommandBlock 	writeCmdBlock;
DeviceDescriptorMap	ddMap;
OldPartitionMap		partitionMap;

	errCode = noErr;
	
	ddMap.sbSig = 0x4552;
	ddMap.sbBlockSize = 512;
	ddMap.sbBlkCount = 40000L;
	ddMap.sbDevType = 0;
	ddMap.sbDevID = 0;
	ddMap.sbData = 0;
	ddMap.sbDrvrCount = 1;
	ddMap.ddBlock1 = 2;
	ddMap.ddSize1 = 3;
	ddMap.ddType1 = 1;
	ddMap.ddBlock2 = 0;
	ddMap.ddSize2 = 0;
	ddMap.ddType2 = 0;
	
	
	StuffSCSICommandBlock(writeCmdBlock, SCSICmd_Write, 0, 0, 0, 0x01, 0);
	
	errCode = SCSIOp(writeCmdBlock, sizeof(SCSICommandBlock), device, kIOTimeout, &ddMap, 512, kNoLoop, kDoWrite, kDoPolled);
	
	if (errCode != noErr)
	{
		errCode = CheckDeviceCondition(device);
		
		if (errCode != noErr)
			printf("%s\n",errorMessage);
	}

	partitionMap.pdSig = 0x5453;
	
	partitionMap.pdStart1 = 0;
	partitionMap.pdSize1 = 1;
	partitionMap.pdFSID1 = 'ddmp';
	
	partitionMap.pdStart2 = 2;
	partitionMap.pdSize2 = 1;
	partitionMap.pdFSID2 = 'dpmp';
	
	partitionMap.pdStart3 = 3;
	partitionMap.pdSize3 = 5;
	partitionMap.pdFSID3 = 'drvr';
	
	partitionMap.pdStart4 = 8;
	partitionMap.pdSize4 = 40000L - 8;
	partitionMap.pdFSID4 = 'TFS1';
	
	partitionMap.pdStart5 = 0;
	partitionMap.pdSize5 = 0;
	partitionMap.pdFSID5 = 0;
	
	
	StuffSCSICommandBlock(writeCmdBlock, SCSICmd_Write, 0, 0, 1, 0x01, 0);
	
	errCode = SCSIOp(writeCmdBlock, sizeof(SCSICommandBlock), device, kIOTimeout, &partitionMap, 512, kNoLoop, kDoWrite, kDoPolled);
	
	if (errCode != noErr)
	{
		errCode = CheckDeviceCondition(device);
		
		if (errCode != noErr)
			printf("%s\n",errorMessage);
	}

EXITPOINT:

	return(errCode);
}





OSErr ReadDevicePartition(SCSIAddress device)
{
OSErr				errCode;
SCSICommandBlock 	readCmdBlock;
DeviceDescriptorMap	*ddMap;
OldPartitionMap		*partitionMap;
char				buffer[512];

	errCode = noErr;
	
	ddMap = (DeviceDescriptorMap *) &buffer;
	partitionMap = (OldPartitionMap *) &buffer;
	
	StuffSCSICommandBlock(readCmdBlock, SCSICmd_Read, 0, 0, 0, 0x01, 0);
	
	errCode = SCSIOp(readCmdBlock, sizeof(SCSICommandBlock), device, kIOTimeout, &buffer, 512, kNoLoop, kDoRead, kDoPolled);
	
	if (errCode != noErr)
	{
		errCode = CheckDeviceCondition(device);
		
		if (errCode != noErr)
			printf("%s\n",errorMessage);
	}
	
	printf("\Device Descriptor Map (sector 0):\n");
	
	printf("ddMap.sbSig = 0x%04X (should be 0x4552)\n",ddMap->sbSig);
	printf("ddMap.sbBlockSize = 0x%04X\n",ddMap->sbBlockSize);
	printf("ddMap.sbBlkCount = 0x%08lX\n",ddMap->sbBlkCount);
	printf("ddMap.sbDevType = 0x%04X\n",ddMap->sbDevType);
	printf("ddMap.sbDevID = 0x%04X\n",ddMap->sbDevID);
	printf("ddMap.sbData = 0x%08lX\n",ddMap->sbData);
	printf("ddMap.sbDrvrCount = 0x%04X\n",ddMap->sbDrvrCount);
	
	printf("ddMap.ddBlock1 = 0x%08lX\n",ddMap->ddBlock1);
	printf("ddMap.ddSize1 = 0x%04X\n",ddMap->ddSize1);
	printf("ddMap.ddType1 = 0x%04X\n",ddMap->ddType1);
	
	printf("ddMap.ddBlock2 = 0x%08lX\n",ddMap->ddBlock2);
	printf("ddMap.ddSize2 = 0x%04X\n",ddMap->ddSize2);
	printf("ddMap.ddType2 = 0x%04X\n",ddMap->ddType2);
	
	printf("ddMap.ddBlock3 = 0x%08lX\n",ddMap->ddBlock3);
	printf("ddMap.ddSize3 = 0x%04X\n",ddMap->ddSize3);
	printf("ddMap.ddType3 = 0x%04X\n",ddMap->ddType3);
	
	printf("ddMap.ddBlock4 = 0x%08lX\n",ddMap->ddBlock4);
	printf("ddMap.ddSize4 = 0x%04X\n",ddMap->ddSize4);
	printf("ddMap.ddType4 = 0x%04X\n",ddMap->ddType4);
	
	printf("\n");
	
	
	
	StuffSCSICommandBlock(readCmdBlock, SCSICmd_Read, 0, 0, 1, 0x01, 0);
	
	errCode = SCSIOp(readCmdBlock, sizeof(SCSICommandBlock), device, kIOTimeout, &buffer, 512, kNoLoop, kDoRead, kDoPolled);
	
	if (errCode != noErr)
	{
		errCode = CheckDeviceCondition(device);
		
		if (errCode != noErr)
			printf("%s\n",errorMessage);
	}
	
	printf("\nPartition Map (sector 1): \n");
	
	printf("partitionMap.pdSig = 0x%04X (should be 0x5453)\n",partitionMap->pdSig);
	
	printf("partitionMap.pdStart1 = 0x%08lX\n",partitionMap->pdStart1);
	printf("partitionMap.pdSize1 = 0x%08lX\n",partitionMap->pdSize1);
	printf("partitionMap.pdFSID1 = %c%c%c%c\n",((char *) &(partitionMap->pdFSID1))[0],((char *) &(partitionMap->pdFSID1))[1],((char *) &(partitionMap->pdFSID1))[2],((char *) &(partitionMap->pdFSID1))[3]);
	
	printf("partitionMap.pdStart2 = 0x%08lX\n",partitionMap->pdStart2);
	printf("partitionMap.pdSize2 = 0x%08lX\n",partitionMap->pdSize2);
	printf("partitionMap.pdFSID2 = %c%c%c%c\n",((char *) &(partitionMap->pdFSID2))[0],((char *) &(partitionMap->pdFSID2))[1],((char *) &(partitionMap->pdFSID2))[2],((char *) &(partitionMap->pdFSID2))[3]);
	
	printf("partitionMap.pdStart3 = 0x%08lX\n",partitionMap->pdStart3);
	printf("partitionMap.pdSize3 = 0x%08lX\n",partitionMap->pdSize3);
	printf("partitionMap.pdFSID3 = %c%c%c%c\n",((char *) &(partitionMap->pdFSID3))[0],((char *) &(partitionMap->pdFSID3))[1],((char *) &(partitionMap->pdFSID3))[2],((char *) &(partitionMap->pdFSID3))[3]);
	
	printf("partitionMap.pdStart4 = 0x%08lX\n",partitionMap->pdStart4);
	printf("partitionMap.pdSize4 = 0x%08lX\n",partitionMap->pdSize4);
	printf("partitionMap.pdFSID4 = %c%c%c%c\n",((char *) &(partitionMap->pdFSID4))[0],((char *) &(partitionMap->pdFSID4))[1],((char *) &(partitionMap->pdFSID4))[2],((char *) &(partitionMap->pdFSID4))[3]);
	
	printf("partitionMap.pdStart5 = 0x%08lX\n",partitionMap->pdStart5);
	printf("partitionMap.pdSize5 = 0x%08lX\n",partitionMap->pdSize5);
	printf("partitionMap.pdFSID5 = %c%c%c%c\n",((char *) &(partitionMap->pdFSID5))[0],((char *) &(partitionMap->pdFSID5))[1],((char *) &(partitionMap->pdFSID5))[2],((char *) &(partitionMap->pdFSID5))[3]);
	
	
	printf("\n");
}





OSErr BorrowBlocks(SCSIAddress device)
{
OSErr				errCode;
SCSICommandBlock 	readCmdBlock;
SCSICommandBlock 	writeCmdBlock;
char				buffer[512];
unsigned int		blockNum;

	errCode = noErr;
	
	for (blockNum = 0; blockNum < 255;blockNum++)
	{
		StuffSCSICommandBlock(readCmdBlock, SCSICmd_Read, 0, 0, blockNum, 0x01, 0);
		
		errCode = SCSIOp(readCmdBlock, sizeof(SCSICommandBlock), 0, kIOTimeout, &buffer, 512, kNoLoop, kDoRead, kDoPolled);
		
		if (errCode != noErr)
		{
			errCode = CheckDeviceCondition(device);
			
			if (errCode != noErr)
				printf("%s\n",errorMessage);
			
			goto EXITPOINT;
		}
		
		
		StuffSCSICommandBlock(writeCmdBlock, SCSICmd_Write, 0, 0, blockNum, 0x01, 0);
		
		errCode = SCSIOp(writeCmdBlock, sizeof(SCSICommandBlock), device, kIOTimeout, &buffer, 512, kNoLoop, kDoWrite, kDoPolled);
		
		if (errCode != noErr)
		{
			errCode = CheckDeviceCondition(device);
			
			if (errCode != noErr)
				printf("%s\n",errorMessage);
			
			goto EXITPOINT;
		}
	}
	
	
EXITPOINT:

	return(errCode);
}




OSErr RezeroDevice(SCSIAddress device)
{
OSErr				errCode;
SCSICommandBlock 	rezeroCmdBlock;

	errCode = noErr;
	
	StuffSCSICommandBlock(rezeroCmdBlock, SCSICmd_RezeroUnit, 0, 0, 0, 0, 0);
	
	errCode = SCSIOp(rezeroCmdBlock, sizeof(SCSICommandBlock), device, kIOTimeout, NULL, 0, kIgnored, kIgnored, kIgnored);
	
	if (errCode != noErr)
	{
		errCode = CheckDeviceCondition(device);
		
		if (errCode != noErr)
			printf("%s\n",errorMessage);
	}

	return(errCode);
}



main()
{
int		i;
OSErr	errCode;

	printf("SCSI.¹\n\n");
	
	printf("resize window and press a key to continue\n\n");
	
	getchar();
	
	i = 6;
	
	printf("\n\n");
	

	printf("Rezeroing Unit\n");
	
	errCode = RezeroDevice(i);
	
	if (errCode != noErr)
		printf("%s\n",errorMessage);
	
	if (errCode == 2)
	{
		errCode = CheckDeviceCondition(i);
		
		if (errCode != noErr)
			printf("%s\n",errorMessage);
	}
	
	
	
	errCode = BorrowBlocks(i);
	

	printf("Setting mode for device %d\n",i);
	
	errCode = SetDeviceMode(i);
	
	if (errCode != noErr)
		printf("%s\n",errorMessage);
	
	if (errCode == 2)
	{
		errCode = CheckDeviceCondition(i);
		
		if (errCode != noErr)
			printf("%s\n",errorMessage);
	}
	
	
	
	printf("Formatting device %d\n",i);
	
	errCode = FormatDevice(i);
	
	if (errCode != noErr)
		printf("%s\n",errorMessage);
	
	if (errCode == 2)
	{
		errCode = CheckDeviceCondition(i);
		
		if (errCode != noErr)
			printf("%s\n",errorMessage);
	}
	
	
	
	
	printf("Setting mode for device %d\n",i);
	
	errCode = SetDeviceMode(i);
	
	if (errCode != noErr)
		printf("%s\n",errorMessage);
	
	if (errCode == 2)
	{
		errCode = CheckDeviceCondition(i);
		
		if (errCode != noErr)
			printf("%s\n",errorMessage);
	}
	
	
	
	printf("\n\nChecking SCSI device #%d\n\n",i);
	
	errCode = CheckSCSIDevice(i);
	
	printf("\n");
	
	if (errCode != noErr)
		printf("SCSI device #%d error: %s\n",i,errorMessage);
	
	if (errCode == 2)
	{
		errCode = CheckDeviceCondition(i);
		
		if (errCode != noErr)
			printf("%s\n",errorMessage);
	}






	printf("Rezeroing Unit\n");
	
	errCode = RezeroDevice(i);
	
	if (errCode != noErr)
		printf("%s\n",errorMessage);
	
	if (errCode == 2)
	{
		errCode = CheckDeviceCondition(i);
		
		if (errCode != noErr)
			printf("%s\n",errorMessage);
	}
	
	
	
	
	printf("\n\nWriting device descriptor and partition maps to device %d\n\n",i);
	
	errCode = WriteDevicePartition(i);
	
	printf("\n");
	
	if (errCode != noErr)
		printf("SCSI device #%d error: %s\n",i,errorMessage);
	
	if (errCode != noErr)
	{
		errCode = CheckDeviceCondition(i);
		
		if (errCode != noErr)
			printf("%s\n",errorMessage);
	}
	
	
	errCode = ReadDevicePartition(i);
	
	if (errCode != noErr)
		printf("SCSI device #%d error: %s\n",i,errorMessage);
	
	if (errCode != noErr)
	{
		errCode = CheckDeviceCondition(i);
		
		if (errCode != noErr)
			printf("%s\n",errorMessage);
	}
	
	
	
	errCode = ReadDevicePartition(0);
	
	if (errCode != noErr)
		printf("SCSI device #%d error: %s\n",i,errorMessage);
	
	if (errCode != noErr)
	{
		errCode = CheckDeviceCondition(i);
		
		if (errCode != noErr)
			printf("%s\n",errorMessage);
	}
	
	
EXITPOINT:

	printf("\n\nThe End.\n");
}