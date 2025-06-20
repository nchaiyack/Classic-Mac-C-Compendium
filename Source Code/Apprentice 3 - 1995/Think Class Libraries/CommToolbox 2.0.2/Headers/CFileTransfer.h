/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�

	CFileTransfer.h
	
	CommToolbox file transfer class.
	
	SUPERCLASS = CBureaucrat.
	
	Original copyright � 1992-93 Romain Vignes. All rights reserved.
	Modifications copyright � 1994-95 Ithran Einhorn. All rights reserved.
	
같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같� */

#ifndef _H_CFileTransfer
#define _H_CFileTransfer

#include <FileTransfers.h>					/* Apple includes */

#ifdef __USETHREADS__
	#include "CThread.h"					/* Thread Manager */
#endif

#include <CBureaucrat.h>					/* Interface for its superclass */
#include "CPtrArray.h"

#include "CConnection.h"					/* connection definition. */

/* Commands */

#define cmdFTransChoose		2300			/* Transfer setup */
#define cmdFTransSend		2301			/* File sending */
#define cmdFTransRecv		2302			/* File receiving */

// callback proc type
typedef FileTransferReadProcPtr			FTReadProcPtr;
typedef FileTransferWriteProcPtr		FTWriteProcPtr;
typedef FileTransferSendProcPtr			FTSendProcPtr;
typedef FileTransferReceiveProcPtr		FTRcveProcPtr;
typedef FileTransferEnvironsProcPtr		FTEnvProcPtr;
typedef FileTransferNotificationProcPtr	FTNtfyProcPtr;
typedef FileTransferChooseIdleProcPtr	FTChIdleProcPtr;

/* class definition */

class CFileTransfer : public CBureaucrat	{

protected:
	TCL_DECLARE_CLASS

	/* Class Variables */
	
	static	CPtrArray<CFileTransfer>	*cFTransList;
	
	/* Instance Variables */

	FTHandle		itsFTrans;
	Boolean			wasFTMode;
	
	Boolean			gStartFT;				// Auto-start							
	long			gFTSearchRefNum;		// Auto-Initiate File Transfers			
	
public:

	/* Instance Variables */

	Boolean			active;
	CConnection		*itsConnection;

	#ifdef __USETHREADS__
	CThread			*itsIdleThread;
	#endif
	
	/* Class Methods */
	
	static void			cInitManager(void);
	static short		cGetFTVersion(void);
	static OSErr		cCheckToolName(Str31 toolName);
	static Boolean		cIsFileTransferCmd(long theCmd);
	static void			cFTransIdle(void);
	static pascal void	*cOneFTIdle (void *threadParam);
	static Boolean		cTestToolMenu(short theMenu, short theItem);
	static Boolean		cTestToolEvent(EventRecord *macEvent, WindowPtr theWindow);
	
	/* Instance Methods */
	
	CFileTransfer(CBureaucrat *aSupervisor,Str31 toolName,
				  FTFlags flags,FTSendProcPtr sendProc,FTRcveProcPtr recvProc,
				  FTReadProcPtr readProc,FTWriteProcPtr writeProc,FTEnvProcPtr environsProc,
				  WindowPtr owner,long refcon, long userData);
	virtual ~CFileTransfer();
	
	virtual void	UpdateMenus(void);
	virtual void	DoCommand(long theCommand);
	virtual void	FileTransferChoose(void);
	virtual short	SetConfig(char *theConfig);
	virtual void	GetToolName(Str31 toolName);
	virtual Ptr		GetConfig(void);
	virtual void	DoIdle(void);
	virtual void	Activate(void);
	virtual void	Deactivate(void);
	virtual Boolean	IsRunning(void);
	virtual Boolean	IsRunningOnSameCircuit(void);	
	virtual void	Start(FTDirection direction);
	virtual Boolean	DoEvent(EventRecord *theEvent,WindowPtr theWindow);
	virtual Boolean	Success(void);
	virtual Boolean	DoMenu(short theMenu,short theItem);
	
	virtual FTHandle GetFTHandle (void);
	
	virtual void	BindConnection(CConnection *aConnection);
	virtual CConnection *GetConnection(void);
	
	virtual Boolean	NeedsConnection (void);
	
	virtual void	AddSearch (void);
	virtual void	RemoveSearch (void);
	
	virtual long GetSearchNum (void);
	virtual void SetSearchNum (long SearchNum);
	
	virtual Boolean	AutoDownload (void);
	virtual void	SetAutoDownload (Boolean state);
	
	// static call-back routines
	static pascal CMErr	DefGetFTEnvirons (long refCon, ConnEnvironRecPtr theEnvPtr);
	static pascal long	DefSendProc (Ptr thePtr,long theSize,long refcon,CMChannel channel,short flags);
	static pascal long	DefReceiveProc (Ptr thePtr,long theSize,long refcon,CMChannel channel,CMFlags *flags);
};

#endif

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */
