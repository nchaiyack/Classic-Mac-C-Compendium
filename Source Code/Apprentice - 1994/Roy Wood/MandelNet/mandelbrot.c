#include "mandel.h"



AssignMandelbrot()
{
int				i,errCode,numRows,responseCode,currentRow;
char			tempString[256];
int				theRow[maxSlaves];
long			finalTicks[maxSlaves];

	SetCursor(&waitCursor);
	
	if (slaveMode==TRUE || numSlaves<=0)
	{
		DisplayMessage("Generating Mandelbrot Image...");
		
		Mandelbrot(1,254);
		
		DisplayMessage("Completed Mandelbrot Image");
	}
		
	else
	{
		numRows = graphRect.bottom - graphRect.top - 2;
		numRows = 0.5 + (double) numRows/(double) pixelSize;
		
		currentRow = graphRect.top + 1;
		
		for (i=0;i<numSlaves;i++)
		{
			if (BreakKey())
				goto EXITPOINT;
			
			errCode = SendWorkOrder(i,currentRow);
			
			if (errCode != noErr)
				slaveStatus[i] = FALSE;
			else
			{
				slaveStatus[i] = TRUE;
				theRow[i] = currentRow;
				currentRow += pixelSize;
				finalTicks[i] = TickCount();
			}
		}
		
		
		while (numRows)
		{
			if (meToo && currentRow<graphRect.bottom-1)
			{
				if (Mandelbrot(currentRow,currentRow+pixelSize) != noErr)
					goto EXITPOINT;
				else
				{
					CopyBits(&(theWPtr->portBits),&graphBits,&graphRect,&graphRect,srcCopy,0L);
					currentRow += pixelSize;
					numRows--;
				}
			}
		
			for (i=0;numRows && i<numSlaves;i++)
			{
				if (BreakKey())
					goto EXITPOINT;
				
				if (slaveStatus[i]==TRUE && TickCount()>=finalTicks[i])
				{
					errCode = QueryNode(i,&responseCode);
					
					if (errCode == noErr)
					{
						if (responseCode==busyResponse)
						{
							sprintf(tempString,"Node #%d still busy",slaveNode[i]);
							DisplayMessage(tempString);
							
							finalTicks[i] = TickCount() + 60.0*timeDelay;
						}
						
						else if (responseCode==idleResponse)
						{
							if (theRow[i] > 0)
							{
								errCode = GetBitMap(i,theRow[i]);
								
								if (errCode == noErr)
								{
									theRow[i] = -1;
									numRows--;
								}
								
								finalTicks[i] = TickCount();
							}
							
							if (theRow[i]<0 && currentRow<graphRect.bottom-1)
							{
								errCode = SendWorkOrder(i,currentRow);
			
								if (errCode != noErr)
									slaveStatus[i] = FALSE;
								
								else
								{
									slaveStatus[i] = TRUE;
									theRow[i] = currentRow;
									currentRow += pixelSize;
									finalTicks[i] = TickCount();
								}
							}
						}
					}
				}
			}
		}
	}
	
EXITPOINT:

	InitCursor();
}





Mandelbrot(firstRow,lastRow)
int		firstRow,lastRow;
{
int				errCode,i,j,iterations;
double			cx,cy,x,y,tx,ty,r;
GrafPtr			oldPort;
EventRecord		theEvent;
char			theChar;
Rect			theRect,myClipRect;

	errCode = noErr;
	
	GetPort(&oldPort);
	SetPort(theWPtr);
	
	myClipRect = graphRect;
	InsetRect(&myClipRect,1,1);
	ClipRect(&myClipRect);
	
	PenNormal();
	
	
	
	for (i=firstRow;i<lastRow;i+=pixelSize)
	{
		for (j=1;j<255;j+=pixelSize)
		{
			theRect.left = j + graphRect.left;
			theRect.top = i + graphRect.top;
			theRect.right = theRect.left + pixelSize;
			theRect.bottom = theRect.top + pixelSize;
			
			PenPat(black);
			PenMode(patXor);
			PaintRect(&theRect);
			
			cx = XToD(graphRect.left + j + pixelSize/2);
			cy = YToD(graphRect.top + i + pixelSize/2);
			
			x = cx;
			y = cy;
			
			iterations = 0;
			
			do
			{
				tx = x*x-y*y + cx;
				ty = 2.0*x*y + cy;
				
				r = tx*tx + ty*ty;
				
				iterations++;
				
				x = tx;
				y = ty;
				
			} while (r<4.0 && iterations<maxIterations);
			
			if (r >= 4.0)
			{
				if (shading)
				{
					if (iterations < maxIterations/4)
						PenPat(white);
					else if (iterations < 2*maxIterations/4)
						PenPat(ltGray);
					else if (iterations < 3*maxIterations/4)
						PenPat(gray);
					else if (iterations < maxIterations)
						PenPat(dkGray);
					else
						PenPat(black);
				}
				else
					PenPat(white);
			}
			else
				PenPat(black);
				
			PenMode(patCopy);
			PaintRect(&theRect);
			
			
			
			if (BreakKey())
			{
				errCode = -1;
				goto EXITPOINT;
			}
			
			if (slaveMode == TRUE)
			{
				if (GetNextEvent(networkMask,&theEvent))
				{
					if ((**receiveRecord).atpProto.atpUserData == stopOrder)
					{
						DisplayMessage("Confirming stopOrder");
			
						(**receiveRecord).atpProto.atpRspUData = confirmOrder;
						
						(**receiveRecord).atpProto.atpSocket = mySocket;
						(**receiveRecord).atpProto.atpRspBuf = 0L;
						(**receiveRecord).atpProto.atpRspSize = 0;
				
						errCode = ATPResponse(receiveRecord,FALSE);
						
						if (errCode != noErr)
							ErrorMessage("ATPResponse",errCode);
						
						errCode = -1;
						
						goto EXITPOINT;
					}
					
					else if ((**receiveRecord).atpProto.atpUserData == queryStatus)
					{
						(**receiveRecord).atpProto.atpRspUData = busyResponse;
						
						(**receiveRecord).atpProto.atpSocket = mySocket;
						(**receiveRecord).atpProto.atpRspBuf = 0L;
						(**receiveRecord).atpProto.atpRspSize = 0;
				
						errCode = ATPResponse(receiveRecord,FALSE);
						
						if (errCode != noErr)
							ErrorMessage("ATPResponse",errCode);
						
						AwaitRequest(receiveRecord,&theMessage);
					}
				}
			}
		}
	}

EXITPOINT:

	ClipRect(&nullClipRect);
	SetPort(oldPort);
	
	return(errCode);
}



StopSlaves()
{
int				i,errCode;
char			tempString[256];
ABRecHandle		sendRecord;


	SetCursor(&waitCursor);
	
	if (numSlaves > 0)
	{
		sendRecord = (ABRecHandle) NewHandle(atpSize);
		
		
		for (i=0;i<numSlaves;i++)
		{
			sprintf(tempString,"Sending stopOrder to node #%d",slaveNode[i]);
			DisplayMessage(tempString);
			
			
			(**sendRecord).atpProto.atpAddress.aNet = slaveNet[i];
			(**sendRecord).atpProto.atpAddress.aNode = slaveNode[i];
			(**sendRecord).atpProto.atpAddress.aSocket = slaveSocket[i];
			
			(**sendRecord).atpProto.atpReqCount =  0;
			(**sendRecord).atpProto.atpDataPtr = 0L;
			
			(**sendRecord).atpProto.atpXO = FALSE;
			(**sendRecord).atpProto.atpTimeOut = rtInterval;
			(**sendRecord).atpProto.atpRetries = rtCount;
			
			(**sendRecord).atpProto.atpRspBuf = 0L;
			(**sendRecord).atpProto.atpRspSize =  0;
			
			(**sendRecord).atpProto.atpUserData = stopOrder;
			
			errCode = ATPRequest(sendRecord,FALSE);
			
			if (errCode || (**sendRecord).atpProto.atpRspUData!=confirmOrder)
			{
				sprintf(tempString,"Node #%d doesn't respond to stop order",slaveNode[i]);
				DisplayMessage(tempString);
				
				slaveStatus[i] = FALSE;
			}
			
			else
			{
				sprintf(tempString,"stopOrder confirmed by node #%d",slaveNode[i]);
				DisplayMessage(tempString);
				
				slaveStatus[i] = TRUE;
			}
		}
	
		DisposHandle(sendRecord);
	}
	
	InitCursor();
}



CheckSlaves()
{
int				i,errCode,busy,idle;
char			tempString[256];
ABRecHandle		sendRecord;

	SetCursor(&waitCursor);
	
	LookupSlaves();
	DrawInfo();
	
	if (numSlaves > 0)
	{
		sendRecord = (ABRecHandle) NewHandle(atpSize);
		busy = 0;
		idle = 0;
		
		for (i=0;i<numSlaves;i++)
		{
			sprintf(tempString,"Checking node #%d",slaveNode[i]);
			DisplayMessage(tempString);
			
			
			(**sendRecord).atpProto.atpAddress.aNet = slaveNet[i];
			(**sendRecord).atpProto.atpAddress.aNode = slaveNode[i];
			(**sendRecord).atpProto.atpAddress.aSocket = slaveSocket[i];
			
			(**sendRecord).atpProto.atpReqCount =  0;
			(**sendRecord).atpProto.atpDataPtr = 0L;
			
			(**sendRecord).atpProto.atpXO = FALSE;
			(**sendRecord).atpProto.atpTimeOut = rtInterval;
			(**sendRecord).atpProto.atpRetries = rtCount;
			
			(**sendRecord).atpProto.atpRspBuf = 0L;
			(**sendRecord).atpProto.atpRspSize =  0;
			
			(**sendRecord).atpProto.atpUserData = queryStatus;
			
			errCode = ATPRequest(sendRecord,FALSE);
			
			if (errCode==noErr)
			{
				if ((**sendRecord).atpProto.atpRspUData==idleResponse)
					idle++;
				else if ((**sendRecord).atpProto.atpRspUData==busyResponse)
					busy++;
				else
				{
					sprintf(tempString,"node #%d -- unknown response",slaveNode[i]);
					DisplayMessage(tempString);
				}
			}
			else
				ErrorMessage("ATPRequest",errCode);
		}
	
		DisposHandle(sendRecord);
		
		sprintf(tempString," Busy nodes: %d \r Idle nodes: %d \r Unaccounted nodes: %d",busy,idle,numSlaves-busy-idle);
		CtoPstr(tempString);
		ErrorAlert(tempString);
	}
	
	InitCursor();
}



SendWorkOrder(i,theRow)
int	i,theRow;
{
char			tempString[256];
slaveMessage	theMessage;
ABRecHandle		sendRecord;

	sendRecord = (ABRecHandle) NewHandle(atpSize);
	
	theMessage.lims[xmin] = lims[xmin];
	theMessage.lims[xmax] = lims[xmax];
	theMessage.lims[ymin] = lims[ymin];
	theMessage.lims[ymax] = lims[ymax];
	
	theMessage.firstRow = theRow;
	theMessage.lastRow = theRow+pixelSize;
	
	theMessage.maxIterations = maxIterations;
	theMessage.pixelSize = pixelSize;
	theMessage.shading = shading;
	
	sprintf(tempString,"Sending workOrder to node #%d",slaveNode[i]);
	DisplayMessage(tempString);
	
	
	(**sendRecord).atpProto.atpAddress.aNet = slaveNet[i];
	(**sendRecord).atpProto.atpAddress.aNode = slaveNode[i];
	(**sendRecord).atpProto.atpAddress.aSocket = slaveSocket[i];
	
	(**sendRecord).atpProto.atpReqCount = sizeof(slaveMessage);
	(**sendRecord).atpProto.atpDataPtr = (Ptr) &theMessage;
	
	(**sendRecord).atpProto.atpXO = FALSE;
	(**sendRecord).atpProto.atpTimeOut = rtInterval;
	(**sendRecord).atpProto.atpRetries = rtCount;
	
	(**sendRecord).atpProto.atpRspBuf = 0L;
	(**sendRecord).atpProto.atpRspSize = 0;
	
	(**sendRecord).atpProto.atpUserData = workOrder;
	
	errCode = ATPRequest(sendRecord,FALSE);
	
	if ((**sendRecord).atpProto.atpRspUData!=confirmOrder)
		errCode = -1;
	
	if (errCode)
	{
		sprintf(tempString,"Node #%d doesn't respond to workOrder",slaveNode[i]);
		DisplayMessage(tempString);
	}
	
	else
	{
		sprintf(tempString,"workOrder confirmed by node #%d",slaveNode[i]);
		DisplayMessage(tempString);
	}
	
	DisposHandle(sendRecord);
	
	return(errCode);
}



QueryNode(i,theResponse)
int		i,*theResponse;
{
char			tempString[256];
ABRecHandle		sendRecord;

	sendRecord = (ABRecHandle) NewHandle(atpSize);
	
	sprintf(tempString,"Querying status of node #%d",slaveNode[i]);
	DisplayMessage(tempString);
	
	(**sendRecord).atpProto.atpAddress.aNet = slaveNet[i];
	(**sendRecord).atpProto.atpAddress.aNode = slaveNode[i];
	(**sendRecord).atpProto.atpAddress.aSocket = slaveSocket[i];
	
	(**sendRecord).atpProto.atpReqCount = 0;
	(**sendRecord).atpProto.atpDataPtr = 0L;
	
	(**sendRecord).atpProto.atpXO = FALSE;
	(**sendRecord).atpProto.atpTimeOut = rtInterval;
	(**sendRecord).atpProto.atpRetries = rtCount;
	
	(**sendRecord).atpProto.atpRspBuf = 0L;
	(**sendRecord).atpProto.atpRspSize = 0;
	
	(**sendRecord).atpProto.atpUserData = queryStatus;
	
	errCode = ATPRequest(sendRecord,FALSE);
	
	if (errCode == noErr)
		*theResponse = (**sendRecord).atpProto.atpRspUData;
	else
		ErrorMessage("ATPRequest",errCode);
	
	DisposHandle(sendRecord);
	
	return(errCode);
}




GetBitMap(i,theRow)
int		i,theRow;
{
char			tempString[256];
slaveMessage	theMessage;
ABRecHandle		sendRecord;
int				bitmapSize;
BitMap			mandelBits;

	sendRecord = (ABRecHandle) NewHandle(atpSize);
	
	theMessage.firstRow = theRow;
	theMessage.lastRow = theRow+pixelSize;
	
	mandelBits.bounds.left = graphRect.left+1;
	mandelBits.bounds.right = graphRect.right-1;
	mandelBits.bounds.top = theRow;
	mandelBits.bounds.bottom = theRow + pixelSize;
	
	mandelBits.rowBytes = (mandelBits.bounds.right - mandelBits.bounds.left + 7)/8;
	if (mandelBits.rowBytes & 1) mandelBits.rowBytes++;
	
	bitmapSize = mandelBits.rowBytes * (mandelBits.bounds.bottom - mandelBits.bounds.top);
	
	mandelBits.baseAddr = NewPtr(bitmapSize);
	
	
	sprintf(tempString,"Sending bitmapRequest to node #%d",slaveNode[i]);
	DisplayMessage(tempString);
	
	(**sendRecord).atpProto.atpAddress.aNet = slaveNet[i];
	(**sendRecord).atpProto.atpAddress.aNode = slaveNode[i];
	(**sendRecord).atpProto.atpAddress.aSocket = slaveSocket[i];
	
	(**sendRecord).atpProto.atpReqCount = sizeof(slaveMessage);
	(**sendRecord).atpProto.atpDataPtr = (Ptr) &theMessage;
	
	(**sendRecord).atpProto.atpXO = FALSE;
	(**sendRecord).atpProto.atpTimeOut = rtInterval;
	(**sendRecord).atpProto.atpRetries = rtCount;
	
	(**sendRecord).atpProto.atpRspBuf = mandelBits.baseAddr;
	(**sendRecord).atpProto.atpRspSize = bitmapSize;
	
	(**sendRecord).atpProto.atpUserData = bitmapRequest;
	
	errCode = ATPRequest(sendRecord,FALSE);
	
	if (errCode!=noErr)
	{
		ErrorMessage("ATPRequest",errCode);
	}
	
	else 
	{
		if ((**sendRecord).atpProto.atpRspUData==bitmapResponse)
		{
			sprintf(tempString,"bitmapRequest confirmed by node #%d",slaveNode[i]);
			DisplayMessage(tempString);
			
			CopyBits(&mandelBits,&(theWPtr->portBits),&(mandelBits.bounds),&(mandelBits.bounds),srcCopy,0L);
			
			CopyBits(&mandelBits,&graphBits,&(mandelBits.bounds),&(mandelBits.bounds),srcCopy,0L);
		}
		
		else
		{
			sprintf(tempString,"Bad response from node #%d",slaveNode[i]);
			DisplayMessage(tempString);
			
			errCode = -1;
		}
	}
	
	DisposHandle(sendRecord);
	DisposPtr(mandelBits.baseAddr);
	
	return(errCode);
}
	