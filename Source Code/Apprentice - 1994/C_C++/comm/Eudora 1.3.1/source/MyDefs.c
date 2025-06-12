/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * This file contains important things common to all source files
 **********************************************************************/
#pragma load SYS_LOAD

/**********************************************************************
 * a pointer into nowhere
 **********************************************************************/
#ifndef nil
#define nil ((long)0)
#endif

/**********************************************************************
 * make dealing with handles a little less unpleasant
 **********************************************************************/
#define LDRef(aHandle)	(HLock((aHandle)),*(aHandle))
#define UL(aHandle) 		HUnlock((aHandle))
#define New(aType)		((aType *)NuPtr(sizeof(aType)))
#define NewH(aType) 	((aType **)NuHandle(sizeof(aType)))
#define NewZH(aType)	 ((aType **)NewZHandle(sizeof(aType)))
#define ZapHandle(aHandle)	do{DisposHandle(aHandle);aHandle=nil;}while(0)
#define ZapPtr(aPtr)	do{DisposPtr(aPtr);aPtr=nil;}while(0)

/************************************************************************
 * repeat ten times: "I hate the Memory Manager."
 ************************************************************************/
#define OFFSET_RECT(tangle,dh,dv) do {																	\
	Rect r = *(tangle); 																									\
	OffsetRect(&r,dh,dv); 																								\
	*(tangle) = r;																												\
	} while (0)
#define INSET_RECT(tangle,dh,dv) do { 																	\
	Rect r = *(tangle); 																									\
	InsetRect(&r,dh,dv);																									\
	*(tangle) = r;																												\
	} while (0)
#define SET_RECT(tangle,lf,tp,rt,bt) do { 															\
	Rect r = *(tangle); 																									\
	SetRect(&r,lf,tp,rt,bt);																							\
	*(tangle) = r;																												\
	} while (0)
#define INVAL_RECT(tangle) do { 																				\
	Rect r = *(tangle); 																									\
	InvalRect(&r);																												\
	} while (0)
#define VALID_RECT(tangle) do { 																				\
	Rect r = *(tangle); 																									\
	ValidRect(&r);																												\
	} while (0)
#define ERASE_RECT(tangle) do { 																				\
	Rect r = *(tangle); 																									\
	EraseRect(&r);																												\
	} while (0)
#define FRAME_RECT(tangle) do { 																				\
	Rect r = *(tangle); 																									\
	FrameRect(&r);																												\
	} while (0)

/**********************************************************************
 * some #defines that don't seem to belong anywhere else
 **********************************************************************/
#define TRUE	true
#define FALSE false
#define True	true
#define False false
#define ENVIRONS_VERSION 2	/* the version of SysEnvirons we expect */
#define InFront ((UPtr)-1) /* for window creation */
#define INFINITY 32766		/* infinity, more or less */
#define FSFCBLen (*(short *)0x3f6)
#define TheMenu (*(short *)0xa26)
#define TABKEY 9
#define CANCEL_ITEM 		(-1)
#define MIN(x,y)				(((x) < (y)) ? (x) : (y))
#define MAX(x,y)				(((x) < (y)) ? (y) : (x))
#define ABS(x)					((x)<0 ? -(x) : (x))
#define GROW_SIZE 15
#define MAX_DEPTH 20		/* max depth for alias tree */
#define fInited (1<<8)	/* why doesn't apple define this? */
#define userCancelled 	-29999 /* user cancelled tcp operations */
#define IsWhite(c)			(c==' ' || c=='\t')
#define K *1024
#define MAX_ALIAS 200
#define SAVE_PORT				GrafPtr oldPort = qd.thePort
#define REST_PORT				SetPort(oldPort)
#ifdef DEBUG
#define CHECKPOINT do{SpinSpot = __LINE__;}while(0)
#define DBLINE do{Str255 s;DebugStr(ComposeString(s,"\p%s %d;hc;sc;g",__FILE__,__LINE__));}while(0)
#define ASSERT(expr) do{if (!(expr) && RunType!=Production) {Str255 s;DebugStr(ComposeString(s,"\passertion failed: " #expr ", %s:%d",__FILE__,__LINE__));}}while(0)
#else
#define CHECKPOINT
#define DBLINE
#define ASSERT(expr)
#endif
#define SPARE_SIZE		((long)40*1024)
#define MEM_CRITICAL	((long)8*1024)

/************************************************************************
 * transport mechanisms
 ************************************************************************/
typedef struct
{
	int (*vConnectTrans)(); 			/* UPtr serverName, short port */
	int (*vSendTrans)();					/* short count, UPtr text,long size, ... */
	int (*vRecvTrans)();					/* UPtr line,long *size */
	int (*vDisTrans)(); 					/* void */
	int (*vDestroyTrans)(); 			/* void */
	int (*vTransError)(); 				/* void */
	void (*vSilenceTrans)();			/* Boolean silence */
	int (*vSendWDS)();						/* wdsEntry *theWDS */
	unsigned char * (*vWhoAmI)(); /* Uptr who */
	int (*vRecvLine)(); 					/* UPtr line,long *size */
} TransVector;

#define ConnectTrans (*CurTrans.vConnectTrans)
#define SendTrans (*CurTrans.vSendTrans)
#define RecvTrans (*CurTrans.vRecvTrans)
#define DisTrans (*CurTrans.vDisTrans)
#define DestroyTrans (*CurTrans.vDestroyTrans)
#define TransError (*CurTrans.vTransError)
#define SilenceTrans (*CurTrans.vSilenceTrans)
#define SendWDS (*CurTrans.vSendWDS)
#define WhoAmI (*CurTrans.vWhoAmI)
#define RecvLine (*CurTrans.vRecvLine)
/**********************************************************************
 * some handy types
 **********************************************************************/
typedef enum {Single, Double, Triple} ClickEnum;
typedef enum {
	MBOX_WIN=userKind+1,
	CBOX_WIN,COMP_WIN,ALIAS_WIN,MESS_WIN,FIND_WIN,PH_WIN,TEXT_WIN,MB_WIN
}WKindEnum;
typedef unsigned char *UPtr, **UHandle, *PStr, *CStr;
#define Uptr UPtr
#define Uhandle UHandle
typedef unsigned short uShort;
typedef unsigned long uLong;
typedef enum {Production, Debugging, Steve} RunTypeEnum;
typedef unsigned char Str127[128];

#include "numcode.h"
#include "MyRes.h"
#include "StringDefs.h"
#include "cursor.h"
#include "progress.h"
#include "mywindow.h"
#include "scrollte.h"
#include "modeless.h"
#include "ends.h"
#include "functions.h"
#include "inet.h"
#include "lineio.h"
#include "mailbox.h"
#include "boxact.h"
#include "main.h"
#include "message.h"
#include "messact.h"
#include "comp.h"
#include "compact.h"
#include "sendmail.h"
#include "pop.h"
#include "tcp.h"
#include "ctb.h"
#include "menu.h"
#include "shame.h"
#include "sort.h"
#include "tefuncs.h"
#include "util.h"
#include "fileutil.h"
#include "winutil.h"
#include "nickwin.h"
#include "nickexp.h"
#include "undo.h"
#include "find.h"
#include "text.h"
#include "Globals.h"
#include "mytcp.h"
#include "address.h"
#include "print.h"
#include "nickmng.h"
#include "binhex.h"
#include "hexbin.h"
#include "ph.h"
#include "utl.h"
#include "prefs.h"
#include "mbwin.h"
#include "buildtoc.h"
#include "squish.h"
#include "uudecode.h"
#include "uupc.h"
#include "md5.h"
#include "lmgr.h"
#include "log.h"
#include "unload.h"

typedef struct {short flags;uLong prefSize;uLong minSize} SizeRec,*SizePtr,**SizeHandle;

#pragma dump EUDORA_LOAD

