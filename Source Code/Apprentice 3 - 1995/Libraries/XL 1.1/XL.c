/*********************************************************************
Project	:	XL				-	XCMDs for everyone
File		:	XL.c			-	The implementation
Author	:	Matthias Neeracher
Started	:	10Sep92								Language	:	MPW C
Modified	:	04Jul94	MN	Adapted for PPCC
Last		:	04Jul94
*********************************************************************/

#include <Memory.h>
#include <StdIO.h>
#include <StdLib.h>
#include <String.h>
#include <TextUtils.h>

#include "XL.h"

static XLHandler *	XLglue;
static XCmdPtr			XLparams;

#ifdef XLDEBUG
XLDebugLevel			XLDebug	=	xl_NoDebug;

static char * Requests[] = {
	0, 
	"SendCardMessage",
	"EvalExpr", 
	"StringLength", 
	"StringMatch", 
	"SendHCMessage", 
	"ZeroBytes",
	"PasToZero",
	"ZeroToPas",
	"StrToLong",
	"StrToNum",
	"StrToBool", 
	"StrToExt", 
	"LongToStr", 
	"NumToStr", 
	"NumToHex", 
	"BoolToStr", 
	"ExtToStr", 
	"GetGlobal", 
	"SetGlobal",
	"GetFieldByName", 
	"GetFieldByNum", 
	"GetFieldByID", 
	"SetFieldByName", 
	"SetFieldByNum", 
	"SetFieldByID", 
	"StringEqual", 
	"ReturnToPas", 
	"ScanToReturn", 
	"GetMaskAndData", 
	"ChangedMaskAndData",
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	"ScanToZero"
};
#endif

static pascal void XLCallBack()
{
	short	request	=	XLparams->request;

	memset(XLparams->outArgs, 0, 4 * sizeof(Handle));
	
	if (request < 1 || request >= XL_SIZE || !XLglue[request])
		request	=	xl_Undefined;

#ifdef XLDEBUG
	switch (XLDebug) {
	case xl_NoDebug:
		break;
	case xl_DebugUndefined:
		if (request != xl_Undefined)
			break;
		
		/* Fall through */
	case xl_DebugAll:
		fprintf(stderr, "\nXL: Request %d (", XLparams->request);
		if (XLparams->request > 0 && XLparams->request < XL_SIZE && Requests[XLparams->request]) {
			if (request != XLparams->request)
				fprintf(stderr, "NOT IMPLEMENTED: ");
				
			fprintf(stderr, "%s)\n", Requests[XLparams->request]);
		} else
			fprintf(stderr, "UNKNOWN%s)\n", (request != XLparams->request) ? ", NOT IMPLEMENTED" : "");
		
		fprintf(stderr, "XL: inArgs   %08X %08X %08X %08X %08X %08X %08X %08X\n",
			XLparams->inArgs[0],
			XLparams->inArgs[1],
			XLparams->inArgs[2],
			XLparams->inArgs[3],
			XLparams->inArgs[4],
			XLparams->inArgs[5],
			XLparams->inArgs[6],
			XLparams->inArgs[7]);
	}
#endif
	
	(*XLglue[request])(XLparams);
	
#ifdef XLDEBUG
	if (XLDebug == xl_DebugAll) {
		fprintf(stderr, "XL: result: %d\n", XLparams->result);
		fprintf(stderr, "XL: outArgs  %08X %08X %08X %08X\n",
			XLparams->outArgs[0],
			XLparams->outArgs[1],
			XLparams->outArgs[2],
			XLparams->outArgs[3]);
	}
#endif
}

#if USESROUTINEDESCRIPTORS
RoutineDescriptor	uXLCallBack = 
		BUILD_ROUTINE_DESCRIPTOR(kPascalStackBased, XLCallBack);
#else
#define uXLCallBack	XLCallBack
#endif

void XLCall(Handle xcmd, XLHandler glue[], XCmdPtr params)
{
	char	state;
	
	XLparams	=	params;
	XLglue	=	glue;
	
	params->returnValue	=	nil;
	params->entryPoint	=	(Ptr) &uXLCallBack;
	
	state		=	HGetState(xcmd);
	HLock(xcmd);

#if USESROUTINEDESCRIPTORS
	CallUniversalProc((UniversalProcPtr)*xcmd, kPascalStackBased |	STACK_ROUTINE_PARAMETER(1, kFourByteCode), params);
#else
	((pascal void (*)(XCmdPtr)) *xcmd)(params);
#endif
	
	HSetState(xcmd, state);
}

void XLCopyGlue(XLHandler * to, XLHandler * from)
{
	BlockMoveData((Ptr) from, (Ptr) to, XL_SIZE * sizeof(XLHandler));
}

static void XLUndefined(XCmdPtr params)
{
	params->result	= xresNotImp;
}

static void XLStringLength(XCmdPtr params)
{
	params->outArgs[0] = strlen((Ptr) params->inArgs[0]);
	
	params->result	= xresSucc;	
}

static void XLStringMatch(XCmdPtr params)
{
	StringPtr	pattern	=	(StringPtr) params->inArgs[0];
	char *		text		=	(char *)		params->inArgs[1];
	
	for (; *text; ++text) {
		int		len	=	pattern[0];
		
		while (len--)
			if (pattern[len+1] != text[len])
				goto nextChar;
		
		params->outArgs[0] = (long) text;
		params->result	= xresSucc;	
		
		return;
nextChar:
		continue;
	}
	
	params->outArgs[0] = 0;
	params->result	= xresSucc;	
}

static void XLZeroBytes(XCmdPtr params)
{
	char *	bytes	=	(char *) params->inArgs[0];
	long *	longs;
	long		len	=	(long) params->inArgs[1];
	long		count;
	
	params->result	= xresSucc;

	/* handle short lengths */
	
	if (len<0)
		return;
	
	switch (len) {
	case 2:
		*bytes++ = 0;
	case 1:
		*bytes++ = 0;
	case 0:
		return;
	default:
		break;
	}
	
	/* Align bytes */
	
	switch ((long) bytes & 3) {
	case 1:
		*bytes++	=	0;
		--len;
	case 2:
		*bytes++	=	0;
		--len;
	case 3:
		*bytes++	=	0;
		--len;
	case 0:
		break;
	}
	
	longs = (long *) bytes;
	count	= len >> 5;
	
	switch ((len >> 2) & 7) {
		do {
			*longs++ = 0;
		case 7:
			*longs++ = 0;
		case 6:
			*longs++ = 0;
		case 5:
			*longs++ = 0;
		case 4:
			*longs++ = 0;
		case 3:
			*longs++ = 0;
		case 2:
			*longs++ = 0;
		case 1:
			*longs++ = 0;
		case 0:
			;
		} while (count--);
	}
	
	bytes = (char *) longs;
	
	while (len-- & 3)
		*bytes++ = 0;
}

static void XLPasToZero(XCmdPtr params)
{
	if (
		PtrToHand(
			(char *)params->inArgs[0] + 1, 
			(Handle *) &params->outArgs[0], 
			*(StringPtr)params->inArgs[0]+1)
	)
		params->outArgs[0]	=	0;
	else
		(*(Handle)params->outArgs[0])[*(StringPtr)params->inArgs[0]] = 0;
		
	params->result	=	xresSucc;
}

static void XLZeroToPas(XCmdPtr params)
{
	char * end;
	
	end = (char *) memccpy((char *)params->inArgs[1] + 1, (char *)params->inArgs[0], 0, 254);
	
	if (end)
		*(char *)params->inArgs[1]	= end - (char *)params->inArgs[1] - 2;
	else 
		*(char *)params->inArgs[1]	= 254;
		
	params->result	=	xresSucc;
}

static char XLNumber[32];

static void XLGetNum(StringPtr num)
{
	int  len =	num[0];
	
	if (len > 31) {
		len = 31;
		XLNumber[31] = 0;
	}
	
	memcpy((void *)XLNumber, (void *) num, len);
}

static void XLStrToLong(XCmdPtr params)
{
	XLGetNum((StringPtr) params->inArgs[0]);
	
	* (unsigned long *)params->outArgs = strtoul(XLNumber, 0, 10);
	
	params->result	= xresSucc;	
}

static void XLStrToNum(XCmdPtr params)
{
	XLGetNum((StringPtr) params->inArgs[0]);

	params->outArgs[0] = strtol(XLNumber, 0, 10);
	
	params->result	= xresSucc;	
}

static void XLStrToBool(XCmdPtr params)
{
	params->result	= xresSucc;	

	if (EqualString((StringPtr) params->inArgs[0], (StringPtr) "\ptrue", false, true))
		params->outArgs[0]	=	1;
	else if (EqualString((StringPtr) params->inArgs[0], (StringPtr) "\pfalse", false, true))
		params->outArgs[0]	=	0;
	else
		params->result	= xresFail;	
}

static void XLStrToExt(XCmdPtr params)
{
	XLGetNum((StringPtr) params->inArgs[0]);

	*(extended *) params->inArgs[1] = atof(XLNumber);
	
	params->result	= xresSucc;	
}

static void XLLongToStr(XCmdPtr params)
{
	*(StringPtr)params->inArgs[1] = sprintf((char *)params->inArgs[1] + 1, "%u", params->inArgs[0]);
	
	params->result	= xresSucc;	
}

static void XLNumToStr(XCmdPtr params)
{
	*(StringPtr)params->inArgs[1] = sprintf((char *)params->inArgs[1] + 1, "%d", params->inArgs[0]);
	
	params->result	= xresSucc;	
}

static void XLNumToHex(XCmdPtr params)
{
	*(StringPtr)params->inArgs[2] = sprintf((char *)params->inArgs[2] + 1, "%0*X", params->inArgs[0], params->inArgs[1]);
	
	params->result	= xresSucc;	
}

static void XLBoolToStr(XCmdPtr params)
{
	if (params->inArgs[0])
		strcpy((char *) params->inArgs[1], "\ptrue");
	else
		strcpy((char *) params->inArgs[1], "\pfalse");
	
	params->result	= xresSucc;	
}


static void XLExtToStr(XCmdPtr params)
{
	*(StringPtr)params->inArgs[1] = sprintf((char *)params->inArgs[1] + 1, "%10G", *(extended *) params->inArgs[0]);
	
	params->result	= xresSucc;	
}

static void XLStringEqual(XCmdPtr params)
{
	params->outArgs[0] 	= 
		EqualString((StringPtr)params->inArgs[0], (StringPtr)params->inArgs[1], false, true);
	params->result			= xresSucc;	
}

static void XLReturnToPas(XCmdPtr params)
{
	int	 len;
	char * end	=	strchr((char *) params->inArgs[0], '\n');
	
	if (end) {
		if ((len = end - (char *) params->inArgs[0] - 1) > 254)
			len = 254;
	} else {
		if ((len = strlen((char *) params->inArgs[0])) > 254)
			len = 254;
	}
	
	strncpy((char *) params->inArgs[1]+1, (char *) params->inArgs[0], len);

	*(StringPtr) params->inArgs[1] = len;	
	params->result						 = xresSucc;	
}

static void XLScanToReturn(XCmdPtr params)
{
	Ptr *	ptr	=	(Ptr *) params->inArgs[0];
	
	for (; **ptr; ++*ptr)
		if (**ptr == '\n')
			break;
			
	params->result = xresSucc;	
}

static void XLScanToZero(XCmdPtr params)
{
	Ptr *	ptr	=	(Ptr *) params->inArgs[0];
	
	for (; **ptr; ++*ptr);
			
	params->result = xresSucc;	
}

XLGlue XLDefaultGlue	=	{
	XLUndefined,
	nil,
	nil,
	XLStringLength,
	XLStringMatch,
	nil,
	XLZeroBytes,
	XLPasToZero,
	XLZeroToPas,
	XLStrToLong,

	XLStrToNum,
	XLStrToBool,
	XLStrToExt,
	XLLongToStr,
	XLNumToStr,
	XLNumToHex,
	XLBoolToStr,
	XLExtToStr,
	nil,
	nil,

	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	XLStringEqual,
	XLReturnToPas,
	XLScanToReturn,
	nil,

	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	XLScanToZero
};
