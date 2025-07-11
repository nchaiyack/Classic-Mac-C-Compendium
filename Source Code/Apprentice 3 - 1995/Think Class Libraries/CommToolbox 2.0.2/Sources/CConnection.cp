/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�

	CConnection.cp
	
	CommToolbox connection class.
	
	SUPERCLASS = CBureaucrat.
	
	Original copyright � 1992-93 Romain Vignes. All rights reserved.
	Modifications copyright � 1994-95 Ithran Einhorn. All rights reserved.
	
같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같� */

#include <CommResources.h>					/* Apple includes */
#include <Connections.h>

#include <CBartender.h>						/* TCL includes */
#include <CPtrArray.h>
#include <CError.h>
#include <Constants.h>
#include <TBUtilities.h>
#include <TCLUtilities.h>

#include "CConnection.h"					/* Other includes */
#include "CFileTransfer.h"
#include "CTermPane.h"

/* Constants & Macros */

#define CONN_STR_RES_ID		2100	/* Connection message resource ID */

#define NO_TOOL_STR_INDEX	1		/* No connection tool */
#define BAD_TOOL_STR_INDEX	2		/* Bad connection tool */
#define NO_REC_STR_INDEX	3		/* Connection record allocation error */
#define CHOOSE_STR_INDEX	4		/* Tool setup error */
#define OPEN_ERR_STR_INDEX	5		/* Error on opening */
#define CLOSE_ERR_STR_INDEX 6		/* Error on closing */
#define WAIT_ERR_STR_INDEX	7		/* Waiting impossible */

#define H_CHOOSE_POS		10		/* Setup dialog position */
#define V_CHOOSE_POS		40

#define BREAK_DELAY			5		/* Approximately 80 ms */

#define FIRST_CONN_CMD		cmdConnChoose	/* First connection command */
#define LAST_CONN_CMD		cmdConnListen	/* Last connection command */

#define RESET_ALRT_ID		2100	/* Reset alert resource ID */

/* Application globals */

extern CBartender	*gBartender;
extern CError		*gError;

TCL_DEFINE_CLASS_M1(CConnection,  CBureaucrat);

/* Class variables initialization */

CPtrArray<CConnection>	*CConnection::cConnList = NULL;

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cIsConnectionCmd
 *
 * Command related to the connection object
 *
 * theCmd:	command to analyse
 *
 * Return TRUE if the command is a connection command
 *
 */
 
Boolean CConnection::cIsConnectionCmd(long theCmd)
{
	return ((theCmd >= FIRST_CONN_CMD) && (theCmd <= LAST_CONN_CMD));
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cCheckToolName
 *
 * Checking existence of a tool by its name
 *
 * toolName:	name of the tool (Pascal string)
 *
 * Return cmGenericError if the tool is not present
 *
 */
 
OSErr CConnection::cCheckToolName(Str31 toolName)
{
	return(CMGetProcID(toolName));
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cTestToolEvent
 *
 * Test if the event is related to a connection tool
 *
 * macEvent:	pointer on the event record
 * theWindow:	pointer on the window record
 *
 * Return TRUE if the event is a terminal event
 */

typedef struct	{
	EventRecord	*theEvent;
	WindowPtr	theWindow;
	Boolean		isToolEvent;
} TestParamRec;


 		/* Test routine */

		static void ConnEvtTest(CConnection *theConnection,long params)
		{
			TestParamRec *testRecPtr = (TestParamRec *)params;
			
			if (testRecPtr->isToolEvent == FALSE)
				testRecPtr->isToolEvent = theConnection->DoEvent(testRecPtr->theEvent,
																 testRecPtr->theWindow);
		}


Boolean CConnection::cTestToolEvent(EventRecord *macEvent,WindowPtr theWindow)
{
	TestParamRec params;
	
	params.theEvent = macEvent;
	params.theWindow = theWindow;
	params.isToolEvent = FALSE;
	
	if (cConnList == NULL)
		return FALSE;
	else
	{
		cConnList->DoForEach1(ConnEvtTest,(long)&params);
		
		return params.isToolEvent;
	}
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cConnIdle
 *
 * Idle time for each connection object
 *
 *
 */
 
 		/* Idle routine for each connection object */

		static void	Conn_Idle(CConnection *theConn)
		{
			theConn->DoIdle();
		}


void CConnection::cConnIdle(void)
{
	if (cConnList != NULL)		/* List exists ? */
		cConnList->DoForEach(Conn_Idle);
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cInitManager
 *
 * Connection Manager Initialization
 *
 */
 
void CConnection::cInitManager(void)
{
	InitCM();
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cGetCMVersion
 *
 * return the version of the Connection Manager
 *
 */
 
short CConnection::cGetCMVersion(void)
{
	return CMGetCMVersion();
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * CConnection
 *
 * Initialisation of the connection object
 *
 * aSupervisor:			object supervisor in the command chain
 * toolName:			name of the connection tool to be used ("" -> d럈ault)
 * flags:(CM)			how to use the connection tool
 * desiredSizes:(CM)	desired sizes of sending and receiving buffers
 * refcon:(CM)			available for the application
 * userData:(CM)		available for the application
 *
 * Parameters followed by CM are exact required parameters for creating a
 * connection record.
 *
 */
 
CConnection::CConnection(CBureaucrat *aSupervisor,Str31 toolName,
						 CMRecFlags flags,CMBufferSizes desiredSizes,
						 long refCon, long userData)
						 
			: CBureaucrat(aSupervisor)	//	Initialize superclass
{
	ConnHandle	theConn;
	OSErr		theErr;
	Str31		tName;
	short		toolProcID;
	Boolean		savedAlloc;
	
	if (toolName[0] == 0)					/* Default tool ? */
	{
		theErr = CRMGetIndToolName(classCM,1,tName);
				
		if ((tName[0] == 0)	|| (theErr != cmNoErr))	 	/* Error checking */
			Failure(cmNoTools,SpecifyMsg(CONN_STR_RES_ID,NO_TOOL_STR_INDEX));
			
		toolProcID = cCheckToolName(tName);			/* Default tool ID */
	}
	else
	{
		toolProcID = cCheckToolName(toolName);		/* Specified tool ID */	
	}
		
	if (toolProcID == cmGenericError)				/* No corresponding tool */
	{
		Failure(cmNoTools,SpecifyMsg(CONN_STR_RES_ID,BAD_TOOL_STR_INDEX));
	}
	
	savedAlloc = SetAllocation(kAllocCanFail);
		
	theConn = CMNew(toolProcID,flags,desiredSizes,(long)this,userData);
	
	SetAllocation(savedAlloc);
	
	FailNIL(theConn);			/* Connection created ? */

	MoveHHi((Handle)theConn);	/* Heap fragmentation� */
		
	itsConn = theConn;
		
	if (cConnList == NULL)	/* first connection object ? */
	{			
		cConnList = new CPtrArray<CConnection>;
	}
	
	cConnList->Add(this);				/* Connection addition */
		
	this->connOpen = FALSE;	
	this->active = FALSE;
	
	#ifdef __USETHREADS__
	if (CThread::cIsPresent())
	{
		itsIdleThread = new CThread(kCooperativeThread, (ThreadEntryProcPtr)cOneConnIdle, this);
	}
	#endif
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Destructor
 *
 * Dispose of a connection object
 *
 */
 
CConnection::~CConnection()
{
	ASSERT(cConnList != NULL);
	
	#ifdef __USETHREADS__
	if (CThread::cIsPresent())
	{
		delete itsIdleThread;
	}
	#endif
	
	cConnList->Remove(this);			/* Dispose of the Connection */
	
	if (cConnList->IsEmpty())
		ForgetObject(cConnList);		/* Dispose of the cluster */

	CMDispose(itsConn);					/* Dispose of the connection record */
	itsConn = NULL;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * UpdateMenus
 *
 * Connection related menus updating
 *
 */
 
void CConnection::UpdateMenus(void)
{
	gBartender->EnableCmd(cmdConnChoose);		/* Setup command */
	
	gBartender->EnableCmd(cmdConnReset);		/* Connection reset */
	
	if (this->IsOpen())							/* Connection open ? */
	{
		gBartender->EnableCmd(cmdConnClose);
		gBartender->EnableCmd(cmdConnBreak);
	}
	else
	{
		gBartender->EnableCmd(cmdConnOpen);
		gBartender->EnableCmd(cmdConnListen);
	}
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoCommand
 *
 * Handle connection related commands
 *
 * theCommand:	command to be executed
 *
 */
 
void CConnection::DoCommand(long theCommand)
{
	switch (theCommand)
	{		
		case cmdConnChoose:		/* Connection tool setup */
			this->ConnectionChoose();
			break;
			
		case cmdConnOpen:		/* Connection opening */
			this->OpenConnection(FALSE,NULL,0L);
			break;
			
		case cmdConnListen:		/* Connection listening */
			this->ListenConnection(FALSE,NULL,0L);
			break;
			
		case cmdConnClose:		/* Connection waiting */
			this->CloseConnection(FALSE,NULL,0L,TRUE);
			break;
			
		case cmdConnReset:		/* Connection reset */
			this->Reset();
			break;
			
		case cmdConnBreak:		/* BREAK signal send */
			this->SendBreak();
			break;
			
		default:				/* Unknown command */
			break;
	}
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoEvent
 *
 * Connection tool related events
 *
 * theEvent:	Pointeur on the event
 * theWindow:	Window associated with the event
 *
 * Return TRUE if it is a tool event
 *
 */
 
Boolean CConnection::DoEvent(EventRecord *theEvent,WindowPtr theWindow)
{
	Boolean		isToolEvent;
	ConnHandle	theConn;
	
	isToolEvent = FALSE;
	
	theConn = (ConnHandle) GetWRefCon(theWindow);
	
	if (theConn == itsConn)	{			/* Tool window ? */
		CMEvent(itsConn,theEvent);
		isToolEvent = TRUE;
	}
	
	return isToolEvent;
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * ConnectionChoose
 *
 * Connection tool setup
 *
 */
 
void CConnection::ConnectionChoose(void)
{
	short		retCode;
	Point		where;
	ConnHandle	hConn;
	
	hConn = this->itsConn;
	
	SetPt(&where,H_CHOOSE_POS,V_CHOOSE_POS);	/* Dialog position */
												
	retCode = CMChoose(&hConn,where,NULL);
	
	this->itsConn = hConn;

	switch (retCode)
	{		
		case chooseCancel:		/* Forget changes */
			break;
			
		case chooseOKMajor:		/* Changed tool */
		
 		// if we belong to a terminal pane, get its file transfer object
 		if (member(itsSupervisor, CTermPane))
 		{
 		 CFileTransfer	*itsFileTransfer = nil;
 		 
		 itsFileTransfer = ((CTermPane *)itsSupervisor)->GetFileTransfer ();
		 
		 if (itsFileTransfer)
 		 	 itsFileTransfer->AddSearch();
 		}
 		// fall through to chooseOKMinor processing
 	
		case chooseOKMinor:		/* Same tool, changed config */
		
			active = FALSE;
			Activate();
		
			itsSupervisor->Notify(NULL); 	/* Document updated */
			
			break;
			
		default:				/* Unknown code (error) */
			SysBeep(3);
			gError->PostAlert(CONN_STR_RES_ID,CHOOSE_STR_INDEX);
			break;	
	}
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * SetConfig
 *
 * Connection configuration change
 *
 * theConfig:	new configuration (C string)
 *
 * Return:		negative value: error (-1 -> unknown error)
 *				positive value: stop index of the parser
 *				cmNoErr if everything is OK
 *
 */
 
short CConnection::SetConfig(char *theConfig)
{
	short retCode;
	
	retCode = CMSetConfig(itsConn,theConfig);
	
	return retCode;
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetToolName
 *
 * Return the name of the current tool
 *
 * toolName:	tool name (Pascal string)
 *
 */
 
void CConnection::GetToolName(Str31 toolName)
{
	SignedByte	savedState;
	
	savedState = (SignedByte)HGetState((Handle)itsConn);
	HLock((Handle)itsConn);

	CMGetToolName((*itsConn)->procID,toolName);
	
	HSetState((Handle)itsConn,(char)savedState);
}	


/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * AddSearch
 *
 * Add auto download search pattern
 *
 * 
 *
 */
 
long CConnection::AddSearch(Str255 theString, CMSearchFlags flags, CommSearchPtr callBack)
{
 if (callBack == nil)
	 callBack = (CommSearchPtr)DefAutoRecCallback;
	 
 return CMAddSearch(itsConn, theString, flags, callBack);
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * RemoveSearch
 *
 * Remove auto download search pattern
 *
 */

void CConnection::RemoveSearch(long refNum)
{
 CMRemoveSearch(itsConn, refNum);
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * ClearSearch
 *
 * Clear all auto download search patterns
 *
 *
 */

void CConnection::ClearSearch(void)
{
 CMClearSearch(itsConn); 
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetConfig
 *
 * Return a C string describing the current config of the connection
 *
 * Return a pointer on the string
 *
 */
 
Ptr CConnection::GetConfig(void)
{
	return(CMGetConfig(itsConn));
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * getStatus
 *
 * Return the connection status
 *
 * sizes:	Buffers sizes (sortie)
 * flags:	Connection status (sortie)
 *
 * Return an error code
 *
 */
 
OSErr CConnection::getStatus(CMBufferSizes *sizes,CMStatFlags *flags)
{
	return(CMStatus(itsConn,*sizes,flags));
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * OpenConnection
 *
 * Connection opening
 *
 * async:		Opening mode (synchrone or asynchrone)
 * completor:	asynchrone callback
 * timeOut:		time period within which the opening must be completed
 *
 */
 
void CConnection::OpenConnection(Boolean async,CommProcPtr completor,long timeOut)
{
	OSErr	theErr;
	
	theErr = CMOpen(itsConn,async,completor,timeOut);
	
	if (theErr == cmNoErr)	{
		this->connOpen = TRUE;
		BroadcastChange(connOpenInd,NULL);
	}
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * ListenConnection
 *
 * Connection listening
 *
 * async:		Listening mode (synchrone or asynchrone)
 * completor:	asynchrone callback
 * timeOut:		time period within which the listening must be completed
 *
 */
 
void CConnection::ListenConnection(Boolean async,CommProcPtr completor,long timeOut)
{
	OSErr	theErr;
	
	theErr = CMListen(itsConn,async,completor,timeOut);
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * CloseConnection
 *
 * Connection closing
 *
 * async:		Closing mode (synchrone or asynchrone)
 * completor:	asynchrone callback
 * timeOut:		time period within which the closing must be completed
 * now:			immediately closing
 *
 */
 
void CConnection::CloseConnection(Boolean async,CommProcPtr completor,long timeOut,
									Boolean now)
{
	OSErr theErr;

	if (*itsConn != nil)
	{											/* have a good connection? */
		if (CMValidate(itsConn) == false)
		{
			theErr = CMAbort(itsConn);								/* just in case, abort open call */
			theErr = CMIOKill(itsConn,cmDataIn);					/* ...and, pending read */
			theErr = CMIOKill(itsConn,cmDataOut);					/* ...and, pending write */
			theErr = CMClose(itsConn,async,completor,timeOut,now);	/* ...close, it */
		}
		else
			theErr = cmUnknownError;
	}
	else
		theErr = nilHandleErr;
	
	if (theErr == cmNoErr)
	{
		this->connOpen = FALSE;
		BroadcastChange(connCloseInd,NULL);
	};
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * IsOpen
 *
 * Connection opening test
 *
 * Return TRUE if the connection is open
 *
 */
 
Boolean CConnection::IsOpen(void)
{
	CMBufferSizes	sizes;
	CMStatFlags		flags;
	OSErr			theErr;
	
	theErr = this->getStatus(&sizes,&flags);
	
	if (theErr == cmNoErr)
		if (flags & cmStatusOpen)		/* Connection open ? */
			return TRUE;
		else
			return FALSE;
	else
		return FALSE;
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DataAvail
 *
 * Data available
 *
 * Return the number of available characters
 *
 */
 
long CConnection::DataAvail(void)
{
	CMBufferSizes	sizes;
	CMStatFlags		flags;
	OSErr			theErr;
	
	theErr = this->getStatus(&sizes,&flags);
	
	if (theErr == cmNoErr)
		if (flags & cmStatusDataAvail)		/* Data available ? */
			return sizes[cmDataIn];			/* Return receiving buffer size */
		else
			return 0;
	else
		return 0;
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DataRead
 *
 * Reading of available data
 *
 * inBuffer:	Receiving buffer
 * buffSize:	Number of chars to read
 * async:		Reading mode (asynchrone or synchrone)
 * completor:	Reading callback
 * timeOut:		time period within which the reading must be completed
 * flags:		end-of-message indicator
 *
 * Return an error code
 *
 */
 
OSErr CConnection::DataRead(Ptr inBuffer,long *buffSize,Boolean async,
							CommProcPtr completor,long timeOut,CMFlags *flags,CMChannel channel)
{
 CMBufferSizes	sizes;
 CMStatFlags	statFlags;
 
 getStatus (&sizes, &statFlags);
 	
 return ((statFlags & cmStatusDRPend) == 0L ?
 		 CMRead(itsConn,inBuffer,buffSize,channel,async,completor,timeOut,flags) : 0);
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoIdle
 *
 * Idle time of the application
 *
 */

void CConnection::DoIdle(void)
{	
	CMIdle(itsConn);
	
	if (!(this->IsOpen()) && this->connOpen)
	{
		this->connOpen = FALSE;
		BroadcastChange(connCloseInd,NULL);
	};
}										

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cOneConnIdle
 * 
 * terminal idle time
 *
 */
 
pascal void	*CConnection::cOneConnIdle (void *threadParam)
{
	CConnection *aConn = (CConnection *)threadParam;
	
	#ifdef __USETHREADS__
	for (;;)
	{
		ThreadBeginCritical();
	#endif

		aConn->DoIdle();	

	#ifdef __USETHREADS__
		ThreadEndCritical();
		YieldToAnyThread();
	}
	#endif

	return nil;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DataWrite
 *
 * Data writing
 *
 * inBuffer:	Sending buffer
 * buffSize:	Number of chars to send
 * async:		Writing mode (synchrone or asychrone)
 * completor:	Asynchrone writing callback
 * timeOut:		time period within which the writing must be completed
 * flags:		end-of-message indicator
 *
 * Return an error code
 *
 */
 
OSErr CConnection::DataWrite(Ptr inBuffer,long *buffSize,Boolean async,
							CommProcPtr completor,long timeOut,CMFlags flags)
{
	return (CMWrite(itsConn,inBuffer,buffSize,cmData,async,completor,timeOut,flags));
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Activate
 *
 * Connection activation
 *
 */

void CConnection::Activate(void)
{
	if (!active)
	{
		CMActivate(itsConn,TRUE);
		active = TRUE;
	}
}										

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Deactivate
 *
 * Connection desactivation
 *
 */

void CConnection::Deactivate(void)
{
	if (active)
	{
		CMActivate(itsConn,FALSE);
		active = FALSE;
	}
}										

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Reset
 *
 * Connection reset
 *
 */

void CConnection::Reset(void)
{
	short	response;
	
	PositionDialog('ALRT', RESET_ALRT_ID);
	
	InitCursor();
	
	response = Alert(RESET_ALRT_ID, NULL);
		
	if (response == answerNO)
		CMReset(itsConn);
}										

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * SendBreak
 *
 * BREAK signal sending
 *
 */

void CConnection::SendBreak(void)
{
	CMBreak(itsConn,BREAK_DELAY,FALSE,NULL);
}										

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetEnvirons
 *
 * Return the connection environs
 *
 * theEnvirons: Pointer on the environs record
 *
 * Return an error code
 *
 */
 
OSErr CConnection::GetEnvirons(ConnEnvironRecPtr theEnvirons)
{
	return(CMGetConnEnvirons(itsConn,theEnvirons));
}	

/*
 * get the connection handle
 */
 
ConnHandle CConnection::GetConnHandle (void)
{
 return itsConn;
}

/*******************************************************************
*	DefAutoRecCallback	- Sets the file transfer flag if an auto-
*					receive string was found.
*
*		theConn			- which connection tool found it
*		data			- ptr to last character in the match
*		refNum			- which search was found
*
**********************************************************************/

pascal void CConnection::DefAutoRecCallback(ConnHandle theConn, Ptr data, long refNum)
{
 CConnection	*theConnection = (CConnection *)CMGetRefCon (theConn);
 CFileTransfer	*itsFileTransfer = nil;
 
 // if we belong to a terminal pane, get its file transfer object
 if (member(theConnection->itsSupervisor, CTermPane))
 	itsFileTransfer = ((CTermPane *)(theConnection->itsSupervisor))->GetFileTransfer ();

 // We can't call _FTStart or _CMRemoveSearch here as 	
 // this proc might be called from Interrupt level		
 
 if (itsFileTransfer)
 {
  if (itsFileTransfer->GetSearchNum () == refNum)
  	  itsFileTransfer->SetAutoDownload (true);		// Set the flag to call FTStart in Idle
 }
} // DefAutoRecCallBack	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */
