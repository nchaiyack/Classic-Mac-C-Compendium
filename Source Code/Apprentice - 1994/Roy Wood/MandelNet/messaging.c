#include "mandel.h"



SetupAppleTalk()
{
int		errCode;

	SetCursor(&waitCursor);
	
	errCode = InitAppleTalk();
	
	if (errCode == 0)
	{
		GetNodeAddress(&myNode,&myNet);
		sprintf(myName," MandelNet Node #%d",myNode);
		
		strcpy(myEntity.objStr,myName);
		CtoPstr((char *) myEntity.objStr);
		
		if (slaveMode)
			strcpy(myEntity.typeStr,slaveSig);
		else
			strcpy(myEntity.typeStr,masterSig);
		
		CtoPstr((char *) myEntity.typeStr);
		strcpy(myEntity.zoneStr,"*");
		CtoPstr((char *) myEntity.zoneStr);
		
		errCode = RegisterName();
	}
	
	InitCursor();
	
	return(errCode);
}



InitAppleTalk()
{
AddrBlock		theAddrBlock;
	
	DisplayMessage("Checking AppleTalk");
	
	SetCursor(&waitCursor);
	
	errCode = ATPLoad();
	
	if (errCode != noErr)
		ErrorMessage("ATPLoad",errCode);
	
	else
	{
		theAddrBlock.aNet = 0;
		theAddrBlock.aNode = 0;
		theAddrBlock.aSocket = 0;
		
		mySocket = 0;
		
		DisplayMessage("Opening AppleTalk socket");
		
		errCode = ATPOpenSocket(theAddrBlock,&mySocket);
		
		if (errCode != noErr)
			ErrorMessage("ATPOpenSocket",errCode);
		else
			DisplayMessage("AppleTalk socket opened successfully");
	}
	
	InitCursor();
	
	return(errCode);
}



ShutDownAppleTalk()
{
int		errCode = noErr;

	SetCursor(&waitCursor);
	
	if (nbpRegistered)	/* Are we registered? */
	{
		UnRegisterName();	/*	Go remove our name from the NBP table */
		
		errCode = ATPCloseSocket(mySocket);		/* Close the socket */
		
		if (errCode != noErr)
			ErrorMessage("ATPCloseSocket",errCode);	/* Oops! Notify the user of the error */
	}
	
	InitCursor();
	
	return(errCode);
}



RegisterName()
{
ABRecHandle		nbpRecord;
	
	SetCursor(&waitCursor);
	
	if (nbpRegistered==FALSE)
	{
		nbpRecord = (ABRecHandle) NewHandle(nbpSize);
		nbpName = NewPtr(nbpNameBufSize);
		
		(**nbpRecord).nbpProto.nbpEntityPtr = &myEntity;
		(**nbpRecord).nbpProto.nbpBufPtr = nbpName;
		(**nbpRecord).nbpProto.nbpBufSize = nbpNameBufSize;
		(**nbpRecord).nbpProto.nbpAddress.aSocket = mySocket;
		(**nbpRecord).nbpProto.nbpRetransmitInfo.retransInterval = rtInterval;
		(**nbpRecord).nbpProto.nbpRetransmitInfo.retransCount = rtCount;
		
		DisplayMessage("Registering on AppleTalk");
		
		errCode = NBPRegister(nbpRecord,FALSE);
		
		if (errCode != noErr)
		{
			nbpRegistered = FALSE;
			DisposPtr(nbpName);
			
			ErrorMessage("NBPRegister",errCode);
		}
		else
		{
			nbpRegistered = TRUE;
			
			DisplayMessage("Registered on AppleTalk");
		}
		
		DisposHandle(nbpRecord);
	}
	
	else
	{
		DisplayMessage("Redundant attempt to register on AppleTalk");
		errCode = noErr;
	}
	
	InitCursor();
	
	return(errCode);
}




UnRegisterName()
{
int	errCode;
	
	SetCursor(&waitCursor);
	
	if (nbpRegistered == TRUE)
	{
		DisplayMessage("Removing node from AppleTalk");
		
		errCode = NBPRemove(&myEntity);
		
		if (errCode != noErr)
			ErrorMessage("NBPRemove",errCode);
		else
			DisplayMessage("Node removed from AppleTalk");
		
		DisposPtr(nbpName);
		nbpName = 0L;
		
		nbpRegistered = FALSE;
	}
	else
	{
		DisplayMessage("Redundant attempt to unregister from AppleTalk");
		errCode = noErr;
	}
	
	InitCursor();
	
	return(errCode);
}



LookupSlaves()
{
int				i;
EntityName		theEntity;
AddrBlock		theAddrBlock;
ABRecHandle		nbpRecord;
char			tempString[256];

	SetCursor(&waitCursor);
	
	nbpRecord = (ABRecHandle) NewHandle(nbpSize);
	
	numSlaves = 0;
	
	strcpy(theEntity.objStr,"=");
	CtoPstr((char *) theEntity.objStr);
	strcpy(theEntity.typeStr,slaveSig);
	CtoPstr((char *) theEntity.typeStr);
	strcpy(theEntity.zoneStr,"*");
	CtoPstr((char *) theEntity.zoneStr);
	
 	(**nbpRecord).nbpProto.nbpEntityPtr = &theEntity;
	(**nbpRecord).nbpProto.nbpBufPtr = (Ptr) myBuffer;
	(**nbpRecord).nbpProto.nbpBufSize = myBufSize;
	(**nbpRecord).nbpProto.nbpDataField = maxSlaves;
	(**nbpRecord).nbpProto.nbpAddress.aSocket = mySocket;
	(**nbpRecord).nbpProto.nbpRetransmitInfo.retransInterval = rtInterval;
	(**nbpRecord).nbpProto.nbpRetransmitInfo.retransCount = rtCount;
	
	DisplayMessage("Looking for nodes on AppleTalk");
	
	errCode = NBPLookup(nbpRecord,FALSE);
	
	if (errCode == noErr) 
	{
		numSlaves = (**nbpRecord).nbpProto.nbpDataField;
		
		if (numSlaves)
		{
			for (i=1;i<=numSlaves && i<=maxSlaves && errCode==noErr;i++)
			{
				sprintf(tempString,"Extracting info for entry #%d",i);
				DisplayMessage(tempString);
				
				errCode = NBPExtract(myBuffer,numSlaves,i,&theEntity,&theAddrBlock);
				
				if (errCode != noErr)
					ErrorMessage("NBPExtract",errCode);
				
				slaveNet[i-1] = theAddrBlock.aNet;
				slaveNode[i-1] = theAddrBlock.aNode;
				slaveSocket[i-1] = theAddrBlock.aSocket;
			}
			
			if (errCode==noErr && numSlaves>maxSlaves)
			{
				DisplayMessage("Too many nodes found-- restricting number used");
				
				numSlaves = maxSlaves;
			}
		}
	}
	
	else
		ErrorMessage("NBPLookup",errCode);
	
	DisposHandle(nbpRecord);
	
	InitCursor();
	
	return(errCode);
}





AwaitRequest(receiveRecord,messagePtr)
ABRecHandle	receiveRecord;slaveMessage	*messagePtr;
{
	(**receiveRecord).atpProto.atpSocket = mySocket;	
	(**receiveRecord).atpProto.atpReqCount = sizeof(slaveMessage);
	(**receiveRecord).atpProto.atpDataPtr = (Ptr) messagePtr;
	
	errCode = ATPGetRequest(receiveRecord,TRUE);
	
	if (errCode != noErr)
		ErrorMessage("ATPGetRequest",errCode);
	
	return(errCode);
}




KillRequest()
{
int		errCode;

	SetCursor(&waitCursor);
	
	errCode = UnRegisterName();
	
	if (errCode == noErr)
	{
		errCode = ATPCloseSocket(mySocket);
		
		if (errCode != noErr)
			ErrorMessage("ATPCloseSocket",errCode);
	}
	
	InitCursor();
	
	return(errCode);
}




RespondToRequest(receiveRecord)
ABRecHandle		receiveRecord;
{
slaveMessage	*messagePtr;
int				errCode,firstRow,lastRow;
BitMap			mandelBits;
int				bitmapSize;

	messagePtr = (slaveMessage *) (**receiveRecord).atpProto.atpDataPtr;
	
	switch ((**receiveRecord).atpProto.atpUserData)
	{
		case workOrder:
			lims[xmin] = messagePtr->lims[xmin];
			lims[xmax] = messagePtr->lims[xmax];
			lims[ymin] = messagePtr->lims[ymin];
			lims[ymax] = messagePtr->lims[ymax];
			
			maxIterations = messagePtr->maxIterations;
			pixelSize = messagePtr->pixelSize;
			shading = messagePtr->shading;
			DrawInfo();
			
			firstRow = messagePtr->firstRow;
			lastRow = messagePtr->lastRow;
			
			DisplayMessage("Confirming workOrder");
			
			(**receiveRecord).atpProto.atpRspUData = confirmOrder;
			
			(**receiveRecord).atpProto.atpSocket = mySocket;
			(**receiveRecord).atpProto.atpRspBuf = 0L;
			(**receiveRecord).atpProto.atpRspSize = 0;
	
			errCode = ATPResponse(receiveRecord,FALSE);
			
			if (errCode != noErr)
				ErrorMessage("ATPResponse",errCode);
			
			AwaitRequest(receiveRecord,&theMessage);
			
			DisplayMessage("Generating assigned Mandelbrot region");
			SetCursor(&waitCursor);
			Mandelbrot(firstRow,lastRow);
			
			CopyBits(&(theWPtr->portBits),&graphBits,&graphRect,&graphRect,srcCopy,0L);
			
			DisplayMessage("Completed assigned Mandelbrot region");
			
			InitCursor();
			
			break;
			
		case bitmapRequest:
			DisplayMessage("Sending bitmap");
			
			mandelBits.bounds.left = graphRect.left+1;
			mandelBits.bounds.right = graphRect.right-1;
			mandelBits.bounds.top = messagePtr->firstRow;
			mandelBits.bounds.bottom = messagePtr->lastRow;
			
			mandelBits.rowBytes = (mandelBits.bounds.right - mandelBits.bounds.left + 1)/8;
			if (mandelBits.rowBytes & 1) mandelBits.rowBytes++;
			
			bitmapSize = mandelBits.rowBytes * (mandelBits.bounds.bottom - mandelBits.bounds.top + 1);
			
			mandelBits.baseAddr = NewPtr(bitmapSize);
			
			CopyBits(&(theWPtr->portBits),&mandelBits,&(mandelBits.bounds),&(mandelBits.bounds),srcCopy,0L);
			
			(**receiveRecord).atpProto.atpRspUData = bitmapResponse;
			(**receiveRecord).atpProto.atpSocket = mySocket;
			(**receiveRecord).atpProto.atpRspBuf = mandelBits.baseAddr;
			(**receiveRecord).atpProto.atpRspSize = bitmapSize;
	
			errCode = ATPResponse(receiveRecord,FALSE);
			
			if (errCode != noErr)
				ErrorMessage("ATPResponse",errCode);
				
			DisposPtr(mandelBits.baseAddr);
			
			break;
		
		case stopOrder:
			DisplayMessage("Confirming stopOrder");
			
			(**receiveRecord).atpProto.atpRspUData = confirmOrder;
			
			(**receiveRecord).atpProto.atpSocket = mySocket;
			(**receiveRecord).atpProto.atpRspBuf = 0L;
			(**receiveRecord).atpProto.atpRspSize = 0;
	
			errCode = ATPResponse(receiveRecord,FALSE);
			
			if (errCode != noErr)
				ErrorMessage("ATPResponse",errCode);
			
			break;
			
		case queryStatus:
			DisplayMessage("Reporting idle status");
			
			(**receiveRecord).atpProto.atpRspUData = idleResponse;
			
			(**receiveRecord).atpProto.atpSocket = mySocket;
			(**receiveRecord).atpProto.atpRspBuf = 0L;
			(**receiveRecord).atpProto.atpRspSize = 0;
	
			errCode = ATPResponse(receiveRecord,FALSE);
			
			if (errCode != noErr)
				ErrorMessage("ATPResponse",errCode);
			
			break;
			
		default:
			errCode = noErr;
	}
	
	return(errCode);
}