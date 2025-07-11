/* 
	GetMyIPAddr.h	
	C definitions of parameter block entries needed for IP calls

	Copyright Apple Computer, Inc. 1989 
	All rights reserved
	
	Modifications by Dave Nebinger (dnebing@andy.bgsu.edu), 01/30/94.
*/

#pragma once

#ifndef __H_GetMyIPAddr__
#define __H_GetMyIPAddr__

#define ipctlGetAddr		15			/* csCode to get our IP address */

// errors
enum {
	getipBaseErr=23300,			// the base address for this section
	getipNilPB,					// the GetIPParmBlkPtr was nil
	getipNilRefNum					// the ioCRefNum was nil
};

typedef struct GetIPParamBlock GetIPParamBlock, GetIPiopb,* GetIPParmBlkPtr,** GetIPParmBlkHdl;

typedef void (*GetIPCompletionProcPtr) (GetIPParmBlkPtr pb);

#if USESROUTINEDESCRIPTORS
	enum {
		uppGetIPCompletionProcInfo=kCStackBased
			| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(GetIPParmBlkPtr)))
	};
	
	typedef UniversalProcPtr GetIPCompletionUPP;
	
	#define CallGetIPCompletionProc(routine,iopb)\
		CallUniversalProc((UniversalProcPtr)routine,uppGetIPCompletionProcInfo,(iopb))
	#define NewGetIPCompletionProc(routine)\
		(GetIPCompletionUPP)NewRoutineDescriptor((ProcPtr)(routine),uppGetIPCompletionProcInfo,GetCurrentISA())
#else
	typedef GetIPCompletionProcPtr GetIPCompletionUPP;
	
	#define CallGetIPCompletionProc(routine,iopb)\
		(*(routine))((iopb))
	#define NewGetIPCompletionProc(routine)\
		(GetIPCompletionUPP)(routine)
#endif

// this macro will expand correctly for either case
#define DisposeGetIPCompletionProc(routine)\
	DisposeRoutineDescriptor((UniversalProcPtr)routine)

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct GetIPParamBlock {
	char 					fill12[12];
	GetIPCompletionUPP		ioCompletion;
	short 				ioResult;
	char 					*ioNamePtr;		
	short 				ioVRefNum;		
	short				ioCRefNum;			
	short 				csCode;
	ip_addr				ourAddress;				/* our IP address */
	long					ourNetMask;				/* our IP net mask */
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

// macros for allocating new blocks...
#define NewGetIPParmBlkPtr() NEWPTR(GetIPParmBlkPtr,sizeof(GetIPParamBlock))
#define DisposeGetIPParmBlkPtr(a) DISPOSEPTR(a)
#define NewGetIPParmBlkHdl() NEWHDL(GetIPParmBlkHdl,sizeof(GetIPParamBlock))
#define DisposeGetIPParmBlkHdl(a) DISPOSEHDL(a)

#ifdef __cplusplus
extern "C" {
#endif

OSErr GetMyIPAddr(GetIPParmBlkPtr pb,Boolean async);

#ifdef __cplusplus
}
#endif

#endif /* __H_GetMyIPAddr__ */
