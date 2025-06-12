/* drvrincludes.h 2.2 - part of Pete Resnick's driver package */

#ifndef __DRVRINCLUDES__
#define __DRVRINCLUDES__


/*
 *	This file customizes driver.c for your environment. It adjusts for
 *	differences between MPW C 3.x, THINK C 4.0.x, 5.0.x, and 6.0.x as
 *	well as allow you to change the resource ID for the THINK C patch.
 *	You should only need to change these next five lines. If you are
 *	using MPW, uncomment the first line and change the version number
 *	to the major release being used, and comment out the THINK_PROC and
 *	MACHEADERS lines. If you are using THINK C, change the THINK_PROC
 *	definition to the resource ID you are using for ThinkProc if you are
 *	using something different than 128 and comment or uncomment the other
 *	three lines depending on your environment.
 */

/* #define MPW				3	/* Using MPW, change the version number to suit */
#define THINK_PROC		128		/* Resource ID for THINK C patch */
#define MACHEADERS				/* Using THINK C MacHeaders */
/* #define THINK_4_SYS7			/* Using THINK C 4 and Sys 7 headers */
/* #define SYSEQU_H				/* Using SysEqu.h instead of LoMem.h */

/*
 *	You shouldn't need to change anything below here!!!!!
 */


#ifndef THINK_C
#	ifndef SYSEQU_H
#		define SYSEQU_H
#	endif /* SYSEQU_H */
#	undef MACHEADERS
#endif	/* THINK_C */

#ifndef MACHEADERS
#	if(!defined(THINK_C) || (THINK_C >= 5) || defined(THINK_4_SYS7))
#		include <Devices.h>
#		include <Errors.h>
#		include <Memory.h>
#		include <Resources.h>
#		include <ToolUtils.h>
#		include <Traps.h>
#		ifdef SYSEQU_H
#			include <SysEqu.h>
#		else
#			include <LoMem.h>
#		endif
#	else
#		include <DeviceMgr.h>
#		include <FileMgr.h>
#		include <MacTypes.h>
#		include <MemoryMgr.h>
#		include <ResourceMgr.h>
#		include <SysErr.h>
#		define PBOpenSync(x) PBOpen(x, false)
#		define PBCloseSync(x) PBClose(x, false)
#		define PBControlSync(x) PBControl(x, false)
#	endif
#endif

#ifndef nil
#	define nil ((void *) 0L)
#endif

#ifdef SYSEQU_H
#	define UTABLEBASE	(*(DCtlHandle **)UTableBase)
#	define UNITNTRYCNT	(*(short *)UnitNtryCnt)
#else
#	define UTABLEBASE	UTableBase
#	define UNITNTRYCNT	UnitNtryCnt
#endif

#ifndef dOpened
#	define dOpened		0x0020
#endif /* dOpened */
#ifndef dRAMBased
#	define dRAMBased	0x0040
#endif /* dRAMBased */
#ifndef offsetof
#	define offsetof(field,type) ((unsigned long)&(((type *)0L)->field))
#endif

/* This structure used in ThinkChangeCode and ThinkCleanup */
typedef struct {
	short inst12[2];
	union {
		short inst34[2];
		ResType theType;
	} p1;
	union {
		ResType theType;
		short inst34[2];
	} p2;
	union {
		short inst56[2];
		Ptr codePtr;
	} p3;
	short inst7;
} CodeChunk;

/* This structure used in ThinkAddRsrcs, ThinkRmvRsrcs, and THINK_PROC */
typedef struct {
	Handle rsrc;
	ResType typ;
	short id;
} RsrcRec;

/*
 *	DrvrInstall and DrvrRemove are documented in Technical Note 108.
 *	DisableInterrupts and ResetStatusRegister are used internally in GrowUTable.
 *	They are written as inline code for portability. THINK C 5 or greater and
 *	MPW 3 or greater can use the "#pragma parameter" directive.
 */
#if((defined(THINK_C) && THINK_C >= 5) || (defined(MPW) && MPW >= 3))
#pragma parameter __D0 DrvrInstall(__A0, __D0)
OSErr DrvrInstall(Handle drvrHandle, short refNum) = {0x2050,0xA03D};
#pragma parameter __D0 DrvrRemove(__D0)
OSErr DrvrRemove(short refNum) = 0xA03E;
short DisableInterrupts(void) = {0x40C0,0x46FC,0x2600};
#pragma parameter ResetStatusRegister(__D0)
void ResetStatusRegister(short oldSR) = 0x46C0;
#else
pascal OSErr DrvrInstall(Handle drvrHandle, short refNum)
	= {0x301F,0x205F,0x2050,0xA03D,0x3E80};
pascal OSErr DrvrRemove(short refNum)
	= {0x301F,0xA03E,0x3E80};
pascal short DisableInterrupts(void) = {0x40D7,0x46FC,0x2600};
pascal void ResetStatusRegister(short oldSR) = {0x46DF};
#endif

#endif /* __DRVRINCLUDES__ */