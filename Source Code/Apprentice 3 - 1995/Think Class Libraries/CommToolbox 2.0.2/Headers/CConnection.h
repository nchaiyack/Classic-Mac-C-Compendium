/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°

	CConnection.h
	
	CommToolbox connection class.
	
	SUPERCLASS = CBureaucrat.
	
	Copyright © 1992-93 Romain Vignes. All rights reserved.
	
°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

#ifndef _H_CConnection
#define _H_CConnection

#ifdef __USETHREADS__
	#include "CThread.h"				/* Thread Manager */
#endif

#include <CBureaucrat.h>				/* Interface for its superclass */
#include "CPtrArray.h"

#include <Connections.h>				/* Apple includes */

/* Connection specific commands */

#define cmdConnChoose	2100			/* Connection setup */
#define cmdConnOpen		2101			/* Connection opening */
#define cmdConnClose	2102			/* Connection closing */
#define cmdConnReset	2103			/* Connection reset */
#define cmdConnBreak	2104			/* BREAK signal sending */
#define cmdConnListen	2105			/* Connection listening */

#define IOBUFSIZE (1024 * 1)

/* Broadcast indications */

enum	{
	connOpenInd = 2101,
	connCloseInd
};

// callback proc type
#ifndef CommProcPtr
typedef ConnectionCompletionProcPtr CommProcPtr;
#endif

typedef ConnectionSearchCallBackProcPtr CommSearchPtr;

/* Class definition */

class CConnection : public CBureaucrat	{

protected:
	TCL_DECLARE_CLASS

	/* Class Variables */
	
	static	CPtrArray<CConnection>	*cConnList;

	/* Instance Variables */

	ConnHandle		itsConn;
	Boolean			connOpen;
	
	#ifdef __USETHREADS__
	CThread			*itsIdleThread;
	#endif
	
	virtual OSErr	getStatus(CMBufferSizes *sizes,CMStatFlags *flags);
	
public:

	/* Instance Variables */

	Boolean			active;
	
	/* Class Methods */
	
	static Boolean		cIsConnectionCmd(long theCmd);
	static OSErr		cCheckToolName(Str31 toolName);
	static void			cConnIdle(void);
	static pascal void 	*cOneConnIdle (void *threadParam);
	static void			cInitManager(void);
	static short		cGetCMVersion(void);
	static Boolean		cTestToolEvent(EventRecord *macEvent,WindowPtr theWindow);

	/* Instance Methods */
	
	CConnection(CBureaucrat *aSupervisor,Str31 toolName,
				CMRecFlags flags,CMBufferSizes desiredSizes,
				long refcon, long userData);
	virtual ~CConnection();
	
	virtual void	UpdateMenus(void);
	virtual void	DoCommand(long theCommand);
	virtual	Boolean DoEvent(EventRecord *theEvent,WindowPtr theWindow);
	virtual void	ConnectionChoose(void);
	virtual short	SetConfig(char *theConfig);
	virtual Ptr		GetConfig(void);
	virtual void	OpenConnection(Boolean async,CommProcPtr completor,long timeOut);
	virtual void	ListenConnection(Boolean async,CommProcPtr completor,long timeOut);
	virtual void	CloseConnection(Boolean async,CommProcPtr completor,long timeOut,
						Boolean now);
	virtual Boolean	IsOpen(void);
	virtual long	DataAvail(void);
	virtual OSErr	DataRead(Ptr inBuffer,long *buffSize,Boolean async,
						CommProcPtr completor,long timeOut,CMFlags *flags,CMChannel channel = cmData);
	virtual void	DoIdle(void);
	virtual OSErr	DataWrite(Ptr inBuffer,long *buffSize,Boolean async,
						CommProcPtr completor,long timeOut,CMFlags flags);
	virtual void	Activate(void);
	virtual void	Deactivate(void);
	virtual void	Reset(void);
	virtual void	SendBreak(void);
	virtual OSErr	GetEnvirons(ConnEnvironRecPtr theEnvirons);
	virtual void	GetToolName(Str31 toolName);
	
	virtual long	AddSearch(Str255 theString, CMSearchFlags flags, CommSearchPtr callBack); 
	virtual void	RemoveSearch(long refNum); 
	virtual void	ClearSearch(void);
		
	virtual ConnHandle GetConnHandle (void);
	
	/* default call-back procs. */
	static pascal void DefAutoRecCallback(ConnHandle theConn, Ptr data, long refNum);
};

#endif

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */
