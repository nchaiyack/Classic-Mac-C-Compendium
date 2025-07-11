/*********************************************************************
Project	:	XL				-	XCMDs for everyone
File		:	XL.h			-	The interface
Author	:	Matthias Neeracher
Started	:	10Sep92								Language	:	MPW C/C++
Modified	:	10Sep92	MN	
Last		:	10Sep92
*********************************************************************/

#ifndef __XL__
#define __XL__

#include <HyperXCmd.h>

enum {
	xl_Undefined,				/* This *must* be defined */
	xl_SendCardMessage,
	xl_EvalExpr,
	xl_StringLength,
	xl_StringMatch,
	xl_SendHCMessage,
	xl_ZeroBytes,
	xl_PasToZero,
	xl_ZeroToPas,
	xl_StrToLong,
	xl_StrToNum,
	xl_StrToBool,
	xl_StrToExt,
	xl_LongToStr,
	xl_NumToStr,
	xl_NumToHex,
	xl_BoolToStr,
	xl_ExtToStr,
	xl_GetGlobal,
	xl_SetGlobal,
	xl_GetFieldByName,
	xl_GetFieldByNum,
	xl_GetFieldByID,
	xl_SetFieldByName,
	xl_SetFieldByNum,
	xl_SetFieldByID,
	xl_StringEqual,
	xl_ReturnToPas,
	xl_ScanToReturn,
	xl_GetMaskAndData,
	xl_ChangedMaskAndData,
	xl_ScanToZero				=	39,
	
	XL_SIZE
};

#ifdef XLDEBUG
typedef enum {
	xl_NoDebug,
	xl_DebugUndefined,
	xl_DebugAll
} XLDebugLevel;
#endif

typedef void (*XLHandler)(XCmdPtr);

typedef XLHandler	XLGlue[XL_SIZE];

extern XLGlue 			XLDefaultGlue;
#ifdef XLDEBUG
extern XLDebugLevel 	XLDebug;
#endif

void XLCall(Handle xcmd, XLHandler glue[], XCmdPtr params);
void XLCopyGlue(XLHandler * to, XLHandler * from);

#endif
