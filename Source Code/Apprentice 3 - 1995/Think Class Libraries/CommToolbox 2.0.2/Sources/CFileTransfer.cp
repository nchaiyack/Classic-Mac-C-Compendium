/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�

	CFileTransfer.c
	
	CommToolbox file transfer class.
	
	SUPERCLASS = CBureaucrat.
	
	Original copyright � 1992-93 Romain Vignes. All rights reserved.
	Modifications copyright � 1994-95 Ithran Einhorn. All rights reserved.
	
같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같� */


#include <CommResources.h>						/* Apple includes */

#include <CBartender.h>							/* TCL includes */
#include <CPtrArray.h>
#include <CError.h>
#include <Constants.h>
#include <TBUtilities.h>
#include <TCLUtilities.h>

#include "CTermPane.h"							/* Other includes */
#include "CFileTransfer.h"

/* Constants & Macros */

#define FTRANS_STR_RES_ID	2300	/* Transfer messages resource ID */

#define NO_TOOL_STR_INDEX	1		/* No transfer tool */
#define BAD_TOOL_STR_INDEX	2		/* Bad transfer tool */
#define NO_REC_STR_INDEX	3		/* Transfer record allocation error */
#define CHOOSE_STR_INDEX	4		/* tool setup error */
#define START_ERR_STR_INDEX	5		/* transfer start error */
#define SUCCESS_STR_INDEX	6		/* Transfer is successful */
#define RUN_ERR_STR_INDEX	7		/* Transfer failed */

#define H_CHOOSE_POS		10		/* Setup dialog position */
#define V_CHOOSE_POS		40

#define TEXT_FILE_TYPE		'TEXT'


/* Application globals */

extern CBartender	*gBartender;
extern CError		*gError;

TCL_DEFINE_CLASS_M1(CFileTransfer,  CBureaucrat);

/* Class variables initialization */

CPtrArray<CFileTransfer>	*CFileTransfer::cFTransList = NULL;

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cIsFileTransferCmd
 *
 * File transfer related command ?
 *
 * theCmd:	the command to be analysed
 *
 * Return TRUE if the command is file transfer related
 *
 */
 
Boolean CFileTransfer::cIsFileTransferCmd(long theCmd)
{
	return ((theCmd >= cmdFTransChoose) && (theCmd <= cmdFTransRecv));
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cInitManager
 *
 * File Transfer Manager Initialization
 *
 */
 
void CFileTransfer::cInitManager(void)
{
	InitFT();
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cGetCMVersion
 *
 * return the version of the File Transfer Manager
 *
 */
 
short CFileTransfer::cGetFTVersion(void)
{
	return FTGetFTVersion();
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cCheckToolName
 *
 * Checking of the tool existence
 *
 * toolName:	name of the tool (pascal string)
 *
 * Return an error code
 *
 */
 
OSErr CFileTransfer::cCheckToolName(Str31 toolName)
{
	return(FTGetProcID(toolName));
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cFTransIdle
 *
 * Idle time for each file transfer object
 *
 *
 */
 
 		/* Idle routine for each file transfer object */

		static void	FTrans_Idle(CFileTransfer *theFTrans)
		{
			theFTrans->DoIdle();
		}


void CFileTransfer::cFTransIdle(void)
{
	if (cFTransList != NULL)		/* List exists ? */
		cFTransList->DoForEach(FTrans_Idle);
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cTestToolMenu
 *
 * Test if the selected menu belongs to a file transfer tool
 *
 * theMenu:	selected menu ID
 * theItem:	selected item ID
 *
 * Return TRUE if the menu is a file transfer tool menu
 */
 

 		/* Test routine */

		static Boolean FTransTest(CFileTransfer *theFTrans)
		{
			return theFTrans->active;
		}


Boolean CFileTransfer::cTestToolMenu(short theMenu, short theItem)
{
	CFileTransfer	*current;
	
	if (cFTransList == NULL)
		return FALSE;
	else
	{
		current = (CFileTransfer *) cFTransList->FindItem(FTransTest);
		
		if (current == NULL)
			return FALSE;
		else
			return current->DoMenu(theMenu,theItem);
	}
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cTestToolEvent
 *
 * Test if the event is related to a file transfer tool
 *
 * macEvent:	pointer on the event record
 * theWindow:	pointer on the window record
 *
 * Return TRUE if the event is a file transfer tool event
 */

typedef struct	{
	EventRecord	*theEvent;
	WindowPtr	theWindow;
	Boolean		isToolEvent;
} TestParamRec;


 		/* Test routine */

		static void FTransEvtTest(CFileTransfer *theFTrans,long params)
		{
			TestParamRec *testRecPtr = (TestParamRec *)params;
			
			if (testRecPtr->isToolEvent == FALSE)
				testRecPtr->isToolEvent = theFTrans->DoEvent(testRecPtr->theEvent,
															 testRecPtr->theWindow);
		}


Boolean CFileTransfer::cTestToolEvent(EventRecord *macEvent,WindowPtr theWindow)
{
	TestParamRec params;
	
	params.theEvent = macEvent;
	params.theWindow = theWindow;
	params.isToolEvent = FALSE;
	
	if (cFTransList == NULL)
		return FALSE;
	else
	{
		cFTransList->DoForEach1(FTransEvtTest,(long)&params);
		
		return params.isToolEvent;
	}
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * CFileTransfer
 *
 * File transfer object initialization
 *
 * aSupervisor:			Supervisor in the chain of commands
 * toolName:			Name of tool to use ("" -> default)
 * flags:(FT)			Tool use flag
 * sendProc:(FT)		Chars sending proc
 * recvProc:(FT)		Chars receiving proc
 * readProc:(FT)		Chars reading proc
 * writeProc:(FT)		Chars writing proc
 * environsProc:(FT)	Environment description proc
 * refcon:(FT)			available for the application
 * userData:(FT)		available for the application
 *
 * Parameters followed by FT are exact required parameters for creating a
 * file transfer record.
 *
 */
 
CFileTransfer::CFileTransfer(CBureaucrat *aSupervisor,Str31 toolName,
							 FTFlags flags,FTSendProcPtr sendProc,FTRcveProcPtr recvProc,
							 FTReadProcPtr readProc,FTWriteProcPtr writeProc,FTEnvProcPtr environsProc,
							 WindowPtr owner,long refCon, long userData)
						
			 : CBureaucrat(aSupervisor)	//	Initialize superclass
{
	FTHandle	theFTrans;
	OSErr		theErr;
	Str31		tName;
	short		toolProcID;
	Boolean		savedAlloc;
	
	gStartFT = false;
	gFTSearchRefNum = 0L;		// Auto-Initiate File Transfers	
			
	// if we belong to a terminal pane, get its connection
	if (member(aSupervisor, CTermPane))
		BindConnection(((CTermPane *)aSupervisor)->GetConnection());
	
	if (toolName[0] == 0)		/* Default tool ? */
	{
		theErr = CRMGetIndToolName(classFT,1,tName);
		
		if ((theErr != ftNoErr) || (tName[0] == 0))	 	/* No tool */
			Failure(ftNoTools,SpecifyMsg(FTRANS_STR_RES_ID,NO_TOOL_STR_INDEX));

		toolProcID = cCheckToolName(tName);			/* Default tool ID */
	}
	else
		toolProcID = cCheckToolName(toolName);			/* Chosen tool ID */
	
	if (toolProcID == ftGenericError)			/* No tool */
			Failure(ftNoTools,SpecifyMsg(FTRANS_STR_RES_ID,BAD_TOOL_STR_INDEX));

	if (sendProc == nil)
		sendProc = (FTSendProcPtr)DefSendProc;

	if (recvProc == nil)
		recvProc = DefReceiveProc;
		
	if (environsProc == nil)
		environsProc = (FTEnvProcPtr)DefGetFTEnvirons;
		
	savedAlloc = SetAllocation(kAllocCanFail);
	
	theFTrans = FTNew(toolProcID,flags,sendProc,recvProc,readProc,writeProc,
					environsProc,owner,(long)this,userData);
					
	SetAllocation(savedAlloc);
	
	FailNIL(theFTrans);				/* File transfer record created ? */

	MoveHHi((Handle)theFTrans);		/* Heap fragmentation� */

	itsFTrans = theFTrans;			/* Instance variable */
	
	if (cFTransList == NULL)		/* FIrst transfer object ? */
	{
		cFTransList = new CPtrArray<CFileTransfer>;
	}
		
	cFTransList->Add(this); 		/* Transfer addition */
		
	this->wasFTMode = FALSE;	
	this->active = FALSE;
	
	AddSearch();					/* allow for auto-downloads */
	
	#ifdef __USETHREADS__
	if (CThread::cIsPresent())
	{
		itsIdleThread = new CThread(kCooperativeThread, (ThreadEntryProcPtr)cOneFTIdle, this);
	}
	#endif
}


/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Destructor
 *
 * File transfer object removal
 *
 */
 
CFileTransfer::~CFileTransfer()
{
	ASSERT(cFTransList != NULL);
	
	#ifdef __USETHREADS__
	if (CThread::cIsPresent())
	{
		delete itsIdleThread;
	}
	#endif
	
	RemoveSearch();					/* tell connection tool to forget about auto-downloads */
	
	cFTransList->Remove(this);		/* Dispose of the file transfer */
	
	if (cFTransList->IsEmpty())		
		ForgetObject(cFTransList);	/* Dispose of the cluster */

	FTDispose(itsFTrans);			/* Transfer record disposal */
	itsFTrans = NULL;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * UpdateMenus
 *
 * Transfer related menus updating
 *
 */
 
void CFileTransfer::UpdateMenus(void)
{
	if (!IsRunning())
	{
		gBartender->EnableCmd(cmdFTransChoose);		/* Setup command */
		
		if (!((*itsFTrans)->attributes & ftSendDisable))
			gBartender->EnableCmd(cmdFTransSend);	/* Send command */
	
		if (!((*itsFTrans)->attributes & ftReceiveDisable))
			gBartender->EnableCmd(cmdFTransRecv);	/* Receive command */
	}
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoCommand
 *
 * Handle file transfer commands
 *
 * theCommand:	command to be executed
 *
 */
 
void CFileTransfer::DoCommand(long theCommand)
{
	switch (theCommand)
	{		
		case cmdFTransChoose:		/* Transfer tool setup */
			this->FileTransferChoose();
			break;
			
		case cmdFTransSend:		/* File sending */
			this->Start(ftTransmitting);
			break;
			
		case cmdFTransRecv:		/* File receiving */
			this->Start(ftReceiving);
			break;
			
		default:				/* Unknown command */
			break;
	}
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * FileTransferChoose
 *
 * Transfer tool setup
 *
 */
 
void CFileTransfer::FileTransferChoose(void)
{
	short		retCode;
	Point		where;
	FTHandle	hFTrans;
	
	hFTrans = this->itsFTrans;
	HUnlock((Handle)hFTrans);
	
	SetPt(&where,H_CHOOSE_POS,V_CHOOSE_POS);	/* Dialog position */
												
	retCode = FTChoose(&hFTrans,where,NULL);
	
	HLock((Handle)hFTrans);
	this->itsFTrans = hFTrans;

	switch (retCode)
	{		
		case chooseCancel:		/* Setup cancelling */
			break;
			
		case chooseOKMinor:		/* Same tool, changed config */
		case chooseOKMajor:		/* Changed tool */
		
			RemoveSearch();		// reset the auto-download search pattern.
			AddSearch();
		
			active = FALSE;
			this->Activate();				/* Activation */
		
			itsSupervisor->Notify(NULL); 	/* Notify document update */
			
			break;
			
		default:				/* Unknown code */
			SysBeep(3);
			gError->PostAlert(FTRANS_STR_RES_ID,CHOOSE_STR_INDEX);
			break;	
	}
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * SetConfig
 *
 * Modify file transfer tool config
 *
 * theConfig:	configuration string (C string))
 *
 * Renvoie:		negative value: error (-1 -> unknown)
 *				positive value: parser stop index
 *				cmNoErr if all is OK
 *
 */
 
short CFileTransfer::SetConfig(char *theConfig)
{
	short retCode;
	
	retCode = FTSetConfig(itsFTrans,theConfig);
	
	if (retCode == ftNoErr)
	{
		RemoveSearch();		// reset the auto-download search pattern.
		AddSearch();
	}
	
	return retCode;
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetToolName
 *
 * Return the file transfer tool name
 *
 * toolName:	Name of the tool
 *
 */
 
void CFileTransfer::GetToolName(Str31 toolName)
{
	SignedByte	savedState;
	
	savedState = HGetState((Handle)itsFTrans);
	HLock((Handle)itsFTrans);
	
	FTGetToolName((*itsFTrans)->procID,toolName);
	
	HSetState((Handle)itsFTrans,savedState);
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetConfig
 *
 * Return the config string of the tool
 *
 * Return a pointer on a C string
 *
 */
 
Ptr CFileTransfer::GetConfig(void)
{
	return(FTGetConfig(itsFTrans));
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoIdle
 *
 * Idle time of the application
 *
 */

void CFileTransfer::DoIdle(void)
{
	if (IsRunning())
	{
		FTExec(itsFTrans);						/* Transfer tool handles these events */
	}
	else if (this->wasFTMode)
	{
		/* Running ? */
				
		this->wasFTMode = FALSE;				/* Transfer halted */
		
		AddSearch();							/* allow for searching for auto-download pattern. */
	}
	else if (AutoDownload ())
	{
		Start(ftReceiving);
	}
}										

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cOneFTIdle
 * 
 * terminal idle time
 *
 */
 
pascal void	*CFileTransfer::cOneFTIdle (void *threadParam)
{
	CFileTransfer *anFT = (CFileTransfer *)threadParam;
	
	#ifdef __USETHREADS__
	for (;;)
	{
		ThreadBeginCritical();
	#endif

		anFT->DoIdle();	

	#ifdef __USETHREADS__
		ThreadEndCritical();
		YieldToAnyThread();
	}
	#endif

	return nil;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Activate
 *
 * Transfer activation
 *
 */

void CFileTransfer::Activate(void)
{
	if (!active)
	{
		FTActivate(itsFTrans,TRUE);
		active = TRUE;
	}
}										

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Deactivate
 *
 * Transfert deactivation
 *
 */

void CFileTransfer::Deactivate(void)
{
	if (active)
	{
		FTActivate(itsFTrans,FALSE);
		active = FALSE;
	}
}										

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * IsRunning
 *
 * Return the state of the file transfer
 *
 * Return TRUE if the transfer is running
 *
 */

Boolean CFileTransfer::IsRunning(void)
{	
	return ((*itsFTrans)->flags & ftIsFTMode);
}										

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * IsRunningOnSameCircuit
 *
 * Return the state of the file transfer
 *
 * Return TRUE if the transfer is running on same circuit
 *
 */

Boolean CFileTransfer::IsRunningOnSameCircuit(void)
{	
	return (IsRunning() && ((*itsFTrans)->attributes & ftSameCircuit));
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * NeedsConnection
 *
 * Return the state of the file transfer
 *
 * Return TRUE if the transfer wants the connection
 *
 */

Boolean CFileTransfer::NeedsConnection(void)
{	
	return IsRunning() || AutoDownload() || wasFTMode;
}										

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Start
 *
 * File transfer startup
 *
 * direction:	transfer direction
 *
 */

void CFileTransfer::Start(FTDirection direction)
{
	SFReply		theReply;
	Point		corner;
	short		numTypes;
	SFTypeList	typeList;
	OSErr		theErr;
	
	switch (direction)
	{		
		case ftTransmitting:	/* Sending */
		
			if ((*itsFTrans)->attributes & ftTextOnly)	 /* Text files only */
			{
				typeList[0] = TEXT_FILE_TYPE;
				numTypes = 1;
			}
			else
				numTypes = -1;			/* All types of file */
				
			FindDlogPosition('DLOG', getDlgID, &corner);
	
			SFPGetFile(corner,"\p", NULL, numTypes, typeList,
						NULL,&theReply, getDlgID, NULL);
						
			if (theReply.good)	/* Validate */
			{
			
				theErr = FTStart(itsFTrans,ftTransmitting,&theReply);
				
				if (theErr == ftNoErr)
				{	
					/* Erreur de lancement */
					this->wasFTMode = TRUE;
				}
			}
		
			break;
			
		case ftReceiving:		/* Receiving */
		
			// We remove the search temporarily in case it comes		
			// across during the transfer. Will be re-added in the	
			// idle loop once the transfer is completed				
			
			RemoveSearch();
			
			theReply.vRefNum = 0;
			theReply.fName[0] = 0;
			theReply.good = TRUE;
		
			theErr = FTStart(itsFTrans,ftReceiving,&theReply);
			
			if (theErr == ftNoErr)
			{	
				/* Erreur de lancement */
				this->wasFTMode = TRUE;				
			}
			break;
			
		default:				/* Other */
			break;
	}
}										

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoEvent
 *
 * Transfer related event handling
 *
 * theEvent:	Pointer on the event record
 * theWindow:	Window in which occured the event
 *
 * Renvoie TRUE if the event is handled by the tool
 *
 */
 
Boolean CFileTransfer::DoEvent(EventRecord *theEvent,WindowPtr theWindow)
{
	Boolean	isToolEvent;
	FTHandle	ftHdl;
	
	isToolEvent = FALSE;
	
	ftHdl = (FTHandle) GetWRefCon(theWindow);
	
	if (ftHdl == itsFTrans)	/* Tool window ? */
	{
		FTEvent(itsFTrans,theEvent);
		isToolEvent = TRUE;
	}
	
	return isToolEvent;
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Success
 *
 * Get the ending status of the file transfer
 *
 * Renvoie TRUE if transfer was successfull
 *
 */

Boolean CFileTransfer::Success(void)
{
	return ((*itsFTrans)->flags & ftSucc);
}										

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * DoMenu
 *
 * Handle tool menus
 *
 * theMenu:	selected menu
 * theItem:	selected item
 *
 * Return TRUE if the menu belongs to the tool
 *
 */
 
Boolean CFileTransfer::DoMenu(short theMenu,short theItem)
{
	return(FTMenu(itsFTrans,theMenu,theItem));
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetFTHandle
 *
 * return the FT handle
 *
 */
 
FTHandle CFileTransfer::GetFTHandle (void)
{
 return itsFTrans;
}
	
/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * BindConnection, GetConnection
 *
 * manage xfer's connection
 *
 */
 
void CFileTransfer::BindConnection(CConnection *aConnection)
{
 itsConnection = aConnection;
}

CConnection *CFileTransfer::GetConnection(void)
{
 return itsConnection;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * AddSearch
 *
 * Add auto download search string
 *
 */

void CFileTransfer::AddSearch (void)
{
	Str255	tempStr;	// the string to look for
	
	if ((itsFTrans != NULL) && (itsConnection)) 
	{
		if ((*itsFTrans)->autoRec[0]) // Do I need to add a search
		{
			SignedByte	savedState;
		
			savedState = HGetState((Handle)itsFTrans);
			HLock((Handle)itsFTrans);
			
			BlockMove((*itsFTrans)->autoRec, tempStr, (*itsFTrans)->autoRec[0] + 1);
					
			gFTSearchRefNum = itsConnection->AddSearch(tempStr, cmSearchSevenBit, nil);
			
			if (gFTSearchRefNum == -1) 
			{
				gFTSearchRefNum = 0;
			}
			
			HSetState((Handle)itsFTrans,savedState);
			
		} // can autoreceive	
		
	} // good FT and Conn	
	
}
	
/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * RemoveSearch
 *
 * Remove auto download search string
 *
 */
 
void CFileTransfer::RemoveSearch (void)
{
 gStartFT = false;
 
 if (itsConnection)
	 if (((*itsFTrans)->autoRec[0] != 0) && (gFTSearchRefNum != 0)) 
	 {
		itsConnection->RemoveSearch(gFTSearchRefNum);
		gFTSearchRefNum = 0;	// We found it already	
	 }
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * get/set instance variables
 *
 *
 */
 
long CFileTransfer::GetSearchNum (void)
{
 return gFTSearchRefNum;
}
void CFileTransfer::SetSearchNum (long SearchNum)
{
 gFTSearchRefNum = SearchNum;
}
	
Boolean	CFileTransfer::AutoDownload (void)
{
 return gStartFT;
}
void	CFileTransfer::SetAutoDownload (Boolean state)
{
 gStartFT = state;
}
	
/*******************************************************************
*	ToolGetConnEnvirons	- Gets the connection environs for
*						the FT or Term tool
*
*		refcon			- the tool refcon
*		theEnvirons		- the environment
*
*		returns			- an environment error
*
**********************************************************************/

pascal OSErr CFileTransfer::DefGetFTEnvirons(long refCon, ConnEnvironRec *theEnvirons)
{
 CFileTransfer	*theFTPane = (CFileTransfer *)refCon;
 CConnection	*theConnection = theFTPane->GetConnection();
 
 return theConnection->GetEnvirons(theEnvirons);
}

/*******************************************************************
*	FTSendProc	- Sends data during a file transfer
*
*		thePtr		- data to send
*		theSize		- bytes to send
*		refcon		- the FTtool refcon
*		channel		- which channel to use
*		flags		- connection flags
*
*		returns		- bytes sent
*
**********************************************************************/

pascal long CFileTransfer::DefSendProc(Ptr thePtr,long theSize,long refCon,CMChannel channel,short flags)
{
	CMErr			theErr;
	long			retCode = 0;					// Assume the worst		
	CFileTransfer	*theFTPane = (CFileTransfer *)refCon;
	CConnection		*theConnection = theFTPane->GetConnection();
	
	if (theConnection->GetConnHandle())
	{
		// Send the data			
		theErr = theConnection->DataWrite(thePtr,&theSize,channel,nil,15,flags);
		if (theErr == noErr)
			retCode = theSize;		// if ok, we sent all	

	} // Good Connection	
	
	return retCode;
}

/*******************************************************************
*	FTReceiveProc	- Gets data during a file transfer
*
*		thePtr		- place for data
*		theSize		- bytes to get
*		refcon		- the FTtool refcon
*		channel		- which channel to use
*		flags		- connection flags
*
*		returns		- bytes gotten
*
**********************************************************************/

pascal long CFileTransfer::DefReceiveProc (Ptr thePtr,long theSize,long refCon,CMChannel channel,CMFlags *flags)
{
	CMErr			theErr;
	long			retCode = 0;					// Assume the worst
	CFileTransfer	*theFTPane = (CFileTransfer *)refCon;
	CConnection		*theConnection = theFTPane->GetConnection();
	
	if (theConnection->GetConnHandle())
	{
		// Read all the data	
		theErr = theConnection->DataRead (thePtr, &theSize, FALSE, NULL, 0L, flags, channel);
		if (theErr == noErr)
			retCode = theSize;	// if ok, we got all		

	} // Good Connection	
	
	return retCode;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */
